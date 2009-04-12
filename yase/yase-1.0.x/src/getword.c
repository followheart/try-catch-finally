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
/*
* Modification history:
* DM 09-01-00 getword() extracted from extract.c
* DM 16-01-00 created open_document() and close_document()
* DM 18-01-00 introduced Errcnt.
* DM 23-01-00 added support for document filters
* DM 23-04-00 added get_filter() and support for pipes 
* DM 18-02-02 when executing commands, surround them by quotes to allow
*             filenames with embedded spaces to work.
* DM 20-02-02 decode logical names (urls) so that urls containing spaces are handled properly.
* DM 05-05-02 simplified the contents of yase.files and yase.docs.
* DM 06-05-02 fixed a bug where the last word would be skipped when scanning html/xml contents.
* DM 06-05-02 moved some of the common code to functions
*/

#include "getword.h"
#include "getconfig.h"
#include "list.h"
#include "xmlparser.h"
#include "util.h"

typedef struct {
	ys_link_t link;
	char ext[32];
	char *cmd;
	ys_bool_t generates_xml;
	ys_bool_t generates_html;
} filter_t;

typedef struct {
	const char *filename;
	const char *logicalname;
	FILE *file;
	filter_t *filter;
	int errcnt;
} ys_query_document_t;

static ys_query_document_t *document_open( const char *logicalname, const char *name );
static int document_close( ys_query_document_t *doc );
static int document_reopen( ys_query_document_t *doc );
static filter_t * get_filter( const char *ext );
static const char *text_getword( ys_query_document_t *doc, 
	char *word, size_t maxlen );
static int text_processor( ys_query_document_t *doc, ys_docdb_t *docdb, 
	ys_pfn_index_t ptrfunc, ys_mkdb_t *arg );
static int html_processor( ys_query_document_t *doc, ys_docdb_t *docdb, 
	ys_pfn_index_t ptrfunc, ys_mkdb_t *arg );
static int xml_processor( ys_query_document_t *doc, ys_docdb_t *docdb, 
	ys_pfn_index_t ptrfunc, ys_mkdb_t *arg );
static int my_html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, 
	ys_pfn_index_t pfn_index, ys_mkdb_t *arg);
static ys_bool_t strendswith( const char *s, const char *suffix );
static const char *filebasename(const char *filename);
static void init_docfile(ys_query_document_t *doc, ys_docdata_t *docfile,
	const char *type);

/**
 * Determines if a filter is available to transform a file.
 * Filters are specified in the yase.config file using the following syntax:
 * ext.filter=type;cmd
 * where,
 *    ext = file extension to be matched (eg "gz", "doc.gz", etc.)
 *    filter = literal
 *    type = one of pipe, xml, htnl, text (optional - default is text)
 *    cmd = unix command to be run via the shell
 *
 * To allow more than one filter to be chained, the pipe type can be used.
 * This type specifies that the output from this must be piped to another
 * filter. 
 * 
 * In case an entire extension is not matched, each part of the extension
 * is matched from the bottom up. This is where the pipe type can prove
 * useful.
 *
 */ 
