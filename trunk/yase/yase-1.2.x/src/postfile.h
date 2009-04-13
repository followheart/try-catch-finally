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

#ifndef postfile_h
#define postfile_h

#include "cbitfile.h"

typedef ys_filepos_t ys_postoff_t;
typedef ys_bool_t ys_pfn_postings_iterator_t(void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf );

YASE_NS_BEGIN

class PostFile {
private:
	YASENS BitFile bf;
public:
	PostFile();
	~PostFile();
	int open(const char *filename, const char *mode = 0);
	void close();
	ys_postoff_t get_gpos();
	void set_gpos(ys_postoff_t pos);
	ys_postoff_t get_ppos();
	void set_ppos(ys_postoff_t pos);
	void write_docnum(ys_docnum_t docnum);
	ys_docnum_t read_docnum();
	void write_doccnt(ys_doccnt_t doccnt);
	ys_doccnt_t read_doccnt();
	
	/**
	 * Evaluate a user supplied function for each posting starting
	 * from the given offset. The offset must point to the start of
	 * postings for a term.
	 */
	void iterate(ys_postoff_t offset,
		ys_pfn_postings_iterator_t *pfunc, void *arg );
	
	/**
	 * Retrieves a term's frequency from the postings file.
	 * offset must point to the start of the terms postings.
	 */
	ys_doccnt_t get_term_frequency(ys_postoff_t offset);
	
	void flush();
};

YASE_NS_END

inline 
YASENS PostFile::PostFile() {}

inline
YASENS PostFile::~PostFile() {}

inline int
YASENS PostFile::open(const char *filename, const char *mode)
{
	return bf.open(filename, mode);
}

inline void
YASENS PostFile::close() 
{
	bf.close();
}

inline ys_postoff_t 
YASENS PostFile::get_gpos()
{
	return (ys_postoff_t) bf.getgpos();
}

inline void 
YASENS PostFile::set_gpos(ys_postoff_t pos)
{
	bf.setgpos((ys_filepos_t) pos);
}

inline ys_postoff_t 
YASENS PostFile::get_ppos()
{
	return (ys_postoff_t) bf.getppos();
}

inline void 
YASENS PostFile::set_ppos(ys_postoff_t pos)
{
	bf.setppos((ys_filepos_t) pos);
}

inline void 
YASENS PostFile::write_docnum(ys_docnum_t docnum)
{
	bf.gammaEncode((ys_uint32_t) docnum+1);
}

inline ys_docnum_t 
YASENS PostFile::read_docnum()
{
	return (ys_docnum_t) bf.gammaDecode()-1;
}

inline void 
YASENS PostFile::write_doccnt(ys_doccnt_t doccnt)
{
	bf.gammaEncode((ys_uint32_t) doccnt+1);
}

inline ys_docnum_t 
YASENS PostFile::read_doccnt()
{
	return (ys_doccnt_t) bf.gammaDecode()-1;
}

inline void
YASENS PostFile::flush()
{
	bf.flush();
}

#endif
