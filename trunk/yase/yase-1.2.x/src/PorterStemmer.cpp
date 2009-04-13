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

// PorterStemmer.cpp: implementation of the PorterStemmer class.

// 02-12-02: Created

#include "PorterStemmer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

PorterStemmer::PorterStemmer()
{
	len = 0;
	word[0] = 0;
}

PorterStemmer::~PorterStemmer()
{
}

void PorterStemmer::set(const char *word)
{
	len = strlen(word);
	strncpy(this->word, word, sizeof this->word);
}

int PorterStemmer::computeM(int length)
{
	int m = 0;
	/* skip consonants */
	int pos = 0;
	while (pos < length) {
		if (isVowel(pos))
			break;
		pos++;
	}
	/* count vowel/consonant pairs */
	while (pos < length) {
		/* we must be at a vowel */
		pos++;
		/* skip vowels */
		while (pos < length && isVowel(pos))
			pos++;
		if (pos == length)
			break;
		/* we must be at a consonant */
		m++;
		pos++;
		/* skip consonants */
		while (pos < length && !isVowel(pos))
			pos++;
	}
	return m;
}

bool PorterStemmer::isVowel(int pos)
{
	switch (word[pos]) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
		return true;
	case 'y':
		if (pos > 0 && !isVowel(pos-1))
			return true;
	}
	return false;
}

bool PorterStemmer::endsDoubleConsonants()
{
	if (len > 1) {
		char ch1 = word[len-1];
		char ch2 = word[len-2];
		if (ch1 == ch2 && !isVowel(len-1))
			return true;
	}
	return false;
}

bool PorterStemmer::endsWithCVC(int length)
{
	if (length < 3)
		return false;
	if (word[length-1] == 'w' || word[length-1] == 'x' || word[length-1] == 'y')
		return false;
	return (!isVowel(length-1) && isVowel(length-2) && !isVowel(length-3));
}

bool PorterStemmer::replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len)
{
	if (len < a_len)
		return false;
	if (strcmp(&word[len-a_len], a) == 0) {
		strcpy(&word[len-a_len], b);
		len += b_len-a_len;
		return true;
	}
	return false;
}

bool PorterStemmer::replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len, int m)
{
	if (len < a_len)
		return false;
	if (strcmp(&word[len-a_len], a) == 0) {
		if (computeM(len-a_len) > m) {
			strcpy(&word[len-a_len], b);
			len += b_len-a_len;
		}
		return true;
	}
	return false;
}

bool PorterStemmer::hasVowel(int length)
{
	for (int pos = 0; pos < length; pos++) {
		if (isVowel(pos))
			return true;
	}
	return false;
}

bool PorterStemmer::replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len, bool vowel)
{
	if (len < a_len)
		return false;
	if (strcmp(&word[len-a_len], a) == 0) {
		if (hasVowel(len-a_len)) {
			strcpy(&word[len-a_len], b);
			len += b_len-a_len;
			return true;
		}
	}
	return false;
}


bool PorterStemmer::step1a()
{
	if (word[len-1] == 's') {
		if (replaceIfEndsWith("sses", 4, "ss", 2))
			return true;
		else if (replaceIfEndsWith("ies", 3, "i", 1))
			return true;
		else if (endsWith("ss", 2))
			return true;
		else if (replaceIfEndsWith("s", 1, "", 0))
			return true;
	}
	return false;
}

bool PorterStemmer::endsWith(const char *a, int a_len)
{
	if (len < a_len)
		return false;
	if (strcmp(&word[len-a_len], a) == 0) {
		return true;
	}
	return false;
}

bool PorterStemmer::step1b()
{
	if (replaceIfEndsWith("eed", 3, "ee", 2, 0))
		return true;
	bool proceed = replaceIfEndsWith("ed", 2, "", 0, true) ||
		replaceIfEndsWith("ing", 3, "", 0, true);
	if (proceed) {
		if (len >= 2) {
			if (replaceIfEndsWith("at", 2, "ate", 3))
				return true;
			else if (replaceIfEndsWith("bl", 2, "ble", 3))
				return true;
			else if (replaceIfEndsWith("iz", 2, "ize", 3))
				return true;
			else if (endsDoubleConsonants()) {
				switch (word[len-1]) {
				case 's':
				case 'l':
				case 'z':
					break;
				default: 
					word[len-1] = 0;
					len--;
					return true;
				}
			}
			else {
				if (computeM(len) == 1 && endsWithCVC(len)) {
					word[len++] = 'e';
					word[len] = 0;
					return true;
				}
			}
		}
	}
	return false;
}

