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
#ifndef tokenizer_h
#define tokenizer_h

#include "yase.h"

YASE_NS_BEGIN

/**
 * YASE requires the ability to parse input documents and
 * extract "terms" or tokens for indexing. This process can be
 * handled in several ways depending upon the needs of the 
 * module that invokes the tokenizer. To cater to the diverse
 * needs, a tokenizer is needed that can be used to gather one
 * character at a time into a buffer. CharTokenizer essentially
 * does that.
 *
 * Mostly, we do not need character by character input - because
 * we can process chunks of characters at a time. For this purpose,
 * the StringTokenizer is provided. One special feature of the 
 * String Tokenizer is its ability to progressively receive chunks
 * of input - this ability is required because of the way the
 * HTML/XML SAX parser works.
 */

class CharTokenizer
{
protected:
	ys_uchar_t term[YS_TERM_LEN];	/* buffer to collect token */
	bool inword;					/* flag to indicate whether a token is being
									 * accumulated.
									 */
	int len;						/* length of current token */
	int binaryCount;				/* How many binary characters encountered since
									 * last reset.
									 */
public:
	enum {
		TC_AGAIN = 0,
		TC_WORD_COMPLETED = 1
	};

	enum {
		TC_EOF_CHAR = -1
	};

	/**
	 * Create a tokenizer.
	 */
	CharTokenizer();

	/**
	 * Destroys the tokenizer.
	 */
	virtual ~CharTokenizer();
	
	/**
	 * Re-initializes the tokenizer.
	 */
	virtual void reset();
	
	/**
	 * Processes a single character. Returns one of these:
	 * TC_WORD_COMPLETED - indicates that a word has been parsed.
	 * TC_AGAIN - indicates that addCh() must be called again with more input.
	 */
	virtual int addCh(int ch);
	
	/**
	 * Returns the token
	 */
	virtual const ys_uchar_t *getWord();
	
	/**
	 * Determines which characters can appear in words.
	 */
	virtual bool isWordChar(int ch) const
	{ 
		return isalnum(ch) != 0; 
	}
	
	/**
	 * Determines which characters are considered as space.
	 */
	virtual bool isSpaceChar(int ch) const
	{ 
		return isspace(ch) != 0; 
	}
	
	/**
	 * Determines which characters are considered binary.
	 */
	virtual bool isBinaryChar(int ch) const
	{ 
		return (ch < 32 || ch > 127) && !isSpaceChar(ch); 
	}
	
	/**
	 * Determines which characters are treated as indicating EOF 
	 */
	virtual bool isEofChar(int ch) const
	{ 
		return ch == TC_EOF_CHAR; 
	}
	
	/**
	 * Counts number of binary characters since last
	 * reset.
	 */
	virtual int countBinary() const
	{ 
		return binaryCount; 
	}
	
	/**
	 * Convert to lower case.
	 */
	virtual int normalize(int ch)
	{
		return ch;
	}

	/**
	 * Terminate input.
	 */
	virtual int endInput()
	{
		return addCh(TC_EOF_CHAR);
	}
};

class LowerCaseTokenizer : public CharTokenizer {
	/**
	 * Convert to lower case.
	 */
	virtual int normalize(int ch)
	{
		return tolower(ch);
	}
};

class CIdentTokenizer : public LowerCaseTokenizer {
	/**
	 * Determines which characters can appear in words.
	 */
	virtual bool isWordChar(int ch) const
	{ 
		return isalnum(ch) || ch == '_'; 
	}
};

template <typename T = LowerCaseTokenizer>
class TStringTokenizer {
public:
	typedef T Tokenizer;
protected:
	Tokenizer tokenizer;				/* Tokenizer for processing individual characters */
	ys_uchar_t *buf;					/* Buffer where input is read to be processed */
	size_t buflen;						/* Current buffer length */
	ys_uchar_t *bufptr;					/* Current offset into buffer */
	ys_uchar_t *endptr;					/* Points to end of input string */
public:
	/**
	 * Create a TStringTokenizer.
	 */
	TStringTokenizer() 
	{ 
		buf = 0; 
		buflen = 0; 
		bufptr = buf; 
		endptr = buf;
	}

	/**
	 * Destroy a TStringTokenizer.
	 */
	virtual ~TStringTokenizer() 
	{ 
		if (buf != 0) 
			free(buf); 
	}

