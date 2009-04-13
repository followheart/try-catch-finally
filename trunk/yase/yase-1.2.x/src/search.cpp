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

// 09-01-03: Modified so that Collection can be specified as parameter

#include "search.h"
#include "rankedsearch.h"
#include "boolsearch.h"

YASENS Search::Search(YASENS Collection *collection)
{
	this->collection = collection;
	input = 0;
	elapsed = 0.0;
}

YASENS Search::~Search()
{
	if (input != 0)
		free(input);
}

void
YASENS Search::reset()
{
}

/**
 * TODO: Check for memory allocatio failures.
 */
void
YASENS Search::addInput(const ys_uchar_t *text)
{
	if (input == 0)
		input = (ys_uchar_t *)strdup((const char *)text);
	else {
		input = (ys_uchar_t *)realloc(
			input,
			strlen((const char *)input)+
			strlen((const char *)text)+1);
		strcat((char *)input, (const char *)text);
	}
}

YASENS Search*
YASENS Search::createSearch(YASENS Collection *collection, int method)
{
	if (method == SM_RANKED)
		return new RankedSearch(collection);
	else if (method == SM_BOOLEAN)
		return new BoolSearch(collection);
	return 0;
}
