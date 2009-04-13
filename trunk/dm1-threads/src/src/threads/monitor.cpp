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
/* Created on 9th August 2002 */
/* 15 sep 2002: Added support for counted locks so that a Monitor may be locked more
 *              than once by the same thread.
 * 17 Sep 2002: Introduced readyQ - code simplified as a result. Renamed from Condition to
 *              Monitor.
 * 29 sep 2002: Introduced Exceptions.
 * 03 feb 2004: It is a FATAL ERROR if a Monitor is destroyed while it is active.
 * 08 feb 2004: Added new method tryLock().
 * 08 feb 2004: Created synonyms enter(), tryEnter() and exit().
 */

#include "dm1/monitor.h"

namespace dm1 {

	int Monitor::debug = 0;

	/**
	 * Constructs a Monitor object.
	 */
	Monitor::Monitor() 
		: waitQ((int)Thread::DM1_MONITOR_WAIT_LIST),
		  readyQ((int)Thread::DM1_MONITOR_WAIT_LIST)
	{
		ENTER("Monitor::ctor");

		if (debug)
			fprintf(stdout, "Monitor.ctor: constructing Monitor\n");
		holder = 0;
		lockCount = 0;

		EXIT("Monitor::ctor");
	}

	/**
	 * Destroys a Monitor object. 
	 */
	Monitor::~Monitor()
	{
		ENTER("Monitor::dtor");

		if (debug)
			fprintf(stdout, "Monitor.dtor: destroying Monitor\n");

		AutoMutex mutex(&this->mutex);
		mutex.lock();

		bool busy = !readyQ.isEmpty() || !waitQ.isEmpty() || holder != 0;
		holder = 0;
		lockCount = 0;

		mutex.unlock();

		if (busy) {
			fprintf(stderr, "Monitor.dtor: FATAL ERROR: Monitor being destroyed is BUSY\n");
			abort();
		}

		EXIT("Monitor::dtor");
	}

	/**
	 * Acquires a lock on the Monitor object.
	 */
	void Monitor::lock()
	{
		ENTER("Monitor::lock");

		Thread *requester = Thread::getCurrentThread();

		if (debug)
			fprintf(stdout, "Monitor.lock: Requested lock on Monitor by thread %s\n", requester->getName());

		AutoMutex mutex(&this->mutex);
		mutex.lock();

		if (holder == 0) {
			// If no one is holding the lock, then
			// we can grant it immediately.
			if (debug)
				fprintf(stdout, "Monitor.lock: Monitor lock granted immediately to thread %s as there are no waiters\n", requester->getName());
			holder = requester;
			lockCount = 1;
		}
		else if (holder == requester) {
			// If the lock is already held by the requester,
			// then we simply increment the count.
			if (debug)
				fprintf(stdout, "Monitor.lock: Monitor lock incremented by thread %s\n", holder->getName());
			lockCount++;
		}
		else {
			// We need to wait until the lock is released and can be granted to us.
			// Note: there may be other threads ahead of us who will get the lock before we do.
			if (debug)
				fprintf(stdout, "Monitor.lock: Monitor locked by %s, therefore thread %s must wait\n", holder->getName(), requester->getName());
			requester->isSignaled = false;
			readyQ.append(requester);
			mutex.unlock();
			requester->wait();
			mutex.lock();
			if (debug) {
				assert(holder == requester);
				assert(lockCount == 1);
				fprintf(stdout, "Monitor.lock: Monitor lock granted to thread %s\n", requester->getName());
			}
		}
		
		mutex.unlock();
		EXIT("Monitor::lock");
	}

