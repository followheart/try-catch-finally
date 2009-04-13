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

/* created 16-Jan 2000 from wquery.c */
/* 29-apr-2000 Added ys_query_output_message() */
/* 15-may-2000 Added ys_query_output_summary() */
/* 25-nov-2000 Added support for URLs in ys_query_output_reference() */
/* 21-apr-2001 Modified to use ys_qout_t as the handle */
/* 15-feb-2002 Modified Page Navigation menu */
/* 16-feb-2002 Added support for html template file */
/* 16-feb-2002 Converted text output to xml format */
/* 18-22 Jan 2003: Converted to C++ from old C stuff */

#include "query.h"
#include "util.h"

enum {
	YS_TEMPLATE_HEADER = 0,
	YS_TEMPLATE_FOOTER,
	YS_TEMPLATE_ROWHEADER,
	YS_TEMPLATE_ROWFOOTER,
	YS_TEMPLATE_ROW,
	YS_TEMPLATE_FIRST_ENABLED,
	YS_TEMPLATE_FIRST_DISABLED,
	YS_TEMPLATE_NEXT_ENABLED,
	YS_TEMPLATE_NEXT_DISABLED,
	YS_TEMPLATE_PREV_ENABLED,
	YS_TEMPLATE_PREV_DISABLED,
	YS_TEMPLATE_LAST_ENABLED,
	YS_TEMPLATE_LAST_DISABLED,
	YS_TEMPLATE_NEW_QUERY,
	YS_TEMPLATE_LEN
};
static char *ys_html_sections[] = {
	"##ys_header##",
	"##ys_footer##",
	"##ys_rowheader##",
	"##ys_rowfooter##",
	"##ys_row##",
	"##ys_firstenabled##",
	"##ys_firstdisabled##",
	"##ys_nextenabled##",
	"##ys_nextdisabled##",
	"##ys_prevenabled##",
	"##ys_prevdisabled##",
	"##ys_lastenabled##",
	"##ys_lastdisabled##",
	"##ys_newquery##"
};

YASE_NS_BEGIN

class TemplateLoader {
	char *html_sections[YS_TEMPLATE_LEN];		
public:
	TemplateLoader();
	~TemplateLoader();
	bool loadTemplate(const char *path, const char *filename);
	const char *getHeader() const { return html_sections[YS_TEMPLATE_HEADER]; }
	bool hasHeader() const { return html_sections[YS_TEMPLATE_HEADER] != 0; }
	const char *getFooter() const { return html_sections[YS_TEMPLATE_FOOTER]; }
	bool hasFooter() const { return html_sections[YS_TEMPLATE_FOOTER] != 0; }
	const char *getRowHeader() const { return html_sections[YS_TEMPLATE_ROWHEADER]; }
	bool hasRowHeader() const { return html_sections[YS_TEMPLATE_ROWHEADER] != 0; }
	const char *getRowFooter() const { return html_sections[YS_TEMPLATE_ROWFOOTER]; }
	bool hasRowFooter() const { return html_sections[YS_TEMPLATE_ROWFOOTER] != 0; }
	const char *getRow() const { return html_sections[YS_TEMPLATE_ROW]; }
	bool hasRow() const { return html_sections[YS_TEMPLATE_ROW] != 0; }
	const char *getFirstEnabled() const { return html_sections[YS_TEMPLATE_FIRST_ENABLED]; }
	bool hasFirstEnabled() const { return html_sections[YS_TEMPLATE_FIRST_ENABLED] != 0; }
	const char *getFirstDisabled() const { return html_sections[YS_TEMPLATE_FIRST_DISABLED]; }
	bool hasFirstDisabled() const { return html_sections[YS_TEMPLATE_FIRST_DISABLED] != 0; }
	const char *getNextEnabled() const { return html_sections[YS_TEMPLATE_NEXT_ENABLED]; }
	bool hasNextEnabled() const { return html_sections[YS_TEMPLATE_NEXT_ENABLED] != 0; }
	const char *getNextDisabled() const { return html_sections[YS_TEMPLATE_NEXT_DISABLED]; }
	bool hasNextDisabled() const { return html_sections[YS_TEMPLATE_NEXT_DISABLED] != 0; }
	const char *getPrevEnabled() const { return html_sections[YS_TEMPLATE_PREV_ENABLED]; }
	bool hasPrevEnabled() const { return html_sections[YS_TEMPLATE_PREV_ENABLED] != 0; }
	const char *getPrevDisabled() const { return html_sections[YS_TEMPLATE_PREV_DISABLED]; }
	bool hasPrevDisabled() const { return html_sections[YS_TEMPLATE_PREV_DISABLED] != 0; }
	const char *getLastEnabled() const { return html_sections[YS_TEMPLATE_LAST_ENABLED]; }
	bool hasLastEnabled() const { return html_sections[YS_TEMPLATE_LAST_ENABLED] != 0; }
	const char *getLastDisabled() const { return html_sections[YS_TEMPLATE_LAST_DISABLED]; }
	bool hasLastDisabled() const { return html_sections[YS_TEMPLATE_LAST_DISABLED] != 0; }
	const char *getNewQuery() const { return html_sections[YS_TEMPLATE_NEW_QUERY]; }
	bool hasNewQuery() const { return html_sections[YS_TEMPLATE_NEW_QUERY] != 0; }
};