static filter_t *
ys_get_filter(
	const char *ext) 
{
	static ys_list_t cache = {0};
	static ys_bool_t first_time = BOOL_TRUE;
	filter_t *f;
	const char *p, *cp1;
	char *cp;
	char pname[1024];
	char extcopy[32];
	int old_len, new_len;

	if (first_time) {
		ys_list_init(&cache);
		first_time = BOOL_FALSE;
	}

	/* first look in the cache */
	f = (filter_t *) ys_list_first(&cache);
	for (; f != 0; f = (filter_t *) ys_list_next(&cache, f)) {
		if (strcmp(f->ext, ext) == 0)
			return f;
	}

	f = (filter_t *) calloc(sizeof(filter_t), 1);
	if (f == 0) {
		fprintf(stderr, "Error allocating memory for a filter_t\n");
		return 0;
	}
	f->generates_xml = BOOL_FALSE;
	f->generates_html = BOOL_FALSE;
	f->cmd = 0;
	strncpy(f->ext, ext, sizeof f->ext);

	/* try an exact match */
	snprintf(pname, sizeof pname, "%s.filter", ext);
	p = ys_get_config(pname);
	if (p != 0) {
		cp1 = strchr(p, ';');
		if (cp1 != 0) {
			if (strncmp(p, "xml;", 4) == 0) {
				f->generates_xml = BOOL_TRUE;
			}	
			else if (strncmp(p, "html;", 5) == 0) {
				f->generates_html = BOOL_TRUE;
			}	
			p = cp1+1;
		}
		f->cmd = (char *) calloc(strlen(p)+6, 1);
		if (f->cmd == 0) {
			fprintf(stderr, 
				"Error allocating memory for a f->cmd\n");
			goto error_return;
		}
		strcpy(f->cmd, p);
		strcat(f->cmd, " \"%s\"");
		goto success_return;
	}

	strncpy(extcopy, ext, sizeof extcopy);
	cp = strrchr(extcopy, '.');
	if (cp == 0) 
		goto error_return;
	while (cp != 0) {
		old_len = f->cmd ? strlen(f->cmd) : 0;
		new_len = old_len;

		*cp++ = 0;
		snprintf(pname, sizeof pname, "%s.filter", cp);
		p = ys_get_config(pname);
		if (p == 0) 
			goto error_return;
		cp1 = strchr(p, ';');
		if (cp1 != 0) {
			if (strncmp(p, "pipe;", 4) != 0)
				goto error_return;
			cp1++;
		}
		else
			cp1 = p;
		new_len += 2 + strlen(cp1) + (old_len==0?5:0);
		f->cmd = realloc(f->cmd, new_len);
		if (f->cmd == 0) {
			fprintf(stderr, 
				"Error allocating memory for a f->cmd\n");
			goto error_return;
		}
		f->cmd[old_len] = 0;
		if (old_len != 0) 
			strcat(f->cmd, "|");
		strcat(f->cmd, cp1);
		if (old_len == 0) 
			strcat(f->cmd, " \"%s\"");
		cp = strrchr(extcopy, '.');
	}
	old_len = f->cmd ? strlen(f->cmd) : 0;
	new_len = old_len;
	snprintf(pname, sizeof pname, "%s.filter", extcopy);
	p = ys_get_config(pname);
	if (p == 0) 
		goto error_return;
	cp1 = strchr(p, ';');
	if (cp1 != 0) {
		if (strncmp(p, "xml;", 4) == 0) {
			f->generates_xml = BOOL_TRUE;
		}
		else if (strncmp(p, "html;", 5) == 0) {
			f->generates_html = BOOL_TRUE;
		}
		cp1++;
	}
	else
		cp1 = p;
	new_len += 2 + strlen(cp1);
	f->cmd = realloc(f->cmd, new_len);
	if (f->cmd == 0) {
		fprintf(stderr, 
			"Error allocating memory for a f->cmd\n");
		goto error_return;
	}
	f->cmd[old_len] = 0;
	strcat(f->cmd, "|");
	strcat(f->cmd, cp1);

success_return:
	ys_list_append(&cache, f);
	if (Ys_debug) {
		fprintf(stderr, "%s: ext=%s filter=", __func__,
			ext);
		fputs(f->cmd, stderr);
		fputs("\n", stderr);
	}
	return f;

error_return:
	if (f != 0) {
		if (f->cmd != 0)
			free(f->cmd);
		free(f);
	}
	return 0;
}

/**
 * Checks is a string ends with a particular character sequence.
 */
static ys_bool_t
strendswith(const char *s, const char *suffix)
{
	int len1 = strlen(s);
	int len2 = strlen(suffix);

	if (len1 < len2) 
		return BOOL_FALSE;
	return strcmp(s+len1-len2, suffix) == 0;
}

/**
 * Removes the path from a filename.
 */
