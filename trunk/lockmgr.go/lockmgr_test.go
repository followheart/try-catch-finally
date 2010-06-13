package lockmgr

import (
	"testing"
	"time"
	"fmt"
	"runtime"
	"sync"
)

func TestSimpleLock(t *testing.T) {
	var j IntObject = 33
	var lockOwner IntObject = 34
	m := NewLockManager()

	ok, err := m.Acquire(lockOwner, j, EXCLUSIVE, ManualDuration, 0)
	if !ok || err != nil {
		t.Errorf("Failed to acquire lock")
		return
	}
	mode := m.FindLock(lockOwner, j)
	if mode != EXCLUSIVE {
		t.Errorf("Failed to find lock")
		return
	}
	ok, err = m.Release(lockOwner, j, false)
	if !ok || err != nil {
		t.Errorf("Failed to release lock")
		return
	}
	mode = m.FindLock(lockOwner, j)
	if mode != NONE {
		t.Errorf("Found returned unexpected value")
		return
	}
}

// sleep for millisecs
func sleepms(m int64) {
	time.Sleep(m * 1000000)
}

func checkLockExists(t *testing.T, l *LockManagerImpl, tran IntObject, lockname IntObject, m LockMode) {
	mode := l.FindLock(tran, lockname)
	if mode != m {
		t.Fatalf("check lock failed: expected %v found %v", m, mode)
		return
	}
}

type Counter struct {
	count int
	m     sync.Mutex
}


func (c *Counter) incr() {
	c.m.Lock()
	c.count++
	c.m.Unlock()
}

func (c *Counter) get() int {
	c.m.Lock()
	n := c.count
	c.m.Unlock()
	return n
}

func (c *Counter) compareAndSet(a, b int) bool {
	ok := false
	c.m.Lock()
	if c.count == a {
		c.count = b
		ok = true
	}
	c.m.Unlock()
	return ok
}

func TestReadAfterWriteLock(t *testing.T) {
	l := NewLockManager()
	var tran1 IntObject = 1
	var tran2 IntObject = 2
	var tran3 IntObject = 3
	var lockname IntObject = 10
	var counter Counter
	l.Acquire(tran1, lockname, EXCLUSIVE, ManualDuration, -1)
	go func() {
		checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
		// We expect this lock to be granted after the exclusive lock is released
		l.Acquire(tran2, lockname, SHARED, ManualDuration, -1)
		checkLockExists(t, l, tran2, lockname, SHARED)
		checkLockExists(t, l, tran1, lockname, NONE)
		l.Release(tran2, lockname, false)
		counter.incr()
	}()
	go func() {
		checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
		// We expect this lock to be granted after the exclusive lock is released
		l.Acquire(tran3, lockname, SHARED, ManualDuration, -1)
		checkLockExists(t, l, tran3, lockname, SHARED)
		checkLockExists(t, l, tran1, lockname, NONE)
		l.Release(tran3, lockname, false)
		counter.incr()
	}()
	sleepms(100)
	checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
	l.Release(tran1, lockname, false)
	sleepms(100)
	if counter.get() != 2 {
		t.Errorf("verification of counter failed: expected %v got %v", 2, counter.get())
	}
}

type IntObject uint32

func (i IntObject) Equals(other Comparable) bool {
	switch j := other.(type) {
	case IntObject:
		return i == j
	default:
		return false
	}
	return false
}

func (i IntObject) Hash() uint32 {
	return uint32(i)
}

func comp(i Lockable, j Lockable) {
	if i == j {
		fmt.Printf("i==j\n")
	} else {
		fmt.Printf("i!=j\n")
	}
}

