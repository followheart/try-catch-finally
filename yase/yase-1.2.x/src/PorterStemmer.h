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

#ifndef porterstemmer_h
#define porterstemmer_h

class PorterStemmer  
{
public:
	PorterStemmer();
	virtual ~PorterStemmer();
	const char *stem(const char *word);
	void verify(const char *in, const char *out);
private:
	void set(const char *word);
	bool step5b();
	bool step5a();
	bool step4();
	bool step3();
	bool step2();
	bool step1c();
	bool step1b();
	bool step1a();
	bool endsWith(const char *a, int a_len);
	bool replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len);
	bool replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len, int m);
	bool replaceIfEndsWith(const char *a, int a_len, const char *b, int b_len, bool vowel);
	bool endsWithCVC(int length);
	bool endsDoubleConsonants();
	bool isVowel(int pos);
	int  computeM(int length);
	bool hasVowel(int length);
private:
	int len;
	char word[256];
};

#endif 
