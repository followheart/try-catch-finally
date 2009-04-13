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
/* 
 * 040801 Created
 * 150801 Modified dm1_thread_create() - added a level of indirection (thread_main)
 *        thus simplifying code for users of the thread library. No need to call 
 *        dm1_thread_init() at start of a thread, or dm1_thread_exit at end.
 * 160801 Added dm1_thread_cleanup()
 * 170801 Added a waiting loop in dm1_thread_wait() to deal with spurious wakeups
 * 060402 Ported to Win32 using PTHREADS-WIN32 library
 * 230402 Added new function dm1_thread_get_name().
 * 280402 Simplified use of condition variables to improve portability.
 * 190502 Added native support for Win32
 * 040802 C++ Port.
 * 130902 Introduced special class _MainThread to represent the initial thread.
 * 130902 Introduced Initializer class to initialize the mainThread.
 * 290902 Added Exceptions.
 * 041002 Added ThreadFactory.
 * 041002 Removed support for deriving from Thread class.
 * 041002 Added automatic garbage collection of threads.
 */

#include "dm1/thread.h"

namespace dm1 {

	/**
	 * A ThreadKey object represents the key for Thread Specific
	 * data. It is mainly used to initialize the key through the
	 * Constructor. By declaring a ThreadKey object as static, it is
	 * possible to initialize the key before any threads execute.
	 */
	class ThreadKey {

	friend class Thread;
	friend class _MainThread;
	friend class Initializer;

	private:
#if DM1_USE_PTHREAD
		pthread_key_t key;
#else
		DWORD key;
#endif

	private:
		ThreadKey();
#if DM1_USE_PTHREAD
		pthread_key_t getKey() { return key; }
#else
		DWORD getKey() { return key; }
#endif
	};

	class _Main : public Runnable {
	public:
		void run() { assert(0); }
	};

	/**
	 * The _MainThread class represents the initial thread
	 * run by the system. 
	 */
	class _MainThread : public Thread {
	public:
		_MainThread() : Thread(new _Main(), "_main") {
#if DM1_USE_PTHREAD
			tid = pthread_self();
#else
			tid = GetCurrentThreadId();
#endif
			init();
			if (debug) {
				fprintf(stdout, "Main thread id = %u\n", (unsigned) tid);
			}
		}
	};

	/**
	 * This is a helper class whose sole purpose is to initialize certain
	 * static objects in a particular order.
	 */
	class Initializer {
	public:
		Initializer() {
			Thread::key = new ThreadKey();
			ThreadFactory::mutex = new Mutex();
			ThreadFactory::threads = new LinkList(Thread::DM1_FACTORY_WAIT_LIST);
			Thread::mainThread = new _MainThread();
		}
		~Initializer() {
			ThreadFactory::shutdown();
		}
	};

	int Thread::debug = 0;
	int ThreadFactory::debug = 0;

	ThreadKey * Thread::key;
	Thread * Thread::mainThread;
	LinkList * ThreadFactory::threads;
	Mutex * ThreadFactory::mutex;

	/*
	 * The initializer object initializes the above two objects.
	 */
	Initializer initialize;

	/**
	 * Construct a thread key.
	 */
	ThreadKey::ThreadKey()
	{
#if DM1_USE_PTHREAD
		int rc = pthread_key_create(&key, 0);
		if (rc != 0) {
#else
		key = TlsAlloc();
		if (key == TLS_OUT_OF_INDEXES) {
			int rc = GetLastError();
#endif
			// Not much point continuing !
			fprintf(stderr, "ThreadKey.ctor: Error: Failed to create ThreadKey\n");
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_UNEXPECTED, rc);
		}

		if (Thread::debug)
			fprintf(stdout, "ThreadKey.ctor: Created ThreadKey %d\n", key);
	}

	/**
	 * A C wrapper to C++ function. 
	 */
	static void 
	dm1_thread_cleanup(
		void *arg)
	{
		Thread *me = (Thread *)arg;
		me->cleanup();
	}

	/**
	 * The Thread library expects the startup function to be in C.
	 * We therefore wrap a C function around the C++ method.
	 */
#if DM1_USE_PTHREAD
	static void *
#else
	static unsigned __stdcall
	//static DWORD WINAPI
#endif
	dm1_thread_main(void *arg)
	{
		Thread *me = (Thread *)arg;
		me->execute();
		return 0;
	}

