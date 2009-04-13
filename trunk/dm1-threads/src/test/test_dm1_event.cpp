#include "dm1/event.h"
#include "dm1/thread.h"
#include "test_dm1.h"

namespace test_dm1_event {

	using namespace dm1;

	Event *event;
	int n;

	class EventWaiterThread : public Runnable {
	public:
		void run();
	};

	void EventWaiterThread::run()
	{
		Event *event = (Event *)getArg();
		fprintf(stderr, "%s: Waiting for event\n", getName());
		event->wait();
		n--;
		fprintf(stderr, "%s: Event wait completed\n", getName());
	}

	class EventNotifierThread : public Runnable {
	public:
		void run();
	};

	void EventNotifierThread::run()
	{
		Event *event = (Event *)getArg();
		wait(5);
		fprintf(stderr, "%s: Signaling event\n", getName());
		event->notify();
	}

	int test()
	{
		Event::setDebug(1);
	
		Thread *t1, *t2, *t3;

		n = 2;
		event = new Event(Event::DM1_EVENT_SINGLE);
	
		t1 = ThreadFactory::createThread(new EventWaiterThread(), "event_waiter1");
		t2 = ThreadFactory::createThread(new EventWaiterThread(), "event_waiter2");
		t3 = ThreadFactory::createThread(new EventNotifierThread(), "event_signal");
		t1->setArg((void *) event);
		t2->setArg((void *) event);
		t3->setArg((void *) event);

		t1->start();
		t2->start();
		t3->start();

		t3->join();
		sleep(5);
		assert(n == 1);
		fprintf(stderr, "main: Signaling event to wakeup second waiter\n");
		event->notify();
		t1->join();
		t2->join();

		assert(n == 0);
		delete event;

		fprintf(stderr, "Starting test of Broadcast Event\n");
		n = 2;
		event = new Event(Event::DM1_EVENT_BROADCAST);

		t1 = ThreadFactory::createThread(new EventWaiterThread(), "event_waiter1");
		t2 = ThreadFactory::createThread(new EventWaiterThread(), "event_waiter2");
		t3 = ThreadFactory::createThread(new EventNotifierThread(), "event_signal");
		t1->setArg((void *) event);
		t2->setArg((void *) event);
		t3->setArg((void *) event);

		t1->start();
		t2->start();
		t3->start();

		t3->join();
		t1->join();
		t2->join();

		assert(n == 0);
		delete event;

		Event::setDebug(0);

		return 0;
	}	
}