func BenchmarkContention_3G_2P(b *testing.B) {

	runtime.GOMAXPROCS(2)
	var lock IntObject = 34
	var owner1 IntObject = 33
	var owner2 IntObject = 35
	var owner3 IntObject = 36

	m := NewLockManager()

	c := make(chan bool)
	go func() {
		for n := 0; n < b.N; n++ {
			m.Acquire(owner1, lock, EXCLUSIVE, ManualDuration, 0)
			runtime.Gosched()
			m.Release(owner1, lock, false)
			runtime.Gosched()
		}
		c <- true
	}()
	go func() {
		for n := 0; n < b.N; n++ {
			m.Acquire(owner3, lock, EXCLUSIVE, ManualDuration, 0)
			runtime.Gosched()
			m.Release(owner3, lock, false)
			runtime.Gosched()
		}
		c <- true
	}()
	for n := 0; n < b.N; n++ {
		m.Acquire(owner2, lock, EXCLUSIVE, ManualDuration, 0)
		runtime.Gosched()
		m.Release(owner2, lock, false)
		runtime.Gosched()
	}
	<-c
	<-c
}

func BenchmarkContention_3G_3P(b *testing.B) {

	runtime.GOMAXPROCS(3)
	var lock IntObject = 34
	var owner1 IntObject = 33
	var owner2 IntObject = 35
	var owner3 IntObject = 36

	m := NewLockManager()

	c := make(chan bool)
	go func() {
		for n := 0; n < b.N; n++ {
			m.Acquire(owner1, lock, EXCLUSIVE, ManualDuration, 0)
			runtime.Gosched()
			m.Release(owner1, lock, false)
			runtime.Gosched()
		}
		c <- true
	}()
	go func() {
		for n := 0; n < b.N; n++ {
			m.Acquire(owner3, lock, EXCLUSIVE, ManualDuration, 0)
			runtime.Gosched()
			m.Release(owner3, lock, false)
			runtime.Gosched()
		}
		c <- true
	}()
	for n := 0; n < b.N; n++ {
		m.Acquire(owner2, lock, EXCLUSIVE, ManualDuration, 0)
		runtime.Gosched()
		m.Release(owner2, lock, false)
		runtime.Gosched()
	}
	<-c
	<-c
}

func BenchmarkContention_2G_2P(b *testing.B) {
	runtime.GOMAXPROCS(2)
	var lock IntObject = 34
	var owner1 IntObject = 33
	var owner2 IntObject = 35

	m := NewLockManager()

	c := make(chan bool)
	go func() {
		for n := 0; n < b.N; n++ {
			m.Acquire(owner1, lock, EXCLUSIVE, ManualDuration, 0)
			runtime.Gosched()
			m.Release(owner1, lock, false)
			runtime.Gosched()
		}
		c <- true
	}()
	for n := 0; n < b.N; n++ {
		m.Acquire(owner2, lock, EXCLUSIVE, ManualDuration, 0)
		runtime.Gosched()
		m.Release(owner2, lock, false)
		runtime.Gosched()
	}
	<-c
}

func BenchmarkContention_1G_1P(b *testing.B) {
	runtime.GOMAXPROCS(1)
	var lock IntObject = 34
	var owner2 IntObject = 35

	m := NewLockManager()
	for n := 0; n < b.N; n++ {
		m.Acquire(owner2, lock, EXCLUSIVE, ManualDuration, 0)
		m.Release(owner2, lock, false)
	}
}


/**
 * Simple lock acquire and release tests
 */
func TestBasics(t *testing.T) {
	lockmgr := NewLockManager()
	var tran1 IntObject = 1
	var lockname IntObject = 10

	lockmgr.Acquire(tran1, lockname, EXCLUSIVE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, EXCLUSIVE)
	// assertEquals(1, lockmgr.getLocks(tran1).length)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, SHARED, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, SHARED)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, INTENTION_EXCLUSIVE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, INTENTION_EXCLUSIVE)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, INTENTION_SHARED, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, INTENTION_SHARED)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, SHARED_INTENTION_EXCLUSIVE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, SHARED_INTENTION_EXCLUSIVE)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, UPDATE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, UPDATE)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
	lockmgr.Acquire(tran1, lockname, UPDATE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, UPDATE)
	lockmgr.Acquire(tran1, lockname, UPDATE, CommitDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, UPDATE)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, UPDATE)
	lockmgr.Acquire(tran1, lockname, EXCLUSIVE, ManualDuration, -1)
	checkLockExists(t, lockmgr, tran1, lockname, EXCLUSIVE)
	lockmgr.Release(tran1, lockname, false)
	lockmgr.Release(tran1, lockname, false)
	lockmgr.Release(tran1, lockname, false)
	checkLockExists(t, lockmgr, tran1, lockname, EXCLUSIVE)
	lockmgr.Release(tran1, lockname, true)
	checkLockExists(t, lockmgr, tran1, lockname, NONE)
}

