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

/* created 16-Jan 2000 from wquery.c */
/* 29-apr-2000 Added ys_query_output_message() */
/* 15-may-2000 Added ys_query_output_summary() */
/* 25-nov-2000 Added support for URLs in ys_query_output_reference() */
/* 21-apr-2001 Modified to use ys_qout_t as the handle */
/* 15-feb-2002 Modified Page Navigation menu */
/* 16-feb-2002 Added support for html template file */
/* 16-feb-2002 Converted text output to xml format */

#include "queryout.h"
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
	
typedef struct {
	FILE *file;			/* Output stream */
	ys_bool_t web_query;		/* Is this a web query ? */
					/* Output is HTML for web queries */
	ys_query_t *query;		/* The query structure */
	char *html_sections[YS_TEMPLATE_LEN];		
	ys_bool_t row_header;		/* Flag to control output of row
					 * header.
					 */
} ys_qout_t;

/**
 * This functions loads the contents of the template file into memory.
 * Each section is loaded into a buffer. The Buffer for each section is dynamically allocated.
 * 16-17 feb 2002.
 */
static int
ys_load_template_file(
	ys_qout_t *qout)
{
	char filename[1024];
	FILE *file;
	char buf[BUFSIZ];
	int index;
	char *cp;
	int len;

	/* initialise sections to null */
	for (index = 0; index < YS_TEMPLATE_LEN; index++) 
		qout->html_sections[index] = 0;
	index = -1;

	snprintf(filename, sizeof filename, "%s/%s", 
		qout->query->collection_path,
		"yase_html.template");
	file = fopen(filename, "r");
	if (file == 0) 
		/* TODO: Although we ignore an error here, it is useful to have debug msg */
		return 0;

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
		cp = qout->html_sections[index];
		len = (cp == 0 ? 0 : strlen(cp));
		cp = realloc(cp, len+strlen(buf)+1);
		if (cp == 0) {
			if (qout->html_sections[index] != 0) {
				free(qout->html_sections[index]);
				qout->html_sections[index] = 0;
			}
			goto end;
		}
		strncpy(cp+len, buf, strlen(buf)+1);
		qout->html_sections[index] = cp;
	}

end:
	fclose(file);
	return 0;
}

/**
 * Opens the output stream for query results.
 */
void *
ys_query_output_open(
	int argc, 
	char *argv[], 
	ys_bool_t web_query, 
	ys_query_t *query)
{
	static ys_qout_t qout;

	qout.file = stdout;
	qout.web_query = web_query;
	qout.query = query;
	if (qout.web_query) {
		fprintf(qout.file, "Content-type: text/html\n");
		fprintf(qout.file, "\n");
		fprintf(qout.file, "<html>\n");
	}
	else {
		fprintf(qout.file, "<xml>\n");
	}

	return &qout;
}

/**
 * This function outputs the HTML header section.
 * It does nothing for text output.
 */
int 
ys_query_output_header(
	void *handle)
{
	ys_qout_t *qout = (ys_qout_t *)handle;

	ys_load_template_file(qout);

	if (qout->web_query) {
		if (qout->html_sections[YS_TEMPLATE_HEADER]) {
			fputs(qout->html_sections[YS_TEMPLATE_HEADER], qout->file);
		}
		else {
			fprintf(qout->file,
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
"<hr width=\"100%\">\n");
		}
	}
	qout->row_header = BOOL_TRUE;
	return 0;
}

/**
 * Output a single query row. For HTML output, this is in the format
 * of a table row.
 */
