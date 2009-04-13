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

// 23-01-03: Created

#include "properties.h"

YASENS Properties::Properties()
{
	n = 0;
	allocated = 0;
	names = 0;
	values = 0;
}

YASENS Properties::~Properties()
{
	if (allocated > 0) {
		for (int i = 0; i < n; i++) {
			if (names[i] != 0)
				free(names[i]);
			if (values[i] != 0)
				free(values[i]);
		}
		free(names);
		free(values);
	}
	allocated = 0;
	n = 0;
}

void
YASENS Properties::add(const char *name, const char *value)
{
	if ( n == allocated ) {
		allocated += 10;
		names = (char **)realloc(names, sizeof(char *)*allocated);
		values = (char **)realloc(values, sizeof(char *)*allocated);
		for (int i = allocated-10; i < allocated; i++) {
			names[i] = 0;
			values[i] = 0;
		}
	}
	names[n] = strdup(name);
	values[n] = strdup(value);
	n++;
}
	
bool
YASENS Properties::load(const char *name)
{
	FILE *file;
	char buf[BUFSIZ];
		
	file = fopen(name, "r");
	if (file == 0) {
		perror("fopen");
		fprintf(stderr, "Unable to open file %s\n", name);
		return false;
	}
	if (Ys_debug) {
		printf("Opened properties file %s\n", name);
	}
	while (fgets(buf, sizeof buf, file) != 0) {
		char *n, *v;
		if (*buf == '#')
			continue;
		n = strtok(buf, "=");
		v = strtok(0, "\n");	
		if (n == 0 || v == 0)
			continue;
		add(n, v);
	}
	fclose(file);
	return true;		
}

const char * 
YASENS Properties::get(const char *name)
{
	int i;
	for (i = 0; i < n; i++) {
		if (strcmp(names[i], name) == 0) {
			return values[i];
		}
	}
	return 0;
}	
