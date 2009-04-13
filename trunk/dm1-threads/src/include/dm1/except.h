/***
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    Author : Dibyendu Majumdar
*    Email  : dibyendu@mazumdar.demon.co.uk
*/
#ifndef dm1_except_h
#define dm1_except_h

#include <stdio.h>

namespace dm1 {

	enum {
		DM1_ERR_UNEXPECTED = 1,
		DM1_ERR_MUTEX_CREATE,
		DM1_ERR_MUTEX_LOCK,
		DM1_ERR_MUTEX_UNLOCK,
		DM1_ERR_EVENT_CREATE,
		DM1_ERR_EVENT_RESET,
		DM1_ERR_EVENT_WAIT,
		DM1_ERR_EVENT_NOTIFY,
		DM1_ERR_MONITOR_STATE,
		DM1_ERR_MONITOR_DESTROYED,
		DM1_ERR_MONITOR_BUSY,
		DM1_ERR_THREAD_CREATE,
		DM1_ERR_THREAD_JOIN,
		DM1_ERR_LATCH_DESTROYED,
		DM1_ERR_LATCH_BUSY,
		DM1_ERR_LATCH_ACQUIRE
	};

	class Exception {
	protected:
		const char *filename;
		int line;
		int category;
		int errorCode;
		const char *type;

	public:
		Exception(const char *filename, int line, int category, int errorCode = 0, 
			const char *type = "Exception") {
			this->filename = filename;
			this->line = line;
			this->category = category;
			this->errorCode = errorCode;
			this->type = type;
		}
		int getCode() const {
			return errorCode;
		}
		int getCategory() const {
			return category;
		}
		const char *getFilename() const {
			return filename;
		}
		int getLine() const {
			return line;
		}
		const char *getType() const {
			return type;
		}
		void dump(FILE *fp = stderr) {
			fprintf(fp, "%s: category %d errcode %d, raised at %s(%d)\n",
				type, category, errorCode, filename, line);
		}
	};

	class ThreadException : public Exception {
	public:
		ThreadException(const char *filename, int line, int category, int errorcode = 0) :
			Exception(filename, line, category, errorcode, "ThreadException") {
		}
	};
}

#endif



