package lockmgr

import (
//	"fmt"
	"sync"
	"runtime"
	"os"
)

type LockMode int
type LockDuration int

const (
	NONE                       LockMode = 0
	INTENTION_SHARED           LockMode = 1
	INTENTION_EXCLUSIVE        LockMode = 2
	SHARED                     LockMode = 3
	SHARED_INTENTION_EXCLUSIVE LockMode = 4
	UPDATE                     LockMode = 5
	EXCLUSIVE                  LockMode = 6
)

const (
	InstantDuration LockDuration = 1
	ManualDuration  LockDuration = 2
	CommitDuration  LockDuration = 3
)

type Comparable interface {
	Equals(other Comparable) bool
}

type Hashable interface {
	Hash() uint32
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
	FindLock(lockOwner LockOwner, lockable Lockable) LockMode
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

func (m LockMode) isCompatible(mode LockMode) bool {
	return compatibilityMatrix[m][mode]
}

func (m LockMode) maximumOf(mode LockMode) LockMode {
	return conversionMatrix[m][mode]
}

var hashPrimes []uint32 = []uint32{53, 97, 193, 389, 769, 1543, 3079, 6151,
	12289, 24593, 49157, 98317, 196613, 393241, 786433}

type ReleaseAction int

const (
	RELEASE       ReleaseAction = 1
	DOWNGRADE     ReleaseAction = 2
)

type LockStatus int

const (
	GRANTED   LockStatus = 1
	GRANTABLE LockStatus = 2
	TIMEOUT   LockStatus = 3
	DEADLOCK  LockStatus = 4
	ERROR     LockStatus = 5
	DOWAIT    LockStatus = 6
	DOCONVERT LockStatus = 7
)

type LockManagerImpl struct {
	htsz                     uint32
	count                    int
	threshold                uint32
	loadFactor               float
	hashTableSize            uint32
	lockHashTable            []LockBucket
	globalLock               sync.RWMutex
	stop                     bool
	deadlockDetectorInterval int
}

type Parker struct {
	sema uint32
}

func (p *Parker) Park() {
	runtime.Semacquire(&p.sema)
}

func (p *Parker) Unpark() {
	runtime.Semrelease(&p.sema)
}

type LockBucket struct {
	sync    sync.Mutex
	head    *LockItem
	tail    *LockItem
	free    *LockItem
	freereq *LockRequest
}

func (b *LockBucket) lock() {
	b.sync.Lock()
}

func (b *LockBucket) unlock() {
	b.sync.Unlock()
}

func (b *LockBucket) newLockItem() *LockItem {
	if b.free != nil {
		l := b.free
		b.free = b.free.next
		l.next = nil
		return l
	}
//	fmt.Print("alloc item\n")
	return new(LockItem)
}

func (b *LockBucket) newLockRequest() *LockRequest {
	if b.freereq != nil {
		l := b.freereq
		b.freereq = b.freereq.next
		l.next = nil
		return l
	}
//	fmt.Print("alloc req\n")
	return new(LockRequest)
}

func (b *LockBucket) addLast(l *LockItem) {
	l.prev = b.tail
	l.next = nil
	if b.head == nil {
		b.head = l
	}
	if b.tail != nil {
		b.tail.next = l
	}
	b.tail = l
}

func (b *LockBucket) remove(l *LockItem) {
	next := l.next
	prev := l.prev
	if next != nil {
		next.prev = prev
	} else {
		b.tail = prev
	}
	if prev != nil {
		prev.next = next
	} else {
		b.head = next
	}
	l.prev = nil
	l.next = b.free
	b.free = l
}

type LockItem struct {
	next *LockItem
	prev *LockItem

	lockable    Lockable
	head        *LockRequest
	tail        *LockRequest
	grantedMode LockMode
	waiting     bool
}

func newLockItem(b *LockBucket, lockable Lockable, grantedMode LockMode) *LockItem {
	l := b.newLockItem()
	l.lockable = lockable
	l.grantedMode = grantedMode
	return l
}


func (l *LockItem) addLast(r *LockRequest) {
	r.prev = l.tail
	r.next = nil
	if l.head == nil {
		l.head = r
	}
	if l.tail != nil {
		l.tail.next = r
	}
	l.tail = r
}

func (l *LockItem) remove(r *LockRequest, b *LockBucket) {
	next := r.next
	prev := r.prev
	if next != nil {
		next.prev = prev
	} else {
		l.tail = prev
	}
	if prev != nil {
		prev.next = next
	} else {
		l.head = next
	}
	b.recycle(r)
}

func (b *LockBucket) recycle(r *LockRequest) {
	r.prev = nil
	r.next = b.freereq
	b.freereq = r
// 	fmt.Printf("free req\n");
}

type LockRequestStatus int

const (
	REQ_GRANTED    LockRequestStatus = 1
	REQ_CONVERTING LockRequestStatus = 2
	REQ_WAITING    LockRequestStatus = 3
	REQ_DENIED     LockRequestStatus = 4
)

type LockRequest struct {
	next  *LockRequest
	prev  *LockRequest
	gnext *LockRequest

	status          LockRequestStatus
	mode            LockMode
	convertMode     LockMode
	count           int
	duration        LockDuration
	convertDuration LockDuration
	lockOwner       LockOwner
	lockItem        *LockItem
	requester       Parker
}

func newLockRequest(b *LockBucket, lockItem *LockItem, lockOwner LockOwner, mode LockMode, duration LockDuration, status LockRequestStatus, count int) *LockRequest {
	r := b.newLockRequest()
	r.lockItem = lockItem
	r.mode = mode
	r.convertMode = mode
	r.duration = duration
	r.convertDuration = duration
	r.lockOwner = lockOwner
	r.status = status
	r.count = count
	return r
}

func NewLockManager() *LockManagerImpl {
	lm := new(LockManagerImpl)
	lm.htsz = 0
	lm.count = 0
	lm.loadFactor = 0.75
	lm.hashTableSize = hashPrimes[lm.htsz]
	lm.threshold = uint32(float(lm.hashTableSize) * lm.loadFactor)
	lm.lockHashTable = make([]LockBucket, lm.hashTableSize)
	lm.stop = false
	lm.deadlockDetectorInterval = 3
	return lm
}

func (lm *LockManagerImpl) rehash() {

	if int(lm.htsz) == len(hashPrimes)-1 {
		return
	}
	lm.writeLock()
	defer lm.writeUnlock()
	if int(lm.htsz) == len(hashPrimes)-1 {
		return
	}
	lm.htsz += 1
	newHashTableSize := hashPrimes[lm.htsz]
	newLockHashTable := make([]LockBucket, newHashTableSize)
	var i uint32
	for i = 0; i < lm.hashTableSize; i++ {
		b := &lm.lockHashTable[i]
		for l := b.head; l != nil; l = l.next {
			if l.lockable != nil {
				h := l.lockable.Hash() % newHashTableSize
				nb := &newLockHashTable[h]
				nb.addLast(l)
			}
		}
	}
	oldLockHashTable := lm.lockHashTable
	oldHashTableSize := lm.hashTableSize
	lm.lockHashTable = newLockHashTable
	lm.hashTableSize = newHashTableSize
	lm.threshold = uint32(float(lm.hashTableSize) * lm.loadFactor)
	for i = 0; i < oldHashTableSize; i++ {
		b := &oldLockHashTable[i]
		b.head = nil
		b.tail = nil
	}
}

func (lm *LockManagerImpl) readLock() {
	lm.globalLock.RLock()
}

func (lm *LockManagerImpl) writeLock() {
	lm.globalLock.Lock()
}

func (lm *LockManagerImpl) readUnlock() {
	lm.globalLock.RUnlock()
}

func (lm *LockManagerImpl) writeUnlock() {
	lm.globalLock.Unlock()
}

func (l *LockItem) findRequest(lockOwner LockOwner) *LockRequest {
	for r := l.head; r != nil; r = r.next {
		if r.lockOwner == lockOwner || r.lockOwner.Equals(lockOwner) {
			return r
		}
	}
	return nil
}

func (b *LockBucket) findLock(lockable Lockable) *LockItem {
	for l := b.head; l != nil; l = l.next {
		if l.lockable == lockable || l.lockable.Equals(lockable) {
			return l
		}
	}
	return nil
}

func (lm *LockManagerImpl) bucket(lockable Lockable) *LockBucket {
	h := lockable.Hash() % lm.hashTableSize
	return &lm.lockHashTable[h]
}

func (lm *LockManagerImpl) FindLock(lockOwner LockOwner, lockable Lockable) LockMode {

	lm.readLock()
	b := lm.bucket(lockable)
	b.lock()
	lockItem := b.findLock(lockable)
	m := NONE
	if lockItem != nil {
		r := lockItem.findRequest(lockOwner)
		if r != nil {
			m = r.mode
		}
	}
	b.unlock()
	lm.readUnlock()
	return m
}

func (lm *LockManagerImpl) Acquire(lockOwner LockOwner, lockable Lockable, mode LockMode, duration LockDuration, timeout int) (bool, os.Error) {

	if lm.count > int(lm.threshold) {
		lm.rehash()
	}

	lm.readLock()
	status, err := lm.doAcquire(lockOwner, lockable, mode, duration, timeout)
	ok := (status == GRANTED || status == GRANTABLE) && err == nil
	if duration == InstantDuration && status == GRANTED && err == nil {
		_, err1 := lm.doReleaseInternal(RELEASE, lockOwner, lockable, NONE, false)
		if err1 != nil {
			ok, err = false, err1
		}
	}
	lm.readUnlock()
	return ok, err
}

func (l *LockItem) checkCompatible(r *LockRequest, mode LockMode) bool {

	iscompatible := true
	for other := l.head; other != nil; other = other.next {
		if other == r {
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

func (lm *LockManagerImpl) doAcquire(o LockOwner, lockable Lockable, m LockMode, d LockDuration, timeout int) (LockStatus, os.Error) {

	b := lm.bucket(lockable)
	b.lock()
	l := b.findLock(lockable)
	status := ERROR
	if l == nil {
		/*
		 * 2. If not found, this is a new lock and therefore grant the
		 * lock, and return success.
		 */
		status := lm.handleNewLock(b, o, lockable, m, d)
		b.unlock()
		return status, nil
	}
	r := l.findRequest(o)
	var err os.Error
	var done bool
	if r == nil {
		r, status, done, err = l.handleNewRequest(b, o, lockable, m, d, timeout)
	} else {
		status, done, err = l.handleConversionRequest(r, m, d, timeout)
	}
	if done {
		b.unlock()
		return status, err
	}
	converting := status == DOCONVERT
	l.prepareToWait(r, m, d, converting)
	b.unlock()

	for {
		lm.readUnlock()
		r.waitForSignal()
		lm.readLock()

		b := lm.bucket(lockable)
		b.lock()
		if r.status == REQ_WAITING || r.status == REQ_CONVERTING {
			b.unlock()
			continue
		}
		status, err = lm.handleWaitResult(b, l, r, converting)
		b.unlock()
		break
	}
	return status, err
}

func (lm *LockManagerImpl) handleNewLock(b *LockBucket, o LockOwner, lockable Lockable, m LockMode, d LockDuration) LockStatus {
	if d != InstantDuration {
		l := newLockItem(b, lockable, m)
		r := newLockRequest(b, l, o, m, d, REQ_GRANTED, 1)
		l.addLast(r)
		b.addLast(l)
		lm.count++
		return GRANTED
	}
	return GRANTABLE
}

func (l *LockItem) handleNewRequest(b *LockBucket, o LockOwner, lockable Lockable, m LockMode, d LockDuration, timeout int) (*LockRequest, LockStatus, bool, os.Error) {

	can_grant := (!l.waiting && m.isCompatible(l.grantedMode))
	if d == InstantDuration && can_grant {
		return nil, GRANTABLE, true, nil
	} else if !can_grant && timeout == 0 {
		return nil, TIMEOUT, true, &LockError{"Lock request timed out"}
	}
	r := newLockRequest(b, l, o, m, d, REQ_WAITING, 1)
	l.addLast(r)
	if can_grant {
		r.status = REQ_GRANTED
		l.grantedMode = m.maximumOf(l.grantedMode)
		return r, GRANTED, true, nil
	}
	return r, DOWAIT, false, nil
}

func (l *LockItem) handleConversionRequest(r *LockRequest, m LockMode, d LockDuration, timeout int) (LockStatus, bool, os.Error) {

	if r.status == REQ_CONVERTING || r.status == REQ_WAITING {
		panic("Internal error: invalid state")
	}
	var status LockStatus = ERROR
	var done bool = false
	var err os.Error = nil
	if r.status == REQ_GRANTED {
		if m == r.mode {
			if d == InstantDuration {
				status = GRANTABLE
			} else {
				r.count++
				if r.duration == ManualDuration &&
					d == CommitDuration {
					r.duration = CommitDuration
				}
				status = GRANTED
			}
			done = true
		} else {
			can_grant := l.checkCompatible(r, m)
			if can_grant {
				if d != InstantDuration {
					r.mode = m.maximumOf(r.mode)
					r.count++
					if r.duration == ManualDuration &&
						d == CommitDuration {
						r.duration = CommitDuration
					}
					l.grantedMode = r.mode.maximumOf(l.grantedMode)
					status = GRANTED
				} else {
					status = GRANTABLE
				}
				done = true
			} else if !can_grant && timeout == 0 {
				status, done, err = TIMEOUT, true, &LockError{"Lock request timed out"}
			} else {
				status, done, err = DOCONVERT, false, nil
			}
		}
	}
	return status, done, err
}

func (l *LockItem) prepareToWait(r *LockRequest, m LockMode, d LockDuration, converting bool) {
	l.waiting = true
	if !converting {
		r.status = REQ_WAITING
	} else {
		r.convertMode = m
		r.convertDuration = d
		r.status = REQ_CONVERTING
	}
}

func (r *LockRequest) waitForSignal() {
	r.requester.Park()
}

func (r *LockRequest) Signal() {
	r.requester.Unpark()
}

func (lm *LockManagerImpl) handleWaitResult(b *LockBucket, l *LockItem, r *LockRequest, converting bool) (LockStatus, os.Error) {
	var status LockStatus = ERROR
	if r.status == REQ_GRANTED {
		status = GRANTED
	} else if r.status == REQ_DENIED {
		status = DEADLOCK
	} else {
		status = TIMEOUT
	}
	if status == GRANTED {
		/*
		 * If after the wait, the lock has been granted, then return
		 * success.
		 */
		return status, nil
	}
	/* 10. Else return failure. */
	if !converting {
		/* If not converting the delete the newly created request. */
		l.remove(r, b)
		if l.head == nil {
			b.remove(l)
			lm.count--
			l = nil
		}
	} else {
		/* If converting, then restore old status */
		r.status = REQ_GRANTED
		r.convertMode = r.mode
	}
	if status == DEADLOCK {
		/*
		 * If we have been chosen as a deadlock victim, then we need to grant the
		 * lock to the waiter who has won the deadlock.
		 */
		l.grantWaiters()
	}
	if status == TIMEOUT {
		return status, &LockError{"Lock request timed out"}
	} else if status == DEADLOCK {
		return status, &LockError{"Lock request denied"}
	}
	panic("not reached")
	return ERROR, &LockError{"error"}
}

func (l *LockItem) grantWaiters() {
	/*
	 * Recalculate granted mode by calculating max mode amongst all
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
	l.grantedMode = NONE
	l.waiting = false
	converting := false
	for r := l.head; r != nil; r = r.next {
		if r.status == REQ_GRANTED {
			l.grantedMode = r.mode.maximumOf(l.grantedMode)
		} else if r.status == REQ_CONVERTING {
			can_grant := l.checkCompatible(r, r.convertMode)
			if can_grant {
				if r.convertDuration == InstantDuration {
					/*
					 * If the request is for an instant duration lock then
					 * don't perform the conversion.
					 */
					r.convertMode = r.mode
				} else {
					r.mode = r.convertMode.maximumOf(r.mode)
					r.convertMode = r.mode
					if r.convertDuration == CommitDuration && r.duration == ManualDuration {
						r.duration = CommitDuration
					}
					l.grantedMode = r.mode.maximumOf(l.grantedMode)
				}
				/*
				 * Treat conversions as lock recursion.
				 */
				r.count++
				r.status = REQ_GRANTED
				r.Signal()
			} else {
				l.grantedMode = r.mode.maximumOf(l.grantedMode)
				converting = true
				l.waiting = true
			}
		} else if r.status == REQ_WAITING {
			if !converting && r.mode.isCompatible(l.grantedMode) {
				r.status = REQ_GRANTED
				l.grantedMode = r.mode.maximumOf(l.grantedMode)
				r.Signal()
			} else {
				l.waiting = true
				break
			}
		}
	}
}

