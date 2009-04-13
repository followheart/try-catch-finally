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
 * 060402 Ported to Win32 using PTHREADS-WIN32 library
 * 260402 Added latch statistics.
 * 280402 Adopted dm1_latch4 as the definitive version. I chose this version
 *        over dm1_latch5 because this allows me to avoid relying upon
 *        pthread_cond_broadcast(). This in turn improves portability.
 * 021002 Enhanced error checking and introduced exceptions.
 * 030204 Its a FATAL ERROR if a Latch is destroyed while it is active.
 */

#include "dm1/latch.h"

namespace dm1 {

	enum {
		DM1_LATCH_STATS_WAITS = 0,
		DM1_LATCH_STATS_NOWAITS = 1,
		DM1_LATCH_STATS_MISSES = 2
	};

	typedef struct {
		const char *name;
		unsigned long nowaits;
		unsigned long waits;
		unsigned long misses;
	} LatchStatistic;

	enum {
		DM1_LATCH_MAX_LATCHES = 500
	};

	Mutex LatchStatisticsMutex; 
	LatchStatistic LatchStatistics[DM1_LATCH_MAX_LATCHES];

	int Latch::debug = 0;

	static int
	dm1_latch_stats_init(
		const char *name);

	static void
	dm1_latch_stats_increment(
		int latch_id,
		int type);

	/**
	 * Constructs a Latch object.
	 */
	Latch::Latch(const char *name) 
		: waitQ((int)Thread::DM1_LATCH_WAIT_LIST)
	{
		mode = DM1_LATCH_FREE;
		count = 0;
		holder = 0;
		this->name = name;
		latchID = dm1_latch_stats_init(name);
	}

	bool
	Latch::acquire(
		LatchMode mode,
		LatchWait wait)
	{
		Thread *me = Thread::getCurrentThread();

		if (debug) {
			fprintf(stdout, "%s: Latch.acquire: Acquiring latch %s in %d mode\n", me->getName(), name, mode);
		}

		AutoMutex mutex(&this->mutex);
		mutex.lock();

		bool result = true;
		if (count == 0) {
			count = 1;
			holder = (mode == DM1_LATCH_X ? me : 0);
			this->mode = mode;
			dm1_latch_stats_increment(latchID, DM1_LATCH_STATS_NOWAITS);
		}
		else if (this->mode == DM1_LATCH_X && holder == me) {
			count++;
			dm1_latch_stats_increment(latchID, DM1_LATCH_STATS_NOWAITS);
		}
		else if (this->mode == DM1_LATCH_S && mode == DM1_LATCH_S && 
			waitQ.isEmpty()) {
			count++;
			dm1_latch_stats_increment(latchID, DM1_LATCH_STATS_NOWAITS);
		}
		else {
			if (wait == DM1_LATCH_NOWAIT) {
				dm1_latch_stats_increment(latchID, DM1_LATCH_STATS_MISSES);
				result = false;
			}
			else {
				dm1_latch_stats_increment(latchID, DM1_LATCH_STATS_WAITS);
				waitQ.append(me);
				me->latchMode = (int)mode;
				mutex.unlock();
				me->wait();
				mutex.lock();
				if (me->latchMode != (int)DM1_LATCH_GRANTED ||
					this->mode != mode ||
					count <= 0 || 
					this->mode == DM1_LATCH_X && holder != me) {
					mutex.unlock();
					fprintf(stderr, "Latch.acquire: Unexpected error while acquiring a Latch\n");
					throw ThreadException(__FILE__, __LINE__, DM1_ERR_LATCH_ACQUIRE);
				}
			}
		}

		if (debug) {
			fprintf(stdout, "%s: Latch.acquire: Acquire latch %s in %d mode - result = %d\n", me->getName(), name, mode, result);
		}
		mutex.unlock();

		return result;
	}

	void Latch::release()
	{
		Thread *me = Thread::getCurrentThread();
		
		if (debug) {
			fprintf(stdout, "%s: Latch.release: Releasing latch %s\n", me->getName(), name);
		}

		AutoMutex mutex(&this->mutex);
		mutex.lock();
		count--;
		assert(count >= 0);
		if (count == 0)
			holder = 0;
		if (count == 0 && !waitQ.isEmpty()) {
			Thread *waiter;
			waiter = (Thread *)waitQ.getFirst();
			if (waiter->latchMode == (int)DM1_LATCH_X) {
				waitQ.remove(waiter);
				mode = DM1_LATCH_X;
				count = 1;
				holder = waiter;
				waiter->latchMode = (int)DM1_LATCH_GRANTED;
				Thread::notify(waiter);
			}
			else {
				while (waiter != 0 && waiter->latchMode == (int)DM1_LATCH_S) {
					waitQ.remove(waiter);
					mode = DM1_LATCH_S;
					count++;
					waiter->latchMode = (int)DM1_LATCH_GRANTED;
					Thread::notify(waiter);
					waiter = (Thread *)waitQ.getFirst();
				}
			}
		}
		mutex.unlock();
	}

	Latch::~Latch() {

		AutoMutex mutex(&this->mutex);
		mutex.lock();

		bool busy = !waitQ.isEmpty() || holder != 0;
		holder = 0;

		mutex.unlock();

		if (busy) {
			fprintf(stderr, "Latch.dtor: FATAL ERROR: Latch being destroyed is busy\n");
			abort();
		}
	}

	static int
	dm1_latch_stats_init(
		const char *name)
	{
		int i;

		LatchStatisticsMutex.lock();
		for (i = 0; i < DM1_LATCH_MAX_LATCHES; i++) {
			if (LatchStatistics[i].name == 0) {
				LatchStatistics[i].name = name;
				break;
			}
			else if (strcmp(LatchStatistics[i].name, name) == 0) {
				break;
			}
		}
		LatchStatisticsMutex.unlock();
		return (i == DM1_LATCH_MAX_LATCHES ? -1: i);
	}

	static void
	dm1_latch_stats_increment(
		int latch_id,
		int type)
	{
		if (latch_id < 0)
			return;

		switch (type) {
		case DM1_LATCH_STATS_NOWAITS:
			LatchStatistics[latch_id].nowaits++; break;
		case DM1_LATCH_STATS_WAITS:
			LatchStatistics[latch_id].waits++; break;
		case DM1_LATCH_STATS_MISSES:
			LatchStatistics[latch_id].misses++; break;
		}
	}

	void 
	Latch::reportStatistics(FILE *fp)
	{
		int i;

		fprintf(fp, "Latch Statistics Report:\n");
		for (i = 0; i < DM1_LATCH_MAX_LATCHES && LatchStatistics[i].name != 0; i++) {
			fprintf(fp, "%s: nowaits = %ld, waits = %ld, misses = %ld\n",
				LatchStatistics[i].name, LatchStatistics[i].nowaits, LatchStatistics[i].waits, LatchStatistics[i].misses);
		}
	}

} /* namespace dm1 */

