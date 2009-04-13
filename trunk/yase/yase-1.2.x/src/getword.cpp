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
* DM 03-01-03 started converting to C++, and used tokenizer classes
* DM 04-01-03 New C++ classes to represent SaxParser, HtmlParser and XmlParser.
*/

#include "getword.h"
#include "getconfig.h"
#include "list.h"
#include "xmlparser.h"
#include "util.h"
#include "tokenizer.h"
#include "saxparser.h"

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
static void init_docfile(ys_query_document_t *doc, ys_docdata_t *docfile,
	const char *type);
static int text_extract_words(ys_docdata_t *docfile, ys_query_document_t *doc, 
	ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg);
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
		f->cmd = (char *)realloc(f->cmd, new_len);
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
	f->cmd = (char *)realloc(f->cmd, new_len);
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
	return strcasecmp(s+len1-len2, suffix) == 0;
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

	d = (ys_query_document_t *) calloc(1, sizeof *d);
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
		d->file = fopen(name, "rb");
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
text_extract_words(ys_docdata_t *docfile, ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	ys_uchar_t word[YS_TERM_LEN];
	const ys_uchar_t *wordp;
	bool skippingBinaryFiles = ys_mkdb_get_skip_binary_files( arg );

	if (ys_dbaddfile(docdb, docfile) == 0) {
		unsigned long docnum;
		int rc = ys_dbadddocptr(docdb, docfile, 0, &docnum);
		/** TODO: FIXME **/
		ys_mkdb_set_curdocnum( arg, docnum );
		FILE *file = doc->file;
		ys_uchar_t buf[BUFSIZ];
		YASENS StringTokenizer st;
		const ys_uchar_t *cp;
		// while (fgets(buf, sizeof buf, file) != 0) {
		size_t n = 0;
		while ((n = fread(buf, 1, sizeof buf, file)) > 0) {
			st.addInput(buf, n);
			// st.addInput(buf);
			cp = st.nextToken();
			while (cp != 0 && (!skippingBinaryFiles || st.countBinary() < 50)) {
				strncpy((char *)word+1, (const char *)cp, sizeof word-1);
				word[0] = strlen((const char *)word+1);
				if (pfn_index(arg, word, docnum) != 0)
					goto done;
				cp = st.nextToken();
			}
		}
		cp = st.endInput();
		if (cp != 0 && (!skippingBinaryFiles || st.countBinary() < 50)) {
			strncpy((char *)word+1, (const char *)cp, sizeof word-1);
			word[0] = strlen((const char *)word+1);
			if (pfn_index(arg, word, docnum) != 0)
				goto done;
		}
	}
done:
	return 0;
}

/**
 * Text file reader
 */
static int
text_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	ys_docdata_t docfile = {0};
	init_docfile(doc, &docfile, "TEXT");
	return text_extract_words(&docfile, doc, docdb, pfn_index, arg);
}

static int
my_html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	ys_docdata_t docfile = {0};
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
	return text_extract_words(&docfile, doc, docdb, pfn_index, arg);
}

#if USE_LIBXML

YASE_NS_BEGIN

class HtmlParser : public SaxParser {
protected:
	ys_docdb_t *docdb;
	ys_pfn_index_t pfn_index;
	ys_mkdb_t *arg;
	ys_docdata_t docfile;
	ys_docdata_t doc;
	unsigned long docnum;
	int pass;
	int intitle;
	ys_uchar_t word[YS_TERM_LEN];
	YASENS UTF8ToAsciiTokenizer st;

public:
	HtmlParser(ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg);

	virtual void 
	startElement(const xmlChar *name, const xmlChar **atts);

	virtual void 
	endElement(const xmlChar *name);

	virtual void 
	characters(const xmlChar *ch, int len);

	virtual void
	parse(ys_query_document_t *doc);

protected:
	void
	scanTokens(const xmlChar *ch, int len);

	void 
	trailingWord();
};

class XmlParser : public HtmlParser {
protected:
	bool docfile_added;
	bool infile;
	bool indoc;

public:
	XmlParser(ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg);