func (lm *LockManagerImpl) releaseLock(a ReleaseAction, o LockOwner, lockable Lockable, downgradeMode LockMode, force bool, b *LockBucket, l *LockItem) (bool, os.Error) {

	/* look for the transaction's lock request. */
	r := l.findRequest(o)
	if r == nil {
		/* not found, return success. */
		return true, nil
	}
	if a == RELEASE {
		if !force {
			if r.duration == CommitDuration {
				/*
				 * If noforce, and lock is held for commit duration, then do
				 * not release the lock request.
				 */
				return false, nil
			}
			r.count--
			if r.count > 0 {
				/*
				 * If noforce, and reference count greater than 0, then do
				 * not release the lock request.
				 */
				return false, nil
			}
		}
		l.remove(r, b)
		if l.head == nil {
			b.remove(l)
			lm.count--
			return true, nil
		}
	} else /* if a == DOWNGRADE */ {
		/*
		 * We need to determine whether is a valid downgrade request. To
		 * do so, we do a reverse check - ie, if the new mode could have
		 * been upgraded to current mode, then it is okay to downgrade.
		 */
		if r.mode == downgradeMode {
			/*
			 * If downgrade request and lock is already in target mode,
			 * return success.
			 */
			return true, nil
		}
		mode := downgradeMode.maximumOf(r.mode)
		if mode == r.mode {
			r.mode = downgradeMode
			r.convertMode = downgradeMode
		} else {
			return false, &LockError{"Invalid downgrade request"}
		}
	}
	/*
	 * 9. Recalculate granted mode by calculating max mode amongst all
	 * granted (including conversion) requests.
	 */
	l.grantWaiters()
	return true, nil
}

func (lm *LockManagerImpl) Release(o LockOwner, lockable Lockable, force bool) (bool, os.Error) {
	lm.readLock()
	ok, err := lm.doReleaseInternal(RELEASE, o, lockable, NONE, force)
	lm.readUnlock()
	return ok, err
}

func (lm *LockManagerImpl) Downgrade(o LockOwner, lockable Lockable, downgradeTo LockMode) (bool, os.Error) {
	lm.readLock()
	ok, err := lm.doReleaseInternal(DOWNGRADE, o, lockable, downgradeTo, false)
	lm.readUnlock()
	return ok, err
}

func (lm *LockManagerImpl) doReleaseInternal(a ReleaseAction, o LockOwner, lockable Lockable, downgradeMode LockMode, force bool) (bool, os.Error) {
	b := lm.bucket(lockable)
	b.lock()
	/* 1. Search for the lock. */
	l := b.findLock(lockable)
	var ok bool
	var err os.Error = nil
	if l != nil {
		ok, err = lm.releaseLock(a, o, lockable, downgradeMode, force, b, l)
	}
	b.unlock()
	return ok, err
}