static const char *
filebasename(const char *filename)
{
	const char *cp = strrchr(filename, '/');
	if (cp == 0) 
		return filename;
	return cp+1;
}

static FILE *
ys_run_cmd( const char *cmd, const char *name, const char *logicalname ) 
{
	static char putenv_filename[1024];
	static char putenv_logicalname[1024];
	static char putenv_size[1024];
	static char putenv_datecreated[1024];
	static char putenv_title[1024];

	char runcmd[1024];
	struct stat statbuf = {0};
	FILE *file;

	snprintf(putenv_filename, sizeof putenv_filename,
		"YASE_FILENAME=%s", name);
	snprintf(putenv_logicalname, sizeof putenv_logicalname,
		"YASE_LOGICALNAME=%s", logicalname);
	snprintf(putenv_title, sizeof putenv_title,
		"YASE_TITLE=%s", filebasename(name));
	if (stat(name, &statbuf) == 0) {
		time_t tt;
		struct tm *tm;

		snprintf(putenv_size, sizeof putenv_size, 
		"YASE_SIZE=%ld", statbuf.st_size);
		tt = statbuf.st_ctime;
		tm = localtime(&tt);
		strftime(putenv_datecreated, 
			sizeof putenv_datecreated,
			"YASE_DATECREATED=%D %R", tm);
	}
	else {
		snprintf(putenv_size, sizeof putenv_size, 
		"YASE_SIZE=unknown");
		snprintf(putenv_datecreated, 
			sizeof putenv_datecreated,
			"YASE_DATECREATED=unknown");
	}
	putenv(putenv_filename);	
	putenv(putenv_logicalname);	
	putenv(putenv_title);	
	putenv(putenv_size);	
	putenv(putenv_datecreated);	

	snprintf(runcmd, sizeof runcmd, cmd, name);
	printf("%s:%s: Executing %s\n", __FILE__, __func__, runcmd); 
	file = popen(runcmd, "r");
	if (file == NULL) {
		perror("popen");
		fprintf(stderr, "Error executing %s\n", runcmd);
	}
	return file;
}
/**
 * Opens a named document file. If an external command is defined in 
 * yase.config, it is executed. Else the file is opened for reading.
 */
static ys_query_document_t *
document_open( const char *logicalname, const char *name )
{
	ys_query_document_t *d;
	char *ext;

	d = calloc(1, sizeof *d);
	if (d == NULL) {
		perror("calloc");
		fprintf(stderr, "Failed to allocate memory\n");
		return NULL;
	}
	d->filter = 0;
	d->filename = name;
	d->logicalname = logicalname;
	ext = strchr(filebasename(name), '.');
	if (ext != NULL) { 
		d->filter = ys_get_filter(ext+1);
	}
	if (d->filter == 0) {
		d->file = fopen(name, "r");
		if (d->file == NULL) {
			perror("fopen");
			fprintf(stderr, "Error opening file %s\n", name);
		}
	}
	else {
		d->file = ys_run_cmd(d->filter->cmd, name, logicalname);
	}
	if (d->file == NULL) {
		free(d);
		return NULL;
	}
	d->errcnt = 0;
	return d;
}

/**
 * Close a document.
 */
static int 
document_close( ys_query_document_t *doc )
{
	ys_query_document_t *d = (ys_query_document_t *)doc;
	int rc = 0;
	if (d == NULL)
		return 0;
	if (d->filter != 0) {
		int ch;
		while ((ch = fgetc(d->file)) != EOF) ;
		rc = pclose(d->file);
	}
	else {
		if (d->file != 0)
			rc = fclose(d->file);	
	}
	if (rc != 0) {
		perror(d->filter ? "pclose" : "fclose");
		fprintf(stderr, "Error reading from document\n");
		rc = -1;
	}
	free(d);
	return 0;
}

