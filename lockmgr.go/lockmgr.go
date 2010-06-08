package main

import (
	"fmt"
	"sync"
	"runtime"
	"time"
	"os"
)

/**
    Algorithm
    ---------

    The main algorithm for the lock manager is shown in the form of use cases.
    Some aspects are not described - such as deadlock detection

    a001 - lock acquire
    *******************

    Main Success Scenario
    .....................

    1) Search for the lock header
    2) Lock header not found
    3) Allocate new lock header
    4) Allocate new lock request
    5) Append lock request to queue with status = GRANTED and reference count of 1.
    6) Set lock granted mode to GRANTED

    Extensions
    ..........

    2. a) Lock header found but client has no prior request for the lock.
     
      1. Do `a003 - handle new request`_.

    b) Lock header found and client has a prior GRANTED lock request.
     
      1. Do `a002 - handle lock conversion`_.

    a003 - handle new request
    *************************

    Main Success Scenario
    .....................

    1) Allocate new request.
    2) Append lock request to queue with reference count of 1.
    3) Check for waiting requests.
    4) Check whether request is compatible with granted mode.
    5) There are no waiting requests and lock request is compatible with
    granted mode.
    6) Set lock's granted mode to maximum of this request and existing granted mode.
    7) Success.

    Extensions
    ..........

    5. a) There are waiting requests or lock request is not compatible with
      granted mode.
     
      1. Do `a004 - lock wait`_.

    a002 - handle lock conversion
    *****************************

    Main Success Scenario
    .....................

    1) Check lock compatibility with granted group.
    2) Lock request is compatible with granted group.
    3) Grant lock request, and update granted mode for the request.

    Extensions
    ..........

    2. a) Lock request is incompatible with granted group.
     
      1. Do `a004 - lock wait`_.


    a004 - lock wait
    ****************

    Main Success Scenario
    .....................

    1) Wait for lock.
    2) Lock granted.
    3) Success.

    Extensions
    ..........

    2. a) Lock was not granted.
     
      1. Failure!

    b001 - release lock
    *******************

    Main Success Scenario
    .....................

    1) Decrease reference count.
    2) Sole lock request and reference count is zero.
    3) Remove lock header from hash table.
    4) Success.

    Extensions
    ..........

    2. a) Reference count greater than zero.
     
      1. Success.

    2. b) Reference count is zero and there are other requests on the lock.
     
      1. Remove request from the queue.
      2. Do `b002 - grant waiters`_.

    b002 - grant waiters
    ********************

    Main Success Scenario
    .....................


    1) Get next granted lock.
    2) Recalculate granted mode.
    3) Repeat from 1) until no more granted requests.
    4) Get next waiting request.
    5) Request is compatible with granted mode.
    6) Grant request and wake up thread waiting for the lock. Increment reference count of
    the granted request and set granted mode to maximum of current mode and granted request.
    7) Repeat from 4) until no more waiting requests.

    Extensions
    ..........

    1. a) Conversion request.
     
      1. Do `b003 - grant conversion request`_.
      2. Resume from 2).

    4. a) "conversion pending" is set (via b003).
     
      1. Done.

    5. a) Request is incompatible with granted mode.
     
      1. Done.


    b003 - grant conversion request
    *******************************

    Main Success Scenario
    .....................

    1) Do `c001 - check request compatible with granted group`_.
    2) Request is compatible.
    3) Grant conversion request.

    Extensions
    ..........

    2. a) Conversion request incompatible with granted group.
     
      1. Set "conversion pending" flag.

    c001 - check request compatible with granted group
    **************************************************

    Main Success Scenario
    .....................

    1) Get next granted request.
    2) Request is compatible with this request.
    3) Repeat from 1) until no more granted requests.

    Extensions
    ..........

    1. a) Request belongs to the caller.
     
      1. Resume from step 3).

    2. a) Request is incompatible with this request.
     
      1. Failure!

**/

type LockMode int