	virtual void 
	startElement(const xmlChar *name, const xmlChar **atts);

	virtual void 
	endElement(const xmlChar *name);

	virtual void 
	characters(const xmlChar *ch, int len);

	virtual void
	parse(ys_query_document_t *doc);
};

YASE_NS_END

void 
YASENS HtmlParser::startElement(const xmlChar *name, const xmlChar **atts)
{
	if (pass == 1) {
		if (strcasecmp((const char *)name, "title") == 0) {
			intitle = 1;
		}
		else if (strcasecmp((const char *)name, "h1") == 0 && 
			docfile.title[0] == 0) {
			intitle = 2;
		}
	}
	else if (pass == 2) {	
		st.reset();
	}
}

void 
YASENS HtmlParser::endElement(const xmlChar *name)
{
	if (pass == 1) {
		if (intitle == 1 && strcasecmp((const char *)name, "title") == 0) {
			intitle = 0;
		}
		else if (intitle == 2 && strcasecmp((const char *)name, "h1") == 0) {
			intitle = 0;
		}
	}
	else if (pass == 2) {
		trailingWord();
	}
}

void 
YASENS HtmlParser::characters(const xmlChar *ch, int len)
{
	int i;

	if (pass == 1 && intitle) {
		char *cp;
		while (isspace(*ch) && len > 0) {
			len--;
			ch++;
		}	
		snprintf(docfile.title, sizeof docfile.title,
			"%.*s", len, ch);
		while ((cp = strchr(docfile.title, '\n')) != 0)
			*cp = ' ';
		while ((cp = strchr(docfile.title, '\r')) != 0)
			*cp = ' ';
	}
	else if (pass == 2) {	
		scanTokens(ch, len);
	}
}

void
YASENS HtmlParser::scanTokens(const xmlChar *ch, int len)
{
	st.addInput(ch, len);
	const ys_uchar_t *cp = st.nextToken();
	while (cp != 0) {
		strncpy((char *)word+1, (const char *)cp, sizeof word-1);
		word[0] = strlen((const char *)word+1);
		pfn_index(arg, word, docnum);
		cp = st.nextToken();
	}
}

void 
YASENS HtmlParser::trailingWord()
{
	const ys_uchar_t *cp = st.endInput();
	if (cp != 0) {
		strncpy((char *)word+1, (const char *)cp, sizeof word-1);
		word[0] = strlen((const char *)word+1);
		pfn_index(arg, word, docnum);
	}
}

YASENS HtmlParser::HtmlParser(
	ys_docdb_t *docdb, 
	ys_pfn_index_t pfn_index, 
	ys_mkdb_t *arg)
{
	memset(&docfile, 0, sizeof docfile);
	memset(&doc, 0, sizeof doc);
	docnum = 0;
	pass = 1;
	intitle = 0;
	word[0] = 0;
	this->docdb = docdb;
	this->pfn_index = pfn_index;
	this->arg = arg;
}
	
/**
 * Read an HTML page, extract words and index them.
 * An HTML page is read in two passes. In the first pass, the title is established.
 * Actual indexing is carried out in the second pass.
 */
void
YASENS HtmlParser::parse(ys_query_document_t *doc)
{
	htmlDocPtr htmldoc;

	fclose(doc->file);
	doc->file = 0;

	pass = 1;
	intitle = 0;
	init_docfile(doc, &docfile, "HTML");
	htmldoc = htmlSAXParseFile(doc->filename, NULL, 
		ys_get_saxp_handler(), (void *)this);
	if (htmldoc != NULL) {
		xmlFreeDoc(htmldoc);
	}
	if (ys_dbaddfile(docdb, &docfile) == 0) {
		int rc = ys_dbadddocptr(docdb, &docfile, 0, &docnum);
		/** TODO: FIXME **/
		ys_mkdb_set_curdocnum( arg, docnum );
		pass = 2;
		htmldoc = htmlSAXParseFile(doc->filename, NULL, 
			ys_get_saxp_handler(), (void *)this);
		if (htmldoc != NULL) {
			xmlFreeDoc(htmldoc);
		}
		xmlCleanupParser();
	}
}

