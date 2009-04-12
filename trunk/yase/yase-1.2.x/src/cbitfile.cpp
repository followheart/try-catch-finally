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

// 29 Nov 2002

#include "cbitfile.h"

int YASENS BitFile::debug = 1;

void
YASENS BitFile::dumpBits(ys_bits_t v, FILE *fp)
{
	ys_bits_t mask = MASK_VALUE;
	int nbits = BITS;
	while (nbits > 0) {
		if ((v & mask) != 0)
			fputc('1', fp);
		else
			fputc('0', fp);
		mask >>= 1;
		nbits--;
	}
	fputc('\n', fp);
}

YASENS BitFile::BitFile()
{
	pch = 0;
	gch = 0;
	pmask = 0;
	gmask = 0;
	pnbit = 0;
	gnbit = 0;
	ppos = 0;
	gpos = 0;

	file = 0;
}

YASENS BitFile::~BitFile()
{
	close();
}

int 
YASENS BitFile::open(const char *filename, const char *mode)
{
	const char *omode = "rb+";
	if ( mode != 0 ) omode = mode;
	file = ys_file_open( filename, omode, mode == 0 ? 0 : YS_FILE_ABORT_ON_ERROR );
	if ( file == 0 && mode == 0 ) {
		file = ys_file_open( filename, "wb+", 
			mode == 0 ? 0 : YS_FILE_ABORT_ON_ERROR );
	}
	if ( file == 0 ) {
		fprintf(stderr, "Error opening file %s\n", filename);
		return -1;
	}
	resetg();
	resetp();
	gpos = 0;
	ppos = 0;
	return 0;
}

void 
YASENS BitFile::resetg() 
{
	gch = 0;
	gnbit = 0;
	gmask = MASK_VALUE;
}

void 
YASENS BitFile::resetp() 
{
	pch = 0;
	pnbit = BITS;
	pmask = MASK_VALUE;
}

void 
YASENS BitFile::flush()
{
	if ( pnbit != BITS ) {
		if (debug) {
			ys_filepos_t pos;
			ys_file_getpos(file, &pos);
			assert(pos == ppos);
		}
		// ys_file_setpos(file, &ppos);
		//file.seek(ppos);
		ys_file_write( (const void *) &pch, 1, sizeof pch, file );
		ppos += 4;
		resetp();
	}
}

void 
YASENS BitFile::fill()
{
	resetg();
	if (!ys_file_eof(file)) {
		if (debug) {
			ys_filepos_t pos;
			ys_file_getpos(file, &pos);
			assert(pos == gpos);
		}
		//file.seek( gpos );
		ys_file_read( (void *) &gch, 1, sizeof gch, file );
		gnbit = BITS;
		gpos += 4;
	}
}

void 
YASENS BitFile::close()
{
	if (file != 0) {
		ys_file_setpos(file, &ppos);
		flush();
		ys_file_close(file);
		file = 0;
	}
}


bool
YASENS BitFile::geof() const
{
	return gnbit == 0 && ys_file_eof(file);
}

void 
YASENS BitFile::setgpos( ys_filepos_t pos )
{
	resetg();
	gpos = pos;
	ys_file_setpos(file, &pos);
}

ys_filepos_t
YASENS BitFile::getgpos() const
{
	return gpos;
}

void 
YASENS BitFile::setppos( ys_filepos_t pos )
{
	resetp();
	ppos = pos;
	ys_file_setpos(file, &pos);
}

ys_filepos_t
YASENS BitFile::getppos() const
{
	return ppos;
}

#ifdef TEST_CBITFILE

YASE_NS_USING

int main()
{
	remove("test");
	BitFile bf;
	char obits[] = "10101010101010101010101010110101010101010101010101010" 
		"11111111111111000000000000001111100001111111111100000" 
		"00000000001111111111111111110101010111010101100010101";

	char *ibits = new char[sizeof obits];
	int i;

	bf.open("test", 0);
	bf.setppos(0);
	for (i = 0; i < sizeof obits; i++) {
  		bf.putBit( obits[i] == '1' ? 1 : 0 );
	}
  	bf.flush();
	bf.setgpos(0);
  	for (i = 0; i < sizeof obits-1; i++) {
  		ys_bits_t bit = bf.getBit();
  		ibits[i] = (bit != 0) ? '1' : '0';
  	}
	ibits[i] = 0;
	if (strncmp(obits, ibits, sizeof obits) != 0 ) {
		fprintf(stderr, "Test failed !\n");
		exit(1);
	}

	i = 0;
	bf.setgpos(0);
	while (!bf.geof()) {
		ys_bits_t bit = bf.getBit();
		if (i < sizeof obits)
			ibits[i] = (bit != 0) ? '1' : '0';
		i++;
    	}
	bf.close();

	remove("test");
	bf.open("test", 0);
	bf.setppos(5);
	for (i = 1; i <= 1000; i++) {
		bf.gammaEncode(i);
	}
	bf.flush();
	bf.setgpos(5);
	for (i = 1; i <= 1000; i++) {
		int j = bf.gammaDecode();
		if (j != i) {
			fprintf(stderr, "Expected %d, read %d\n", i, j);
			fprintf(stderr, "Test failed !\n");
			exit(1);
		}
	}
	bf.close();
	printf("gamma test ok\n");
	remove("test");

	bf.open("test", 0);
	bf.setppos(5);
	for (i = 1; i <= 1000; i++) {
		bf.gammaEncode64(i);
	}
	bf.flush();
	bf.setgpos(5);
	for (i = 1; i <= 1000; i++) {
		int j = (int)bf.gammaDecode64();
		if (j != i) {
			fprintf(stderr, "Expected %d, read %d\n", i, j);
			fprintf(stderr, "Test failed !\n");
			exit(1);
		}
	}
	bf.close();
	printf("64 bit gamma test ok\n");
	remove("test");
	
	return 0;
}

#endif

