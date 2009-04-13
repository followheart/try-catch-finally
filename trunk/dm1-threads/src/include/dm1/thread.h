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
#ifndef dm1_thread_h
#define dm1_thread_h

#include "dm1/common.h"
#include "dm1/except.h"
#include "dm1/port.h"
#include "dm1/list.h"
#include "dm1/mutex.h"
#include "dm1/event.h"

namespace dm1 {

	class Thread;
	class ThreadKey;

	class Runnable {
	friend class Thread;

	protected:
		void *arg;

	private:
		Thread *owner;

	private:
		void setThread(Thread *owner) { this->owner = owner; }

	public:
		Runnable() { arg = 0; owner = 0; }
		virtual ~Runnable() {}

		/** 
		 * Derived classes must implement this method. This is
		 * the main body of the thread.
		 */
		virtual void run() {}
		
		/**
		 * Sets an arbitrary argument for the thread.
		 */
		void *getArg() const { return arg; }

		/**
		 * Returns the thread argument.
		 */
		void setArg(void *arg) { this->arg = arg; }

		/**
		 * Each Runnable object has an associated Thread that
		 * owns it. This method returns the Thread owning
		 * the object.
		 */
		Thread *getThread() const { return owner; }

		/** 
		 * Unconditional wait. Puts the owning thread to sleep 
		 * until another thread signals it.
		 */
		void wait();

		/**
		 * Conditional wait. If the thread is not signalled
		 * before the timeout expires, then this method returns
		 * ETIMEDOUT.
		 */
		int  wait(unsigned secs);

		/**
		 * Terminates the owning thread.
		 */
		void exit();

		/**
		 * Returns the owning thread’s name.
		 */
		const char *getName() const;
	};

	class Thread : public DLinkable {

		friend class ThreadKey;
		friend class ThreadFactory;
		friend class Initializer;
		friend class Monitor;
		friend class Latch;

	protected:
		enum ThreadStatus {
			DM1_THREAD_UNUSED = 0,
			DM1_THREAD_INITED = 1,
			DM1_THREAD_RUNNING = 2,
			DM1_THREAD_DEAD = 3
		};

		enum {
			DM1_LATCH_WAIT_LIST = 0,
			DM1_MONITOR_WAIT_LIST = 0,
			DM1_FACTORY_WAIT_LIST = 1
		};

		/**
		 * Name of the thread.
		 */
		const char *name;

		/**
		 * Status of the thread.
		 */
		ThreadStatus status;

#if !DM1_USE_PTHREAD
		HANDLE handle;
		DWORD tid;
#else
		pthread_t tid;
#endif
		Event event;

		// Following member used by Latch object
		int latchMode;

		// Following member used by Condition object
		bool isSignaled;

		Runnable *toRun;

		bool isDaemon;

	protected:
		static ThreadKey *key;
		static Thread *mainThread;
		static int debug;

	protected:
		void init();
		Thread *getNextLatchWaiter() const {
			return (Thread *)getNext((int)DM1_LATCH_WAIT_LIST);
		}
		Thread *getNextMonitorWaiter() const {
			return (Thread *)getNext((int)DM1_MONITOR_WAIT_LIST);
		}
		Thread *getNextFactoryThread() const {
			return (Thread *)getNext((int)DM1_FACTORY_WAIT_LIST);
		}

	public:
		/* Following are really private functions */
		void cleanup();
		void execute();

	protected:
		explicit Thread(Runnable *toRun, const char *name = "UnNamed Thread", bool isDaemon = false);

	private:
		Thread(const Thread&);
		Thread& operator=(const Thread&);

	public:
		virtual ~Thread();

		/** 
		 * Unconditional wait. Puts the thread to sleep until 
		 * another thread signals it.
		 */
		void wait();

		/**
		 * Conditional wait. If the thread is not signalled 
		 * before the timeout expires, then this method returns 
		 * ETIMEDOUT.
		 */
		int  wait(unsigned secs);

		/**
		 * Terminates the thread.
		 */
		void exit();

		/**
		 * Blocks the caller until the thread being joined 
		 * terminates. 
		 */
		void join();

		/**
		 * Creates an Operating system thread and executes the 
		 * run() method of the Runnable object.
		 */
		void start();

		/**
		 * Returns the thread’s name.
		 */
		const char *getName() const { return name; }

		/**
		 * Returns the Runnable object associated with the 
		 * Thread.
		 */
		Runnable *getRunnable() const { return toRun; }
		
		/**
		 * Sets an arbitrary argument for the thread.
		 */
		void setArg(void *arg) { toRun->setArg(arg); }

		/**
		 * Returns the thread argument.
		 */
		void *getArg() { return toRun->getArg(); }

		/**
		 * Sends a signal to Thread “other”.
		 */
		static void notify(Thread *other);

		/** 
		 * Returns the Thread object that represents the current 
		 * thread.
		 */
		static Thread *getCurrentThread();

		/**
		 * Returns the Thread object that represents the main 
		 * thread.
		 */
		static Thread *getMainThread() { return mainThread; }

		static void setDebug(int value) { debug = value; }
	};

	class ThreadFactory {

	friend class Initializer;
	friend class Thread;

	protected:
		static LinkList *threads;
		static Mutex *mutex;
		static int debug;

	protected:
		static void lock() { mutex->lock(); }
		static void unlock() { mutex->unlock(); }
		static void garbageCollect(bool shuttingdown = false);

	public:
		static void shutdown();

		/** 
		 * Creates a new Thread instance.
		 */
		static Thread *createThread(Runnable *toRun, 
			const char *name = "UnNamed Thread", 
			bool isDaemon = false);

		/**
		 * Enables debug messages.
		 */
		static void setDebug(int value) { debug = value; }
	};

	inline void Runnable::wait() { owner->wait(); }
	inline int  Runnable::wait(unsigned secs) { return owner->wait(secs); }
	inline void Runnable::exit() { owner->exit(); }
	inline const char *Runnable::getName() const { return owner->getName(); }
}

#endif
