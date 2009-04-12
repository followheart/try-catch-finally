/***
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 2000-2002   Dibyendu Majumdar.
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

/* most of the code here is based upon code in mg 1.2.1:
 * bitio_m.h -- Macros for bitio
 * Copyright (C) 1994  Neil Sharman and Alistair Moffat
 */

#include "compress.h"

static inline ys_intmax_t 
ys_floorlog2(ys_uintmax_t x)
{
	ys_intmax_t y;
	for (y = -1; x != 0; x>>=1, y++);
	return y;
}

static inline ys_intmax_t 
ys_ceillog2(ys_uintmax_t x)
{
	ys_intmax_t y;
	for (y = 0, x = x-1; x != 0; x>>=1, y++);
	return y;
}

int 
ys_unary_encode(ys_bitfile_t *bf, ys_uintmax_t x)
{
	assert(x > 0);
	while (--x) {
		if (ys_bputbit(bf, 0) != 0) {
			return -1;
		}
	}
	if (ys_bputbit(bf, 1) != 0)
		return -1;
	return 0;
}

int 
ys_unary_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr)
{
	ys_uintmax_t x = 1;
	for (;;) {
		ys_bit_t bit;
		if (ys_bgetbit(bf, &bit) <= EOF)
			return -1;
		if (bit == 0) 
			x++;
		else
			break;
	}
	*x_ptr = x;
	return 0;
}

int 
ys_gamma_encode(ys_bitfile_t *bf, ys_uintmax_t x)
{
	ys_intmax_t logx;
	ys_intmax_t nbits;

	assert(x > 0);
	logx = ys_floorlog2(x);
	nbits = logx+1;
	while (logx--) {
		if (ys_bputbit(bf, 0) != 0)
			return -1;
	}
	while (--nbits>=0) {
		if (ys_bputbit(bf, (x>>nbits)&0x1) != 0)
			return -1; 
	}
	return 0;
}

#ifndef _INLINE_MAX
int 
ys_gamma_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr)
{
	ys_uintmax_t x = 1;
	register ys_intmax_t nbits = 0;
	ys_bit_t bit;

	for (;;) {
		if (ys_bgetbit(bf, &bit) <= EOF)
			return -1;
		if (bit == 0)
			nbits++;
		else
			break;
	}
	while ( nbits-- > 0 ) {
		if (ys_bgetbit(bf, &bit) <= EOF)
			return -1;
		x += x + bit;
	}
	*x_ptr = x;
	return 0;
}
#endif

int 
ys_binary_encode(ys_bitfile_t *bf, ys_uintmax_t x,
	ys_uintmax_t b) 
{
	ys_intmax_t nbits, logofb, thresh;

	assert(x > 0);
	logofb = ys_ceillog2(b);
	thresh = (1<<logofb) - b;
	if (--x < thresh)
		nbits = logofb-1;
	else
	{
		nbits = logofb;
		x += thresh;
	}
	while (--nbits>=0)
		if (ys_bputbit(bf, ((x>>nbits)&0x1)) != 0)
			return -1;
	return 0;
}

int 
ys_binary_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr, ys_uintmax_t b) 
{
	ys_uintmax_t x = 0;
	ys_intmax_t i, logofb, thresh;
	ys_bit_t bit;

	if (b != 1)
	{	
		logofb = ys_ceillog2(b);
		thresh = (1<<logofb) - b;
		logofb--;
		for (i=0; i < logofb; i++) {
			if (ys_bgetbit(bf, &bit) <= EOF)
				return -1;
			x += x + bit;
		}
		if (x >= thresh)
		{
			if (ys_bgetbit(bf, &bit) <= EOF)
				return -1;
			x += x + bit;
			x -= thresh;
		}
		x = x+1;
	}
	else
		x = 1;
	*x_ptr = x;
	return 0;
}

int 
ys_delta_encode(ys_bitfile_t *bf, ys_uintmax_t x)
{
	ys_intmax_t logx;

	logx = ys_floorlog2(x);
	if (ys_gamma_encode(bf, logx+1) != 0 ||
	    ys_binary_encode(bf, x+1, 1<<logx) != 0)
		return -1;
	return 0;
}

int 
ys_delta_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr)
{
	ys_intmax_t logx;
	ys_uintmax_t x;
	if (ys_gamma_decode(bf, &x) != 0)
		return -1;
	logx = x-1;
	if (ys_binary_decode(bf, &x, 1<<logx) != 0)
		return -1;
	x -= 1;
	x = x + (1<<logx);
	*x_ptr = x;
	return 0;
}

