VERSION = 1.0.3
TEMPLATE = lib
CONFIG = thread debug warn_on dll
HEADERS = common.h \
	event.h \
	except.h \
	list.h \
	thread.h \
	monitor.h \
	threadpool.h \
	port.h \
	mutex.h \
	latch.h
INCLUDEPATH = ../../include
SOURCES = event.cpp \
	list.cpp \
	latch.cpp \
	monitor.cpp \
	thread.cpp \
	threadpool.cpp
DESTDIR = ../../lib
DEPENDPATH = ../../include/dm1
