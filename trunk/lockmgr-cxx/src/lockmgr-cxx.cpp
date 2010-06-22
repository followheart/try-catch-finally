//============================================================================
// Name        : lockmgr-cxx.cpp
// Author      : dibyendu majumdar
// Version     :
// Copyright   : copyright 2010
// Description : Lock Manager in C++
//============================================================================

#include <iostream>
#include <pthread.h>
#include <ctime>
#include <cerrno>
#include <cassert>

#include "tbb/compat/thread"
#include "tbb/spin_mutex.h"
#include "tbb/spin_rw_mutex.h"
#include "tbb/tick_count.h"

// undefine following to use TBB mutexes
#define USE_PTHREAD_MUTEX

using namespace std;

namespace ks2 {

enum LockDuration {
	INSTANT_DURATION, MANUAL_DURATION, COMMIT_DURATION
};

enum LockMode {
	NONE = 0,
	INTENTION_SHARED = 1,
	INTENTION_EXCLUSIVE = 2,
	SHARED = 3,
	SHARED_INTENTION_EXCLUSIVE = 4,
	UPDATE = 5,
	EXCLUSIVE = 6
};

static bool LockCompatibilityMatrix[] = { true, true, true, true, true, true,
		true, true, true, true, true, true, false, false, true, true, true,
		false, false, false, false, true, true, false, true, false, false,
		false, true, true, false, false, false, false, false, true, false,
		false, true, false, false, false, true, false, false, false, false,
		false, false };

static inline bool isCompatible(int req, int cur) {
	return LockCompatibilityMatrix[req * 7 + cur];
}

static LockMode conversionMatrix[] = { NONE, INTENTION_SHARED,
		INTENTION_EXCLUSIVE, SHARED, SHARED_INTENTION_EXCLUSIVE, UPDATE,
		EXCLUSIVE, INTENTION_SHARED, INTENTION_SHARED, INTENTION_EXCLUSIVE,
		SHARED, SHARED_INTENTION_EXCLUSIVE, UPDATE, EXCLUSIVE,
		INTENTION_EXCLUSIVE, INTENTION_EXCLUSIVE, INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE, EXCLUSIVE,
		EXCLUSIVE, SHARED, SHARED, SHARED_INTENTION_EXCLUSIVE, SHARED,
		SHARED_INTENTION_EXCLUSIVE, UPDATE, EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE, EXCLUSIVE,
		UPDATE, UPDATE, EXCLUSIVE, UPDATE, SHARED_INTENTION_EXCLUSIVE, UPDATE,
		EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE,
		EXCLUSIVE, EXCLUSIVE };

static inline LockMode maximumOf(int newmode, int oldmode) {
	return conversionMatrix[newmode * 7 + oldmode];
}

class Lockable {
public:
	virtual bool equals(const Lockable* l) = 0;
	virtual unsigned int hash() = 0;
};

class LockOwner {
public:
	virtual bool equals(const LockOwner* l) = 0;
	virtual unsigned int hash() = 0;
};

class LockManager {
public:
	virtual bool acquire(LockOwner *owner, Lockable *lockable, LockMode mode,
			LockDuration duration, int timeout) = 0;
	virtual bool downgrade(LockOwner *owner, Lockable *lockable,
			LockMode downgradeTo) = 0;
	virtual bool release(LockOwner *owner, Lockable *lockable, bool force) = 0;
	virtual LockMode findLock(LockOwner *owner, Lockable *lockable) = 0;
};

enum LockStatus {
	GRANTED, GRANTABLE, TIMEOUT, DEADLOCK, ERROR, DOWAIT, DOWAITCONVERT
};

class Event {
public:
	virtual void reset() = 0;
	virtual void wait() = 0;
	virtual int wait(unsigned secs) = 0;
	virtual void notify() = 0;
};

#ifdef USE_PTHREAD_MUTEX

class mutex {
	pthread_mutex_t m;
public:
	mutex() {
		pthread_mutex_init(&m, 0);
	}
	~mutex() {
		pthread_mutex_destroy(&m);
	}
	void lock() {
		pthread_mutex_lock(&m);
	}
	void unlock() {
		pthread_mutex_unlock(&m);
	}
};

class rw_mutex {
	pthread_rwlock_t m;
public:
	rw_mutex() {
		pthread_rwlock_init(&m, 0);
	}
	~rw_mutex() {
		pthread_rwlock_destroy(&m);
	}
	void lock_read() {
		pthread_rwlock_rdlock(&m);
	}
	void lock() {
		pthread_rwlock_wrlock(&m);
	}
	bool try_lock() {
		if (pthread_rwlock_trywrlock(&m) == EBUSY) {
			return false;
		}
		return true;
	}
	void unlock() {
		pthread_rwlock_unlock(&m);
	}
};

#else

typedef tbb::spin_mutex mutex;
typedef tbb::spin_rw_mutex rw_mutex;

#endif

Event *NewEvent();

class EventImpl: public Event {

private:
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool signaled;
	unsigned waitersCount;
private:
	EventImpl(const EventImpl& other);
	EventImpl& operator=(const EventImpl& other);
public:
	EventImpl *next;
public:
	EventImpl();
	~EventImpl();
	void reset();
	void wait();
	int wait(unsigned secs);
	void notify();
};

static EventImpl* ev_head = 0;
static mutex ev_lock;

Event *NewEvent() {
	EventImpl *e = 0;
	ev_lock.lock();
	if (ev_head != 0) {
		e = ev_head;
		ev_head = e->next;
		e->next = 0;
	}
	ev_lock.unlock();
	if (e == 0) {
		e = new EventImpl();
	}
	return e;
}

void FreeEvent(Event *e) {
	e->reset();
	EventImpl *impl = static_cast<EventImpl*> (e);
	ev_lock.lock();
	impl->next = ev_head;
	ev_head = impl;
	ev_lock.unlock();
}

EventImpl::EventImpl() {
	signaled = false;
	waitersCount = 0;
	next = 0;
	pthread_mutex_init(&mutex, 0);
	pthread_cond_init(&cond, 0);
}

EventImpl::~EventImpl() {
	int rc;
	while ((rc = pthread_cond_destroy(&cond)) == EBUSY)
		notify();
	pthread_mutex_destroy(&mutex);
}

void EventImpl::reset() {
	pthread_mutex_lock(&mutex);
	signaled = false;
	pthread_mutex_unlock(&mutex);
}

void EventImpl::wait() {
	int rc = 0;
	pthread_mutex_lock(&mutex);
	if (signaled) {
		signaled = false;
	} else {
		waitersCount++;
		rc = pthread_cond_wait(&cond, &mutex);
		waitersCount--;
	}
	pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n",
				rc);
		abort();
	}
}