func assertTrue(t *testing.T, value bool) {
	if !value {
		t.Fatalf("assertion failed")
	}
}

func lock1(t *testing.T, lockMgr LockManager, tran IntObject, lockname IntObject, sync *Counter) {
	fmt.Printf("T1(1) locking %v in shared mode\n", lockname)
	lockMgr.Acquire(tran, lockname, SHARED, ManualDuration, 60)
	fmt.Printf("T1(2) acquired lock %v in SHARED mode\n", lockname)
	assertTrue(t, sync.compareAndSet(0, 2))
	sleepms(1500)
	fmt.Printf("T1(6) locking %v in EXCLUSIVE mode (should trigger conversion request and block) ...\n", lockname)
	lockMgr.Acquire(tran, lockname, EXCLUSIVE, ManualDuration, 60)
	fmt.Printf("T1(8) lock %v acquired in EXCLUSIVE mode\n", lockname)
	assertTrue(t, sync.compareAndSet(4, 8))
	sleepms(1000)
	fmt.Printf("T1(10) downgrading to shared mode\n")
	lockMgr.Downgrade(tran, lockname, SHARED)
	fmt.Printf("T1(11) releasing lock\n")
	lockMgr.Release(tran, lockname, false)
	fmt.Printf("T1(12) releasing lock (should grant exclusive to T3)\n")
	lockMgr.Release(tran, lockname, false)
}

func lock2(t *testing.T, lockMgr LockManager, tran IntObject, lockname IntObject, sync *Counter) {
	fmt.Printf("T2(3) locking %v in shared mode\n", lockname)
	lockMgr.Acquire(tran, lockname, SHARED, ManualDuration, 60)
	fmt.Printf("T2(4) acquired lock %v in SHARED mode\n", lockname)
	assertTrue(t, sync.compareAndSet(2, 4))
	sleepms(2000)
	fmt.Printf("T2(7) releasing lock (should grant conversion request T1)\n")
	lockMgr.Release(tran, lockname, false)
	sleepms(500)
	fmt.Printf("T2(9) locking %v in shared mode (should block) ...\n", lockname)
	lockMgr.Acquire(tran, lockname, SHARED, ManualDuration, 60)
	fmt.Printf("T2(15) acquired lock %v in SHARED mode\n", lockname)
	assertTrue(t, sync.compareAndSet(13, 16))
	fmt.Printf("T2(16) releasing lock\n")
	lockMgr.Release(tran, lockname, false)
}

func lock3(t *testing.T, lockMgr LockManager, tran IntObject, lockname IntObject, sync *Counter) {
	fmt.Printf("T3(5) locking %v in exclusive mode (should wait) ...\n", lockname)
	lockMgr.Acquire(tran, lockname, EXCLUSIVE, ManualDuration, 60)
	fmt.Printf("T3(13) acquired lock %v in EXCLUSIVE mode\n", lockname)
	assertTrue(t, sync.compareAndSet(8, 13))
	fmt.Printf("T3(14) releasing lock (should grant shared lock to T2)\n")
	lockMgr.Release(tran, lockname, false)
}

/**
 * A complex test case where various interactions between three threads are
 * tested. The thread interactions are defined in terms of a state machine
 * which is validated.
 */
func TestLocking(t *testing.T) {

	var tran1 IntObject = 1
	var tran2 IntObject = 2
	var tran3 IntObject = 3
	var lockname IntObject = 10
	var sync Counter
	lockmgr := NewLockManager()

	go lock1(t, lockmgr, tran1, lockname, &sync)
	sleepms(500)
	go lock2(t, lockmgr, tran2, lockname, &sync)
	sleepms(500)
	go lock3(t, lockmgr, tran3, lockname, &sync)
	sleepms(3000)
	assertTrue(t, 16 == sync.get())
}
