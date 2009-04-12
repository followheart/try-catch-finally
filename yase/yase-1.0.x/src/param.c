/***
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 2000-2002  Dibyendu Majumdar.
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
#include "yase.h"
#include "param.h"

typedef struct {
	int n, allocated;
	char **names;
	char **values;
} parameters_t;

static int 
add_parameter(parameters_t *p, const char *name, const char *value)
{
	if ( p->n == p->allocated ) {
		p->allocated += 10;
		p->names = realloc(p->names, sizeof(char *)*p->allocated);
		p->values = realloc(p->values, sizeof(char *)*p->allocated);
	}
	p->names[p->n] = strdup(name);
	p->values[p->n] = strdup(value);
	p->n++;
	return 0;
}
	
void * 
ys_load_properties(const char *name)
{
	FILE *file;
	parameters_t *p;
	char buf[BUFSIZ];

	p = (parameters_t*) calloc(1, sizeof(parameters_t));
	if (p == 0) {
		perror("calloc");
		fprintf(stderr, "Failed to allocate memory\n");
		return 0;
	}
	p->n = p->allocated = 0;
	p->names = p->values = 0;
			
	file = fopen(name, "r");
	if (file == 0) {
		perror("fopen");
		fprintf(stderr, "Unable to open file %s\n", name);
		return p;
	}
	if (Ys_debug) {
		printf("opened parameter file %s\n", name);
	}
	
	while (fgets(buf, sizeof buf, file) != 0) {
		char *n, *v;
		if (*buf == '#')
			continue;
		n = strtok(buf, "=");
		v = strtok(0, "\n");	
		if (n == 0 || v == 0)
			continue;
		add_parameter(p, n, v);
	}
	fclose(file);
	return p;		
}

const char * 
ys_get_property(void *handle, const char *name)
{
	int i;
	parameters_t *p = (parameters_t *)handle;
	if (p == 0)
		return 0;
	for (i = 0; i < p->n; i++) {
		if (strcmp(p->names[i], name) == 0)
			return p->values[i];
	}
	return 0;
}

#ifdef TEST_PARAM

int main()
{
	parameters_t *p = (parameters_t *)ys_load_properties("yase.config");
	int i;
	for (i = 0; i < p->n; i++) {
		printf("%s = %s\n", p->names[i], p->values[i]);
	}
	return 0;
}

#endif
	