void
ys_query_output_row(
	void *handle, 
	const char *reference,		/* HTML Link */
	const char *anchor, 		/* Anchor */
	ys_docdata_t *docfile,		/* Document File */
	ys_docdata_t *doc, 		/* Document */
	float rank, 			/* Rank */
	int matchcount) 		/* Number of hits */
{
	ys_qout_t *qout = (ys_qout_t *)handle;
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

	if (qout->web_query) {
		if (qout->row_header) {
			qout->row_header = BOOL_FALSE;
			if (qout->html_sections[YS_TEMPLATE_ROWHEADER]) {
				fputs(qout->html_sections[YS_TEMPLATE_ROWHEADER], qout->file);
			}
			else {
				fputs(default_rowheader, qout->file);
			}
		}

		if (strncasecmp(reference, "http://", 7) == 0) {
			strncpy(myref1, reference, sizeof myref1);
		}
		else {
			snprintf(myref1, sizeof myref1, "http://%s/%s", 
				qout->query->hostname, reference);
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

		if (qout->html_sections[YS_TEMPLATE_ROW]) {
			const char *cp = qout->html_sections[YS_TEMPLATE_ROW];
			while (*cp) {
				if (*cp == '%') {
					switch (*(cp+1)) {
					case '%':
						fputc('%', qout->file);
						cp += 2;
						break;
					case 'l':
						fputs(myref, qout->file);
						cp += 2;
						break;
					case 't':
						fputs(mytitle, qout->file);
						cp += 2;
						break;
					case 'r':
						fprintf(qout->file, "%.2f", rank);
						cp += 2;
						break;
					case 's':
						fprintf(qout->file, "%s", docfile->size);
						cp += 2;
						break;
					case 'h':
						fprintf(qout->file, "%d", matchcount);
						cp += 2;
						break;
					default:
						fputc(*cp, qout->file);
						cp++;
						break;
					}
				}
				else {
					fputc(*cp, qout->file);
					cp++;
				}
			}
		}
		else {
			fprintf(qout->file, "<tr>\n<td valign=\"bottom\" nowrap>\n");
			fprintf(qout->file, "<a href=\"%s\">%s</a></td>\n", myref, mytitle);
			fprintf(qout->file, "<td>%s</td>\n", docfile->size);
			fprintf(qout->file, "<td>%.2f</td>\n", rank);
			fprintf(qout->file, "</tr>\n");
		}
	}
	else {
		fprintf(qout->file, "<docfile>\n");
		if (doc->title[0]) {
			fprintf(qout->file, "\t<doc>\n");
			fprintf(qout->file, "\t\t<title>%s</title>\n", doc->title);
			fprintf(qout->file, "\t\t<anchor>%s</anchor>\n", doc->anchor);
			fprintf(qout->file, "\t</doc>\n");
		}
		fprintf(qout->file, "\t<name>%s</name>\n", docfile->logicalname);
		fprintf(qout->file, "\t<size>%s</size>\n", docfile->size);
		fprintf(qout->file, "\t<title>%s</title>\n", docfile->title);
		fprintf(qout->file, "\t<author>%s</author>\n", docfile->author);
		fprintf(qout->file, "\t<rank>%.2f</rank>\n", rank);
		fprintf(qout->file, "\t<date>%s</date>\n", docfile->datecreated);
		fprintf(qout->file, "\t<hits>%d</hits>\n", matchcount);
		fprintf(qout->file, "</docfile>\n");
	}
}

/**
 * Outputs a general purpose message.
 */
void
ys_query_output_message(
	void *handle, 
	ys_qry_msg_t type,
	const char *fmt, 
	...)
{
	ys_qout_t *qout = (ys_qout_t *)handle;
	va_list args;

	if (!qout->web_query) {
		fprintf(qout->file, "<%s>\n",
			(type == YS_QRY_MSG_ERROR) ? "error" : "info");
	}
	va_start(args, fmt);
	vfprintf(qout->file, fmt, args);
	va_end(args);
	if (qout->web_query)  {
		const char *cp = fmt;
		while (*cp) {
			if (*cp == '\n')
				fprintf(qout->file, "<br>\n");
			cp++;
		}
	}
	if (!qout->web_query) {
		fprintf(qout->file, "</%s>\n",
			(type == YS_QRY_MSG_ERROR) ? "error" : "info");
	}
}

/**
 * This is a helper function for substituting href links in an html section.
 * %l is used to denote the pace holder for a link.
 */
static const char *
ys_subst_link(
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
int 
ys_query_output_summary(
	void *handle, 
	int matches, 		/* Number of matced items */
	int curpage,		/* Current page */
	int pagecount, 		/* Total Page count */
	int pagesize,
	double elapsed_time)		/* Size of each page */
{
	ys_qout_t *qout = (ys_qout_t *)handle;
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
	const char *yasequery_cmd = "/cgi-bin/yasequery";
	const char *default_templatefooter =
		"<hr width=\"100%\">\n"
		"Powered by <a href=\"http://www.mazumdar.demon.co.uk/yase_index.html\">YASE</a> - Copyright (C) 2000-2002 by "
		"<a href=\"mailto:dibyendu@mazumdar.demon.co.uk\">Dibyendu Majumdar\n" 
		"</body>\n";

	if (!qout->web_query)
		return 0;

	if (qout->html_sections[YS_TEMPLATE_ROWFOOTER]) {
		fputs(qout->html_sections[YS_TEMPLATE_ROWFOOTER], qout->file);
	}
	else {
		fputs(default_rowfooter, qout->file);
	}
	fputs("<hr width=\"100%\">\n", qout->file);

	if (qout->query->method == YS_QM_ALL)
		sm = "all";
	else if (qout->query->method == YS_QM_BOOLEAN)
		sm = "boolean";
	else
		sm = "ranked";

	snprintf(default_navfmt, sizeof default_navfmt,
		"        <td><a href=\""
		"%s?yp=%s&q=%s&sm=%s&em=%c&de=%c&ps=%d&hr=%s&cp=%%d"
		"\">%%s</a></td>\n",
		yasequery_cmd,
		qout->query->saved_collection_path,
		ys_url_encode_string(qout->query->saved_query, tmpbuf, sizeof tmpbuf, YS_URLX_ESCAPE_PERCENT | YS_URLX_SPACE_TO_PLUS),
		sm,
		qout->query->exact_match ? 'y' : 'n',
		qout->query->dump_env ? 'y' : 'n',
		pagesize,
		qout->query->http_referer);

	snprintf(href_fmt, sizeof href_fmt,
		"%s?yp=%s&q=%s&sm=%s&em=%c&de=%c&ps=%d&hr=%s&cp=%%d",
		yasequery_cmd,
		qout->query->saved_collection_path,
		tmpbuf, 
		sm,
		qout->query->exact_match ? 'y' : 'n',
		qout->query->dump_env ? 'y' : 'n',
		pagesize,
		qout->query->http_referer);
		
	fputs(default_navheader, qout->file);

	if (curpage > 1) {
		if (qout->html_sections[YS_TEMPLATE_FIRST_ENABLED]) {
			snprintf(href, sizeof href, href_fmt, 1);
			fprintf(qout->file, td_format,
				ys_subst_link(qout->html_sections[YS_TEMPLATE_FIRST_ENABLED],
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(qout->file, default_navfmt, 1, "First Page");
		}
	}
	else {
		if (qout->html_sections[YS_TEMPLATE_FIRST_DISABLED]) {
			fprintf(qout->file, td_format,
				qout->html_sections[YS_TEMPLATE_FIRST_DISABLED]);
		}
		else {
			fprintf(qout->file, td_format, "First Page");
		}
	}
	if (curpage > 1) {
		if (qout->html_sections[YS_TEMPLATE_PREV_ENABLED]) {
			snprintf(href, sizeof href, href_fmt, curpage-1);
			fprintf(qout->file, td_format,
				ys_subst_link(qout->html_sections[YS_TEMPLATE_PREV_ENABLED],
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(qout->file, default_navfmt, curpage-1, "Prev Page");
		}
	}
	else {
		if (qout->html_sections[YS_TEMPLATE_PREV_DISABLED]) {
			fprintf(qout->file, td_format,
				qout->html_sections[YS_TEMPLATE_PREV_DISABLED]);
		}
		else {
			fprintf(qout->file, td_format, "Prev Page");
		}
	}
	if (curpage < pagecount) {
		if (qout->html_sections[YS_TEMPLATE_NEXT_ENABLED]) {
			snprintf(href, sizeof href, href_fmt, curpage+1);
			fprintf(qout->file, td_format,
				ys_subst_link(qout->html_sections[YS_TEMPLATE_NEXT_ENABLED],
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(qout->file, default_navfmt, curpage+1, "Next Page");
		}
	}
	else {
		if (qout->html_sections[YS_TEMPLATE_NEXT_DISABLED]) {
			fprintf(qout->file, td_format,
				qout->html_sections[YS_TEMPLATE_NEXT_DISABLED]);
		}
		else {
			fprintf(qout->file, td_format, "Next Page");
		}
	}
	if (curpage < pagecount) {
		if (qout->html_sections[YS_TEMPLATE_LAST_ENABLED]) {
			snprintf(href, sizeof href, href_fmt, pagecount);
			fprintf(qout->file, td_format,
				ys_subst_link(qout->html_sections[YS_TEMPLATE_LAST_ENABLED],
					tmpbuf, sizeof tmpbuf, href));
		}
		else {
			fprintf(qout->file, default_navfmt, pagecount, "Last Page");
		}
	}
	else {
		if (qout->html_sections[YS_TEMPLATE_LAST_DISABLED]) {
			fprintf(qout->file, td_format,
				qout->html_sections[YS_TEMPLATE_LAST_DISABLED]);
		}
		else {
			fprintf(qout->file, td_format, "Last Page");
		}
	}
	if (qout->html_sections[YS_TEMPLATE_NEW_QUERY]) {
		fprintf(qout->file, td_format,
			ys_subst_link(qout->html_sections[YS_TEMPLATE_NEW_QUERY],
				tmpbuf, sizeof tmpbuf, qout->query->http_referer));
	}
	else {
		fprintf(qout->file, "        <td><a href=\"%s\">New Query</a></td>\n",
			qout->query->http_referer);
	}

	fputs(default_navfooter, qout->file);
	fprintf(qout->file, "<br>Page %d displayed, out of %d pages, containing %d items",
		curpage, pagecount, matches);
	fprintf(qout->file, "<br>Query was processed in %.2g seconds",
		elapsed_time);

	if (qout->html_sections[YS_TEMPLATE_FOOTER]) {
		fputs(qout->html_sections[YS_TEMPLATE_FOOTER], qout->file);
	}
	else {
		fputs(default_templatefooter, qout->file);
	}
	return 0;
}

/**
 * Closes the output stream.
 */
int 
ys_query_output_close(void *handle)
{
	ys_qout_t *qout = (ys_qout_t *)handle;
	size_t index;
	if (qout->web_query)
		fputs("</html>\n", qout->file);
	else
		fputs("</xml>\n", qout->file);
	fflush(qout->file);
	for (index = 0; index < YS_TEMPLATE_LEN; index++) {
		if (qout->html_sections[index] != 0) {
			free(qout->html_sections[index]);
			qout->html_sections[index] = 0;
		}
	}
	return 0;
}