const (
	NONE                       LockMode = 0
	INTENTION_SHARED           LockMode = 1
	INTENTION_EXCLUSIVE        LockMode = 2
	SHARED                     LockMode = 3
	SHARED_INTENTION_EXCLUSIVE LockMode = 4
	UPDATE                     LockMode = 5
	EXCLUSIVE                  LockMode = 6
)

type LockDuration int

const (
	INSTANT_DURATION LockDuration = 1
	MANUAL_DURATION  LockDuration = 2
	COMMIT_DURATION  LockDuration = 3
)

type Comparable interface {
	Equals(other Comparable) bool
}

type Hashable interface {
	HashCode() int
}

type LockOwner interface {
	Comparable
}

type Lockable interface {
	Comparable
	Hashable
}

type LockManager interface {
	Acquire(lockOwner LockOwner, lockable Lockable, lockMode LockMode, lockDuration LockDuration, timeout int) (bool, os.Error)
	Release(lockOwner LockOwner, lockable Lockable, force bool) (bool, os.Error)
	Downgrade(lockOwner LockOwner, lockable Lockable, newLockMode LockMode) (bool, os.Error)
}


/*
 * Implementation
 */


type LockError struct {
	e string
}

func (le *LockError) String() string {
	return le.e
}

var compatibilityMatrix = [7][7]bool{
	[7]bool{true, true, true, true, true, true, true},
	[7]bool{true, true, true, true, true, false, false},
	[7]bool{true, true, true, false, false, false, false},
	[7]bool{true, true, false, true, false, true, false},
	[7]bool{true, true, false, false, false, false, false},
	[7]bool{true, false, false, true, false, false, false},
	[7]bool{true, false, false, false, false, false, false},
}

var conversionMatrix = [][]LockMode{
	[]LockMode{NONE, INTENTION_SHARED, INTENTION_EXCLUSIVE, SHARED,
		SHARED_INTENTION_EXCLUSIVE, UPDATE, EXCLUSIVE},
	[]LockMode{INTENTION_SHARED, INTENTION_SHARED, INTENTION_EXCLUSIVE, SHARED,
		SHARED_INTENTION_EXCLUSIVE, UPDATE, EXCLUSIVE},
	[]LockMode{INTENTION_EXCLUSIVE, INTENTION_EXCLUSIVE, INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		EXCLUSIVE, EXCLUSIVE},
	[]LockMode{SHARED, SHARED, SHARED_INTENTION_EXCLUSIVE, SHARED,
		SHARED_INTENTION_EXCLUSIVE, UPDATE, EXCLUSIVE},
	[]LockMode{SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		SHARED_INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE,
		EXCLUSIVE},
	[]LockMode{UPDATE, UPDATE, EXCLUSIVE, UPDATE, SHARED_INTENTION_EXCLUSIVE,
		UPDATE, EXCLUSIVE},
	[]LockMode{EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE, EXCLUSIVE,
		EXCLUSIVE},
}

func (this LockMode) isCompatible(mode LockMode) bool {
	return compatibilityMatrix[this][mode]
}

func (this LockMode) maximumOf(mode LockMode) LockMode {
	return conversionMatrix[this][mode]
}

var hashPrimes = []int{53, 97, 193, 389, 769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317, 196613, 393241, 786433}

type ReleaseAction int

const (
	RELEASE       ReleaseAction = 1
	FORCE_RELEASE ReleaseAction = 2
	DOWNGRADE     ReleaseAction = 3
)

type LockStatus int

const (
	GRANTED   LockStatus = 1
	GRANTABLE LockStatus = 2
	TIMEOUT   LockStatus = 3
	DEADLOCK  LockStatus = 4
)

type LockManagerImpl struct {
	htsz                     int
	count                    int
	threshold                int
	loadFactor               float
	hashTableSize            int
	lockHashTable            []LockBucket
	globalLock               sync.RWMutex
	stop                     bool
	deadlockDetectorInterval int
}

type Parker struct {
	sema uint32
}

func NewParker() *Parker {
	return &Parker{0}
}

func (this *Parker) Park() {
	runtime.Semacquire(&this.sema)
}

