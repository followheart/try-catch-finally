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
/* 26 Jan 2003: YaseQuery class created */

#include "query.h"
#include "util.h"
#include "properties.h"

#ifndef WIN32
extern char ** environ;
#endif

YASE_NS_BEGIN

class YaseQuery {
private:
	YASENS Collection *collection;
	YASENS Properties *prop;
	YASENS QueryForm *form;
	YASENS QueryInput *input;
	YASENS QueryOutput *output;
public:
	YaseQuery();
	~YaseQuery();
	bool loadProperties(const char *yasequery_location);
	void dumpEnv();
	int process(int argc, const char *argv[]);
};

YASE_NS_END

YASENS YaseQuery::YaseQuery()
{
	output = new YASENS HtmlOutput();
	prop = new YASENS Properties();
	form = new YASENS QueryForm();
	input = new YASENS WebInput();
	collection = new YASENS Collection();
	output->start();
}

YASENS YaseQuery::~YaseQuery()
{
	output->end();
	delete prop;
	delete collection;
	delete input;
	delete form;
	delete output;
}

bool
YASENS YaseQuery::loadProperties(const char *yasequery_location)
{
	char pathname[1024];
	strncpy(pathname, yasequery_location, sizeof pathname);
	char *cp = strchr(pathname, '\\');
	while (cp != 0) {
		*cp = '/';
		cp = strchr(pathname, '\\');
	}
	cp = strrchr(pathname, '/');
	if (cp != 0) {
		*cp = 0;
	}
	else
		return false;
	char filename[1024];
	snprintf(filename, sizeof filename, "%s/yasequery.properties", pathname);
	return prop->load(filename);
}

void
YASENS YaseQuery::dumpEnv()
{
	int j;
	for (j = 0; environ[j] != 0; j++) {
		output->message("%s\n", environ[j]);
	}
}

int
YASENS YaseQuery::process(int argc, const char *argv[])
{
	int rc = 0;
	const char *collection_path = 0;
	if (! loadProperties(argv[0])) {
		output->message("Failed to load yasequery.properties\n");
		return 1;
	}
	input->getInput(form);
	collection_path = prop->get(form->getCollectionPath());
	if (collection_path == 0) {
		output->message("Unrecognised Collection %s\n", form->getCollectionPath());
		return 1;
	}
	if (collection->open(collection_path, "r") != 0) {
		output->message("Failed to open database\n");
		return 1;
	}
	YASENS QueryAction action;
	YASENS SearchResultSet *rs = action.doSearch(collection, form);
	if (rs != 0) {
		output->doOutput(collection, form, input, rs);
		delete rs;
	}
	else {
		output->message("Failed to process query\n");
		rc = 1;
	}
	return rc;
}

int 
main(int argc, const char *argv[])
{
	YASENS YaseQuery yq;
	return yq.process(argc, argv);
}