static int 
document_reopen( ys_query_document_t *doc )
{
	int rc = 0;
	const char *name = doc->filename;
	const char *logicalname = doc->logicalname;
	if (doc->filter != 0) {
		int ch;
		while ((ch = fgetc(doc->file)) != EOF) ;
		rc = pclose(doc->file);
	}
	else {
		rewind(doc->file);	
		return 0;
	}
	if ( rc != 0 ) {
		perror("pclose");
		fprintf(stderr, "Error reading from document\n");
		rc = 0;
	}
	if ( rc == 0 ) {
		doc->file = ys_run_cmd(doc->filter->cmd, name, logicalname);
		if (doc->file == NULL) {
			rc = -1;
		}
		doc->errcnt = 0;
	}
	return rc;
}

/**
 * Get the next word from a text file.
 */
static const char *
text_getword( ys_query_document_t *d, char *word, size_t maxlen )
{
	int ch;
	int inword = 0;
	size_t wordlen = 0;
	FILE *fp = d->file;

	while ((ch = fgetc(fp)) != EOF) {
		int wordchar = isalnum(ch) || ch == '_';
		if (wordchar) {
			if (!inword) {
				wordlen = 0;
				word[wordlen++] = tolower(ch);
				inword = 1;
			}
			else {
				if (wordlen < maxlen-1)
					word[wordlen++] = tolower(ch);
			}
		}
		else {
			if ((ch < 32 || ch > 127) &&
			    !isspace(ch)) {
				if (d->errcnt >= 20)
					/* probably a binary file */
					return NULL;
				d->errcnt++;
			}
			if (inword) {
				word[wordlen] = 0;
				inword = 0;
				assert(wordlen < maxlen);
				return word;
			}
		}
	}
	return NULL;
}

static void
init_docfile(
	ys_query_document_t *doc, 
	ys_docdata_t *docfile,
	const char *type)
{
	struct stat statbuf = {0};

	memset(docfile, 0, sizeof *docfile);
	strncpy(docfile->filename, doc->filename, sizeof docfile->filename);
	strncpy(docfile->logicalname, doc->logicalname, sizeof docfile->logicalname);
	snprintf(docfile->title, sizeof docfile->title,
		"%s", filebasename(doc->logicalname));
	snprintf(docfile->type, sizeof docfile->type, type);
	if (stat(doc->filename, &statbuf) == 0) {
		time_t tt;
		struct tm *tm;

		snprintf(docfile->size, sizeof docfile->size, 
			"%ld", statbuf.st_size);
		tt = statbuf.st_ctime;
		tm = localtime(&tt);
		strftime(docfile->datecreated, sizeof docfile->datecreated,
			"%D %R", tm);
	}
}

/**
 * Text file reader
 */
static int
text_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	ys_docdata_t docfile = {0};
	struct stat statbuf = {0};
	char word[256];
	const char *wordp;

	init_docfile(doc, &docfile, "TEXT");
	if (ys_dbaddfile(docdb, &docfile) == 0) {
		unsigned long docnum;
		int rc = ys_dbadddocptr(docdb, &docfile, 0, &docnum);
		while ((wordp = 
			text_getword(doc, word+1, sizeof word-1)) != 0) {
			word[0] = strlen(wordp);
			if (pfn_index(arg, word, docnum) != 0)
				break;
		}
#if !_NEWSTYLE_DOCWT
		ys_dbadddocweight(docdb, docnum, 
			ys_mkdb_calculate_document_weight(arg, docnum));
#endif
	}
	return 0;
}

/**
 * Extract words from a document and add to the YASE index.
 */