	/**
	 * Set the input buffer - any previous content is lost and the
	 * tokenizer reset.
	 */
	void setInput(const ys_uchar_t *string, size_t len);
	void setInput(const ys_uchar_t *string)
	{
		setInput(string, strlen((const char *)string));
	}

	/**
	 * Append to the input buffer - preserving unprocessed buffer contents.
	 */
	void addInput(const ys_uchar_t *string, size_t len);
	void addInput(const ys_uchar_t *string)
	{
		addInput(string, strlen((const char *)string));
	}

	/**
	 * Reset the tokenizer.
	 */
	void reset() 
	{ 
		bufptr = buf; 
		endptr = buf;
		tokenizer.reset();
	}

	/**
	 * Get next token. Returns NULL if input exhausted.
	 */
	virtual const ys_uchar_t *nextToken();
	virtual const ys_uchar_t *endInput();
	int countBinary() const 
	{
		return tokenizer.countBinary();
	}
};

/**
 * This tokenizer handles UTF-8 input. Non-ascii characters are
 * ignored (treated as space).
 */
template <typename T = LowerCaseTokenizer>
class TUTF8ToAsciiTokenizer : public TStringTokenizer<T> {
public:
	TUTF8ToAsciiTokenizer() {}
	const ys_uchar_t *nextToken();
};

typedef TStringTokenizer<LowerCaseTokenizer> StringTokenizer;
typedef TUTF8ToAsciiTokenizer<LowerCaseTokenizer> UTF8ToAsciiTokenizer;

template <typename T>
void
TStringTokenizer<T>::setInput(const ys_uchar_t *string, size_t ilen)
{
	size_t len = ilen+1;
	if (buf == 0) {
		buflen = len;
		buf = (ys_uchar_t *)calloc(1, buflen);
	}
	else {
		if (len > buflen) {
			buf = (ys_uchar_t *)realloc(buf, len);
			buflen = len;
		}
	}
	assert(buf != 0);
	memcpy(buf, string, ilen);
	bufptr = buf;
	endptr = buf + ilen;
	*endptr = 0;
}

template <typename T>
void
TStringTokenizer<T>::addInput(const ys_uchar_t *string, size_t ilen)
{
	size_t len = ilen+1;
	size_t old_len = 0;
	ys_uchar_t *oldbuf = 0;
	if (buf == 0) {
		buflen = len;
		buf = (ys_uchar_t *)calloc(1, buflen);
	}
	else {
		if (bufptr < endptr) {
			assert(buf != 0);
			assert(bufptr >= buf && bufptr < buf+buflen);
			assert(endptr >= bufptr && endptr < buf+buflen);
			old_len = endptr-bufptr; 
			len += old_len;
		}
		if (len > buflen) {
			oldbuf = buf;
			buf = (ys_uchar_t *)calloc(1, len);
			buflen = len;
		}
	}
	assert(buf != 0);
	ys_uchar_t *cp = buf;
	if (old_len > 0) {
		memcpy(cp, bufptr, old_len);
		cp += old_len;
		if (oldbuf != 0)
			free(oldbuf);
	}
	memcpy(cp, string, ilen);
	bufptr = buf;
	endptr = buf+old_len+ilen;
	*endptr = 0;
}

template <typename T>
const ys_uchar_t *
TStringTokenizer<T>::nextToken()
{
	if (buf == 0 || bufptr >= endptr)
		return 0;
	do {
		int state = tokenizer.addCh(*bufptr++);
		if (state == Tokenizer::TC_WORD_COMPLETED)
			return tokenizer.getWord();
	} while (bufptr < endptr);
	return 0;
}	

template <typename T>
const ys_uchar_t *
TStringTokenizer<T>::endInput()
{
	int state = tokenizer.endInput();
	if (state == Tokenizer::TC_WORD_COMPLETED)
		return tokenizer.getWord();
	return 0;
}	

/**
 * Non-ascii characters are simply treated as delimiters and
 * discarded.
 */
template <typename T>
const ys_uchar_t *
TUTF8ToAsciiTokenizer<T>::nextToken()
{
	if (buf == 0 || bufptr >= endptr)
		return 0;
	do {
		int ch = *bufptr++;
		if (ch & 0x80)	/* non-ascii */
			ch = ' ';
		int state = tokenizer.addCh(ch);
		if (state == Tokenizer::TC_WORD_COMPLETED)
			return tokenizer.getWord();
	} while (bufptr < endptr);
	return 0;
}	

YASE_NS_END

#endif
