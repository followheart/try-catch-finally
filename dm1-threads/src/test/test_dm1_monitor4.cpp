#include "dm1/monitor.h"

namespace test_dm1_monitor4 {

	using namespace dm1;

	Monitor * cond;

	class MonitorWaiter : public Runnable {
	public:
		void run();
	};

	void MonitorWaiter::run()
	{
		cond->lock();
		cond->wait();
		printf("MonitorWaiter woke up after wait()\n");
		cond->unlock();
	}

	int test()
	{
		setbuf(stdout, 0);
		Monitor::setDebug(1);
		Thread::setDebug(1);

		cond = new Monitor();

		Thread *t1 = ThreadFactory::createThread(new MonitorWaiter(), "Waiter1", true);
		Thread *t2 = ThreadFactory::createThread(new MonitorWaiter(), "Waiter2", true);

		t1->start();
		t2->start();

		sleep(1);
		delete cond;
		fprintf(stderr, "After destruction of Monitor\n");

		return 0;
	}	
}

