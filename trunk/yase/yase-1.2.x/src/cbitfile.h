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

#ifndef cbitfile_h
#define cbitfile_h

#include "yase.h"
#include "ystdio.h"

typedef ys_uint32_t ys_bits_t;

YASE_NS_BEGIN

class BitFile {

private:
	ys_bits_t pch;
	ys_bits_t gch;
	ys_bits_t pmask;
	ys_bits_t gmask;
	ys_uint16_t pnbit;
	ys_uint16_t gnbit;
	ys_filepos_t ppos;
	ys_filepos_t gpos;

	ys_file_t *file;

	enum {
		BITS = sizeof(ys_bits_t)*8,
		MASK_VALUE = 1 << (BITS-1)
	};

private:
	static int debug;

private:
	void resetg();
	void resetp();
	void fill();

public:
	BitFile();
	~BitFile();
	int open(const char *filename, const char *mode = 0);
	void putBit(ys_bits_t bit);
	ys_bits_t getBit();
	void flush();
	bool geof() const;
	void close();
	ys_filepos_t getppos() const;
	void setppos(ys_filepos_t pos);
	ys_filepos_t getgpos() const;
	void setgpos(ys_filepos_t pos);
	void gammaEncode(ys_uint32_t x);
	ys_uint32_t gammaDecode();
	void gammaEncode64(ys_uint64_t x);
	ys_uint64_t gammaDecode64();

	static void dumpBits(ys_bits_t v, FILE *fp);
	static void setDebug(int value) { debug = value; }
};

YASE_NS_END 

inline void 
YASENS BitFile::putBit( ys_bits_t bit )
{
	if (pnbit == 0)
		flush();
	if (bit != 0)
		pch |= pmask;
	else
		pch &= ~pmask;
	pmask >>= 1;
	pnbit--;
}

inline ys_bits_t
YASENS BitFile::getBit()
{
	ys_bits_t bit;
	if ( gnbit == 0 )
		fill();
	bit = ((gch & gmask) != 0) ? 1 : 0;
	gmask >>= 1;
	gnbit--;
	return bit;
}

inline void 
YASENS BitFile::gammaEncode(ys_uint32_t x)
{
	assert(x > 0);
	ys_uint32_t x1 = x;
	int logx;
	for (logx = -1; x1 != 0; x1 >>= 1, logx++)
		;
	int nbits = logx+1;
	while ( logx-- > 0 )
		putBit(0);
	while ( --nbits >= 0 )
		putBit( (x >> nbits) & 0x1 );
}

inline ys_uint32_t 
YASENS BitFile::gammaDecode()
{
	int nbits = 0;
	ys_bits_t bit = 0;
	for (;;) {
		bit = getBit();
		if (bit == 0)
			nbits++;
		else
			break;
	}
	ys_uint32_t x = 1;
	while ( nbits-- > 0 ) {
		bit = getBit();
		x += x + bit;
	}
	return x;
}

inline void 
YASENS BitFile::gammaEncode64(ys_uint64_t x)
{
	assert(x > 0);
	ys_uint64_t x1 = x;
	int logx;
	for (logx = -1; x1 != 0; x1 >>= 1, logx++)
		;
	int nbits = logx+1;
	while ( logx-- > 0 )
		putBit(0);
	while ( --nbits >= 0 )
		if ((x >> nbits) & 0x1)
			putBit(1);
		else
			putBit(0);
}

inline ys_uint64_t 
YASENS BitFile::gammaDecode64()
{
	int nbits = 0;
	ys_bits_t bit = 0;
	for (;;) {
		bit = getBit();
		if (bit == 0)
			nbits++;
		else
			break;
	}
	ys_uint64_t x = 1;
	while ( nbits-- > 0 ) {
		bit = getBit();
		x += x + bit;
	}
	return x;
}

#endif