	/**
	 * Initialize the thread. This is called immediately after the thread has
	 * started executing. 
	 */
	void 
	Thread::init()
	{
		ENTER("Thread.init");

		ThreadFactory::lock();
		status = DM1_THREAD_RUNNING;
		ThreadFactory::unlock();

		if (debug)
			fprintf(stdout, "Thread.init: Initializing thread %s\n", getName());
#if !DM1_USE_PTHREAD
		assert(tid == GetCurrentThreadId());
		if (! TlsSetValue(key->getKey(), (LPVOID) this)) {
			int rc = GetLastError();
#else
		tid = pthread_self();	/* BUG in Linux pthread package ?? */
		int rc = pthread_setspecific(key->getKey(), (const void *)this);
		if (rc != 0) {
#endif
			fprintf(stderr, "Thread.init: Error: Failed to set key value\n");
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_THREAD_CREATE, rc);
		}

		EXIT("Thread.init");
	}

	/**
	 * Cleanup the thread. This is called before the thread terminates.
	 */
	void 
	Thread::cleanup()
	{
		ENTER("Thread.cleanup");
		if (debug)
			fprintf(stdout, "Thread.cleanup: Cleaning up thread %s\n", getName());
#if !DM1_USE_PTHREAD
		TlsSetValue(key->getKey(), 0);
#else
		pthread_setspecific(key->getKey(), 0);
#endif
		ThreadFactory::lock();
		status = DM1_THREAD_DEAD;
		ThreadFactory::unlock();

		EXIT("Thread.cleanup");
	}


	/**
	 * This is the wrapper around the run() method.
	 * It performs initialization and cleanup, and also handles any
	 * exceptions raised by the run() method.
	 */
	void 
	Thread::execute()
	{
        ENTER("Thread.execute");
		if (debug)
			fprintf(stdout, "Thread.execute: Executing the run method of thread %s\n", getName());
		init();
#if DM1_USE_PTHREAD
		pthread_cleanup_push(dm1_thread_cleanup, this);
#endif
		try {
			toRun->run();
		}
		catch (Exception e) {
			fprintf(stderr, "Thread.execute: Exception caught while executing thread %s\n",
				getName());
			e.dump(stderr);
		}
		catch (...) {
			fprintf(stderr, "Thread.execute: Unknown Exception caught while executing thread %s\n",
				getName());
		}
#if !DM1_USE_PTHREAD
		cleanup();
#else
		pthread_cleanup_pop(1);
#endif
		EXIT("Thread.execute");
	}

	/**
	 * This is the method that should be called to start the Thread
	 */
	void 
	Thread::start() 
	{
		ENTER("Thread.start");
		if (debug)
			fprintf(stdout, "Thread.start: Starting thread %s\n", getName());
#if DM1_USE_PTHREAD
		int rc = pthread_create(&tid, 0, dm1_thread_main, this);
		if (rc != 0) {
#else
		handle = (HANDLE) _beginthreadex(NULL, 0, dm1_thread_main, 
			this, 0, (unsigned int *)&tid);
		//handle = (HANDLE) CreateThread(NULL, 0, dm1_thread_main, 
		//	this, CREATE_SUSPENDED, &tid);
		//if (handle == 0 || ResumeThread(handle) == (DWORD)-1) {
		if (handle == 0) {
			int rc = GetLastError();
#endif
			fprintf(stderr, "Thread.start: Error: Failed to start thread %s\n", getName());
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_THREAD_CREATE, rc);
		}
		EXIT("Thread.start");
	}

	/**
	 * The Thread constructor.
	 */
	Thread::Thread(Runnable *toRun, const char *name, bool isDaemon) : DLinkable()
	{
		ENTER("Thread.ctor");
		status = DM1_THREAD_UNUSED;
		this->isDaemon = isDaemon;
		this->toRun = toRun;
		toRun->setThread(this);
		latchMode = 0;
		isSignaled = 0;
		this->name = strdup(name);
		status = DM1_THREAD_INITED;
		EXIT("Thread.ctor");
	}

	/**
	 * Terminates the thread.
	 */
	void 
	Thread::exit()
	{
		ENTER("Thread.exit");
		if (debug)
			fprintf(stdout, "Thread.exit: Exiting thread %s\n", getName());
	#if !DM1_USE_PTHREAD
		cleanup();
		_endthreadex(0);
		//ExitThread(0);
	#else
		pthread_exit(0);
	#endif
		EXIT("Thread.exit");
	}

	/**
	 * Waits until there is a timeout or some other thread signals this
	 * thread.
	 */
	int
	Thread::wait(unsigned secs)
	{
		ENTER("Thread.wait");
		if (debug) 
			fprintf(stdout, "Thread.wait: Thread %s going to sleep\n",
				getName());
		int rc;
		if (secs == 0)
			return 0;
		rc = event.wait(secs);
		if (debug) 
			fprintf(stdout, "Thread.wait: Thread %s ended sleep, rc = %d\n",
				getName(), rc);
		EXIT("Thread.wait");
		return rc;
	}