/*
 * Wait for a event to be signalled.
 */
int EventImpl::wait(unsigned secs) {
	struct timespec ts;
	int rc = 0;

	pthread_mutex_lock(&mutex);
	if (signaled) {
		signaled = false;
	} else {
		ts.tv_sec = time(0) + secs; /* Is this portable ? */
		ts.tv_nsec = 0;
		waitersCount++;
		rc = pthread_cond_timedwait(&cond, &mutex, &ts);
		waitersCount--;
	}
	pthread_mutex_unlock(&mutex);
	if (rc != 0 && rc != ETIMEDOUT) {
		fprintf(stderr, "Event.wait: Failed to wait on Event: errcode = %d\n",
				rc);
		abort();
	}
	return rc;
}

/**
 * Signal a event variable, waking up any waiting thread(s).
 */
void EventImpl::notify() {
	int rc = 0;
	pthread_mutex_lock(&mutex);
	if (waitersCount == 0) {
		signaled = true;
	} else {
		rc = pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		fprintf(stderr, "Event.notify: Failed to notify Event: errcode = %d\n",
				rc);
		abort();
	}
}

enum LockRequestStatus {
	REQ_GRANTED, REQ_CONVERTING, REQ_WAITING, REQ_DENIED
};

class LockItem;
class LockRequest {
public:
	LockRequestStatus status;
	LockMode mode;
	LockMode convertMode;
	LockDuration convertDuration;
	int count;
	LockDuration duration;
	LockOwner *owner;
	LockItem *lockItem;
	Event *event;
	LockRequest *next;
	LockRequest *prev;