int 
ys_document_process(
	const char *logicalnamep, 
	const char *filename, 
	ys_docdb_t *docdb, 
	ys_pfn_index_t ptrfunc, 
	ys_mkdb_t *arg) 
{
	ys_query_document_t *doc = 0;
	int rc = 0;
	char logicalname[1024];

	ys_url_decode_string(logicalnamep, logicalname, sizeof logicalname);
	doc = (ys_query_document_t *)document_open(logicalname, filename);
	if (doc == 0)
		return -1;

	printf("Processing %s\n", logicalname);
	if (doc->filter == 0) {
		if (strendswith(filename, ".html") ||
		    strendswith(filename, ".htm"))
			rc = html_processor(doc, docdb, ptrfunc, arg);
		else 
			rc = text_processor(doc, docdb, ptrfunc, arg);
	}
	else {
		if (doc->filter->generates_xml)
			rc = xml_processor(doc, docdb, ptrfunc, arg);
		else if (doc->filter->generates_html)
			rc = my_html_processor(doc, docdb, ptrfunc, arg);
		else
			rc = text_processor(doc, docdb, ptrfunc, arg);
	}
	
	document_close(doc);
	return rc;
}

static int
my_html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	ys_docdata_t docfile = {0};
	char word[256];
	const char *wordp;
	ys_html_data_t htdata = {0};

	if ( ys_html_get_data(doc->file, &htdata) != 0 )
		return -1;
	if ( document_reopen(doc) != 0 )
		return -1;

	init_docfile(doc, &docfile, "HTML");
	if (htdata.title != 0 && strlen(htdata.title) > 0) {
		strncpy(docfile.title, htdata.title, 
			sizeof docfile.title);
	}
	if (htdata.author != 0 && strlen(htdata.author) > 0) {
		strncpy(docfile.author, htdata.author, 
			sizeof docfile.author);
	}
	if (htdata.keywords != 0 && strlen(htdata.keywords) > 0) {
		strncpy(docfile.keywords, htdata.keywords, 
			sizeof docfile.keywords);
	}
	if (ys_dbaddfile(docdb, &docfile) == 0) {
		unsigned long docnum;
		int rc = ys_dbadddocptr(docdb, &docfile, 0, &docnum);
		while ((wordp = 
			text_getword(doc, word+1, sizeof word-1)) != 0) {
			word[0] = strlen(wordp);
			if (pfn_index(arg, word, docnum) != 0)
				break;
		}
#if !_NEWSTYLE_DOCWT
		ys_dbadddocweight(docdb, docnum, 
			ys_mkdb_calculate_document_weight(arg, docnum));
#endif
	}
	return 0;
}

#if USE_LIBXML

#include <libxml/xmlmemory.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/debugXML.h>

typedef struct {
	ys_docdata_t docfile;
	ys_docdata_t doc;
	ys_docdb_t *docdb;
	ys_pfn_index_t pfn_index;
	ys_mkdb_t *arg;
	unsigned long docnum;
	int pass;
	int intitle;
	int infile;
	int indoc;
	int docfile_added;
	char word[256];
	int n;
	int inword;
} htmlctx_t;
typedef htmlctx_t xmlctx_t;

static void
reset_tokenizer(
	void *ctx)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	hctx->n = 1;
	hctx->inword = 0;
	hctx->word[0] = 0;
	hctx->word[1] = 0;
}

static void
complete_token(
	void *ctx)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	hctx->word[hctx->n] = 0;
	hctx->word[0] = strlen(hctx->word+1);
	hctx->pfn_index(hctx->arg, hctx->word, hctx->docnum);
	reset_tokenizer(ctx);
}

static void 
scan_tokens(void *ctx, const xmlChar *ch, int len)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	int i;

	for (i = 0; i < len; i++) {
		if (hctx->inword && !isalnum(ch[i])) {
			complete_token(ctx);
		}
		else if (hctx->inword && isalnum(ch[i])) {
			if (hctx->n < sizeof hctx->word-1)
				hctx->word[hctx->n++] = tolower(ch[i]);
		}
		else if (!hctx->inword && isalnum(ch[i])) {
			hctx->word[hctx->n++] = tolower(ch[i]);
			hctx->inword = 1;
		}
	}
}

static int 
is_standalone(void *ctx) 
{ 
	return(0); 
}

static int 
has_internal_subset(void *ctx) 
{ 
	return(0); 
}

static int 
has_external_subset(void *ctx) 
{ 
	return(0); 
}

static void 
internal_subset(void *ctx, const xmlChar *name,
	       const xmlChar *ExternalID, const xmlChar *SystemID)
{ 
}