func (this *Parker) Unpark() {
	runtime.Semrelease(&this.sema)
}

type Link interface {
	Next() Link
	Prev() Link
	SetNext(link Link)
	SetPrev(link Link)
}

type List interface {
	Clear()
	GetHead() Link
	GetTail() Link
	AddLast(Link)
	AddFirst(Link)
	Remove(Link)
}

type SimpleLinkedList struct {
	head Link
	tail Link
}

func NewList() *SimpleLinkedList {
	return &SimpleLinkedList{nil, nil}
}

func (this *SimpleLinkedList) Clear() {
	this.head = nil
	this.tail = nil
}

func (this *SimpleLinkedList) GetHead() Link {
	return this.head
}

func (this *SimpleLinkedList) GetTail() Link {
	return this.tail
}

func (this *SimpleLinkedList) AddLast(link Link) {
	link.SetPrev(this.tail)
	link.SetNext(nil)
	if this.head == nil {
		this.head = link
	}
	if this.tail != nil {
		this.tail.SetNext(link)
	}
	this.tail = link
}

func (this *SimpleLinkedList) AddFirst(link Link) {
	link.SetNext(this.head)
	link.SetPrev(nil)
	if this.tail == nil {
		this.tail = link
	}
	if this.head != nil {
		this.head.SetPrev(link)
	}
	this.head = link
}

func (this *SimpleLinkedList) Remove(link Link) {
	next := link.Next()
	prev := link.Prev()
	if next != nil {
		next.SetPrev(prev)
	} else {
		this.tail = prev
	}
	if prev != nil {
		prev.SetNext(next)
	} else {
		this.head = next
	}
	link.SetNext(nil)
	link.SetPrev(nil)
}

type Element struct {
	next Link
	prev Link
}

func (e *Element) Next() Link {
	return e.next
}

func (e *Element) Prev() Link {
	return e.prev
}

func (e *Element) SetNext(link Link) {
	e.next = link
}

func (e *Element) SetPrev(link Link) {
	e.prev = link
}

type LockParams struct {
	lockOwner LockOwner
	lockable  Lockable
	mode      LockMode
	duration  LockDuration
	timeout   int

	downgradeMode LockMode
	action        ReleaseAction
}

type LockBucket struct {
	sync  sync.Mutex
	chain List
}

type LockItem struct {
	Element

	lockable    Lockable
	queue       List
	grantedMode LockMode
	waiting     bool
}

func NewLockItem(lockable Lockable, grantedMode LockMode) *LockItem {
	lockItem := new(LockItem)
	lockItem.lockable = lockable
	lockItem.grantedMode = grantedMode
	return lockItem
}

type LockRequestStatus int

const (
	REQ_GRANTED    LockRequestStatus = 1
	REQ_CONVERTING LockRequestStatus = 2
	REQ_WAITING    LockRequestStatus = 3
	REQ_DENIED     LockRequestStatus = 4
)

type LockRequest struct {
	Element

	status      LockRequestStatus
	mode        LockMode
	convertMode LockMode
	count       int
	duration    LockDuration
	convertDuration    LockDuration
	lockOwner   LockOwner
	lockItem    *LockItem
	requester   *Parker
}

func NewLockRequest(lockItem *LockItem, lockOwner LockOwner, mode LockMode, duration LockDuration) *LockRequest {
	request := new(LockRequest)
	request.lockItem = lockItem
	request.mode = mode
	request.convertMode = mode
	request.duration = duration
	request.lockOwner = lockOwner
	return request
}

type LockContext struct {
	params      *LockParams
	lockRequest *LockRequest
	bucket      *LockBucket
	converting  bool
	lockItem    *LockItem
	status      LockStatus
}

