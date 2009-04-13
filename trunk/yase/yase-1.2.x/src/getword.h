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
#ifndef getword_h
#define getword_h

#include "yase.h"
#include "docdb.h"
#include "makedb.h"

typedef int ys_getword_opts_t;
	
typedef int (*ys_pfn_index_t)(ys_mkdb_t *arg, ys_uchar_t *word, ys_docnum_t docnum);
extern int ys_document_process(const char *logicalname, const char *physicalname, 
	ys_docdb_t *docdb, ys_pfn_index_t ptrfunc, ys_mkdb_t *arg) ;

#endif