static xmlParserInputPtr
resolve_entity(void *ctx, const xmlChar *publicId, const xmlChar *systemId)
{ 
	return(NULL); 
}

static xmlEntityPtr 
get_entity(void *ctx, const xmlChar *name)
{ 
	return(NULL); 
}

static xmlEntityPtr 
get_parameter_entity(void *ctx, const xmlChar *name)
{ 
	return(NULL); 
}

static void 
entity_decl(void *ctx, const xmlChar *name, int type,
          const xmlChar *publicId, const xmlChar *systemId, xmlChar *content)
{ 
}

static void 
attribute_decl(void *ctx, const xmlChar *elem, const xmlChar *name,
              int type, int def, const xmlChar *defaultValue,
	      xmlEnumerationPtr tree)
{ 
}

static void 
element_decl(void *ctx, const xmlChar *name, int type,
	    xmlElementContentPtr content)
{ 
}

static void 
notation_decl(void *ctx, const xmlChar *name,
	     const xmlChar *publicId, const xmlChar *systemId)
{ 
}

static void 
unparsed_entity_decl(void *ctx, const xmlChar *name,
		   const xmlChar *publicId, const xmlChar *systemId,
		   const xmlChar *notationName)
{ 
}

static void 
set_document_locator(void *ctx, xmlSAXLocatorPtr loc)
{ 
}

static void 
start_document(void *ctx) 
{ 
}

static void 
end_document(void *ctx) 
{ 
}

static void 
start_element_html(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	if (hctx->pass == 1) {
		if (strcasecmp(name, "title") == 0) {
			hctx->intitle = 1;
		}
		else if (strcasecmp(name, "h1") == 0 && 
			hctx->docfile.title[0] == 0) {
			hctx->intitle = 2;
		}
	}
	else if (hctx->pass == 2) {	
		reset_tokenizer(ctx);
	}
}

static void 
end_element_html(void *ctx, const xmlChar *name)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	if (hctx->pass == 1) {
		if (hctx->intitle == 1 && strcasecmp(name, "title") == 0) {
			hctx->intitle = 0;
		}
		else if (hctx->intitle == 2 && strcasecmp(name, "h1") == 0) {
			hctx->intitle = 0;
		}
	}
	else if (hctx->pass == 2) {
		if (hctx->inword) {
			complete_token(ctx);
		}
	}
}

static void 
characters_html(void *ctx, const xmlChar *ch, int len)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	int i;

	if (hctx->pass == 1 && hctx->intitle) {
		char *cp;
		while (isspace(*ch) && len > 0) {
			len--;
			ch++;
		}	
		snprintf(hctx->docfile.title, sizeof hctx->docfile.title,
			"%.*s", len, ch);
		while ((cp = strchr(hctx->docfile.title, '\n')) != 0)
			*cp = ' ';
		while ((cp = strchr(hctx->docfile.title, '\r')) != 0)
			*cp = ' ';
	}
	else if (hctx->pass == 2) {	
		scan_tokens(ctx, ch, len);
	}
}

static void 
do_reference(void *ctx, const xmlChar *name) 
{ 
}

static void 
ignorable_whitespace(void *ctx, const xmlChar *ch, int len) 
{ 
}

static void 
processing_instruction(void *ctx, const xmlChar *target,
                      const xmlChar *data)
{ 
}

static void 
do_comment(void *ctx, const xmlChar *value)
{ 
}

static void 
warning(void *ctx, const char *msg, ...) 
{ 
}

static void 
error(void *ctx, const char *msg, ...) 
{ 
}

static void 
fatal_error(void *ctx, const char *msg, ...) 
{ 
}

static xmlSAXHandler html_handler_struct = {
    internal_subset, is_standalone, has_internal_subset,
    has_external_subset, resolve_entity, get_entity,
    entity_decl, notation_decl, attribute_decl,
    element_decl, unparsed_entity_decl, set_document_locator,
    start_document, end_document, start_element_html,
    end_element_html, do_reference, characters_html,
    ignorable_whitespace, processing_instruction,
    do_comment, warning, error, fatal_error, 
    get_parameter_entity,
};

