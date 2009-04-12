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
/* 26-apr-2000 - added support for ranked queries */
/* 27-apr-2000 - removed word extraction logic to ys_extract_terms() */
/* 15-feb-2002 - modified to accept short names of variables */
/* 16-mar-2002 - renamed static functions */

#include "queryin.h"
#include "queryout.h"
#include "util.h"

static ys_bool_t
ys_get_next_field(ys_query_t *query, const char *line, 
	char *name, size_t namelen,
	char *value, size_t valuelen);

static void
ys_save_term( ys_query_t *query, char *word );

static int
ys_extract_terms( ys_query_t *query, const char *input );

static int
ys_parse_parameter( ys_query_t *query, const char *name, const char *value );

static int
ys_parse_web_query( char *query_string, ys_query_t *query );

static int
ys_parse_stdin_query( ys_query_t *query );

/**
 * Parse a QUERY_STRING and return the next field/value pair.
 * Semantics similar to strtok() - the input 'line' should be passed
 * the first time only; each call returns the next pair until all 
 * pairs are exhausted and NULL is returned.
 */ 
static ys_bool_t 
ys_get_next_field(
	ys_query_t *query, 
	const char *line, 
	char *name, size_t namelen,
	char *value, size_t valuelen)
{
	static char *buf = 0;
	static char *bufp = 0;
	char *cp;
	char *token;
	int i;

	if (line != 0) {
		if (buf != 0)
			free(buf);
		buf = (char *) calloc(1, strlen(line)+1);
		if (buf == 0) {
			ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
				"Error: failed to allocate memory\n");
			bufp = buf;
			return BOOL_FALSE;
		}
		strcpy(buf, line);
		bufp = buf;
	}

	if (bufp == 0)
		return BOOL_FALSE;

	cp = strchr(bufp, '&');
	if (cp != 0) {
		*cp++ = 0;
	}
	token = bufp;
	bufp = cp;

	cp = strchr(token, '=');
	if (cp == 0) {
		bufp = 0;
		return BOOL_FALSE;
	}
	*cp++ = 0;
	strncpy(name, token, namelen);
	name[namelen-1] = 0;

	/* TODO: Following should be replaced by a call to ys_url_decode_string,
	 * but, ys_url_decode_string doesn't replace pluses with spaces.
	 */
	token = value;
	for (i = 0; i < valuelen-1; i++) {
		if (*cp) {
			if (cp[0] == '%' && isxdigit(cp[1]) && isxdigit(cp[2])) {
				int ch = ys_url_decode_char(cp);
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
	return BOOL_TRUE;
}

/**
 * Add a query term to the list. If the term has already been added,
 * we increment the term.qtf - and update maximum query term frequency.
 */
static void
ys_save_term( ys_query_t *query, char *word )
{
	int i;
	for (i = 0; i < query->termcount; i++) {
		if (strcmp(query->terms[i].text, word) == 0) {
			query->terms[i].qtf++;
			if (query->terms[i].qtf > query->qmf)
				query->qmf = query->terms[i].qtf;
			return;
		}
	}
	if (query->termcount < YS_QUERY_MAXTERMS) {
		strncpy(query->terms[query->termcount].text, word,
			sizeof query->terms[query->termcount].text);
		query->terms[query->termcount].qtf = 1;
		query->terms[query->termcount].qtw = 0.0;
		query->terms[query->termcount].idf = 0.0;
		query->termcount++;
		if (query->qmf == 0)
			query->qmf = 1;
	}
}

/**
 * Extract terms from the input string and save them in the query.
 * Note that we have to use the same logic here that is used when building
 * the YASE index (see getword.c).
 */
static int 
ys_extract_terms( ys_query_t *query, const char *input )
{
	char ch;
	int inword = 0;
	size_t wordlen = 0;
	char word[YS_TERM_LEN];
	size_t maxlen = sizeof word;
	
	/* For boolean queries we need to save the entire query
	 * expression because it has to be parsed by the boolean
	 * query processor.
	 */
	if (query->boolean_query_expr == 0)
		query->boolean_query_expr = strdup(input);
	else {
		query->boolean_query_expr = realloc(
			query->boolean_query_expr,
			strlen(query->boolean_query_expr)+
			strlen(input)+1);
		strcat(query->boolean_query_expr, input);
	}

	while ((ch = *input++) != 0) {
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
			if (inword) {
				word[wordlen] = 0;
				inword = 0;
				assert(wordlen < maxlen);
				ys_save_term( query, word );
			}
		}
	}
	if (inword) {
		word[wordlen] = 0;
		inword = 0;
		assert(wordlen < maxlen);
		ys_save_term( query, word );
	}
	return 0;
}

/**
 * Parse the standard query parameters.
 * 15-feb-2002 modified to accept short names.
 */
static int
ys_parse_parameter( ys_query_t *query, const char *name, const char *value )
{
	if (strcmp(name, "em") == 0 ||
	    strcmp(name, "exact_match") == 0) {
		query->exact_match = *value == 'y';
	}
	else if (strcmp(name, "de") == 0 ||
		 strcmp(name, "dump_env") == 0) {
		query->dump_env = *value == 'y';
	}
	else if (strcmp(name, "sm") == 0 ||
		 strcmp(name, "searchmethod") == 0) {
		if (strcmp(value, "all") == 0) {
			query->method = YS_QM_ALL;
		}
		else if (strcmp(value, "boolean") == 0) {
			query->method = YS_QM_BOOLEAN;
		}
		else {
			query->method = YS_QM_RANKED;
		}
	}
	else if (strcmp(name, "ps") == 0 ||
		 strcmp(name, "pagesize") == 0) {
		query->pagesize = atoi(value);
		if (query->pagesize < 5)
			query->pagesize = 5;
	}
	else if (strcmp(name, "cp") == 0 ||
	         strcmp(name, "curpage") == 0) {
		query->curpage = atoi(value);
		if (query->curpage < 1)
			query->curpage = 1;
	}
	else if (strcmp(name, "hn") == 0 ||
		 strcmp(name, "host_name") == 0) {
		strncpy(query->hostname, value,
			sizeof query->hostname);
	}
	else if (strcmp(name, "hr") == 0 ||
		 strcmp(name, "http_referer") == 0) {
		query->http_referer = strdup(value);
	}
	return 0;
}

/**
 * Parses a query submitted from a Browser, and populates the query
 * structure.
 * 15-feb-2002 modified to accept short names.
 */
static int 
ys_parse_web_query( char *query_string, ys_query_t *query )
{
	char name[31];
	char value[1024];

	query->method = YS_QM_RANKED;
	query->exact_match = BOOL_TRUE;
	query->pagesize = 5;
	query->curpage = 1;
	query->termcount = 0;
	query->qmf = 0;
	query->web_query = BOOL_TRUE;

	if (query_string == 0) {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: QUERY_STRING undefined\n");
		return -1;
	}

	while (ys_get_next_field( query, query_string, name, sizeof name, 
		value, sizeof value)) {

		query_string = 0;

		if (strcmp(name, "yp") == 0 ||
		    strcmp(name, "collection_path") == 0) {
			char *docroot = getenv("DOCUMENT_ROOT");
			query->saved_collection_path = strdup(value);
			if (docroot != 0) {
				snprintf(query->collection_path, 
					sizeof query->collection_path,
					"%s/%s", docroot, value);
			}
			else {
				ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: DOCUMENT_ROOT undefined\n");
				return -1;
			}
		}
		else if (strcmp(name, "q") == 0 ||
			 strcmp(name, "query") == 0) {
			query->saved_query = strdup(value);
			ys_extract_terms( query, value );
		}
		else {
			ys_parse_parameter( query, name, value );
		}
	}

	if (query->dump_env) {
		int i;
		extern char **environ;
		ys_query_output_message(query->outp, YS_QRY_MSG_INFO, "Environment Dump\n");
		for (i = 0; environ[i] != 0; i++) {
			ys_query_output_message(query->outp, YS_QRY_MSG_INFO, "%s\n", environ[i]);
		}
	}

	if (query->http_referer == 0) {
		const char *cp = getenv("HTTP_REFERER");
		if (cp == 0) {
			ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: HTTP_REFERER "
				" undefined\n");
			return -1;
		}
		query->http_referer = strdup(cp);
	}

	if ( query->collection_path[0] == 0 || 
	    (query->method == YS_QM_BOOLEAN && 
	     query->boolean_query_expr == 0) || 
	    (query->method != YS_QM_BOOLEAN && 
	     query->termcount == 0) ) {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"Error: no terms defined or "
			"collection_path (pt) undefined\n");
		return -1;
	}

	if (query->hostname[0] == 0) {
		char *cp = getenv("HTTP_HOST");
		if (cp == 0) 
			cp = "localhost";
		strncpy(query->hostname, cp, sizeof query->hostname);
	}

	return 0;
}

