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
// 04-12-02: Created - represents the postings file.

#include "postfile.h"

ys_doccnt_t
YASENS PostFile::get_term_frequency(ys_postoff_t offset)
{
	set_gpos( offset );
	return read_doccnt();
}

void 
YASENS PostFile::iterate(ys_postoff_t offset,
		ys_pfn_postings_iterator_t *pfunc, void *arg )
{
	ys_doccnt_t tf;
	ys_docnum_t docnum, n;	
	ys_doccnt_t dtf;
	ys_doccnt_t i;

	tf = get_term_frequency(offset);
#if _DUMP_POSTINGS
	printf("(%lu ", tf);
#endif
	for ( n = 0, i = 0; i < tf; i++ ) {
		docnum = read_docnum();
		dtf = read_doccnt();
#if _DUMP_POSTINGS
		printf("<%lu,%lu>", docnum, dtf); 
#endif
		n += docnum;
		if (! pfunc( arg, n, dtf ) )
			break;
	}	
#if _DUMP_POSTINGS
	printf(")\n"); 
#endif
}

