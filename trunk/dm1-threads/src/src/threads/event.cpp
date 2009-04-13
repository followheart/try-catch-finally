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
 *   Created 16 August 2001
 *   170801 Added dm1_cond_t
 *   280402 Simplified condition variables to improve portability. On Windows, condition
 *          variables could be implemented as Auto-Reset Events. We do not make use
 *          of pthread_cond_broadcast because it is harder to port to Windows.
 *   190502 Implemented Mutexes as CriticalSections, and Condition variables as
 *          Events on Win32, removing dependency on Win32 pthread library.
 *   200502 Renamed conditioned variables as events because events are a sub-set of
 *          condition variables, and it is not trivial to support condition variables 
 *          in Win32.
 *   210502 Enhanced events to support broadcasting. New function dm1_event_reset() 
 *          introduced.
 *   300702 Ported to C++.
 *   021002 Initial version with Exception handling.
 *   Dibyendu Majumdar
 */

#include "dm1/event.h"

namespace dm1 {

	int Event::debug = 0;

#if DM1_USE_PTHREAD

	/*
	 * Initialize a event. An event can be of two types:
	 * DM1_EVENT_SINGLE - supports signalling and waking one waiting thread
	 * DM1_EVENT_BROADCAST - supports signalling and waking up multiple threads.
	 */
	Event::Event(EventType type)
	{
		signaled = false;
		waitersCount = 0;
		this->type = type;
		int rc = pthread_cond_init(&cond, 0);
		if (rc != 0) {
			fprintf(stderr, "Event.ctor: Error creating Event object: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_CREATE, rc);
		}
		if (debug)
			fprintf(stdout, "Event.ctor: Event created\n");
	}

	/*
	 * Reset an event to non-signalled state.
	 */
	void Event::reset() 
	{
		mutex.lock();
		signaled = false;
		mutex.unlock();
	}

	/*
	 * Wait for a event to be signalled.
	 */
	void Event::wait()
	{
		int rc = 0;
		mutex.lock();
		if (signaled) {
			signaled = false;
		}
		else {
			waitersCount++;
			rc = pthread_cond_wait(&cond, mutex.getRealMutex());
			waitersCount--;
		}
		mutex.unlock();
		if (rc != 0) {
			fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_WAIT, rc);
		}
	}

	/*
	 * Wait for a event to be signalled.
	 */
	int Event::wait(unsigned secs)
	{
		struct timespec ts;
		int rc = 0;

		mutex.lock();
		if (signaled) {
			signaled = false;
		}
		else {
			ts.tv_sec = time(0) + secs;					/* Is this portable ? */
			ts.tv_nsec = 0;
			waitersCount++;
			rc = pthread_cond_timedwait(&cond, mutex.getRealMutex(), &ts);
			waitersCount--;
		}

		mutex.unlock();
		if (rc != 0 && rc != ETIMEDOUT) {
			fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_WAIT, rc);
		}
		return rc;
	}

	/**
	 * Signal a event variable, waking up any waiting thread(s).
	 */
	void Event::notify()
	{
		int rc = 0;
		mutex.lock();
		switch (type) {

		case DM1_EVENT_SINGLE:
			if (waitersCount == 0) {
				signaled = true;
			}
			else {
				rc = pthread_cond_signal(&cond); 
			}	
			break;

		case DM1_EVENT_BROADCAST: 
			rc = pthread_cond_broadcast(&cond); 
			break;
		
		}
		mutex.unlock();
		if (rc != 0) {
			fprintf(stderr, "Event.notify: Failed to notify Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_NOTIFY, rc);
		}

	}

	/*
	 * Destroy a event.
	 */
	Event::~Event()
	{
		if (debug)
			fprintf(stdout, "Event.dtor: Event destroyed\n");
		int rc;
		while ((rc = pthread_cond_destroy(&cond)) == EBUSY)
			notify();
		if (rc != 0) {
			fprintf(stderr, "Event.dtor: Event being destroyed is BUSY\n");
		}
	}

#else	/* !DM1_USE_PTHREAD */

	Event::Event(EventType type)
	{
		if (debug)
			fprintf(stdout, "Event.ctor: Event created\n");
		enum {
			AUTORESET = FALSE,
			MANUALRESET = TRUE
		};
		this->type = type;
		switch (type) {
		case DM1_EVENT_SINGLE: event = CreateEvent(NULL, AUTORESET, FALSE, NULL); break;
		case DM1_EVENT_BROADCAST: event = CreateEvent(NULL, MANUALRESET, FALSE, NULL); break;
		default: assert(0);
		}
		if (event == INVALID_HANDLE_VALUE) {
			DWORD rc = GetLastError();
			fprintf(stderr, "Event.ctor: Failed to create an Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_CREATE, rc);
		}
	}

	void Event::reset()
	{
		if (!ResetEvent(event)) {
			DWORD rc = GetLastError();
			fprintf(stderr, "Event.reset: Failed to reset an Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_RESET, rc);
		}
	}

	void Event::wait()
	{
		if (WaitForSingleObject(event, INFINITE) == WAIT_FAILED) {
			DWORD rc = GetLastError();
			fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_WAIT, rc);
		}
	}


	int Event::wait(unsigned secs)
	{
		int rc = 0;
		DWORD millisecs;
		DWORD status;
	
		millisecs = secs*1000;
		status = WaitForSingleObject(event, millisecs);
		if (status == WAIT_TIMEOUT) {
			rc = ETIMEDOUT;
		}
		else if (status == WAIT_FAILED) {
			rc = GetLastError();
			fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_WAIT, rc);
		}
		return rc;
	}

	void Event::notify()
	{
		BOOL success = FALSE;

		switch(type) {
		case DM1_EVENT_SINGLE: success = SetEvent(event); break;
		case DM1_EVENT_BROADCAST: success = PulseEvent(event); break;
		default: assert(0);
		}
		if (!success) {
			DWORD rc = GetLastError();
			fprintf(stderr, "Event.notify: Failed to notify Event: errcode = %d\n", rc);
			throw ThreadException(__FILE__, __LINE__, DM1_ERR_EVENT_NOTIFY, rc);
		}
	}

	Event::~Event()
	{
		if (debug)
			fprintf(stdout, "Event.dtor: Event destroyed\n");
		if (!CloseHandle(event)) {
			fprintf(stderr, "Event.dtor: Failed to close Event handle: errcode = %d\n",
				GetLastError());
		}
		event = INVALID_HANDLE_VALUE;
	}

#endif
}