/**
 * Parses a query submitted from on stdin, and populates the query
 * structure.
 * 15-feb-2002 modified to accept short names.
 */
static int 
ys_parse_stdin_query( ys_query_t *query )
{
	char name[31];
	char value[1024];
	char line[BUFSIZ];

	query->method = YS_QM_RANKED;
	query->exact_match = BOOL_TRUE;
	query->pagesize = -1;
	query->matches = 0;
	query->curpage = 1;
	query->termcount = 0;
	query->qmf = 0;
	query->web_query = BOOL_FALSE;

	while (fgets(line, sizeof line, stdin) != 0 && *line != '\n') {
		strtok(line, "\n");
		if (*line == ':') {
			char *linep = line+1;
			while (ys_get_next_field( query, linep, name, sizeof name, 
				value, sizeof value) ) {

				linep = 0;
				if (strcmp(name, "yp") == 0 ||
				    strcmp(name, "collection_path") == 0) {
					strncpy(query->collection_path, value,
						sizeof query->collection_path);
				}
				else {
					ys_parse_parameter( query, name, value );
				}
			}
		}
		else {
			ys_extract_terms( query, line );
		}
	}

	if (query->dump_env) {
		int j;
		extern char **environ;
		ys_query_output_message(query->outp, YS_QRY_MSG_INFO, "Environment Dump\n");
		for (j = 0; environ[j] != 0; j++) {
			ys_query_output_message(query->outp, YS_QRY_MSG_INFO, "%s\n", environ[j]);
		}
	}
	
	if (query->collection_path[0] == 0) {
		strncpy(query->collection_path, ".", 
			sizeof query->collection_path);
	}

	if ((query->method == YS_QM_BOOLEAN && 
	     query->boolean_query_expr == 0) || 
	    (query->method != YS_QM_BOOLEAN && 
	     query->termcount == 0)) {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: no terms defined\n");
		return -1;
	}

	fflush(stdin);

	return 0;
}

/**
 * Gets input for a query and collects all necessary data into the
 * ys_query_t structure.
 */
int 
ys_query_input(int argc, char *argv[], ys_query_t *query)
{
	char *query_string;

	query_string = getenv("QUERY_STRING");
	if (query_string != 0)
		return ys_parse_web_query( query_string, query );
	else
		return ys_parse_stdin_query( query );
}