bool PorterStemmer::step1c()
{
	if (len > 1 && word[len-1] == 'y' && hasVowel(len-1)) {
		word[len-1] = 'i';
		return true;
	}
	return false;
}

bool PorterStemmer::step2()
{
	if (len >= 3) {
		switch (word[len-2]) {
		case 'a':
			if (replaceIfEndsWith("ational", 7, "ate", 3, 0))
				return true;
			else if (replaceIfEndsWith("tional", 6, "tion", 4, 0))
				return true;
			break;
		case 'c':
			if (replaceIfEndsWith("enci", 4, "ence", 4, 0))
				return true;
			else if (replaceIfEndsWith("anci", 4, "ance", 4, 0))
				return true;
			break;
		case 'e':
			if (replaceIfEndsWith("izer", 4, "ize", 3, 0))
				return true;
			break;
		case 'l':
//			if (replaceIfEndsWith("abli", 4, "able", 4, 0))
//				return true;
			if (replaceIfEndsWith("bli", 3, "ble", 3, 0))
				return true;
			else if (replaceIfEndsWith("alli", 4, "al", 2, 0))
				return true;
			else if (replaceIfEndsWith("entli", 5, "ent", 3, 0))
				return true;
			else if (replaceIfEndsWith("eli", 3, "e", 1, 0))
				return true;
			else if (replaceIfEndsWith("ousli", 5, "ous", 3, 0))
				return true;
			break;
		case 'o':
			if (replaceIfEndsWith("ization", 7, "ize", 3, 0))
				return true;
			else if (replaceIfEndsWith("ation", 5, "ate", 3, 0))
				return true;
			else if (replaceIfEndsWith("ator", 4, "ate", 3, 0))
				return true;
			break;
		case 's':
			if (replaceIfEndsWith("alism", 5, "al", 2, 0))
				return true;
			else if (replaceIfEndsWith("iveness", 7, "ive", 3, 0))
				return true;
			else if (replaceIfEndsWith("fulness", 7, "ful", 3, 0))
				return true;
			else if (replaceIfEndsWith("ousness", 7, "ous", 3, 0))
				return true;
			break;
		case 't':
			if (replaceIfEndsWith("aliti", 5, "al", 2, 0))
				return true;
			else if (replaceIfEndsWith("iviti", 5, "ive", 3, 0))
				return true;
			else if (replaceIfEndsWith("biliti", 6, "ble", 3, 0))
				return true;
			break;
		case 'g':
			if (replaceIfEndsWith("logi", 4, "log", 3, 0))
				return true;
			break;
		}
	}
	return false;
}

bool PorterStemmer::step3()
{
	if (len >= 3) {
		switch(word[len-1]) {
		case 'e':
			if (replaceIfEndsWith("icate", 5, "ic", 2, 0))
				return true;
			else if (replaceIfEndsWith("ative", 5, "", 0, 0))
				return true;
			else if (replaceIfEndsWith("alize", 5, "al", 2, 0))
				return true;
			break;
		case 'i':
			if (replaceIfEndsWith("iciti", 5, "ic", 2, 0))
				return true;
			break;
		case 'l':
			if (replaceIfEndsWith("ical", 4, "ic", 2, 0))
				return true;
			else if (replaceIfEndsWith("ful", 3, "", 0, 0))
				return true;
			break;
		case 's':
			if (replaceIfEndsWith("ness", 4, "", 0, 0))
				return true;
			break;
		}
	}
	return false;
}