func NewLockManager() *LockManagerImpl {
	lockmgr := new(LockManagerImpl)
	lockmgr.htsz = 0
	lockmgr.count = 0
	lockmgr.loadFactor = 0.75
	lockmgr.hashTableSize = hashPrimes[lockmgr.htsz]
	lockmgr.threshold = int(float(lockmgr.hashTableSize) * lockmgr.loadFactor)
	lockmgr.lockHashTable = make([]LockBucket, lockmgr.hashTableSize)
	lockmgr.stop = false
	lockmgr.deadlockDetectorInterval = 3
	return lockmgr
}

func (l *LockManagerImpl) rehash() {

	if l.htsz == len(hashPrimes)-1 {
		return
	}
	l.globalLock.Lock()
	defer l.globalLock.Unlock()
	if l.htsz == len(hashPrimes)-1 {
		return
	}
	l.htsz += 1
	newHashTableSize := hashPrimes[l.htsz]
	newLockHashTable := make([]LockBucket, newHashTableSize)
	for i := 0; i < l.hashTableSize; i++ {
		bucket := &l.lockHashTable[i]
		for link := bucket.chain.GetHead(); link != nil; {
			next := link.Next()
			item, _ := link.(*LockItem)
			if item.lockable == nil {
				continue
			}
			h := (item.lockable.HashCode() & 0x7FFFFFFF) % newHashTableSize
			newBucket := &newLockHashTable[h]
			newBucket.chain.AddLast(item)
			link = next
		}
	}
	oldLockHashTable := l.lockHashTable
	oldHashTableSize := l.hashTableSize
	l.lockHashTable = newLockHashTable
	l.hashTableSize = newHashTableSize
	l.threshold = (int)(float(l.hashTableSize) * l.loadFactor)
	for i := 0; i < oldHashTableSize; i++ {
		bucket := &oldLockHashTable[i]
		bucket.chain.Clear()
	}
}

func (item *LockItem) findRequest(lockOwner LockOwner) *LockRequest {
	for link := item.queue.GetHead(); link != nil; link = link.Next() {
		request, _ := link.(*LockRequest)
		if request.lockOwner == lockOwner || request.lockOwner.Equals(lockOwner) {
			return request
		}
	}
	return nil
}

func (bucket *LockBucket) findLock(context *LockContext) bool {
	bucket.sync.Lock()
	defer bucket.sync.Unlock()
	context.lockItem = nil
	for link := bucket.chain.GetHead(); link != nil; {
		next := link.Next()
		item, _ := link.(*LockItem)
		if item.lockable == nil {
			bucket.chain.Remove(link)
			continue
		}
		if item.lockable == context.params.lockable || item.lockable.Equals(context.params.lockable) {
			context.lockItem = item
			context.lockRequest = item.findRequest(context.params.lockOwner)
			if context.lockRequest == nil {
				return false
			}
			return true
		}
		link = next
	}
	return false
}

func (l *LockManagerImpl) findLock(context *LockContext) LockMode {
	h := (context.params.lockable.HashCode() & 0x7FFFFFFF) % l.hashTableSize
	context.bucket = &l.lockHashTable[h]
	found := context.bucket.findLock(context)
	if found {
		return context.lockRequest.mode
	}
	return NONE
}

func (l *LockManagerImpl) Acquire(lockOwner LockOwner, lockable Lockable, mode LockMode, duration LockDuration, timeout int) (bool, os.Error) {

	params := &LockParams{}
	params.lockOwner = lockOwner
	params.lockable = lockable
	params.mode = mode
	params.duration = duration
	params.timeout = timeout

	context := &LockContext{}
	context.params = params

	if l.count > l.threshold {
		l.rehash()
	}

	l.globalLock.RLock()
	defer l.globalLock.RUnlock()

	ok, err := l.doAcquire(context)
	if ok && duration == INSTANT_DURATION && context.status == GRANTED {
		context.params.action = RELEASE
		_, err1 := l.doReleaseInternal(context)
		if err1 != nil {
			return false, err1
		}
	}
	return ok, err
}

func (l *LockItem) checkCompatible(request *LockRequest, mode LockMode) bool {

	iscompatible := true

	for link := l.queue.GetHead(); link != nil; link = link.Next() {
		other, _ := link.(*LockRequest)
		if other == request {
			continue
		} else if other.status == REQ_WAITING {
			break
		} else {
			if !mode.isCompatible(other.mode) {
				iscompatible = false
				break
			}
		}
	}
	return iscompatible
}

