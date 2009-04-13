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
 *   28 July 2002 Started port to C++.
 *   Dibyendu Majumdar
 */
#ifndef dm1_event_h
#define dm1_event_h

#include "dm1/common.h"
#include "dm1/except.h"
#include "dm1/port.h"
#include "dm1/mutex.h"

namespace dm1 {

	class Event {
		
	public:
		enum EventType {
			DM1_EVENT_SINGLE = 1,
			DM1_EVENT_BROADCAST = 2
		};

	private:

		EventType type;
#if DM1_USE_PTHREAD
		pthread_cond_t cond;
		Mutex mutex;
		bool signaled;
		unsigned waitersCount;
#else
		HANDLE event;
#endif
		static int debug;

	private:
		Event(const Event& other);
		Event& operator=(const Event& other);

	public:
		/**
		 * Creates an Event. The Default is create an Event of type Single.
		 */
		explicit Event(EventType type = DM1_EVENT_SINGLE);

		/**
		 * Destroys the Event.
		 */
		~Event();

		/**
		 * Sets the Event to Non-Signalled state.
		 */
		void reset();

		/**
		 * Unconditional wait. Waits for the Event to become Signalled. 
		 * Wait returns immediately if Event is already in Signalled state; 
		 * the Event is automatically reset to Not-Signalled state.
		 */
		void wait();

		/**
		 * Conditional wait. Waits for upto “secs” seconds for the Event
		 * to be signalled. Wait returns immediately if Event is already 
		 * in Signalled state; the Event is automatically reset to 
		 * Non-Signalled state.
		 */
		int  wait(unsigned secs);

		/**
		 * Sets an Event to Signalled state, and wakes up one waiting 
		 * thread in case of an Event of type Single, or all waiting threads 
		 * in case of an Event of type Broadcast.
		 */
		void notify();

		static void setDebug(int value) { debug = value; }

	};
}

#endif
