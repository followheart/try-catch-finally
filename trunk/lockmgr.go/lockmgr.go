package main

import (
	"fmt"
	"sync"
	"runtime"
	"time"
	"os"
)

type LockError struct {
	e string
}

func (le *LockError) String() string {
	return le.e
}

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

func waiter(parker *Parker) {
	fmt.Printf("Going to sleep\n")
	parker.Park()
	fmt.Printf("Woke up\n")
}

type Link interface {
	Next() Link
	Prev() Link
	SetNext(link Link)
	SetPrev(link Link)
}

type SimpleLinkedList struct {
	head Link
	tail Link
}

func NewList() *SimpleLinkedList {
	return &SimpleLinkedList{nil, nil}
}

func (this *SimpleLinkedList) clear() {
	this.head = nil
	this.tail = nil
}

func (this *SimpleLinkedList) getHead() Link {
	return this.head
}

func (this *SimpleLinkedList) addLast(link Link) {
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

func (this *SimpleLinkedList) addFirst(link Link) {
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

func (this *SimpleLinkedList) remove(link Link) {
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
	i    int
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
	chain SimpleLinkedList
}

type LockItem struct {
	Element

	lockable    Lockable
	queue       SimpleLinkedList
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
		for link := bucket.chain.getHead(); link != nil; {
			next := link.Next()
			item, _ := link.(*LockItem)
			if item.lockable == nil {
				continue
			}
			h := (item.lockable.HashCode() & 0x7FFFFFFF) % newHashTableSize
			newBucket := &newLockHashTable[h]
			newBucket.chain.addLast(item)
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
		bucket.chain.clear()
	}
}

func (item *LockItem) findRequest(lockOwner LockOwner) *LockRequest {
	for link := item.queue.getHead(); link != nil; link = link.Next() {
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
	for link := bucket.chain.getHead(); link != nil; {
		next := link.Next()
		item, _ := link.(*LockItem)
		if item.lockable == nil {
			bucket.chain.remove(link)
			continue
		}
		if item.lockable == context.params.lockable || item.lockable.Equals(context.params.lockable) {
			context.lockItem = item
			context.lockRequest = item.findRequest(context.params.lockOwner)
			return true
		}
		link = next
	}
	return false
}

func (l *LockManagerImpl) findLock(lockOwner LockOwner, lockable Lockable) LockMode {
	params := &LockParams{}
	params.lockOwner = lockOwner
	params.lockable = lockable

	context := &LockContext{}
	context.params = params

	l.globalLock.RLock()
	defer l.globalLock.RUnlock()

	var h int = (context.params.lockable.HashCode() & 0x7FFFFFFF) % l.hashTableSize
	context.lockItem = nil
	context.bucket = &l.lockHashTable[h]
	context.lockRequest = nil

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
	if ok && duration == INSTANT_DURATION {
		_, err1 := l.doRelease(context)
		if err1 != nil {
			return false, err1
		}
	}
	return ok, err
}

func (l *LockManagerImpl) doRelease(context *LockContext) (bool, os.Error) {
	return false, &LockError{"Not yet implemented"}
}

func (l *LockItem) checkCompatible(request *LockRequest, mode LockMode) bool {

	iscompatible := true

	for link := l.queue.getHead(); link != nil; link = link.Next() {
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
		lockItem.queue.addLast(r)
		context.bucket.chain.addLast(lockItem)
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
		return false, &LockError{"Lock request denied"}
	}

	context.lockRequest = NewLockRequest(context.lockItem,
		context.params.lockOwner, context.params.mode,
		context.params.duration)
	context.lockItem.queue.addLast(context.lockRequest)
	if can_grant {
		context.lockItem.grantedMode = context.params.mode.maximumOf(context.lockItem.grantedMode)
		context.status = GRANTED
		return true, nil
	} else {
		context.converting = false
	}
	return false, nil
}

func (l *LockManagerImpl) handleConversionRequest(context *LockContext) (bool, os.Error) {
	if context.lockRequest.status == REQ_CONVERTING ||
		context.lockRequest.status == REQ_WAITING {
		return false, &LockError{"Internal error: invalid state"}
	} else if context.lockRequest.status == REQ_GRANTED {
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
				return false, &LockError{"Lock request denied"}
			} else {
				context.converting = true
			}
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
}

func (l *LockManagerImpl) waitForSignal(context *LockContext) {
	context.lockRequest.requester = NewParker()
	context.lockRequest.requester.Park()
	context.lockRequest.requester = nil
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
            context.lockItem.queue.remove(context.lockRequest)
            if context.lockItem.queue.getHead() == nil {
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
	list.addLast(&Element{1, nil, nil})

	for link := list.getHead(); link != nil; link = link.Next() {
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