	/**
	 * Try to acquire a lock on the Monitor object.
	 * On success return true, else return false.
	 */
	bool Monitor::tryLock()
	{
		ENTER("Monitor::tryLock");

		Thread *requester = Thread::getCurrentThread();

		if (debug)
			fprintf(stdout, "Monitor.tryLock: Requested lock on Monitor by thread %s\n", requester->getName());

		AutoMutex mutex(&this->mutex);
		mutex.lock();

		bool result = true;
		if (holder == 0) {
			// If no one is holding the lock, then
			// we can grant it immediately.
			if (debug)
				fprintf(stdout, "Monitor.tryLock: Monitor lock granted to thread %s as there are no waiters\n", requester->getName());
			holder = requester;
			lockCount = 1;
		}
		else if (holder == requester) {
			// If the lock is already held by the requester,
			// then we simply increment the count.
			if (debug)
				fprintf(stdout, "Monitor.tryLock: Monitor lock incremented by thread %s\n", holder->getName());
			lockCount++;
		}
		else {
			// We need to wait until the lock is released and can be granted to us.
			// Note: there may be other threads ahead of us who will get the lock before we do.
			if (debug)
				fprintf(stdout, "Monitor.tryLock: Monitor currently locked by %s, thread %s cannot be granted lock\n", holder->getName(), requester->getName());
			result = false;
		}
		
		mutex.unlock();
		EXIT("Monitor::tryLock");
		
		return result;
	}


	/**
	 * Unlocks a Monitor object.
	 */
	void Monitor::unlock()
	{
		ENTER("Monitor::unlock");

		Thread *requester = Thread::getCurrentThread();

		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		if (holder != requester) {
			if (holder == 0) 
				fprintf(stderr, "Monitor.unlock: Error: unlock requested by %s, but lock is not held\n",
					requester->getName());
			else 
				fprintf(stderr, "Monitor.unlock: Error: unlock requested by %s, but lock is held by %s\n",
					requester->getName(), holder->getName());
			mutex.unlock();
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_MONITOR_STATE);
		}

		lockCount--;
		if (lockCount == 0) {
			// We grant the lock to the next waiting thread.
			if (debug)
				fprintf(stdout, "Monitor.unlock: Monitor unlocked by thread %s\n", holder->getName());
			holder = 0;
			grantMonitor();
		}
		else {
			if (debug)
				fprintf(stdout, "Monitor.unlock: Monitor lock decremented by thread %s\n", holder->getName());
		}
		
