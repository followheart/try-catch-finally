/***
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
*/

#ifndef dm1_common_h
#define dm1_common_h

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ENTER(x)	if (debug >= 3) do { fputs("ENTER(" x ")\n", stdout); } while(0)
#define EXIT(x)		if (debug >= 3) do { fputs("EXIT(" x ")\n", stdout); } while(0)

#define DM1NS_BEGIN namespace dm1 {
#define DM1NS_END   }
#define DM1NS dm1::
#define DM1NS_USING using namespace dm1;

#endif

