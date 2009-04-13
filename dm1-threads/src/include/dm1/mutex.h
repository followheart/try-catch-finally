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
#ifndef dm1_mutex_h
#define dm1_mutex_h

#include "dm1/common.h"
#include "dm1/port.h"
#include "dm1/except.h"

namespace dm1 {

	class Mutex {

	public:
		/**
		 * Creates a Mutex object.
		 */
		explicit Mutex();

		/**
		 * Destroys the Mutex object.
		 */
		~Mutex();

		/**
		 * Acquires the Mutex lock. Calling thread is
		 * blocked until the Mutex lock is granted.
		 * Recursive calls are not supported.
		 */
		void lock();

		/**
		 * Unlocks the Mutex object. Caller must hold the 
		 * lock before calling unlock.
		 */
		void unlock();

	public:
#if DM1_USE_PTHREAD
		pthread_mutex_t *getRealMutex();
#else
		CRITICAL_SECTION* getRealMutex();
#endif

	private:
		Mutex(const Mutex&);
		Mutex& operator=(const Mutex&);

	private:
#if DM1_USE_PTHREAD
		pthread_mutex_t mutex;
#else
		CRITICAL_SECTION mutex;
#endif
	};

	class AutoMutex {
	private:
		Mutex *m;
		bool locked;

	private:
		AutoMutex(const AutoMutex&);
		AutoMutex& operator=(const AutoMutex&);

	public:
		void lock() {
			if (! locked) {
				locked = true;
				m->lock();
			}
		}

		void unlock() {
			if (locked) {
				locked = false;
				m->unlock();
			}
		}

		explicit AutoMutex(Mutex *mutex) :  m(mutex), locked(false) 
		{}

		~AutoMutex() { unlock(); }
	};

#if !DM1_USE_PTHREAD

	inline Mutex::Mutex() {
		::InitializeCriticalSection(&mutex);
	}

	inline Mutex::~Mutex() {
		::DeleteCriticalSection(&mutex);
	}

	inline void Mutex::lock() {
		::EnterCriticalSection(&mutex);
	}
 
	inline void Mutex::unlock() {
		::LeaveCriticalSection(&mutex);
	}

	inline CRITICAL_SECTION *Mutex::getRealMutex() {
		return &mutex;
	}

#else
	inline Mutex::Mutex() {
		int rc = pthread_mutex_init(&mutex, 0);
		if (rc != 0) {
			fprintf(stderr, "Mutex.ctor: Error creating Mutex: errcode = %d\n",
				rc);
			throw Exception(__FILE__, __LINE__, DM1_ERR_MUTEX_CREATE, rc);
		}
	}

	inline void Mutex::lock() {
		int rc = pthread_mutex_lock(&mutex);
		if (rc != 0) {
			fprintf(stderr, "Mutex.lock: Error locking Mutex: errcode = %d\n",
				rc);
			throw Exception(__FILE__, __LINE__, DM1_ERR_MUTEX_LOCK, rc);
		}
	}

	inline void Mutex::unlock() {
		int rc = pthread_mutex_unlock(&mutex);
		if (rc != 0) {
			fprintf(stderr, "Mutex.ctor: Error unlocking Mutex: errcode = %d\n",
				rc);
			throw Exception(__FILE__, __LINE__, DM1_ERR_MUTEX_UNLOCK, rc);
		}
	}

	inline Mutex::~Mutex() {
		int rc = pthread_mutex_destroy(&mutex);
		if (rc != 0)
			fprintf(stderr, "Mutex.dtor: Error destroying Mutex: errcode = %d\n", rc);
	}

	inline pthread_mutex_t *Mutex::getRealMutex() {
		return &mutex;
	}
#endif

}

#endif
