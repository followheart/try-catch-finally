#include "test_dm1.h"
#include "dm1/thread.h"

namespace test_dm1_thread {

	using namespace dm1;

	class Sleeper : public Runnable {
	public:
		void run();
	};

	void Sleeper::run()
	{
		fprintf(stderr, "%s: Sleeping\n", getName());
		wait();
		fprintf(stderr, "%s: Woken up !\n", getName());
	}

	class Waker : public Runnable {
	public:
		void run();
	};

	void Waker::run()
	{
		Thread *other = (Thread *) getArg();

		wait(5);
		fprintf(stderr, "%s: Waking up %s\n", getName(),
			other->getName());
		Thread::notify(other);
		exit();
	}

	class Main : public Runnable {
	public:
		Main();
		~Main();
		void run();
	};

	void Main::run() 
	{
		fprintf(stderr, "Current Thread name = %s\n", Thread::getCurrentThread()->getName());

		Thread *t1 = ThreadFactory::createThread(new Sleeper(), "sleeper");
		Thread *t2 = ThreadFactory::createThread(new Waker(), "waker");

		t2->setArg((void *)t1);

		t1->start();
		t2->start();

		t1->join();
		t2->join();
	}	

	Main::Main() {
		fprintf(stdout, "Creating Main\n");
		Thread *t = ThreadFactory::createThread(this, "main", true);
		t->start();
	}

	Main::~Main() {
		fprintf(stdout, "Destroying Main\n");
	}

	int test()
	{
		printf("in %s thread\n", Thread::getCurrentThread()->getName());
		Thread::setDebug(1);
		ThreadFactory::setDebug(1);

		new Main();

		return 0;
	}
}

