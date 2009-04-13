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
/*
* Modification history:
* DM 09-01-00 extracted from extract.c
* DM 16-01-00 scan_directory() renamed as ys_locate_documents()
* DM 25-11-00 Added locate_http_documents()
*/

#include "locator.h"
#include "list.h"
#include "makedb.h"
#include "util.h"

#ifdef USE_WGET

#ifdef	__cplusplus
extern "C" {
#endif

typedef int (*eval_func_t)(void *, const char *, const char *);
extern int wget(int, char **, eval_func_t, void *);

#ifdef	__cplusplus
}
#endif

/**
 * Scan a web site recursively. 
 * Files are downloaded, evaluated and finally deleted.
 */
static int
locate_http_documents(
	const char *pathname, 
	ys_pfn_document_processor_t *pfunc,
	ys_mkdb_t *arg)
{
	
	int argc = 0;
	char **argv;
	ys_wget_option_t *opt_ptr;
	ys_list_t *wget_opts;
	int rc;
	
	/* --quiet, --recursive, --delete-after, --accept=html,htm */
        /* --wait=1 --directory-prefix=dir */

	wget_opts = ys_mkdb_get_wgetargs(arg);
	for (argc = 0, opt_ptr = (ys_wget_option_t*) ys_list_first(wget_opts); 
		opt_ptr != 0;
		opt_ptr = (ys_wget_option_t *)ys_list_next(wget_opts, opt_ptr)) {
		argc++;
	}
	argv = (char **)calloc(argc+3, sizeof(char *));
	if (argv == 0) {
		fprintf(stderr, "Out of memory in locate_http_documents()\n");
		exit(1);
	}
	argc = 0;
	argv[argc++] = "yasemakedb";
	for (opt_ptr = (ys_wget_option_t*) ys_list_first(wget_opts); 
		opt_ptr != 0;
		opt_ptr = (ys_wget_option_t *)ys_list_next(wget_opts, opt_ptr)) {
		argv[argc++] = opt_ptr->option; 
	}
	argv[argc++] = (char *)pathname;
	argv[argc] = 0;

	rc = wget(argc, argv, (eval_func_t)pfunc, (void *)arg);
	free(argv);
	if (rc == 0)
		return 0;
	return -1;
}
#endif

/**
 * Scan a diven directory or web site recursively. Directory structures are
 * left undisturbed.
 * Web sites are handled by locate_http_documents() above.
 */
int
ys_locate_documents(
	const char *pathname, 
	ys_pfn_document_processor_t *pfunc,
	ys_mkdb_t *arg)
{
	char curpath[PATH_MAX] = {0};
	DIR  *dirfp = NULL;
	struct dirent  *dp = NULL;
	int  pathlen = 0;

#ifdef USE_WGET
	if (strncmp(pathname, "http://", 7) == 0) {
		return locate_http_documents(pathname, pfunc, arg);
	}
#endif

	pathlen = strlen(pathname);
	dirfp = opendir(pathname);
	if (dirfp == NULL) {
		perror("opendir");
		fprintf(stderr, "Cannot open directory '%s'\n", pathname);
		return 0;
	}

	strncpy(curpath, pathname, sizeof curpath);
	for (dp = readdir(dirfp); 
	     dp != NULL; 
	     dp = readdir(dirfp)) {
		
		int is_directory = 0;
		struct stat statbuf;

		if (strcmp(dp->d_name, ".") == 0 || 
		    strcmp(dp->d_name, "..") == 0)
			continue;

#if _IGNORE_YASE_FILES
		if (strstr(dp->d_name, "yase."))
			continue;
		if (strstr(dp->d_name, "tmp."))
			continue;
#endif

		snprintf(curpath+pathlen, sizeof curpath-pathlen, 
			"/%s", dp->d_name);
		if (stat(curpath, &statbuf) < 0) {
			perror("stat");
			fprintf(stderr, "Unable to get information about %s\n",
				dp->d_name);
			continue;
		}
		is_directory = ((statbuf.st_mode & S_IFMT) == S_IFDIR);
		if (is_directory) {
			if (ys_locate_documents(curpath, pfunc, arg) < 0)
				return -1;
		}
		else {
			if (pfunc(arg, curpath, curpath) < 0)
				return -1;
		}
	}
	closedir(dirfp);
	return 0;
}