#ifdef TEST_COMPRESS

int main()
{
	ys_uintmax_t x = 9;
	ys_uintmax_t y;
	ys_bitfile_t *bf;
	int ia[] = { 1, 2, 1, 20, 5, 6, 1, 15, 100, 1, 0 };
	
	assert(ys_floorlog2(x) == 3);
	assert(ys_ceillog2(x) == 4);

	bf = ys_bopen( "unarytest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_unary_encode(bf, ia[x]);
	}
	ys_bclose(bf);		
	bf = ys_bopen( "unarytest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_unary_decode(bf, &y);
		assert(y == ia[x]);
	}
	ys_bclose(bf);		
	remove("unarytest");

	bf = ys_bopen( "gammatest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_gamma_encode(bf, ia[x]);
	}
	ys_bclose(bf);		
	bf = ys_bopen( "gammatest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_gamma_decode(bf, &y);
		assert(y == ia[x]);
	}
	ys_bclose(bf);		
	remove("gammatest");
			
	bf = ys_bopen( "deltatest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_delta_encode(bf, ia[x]);
	}
	ys_bclose(bf);		
	bf = ys_bopen( "deltatest", 0 );
	for (x = 0; ia[x] != 0; x++) {
		ys_delta_decode(bf, &y);
		assert(y == ia[x]);
	}
	ys_bclose(bf);		
	remove("deltatest");
	fprintf(stdout, "Test completed OK\n");

	return 0;
}

#endif

#if 0
/**************************************************************************
 *
 * bitio_m.h -- Macros for bitio
 * Copyright (C) 1994  Neil Sharman and Alistair Moffat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 *
 **************************************************************************/


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef BIO_ENCODE_PROLOGUE
#define BIO_ENCODE_PROLOGUE
#endif

#ifndef BIO_DECODE_PROLOGUE
#define BIO_DECODE_PROLOGUE
#endif

#ifndef BIO_ENCODE_EPILOGUE
#define BIO_ENCODE_EPILOGUE
#endif

#ifndef BIO_DECODE_EPILOGUE
#define BIO_DECODE_EPILOGUE
#endif

#ifndef DECODE_ADD
#define DECODE_ADD(b) (b) += (b) + DECODE_BIT
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define POSITIVE(f, x)							\
  if ((x)<=0) 								\
    fprintf(stderr,"Error: Cannot "#f" encode %lu\n",(unsigned long)x),exit(1);


#define CEILLOG_2(x,v)							\
do {									\
  register int _B_x  = (x) - 1;						\
  (v) = 0;								\
  for (; _B_x ; _B_x>>=1, (v)++);					\
} while(0)


#define FLOORLOG_2(x,v)							\
do {									\
  register int _B_x  = (x);						\
  (v) = -1;								\
  for (; _B_x ; _B_x>>=1, (v)++);					\
} while(0)


/****************************************************************************/



#define UNARY_ENCODE(x)							\
do {									\
  register unsigned long _B_x = (x);					\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(unary, _B_x);						\
  while(--_B_x) ENCODE_BIT(0);						\
  ENCODE_BIT(1);							\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define UNARY_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(unary, _B_x);						\
  (count) += _B_x;							\
  while(--_B_x) ENCODE_BIT(0);						\
  ENCODE_BIT(1);							\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define UNARY_DECODE(x)							\
do {									\
  BIO_DECODE_PROLOGUE;							\
  (x) = 1;								\
  while (!DECODE_BIT) (x)++;						\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define UNARY_DECODE_L(x, count)					\
do {									\
  BIO_DECODE_PROLOGUE;							\
  (x) = 1;								\
  while (!DECODE_BIT) (x)++;						\
  (count) += (x);							\
  BIO_DECODE_EPILOGUE;							\
} while(0)


#define UNARY_LENGTH(x, count)						\
do {									\
  POSITIVE(unary, x);							\
  (count) = (x);							\
} while(0)


/****************************************************************************/


#define BINARY_ENCODE(x, b)						\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    {									\
      _B_nbits = _B_logofb;						\
      _B_x += _B_thresh;						\
    }									\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_x>>_B_nbits) & 0x1);					\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define BINARY_ENCODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    {									\
      _B_nbits = _B_logofb;						\
      _B_x += _B_thresh;						\
    }									\
  (count) += _B_nbits;							\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_x>>_B_nbits) & 0x1);					\
  BIO_ENCODE_EPILOGUE;							\
} while(0)


