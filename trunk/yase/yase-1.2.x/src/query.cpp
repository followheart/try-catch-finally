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

/* 12-22 Jan 2003: Converted to C++ from old C stuff */

#include "query.h"
#include "util.h"
#include "properties.h"

YASENS QueryForm::QueryForm()
{
	collection_path[0] = 0;
	queryexpr[0] = 0;
	method = YASENS Search::SM_RANKED;
	curpage = 1;
	pagesize = 10;
	dumpenv = false;
}

YASENS QueryForm::~QueryForm()
{
}

static const char *
safe_getenv(const char *name)
{
	const char *p = getenv(name);
	if (p == 0)
		p = "";
	return p;
}

YASENS WebInput::WebInput()
{
	buf = bufp = 0;
	referrer = safe_getenv("HTTP_REFERER");
	host = safe_getenv("HTTP_HOST");
	documentroot = safe_getenv("DOCUMENT_ROOT");
	requestmethod = safe_getenv("REQUEST_METHOD");
	scriptname = safe_getenv("SCRIPT_NAME");
	const char *cl = safe_getenv("CONTENT_LENGTH");
	contentlength = atol(cl);
	if (strcasecmp(requestmethod, "get") == 0)
		querystring = (ys_uchar_t *) strdup(safe_getenv("QUERY_STRING"));
	else {
		querystring = (ys_uchar_t *) malloc(contentlength);
		size_t n = fread(querystring, 1, contentlength, stdin);
		// if (n != (size_t)contentlength)
		//	ERROR
	}
}

YASENS WebInput::~WebInput()
{
	if (buf != 0)
		free(buf);
	if (querystring != 0)
		free(querystring);
}

/**
 * Parse a QUERY_STRING and return the next field/value pair.
 * Semantics similar to strtok() - the input 'line' should be passed
 * the first time only; each call returns the next pair until all 
 * pairs are exhausted and NULL is returned.
 */ 
bool
YASENS WebInput::getNextField(
	const ys_uchar_t *line, 
	ys_uchar_t *name, size_t namelen,
	ys_uchar_t *value, size_t valuelen)
{
	ys_uchar_t *cp;
	ys_uchar_t *token;
	unsigned int i;

	if (line != 0) {
		if (buf != 0)
			free(buf);
		buf = (ys_uchar_t *) calloc(1, strlen((const char *)line)+1);
		if (buf == 0) {
			// ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			//		"Error: failed to allocate memory\n");
			bufp = buf;
			return false;
		}
		strcpy((char *)buf, (const char *)line);
		bufp = buf;
	}

	if (bufp == 0 || *bufp == 0)
		return false;

	cp = (ys_uchar_t *) strchr((char *)bufp, '&');
	if (cp != 0) {
		*cp++ = 0;
	}
	token = bufp;
	bufp = cp;

	cp = (ys_uchar_t *) strchr((char *)token, '=');
	if (cp == 0) {
		bufp = 0;
		return false;
	}
	*cp++ = 0;
	strncpy((char *)name, (const char *)token, namelen);
	name[namelen-1] = 0;

	/* TODO: Following should be replaced by a call to ys_url_decode_string,
	 * but, ys_url_decode_string doesn't replace pluses with spaces.
	 */
	token = value;
	for (i = 0; i < valuelen-1; i++) {
		if (*cp) {
			if (cp[0] == '%' && isxdigit(cp[1]) && isxdigit(cp[2])) {
				int ch = ys_url_decode_char((const char *)cp);
				cp += 3;
				*token++ = ch;
			}
			else if (*cp == '+') {
				*token++ = ' ';
				cp++;
			}
			else
				*token++ = *cp++;
		}
		else {
			break;
		}
	}
	value[i] = 0;
	return true;
}

/**
 * Parse the standard query parameters.
 * 15-feb-2002 modified to accept short names.
 */
bool
YASENS WebInput::processParameter(QueryForm *form, const ys_uchar_t *name, const ys_uchar_t *value)
{
	if (strcmp((const char *)name, "de") == 0 ||
		strcmp((const char *)name, "dump_env") == 0) {
		form->setDumpEnv(*value == 'y');
	}
	else if (strcmp((const char *)name, "sm") == 0 ||
		 strcmp((const char *)name, "searchmethod") == 0) {
		if (strcmp((const char *)value, "boolean") == 0) {
			form->setMethod(YASENS Search::SM_BOOLEAN);
		}
		else {
			form->setMethod(YASENS Search::SM_RANKED);
		}
	}
	else if (strcmp((const char *)name, "ps") == 0 ||
		 strcmp((const char *)name, "pagesize") == 0) {
		int pagesize = atoi((const char *)value);
		if (pagesize < 5)
			pagesize = 5;
		form->setPageSize(pagesize);
	}
	else if (strcmp((const char *)name, "cp") == 0 ||
	         strcmp((const char *)name, "curpage") == 0) {
		int curpage = atoi((const char *)value);
		if (curpage < 1)
			curpage = 1;
		form->setCurrentPage(curpage);
	}
	else if (strcmp((const char *)name, "q") == 0 ||
		strcmp((const char *)name, "query") == 0) {
		form->setQueryExpr(value);
	}
	else if (strcmp((const char *)name, "yp") == 0 ||
		strcmp((const char *)name, "collection_path") == 0) {
		form->setCollectionPath((const char *)value);
	}
#if 0
	else if (strcmp((const char *)name, "hn") == 0 ||
		strcmp((const char *)name, "host_name") == 0) {
		strncpy(query->hostname, value,
			sizeof query->hostname);
	}
	else if (strcmp((const char *)name, "hr") == 0 ||
		 strcmp((const char *)name, "http_referer") == 0) {
		query->http_referer = strdup(value);
	}
#endif
	return true;
}

bool
YASENS WebInput::getInput(QueryForm *form)
{
	ys_uchar_t name[31];
	ys_uchar_t value[4096];
	const ys_uchar_t *query = getQueryString();

	while (getNextField(query, name, sizeof name, value, sizeof value)) {
		processParameter(form, name, value);
		query = 0;
	}
	return true;
}

bool
YASENS TestInput::getInput(QueryForm *form)
{
	form->setCollectionPath("../yasemakedb");
	form->setMethod(YASENS Search::SM_RANKED);
	form->setQueryExpr((const ys_uchar_t *)"nine days old porridge");
	return true;
}

YASENS QueryAction::QueryAction()
{
}

YASENS QueryAction::~QueryAction()
{
}

YASENS SearchResultSet *
YASENS QueryAction::doSearch(YASENS Collection *collection, YASENS QueryForm *form)
{
	YASENS Search *search = YASENS Search::createSearch(collection, form->getMethod());
	if (search == 0)
		return 0;
	search->addInput(form->getQueryExpr());
	YASENS SearchResultSet *rs = 0;
	if (search->parseQuery()) {
		rs = search->executeQuery();
	}
	delete search;
	return rs;
}

void
YASENS ConsoleOutput::doOutput(YASENS Collection *collection, YASENS QueryForm *form, YASENS QueryInput *input, YASENS SearchResultSet *rs)
{
	ys_docdb_t *db = collection->getDocDb();
	ys_docdata_t docfile, doc;
	if (rs != 0) {
		YASENS SearchResultItem *item = rs->getNext();
		while (item != 0) {
			ys_dbgetdocumentref(db, item->getDocnum(),
				&docfile, &doc);
			item->dump(stdout);
			fprintf(stdout, "Filename %s, Title %s\n",
				docfile.filename, docfile.title);
			item = rs->getNext();
		}
	}
}