static xmlSAXHandlerPtr html_handler = &html_handler_struct;

/**
 * Read an HTML page, extract words and index them.
 * An HTML page is read in two passes. In the first pass, the title is established.
 * Actual indexing is carried out in the second pass.
 */
static int
html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	htmlDocPtr htmldoc;
	htmlctx_t hctx = {0};

	fclose(doc->file);
	doc->file = 0;

	hctx.docdb = docdb;
	hctx.pfn_index = pfn_index;
	hctx.arg = arg;
	hctx.pass = 1;
	hctx.intitle = 0;
	init_docfile(doc, &hctx.docfile, "HTML");
	htmldoc = htmlSAXParseFile(doc->filename, NULL, 
		html_handler, &hctx);
	if (htmldoc != NULL) {
		xmlFreeDoc(htmldoc);
	}
	if (ys_dbaddfile(docdb, &hctx.docfile) == 0) {
		int rc = ys_dbadddocptr(docdb, &hctx.docfile, 0, &hctx.docnum);
		hctx.pass = 2;
		htmldoc = htmlSAXParseFile(doc->filename, NULL, 
			html_handler, &hctx);
		if (htmldoc != NULL) {
			xmlFreeDoc(htmldoc);
		}
		xmlCleanupParser();
#if !_NEWSTYLE_DOCWT
		ys_dbadddocweight(docdb, hctx.docnum,
			ys_mkdb_calculate_document_weight(arg, hctx.docnum));
#endif
	}
	return 0;
}

static void 
start_element_xml(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	xmlctx_t *hctx = (xmlctx_t *)ctx;
	int i;
	if (strcasecmp(name, "yasefile") == 0) {
		hctx->infile = 1;
		for (i = 0; atts[i] != 0; i++) {
			if (strcasecmp(atts[i], "title") == 0) {
				strncpy(hctx->docfile.title,
					atts[++i], 
					sizeof hctx->docfile.title);	 
			}
			else if (strcasecmp(atts[i], "author") == 0) {
				strncpy(hctx->docfile.author,
					atts[++i], 
					sizeof hctx->docfile.author);	 
			}
			else if (strcasecmp(atts[i], "datecreated") == 0) {
				strncpy(hctx->docfile.datecreated,
					atts[++i], 
					sizeof hctx->docfile.datecreated); 
			}
			else if (strcasecmp(atts[i], "type") == 0) {
				strncpy(hctx->docfile.type,
					atts[++i], 
					sizeof hctx->docfile.type); 
			}
			else if (strcasecmp(atts[i], "keywords") == 0) {
				strncpy(hctx->docfile.keywords,
					atts[++i], 
					sizeof hctx->docfile.keywords); 
			}
			else {
				i++;
			}
		}
	}
	else if (strcasecmp(name, "yasedoc") == 0 && hctx->infile) {
		hctx->indoc = 1;
		for (i = 0; atts[i] != 0; i++) {
			if (strcasecmp(atts[i], "title") == 0) {
				strncpy(hctx->doc.title,
					atts[++i], 
					sizeof hctx->doc.title);	 
			}
			else if (strcasecmp(atts[i], "anchor") == 0) {
				strncpy(hctx->doc.anchor,
					atts[++i], 
					sizeof hctx->doc.anchor);	 
			}
			else if (strcasecmp(atts[i], "keywords") == 0) {
				strncpy(hctx->doc.keywords,
					atts[++i], 
					sizeof hctx->doc.keywords); 
			}
			else {
				i++;
			}
		}
		if (!hctx->docfile_added) {
			ys_dbaddfile(hctx->docdb, &hctx->docfile);
			hctx->docfile_added = 1;
		}	
		ys_dbadddoc(hctx->docdb, &hctx->docfile, &hctx->doc);
		ys_dbadddocptr(hctx->docdb, &hctx->docfile, 
			&hctx->doc, &hctx->docnum);
	}
	reset_tokenizer(ctx);
}

