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
#ifndef dm1_threadpool_h
#define dm1_threadpool_h

#include "dm1/common.h"
#include "dm1/port.h"
#include "dm1/list.h"
#include "dm1/mutex.h"
#include "dm1/thread.h"
#include "dm1/monitor.h"

namespace dm1 {

	/**
	 * Base class for all tasks. 
	 */
	class Task : public SLinkable {
	public:
		virtual ~Task() {}

		/**
		 * Derived classes need to implement this method.
		 */
		virtual void run() = 0;
	};

	class ThreadPool;

	/**
	 * An instance of MonitorThread class is responsible for monitoring spare Worker threads
	 * and terminating them when necessary.
	 */
	class MonitorThread : public Runnable {

	friend class ThreadPool;

	private:
		ThreadPool *pool;
		bool shouldTerminate;
		Mutex mutex;

	private:
		MonitorThread(ThreadPool *pool);
		void run();
		void terminate();
	};

	class PoolObject;

	/**
	 * The ThreadPool contains a pool of Worker threads. A Worker thread is responsible
	 * for executing a task.
	 */
	class WorkerThread : public Runnable {

	friend class PoolObject;
	friend class ThreadPool;

	private:
		Task *toRun;
		ThreadPool *pool;
		bool shouldTerminate;
		bool shouldRun;
		Monitor cond;
		PoolObject *po;

	private:
		void setPoolObject(PoolObject *po) { this->po = po; }
		PoolObject *getPoolObject() const { return po; }

	private:
		WorkerThread(ThreadPool *pool);
		void run();
		void schedule(Task *toRun);
		void terminate();
	};

	class QueueHandlerThread : public Runnable {

	friend class ThreadPool;

	private:
		ThreadPool *pool;
		bool shouldTerminate;
		Mutex mutex;
		Monitor taskReady;

	private:
		QueueHandlerThread(ThreadPool *pool);
		void run();
		void terminate();
		void notifyTaskReady();
	};


	/**
	 * This is a helper class to allow WorkerThread objects to be
	 * maintained in a linked list.
	 */
	class PoolObject : public SLinkable {

	friend class ThreadPool;

	private:
		WorkerThread *t;

	private:
		PoolObject(WorkerThread *t) {
			this->t = t;
			t->setPoolObject(this);
		}
		WorkerThread *getThread() {
			return t;
		}
	};

	/**
	 * The ThreadPool class implements a pool of Worker threads. Tasks can be
	 * queued for execution by one of the Worker threads. The pool is dynamic - it will
	 * start new worker threads if necessary, or terminate existing worker threads when
	 * no longer required.
	 */
	class ThreadPool {

	friend class MonitorThread;
	friend class WorkerThread;
	friend class QueueHandlerThread;

	public:
		enum {
			MAX_THREADS = 200,
			MAX_SPARE_THREADS = 50,
			MIN_SPARE_THREADS = 5,
			WORK_WAIT_TIMEOUT = 60,
			QUEUE_HANDLER_TIMEOUT = 30
		};

	protected:
		LinkList idleThreads;
		LinkList taskQueue;
		Mutex taskQueueLock;
		MonitorThread *monitor;
		QueueHandlerThread *queueHandler;
		int maxThreads;
		int minSpareThreads;
		int maxSpareThreads;
		int monitorTimeout;
		int queueHandlerTimeout;
		int currentThreadCount;
		int currentThreadsBusy;
		bool stopThePool;
		Monitor cond;
		static int debug;

	protected:
		void harvestSpareWorkers();
		void returnWorkerThread(WorkerThread *c);
		void adjustLimits();
		void openThreads(int toOpen);
		int  getMonitorTimeout() const { return monitorTimeout; }
		int  getQueueHandlerTimeout() const { return queueHandlerTimeout; }
		Task *getTask();

	private:
		ThreadPool(const ThreadPool&);
		ThreadPool& operator=(const ThreadPool&);

	public:

		/**
		 * Creates a thread pool.
		 */
		explicit ThreadPool(
			int maxThreads = MAX_THREADS, 
			int minSpareThreads = MIN_SPARE_THREADS,
			int maxSpareThreads = MAX_SPARE_THREADS, 
			int monitorTimeout = WORK_WAIT_TIMEOUT,
			int queueHandlerTimeout = QUEUE_HANDLER_TIMEOUT);

		/**
		 * Starts the ThreadPool.
		 */
		void start();

		/**
		 * Synchronously executes the task provided.
		 * Caller blocks until the task is completed.
		 * Task must be allocated on the heap.
		 * The pool will delete the task when it is
		 * completed.
		 */
		void runTask(Task *toRun);

		/**
		 * Asynchronously runs the task submitted.
		 * Control returns to the caller immediately.
		 * Task must be allocated on the heap.
		 * The pool will delete the task when it is
		 * completed.
		 */
		void queueTask(Task *toRun);

		/**
		 * Shuts down the pool.
		 */
		void shutdown();

		static void setDebug(int level) { debug = level; }
		static int getDebug() { return debug; }
	};

}

#endif