bool PorterStemmer::step4()
{
	if (len >= 2) {
		switch(word[len-2]) {
		case 'a':
			if (replaceIfEndsWith("al", 2, "", 0, 1))
				return true;
			break;
		case 'c':
			if (replaceIfEndsWith("ance", 4, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("ence", 4, "", 0, 1))
				return true;
			break;
		case 'e':
			if (replaceIfEndsWith("er", 2, "", 0, 1))
				return true;
			break;
		case 'i':
			if (replaceIfEndsWith("ic", 2, "", 0, 1))
				return true;
			break;
		case 'l':
			if (replaceIfEndsWith("able", 4, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("ible", 4, "", 0, 1))
				return true;
			break;
		case 'n':
			if (replaceIfEndsWith("ant", 3, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("ement", 5, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("ment", 4, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("ent", 3, "", 0, 1))
				return true;
			break;
		case 'o':
			if (len >= 4 && (word[len-4] == 's' || word[len-4] == 't') && endsWith("ion", 3)) {
				if (replaceIfEndsWith("ion", 3, "", 0, 1))
					return true;
			}
			else if (replaceIfEndsWith("ou", 2, "", 0, 1))
				return true;
			break;
		case 's':
			if (replaceIfEndsWith("ism", 3, "", 0, 1))
				return true;
			break;
		case 't':
			if (replaceIfEndsWith("ate", 3, "", 0, 1))
				return true;
			else if (replaceIfEndsWith("iti", 3, "", 0, 1))
				return true;
			break;
		case 'u':
			if (replaceIfEndsWith("ous", 3, "", 0, 1))
				return true;
			break;
		case 'v':
			if (replaceIfEndsWith("ive", 3, "", 0, 1))
				return true;
			break;
		case 'z':
			if (replaceIfEndsWith("ize", 3, "", 0, 1))
				return true;
			break;
		}
	}
	return false;
}

bool PorterStemmer::step5a()
{
	if (len > 2 && word[len-1] == 'e') {
		int m = computeM(len-1);
		if (m > 1 || (m == 1 && !endsWithCVC(len-1))) {
			word[len-1] = 0;
			len--;
			return true;
		}
	}
	return false;
}

bool PorterStemmer::step5b()
{
	if (len >= 5) {
		if (word[len-1] == 'l' && endsDoubleConsonants()) {
			if (computeM(len) > 1) {
				word[len-1] = 0;
				len--;
				return true;
			}
		}
	}
	return false;
}

const char *PorterStemmer::stem(const char *word)
{
	set(word);
	if (len <= 2) return word;
	step1a();
	step1b();
	step1c();
	step2();
	step3();
	step4();
	step5a();
	step5b();
	return this->word;
}

void PorterStemmer::verify(const char *in, const char *out)
{
	stem(in);
	printf("in(%s), out(%s), expected(%s)\n", in, word, out);
	assert(strcmp(word, out) == 0);
}

int main(int argc, const char *argv[])
{
	PorterStemmer stemmer;
	stemmer.verify("sensibility", "sensibl");
	stemmer.verify("replacement", "replac");
	stemmer.verify("irritant", "irrit");
	stemmer.verify("adjustment", "adjust");
	stemmer.verify("dependent", "depend");
	stemmer.verify("adoption", "adopt");
	stemmer.verify("communism", "commun");
	stemmer.verify("activate", "activ");
	stemmer.verify("angularity", "angular");
	stemmer.verify("homologous", "homolog");
	stemmer.verify("effective", "effect");
	stemmer.verify("bowdlerize", "bowdler");
	stemmer.verify("rate", "rate");
	stemmer.verify("cease", "ceas");
	stemmer.verify("sky", "sky");
	stemmer.verify("roll", "roll");
	stemmer.verify("generalization", "gener");

	if (argc > 1) {
		FILE *file = fopen(argv[1], "r");
		char buf[BUFSIZ];
		if (file == NULL) {
			fprintf(stderr, "cannot open file %s\n", argv[1]);
			exit(1);
		}
		while (fgets(buf, sizeof buf, file) != NULL) {
			char *cp = strchr(buf, '\n');
			if (cp != NULL) *cp = 0;
			fprintf(stdout, "%s\n", stemmer.stem(buf));
		}
		fclose(file);
	}
	return 0;
}