YASE_NS_END

YASENS TemplateLoader::TemplateLoader()
{
	/* initialise sections to null */
	for (int index = 0; index < YS_TEMPLATE_LEN; index++) 
		html_sections[index] = 0;
}

YASENS TemplateLoader::~TemplateLoader()
{
	/* initialise sections to null */
	for (int index = 0; index < YS_TEMPLATE_LEN; index++) 
		if (html_sections[index] != 0)
			free(html_sections[index]);
}

/**
 * This functions loads the contents of the template file into memory.
 * Each section is loaded into a buffer. The Buffer for each section is dynamically allocated.
 * 16-17 feb 2002.
 * 18-22 migrated to C++.
 */
bool
YASENS TemplateLoader::loadTemplate(const char *path, const char *templatename)
{
	char filename[1024];
	FILE *file;
	char buf[BUFSIZ];
	int index;
	char *cp;
	int len;

	snprintf(filename, sizeof filename, "%s/%s", path, templatename);
	file = fopen(filename, "r");
	if (file == 0) 
		/* TODO: Although we ignore an error here, it is useful to have debug msg */
		return false;

	index = -1;
	while ( fgets(buf, sizeof buf, file) != 0 ) {
		if (index == -1 || strncmp(buf, "##ys_", 4) == 0) {
			cp = strchr(buf, '\n');
			if (cp)
				*cp = 0;
			for (index = 0; index < YS_TEMPLATE_LEN; index++) {
				if (strcmp(buf, ys_html_sections[index]) == 0) {
					break;
				}
			}
			if (index == YS_TEMPLATE_LEN)
				goto end;
			continue;
		}
		cp = html_sections[index];
		len = (cp == 0 ? 0 : strlen(cp));
		cp = (char *) realloc(cp, len+strlen(buf)+1);
		if (cp == 0) {
			if (html_sections[index] != 0) {
				free(html_sections[index]);
				html_sections[index] = 0;
			}
			goto end;
		}
		strncpy(cp+len, buf, strlen(buf)+1);
		html_sections[index] = cp;
	}

end:
	fclose(file);
	return true;
}

YASENS HtmlOutput::HtmlOutput()
{
	out = stdout;
	doRowHeader = false;
	form = 0;
	input = 0;
	t = new YASENS TemplateLoader();
}

YASENS HtmlOutput::~HtmlOutput()
{
	delete t;
}

