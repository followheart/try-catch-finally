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
#include "btree.h"
#include <getopt.h>

static void usage(void)
{
	fprintf(stderr, "Usage: index [-h <dbpath>]\n");
	exit(1);
}

int 
main(int argc, char *argv[])
{
	int c;
	char *dbpath = 0;
	char wordfile[1024];
	char treefile[1024];

	opterr = 0;
	while ( (c = getopt(argc, argv, "h:")) != EOF) {
		switch (c) {
		case 'h':
			dbpath = optarg; break;
		case '?':
			fprintf(stderr, "Unrecognised option: -%c\n",
				optopt);
			usage();
			exit(1);
		}
	}

	if (dbpath == 0)
		dbpath = ".";
	
	snprintf(wordfile, sizeof wordfile, "%s/%s", dbpath, "yase.words");
	snprintf(treefile, sizeof treefile, "%s/%s", dbpath, "yase.btree");
	return ys_btree_create( treefile, wordfile );
}
