package lockmgr

import (
	"testing"
)

func TestSimpleLock(t *testing.T) {
        var j IntObject = 33
        var lockOwner IntObject = 34
        m := NewLockManager()

        ok, err := m.Acquire(lockOwner, j, EXCLUSIVE, MANUAL_DURATION, 0)
	if !ok || err != nil {
		t.Errorf("Failed to acquire lock")
		return
	}
	mode := m.FindLock(kickOwner, j)
	if mode != EXCLUSIVE {
		t.Errorf("Failed to find lock")
		return
	}
        ok, err = m.Release(lockOwner, j, false)
	if !ok || err != nil {
		t.Errorf("Failed to release lock")
		return
	}
	mode := m.FindLock(kickOwner, j)
	if mode != NONE {
		t.Errorf("Found returned unexpected value")
		return
	}
}

// sleep for millisecs
func sleepms(m int64) {
	time.Sleep(millisecs * 1000000)
}

func checkLockExists(t *testing.T, l *LockManagerImpl, tran IntObject, lockname IntObject, m LockMode) {
	mode := l.FindLock(tran, lockname)
	if mode != m {
		t.Fatalf("check lock failed: expected %v found %v", m, mode)
		return
	}
}

func TestReadAfterWriteLock(t *testing.T) {
	l := NewLockManager()
	var tran1 IntObject = 1
	var tran2 IntObject = 2
	var tran3 IntObject = 3
	var lockname InObject = 10
	counter := AtomicInt{0}
	l.Acquire(tran1, lockname, EXCLUSIVE, MANUAL_DURATION, -1)
	go t1() {
		checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
		// We expect this lock to be granted after the exclusive lock is released
		l.Acquire(tran2, lockname, SHARED, MANUAL_DURATION, -1)
		checkLockExists(t, l, tran2, lockname, SHARED)
		checkLockExists(t, l, tran1, lockname, NONE)
		l.Release(tran2, lockname)
		counter.Incr()
	}()
	go t2() {
		checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
		// We expect this lock to be granted after the exclusive lock is released
		l.Acquire(tran3, lockname, SHARED, MANUAL_DURATION, -1)
		checkLockExists(t, l, tran3, lockname, SHARED)
		checkLockExists(t, l, tran1, lockname, NONE)
		l.Release(tran3, lockname)
		counter.Incr()
	}()
	sleepms(100)
	checkLockExists(t, l, tran1, lockname, EXCLUSIVE)
	l.Release(tran1, lockname)
	sleepms(100)
	if counter.get() != 2 {
		t.Errorf("verification of counter failed: expected %v got %v", 2, counter.get())
	}			
}
