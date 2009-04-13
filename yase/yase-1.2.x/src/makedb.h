/***
*    YASE (Yet Another Search Engine)
*    Copyright (C) 2000-2003  Dibyendu Majumdar.
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
#ifndef makedb_h
#define makedb_h

#include "yase.h"
#include "list.h"

#ifdef	__cplusplus
//extern "C" {
#endif

typedef struct ys_mkdb_t ys_mkdb_t;

typedef struct ys_wget_option_t {
	ys_link_t l;
	char option[128];
} ys_wget_option_t;

typedef struct {
	char *rootpath;
	char *dbpath;
	size_t memlimit;
	ys_bool_t stem;
	ys_list_t *wget_opts;
	bool skipBinaryFiles;
} ys_mkdb_userargs_t;

extern int 
ys_mkdb_create_database( char *pathname[], ys_mkdb_userargs_t *args );

extern float 
ys_mkdb_calculate_document_weight( ys_mkdb_t *arg, ys_docnum_t docnum );

extern int 
ys_mkdb_create_btree( ys_mkdb_userargs_t *args );

extern ys_list_t * 
ys_mkdb_get_wgetargs( ys_mkdb_t *mkdb );

extern bool
ys_mkdb_get_skip_binary_files( ys_mkdb_t *mkdb );

extern void
ys_mkdb_set_curdocnum( ys_mkdb_t *mkdb, ys_docnum_t docnum );

/* #define CALC_LOGDTF(dtf)		dtf */
#define CALC_LOGDTF(dtf)		(1.0 + log(dtf)) /* MG */
/* #define CALC_LOGDTF(dtf)		log(1.0 + dtf)  */

#define CALC_IDF(N,tf)			log(1.0 + N/tf) /* MG */
/* #define CALC_IDF(N,tf)		(1.0 + log(N/tf)) */

#define CALC_QTW(qtf, qmf, idf) \
		((0.5 + (0.5 * (qtf)/(qmf))) * (idf))

#ifdef	__cplusplus
//}
#endif

#endif
