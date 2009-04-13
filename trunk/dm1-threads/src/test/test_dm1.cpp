#include "test_dm1.h"

int main() {

	test_dm1_event::test();
	test_dm1_monitor::test();
	test_dm1_monitor2::test();
	test_dm1_monitor3::test();
	//test_dm1_monitor4::test();	// Will abort
	test_dm1_latch::test();
	test_dm1_thread::test();
	test_dm1_threadpool::test();

	return 0;
}

