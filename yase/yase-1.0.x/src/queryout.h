/***
*    YASE (Yet Another Search Engine)
*    Copyright (C) 2000-2002  Dibyendu Majumdar.
*
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
*    Website: www.mazumdar.demon.co.uk/yase_index.html
*/

/* created 16-Jan 2000 */

#ifndef queryout_h
#define queryout_h

#include "yase.h"
#include "query.h"
#include "docdb.h"

typedef enum {
	YS_QRY_MSG_ERROR = 1,
	YS_QRY_MSG_INFO = 2
} ys_qry_msg_t; 

extern void * ys_query_output_open(int argc, char *argv[], 
	ys_bool_t web_query, ys_query_t *query);
extern int ys_query_output_header(void *handle);
extern void ys_query_output_row(void *handle, const char *reference,
	const char *anchor, ys_docdata_t *docfile,
	ys_docdata_t *doc, float rank, int matchcount);
extern void ys_query_output_message(void *handle, ys_qry_msg_t type, 
	const char *fmt, ...);
extern int ys_query_output_summary(void *handle, int matches, int curpage,
	int pagecount, int pagesize, double elapsed_time);
extern int ys_query_output_close(void *handle);

#endif
