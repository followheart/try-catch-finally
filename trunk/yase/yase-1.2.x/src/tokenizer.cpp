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
// 15-Dec-02: Started
// 31-Dec-02: Complete re-write. New StringTokenizer class.
// 01-Jan-03: Added support for extracting ascii text out of utf-8 input
// 02-Jan-03: Revised - added test case with file
// 07-Jan-03: Converted StringTokenizer to a template 

#include "tokenizer.h"

YASENS CharTokenizer::CharTokenizer()
{
	reset();
}

YASENS CharTokenizer::~CharTokenizer()
{
}

void 
YASENS CharTokenizer::reset()
{
	inword = false;
	len = 0;
	term[len] = 0;
	binaryCount = 0;
}

int 
YASENS CharTokenizer::addCh(int ch)
{
	if (isEofChar(ch))
		goto complete_word;
	if (isWordChar(ch)) {
		ch = normalize(ch);
		if (!inword) {
			len = 0;
			term[len++] = ch;
			inword = true;
		}
		else {
			if (len < sizeof term-2)
				term[len++] = ch;
		}
	}
	else {
		if (isBinaryChar(ch))
			binaryCount++;
complete_word:
		if (inword) {
			term[len] = 0;
			inword = false;
			return TC_WORD_COMPLETED;
		}
	}
	return TC_AGAIN;	
}

const ys_uchar_t *
YASENS CharTokenizer::getWord()
{
	if (inword) {
		term[len] = 0;
		inword = false;
	}
	return term;
}


#ifdef TEST_TOKENIZER

YASE_NS_USING

void parsefile(const char *name)
{
	FILE *file = fopen(name, "r");
	if (file == 0) {
		fprintf(stderr, "Unable to open file '%s'\n",
			name);
		exit(1);
	}
	char buf[60];		/* force some lines to break in the middle */
	TStringTokenizer<CharTokenizer> st;
	const ys_uchar_t *cp;
	while (fgets(buf, sizeof buf, file) != 0) {
		st.addInput(buf);
		cp = st.nextToken();
		while (cp != 0) {
			printf("%s\n", cp);
			cp = st.nextToken();
		}
	}
	cp = st.endInput();
	if (cp != 0)
			printf("Trailing token = %s\n", cp);
	fclose(file);
}


int main(int argc, const char *argv[])
{
	const char text[] = "This is a sample ( input)string.\n\t be";
	CharTokenizer *t = new CharTokenizer();
	for (int i = 0; i < sizeof text; i++) {
		int state = t->addCh(text[i]);
		switch (state) {
		case CharTokenizer::TC_WORD_COMPLETED:
			printf("%s\n", t->getWord());
			break;
		default:
			break;
		}
	}
	printf("Number of binary characters = %d\n", t->countBinary());
	delete t;
	TStringTokenizer<CharTokenizer> t2;
	t2.setInput(text);
	const ys_uchar_t *cp = t2.nextToken();
	while (cp != 0) {
		printf("StringTokenizer.nextToken() = %s\n", cp);
		cp = t2.nextToken();
	}
	t2.addInput("fore new_words added+here.\nabout");
	cp = t2.nextToken();
	while (cp != 0) {
		printf("StringTokenizer.nextToken() = %s\n", cp);
		cp = t2.nextToken();
	}
	cp = t2.endInput();
	if (cp != 0) {
		printf("StringTokenizer.nextToken() = %s\n", cp);
	}
	printf("Number of binary characters = %d\n", t2.countBinary());
	if (argc == 2) {
		parsefile(argv[1]);
	}
	return 0;
}

#endif