void 
YASENS HtmlOutput::doOutput(YASENS Collection *collection, QueryForm *form, QueryInput *input, YASENS SearchResultSet *rs)
{
	this->form = form;
	this->input = dynamic_cast<WebInput *>(input);

	int start_result = 1;
	int end_result;
	int cur_result = 0;
	int page_count = 1;
	int pagesize = form->getPageSize();
	int curpage = form->getCurrentPage();
	int matches = rs->getCount();

	if (pagesize > 0) {
		page_count = YS_DIVIDE_AND_ROUNDUP(matches,pagesize);
		if (curpage > page_count)
			curpage = page_count;
		else if (curpage <= 0)
			curpage = 1;
		start_result = pagesize * (curpage-1) + 1;
		if (start_result > matches)
			start_result = matches - pagesize;
		if (start_result <= 0)
			start_result = 1;
		end_result = start_result + pagesize - 1;
	}
	else {
		start_result = 1;
		end_result = matches;
		curpage = 1;
	}

	ys_docdb_t *db = collection->getDocDb();
	ys_docdata_t docfile, doc;

	doHeader();
	if (rs != 0) {
		YASENS SearchResultItem *item = rs->getNext();
		cur_result = 0;
		while (item != 0 && cur_result < end_result) {
			if (++cur_result >= start_result) {
				ys_dbgetdocumentref(db, item->getDocnum(),
					&docfile, &doc);
				outputRow(docfile.logicalname, "",
					&docfile, &doc, item->getScore(), item->getHits());
			}
			item = rs->getNext();
		}
		outputSummary(matches, curpage,	page_count, pagesize, rs->getElapsedTime());
	}
	doFooter();
}

void
YASENS HtmlOutput::start()
{
	fputs("Content-type: text/html\n\n<html>\n", out);
}

void
YASENS HtmlOutput::end()
{
	fputs("</html>\n", out);
}

/**
 * This function outputs the HTML header section.
 * It does nothing for text output.
 */
void
YASENS HtmlOutput::doHeader()
{
	if (t->hasHeader()) {
		fputs(t->getHeader(), out);
	}	
	else {
		fputs(
"<head>\n"
"<title>YASE - Query Results</title>\n"
"<link rev=\"made\" href=\"mailto:dibyendu@mazumdar.demon.co.uk\">\n"
"<link rel=\"stylesheet\" type=\"text/css\" href=\"yase.css\">\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO 8859-1\">\n"
"<meta name=\"author\" content=\"Dibyendu Majumdar\">\n"
"<meta name=\"description\" content=\"YASE Query Results\">\n"
"<meta name=\"keywords\" content=\"YASE Search Engine Information Retrieval Boolean Ranked Queries\">\n"
"</head>\n\n"
"<body>\n"
"<h1>YASE - Query Results</h1>\n"
"<hr width=\"100%\">\n", out);
	}
	doRowHeader = true;
}

void
YASENS HtmlOutput::doFooter()
{
	const char *default_templatefooter =
		"<hr width=\"100%\">\n"
		"Powered by <a href=\"http://www.mazumdar.demon.co.uk/yase_index.html\">YASE</a> - Copyright (C) 2000-2003 by "
		"<a href=\"mailto:dibyendu@mazumdar.demon.co.uk\">Dibyendu Majumdar\n" 
		"</body>\n";

	if (t->hasFooter()) {
		fputs(t->getFooter(), out);
	}
	else {
		fputs(default_templatefooter, out);
	}
}

/**
 * Output a single query row. For HTML output, this is in the format
 * of a table row.
 */