func (l *LockManagerImpl) doAcquire(context *LockContext) (bool, os.Error) {

	context.converting = false

	h := (context.params.lockable.HashCode() & 0x7FFFFFFF) % l.hashTableSize
	context.lockItem = nil
	context.lockRequest = nil
	context.bucket = &l.lockHashTable[h]

	context.bucket.sync.Lock()
	if l.findLock(context.params.lockOwner, context.params.lockable) == NONE {
		ok, err := l.handleNewRequest(context)
		if ok || err != nil {
			context.bucket.sync.Unlock()
			return ok, err
		}
	} else {
		ok, err := l.handleConversionRequest(context)
		if ok || err != nil {
			context.bucket.sync.Unlock()
			return ok, err
		}
	}

	l.prepareToWait(context)
	context.bucket.sync.Unlock()

	for {
		l.globalLock.RUnlock()
		l.waitForSignal(context)
		l.globalLock.RLock()

		h := (context.params.lockable.HashCode() & 0x7FFFFFFF) % l.hashTableSize
		context.bucket = &l.lockHashTable[h]
		context.bucket.sync.Lock()
		if context.lockRequest.status == REQ_WAITING || context.lockRequest.status == REQ_CONVERTING {
			context.bucket.sync.Unlock()
			continue
		}
		ok, err := l.handleWaitResult(context)
		context.bucket.sync.Unlock()
		return ok, err
	}
	return false, &LockError{"Not reached"}
}

func (l *LockManagerImpl) handleNewLock(context *LockContext) {

	if context.params.duration != INSTANT_DURATION {
		lockItem := NewLockItem(context.params.lockable, context.params.mode)
		r := NewLockRequest(lockItem, context.params.lockOwner,
			context.params.mode, context.params.duration)
		lockItem.queue.AddLast(r)
		context.bucket.chain.AddLast(lockItem)
		l.count++
		context.status = GRANTED
	} else {
		context.status = GRANTABLE
	}
}

func (l *LockManagerImpl) handleNewRequest(context *LockContext) (bool, os.Error) {

	can_grant := (!context.lockItem.waiting && context.params.mode.isCompatible(context.lockItem.grantedMode))
	if context.params.duration == INSTANT_DURATION && can_grant {
		context.status = GRANTABLE
		return true, nil
	} else if !can_grant && context.params.timeout == 0 {
		context.status = TIMEOUT
		return false, &LockError{"Lock request timed out"}
	}

	context.lockRequest = NewLockRequest(context.lockItem,
		context.params.lockOwner, context.params.mode,
		context.params.duration)
	context.lockItem.queue.AddLast(context.lockRequest)
	if can_grant {
		context.lockItem.grantedMode = context.params.mode.maximumOf(context.lockItem.grantedMode)
		context.status = GRANTED
		return true, nil
	} 
	context.converting = false
	return false, nil
}

func (l *LockManagerImpl) handleConversionRequest(context *LockContext) (bool, os.Error) {

	if context.lockRequest.status == REQ_CONVERTING ||
		context.lockRequest.status == REQ_WAITING {
		return false, &LockError{"Internal error: invalid state"}
	} 

	if context.lockRequest.status == REQ_GRANTED {
		if context.params.mode == context.lockRequest.mode {
			if context.params.duration != INSTANT_DURATION {
				context.lockRequest.count++
			}
			if context.params.duration == INSTANT_DURATION {
				context.status = GRANTABLE
			} else {
				if context.lockRequest.duration == MANUAL_DURATION &&
					context.params.duration == COMMIT_DURATION {
					context.lockRequest.duration = COMMIT_DURATION
				}
				context.status = GRANTED
			}
			return true, nil
		} else {
			can_grant := context.lockItem.checkCompatible(
				context.lockRequest, context.params.mode)
			if can_grant {
				if context.params.duration != INSTANT_DURATION {
					context.lockRequest.mode =
						context.params.mode.maximumOf(context.lockRequest.mode)
					context.lockRequest.count++
					if context.lockRequest.duration == MANUAL_DURATION &&
						context.params.duration == COMMIT_DURATION {
						context.lockRequest.duration = COMMIT_DURATION
					}
					context.lockItem.grantedMode = context.lockRequest.mode.maximumOf(context.lockItem.grantedMode)
					context.status = GRANTED
				} else {
					context.status = GRANTABLE
				}
				return true, nil
			} else if !can_grant && context.params.timeout == 0 {
				context.status = TIMEOUT
				return false, &LockError{"Lock request timed out"}
			} 
			context.converting = true
			return false, nil
		}
	}
	return false, &LockError{"Internal error: unexpected state"}
}

