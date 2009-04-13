#include "test_dm1.h"
#include "dm1/threadpool.h"

namespace test_dm1_threadpool {

	using namespace dm1;

	class MyTask : public Task {
		int id;

	public:
		MyTask(int id) {
			this->id = id;
		}

		void run() {
			Thread *t = Thread::getCurrentThread();
			fprintf(stdout, "Running task %d under thread %s (%p)\n", id, t->getName(), t);
			sleep(10);
		}

		~MyTask() {
			fprintf(stdout, "Task %d destroyed\n", id);
		}
	};

	int test() {

		setbuf(stdout, 0);
	
		ThreadPool::setDebug(1);
		ThreadFactory::setDebug(1);
	
		ThreadPool tp(5, 2, 2, 10, 3);
		int i;

		tp.start();
		for (i = 1; i <= 25; i++) {
			fprintf(stdout, "Queueing task %d\n", i);
			tp.queueTask(new MyTask(i));
			if ((i % 10) == 0) 
				sleep(30);
		}
		sleep(15);
		tp.shutdown();

		return 0;
	}
}

