/***
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    Author : Dibyendu Majumdar
*    Email  : dibyendu@mazumdar.demon.co.uk
*/
// The code here is an adaptation of the Java threadpool code in the Jakarta
// Tomcat Connectors project. It is different from the Java code in the 
// following ways:
// 1. In Java, certain operations on primitive types are guaranteed to be
//    thread safe. C++ offers no such guarantee - hence there is more
//    locking.
// 2. In Java, Threads and other objects are automatically garbage collected.
//    In C++, we have to wait and cleanup each thread.
// 3. The original code did not have support for queueing tasks, i.e., if
//    all threads are busy, the Pool would block the caller until a thread became
//    available to run the task. This version allows the caller to queue a task and
//    not wait for its execution. A separate thread, called the QueueHandler, monitors the
//    task queue, and runs any tasks in the queue.
//
// 15 sep 2002: first working version.
// 27 sep 2002: added queueTask()
// 28 sep 2002: added taskReady monitor.
// 04 oct 2002: code simplified due to garbage collection in ThreadFactory

#include "dm1/threadpool.h"

namespace dm1 {

/**
 * An instance of MonitorThread is created for each ThreadPool.
 * MonitorThread is responsible for terminating excess idle threads.
 */
MonitorThread::MonitorThread(ThreadPool *pool) {
	this->pool = pool;
	shouldTerminate = false;
	Thread *t = ThreadFactory::createThread(this, "MonitorThread", true);
	t->start();
}

/**
 * Executes the main body of MonitorThread.
 */
void MonitorThread::run() {
	while (true) {
		// Sleep for a bit
		wait(pool->getMonitorTimeout());

		// Check if I should terminate.
		// termination happens when the pool is shutting down.
		mutex.lock();
		bool st = shouldTerminate;
		mutex.unlock();
		if (st) {
			break;
		}

		// Harvest idle threads.
		pool->harvestSpareWorkers();
	}
	if (ThreadPool::getDebug())
		fprintf(stdout, "MonitorThread.run: %s thread terminated\n", getName());
}

/**
 * Instructs the MonitorThread to terminate.
 */
void MonitorThread::terminate() {
	mutex.lock();
	shouldTerminate = true;
	mutex.unlock();

	// Wake up the MonitorThread in case it is sleeping.
	Thread::notify(getThread());
}

/**
 * The QueueHandlerThread executes queued tasks. 
 */
QueueHandlerThread::QueueHandlerThread(ThreadPool *pool) {
	this->pool = pool;
	shouldTerminate = false;
	Thread *t = ThreadFactory::createThread(this, "QueueHandlerThread", true);
	t->start();
}

/**
 * Executes the main body of QueueHandlerThread.
 */
void QueueHandlerThread::run() {
	while (true) {
		Task * task = pool->getTask();

		if (task == 0) {
			// Wait until we are notified that a new task has arrived
			taskReady.lock();
			taskReady.wait();
			taskReady.unlock();
		}
		else {
			pool->runTask(task);
		}

		mutex.lock();
		bool st = shouldTerminate;
		mutex.unlock();

		// Check if I should terminate.
		// termination happens when the pool is shutting down.
		if (st)
			break;
	}
	if (ThreadPool::getDebug())
		fprintf(stdout, "QueueHandlerThread.run: %s thread terminated\n", getName());
}

/**
 * Instructs the QueueHandlerThread to stop.
 */
void QueueHandlerThread::terminate() {
	mutex.lock();
	shouldTerminate = true;
	mutex.unlock();
	notifyTaskReady();	/* In case the thread is waiting */
}

void QueueHandlerThread::notifyTaskReady() {
	taskReady.notify();
}


/**
 * A WorkerThread is responsible for executing tasks.
 * TODO: Give a meaningful name to each thread.
 */
WorkerThread::WorkerThread(ThreadPool *pool) {
	this->pool = pool;
	toRun = 0;
	po = 0;
	shouldTerminate = false;
	shouldRun = false;
	Thread *t = ThreadFactory::createThread(this, "WorkerThread", true);
	t->start();
}

/**
 * Executes the main body of the WorkerThread.
 */
void WorkerThread::run() {

	while (true) {

		cond.lock();
		while ( !shouldRun && !shouldTerminate ) {
			cond.wait();
		}
		bool sr = shouldRun;
		bool st = shouldTerminate;
		cond.unlock();

		if ( st ) {
			break;
		}

		if ( sr ) {
			// This is safe because this thread is 
			// not going to be touched until it returns itself
			// to the pool.

			try {
				toRun->run();
			}
			catch(Exception e) {
				fprintf(stderr, "WorkerThread.run: Caught exception raised from %s(%d)\n",
					e.getFilename(), e.getLine());
			}
			catch(...) {
				fprintf(stderr, "WorkerThread.run: Caught exception raised by task\n");
			}
			delete toRun;
			toRun = 0;
			shouldRun = false;
			pool->returnWorkerThread(this);
		}
	}

	delete toRun;
	shouldTerminate = false;
	shouldRun = false;
	toRun = 0;
	if (ThreadPool::getDebug())
		fprintf(stdout, "WorkerThread.run: %s thread terminated\n", getName());
}

/**
 * Instructs the WorkerThread to stop.
 */
void WorkerThread::terminate() {
	cond.lock();
	shouldTerminate = true;
	cond.notify();
	cond.unlock();
}

/**
 * Requests that the WorkerThread execute a task.
 */
void WorkerThread::schedule(Task *toRun) {
	cond.lock();
	shouldRun = true;
	this->toRun = toRun;
	cond.notify();
	cond.unlock();
}

int ThreadPool::debug = 0;

/**
 * Creates the ThreadPool.
 */
ThreadPool::ThreadPool(int maxThreads, int minSpareThreads, 
		       int maxSpareThreads, int monitorTimeout,
		       int queueHandlerTimeout) {
	this->maxThreads      = maxThreads;
	this->maxSpareThreads = maxSpareThreads;
	this->minSpareThreads = minSpareThreads;
	this->monitorTimeout = monitorTimeout;
	this->queueHandlerTimeout = queueHandlerTimeout;
	currentThreadCount  = 0;
	currentThreadsBusy  = 0;
	stopThePool = false;
}

/**
 * Starts the ThreadPool. Initial WorkThreads are created.
 * The MonitorThread and the QueueHandlerThread are started.
 */
void ThreadPool::start() {
	cond.lock();
	stopThePool = false;
	currentThreadCount  = 0;
	currentThreadsBusy  = 0;

	adjustLimits();

	// Start initial threads
	openThreads(minSpareThreads);

	// Start monitor thread
	monitor = new MonitorThread(this);

	// Start queue Handler thread
	queueHandler = new QueueHandlerThread(this);
	
	cond.unlock();
}

/**
 * Gets the next queued task.
 */
Task *
ThreadPool::getTask() {
	taskQueueLock.lock();
	Task *t = (Task *)taskQueue.pop();
	taskQueueLock.unlock();
	return t;
}
/**
 * Enqueues a task for execution.
 */
void
ThreadPool::queueTask(Task *toRun) {

	cond.lock();
	if (0 == currentThreadCount || stopThePool) {
		cond.unlock();
		delete toRun;
		return;
	}
	cond.unlock();
	
	taskQueueLock.lock();
	assert( !taskQueue.contains(toRun));
	taskQueue.prepend(toRun);
	taskQueueLock.unlock();

	queueHandler->notifyTaskReady();
}

/**
 * Run a task using one of the Worker Threads.
 * Note: Caller blocks if there is no available thread.
 */
void 
ThreadPool::runTask(Task *r) {

	cond.lock();
	if (0 == currentThreadCount || stopThePool) {
		cond.unlock();
		delete r;
		return;
	}
	// Obtain a free thread from the pool.
	if (currentThreadsBusy == currentThreadCount) {
		// All threads are busy
		if(currentThreadCount < maxThreads) {
			// Not all threads were open,
			// Open new threads up to the max number of idel threads
			int toOpen = currentThreadCount + minSpareThreads;
			openThreads(toOpen);
		} 
		else {
			// Wait for a thread to become idle.
			while (currentThreadsBusy == currentThreadCount) {
				if (debug) {
					fprintf(stdout, "All worker threads are busy ... waiting\n");
				}
				cond.wait();
			}

			// Pool was stopped. Get away of the pool.
			if (0 == currentThreadCount || stopThePool) {
				cond.unlock();
				return;
			}
		}
	}

	PoolObject *po = (PoolObject *) idleThreads.pop();
	WorkerThread *c = po->getThread();
	currentThreadsBusy++;
	cond.unlock();

	c->schedule(r);
}


/**
 * Stop the thread pool
 */
void 
ThreadPool::shutdown() {

	PoolObject *po;

	if (debug)
		fprintf(stdout, "ThreadPool.shutdown: Shutting down ThreadPool\n");

	cond.lock();
	if ( !stopThePool ) {
		stopThePool = true;

		// Instruct all the threads to stop
		monitor->terminate();
		queueHandler->terminate();
		for (po = (PoolObject *)idleThreads.pop();
			po != 0;
			po = (PoolObject *)idleThreads.pop()) {
			WorkerThread *t = po->getThread();
			t->terminate();
			delete po;
		}
		// Active threads do not need to be instructed 
		// because they will notice anyway.
	}
	currentThreadsBusy = currentThreadCount = 0;
	cond.notifyAll();
	cond.unlock();

	monitor = 0;
	queueHandler = 0;

	Task *task = (Task *)taskQueue.pop();
	while (task != 0) {
		delete task;
		task = (Task *)taskQueue.pop();
	}
}

/**
 * Called by the monitor thread to harvest idle threads.
 */
void 
ThreadPool::harvestSpareWorkers() {

	PoolObject *po;
	int toFree = 0;

	cond.lock();
	if (stopThePool) {
		cond.unlock();
		return;
	}
	if ((currentThreadCount - currentThreadsBusy) > maxSpareThreads) {
		toFree = currentThreadCount -
			currentThreadsBusy -
			maxSpareThreads;

		if (debug) 
			fprintf(stdout, "ThreadPool.harvestSpareWorkers: Freeing %d threads because they are idle\n",
				toFree);
		for (int i = 0 ; i < toFree ; i++) {
			po = (PoolObject *) idleThreads.pop();
			WorkerThread *c = po->getThread();
			c->terminate();
			currentThreadCount --;
		}
		if (debug)
			fprintf(stdout, "ThreadPool.harvestSpareWorkers: Current Thread Count = %d\n",
				currentThreadCount);
	}
	cond.unlock();
}

/**
 * Returns the thread to the pool.
 * Called by threads as they are becoming idle.
 */
void 
ThreadPool::returnWorkerThread(WorkerThread *c) {

	cond.lock();
	if (0 == currentThreadCount || stopThePool) {
		c->terminate();
	}
	else {
		currentThreadsBusy--;
		PoolObject *po = c->getPoolObject();
		idleThreads.push(po);
		cond.notify();
	}
	cond.unlock();
}


/*
 * Checks for problematic configuration and fix it.
 * The fix provides reasonable settings for a single CPU
 * with medium load.
 */
void 
ThreadPool::adjustLimits() {
	if(maxThreads <= 0) {
		maxThreads = MAX_THREADS;
	}

	if(maxSpareThreads >= maxThreads) {
		maxSpareThreads = maxThreads;
	}

	if(maxSpareThreads <= 0) {
		if(1 == maxThreads) {
			maxSpareThreads = 1;
		} else {
			maxSpareThreads = maxThreads/2;
		}
	}

	if(minSpareThreads >  maxSpareThreads) {
		minSpareThreads =  maxSpareThreads;
	}

	if(minSpareThreads <= 0) {
		if(1 == maxSpareThreads) {
			minSpareThreads = 1;
		} else {
			minSpareThreads = maxSpareThreads/2;
		}
	}
}

/**
 * Starts a number of new threads if necessary.
 */
void 
ThreadPool::openThreads(int toOpen) {

	if (toOpen > maxThreads) {
		toOpen = maxThreads;
	}

	if (debug)
		fprintf(stdout, "ThreadPool.openThreads: Starting %d new threads\n", toOpen-currentThreadCount);
	for (int i = currentThreadCount ; i < toOpen ; i++) {
		WorkerThread *c = new WorkerThread(this);
		PoolObject *po = new PoolObject(c);
		idleThreads.push(po);
	}

	currentThreadCount = toOpen;
}

}

