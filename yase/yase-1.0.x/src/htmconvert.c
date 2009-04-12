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
* This is a standalone program that converts HTML output from 
* wvHtml 0.7.2 to YASE xml format. 
*
* Modification history:
* DM 09-May-02 Created
*/

#include "yase.h"

#include <libxml/xmlmemory.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/debugXML.h>

enum {
	YS_FIRST_PASS = 0,
	YS_IN_TITLE,
	YS_SECOND_PASS,
	YS_IN_HEADING,
	YS_IN_DOC
};

typedef struct {
	int state;
	char title[1024];
	char word[256];
	int n;
	int inword;
	char keywords[1024];
	char author[31];
} htmlctx_t;
typedef htmlctx_t xmlctx_t;

static void characters_html(void *ctx, const xmlChar *ch, int len);
static void do_reference(void *ctx, const xmlChar *name);
static void do_comment(void *ctx, const xmlChar *value);

static const char *
filebasename(const char *filename)
{
	const char *cp = strrchr(filename, '/');
	if (cp == 0) 
		return filename;
	return cp+1;
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

static int
is_heading(const char *tagname)
{
	if (strcasecmp(tagname, "h1") == 0 ||
	    strcasecmp(tagname, "h2") == 0)
		return 1;
	return 0;
}

static void 
start_element_html(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	if (hctx->state == YS_FIRST_PASS) {
		if (strcasecmp(name, "title") == 0) {
			hctx->state = YS_IN_TITLE;
		}
		else if (hctx->title[0] == 0 && strcasecmp(name, "h1") == 0) {
			hctx->state = YS_IN_TITLE;
		}
		else if (strcasecmp(name, "meta") == 0) {
			int i;
			int type = 0;

			for (i = 0; atts != 0 && atts[i] != 0; i += 2) {
				if (strcasecmp(atts[i], "name") == 0)  {
					if (strcasecmp(atts[i+1], "author") == 0) {
						type = 1;
					}
					else if (strcasecmp(atts[i+1], "keywords") == 0) {
						type = 2;
					}
					break;
				}
			}
			for (i = 0; type != 0 && atts != 0 && atts[i] != 0; i += 2) {
				if (strcasecmp(atts[i], "content") == 0) {
					if (type == 1)
						strncpy(hctx->author, atts[i+1], sizeof hctx->author);
					else if (type == 2)
						strncpy(hctx->keywords, atts[i+1], sizeof hctx->keywords);
					break;
				}
			}
		}
	}
	else if (hctx->state >= YS_SECOND_PASS) {
		if (is_heading(name)) {
			if (hctx->state == YS_IN_DOC) {
				printf("</YASEDOC>\n");
			}
			hctx->state = YS_IN_HEADING;	
			hctx->title[0] = 0;
		}
		hctx->n = 1;
		hctx->inword = 0;
		hctx->word[0] = 0;
		hctx->word[1] = 0;
	}
}

static void 
end_element_html(void *ctx, const xmlChar *name)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;

	if (hctx->state == YS_IN_DOC) {
		if (hctx->inword) {
			hctx->word[hctx->n] = 0;
			hctx->word[0] = strlen(hctx->word+1);
			printf("%s ", hctx->word+1);
			hctx->word[1] = hctx->word[0] = 0;
			hctx->n = 1;
			hctx->inword = 0;
		}
	}
	else if (hctx->state == YS_IN_TITLE) {
		if (strcasecmp(name, "title") == 0 || strcasecmp(name, "h1") == 0) {
			hctx->state = YS_FIRST_PASS;
		}
	}
	else if (hctx->state == YS_IN_HEADING) {
		if (is_heading(name)) {
			if (hctx->title[0] == 0)
				strncpy(hctx->title, "Untitled", sizeof hctx->title);
			printf("<YASEDOC title=\"%s\">\n", hctx->title);
			hctx->state = YS_IN_DOC;
			characters_html(hctx, hctx->title, strlen(hctx->title));
		}
	}		
}

static void 
characters_html(void *ctx, const xmlChar *ch, int len)
{
	htmlctx_t *hctx = (htmlctx_t *)ctx;
	int i;

	if (hctx->state == YS_IN_TITLE || hctx->state == YS_IN_HEADING) {
		char *cp;
		while (isspace(*ch) && len > 0) {
			len--;
			ch++;
		}	
		snprintf(hctx->title+strlen(hctx->title), 
			sizeof hctx->title-strlen(hctx->title),
			"%.*s", len, ch);
		while ((cp = strpbrk(hctx->title, "\n\r\t&<>")) != 0)
			*cp = ' ';
	}
	else if (hctx->state == YS_IN_DOC) {	
		for (i = 0; i < len; i++) {
			if (hctx->inword && !isalnum(ch[i])) {
				hctx->word[hctx->n] = 0;
				hctx->word[0] = strlen(hctx->word+1);
				printf("%s ", hctx->word+1);
				hctx->word[1] = hctx->word[0] = 0;
				hctx->n = 1;
				hctx->inword = 0;
				if (!isalnum(ch[i])) {
					if (ch[i] == '.' || 
						ch[i] == '!' || 
						ch[i] == '?' ||
						ch[i] == ',') {
						fputc(ch[i], stdout);
					}
				}
			}
			else if (hctx->inword && isalnum(ch[i])) {
				if (hctx->n < sizeof hctx->word-1)
					hctx->word[hctx->n++] = ch[i];
			}
			else if (!hctx->inword && isalnum(ch[i])) {
				hctx->word[hctx->n++] = ch[i];
				hctx->inword = 1;
			}
			else if (!hctx->inword && !isalnum(ch[i])) {
				if (!isalnum(ch[i])) {
					if (ch[i] == '.' || 
						ch[i] == '!' || 
						ch[i] == '?' ||
						ch[i] == ',') {
						fputc(ch[i], stdout);
					}
				}
			}
		}
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

static int
html_processor(const char *filename)
{
	struct stat statbuf = {0};
	htmlDocPtr htmldoc;
	htmlctx_t hctx = {0};

	hctx.state = YS_FIRST_PASS;
	htmldoc = htmlSAXParseFile(filename, NULL, 
		html_handler, &hctx);
	if (htmldoc != NULL) {
		xmlFreeDoc(htmldoc);
	}
	if (hctx.title[0] == 0)
		strncpy(hctx.title, filebasename(filename), sizeof hctx.title);
	printf("<?xml version=\"1.0\"?>\n");
	printf("<!DOCTYPE YASEFILE SYSTEM \"yase.dtd\">\n");
	printf("<YASEFILE title=\"%s\" type=\"HTML\" author=\"%s\" keywords=\"%s\">\n", hctx.title, hctx.author, hctx.keywords);
	printf("<YASEDOC title=\"%s\">\n", hctx.title);
	hctx.state = YS_IN_DOC;
	htmldoc = htmlSAXParseFile(filename, NULL, 
		html_handler, &hctx);
	if (htmldoc != NULL) {
		xmlFreeDoc(htmldoc);
	}
	printf("</YASEDOC>\n");
	printf("</YASEFILE>\n");
	xmlCleanupParser();
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return 0;
	return html_processor(argv[1]);
}