	void init() {
		status = REQ_WAITING;
		mode = convertMode = NONE;
		duration = convertDuration = MANUAL_DURATION;
		count = 0;
		owner = 0;
		lockItem = 0;
		next = 0;
		prev = 0;
	}

	LockRequest() {
		event = NewEvent();
		init();
	}

	void signal() {
		assert(event != 0);
		event->notify();
	}

	void waitForSignal(int timeout) {
		assert(event != 0);
		if (timeout < 0)
			event->wait();
		else
			event->wait(timeout);
	}
};

class LockBucket {
public:
	LockItem *head;
	LockItem *tail;
	LockItem *free;
	LockRequest * freereq;
	mutex m;

	LockBucket();
	LockItem *newLockItem();
	LockRequest *newLockRequest(LockItem *l, LockOwner *o, LockMode m,
			LockDuration d, LockRequestStatus status, int ref_count);
	LockRequest *newLockRequest();
	void lock();
	void unlock();
	void addLast(LockItem *l);
	void remove(LockItem *l);
	void recycle(LockRequest *r);
	LockItem *findLock(Lockable *lockable);
	LockItem *newLockItem(Lockable *lockable, LockMode m);
};

class LockItem {
public:
	Lockable *target;
	LockMode grantedMode;
	bool waiting;
	LockItem *next;
	LockItem *prev;
	LockRequest *head;
	LockRequest *tail;

	void init();
	LockItem();
	void addLast(LockRequest *r);
	void remove(LockRequest *r, LockBucket *b);
	LockRequest *find(LockOwner *owner);
	bool checkCompatible(LockRequest *request, LockMode mode);
	LockRequest
	*handleNewRequest(LockBucket *b, LockOwner *o, Lockable *lockable,
			LockMode m, LockDuration d, int timeout, bool& timedout);
	LockStatus handleConversionRequest(LockRequest *r, LockMode m,
			LockDuration d, int timeout);
	void prepareToWait(LockRequest *r, LockMode m, LockDuration d,
			bool converting);
	void grantWaiters();
};

void LockItem::init() {
	target = 0;
	grantedMode = NONE;
	head = tail = 0;
	next = prev = 0;
	waiting = false;
}

LockItem::LockItem() {
	init();
}

void LockItem::addLast(LockRequest *r) {
	r->prev = tail;
	r->next = 0;
	if (head == 0) {
		head = r;
	}
	if (tail != 0) {
		tail->next = r;
	}
	tail = r;
}

void LockItem::remove(LockRequest *r, LockBucket *b) {
	LockRequest *next = r->next;
	LockRequest *prev = r->prev;
	if (next != 0) {
		next->prev = prev;
	} else {
		tail = prev;
	}
	if (prev != 0) {
		prev->next = next;
	} else {
		head = next;
	}
	b->recycle(r);
}

LockRequest *LockItem::find(LockOwner *owner) {
	for (LockRequest *r = head; r != 0; r = r->next) {
		if (r->owner == owner || r->owner->equals(owner)) {
			return r;
		}
	}
	return 0;
}

bool LockItem::checkCompatible(LockRequest *request, LockMode mode) {
	bool iscompatible = true;
	/* Check if there are other holders */
	for (LockRequest *other = head; other != 0; other = other->next) {
		if (other == request)
			continue;
		else if (other->status == REQ_WAITING)
			break;
		else {
			if (!isCompatible(mode, other->mode)) {
				iscompatible = false;
				break;
			}
		}
	}
	return iscompatible;
}

LockRequest *LockItem::handleNewRequest(LockBucket *b, LockOwner *o,
		Lockable *lockable, LockMode m, LockDuration d, int timeout,
		bool& timedout) {
	bool can_grant = (!waiting && isCompatible(m, grantedMode));
	if (d == INSTANT_DURATION && can_grant) {
		return 0;
	} else if (!can_grant && timeout == 0) {
		timedout = true;
		return 0;
	}
	LockRequest *r = b->newLockRequest(this, o, m, d, REQ_WAITING, 0);
	addLast(r);
	if (can_grant) {
		r->status = REQ_GRANTED;
		r->count = 1;
		grantedMode = maximumOf(m, grantedMode);
	}
	return r;
}