#define BINARY_DECODE(x, b)						\
do {									\
  register unsigned long _B_x = 0;					\
  register unsigned long _B_b = (b);					\
  register int _B_i, _B_logofb, _B_thresh;				\
  BIO_DECODE_PROLOGUE;							\
  if (_B_b != 1)							\
    {									\
      CEILLOG_2(_B_b, _B_logofb);					\
      _B_thresh = (1<<_B_logofb) - _B_b;				\
      _B_logofb--;							\
      for (_B_i=0; _B_i < _B_logofb; _B_i++)			 	\
        DECODE_ADD(_B_x);						\
      if (_B_x >= _B_thresh)						\
        {								\
          DECODE_ADD(_B_x);						\
          _B_x -= _B_thresh;						\
	}								\
      (x) = _B_x+1;							\
    }									\
  else									\
    (x) = 1;								\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define BINARY_DECODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x = 0;					\
  register unsigned long _B_b = (b);					\
  register int _B_i, _B_logofb, _B_thresh;				\
  BIO_DECODE_PROLOGUE;							\
  if (_B_b != 1)							\
    {									\
      CEILLOG_2(_B_b, _B_logofb);					\
      _B_thresh = (1<<_B_logofb) - _B_b;				\
      _B_logofb--;							\
      (count) += _B_logofb;					       	\
      for (_B_i=0; _B_i < _B_logofb; _B_i++)			 	\
        DECODE_ADD(_B_x);						\
      if (_B_x >= _B_thresh)						\
        {								\
          DECODE_ADD(_B_x);						\
          _B_x -= _B_thresh;						\
          (count)++;							\
	}								\
      (x) = _B_x+1;							\
    }									\
  else									\
    (x) = 1;								\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define BINARY_LENGTH(x, b, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    _B_nbits = _B_logofb;						\
  (count) = _B_nbits;							\
} while(0)

/****************************************************************************/




#define GAMMA_ENCODE(x)							\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  register int _B_nbits;						\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  _B_nbits = _B_logofb+1;						\
  while(_B_logofb--) ENCODE_BIT(0);					\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_xx>>_B_nbits) & 0x1);				\
  BIO_ENCODE_EPILOGUE;							\
} while (0)

#define GAMMA_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  register int _B_nbits;						\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  _B_nbits = _B_logofb+1;						\
  (count) += _B_logofb*2+1;						\
  while(_B_logofb--) ENCODE_BIT(0);					\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_xx>>_B_nbits) & 0x1);				\
  BIO_ENCODE_EPILOGUE;							\
} while (0)

#define GAMMA_DECODE(x)							\
do {									\
  register unsigned long _B_xx = 1;					\
  register int _B_nbits = 0;						\
  BIO_DECODE_PROLOGUE;							\
  while(!DECODE_BIT) _B_nbits++;					\
  while (_B_nbits-- > 0)						\
    DECODE_ADD(_B_xx);							\
  (x) = _B_xx;								\
  BIO_DECODE_EPILOGUE;							\
} while (0)

#define GAMMA_DECODE_L(x, count)					\
do {									\
  register unsigned long _B_xx = 1;					\
  register int _B_nbits = 0;						\
  BIO_DECODE_PROLOGUE;							\
  while(!DECODE_BIT) _B_nbits++;					\
  (count) += _B_nbits*2+1;						\
  while (_B_nbits-- > 0)						\
    DECODE_ADD(_B_xx);							\
  (x) = _B_xx;								\
  BIO_DECODE_EPILOGUE;							\
} while (0)

#define GAMMA_LENGTH(x, count)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  (count) = _B_logofb*2+1;						\
} while (0)



/****************************************************************************/


#define DELTA_ENCODE(x)							\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx;							\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_ENCODE(_B_logx+1);						\
  BINARY_ENCODE(_B_xxx+1, 1<<_B_logx);					\
} while (0)

#define DELTA_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx;							\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_ENCODE_L(_B_logx+1, count);					\
  BINARY_ENCODE_L(_B_xxx+1, 1<<_B_logx, count);				\
} while (0)


#define DELTA_DECODE(x)							\
do {									\
  register unsigned long _B_xxx;					\
  register int _B_logx;							\
  GAMMA_DECODE(_B_logx); _B_logx--;					\
  BINARY_DECODE(_B_xxx, 1<<_B_logx); _B_xxx--;				\
  (x) = _B_xxx + (1<<_B_logx);						\
} while (0)

