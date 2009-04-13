#include "dm1/monitor.h"

namespace test_dm1_monitor {

	using namespace dm1;

	Monitor cond;

	class MonitorWaiterThread : public Runnable {
	public:
		void run();
	};

	void MonitorWaiterThread::run()
	{
		cond.lock();
		cond.lock();
		cond.wait();
		printf("MonitorWaiterThread woke up after wait()\n");
		cond.unlock();
		cond.unlock();
	}

	class MonitorTimedWaiterThread : public Runnable {
	public:
		void run();
	};

	void MonitorTimedWaiterThread::run()
	{
		cond.lock();
		cond.wait(5);
		printf("MonitorTimedWaiterThread woke up after wait()\n");
		cond.unlock();
	}

	class MonitorNotifierThread : public Runnable {
	public:
		void run();
	};

	void MonitorNotifierThread::run()
	{
		sleep(15);
		printf("Notifying all threads\n");
		cond.notifyAll();
	}

	int test()
	{
		setbuf(stdout, 0);
		Monitor::setDebug(1);
	
		Thread *t1 = ThreadFactory::createThread(new MonitorWaiterThread(), "Waiter");
		Thread *t2 = ThreadFactory::createThread(new MonitorTimedWaiterThread(), "TimedWaiter");
		Thread *t3 = ThreadFactory::createThread(new MonitorNotifierThread(), "Notifier");

		t1->start();
		sleep(1);
		t2->start();
		t3->start();

		t1->join();
		t2->join();
		t3->join();
		fflush(stdout);
		sleep(10);
		Monitor::setDebug(0);
		return 0;
	}	
}

