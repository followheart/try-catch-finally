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
#ifndef dm1_monitor_h
#define dm1_monitor_h

#include "dm1/common.h"
#include "dm1/except.h"
#include "dm1/port.h"
#include "dm1/list.h"
#include "dm1/mutex.h"
#include "dm1/thread.h"

namespace dm1 {

	class Monitor {

	protected:
		static int debug;

	protected:
		Mutex mutex;
		Thread *holder;		
		int lockCount;
		LinkList waitQ;
		LinkList readyQ;

	protected:
		void grantMonitor();

	private:
		Monitor(const Monitor&);
		Monitor& operator=(const Monitor&);

	public:
		/**
		 * Creates a Monitor Object.
		 */
		explicit Monitor();

		/**
		 * Destroys a Monitor object.
		 */
		~Monitor();

		/**
		 * Requests a lock on the Monitor object. 
		 * Will block if Lock is not available.
		 */
		void lock();
		inline void enter() { lock(); }

		/**
		 * Requests a lock on the Monitor object. 
		 * Will return false if Lock is not available.
		 * Returns true if lock was acquired.
		 */
		bool tryLock();
		inline bool tryEnter() { return tryLock(); }

		/** 
		 * Requester must be a holder of the lock. 
		 * If the Unlock succeeds, the lock is 
		 * granted to the oldest waiting thread 
		 * on the ReadyQ.
		 */
		void unlock();
		inline void exit() { unlock(); }

		/**
		 * Requester must be a holder of the lock. 
		 * The lock is released and requester put 
		 * to sleep until the Monitor is signalled. 
		 * Lock is re-acquired before control 
		 * returns to the caller.
		 */
		void wait();

		/**
		 * Requester must be a holder of the lock. 
		 * The lock is released and requester put 
		 * to sleep until the Monitor is signalled 
		 * or the timeout expires. Lock is re-acquired 
		 * before control returns to the caller. 
		 */
		bool wait(unsigned secs);

		/**
		 * Signals and wakes up one thread waiting on 
		 * the Monitor object. The thread woken up may 
		 * not be the one signalled.
		 */
		void notify();

		/**
		 * Signalls all waiting threads and wakes up 
		 * one thread.
		 */
		void notifyAll();

		void dump(FILE *fp = stdout);
		static void setDebug(int value) { debug = value; }
	};

}

#endif