static void 
end_element_xml(void *ctx, const xmlChar *name)
{
	xmlctx_t *hctx = (xmlctx_t *)ctx;
	if (hctx->indoc && strcasecmp(name, "yasedoc") == 0) {
		hctx->indoc = 0;
		if (hctx->inword) {
			complete_token(ctx);
		}
#if !_NEWSTYLE_DOCWT
		ys_dbadddocweight(hctx->docdb, hctx->docnum,
			ys_mkdb_calculate_document_weight(hctx->arg, hctx->docnum));
#endif
	}
	else if (hctx->infile && strcasecmp(name, "yasefile") == 0) {
		hctx->infile = 0;
	}
}

static void 
characters_xml(void *ctx, const xmlChar *ch, int len)
{
	xmlctx_t *hctx = (xmlctx_t *)ctx;
	int i;

	if (!hctx->indoc)
		return;
	scan_tokens(ctx, ch, len);
}

static xmlSAXHandler xml_handler_struct = {
    internal_subset, is_standalone, has_internal_subset,
    has_external_subset, resolve_entity, get_entity,
    entity_decl, notation_decl, attribute_decl,
    element_decl, unparsed_entity_decl, set_document_locator,
    start_document, end_document, start_element_xml,
    end_element_xml, do_reference, characters_xml,
    ignorable_whitespace, processing_instruction,
    do_comment, warning, error, fatal_error, 
    get_parameter_entity,
};

static xmlSAXHandlerPtr xml_handler = &xml_handler_struct;

static int
xml_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	xmlctx_t hctx = {0};
	int rc;
	char chars[10];
	xmlParserCtxtPtr ctxt;
	FILE *f = doc->file;

	hctx.docdb = docdb;
	hctx.pfn_index = pfn_index;
	hctx.arg = arg;
	hctx.infile = 0;
	hctx.indoc = 0;
	hctx.docfile_added = 0;
	init_docfile(doc, &hctx.docfile, "UNKNOWN");
	rc = fread(chars, 1, 4, f);
	if (rc > 0) {
		ctxt = xmlCreatePushParserCtxt(xml_handler, &hctx,
			chars, rc, "<stdin>");
		while ((rc = fread(chars, 1, 3, f)) > 0) {
			xmlParseChunk(ctxt, chars, rc, 0);
		}
		rc = xmlParseChunk(ctxt, chars, 0, 1);
		xmlFreeParserCtxt(ctxt);
		if (rc != 0) {
			fprintf(stderr,
				"xmlParseChunk returned error %d\n", rc);
		}
	}
	xmlCleanupParser();
	return 0;
}

#else

static int
xml_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	return text_processor(doc, docdb, pfn_index, arg);
}

static int
html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	return my_html_processor(doc, docdb, pfn_index, arg); 
}

#endif

#ifdef TESTING_GETFILTER

const char *ys_get_config(const char *name) {
	if (strcmp(name, "gz.filter") == 0) {
		return "pipe;gunzip -c";
	}
	else if (strcmp(name, "doc.filter") == 0) {
		return "xml;catdoc|htm2xml";
	}
	else if (strcmp(name, "txt.filter") == 0) {
		return "text;cat";
	}
	else if (strcmp(name, "c.filter") == 0) {
		return "text;cat";
	}
	return 0;
}

int main() {
	ys_query_document_t *doc;
	filter_t *f = ys_get_filter("gz");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("c.gz");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("txt.gz");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("doc.gz");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("c");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("doc");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);
	f = ys_get_filter("doc.gz");
	fprintf(stderr, "%s=%s,xml=%d\n", f->ext, f->cmd, f->generates_xml);

	doc = document_open("xx.c.gz");
	document_close(doc);
	assert(strendswith("mydoc.tar.gz", "gz"));
	return 0;
}

#endif
