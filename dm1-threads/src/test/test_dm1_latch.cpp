#include "dm1/list.h"
#include "dm1/mutex.h"
#include "dm1/thread.h"
#include "dm1/latch.h"
#include "test_dm1.h"

namespace test_dm1_latch {

	using namespace dm1;

	Latch *mylatch;

	class SharedThread : public Runnable {
	public:
		void run();
	};

	void SharedThread::run() 
	{
		int i;
		for (i = 0; i < 100; i++) {
			printf("%s: Requesting latch in shared mode\n", getName());
			mylatch->acquire(Latch::DM1_LATCH_S, Latch::DM1_LATCH_WAIT);
			printf("%s: Acquired latch in shared mode\n", getName());
			msleep(5);
			printf("%s: Freed latch\n", getName());
			mylatch->release();
			msleep(5);
		}
	}

	class ExclusiveThread : public Runnable {
	public:
		void run();
	};

	void ExclusiveThread::run() 
	{
		int i;
		for (i = 0; i < 100; i++) {
			printf("%s: Requesting latch in exclusive mode\n", getName());
			mylatch->acquire(Latch::DM1_LATCH_X, Latch::DM1_LATCH_WAIT);
			printf("%s: Acquired latch in exclusive mode\n", getName());
			msleep(5);
			printf("%s: Freed latch\n", getName());
			mylatch->release();
			msleep(5);
		}
	}

	int test()
	{
		mylatch = new Latch("mylatch");

		Thread *t1 = ThreadFactory::createThread(new SharedThread(), "shared 1");
		Thread *t2 = ThreadFactory::createThread(new ExclusiveThread(), "exclusive");
		Thread *t3 = ThreadFactory::createThread(new SharedThread(), "shared 2");

		t1->start();
		t2->start();
		t3->start();

		t1->join();
		t2->join();
		t3->join();

		Latch::reportStatistics();

		delete mylatch;

		return 0;
	}	
}

