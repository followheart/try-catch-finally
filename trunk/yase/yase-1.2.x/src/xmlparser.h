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

/**
 * A tiny XML parser.
 * Dibyendu Majumdar
 * 
 * 18-12-00 First version.
 */

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "list.h"
#include "util.h"

typedef struct ys_xml_attr_t {
	ys_link_t l;
	char *name;
	char *value;
} ys_xml_attr_t;

enum {
	YS_XML_TAG_OPEN = 0,
	YS_XML_TAG_CLOSE,
	YS_XML_TAG_EMPTY,
	YS_XML_TAG_COMMENT,
	YS_XML_TAG_PI,
	YS_XML_TAG_CODE,
	YS_XML_TAG_SECTION,
	YS_XML_TAG_CDATA
};

typedef struct ys_xml_tag_t {
	char *name;
	int type;
	ys_list_t attrs;
} ys_xml_tag_t;

enum 
{
    YS_FREADER_PUSHBACK_BUFFER_LEN = 32
};

typedef struct 
{
    FILE *            file;
    int               ch;
    char              pushback_buffer[YS_FREADER_PUSHBACK_BUFFER_LEN];
    int               pb_index;
} ys_freader_t;

typedef struct ys_html_data_t {
	char *title;
	char *author;
	char *date;
	char *copyright;
	char *keywords;
	char *description;
	char *charset;
	char *lang;
	char *robots;
} ys_html_data_t;

extern int
ys_xml_parse_tag( ys_freader_t *fp, ys_xml_tag_t *tag );

extern void
ys_xml_free_tag( ys_xml_tag_t *tag );

extern void
ys_xml_release_memory(void);

extern int
ys_xml_parse_element( ys_freader_t *fp, struct ys_string *sb, ys_xml_tag_t *tag ) ;
			
extern int
ys_html_get_data(FILE *file, ys_html_data_t *htdata);

int
ys_freader_open(
    ys_freader_t *file_ptr, 
    const char *filename);

int
ys_freader_open2(
    ys_freader_t *file_ptr, 
    FILE *file);

int 
ys_freader_close(
    ys_freader_t *file_ptr);

int
ys_freader_unget_string(
    ys_freader_t *file_ptr, 
    const char *buffer, 
    int len);

int
ys_freader_unget_char(
    ys_freader_t *file_ptr, 
    int ch);

int
ys_freader_get_char(
    ys_freader_t *file_ptr);

#endif