func (l *LockManagerImpl) prepareToWait(context *LockContext) {
	context.lockItem.waiting = true
	if !context.converting {
		context.lockRequest.status = REQ_WAITING
	} else {
		context.lockRequest.convertMode = context.params.mode
		context.lockRequest.convertDuration = context.params.duration
		context.lockRequest.status = REQ_CONVERTING
	}
	context.lockRequest.requester = NewParker()
}

func (l *LockManagerImpl) waitForSignal(context *LockContext) {
	context.lockRequest.requester.Park()
}

func (l *LockManagerImpl) handleWaitResult(context *LockContext) (bool, os.Error) {
        lockRequestStatus := context.lockRequest.status
        if lockRequestStatus == REQ_GRANTED {
            context.status = GRANTED
        } else if lockRequestStatus == REQ_DENIED {
            context.status = DEADLOCK
        } else {
            context.status = TIMEOUT
        }

        if context.status == GRANTED {
            /*
             * 9. If after the wait, the lock has been granted, then return
             * success.
             */
            return true, nil
        }

        /* 10. Else return failure. */

        if !context.converting {
            /* If not converting the delete the newly created request. */
            context.lockItem.queue.Remove(context.lockRequest)
            if context.lockItem.queue.GetHead() == nil {
                context.lockItem.lockable = nil // Setup lock for garbage collection
                l.count--
            }
        } else {
            /* If converting, then restore old status */
            context.lockRequest.status = REQ_GRANTED
            context.lockRequest.convertMode = context.lockRequest.mode
        }
        if context.status == DEADLOCK {
            /* 
             * If we have been chosen as a deadlock victim, then we need to grant the
             * lock to the waiter who has won the deadlock.
             */
            l.grantWaiters(RELEASE, context.lockRequest,
                    context.handle, context.lockItem)
        }
        if context.status == TIMEOUT {
		return false, &LockError{"Lock request timed out"}
        } else if context.status == DEADLOCK {
		return false, &LockError{"Lock request denied"}
        }
	return false, &LockError{"Lock request denied"}
    }