void 
YASENS XmlParser::startElement(const xmlChar *name, const xmlChar **atts)
{
	int i;
	if (strcasecmp((const char *)name, "yasefile") == 0) {
		infile = true;
		for (i = 0; atts[i] != 0; i++) {
			if (strcasecmp((const char *)atts[i], "title") == 0) {
				strncpy(docfile.title,
					(const char *)atts[++i], 
					sizeof docfile.title);	 
			}
			else if (strcasecmp((const char *)atts[i], "author") == 0) {
				strncpy(docfile.author,
					(const char *)atts[++i], 
					sizeof docfile.author);	 
			}
			else if (strcasecmp((const char *)atts[i], "datecreated") == 0) {
				strncpy(docfile.datecreated,
					(const char *)atts[++i], 
					sizeof docfile.datecreated); 
			}
			else if (strcasecmp((const char *)atts[i], "type") == 0) {
				strncpy(docfile.type,
					(const char *)atts[++i], 
					sizeof docfile.type); 
			}
			else if (strcasecmp((const char *)atts[i], "keywords") == 0) {
				strncpy(docfile.keywords,
					(const char *)atts[++i], 
					sizeof docfile.keywords); 
			}
			else {
				i++;
			}
		}
	}
	else if (strcasecmp((const char *)name, "yasedoc") == 0 && infile) {
		indoc = true;
		for (i = 0; atts[i] != 0; i++) {
			if (strcasecmp((const char *)atts[i], "title") == 0) {
				strncpy(doc.title,
					(const char *)atts[++i], 
					sizeof doc.title);	 
			}
			else if (strcasecmp((const char *)atts[i], "anchor") == 0) {
				strncpy(doc.anchor,
					(const char *)atts[++i], 
					sizeof doc.anchor);	 
			}
			else if (strcasecmp((const char *)atts[i], "keywords") == 0) {
				strncpy(doc.keywords,
					(const char *)atts[++i], 
					sizeof doc.keywords); 
			}
			else {
				i++;
			}
		}
		if (!docfile_added) {
			ys_dbaddfile(docdb, &docfile);
			docfile_added = true;
		}	
		ys_dbadddoc(docdb, &docfile, &doc);
		ys_dbadddocptr(docdb, &docfile, 
			&doc, &docnum);
		/** TODO: FIXME **/
		ys_mkdb_set_curdocnum( arg, docnum );
	}
	st.reset();
}

void 
YASENS XmlParser::endElement(const xmlChar *name)
{
	if (indoc && strcasecmp((const char *)name, "yasedoc") == 0) {
		indoc = false;
		trailingWord();
	}
	else if (infile && strcasecmp((const char *)name, "yasefile") == 0) {
		infile = false;
	}
}

void 
YASENS XmlParser::characters(const xmlChar *ch, int len)
{
	if (!indoc)
		return;
	scanTokens(ch, len);
}

YASENS XmlParser::XmlParser(
	ys_docdb_t *docdb, 
	ys_pfn_index_t pfn_index, 
	ys_mkdb_t *arg) : HtmlParser(docdb, pfn_index, arg)
{
	docfile_added = false;
	infile = false;
	indoc = false;
}

void
YASENS XmlParser::parse(ys_query_document_t *doc)
{
	int rc;
	char chars[10];
	xmlParserCtxtPtr ctxt;
	FILE *f = doc->file;

	infile = false;
	indoc = false;
	docfile_added = false;
	init_docfile(doc, &docfile, "UNKNOWN");
	rc = fread(chars, 1, 4, f);
	if (rc > 0) {
		ctxt = xmlCreatePushParserCtxt(ys_get_saxp_handler(), (void *)this,
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
}

static int
html_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	YASENS HtmlParser parser(docdb, pfn_index, arg);
	parser.parse(doc);
	return 0;
}

static int
xml_processor(ys_query_document_t *doc, ys_docdb_t *docdb, ys_pfn_index_t pfn_index, ys_mkdb_t *arg)
{
	YASENS XmlParser parser(docdb, pfn_index, arg);
	parser.parse(doc);
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