void 
YASENS HtmlOutput::outputRow(
	const char *reference,		/* HTML Link */
	const char *anchor, 		/* Anchor */
	ys_docdata_t *docfile,		/* Document File */
	ys_docdata_t *doc, 		/* Document */
	double rank, 			/* Rank */
	int matchcount) 		/* Number of hits */
{
	char mytitle[BUFSIZ] = {0};
	char myref1[BUFSIZ] = {0};
	char myref[BUFSIZ] = {0};
	char *default_rowheader = 
"<table border=\"1\" width=\"100%\" CELLSPACING=\"0\" CELLPADDING=\"0\">\n"
"    <tr ALIGN=\"left\" VALIGN=\"top\">\n"
"        <th>Title</th>\n"
"        <th>Size</th>\n"
"        <th>Rank</th>\n"
"    </tr>";


	if (doRowHeader) {
		doRowHeader = false;
		if (t->hasRowHeader()) {
			fputs(t->getRowHeader(), out);
		}
		else {
			fputs(default_rowheader, out);
		}
	}
	if (strncasecmp(reference, "http://", 7) == 0) {
		strncpy(myref1, reference, sizeof myref1);
	}
	else {
		snprintf(myref1, sizeof myref1, "http://%s/%s", 
			input->getHttpHost(), reference);
	}
	ys_url_encode_string(myref1, myref, sizeof myref, YS_URLX_SPACE_TO_HEX);
	if (doc->title[0]) {
		if (docfile->title[0]) {
			snprintf(mytitle, sizeof mytitle, "<i>%s</i><br>%s",
				doc->title, docfile->title);
		}
		else {
			snprintf(mytitle, sizeof mytitle, "<i>%s</i><br>%s",
				doc->title, docfile->logicalname);
		}
	}
	else {
		if (docfile->title[0]) {
			snprintf(mytitle, sizeof mytitle, "%s",
				docfile->title);
		}
		else {
			snprintf(mytitle, sizeof mytitle, "%s",
				docfile->logicalname);
		}
	}

	if (t->hasRow()) {
		const char *cp = t->getRow();
		while (*cp) {
			if (*cp == '%') {
				switch (*(cp+1)) {
				case '%':
					fputc('%', out);
					cp += 2;
					break;
				case 'l':
					fputs(myref, out);
					cp += 2;
					break;
				case 't':
					fputs(mytitle, out);
					cp += 2;
					break;
				case 'r':
					fprintf(out, "%.2f", rank);
					cp += 2;
					break;
				case 's':
					fprintf(out, "%s", docfile->size);
					cp += 2;
					break;
				case 'h':
					fprintf(out, "%d", matchcount);
					cp += 2;
					break;
				default:
					fputc(*cp, out);
					cp++;
					break;
				}
			}
			else {
				fputc(*cp, out);
				cp++;
			}
		}
	}
	else {
		fprintf(out, "<tr>\n<td valign=\"bottom\" nowrap>\n");
		fprintf(out, "<a href=\"%s\">%s</a></td>\n", myref, mytitle);
		fprintf(out, "<td>%s</td>\n", docfile->size);
		fprintf(out, "<td>%.2f</td>\n", rank);
		fprintf(out, "</tr>\n");
	}
}

/**
 * This is a helper function for substituting href links in an html section.
 * %l is used to denote the pace holder for a link.
 */
const char *
YASENS HtmlOutput::substLink(
	const char *fmt,
	char *buf,
	size_t buflen,
	const char *link)
{
	register const char *cp;
	register char *bufptr = buf;
	register size_t len;

	cp = fmt;
	len = 0;
	*bufptr = 0;
	while (*cp && len < buflen-1) {
		if (*cp == '%' && *(cp+1) == 'l') {
			size_t n;
			n = strlen(link);
			if (len+n+1 > buflen)
				break;
			strncpy(bufptr, link, n+1);
			bufptr += n;
			len += n;
			cp += 2;
		}
		else {
			*bufptr++ = *cp++;
			len++;
		}
	}
	*bufptr = 0;
	return buf;
}



/**
 * Outputs the results summary and the page navigation links. 
 * Only applicable for web (HTML) output.
 */