func (l *LockManagerImpl) grantWaiters(action ReleaseAction, myReq *LockRequest, lockitem LockItem) {
        /*
         * 9. Recalculate granted mode by calculating max mode amongst all
         * granted (including conversion) requests. If a conversion request
         * is compatible with all other granted requests, then grant the
         * conversion, recalculating granted mode. If a waiting request is
         * compatible with granted mode, and there are no pending conversion
         * requests, then grant the request, and recalculate granted mode.
         * Otherwise, we are done. Note that this means that FIFO is
         * respected for waiting requests, but conversion requests are
         * granted as soon as they become compatible. Also, note that if a
         * conversion request is pending, waiting requests cannot be
         * granted.
         */
	lockitem.grantedMode = NONE
	lockitem.waiting = false
	converting := false
	for link := lockitem.queue.GetHead(); link != null; link = link.Next() {
		r, _ := link.(*LockRequest)
		if r.status == REQ_GRANTED {	
			lockitem.grantedMode = r.mode.maximumOf(lockitem.grantedMode)
		} else if r.status == REQ_CONVERTING) {
			can_grant := lockitem.checkCompatible(lockitem, r, r.convertMode)
			if can_grant {
				if r.convertDuration == INSTANT_DURATION {
					/*
					 * If the request is for an instant duration lock then
					 * don't perform the conversion.
					 */
					r.convertMode = r.mode
				} else {
					r.mode = r.convertMode.maximumOf(r.mode)
					r.convertMode = r.mode
					if r.convertDuration == COMMIT_DURATION
						&& r.duration == MANUAL_DURATION {
						r.duration = COMMIT_DURATION
					}
					lockitem.grantedMode = r.mode.maximumOf(lockitem.grantedMode)
				}
				/*
				 * Treat conversions as lock recursion.
				 */
				r.count++
				r.status = REQ_GRANTED
				r.requester.Unpark()
			} else {
				lockitem.grantedMode = r.mode.maximumOf(lockitem.grantedMode)
				converting = true
				lockitem.waiting = true
			}
		} else if r.status == REQ_WAITING {
			if !converting && r.mode.isCompatible(lockitem.grantedMode) {
				r.status = REQ_GRANTED
				lockitem.grantedMode = r.mode.maximumOf(lockitem.grantedMode)
				r.requester.Unpark()
			} else {
				lockitem.waiting = true
				break
			}
		}
	}
}

func (l *LockManagerImpl) releaseLock(context *LockContext) (bool, os.Error) {

        released := false
        /* 3. If lock found, look for the transaction's lock request. */
        context.lockRequest = context.lockitem.findRequest(context.parms.lockOwner)

        if context.lockRequest == nil {
            /* 4. If not found, return success. */
            /*
             * Rather than throwing an exception, we return success. This allows
             * us to use this method in situations where for reasons of efficiency,
             * the client cannot track the status of lock objects, and therefore
             * may end up trying to release the same lock multiple times.
             */
            return true, nil
        }

        if context.lockRequest.status == REQ_CONVERTING
                || context.lockRequest.status == REQ_WAITING {
            /* 5. If lock in invalid state, return error. */
            return false, &LockError{"Invalid state"}
        }

        if context.params.action == DOWNGRADE
                && context.lockRequest.mode == context.parms.downgradeMode {
            /*
             * If downgrade request and lock is already in target mode,
             * return success.
             */
            return true, nil
        }

        if context.parms.action == RELEASE
                && context.lockRequest.duration == COMMIT_DURATION {
            /*
             * 6(1). If noforce, and lock is held for commit duration, then do
             * not release the lock request.
             */
            return false, nil
        }

        if context.parms.action == RELEASE
                && context.lockRequest.count > 1 {
            /*
             * 6. If noforce, and reference count greater than 0, then do
             * not release the lock request. Decrement reference count if
             * greater than 0, and, return Ok.
             */
            context.lockRequest.count--
            return false, nil
        }

        /*
         * Either the lock is being downgraded or it is being released and
         * its reference count == 0 or it is being forcibly released.
         */

        if context.lockRequest == context.lockitem.queue.GetHead()
                && context.lockRequest == context.lockitem.queue.GetTail()
                && context.parms.action != DOWNGRADE {
            /* 7. If sole lock request, then release the lock and return Ok. */
            context.lockitem.queue.Remove(context.lockRequest)
            context.lockitem.lockable = nil
            count--
            return true, nil
        }

        /*
         * 8. If not downgrading, delete the lock request from the queue.
         * Otherwise, downgrade the mode assigned to the lock request.
         */
        if context.parms.action != ReleaseAction.DOWNGRADE {
            context.lockitem.queue.Remove(context.lockRequest)
            released = true
        } else {
            /*
             * We need to determine whether is a valid downgrade request. To
             * do so, we do a reverse check - ie, if the new mode could have
             * been upgraded to current mode, then it is okay to downgrade.
             */
            mode := context.parms.downgradeMode.maximumOf(context.lockRequest.mode)
            if mode == context.lockRequest.mode {
                context.lockRequest.convertMode = context.lockRequest.mode = context.parms.downgradeMode
            } else {
                return false, &LockError{"Invalid downgrade request"}
            }
            released = false
        }
        /*
         * 9. Recalculate granted mode by calculating max mode amongst all
         * granted (including conversion) requests.
         */
        ok, err := grantWaiters(context.parms.action, context.lockRequest, context.lockitem)
        if err != nil {
               return released, err
        }
        return released, nil
    }