LockStatus LockItem::handleConversionRequest(LockRequest *r, LockMode m,
		LockDuration d, int timeout) {
	if (r->status == REQ_CONVERTING || r->status == REQ_WAITING) {
		return ERROR;
	}
	LockStatus status = ERROR;
	if (r->status == REQ_GRANTED) {
		if (m == r->mode) {
			if (d == INSTANT_DURATION) {
				status = GRANTABLE;
			} else {
				r->count++;
				if (r->duration == MANUAL_DURATION && d == COMMIT_DURATION) {
					r->duration = COMMIT_DURATION;
				}
				status = GRANTED;
			}
		} else {
			bool can_grant = checkCompatible(r, m);
			if (can_grant) {
				if (d != INSTANT_DURATION) {
					r->mode = maximumOf(m, r->mode);
					r->count++;
					if (r->duration == MANUAL_DURATION && d == COMMIT_DURATION) {
						r->duration = COMMIT_DURATION;
					}
					grantedMode = maximumOf(r->mode, grantedMode);
					status = GRANTED;
				} else {
					status = GRANTABLE;
				}
			} else if (!can_grant && timeout == 0) {
				status = TIMEOUT;
			} else {
				status = DOWAITCONVERT;
			}
		}
	}
	return status;
}

void LockItem::prepareToWait(LockRequest *r, LockMode m, LockDuration d,
		bool converting) {
	waiting = true;
	if (!converting) {
		r->status = REQ_WAITING;
	} else {
		r->convertMode = m;
		r->convertDuration = d;
		r->status = REQ_CONVERTING;
	}
}

void LockItem::grantWaiters() {
	grantedMode = NONE;
	waiting = false;
	bool converting = false;
	for (LockRequest *r = head; r != 0; r = r->next) {
		if (r->status == REQ_GRANTED) {
			grantedMode = maximumOf(r->mode, grantedMode);
		} else if (r->status == REQ_CONVERTING) {
			bool can_grant = checkCompatible(r, r->convertMode);
			if (can_grant) {
				if (r->convertDuration == INSTANT_DURATION) {
					r->convertMode = r->mode;
				} else {
					r->mode = maximumOf(r->convertMode, r->mode);
					r->convertMode = r->mode;
					if (r->convertDuration == COMMIT_DURATION && r->duration
							== MANUAL_DURATION) {
						r->duration = COMMIT_DURATION;
					}
					grantedMode = maximumOf(r->mode, grantedMode);
				}
				r->count++;
				r->status = REQ_GRANTED;
				r->signal();
			} else {
				grantedMode = maximumOf(r->mode, grantedMode);
				converting = true;
				waiting = true;
			}
		} else if (r->status == REQ_WAITING) {
			if (!converting && isCompatible(r->mode, grantedMode)) {
				r->status = REQ_GRANTED;
				r->count = 1;
				grantedMode = maximumOf(r->mode, grantedMode);
				r->signal();
			} else {
				waiting = true;
				break;
			}
		}
	}
}

LockBucket::LockBucket() {
	head = 0;
	tail = 0;
	free = 0;
	freereq = 0;
}

void LockBucket::lock() {
	m.lock();
}

void LockBucket::unlock() {
	m.unlock();
}

LockItem *LockBucket::newLockItem() {
	if (free != 0) {
		LockItem *l = free;
		free = free->next;
		l->next = 0;
		return l;
	}
	//	fmt.Print("alloc item\n")
	return new LockItem();
}

LockRequest *LockBucket::newLockRequest(LockItem *l, LockOwner *o, LockMode m,
		LockDuration d, LockRequestStatus status, int ref_count) {
	LockRequest *r = newLockRequest();
	r->lockItem = l;
	r->duration = r->convertDuration = d;
	r->mode = r->convertMode = m;
	r->status = status;
	r->count = ref_count;
	r->owner = o;
	r->next = 0;
	r->prev = 0;
	return r;
}

LockRequest *LockBucket::newLockRequest() {
	if (freereq != 0) {
		LockRequest *r = freereq;
		freereq = freereq->next;
		return r;
	}
	//	fmt.Print("alloc req\n")
	return new LockRequest();
}

void LockBucket::addLast(LockItem *l) {
	l->prev = tail;
	l->next = 0;
	if (head == 0) {
		head = l;
	}
	if (tail != 0) {
		tail->next = l;
	}
	tail = l;
}