void
YASENS HtmlOutput::outputSummary(
	int matches, 		/* Number of matced items */
	int curpage,		/* Current page */
	int pagecount, 		/* Total Page count */
	int pagesize,		/* Size of each page */
	double elapsed_time)		
{
	char *td_format = "        <td>\n%s        </td>\n";
	char default_navfmt[4096];
	char tmpbuf[4096];
	char href[4096];
	char href_fmt[4096];
	const char *sm;
	const char *default_rowfooter = "</table>\n";
	const char *default_navheader =	
		"<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n"
		"    <tr align=\"center\" valign=\"top\"><td valign=\"bottom\" nowrap>\n";
	const char *default_navfooter = "    </tr>\n</table>\n";
	const char *yasequery_cmd = input->getScriptName();

	if (t->hasRowFooter()) {
		fputs(t->getRowFooter(), out);
	}
	else {
		fputs(default_rowfooter, out);
	}
	fputs("<hr width=\"100%\">\n", out);

	if (form->getMethod() == YASENS Search::SM_BOOLEAN)
		sm = "boolean";
	else
		sm = "ranked";

	snprintf(default_navfmt, sizeof default_navfmt,
		"        <td><a href=\""
		"%s?yp=%s&q=%s&sm=%s&de=%c&ps=%d&hr=%s&cp=%%d"
		"\">%%s</a></td>\n",
		yasequery_cmd,
		form->getCollectionPath(),
		ys_url_encode_string((const char *) form->getQueryExpr(), tmpbuf, sizeof tmpbuf, YS_URLX_ESCAPE_PERCENT | YS_URLX_SPACE_TO_PLUS),
		sm,
		form->getDumpEnv() ? 'y' : 'n',
		pagesize,
		"" /* input->getHttpReferrer()*/);

	snprintf(href_fmt, sizeof href_fmt,
		"%s?yp=%s&q=%s&sm=%s&de=%c&ps=%d&hr=%s&cp=%%d",
		yasequery_cmd,
		form->getCollectionPath(),
		tmpbuf, 
		sm,
		form->getDumpEnv() ? 'y' : 'n',
		pagesize,
"" /* input->getHttpReferrer()*/);
		
	fputs(default_navheader, out);

	if (curpage > 1) {
		if (t->hasFirstEnabled()) {
			snprintf(href, sizeof href, href_fmt, 1);
			fprintf(out, td_format,
				substLink(t->getFirstEnabled(),
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(out, default_navfmt, 1, "First Page");
		}
	}
	else {
		if (t->hasFirstDisabled()) {
			fprintf(out, td_format,
				t->getFirstDisabled());
		}
		else {
			fprintf(out, td_format, "First Page");
		}
	}
	if (curpage > 1) {
		if (t->hasPrevEnabled()) {
			snprintf(href, sizeof href, href_fmt, curpage-1);
			fprintf(out, td_format,
				substLink(t->getPrevEnabled(),
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(out, default_navfmt, curpage-1, "Prev Page");
		}
	}
	else {
		if (t->hasPrevDisabled()) {
			fprintf(out, td_format,
				t->getPrevDisabled());
		}
		else {
			fprintf(out, td_format, "Prev Page");
		}
	}
	if (curpage < pagecount) {
		if (t->hasNextEnabled()) {
			snprintf(href, sizeof href, href_fmt, curpage+1);
			fprintf(out, td_format,
				substLink(t->getNextEnabled(),
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(out, default_navfmt, curpage+1, "Next Page");
		}
	}
	else {
		if (t->getNextDisabled()) {
			fprintf(out, td_format,
				t->getNextDisabled());
		}
		else {
			fprintf(out, td_format, "Next Page");
		}
	}
	if (curpage < pagecount) {
		if (t->getLastEnabled()) {
			snprintf(href, sizeof href, href_fmt, pagecount);
			fprintf(out, td_format,
				substLink(t->getLastEnabled(),
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(out, default_navfmt, pagecount, "Last Page");
		}
	}
	else {
		if (t->getLastDisabled()) {
			fprintf(out, td_format,
				t->getLastDisabled());
		}
		else {
			fprintf(out, td_format, "Last Page");
		}
	}
	if (t->hasNewQuery()) {
		fprintf(out, td_format,
			substLink(t->getNewQuery(),
				tmpbuf, sizeof tmpbuf, input->getHttpReferrer()));
	}
	else {
		fprintf(out, "        <td><a href=\"%s\">New Query</a></td>\n",
			input->getHttpReferrer());
	}

	fputs(default_navfooter, out);
	fprintf(out, "<br>Page %d displayed, out of %d pages, containing %d items",
		curpage, pagecount, matches);
	fprintf(out, "<br>Query was processed in %.2g seconds",
		elapsed_time);

}

/**
 * BUG - the handling of new lines is not correct because it will 
 * only work if new lines are at the end.
 */
void
YASENS HtmlOutput::message(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	const char *cp = fmt;
	while (*cp) {
		if (*cp == '\n')
			fputs("<br>\n", stdout);
		cp++;
	}
}
