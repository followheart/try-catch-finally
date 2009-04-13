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
#ifndef dm1_latch_h
#define dm1_latch_h

#include "dm1/common.h"
#include "dm1/except.h"
#include "dm1/list.h"
#include "dm1/mutex.h"
#include "dm1/thread.h"

namespace dm1 {

	class Latch {

	public:
		enum LatchMode {
			DM1_LATCH_FREE = 0,
			DM1_LATCH_S = 1,
			DM1_LATCH_X = 2,
			DM1_LATCH_GRANTED = 3
		};

		enum LatchWait {
			DM1_LATCH_WAIT = true,
			DM1_LATCH_NOWAIT = false
		};

	protected:
		Mutex mutex;
		LatchMode mode;
		int count;			/* Number of times latch held */
		Thread *holder;			/* Only exclusive holder is
						 * is remembered - to allow
						 * the same holder to acquire
						 * the latch multiple times.
						 */
		LinkList waitQ;
		const char *name;
		int latchID;

		static int debug;

	private:
		Latch(const Latch&);
		Latch& operator=(const Latch&);

	public:
		/**
		 * Create a Latch. Latches can be given
		 * names.
		 */
		explicit Latch(const char *name);

		/**
		 * Destroys the Latch.
		 */
		virtual ~Latch();
	
		/**
		 * Attempts to acquire a Latch is the mode
		 * requested. Will wait if wait mode is 
		 * set to DM1_LATCH_WAIT. Returns false if
		 * Latch is not available and DM1_LATCH_NOWAIT
		 * has been specified.
		 */
		bool acquire(
			LatchMode mode,
			LatchWait wait);

		/**
		 * Releases the latch. If the Latch was
		 * acquired more than once, it must be released
		 * an equal number of times.
		 */
		void release();

		static void reportStatistics(FILE *fp = stderr);
		static void setDebug(int value) { debug = value; }
	};

	class AutoLatch {
		Latch *latch;
	public:
		AutoLatch(Latch *latch, Latch::LatchMode mode) { 
			this->latch = latch; 
			latch->acquire(mode, Latch::DM1_LATCH_WAIT);
		}
		~AutoLatch() { 
			latch->release(); 
		}
	};

}

#endif