void LockBucket::remove(LockItem *l) {
	LockItem *next = l->next;
	LockItem *prev = l->prev;
	if (next != 0) {
		next->prev = prev;
	} else {
		tail = prev;
	}
	if (prev != 0) {
		prev->next = next;
	} else {
		head = next;
	}
	l->init();
	l->next = free;
	free = l;
}

void LockBucket::recycle(LockRequest *r) {
	r->init();
	r->next = freereq;
	freereq = r;
	// 	fmt.Printf("free req\n");
}

LockItem *LockBucket::findLock(Lockable *lockable) {
	for (LockItem *l = head; l != 0; l = l->next) {
		if (l->target == lockable || l->target->equals(lockable)) {
			return l;
		}
	}
	return 0;
}

LockItem *LockBucket::newLockItem(Lockable *lockable, LockMode m) {
	LockItem *item = newLockItem();
	item->target = lockable;
	item->grantedMode = m;
	return item;
}

static int primes[] = { 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
		49157, 98317, 196613, 393241, 786433 };

class LockManagerImpl: public LockManager {
public:
	enum ReleaseAction {
		RELEASE, DOWNGRADE
	};
	int htsz;
	volatile int count;
	volatile int threshold;
	float loadFactor;
	LockBucket* locktable;
	int N;
	rw_mutex globalLock;

	LockManagerImpl();
	void rehash();
	LockBucket *h(Lockable *lockable);
	LockMode findLock(LockOwner *owner, Lockable *lockable);
	bool acquire(LockOwner *owner, Lockable *lockable, LockMode mode,
			LockDuration duration, int timeout);
	LockStatus doAcquire(LockOwner *o, Lockable *lockable, LockMode m,
			LockDuration d, int timeout);
	LockStatus checkStatus(LockDuration d, LockRequest *r);
	LockStatus handleNewLock(LockBucket *b, LockOwner *o, Lockable *lockable,
			LockMode m, LockDuration d);
	LockStatus handleWaitResult(LockBucket *b, LockItem *l, LockRequest *r,
			bool converting);
	bool release(LockOwner *o, Lockable *lockable, bool force);
	bool downgrade(LockOwner *o, Lockable *lockable, LockMode downgradeTo);
	bool doReleaseInternal(ReleaseAction a, LockOwner *o, Lockable *lockable,
			LockMode downgradeMode, bool force);
	bool releaseLock(ReleaseAction a, LockOwner *o, Lockable *lockable,
			LockMode downgradeMode, bool force, LockBucket *b, LockItem *l);
};

LockManagerImpl::LockManagerImpl() {
	loadFactor = 0.75f;
	htsz = 0;
	count = 0;
	N = primes[htsz];
	locktable = new LockBucket[N];
	threshold = (int) (N * loadFactor);
}

void LockManagerImpl::rehash() {
	if (htsz == sizeof primes / sizeof(int) - 1) {
		return;
	}
	if (!globalLock.try_lock()) {
		return;
	}
	if (htsz == sizeof primes / sizeof(int) - 1) {
		globalLock.unlock();
		return;
	}
	int newHashTableSize = primes[++htsz];
	LockBucket* newLockHashTable = new LockBucket[newHashTableSize];
	for (int i = 0; i < N; i++) {
		LockBucket *bucket = &locktable[i];
		for (LockItem *item = bucket->head; item != 0; item = item->next) {
			int h = (item->target->hash() & 0x7FFFFFFF) % newHashTableSize;
			LockBucket *newBucket = &newLockHashTable[h];
			newBucket->addLast(item);
		}
	}
	LockBucket* oldLockHashTable = locktable;
	locktable = newLockHashTable;
	N = newHashTableSize;
	threshold = (int) (N * loadFactor);
	// TODO free up all the lockitems and lockrequests
	delete oldLockHashTable;
	globalLock.unlock();
}

LockBucket *LockManagerImpl::h(Lockable *lockable) {
	int h = (lockable->hash() & 0x7FFFFFFF) % N;
	return &locktable[h];
}

