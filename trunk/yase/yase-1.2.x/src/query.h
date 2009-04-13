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

#ifndef query_h
#define query_h

#include "yase.h"
#include "avl3.h"
#include "btree.h"
#include "postfile.h"
#include "docdb.h"
#include "search.h"

YASE_NS_BEGIN

/**
 * QueryForm holds the data captured from the browser.
 */
class QueryForm {
protected:
	char collection_path[1024];
	ys_uchar_t queryexpr[4096];
	int method;
	int curpage;
	int pagesize;
	bool dumpenv;
public:
	QueryForm();
	~QueryForm();

	void setCollectionPath(const char *path) { strncpy(collection_path, path, sizeof collection_path); }
	void setQueryExpr(const ys_uchar_t *expr) { strncpy((char *)queryexpr, (const char *)expr, sizeof queryexpr); }
	void setMethod(int m) { method = m; }
	void setCurrentPage(int p) { curpage = p; }
	void setPageSize(int l) { pagesize = l; }
	void setDumpEnv(bool v) { dumpenv = v; }
	const char *getCollectionPath() const { return collection_path; }
	const ys_uchar_t *getQueryExpr() const { return queryexpr; }
	int getMethod() const { return method; }
	int getCurrentPage() const { return curpage; }
	int getPageSize() const { return pagesize; }
	bool getDumpEnv() const { return dumpenv; }
};

/**
 * QueryInput is an abstract class that represents a generic input source.
 */
class QueryInput {
protected:
	QueryInput() {}
public:
	virtual ~QueryInput() {}
	virtual bool getInput(QueryForm *form) = 0;
};

/**
 * WebInput implements CGI input.
 */
class WebInput : public QueryInput {
private:
	const char *referrer;
	const char *host;
	const char *documentroot;
	const char *requestmethod;
	int contentlength;
	ys_uchar_t *querystring;
	const char *scriptname;
private:
	ys_uchar_t *buf;
	ys_uchar_t *bufp;
private:
	bool getNextField(const ys_uchar_t *line, ys_uchar_t *name, size_t namelen, ys_uchar_t *value, size_t valuelen);
	bool processParameter(QueryForm *form, const ys_uchar_t *name, const ys_uchar_t *value);
public:
	WebInput();
	virtual ~WebInput();

	const char *getHttpReferrer() const { return referrer; }
	const char *getHttpHost() const { return host; }
	const char *getDocumentRoot() const { return documentroot; }
	const char *getRequestMethod() const { return requestmethod; }
	const ys_uchar_t *getQueryString() const { return querystring; }
	const char *getScriptName() const { return scriptname; }
	int getContentLength() const { return contentlength; }
	virtual bool getInput(QueryForm *form);
};

/**
 * TestInput is a simple input source useful for testing.
 */
class TestInput : public QueryInput {
public:
	TestInput() {}
	virtual ~TestInput() {}
	virtual bool getInput(QueryForm *form);
};

/**
 * Abstract base class that defines the interface for
 * Output processor.
 */
class QueryOutput
{
public:
	QueryOutput()
	{
	}
	virtual ~QueryOutput()
	{
	}
	virtual void doOutput(YASENS Collection *collection, QueryForm *form, QueryInput *input, YASENS SearchResultSet *rs) = 0;
	virtual void start() = 0;
	virtual void end() = 0;
	virtual void message(const char *fmt, ...) = 0;
};

/**
 * Html output handler.
 */
class HtmlOutput : public QueryOutput {
private:
	FILE *out;
	bool doRowHeader;
	class TemplateLoader *t;
	QueryForm *form;
	WebInput *input;
public:
	HtmlOutput();
	virtual ~HtmlOutput();
	virtual void doOutput(YASENS Collection *collection, QueryForm *form, QueryInput *input, YASENS SearchResultSet *rs);
	virtual void start();
	virtual void end();
	virtual void message(const char *fmt, ...);
private:
	void doHeader();
	void doFooter();
	void outputRow(
		const char *reference,		/* HTML Link */
		const char *anchor, 		/* Anchor */
		ys_docdata_t *docfile,		/* Document File */
		ys_docdata_t *doc, 		/* Document */
		double rank, 			/* Rank */
		int matchcount); 		/* Number of hits */
	const char * substLink(
		const char *fmt,
		char *buf,
		size_t buflen,
		const char *link);
	void outputSummary(
		int matches, 		/* Number of matced items */
		int curpage,		/* Current page */
		int pagecount, 		/* Total Page count */
		int pagesize,		/* Size of each page */
		double elapsed_time);
};


/** 
 * ConsoleOutput implements a simple output format to
 * stdout. Mostly useful for testing.
 */
class ConsoleOutput : public QueryOutput {
public:
	ConsoleOutput()
	{
	}
	virtual ~ConsoleOutput() {}
	virtual void doOutput(YASENS Collection *collection, QueryForm *form, QueryInput *input, YASENS SearchResultSet *rs);
	virtual void start() {}
	virtual void end() {}
	virtual void message(const char *fmt, ...) {}
};

class QueryAction
{
public:
	QueryAction();
	~QueryAction();
	YASENS SearchResultSet *doSearch(YASENS Collection *collection, QueryForm *form);
};	

YASE_NS_END
	
#endif
