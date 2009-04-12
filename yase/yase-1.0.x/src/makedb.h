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
#ifndef makedb_h
#define makedb_h

#include "yase.h"
#include "list.h"
#include "bitfile.h"

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
} ys_mkdb_userargs_t;

extern int 
ys_mkdb_create_database( char *pathname[], ys_mkdb_userargs_t *args );

typedef ys_bool_t ys_pfn_postings_iterator_t(void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf );
extern void 
ys_mkdb_foreachposting( ys_bitfile_t *postings_file, ys_filepos_t offset,
	ys_pfn_postings_iterator_t *pfunc, void *arg );

extern int 
ys_mkdb_get_term_frequency( ys_bitfile_t *postings_file, ys_filepos_t offset,
	ys_doccnt_t *tf );

extern float 
ys_mkdb_calculate_document_weight( ys_mkdb_t *arg, ys_docnum_t docnum );

extern int 
ys_mkdb_create_btree( ys_mkdb_userargs_t *args );

extern ys_list_t * 
ys_mkdb_get_wgetargs( ys_mkdb_t *mkdb );

/* #define CALC_LOGDTF(dtf)		dtf */
#define CALC_LOGDTF(dtf)		(1.0 + log(dtf)) /* MG */
/* #define CALC_LOGDTF(dtf)		log(1.0 + dtf)  */

#define CALC_IDF(N,tf)			log(1.0 + N/tf) /* MG */
/* #define CALC_IDF(N,tf)		(1.0 + log(N/tf)) */

#define CALC_QTW(qtf, qmf, idf) \
		((0.5 + (0.5 * (qtf)/(qmf))) * (idf))

#endif