LockMode LockManagerImpl::findLock(LockOwner *owner, Lockable *lockable) {
	LockMode m = NONE;
	globalLock.lock_read();
	LockBucket *b = h(lockable);
	b->lock();
	LockItem *lockItem = b->findLock(lockable);
	if (lockItem != 0) {
		LockRequest *r = lockItem->find(owner);
		if (r != 0) {
			m = r->mode;
		}
	}
	b->unlock();
	globalLock.unlock();
	return m;
}

bool LockManagerImpl::acquire(LockOwner *owner, Lockable *lockable,
		LockMode mode, LockDuration duration, int timeout) {

	if (count > threshold) {
		rehash();
	}

	globalLock.lock_read();
	LockStatus status = ERROR;
	status = doAcquire(owner, lockable, mode, duration, timeout);
	if (duration == INSTANT_DURATION && status == GRANTED) {
		/*
		 * Handle the case where the lock was granted after a wait.
		 */
		doReleaseInternal(RELEASE, owner, lockable, NONE, false);
	}
	globalLock.unlock();
	return status == GRANTED || status == GRANTABLE;
}

LockStatus LockManagerImpl::doAcquire(LockOwner *o, Lockable *lockable,
		LockMode m, LockDuration d, int timeout) {
	LockItem *l = 0;
	LockRequest *r = 0;
	bool converting = false;
	LockStatus status = ERROR;

	LockBucket *b = h(lockable);
	b->lock();
	do {
		l = b->findLock(lockable);
		if (l == 0) {
			status = handleNewLock(b, o, lockable, m, d);
			break;
		}
		r = l->find(o);
		if (r == 0) {
			bool timedout = false;
			r = l->handleNewRequest(b, o, lockable, m, d, timeout, timedout);
			if (timedout) {
				status = TIMEOUT;
			} else {
				status = checkStatus(d, r);
			}
		} else {
			status = l->handleConversionRequest(r, m, d, timeout);
		}
		if (status == GRANTED || status == GRANTABLE) {
			break;
		}
		converting = status == DOWAITCONVERT;
		l->prepareToWait(r, m, d, converting);
	} while (false);
	b->unlock();
	if (status != DOWAIT and status != DOWAITCONVERT) {
		return status;
	}
	for (;;) {
		globalLock.unlock();
		r->waitForSignal(-1);
		globalLock.lock_read();
		b = h(lockable);
		b->lock();
		if (r->status == REQ_WAITING || r->status == REQ_CONVERTING) {
			b->unlock();
			continue;
		}
		status = handleWaitResult(b, l, r, converting);
		b->unlock();
		break;
	}
	return status;
}

LockStatus LockManagerImpl::checkStatus(LockDuration d, LockRequest *r) {
	LockStatus status;
	if (r == 0) {
		status = GRANTABLE;
	} else {
		if (r->status == REQ_WAITING) {
			status = DOWAIT;
		} else {
			status = GRANTED;
		}
	}
	return status;
}

LockStatus LockManagerImpl::handleNewLock(LockBucket *b, LockOwner *o,
		Lockable *lockable, LockMode m, LockDuration d) {
	if (d != INSTANT_DURATION) {
		LockItem *l = b->newLockItem(lockable, m);
		LockRequest *r = b->newLockRequest(l, o, m, d, REQ_GRANTED, 1);
		l->addLast(r);
		b->addLast(l);
		count++;
		return GRANTED;
	}
	return GRANTABLE;
}

LockStatus LockManagerImpl::handleWaitResult(LockBucket *b, LockItem *l,
		LockRequest *r, bool converting) {
	LockStatus status;
	if (r->status == REQ_GRANTED) {
		status = GRANTED;
	} else if (r->status == REQ_DENIED) {
		status = DEADLOCK;
	} else {
		status = TIMEOUT;
	}
	if (status == GRANTED) {
		return status;
	}
	if (!converting) {
		l->remove(r, b);
		if (l->head == 0) {
			b->remove(l);
			count--;
		}
	} else {
		r->status = REQ_GRANTED;
		r->convertMode = r->mode;
	}
	if (status == DEADLOCK) {
		l->grantWaiters();
	}
	return status;
}

bool LockManagerImpl::release(LockOwner *o, Lockable *lockable, bool force) {
	bool ok = false;
	globalLock.lock();
	ok = doReleaseInternal(RELEASE, o, lockable, NONE, force);
	globalLock.unlock();
	return ok;
}