		mutex.unlock();
		EXIT("Monitor::unlock");
	}

	/**
	 * Grants the Monitor lock to the first thread in the ReadyQ.
	 * It also wakes up the thread that has been granted the lock.
	 */
	void Monitor::grantMonitor()
	{
		ENTER("Monitor::grantMonitor");

		assert(holder == 0);

		holder = (Thread *)readyQ.getFirst();
		if (holder != 0) {
			if (debug)
				fprintf(stdout, "Monitor.grantMonitor: Monitor granted to thread %s\n", holder->getName());
			lockCount = 1;
			readyQ.remove(holder);
			Thread::notify(holder);
		}
		
		EXIT("Monitor::grantMonitor");
	}

	/**
	 * Waits for the Monitor to be signalled. Unlocks the Monitor before
	 * going to sleep, and reacquires the lock after waking up.
	 */
	void Monitor::wait()
	{
		ENTER("Monitor::wait");

		Thread *waiter = Thread::getCurrentThread();

		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		if (holder != waiter) {
			if (holder == 0) 
				fprintf(stderr, "Monitor.wait: Error: wait requested by %s, but lock is not held\n",
					waiter->getName());
			else 
				fprintf(stderr, "Monitor.wait: Error: wait requested by %s, but lock is held by %s\n",
					waiter->getName(), holder->getName());
			mutex.unlock();
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_MONITOR_STATE);
		}
		if (debug) {
			fprintf(stdout, "Monitor.wait: Thread %s about to wait on Monitor\n", waiter->getName());
		}

		waiter->isSignaled = false;
		waitQ.append(waiter);
		int savedCount = lockCount;
		holder = 0;
		lockCount = 0;
		grantMonitor();
		mutex.unlock();
		waiter->wait();
		mutex.lock();
		lockCount = savedCount;
		if (debug) {
			assert(holder == waiter);
			assert(waiter->isSignaled);
			fprintf(stdout, "Monitor.wait: Thread %s finished wait on Monitor\n", waiter->getName());
		}
		mutex.unlock();

		EXIT("Monitor::wait");
	}

	/**
	 * Similar to wait but has a timeout associated.
	 */
	bool Monitor::wait(unsigned secs)
	{
		ENTER("Monitor::wait");
		
		int rc = 0;
		bool result;

		Thread *waiter = Thread::getCurrentThread();

		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		if (holder != waiter) {
			if (holder == 0) 
				fprintf(stderr, "Monitor.wait: Error: wait requested by %s, but lock is not held\n",
					waiter->getName());
			else 
				fprintf(stderr, "Monitor.wait: Error: wait requested by %s, but lock is held by %s\n",
					waiter->getName(), holder->getName());
			mutex.unlock();
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_MONITOR_STATE);
		}
		if (debug) {
			fprintf(stdout, "Monitor.wait: Thread %s about to wait on Monitor\n", waiter->getName());
		}
		waiter->isSignaled = false;
		waitQ.append(waiter);
		int savedCount = lockCount;
		holder = 0;
		lockCount = 0;
		grantMonitor();
		mutex.unlock();
		rc = waiter->wait(secs);
		mutex.lock();

		if (holder != waiter) {
			assert(rc == ETIMEDOUT);
			assert(! waiter->isSignaled);

			result = false;
			if (holder == 0) {
				waitQ.remove(waiter);
				holder = waiter;
			}
			else {
				waitQ.remove(waiter);
				readyQ.append(waiter);
				mutex.unlock();
				waiter->wait();
				mutex.lock();
				assert(holder == waiter);
			}
		}
		else {
			result = true;
		}
		lockCount = savedCount;
		if (debug)
			fprintf(stdout, "Monitor.wait: Thread %s finished timed wait on Monitor - result = %d\n", waiter->getName(), (int)result);
		
		mutex.unlock();
		EXIT("Monitor::wait");
		return result;
	}

	/**
	 * Sets one waiting thread to signalled state. 
	 */
	void Monitor::notify()
	{
		ENTER("Monitor::notify");
		
		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		Thread *waiter = (Thread *)waitQ.getFirst();
		if (waiter != 0) {
			if (debug)
				fprintf(stdout, "Monitor.notify: Thread %s signaled on Monitor\n", waiter->getName());
			waiter->isSignaled = true;
			waitQ.remove(waiter);
			readyQ.append(waiter);
			if (holder == 0)
				grantMonitor();
		}
		
		mutex.unlock();
		EXIT("Monitor::notify");
	}

	/**
	 * Sets all waiting thread to signalled state. 
	 */
	void Monitor::notifyAll()
	{
		ENTER("Monitor::notifyAll");
		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		Thread *waiter = (Thread *)waitQ.getFirst();
		while (waiter != 0) {
			if (debug)
				fprintf(stdout, "Monitor.notifyAll: Thread %s signaled on Monitor\n", waiter->getName());
			waiter->isSignaled = true;
			waitQ.remove(waiter);
			readyQ.append(waiter);
			waiter = (Thread *)waitQ.getFirst();
		}
		if (!readyQ.isEmpty() && holder == 0)
			grantMonitor();
		
		mutex.unlock();
		EXIT("Monitor::notifyAll");
	}

	/** 
	 * Dumps the contents of the Monitor object.
	 */
	void Monitor::dump(FILE *fp)
	{
		ENTER("Monitor::dump");

		AutoMutex mutex(&this->mutex);
		mutex.lock();
		
		if (holder != 0) {
			fprintf(fp, "Holder = %s, LockCount = %d\n", holder->getName(), lockCount);
		}
		else {
			fprintf(fp, "Holder = null\n");
		}
		Thread *waiter = (Thread *)readyQ.getFirst();
		fprintf(fp, "readyQ = {\n");
		while (waiter != 0) {
			fprintf(fp, "\t%s\n", waiter->getName());
			waiter = waiter->getNextMonitorWaiter();
		}
		fprintf(fp, "}\n");
		waiter = (Thread *)waitQ.getFirst();
		fprintf(fp, "waitQ = {\n");
		while (waiter != 0) {
			fprintf(fp, "\t%s\n", waiter->getName());
			waiter = waiter->getNextMonitorWaiter();
		}
		fprintf(fp, "}\n");

		mutex.unlock();
		EXIT("Monitor::dump");
	}
}	