	/**
	 * Waits until some other thread wakes it up.
	 */
	void
	Thread::wait()
	{
		ENTER("Thread.wait");
		if (debug) 
			fprintf(stdout, "Thread.wait: Thread %s going to sleep\n",
				getName());
		event.wait();
		EXIT("Thread.wait");
	}

	/**
	 * Wakes up a waiting thread.
	 */
	void 
	Thread::notify(Thread *other)
	{
		ENTER("Thread.notify");
		if (debug) 
			fprintf(stdout, "Thread.notify: Notifying thread %s\n",
				other->getName());
		other->event.notify();
		EXIT("Thread.notify");
	}

	/**
	 * Waits for the thread to complete.
	 */
	void 
	Thread::join()
	{
		ENTER("Thread.join");
		if (debug) 
			fprintf(stdout, "Thread.join: Waiting for thread %s\n",
				getName());

#if DM1_USE_PTHREAD
		int rc = pthread_join(tid, 0);
		if (rc != 0) {
#else
		if (WaitForSingleObject(handle, INFINITE) != WAIT_FAILED) {
			CloseHandle(handle);
		}
		else {
			DWORD rc = GetLastError();
#endif
			fprintf(stderr, "Thread.join: Error: Failed to join thread %s\n", getName());
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_THREAD_JOIN, rc);
		}
		if (debug) 
			fprintf(stdout, "Thread.join: Wait for thread %s completed\n",
				getName());
	
		ThreadFactory::lock();
		status = DM1_THREAD_UNUSED;
		ThreadFactory::unlock();

		EXIT("Thread.join");
	}

	/**
	 * Destroys the Thread object.
	 * IMPORTANT: Thread must be joined before this is called.
	 */
	Thread::~Thread() 
	{
		ENTER("Thread.dtor");
		if (this != Thread::mainThread && status != DM1_THREAD_UNUSED) {
			fprintf(stderr, "Thread.dtor: Error: Thread object destroyed before Thread %s has been joined\n", getName());
		}
		free((void *)name);
		name = 0;
		delete toRun;
		toRun = 0;
		EXIT("Thread.dtor");
	}

	/**
	 * Returns the current Thread object.
	 */
	Thread *
	Thread::getCurrentThread()
	{
		ENTER("Thread.getCurrentThread");
		Thread *current;
#if DM1_USE_PTHREAD
		current = (Thread *)pthread_getspecific(key->getKey());
#else
		current = (Thread *)TlsGetValue(key->getKey());
#endif
		EXIT("Thread.getCurrentThread");
		return current;
	}

	void ThreadFactory::garbageCollect(bool shuttingdown) {
		ENTER("ThreadFactory.garbageCollect");

		/* Wait for daemon threads */
		Thread *t = (Thread *) threads->getFirst();
		while (t != 0) {
			if (t->isDaemon && 
				(shuttingdown && t->status != Thread::DM1_THREAD_UNUSED || 
				!shuttingdown && t->status == Thread::DM1_THREAD_DEAD)) {
				unlock();
				t->join();
				lock();
				t = (Thread *) threads->getFirst();
				continue;
			}
			else {
				t = t->getNextFactoryThread();
			}
		}

		/* Delete threads that are done */
		t = (Thread *) threads->getFirst();
		while (t != 0) {
			Thread *next = t->getNextFactoryThread();
			if (t->status == Thread::DM1_THREAD_UNUSED) {
				threads->remove(t);
				if (debug)
					fprintf(stdout, "ThreadFactory.garbageCollect: Destroying %s\n", t->getName());
				delete t;
			}
			t = next;
		}

		EXIT("ThreadFactory.garbageCollect");
	}

	Thread * ThreadFactory::createThread(
		Runnable *toRun, 
		const char *name, 
		bool isDaemon) {

		ENTER("ThreadFactory.createThread");
		lock();
		garbageCollect();
		Thread *t = new Thread(toRun, name, isDaemon);
		threads->append(t);
		unlock();
		EXIT("ThreadFactory.createThread");
		return t;
	}

	void ThreadFactory::shutdown() {
		ENTER("ThreadFactory.shutdown");
		lock();
		garbageCollect(true);
		if (threads->getCount() != 0) {
			fprintf(stderr, "ThreadFactory.shutdown: Error: There are still some active threads\n");
		}
		unlock();
		EXIT("ThreadFactory.shutdown");
	}
}