bool LockManagerImpl::downgrade(LockOwner *o, Lockable *lockable,
		LockMode downgradeTo) {
	bool ok;
	globalLock.lock();
	ok = doReleaseInternal(DOWNGRADE, o, lockable, downgradeTo, false);
	globalLock.unlock();
	return ok;
}

bool LockManagerImpl::doReleaseInternal(ReleaseAction a, LockOwner *o,
		Lockable *lockable, LockMode downgradeMode, bool force) {
	LockBucket *b = h(lockable);
	bool ok = false;
	b->lock();
	do {
		LockItem *l = b->findLock(lockable);
		if (l == 0) {
			ok = true;
			break;
		}
		ok = releaseLock(a, o, lockable, downgradeMode, force, b, l);
	} while (false);
	b->unlock();
	return ok;
}

bool LockManagerImpl::releaseLock(ReleaseAction a, LockOwner *o,
		Lockable *lockable, LockMode downgradeMode, bool force, LockBucket *b,
		LockItem *l) {

	LockRequest *r = l->find(o);
	if (r == 0) {
		return true;
	}
	if (r->status == REQ_CONVERTING || r->status == REQ_WAITING) {
		return false;
	}
	if (a == RELEASE) {
		if (!force) {
			if (r->duration == COMMIT_DURATION) {
				return false;
			}
			r->count--;
			if (r->count > 0) {
				return false;
			}
		}
		l->remove(r, b);
		if (l->head == 0) {
			b->remove(l);
			count--;
			return true;
		}
	} else /* if a == DOWNGRADE */{
		if (r->mode == downgradeMode) {
			return true;
		}
		LockMode mode = maximumOf(downgradeMode, r->mode);
		if (mode == r->mode) {
			r->mode = downgradeMode;
			r->convertMode = downgradeMode;
		} else {
			return false;
		}
	}
	l->grantWaiters();
	return true;
}

} // namespace ks2

class MyOwner: public ks2::LockOwner {
	unsigned int id;
public:
	MyOwner(unsigned int id) {
		this->id = id;
	}
	bool equals(const LockOwner *other) {
		return id == static_cast<const MyOwner *> (other)->id;
	}
	unsigned int hash() {
		return id;
	}
};

class MyLock: public ks2::Lockable {
	unsigned int id;
public:
	MyLock(unsigned int id) {
		this->id = id;
	}
	bool equals(const Lockable *other) {
		return id == static_cast<const MyLock *> (other)->id;
	}
	unsigned int hash() {
		return id;
	}
};

struct T {
	ks2::LockManager *lm;
	MyOwner *o;
	MyLock *lock;
	int N;

	void operator()() {
		for (int i = 0; i < N; i++) {
			bool ok = lm->acquire(o, lock, ks2::EXCLUSIVE,
					ks2::MANUAL_DURATION, -1);
			assert(ok);
			ok = lm->release(o, lock, false);
			assert(ok);
		}
	}
};

int main(int argc, const char *argv[]) {

	int n_threads = 3;
	if (argc == 2) {
		char c = argv[1][0];
		n_threads = c - '0';
		n_threads = (n_threads < 0 || n_threads > 3) ? 3 : n_threads;
	}

	ks2::LockManager *lm = new ks2::LockManagerImpl();
	MyOwner o1(1);
	MyOwner o2(2);
	MyOwner o3(3);
	MyLock lock(10);

	int N = (n_threads == 1) ? 500000 : 100000;

	T f1 = { lm, &o1, &lock, N };
	T f2 = { lm, &o2, &lock, N };
	T f3 = { lm, &o3, &lock, N };

	tbb::tick_count s1 = tbb::tick_count::now();

	if (n_threads == 1) {
		std::thread t1(f1);
		t1.join();
	} else if (n_threads == 2) {
		std::thread t1(f1);
		std::thread t2(f2);
		t1.join();
		t2.join();
	} else {
		std::thread t1(f1);
		std::thread t2(f2);
		std::thread t3(f3);
		t1.join();
		t2.join();
		t3.join();
	}

	tbb::tick_count s2 = tbb::tick_count::now();

	cerr << "threads " << n_threads << " iterations = " << N << "    " << ((s2
			- s1).seconds()) * 1E9 / N << " ns/op" << endl;

	return 0;
}

