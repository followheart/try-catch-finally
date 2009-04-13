#ifndef test_dm1_h
#define test_dm1_h

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace test_dm1_event {
	extern int test();
}
namespace test_dm1_latch {
	extern int test();
}
namespace test_dm1_monitor {
	extern int test();
}
namespace test_dm1_monitor2 {
	extern int test();
}
namespace test_dm1_monitor3 {
	extern int test();
}
namespace test_dm1_monitor4 {
	extern int test();
}
namespace test_dm1_thread {
	extern int test();
}
namespace test_dm1_threadpool {
	extern int test();
}

#endif