#define DELTA_DECODE_L(x, count)					\
do {									\
  register unsigned long _B_xxx;					\
  register int _B_logx;							\
  GAMMA_DECODE_L(_B_logx, count); _B_logx--;				\
  BINARY_DECODE_L(_B_xxx, 1<<_B_logx, count); _B_xxx--;			\
  (x) = _B_xxx + (1<<_B_logx);						\
} while (0)

#define DELTA_LENGTH(x, count)						\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx, _B_dcount;					\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_LENGTH(_B_logx+1, count);					\
  BINARY_LENGTH(_B_xxx+1, 1<<_B_logx, _B_dcount);			\
  (count) += _B_dcount;							\
} while (0)


/****************************************************************************/




#define ELIAS_ENCODE(x, b, s)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0;							\
  register double _B_pow=1.0;						\
  POSITIVE(elias, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_ENCODE(_B_k+1);							\
  BINARY_ENCODE(_B_xx-_B_lower+1, _B_upper-_B_lower+1);			\
} while (0)

#define ELIAS_ENCODE_L(x, b, s, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0;							\
  register double _B_pow=1.0;						\
  POSITIVE(elias, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_ENCODE_L(_B_k+1, count);					\
  BINARY_ENCODE_L(_B_xx-_B_lower+1, _B_upper-_B_lower+1, count);	\
} while (0)

#define ELIAS_DECODE(x, b, s)						\
do {									\
  register unsigned long _B_xx;						\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k;							\
  register double _B_pow=1.0;						\
  UNARY_DECODE(_B_k); _B_k--;						\
  while (_B_k--)							\
    {									\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  BINARY_DECODE(_B_xx, _B_upper-_B_lower+1); _B_xx--;			\
  POSITIVE(gamma, _B_xx+_B_lower);				  	\
  (x) = _B_xx+_B_lower;							\
} while (0)

#define ELIAS_DECODE_L(x, b, s, count)					\
do {									\
  register unsigned long _B_xx;						\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k;							\
  register double _B_pow=1.0;						\
  UNARY_DECODE_L(_B_k, count); _B_k--;					\
  while (_B_k--)							\
    {									\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  BINARY_DECODE_L(_B_xx, _B_upper-_B_lower+1, count); _B_xx--;		\
  POSITIVE(gamma, _B_xx+_B_lower);				  	\
  (x) = _B_xx+_B_lower;							\
} while (0)

#define ELIAS_LENGTH(x, b, s, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0, _B_ecount;					\
  register double _B_pow=1.0;						\
  POSITIVE(gamma, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_LENGTH(_B_k+1, count);						\
  BINARY_LENGTH(_B_xx-_B_lower+1, _B_upper-_B_lower+1, _B_ecount);	\
  (count) += _B_ecount;							\
} while (0)


/****************************************************************************/


#define BBLOCK_ENCODE(x, b)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_ENCODE(_B_xdivb+1);						\
  BINARY_ENCODE(_B_xx+1, _B_bb);					\
} while (0)

#define BBLOCK_ENCODE_L(x, b, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_ENCODE_L(_B_xdivb+1, count);					\
  BINARY_ENCODE_L(_B_xx+1, _B_bb, count);				\
} while (0)

#define BBLOCK_DECODE(x, b)						\
do {									\
  register unsigned long _B_x1, _B_xx = 0;				\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb;						\
  UNARY_DECODE(_B_xdivb); _B_xdivb--;					\
  while (_B_xdivb--)							\
    _B_xx += _B_bb;							\
  BINARY_DECODE(_B_x1, _B_bb);						\
  (x) = _B_xx+_B_x1;							\
} while (0)

#define BBLOCK_DECODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x1, _B_xx = 0;				\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb;						\
  UNARY_DECODE_L(_B_xdivb, count); _B_xdivb--;				\
  while (_B_xdivb--)							\
    _B_xx += _B_bb;							\
  BINARY_DECODE_L(_B_x1, _B_bb, count);					\
  (x) = _B_xx+_B_x1;							\
} while (0)

#define BBLOCK_LENGTH(x, b, count)					\
do {									\
  register unsigned long _B_bcount, _B_xx = (x);			\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_LENGTH(_B_xdivb+1, count);					\
  BINARY_LENGTH(_B_xx+1, _B_bb, _B_bcount);				\
  (count) += _B_bcount;  						\
} while (0)

#endif