func (l *LockManagerImpl) release(lockOwner LockOwner, lockable Lockable, bool force) (bool, os.Error) {
        params := new(LockParams)
        params.lockable = lockable
        params.lockOwner = lockOwner
	if force {
		params.action = FORCE_RELEASE
	} else {
		params.action = RELEASE
	}
        context := new(LockContext)
	context.params = params

        l.globalLock.RLock()
	defer l.globalLock.RUnlock()
        return l.doReleaseInternal(context)
    }

func (l *LockManagerImpl) downgrade(lockOwner LockOwner, lockable Lockable, downgradeTo LockMode) (bool, os.Error) {
        params := new(LockParams)
        params.lockable = lockable
        params.lockOwner = lockOwner
        params.action = DOWNGRADE
        params.downgradeMode = downgradeTo

        context := new(LockContext)
	context.params = params

        l.globalLock.RLock()
	defer l.globalLock.RUnlock()
        return l.doReleaseInternal(context)
    }

func (l *LockManagerImpl) doReleaseInternal(context *LockContext) (bool, os.Error) {
        context.lockRequest = nil

        h := (context.parms.lockable.HashCode() & 0x7FFFFFFF) % l.hashTableSize
        context.bucket = &l.lockHashTable[h]
	context.bucket.sync.Lock()
	defer context.bucket.sync.Unlock()
            /* 1. Search for the lock. */
            if !context.bucket.findLock(context) {
                /* 2. If not found, return success. */
                /*
                 * Rather than throwing an exception, we return success. This allows
                 * us to use this method in situations where for reasons of efficiency,
                 * the client cannot track the status of lock objects, and therefore
                 * may end up trying to release the same lock multiple times.
                 */
                return true, nil
            }
            return l.releaseLock(context)
    }

type IntObject int

func (i IntObject) Equals(other Comparable) bool {
	switch j := other.(type) {
	case IntObject:
		return i == j
	default:
		return false
	}
	return false
}

func (i IntObject) HashCode() int {
	return int(i)
}

func main() {
	fmt.Printf("Lock compatible = %t\n", NONE.isCompatible(NONE))
	fmt.Printf("Lock compatible = %t\n", EXCLUSIVE.isCompatible(SHARED))
	fmt.Printf("Lock maxof = %v\n", SHARED.maximumOf(EXCLUSIVE))

	lockRequester := NewParker()
	go waiter(lockRequester)

	time.Sleep(1000000000)
	fmt.Printf("Signalling\n")
	lockRequester.Unpark()

	time.Sleep(1000000000)

	list := NewList()
	list.AddLast(&Element{1, nil, nil})

	for link := list.GetHead(); link != nil; link = link.Next() {
		e, _ := link.(*Element)
		fmt.Printf("list item = %v\n", e.i)
	}

	lockmgr := NewLockManager()
	fmt.Printf("Len of hash table = %v\n", len(lockmgr.lockHashTable))
	fmt.Printf("threshold = %v\n", lockmgr.threshold)
	lockmgr.rehash()
	fmt.Printf("Len of hash table = %v\n", len(lockmgr.lockHashTable))
	fmt.Printf("threshold = %v\n", lockmgr.threshold)

	var i IntObject = 33
	var j IntObject = 33
	var k IntObject = 34

	fmt.Printf("i == j: %v\n", i.Equals(j))
	fmt.Printf("i == k: %v\n", i.Equals(k))

	_, err := lockmgr.Acquire(i, j, SHARED, COMMIT_DURATION, 10)
	if err != nil {
		fmt.Printf("error occurred: %v\n", err)
	}
}
