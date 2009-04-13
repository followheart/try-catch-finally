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
 * 25-12-00 An almost complete version of a minimal xml/html parser.
 */
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yase.h"
#include "alloc.h"
#include "list.h"
#include "xmlparser.h"

static int Charmap[128];
enum {
	MAP_UPPERCASE = 1,
	MAP_LOWERCASE = 2,
	MAP_SPACE = 4,
	MAP_DIGIT = 8,
	MAP_LETTER = 16,
	MAP_FOLLOW_LBRACKET = 32,
	MAP_TAGNAME = 64
};

static void
charmap_init(void)
{
	static const char *letters[] = {
		"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"\n\t\r\f ",
		"0123456789",
		"!?#/%",
		"-[_:.",
		NULL
	};
	static int types[] = {
		MAP_LOWERCASE|MAP_LETTER|MAP_FOLLOW_LBRACKET|MAP_TAGNAME,
		MAP_UPPERCASE|MAP_LETTER|MAP_FOLLOW_LBRACKET|MAP_TAGNAME,
		MAP_SPACE,
		MAP_DIGIT|MAP_TAGNAME,
		MAP_FOLLOW_LBRACKET,
		MAP_TAGNAME
	};
	static int done = 0;
	int i;

	if (done)
		return;
	done = 1;
	for ( i = 0; letters[i] != 0; i++ ) {
		const char *cp;
		cp = letters[i];
		while ( *cp != 0 ) {
			Charmap[(int)*cp] = types[i];
			cp++;
		}
	}
}

#if 1

#define is_ascii(ch) ((ch) > 0 && (ch) < 128)
#define is_space_char(ch) (is_ascii(ch) && (Charmap[ch] & MAP_SPACE))
#define is_name_char(ch, first) (is_ascii(ch) \
	&& ((first) ? (Charmap[ch] & MAP_FOLLOW_LBRACKET) \
		: (Charmap[ch] & MAP_TAGNAME)))

#else
static int
is_name_char( int ch, int is_first_char )
{
	if ( is_first_char )
		return (isalnum(ch) 
			|| ch == '!' 
			|| ch == '?'
			|| ch == '#'
			|| ch == '/'
			|| ch == '%');
	else
		return (isalnum(ch) 
			|| ch == '['
			|| ch == '_' 
			|| ch == ':' 
			|| ch == '.' 
			|| ch == '-');
}

#define is_space_char(ch) isspace(ch)
#endif

static ys_allocator_t *String_allocator = 0;
static char *
string_copy(const char *s)
{
	char *copy;
	if (String_allocator == 0)
		String_allocator = ys_new_allocator(0, 100);
	if (s != 0) {
		copy = (char *) ys_allocate(String_allocator, strlen(s)+1);
		strcpy(copy, s);
	}
	else {
		copy = (char *) ys_allocate(String_allocator, 1);
		*copy = 0;
	}
	return copy;
}

void
ys_xml_release_memory(void)
{
	if (String_allocator != 0)
		ys_destroy_allocator(String_allocator);
	String_allocator = 0;
}	

/***
* Open a file.
*/
int
ys_freader_open(
    ys_freader_t *file_ptr, 
    const char *filename)
{
    int rc = 0;
    file_ptr->file = fopen(filename, "r");
    if (file_ptr->file)
    {
        file_ptr->ch = EOF;
        file_ptr->pushback_buffer[0] = 0;
        file_ptr->pb_index = YS_FREADER_PUSHBACK_BUFFER_LEN;
    }
    else
    {
        fprintf(stderr, "Failed to open file '%s': %s\n", 
		filename, strerror(errno));
        rc = -1;
    }

    return rc;
}

/***
* Open a file.
*/
int
ys_freader_open2(
    ys_freader_t *file_ptr, 
    FILE *file)
{
    file_ptr->file = file;
    file_ptr->ch = EOF;
    file_ptr->pushback_buffer[0] = 0;
    file_ptr->pb_index = YS_FREADER_PUSHBACK_BUFFER_LEN;
    return 0;
}

/***
* Close a file.
*/
int 
ys_freader_close(
    ys_freader_t *file_ptr)
{
    int rc = 0;
    if (file_ptr->file != NULL)
    {
        rc = fclose(file_ptr->file);
        if (rc != 0)
            fprintf(stderr, "Failed to close file: %s\n", 
		    strerror(errno));
    }
    return rc;
}

/***
* Unget a number of characters.
*/
int
ys_freader_unget_string(
    ys_freader_t *file_ptr, 
    const char *buffer, 
    int len)
{
    int rc = 0;
    int i = 0;

    /* printf("pushing back '%.*s'\n", len, buffer);  */
    for (i = len-1; i >= 0; i--)
    {
        if (file_ptr->pb_index <= 0)
        {
            rc = -1;    
            break;
        }
        file_ptr->pb_index--;
        file_ptr->pushback_buffer[file_ptr->pb_index] 
            = buffer[i];
        /* printf("pushed %c\n", buffer[i]);  */
    }
    
    return rc;
}

int
ys_freader_unget_char(
    ys_freader_t *file_ptr, 
    int ch)
{
    int rc = 0;

    if (file_ptr->pb_index <= 0)
    {
        rc = -1;    
    }
    else 
    {
        file_ptr->pb_index--;
        file_ptr->pushback_buffer[file_ptr->pb_index] 
            = ch;
        /* printf("pushed %c\n", ch);  */
        rc = 0;
    }
    
    return rc;
}

/***
* Read a character from the file.
*/
int
ys_freader_get_char(
    ys_freader_t *file_ptr)
{
    if (file_ptr->pb_index < YS_FREADER_PUSHBACK_BUFFER_LEN)
    { /* clear the pushback stack first */
        file_ptr->ch = 
            file_ptr->pushback_buffer[file_ptr->pb_index];
        /* printf("popped %c\n", file_ptr->ch);  */
        file_ptr->pb_index++;
    }
    else
    {
        file_ptr->ch = fgetc(file_ptr->file);
    }

    return file_ptr->ch;
}

/**
 * This function peeks ahead into an input stream to see if the input will match the given
 * string. If yes, the function returns BOOL_TRUE, else BOOL_FALSE. The input stream is left undisturbed.
 */
int
ys_freader_peek(
	ys_freader_t *file_ptr,
	const char *s,
	int len)
{
	static char buf[80];
	int n;
	int rc;
	
	if (len > sizeof buf) {
		fprintf(stderr, "Unexpected error: lookahead buffer overflow\n");
		exit(EXIT_FAILURE);
		return 0;
	}
	for (n = 0; n < len; n++) {
		if ( (buf[n] = ys_freader_get_char(file_ptr)) == EOF )
			break;
	}
	if (n != len || strncasecmp(s, buf, len) != 0)	
		rc = 0;
	else
		rc = 1;
	if ( ys_freader_unget_string(file_ptr, buf, n) != 0 ) {
		fprintf(stderr, "Unexpected error: unget buffer overflow\n");
		exit(EXIT_FAILURE);
		return 0;
	}
	return rc;
}
                
/**
 * Free memory allocated to a tag. Note that the strings are not freed because
 * these are allocated using the the String_allocator object and can be freed
 * in one go.
 */
void
ys_xml_free_tag( ys_xml_tag_t *tag )
{
	ys_xml_attr_t *attr;
	if (tag == 0 || tag->name == 0)
		return;
	tag->name = 0;
	for (attr = (ys_xml_attr_t *) ys_list_first(&tag->attrs);
		attr != 0;) {
		ys_xml_attr_t *tmp = attr;
		attr = (ys_xml_attr_t *) ys_list_next(&tag->attrs, attr);
		ys_list_remove(&tag->attrs, tmp);
		free(tmp);
	}
	memset( tag, 0, sizeof *tag );
}

static void
dump_tag( ys_xml_tag_t *tag, FILE *fp )
{
	ys_xml_attr_t *attr;
	int n = 0;
	if (tag == 0 || tag->name == 0)
		return;
	if (tag->type == YS_XML_TAG_EMPTY)
		fprintf(fp, "Empty ");
	else if (tag->type == YS_XML_TAG_COMMENT)
		fprintf(fp, "Comment ");
	else if (tag->type == YS_XML_TAG_PI)
		fprintf(fp, "Processing Instruction ");
	else if (tag->type == YS_XML_TAG_OPEN)
		fprintf(fp, "Start ");
	else
		fprintf(fp, "End ");
	fprintf(fp, "Tag '%s' {", tag->name);
	for (attr = (ys_xml_attr_t *) ys_list_first(&tag->attrs);
		attr != 0;
		attr = (ys_xml_attr_t *) ys_list_next(&tag->attrs, attr)) {
		n++;
		fprintf(fp, "\n\t%s", attr->name);
		if (attr->value != 0) {
			fprintf(fp, "='%s'", attr->value);
		}
	}
	fprintf(fp, "%s}\n", n==0 ? "" : "\n");
}

/**
 * This function parses an XML/HTML tag. First non-space input character is 
 * expected to be '<'.
 */
int
ys_xml_parse_tag( ys_freader_t *fp, ys_xml_tag_t *tag ) 
{
	enum { EXPECT_LBRACKET = 1, EXPECT_NAME, EXPECT_ATTRIBUTES, 
		EXPECT_SPECIAL_TAG, EXPECT_RBRACKET, DONE };
	enum { EXPECT_ATTRNAME, EXPECT_EQUALTO, EXPECT_ATTRVALUE };

	int ch;	/* input character */
	int state; /* parser state */
	int attr_state; /* parser state when pasing attributes */
	int in_quote = 0;	/* flag to indicate we are in quoted strings */
	int quote_char;	/* quote character */
	static ys_string sb0, sb_attr_name, sb_attr_value;
	static int first_time = 1;
	ys_xml_attr_t *attr = 0;
	ys_string *sb_tag = &sb0;
	ys_list_t *attrs = &tag->attrs;
	const char *expected_end_tag = 0;

	/* Assume it will an start tag */
	tag->type = YS_XML_TAG_OPEN;
	tag->name = 0;
	if (first_time) {
		first_time = 0;
		charmap_init();
		ys_string_init( &sb_attr_name );
		ys_string_init( &sb_attr_value );
		ys_string_init( sb_tag );
	}
	else {
		ys_string_reset( &sb_attr_name );
		ys_string_reset( &sb_attr_value );
		ys_string_reset( sb_tag );
	}
	ys_list_init(attrs);

	/* start */
	state = EXPECT_LBRACKET;
	ch = ys_freader_get_char(fp);
	while ( ch != EOF && state != DONE ) {

		switch ( state ) {

		case EXPECT_LBRACKET:
			if ( is_space_char(ch) )
				break;
			if ( ch != '<' ) {
				fprintf(stderr,
					"Expect tag name to start with '<'\n");
				goto unget_exit;
			}
			state = EXPECT_NAME;
			break;

		case EXPECT_NAME:
			if ( !is_name_char(ch, ys_string_length(sb_tag) == 0) ) {
				if ( ys_string_length(sb_tag) == 0 ) {
					fprintf(stderr,
						"Tag name must follow '<' "
						"and must be at least one "
						"character\n");
					goto unget_exit;
				}
				if ( ch == '>' || ch == '/' ) {
					ys_freader_unget_char(fp, ch);	
					state = EXPECT_RBRACKET;
				}
				else {
					if ( strncmp(sb_tag->buf, "!--", 3) == 0) {
						tag->type = YS_XML_TAG_COMMENT;
						expected_end_tag = "-->";
						state = EXPECT_SPECIAL_TAG;
					}
					else if ( strncasecmp(sb_tag->buf, "![CDATA[", 8 ) == 0) {
						tag->type = YS_XML_TAG_CDATA;
						expected_end_tag = "]]>";
						state = EXPECT_SPECIAL_TAG;
					}
					else if ( strncmp(sb_tag->buf, "![", 2) == 0) {
						tag->type = YS_XML_TAG_SECTION;
						expected_end_tag = "]>";
						state = EXPECT_SPECIAL_TAG;
					}
					else if ( strncasecmp(sb_tag->buf, "?php", 4) == 0) {
						tag->type = YS_XML_TAG_CODE;
						expected_end_tag = "?>";
						state = EXPECT_SPECIAL_TAG;
					}
					else if ( sb_tag->buf[0] == '#' 
						|| sb_tag->buf[0] == '%')  {
						tag->type = YS_XML_TAG_CODE;
						expected_end_tag = sb_tag->buf[0] == '#' ? "#>" : "%>";
						state = EXPECT_SPECIAL_TAG;
					}
					else if ( sb_tag->buf[0] == '!' 
						|| sb_tag->buf[0] == '?' ) {
						tag->type = YS_XML_TAG_PI;
						expected_end_tag = ">";
						state = EXPECT_SPECIAL_TAG;
					}
					else {
						state = EXPECT_ATTRIBUTES;
						attr_state = EXPECT_ATTRNAME;
					}
				}
			}
			else if ( ch == '/' && ys_string_length(sb_tag) == 0) {
				tag->type = YS_XML_TAG_CLOSE;
			}
			else { 
				ys_string_addch( sb_tag, ch );
			}
			break;

		case EXPECT_ATTRIBUTES:
			
			switch (attr_state) {

			case EXPECT_ATTRNAME:

				/* parse attribute name */
				while ( is_space_char(ch) && ch != EOF ) {
					ch = ys_freader_get_char(fp);
				}
				if ( ch == EOF )  {
					fprintf(stderr, "EOF while reading attribute name\n");
					goto error_exit;
				}
				else if ( ch == '>' || ch == '/' ) {
					ys_freader_unget_char(fp, ch);	
					state = EXPECT_RBRACKET;
					break;
				}
				ys_string_reset( &sb_attr_name );
				while ( ch != EOF 
					&& !is_space_char(ch) 
					&& ch != '>' 
					&& ch != '=' 
					&& ch != '/' ) {
					ys_string_addch( &sb_attr_name, ch );
					ch = ys_freader_get_char(fp);
				}
				ys_string_nullterminate( &sb_attr_name );
				if ( ys_string_length( &sb_attr_name ) > 0 && attrs != 0 ) {
					attr = (struct ys_xml_attr_t *) calloc( 1, sizeof(ys_xml_attr_t) );
					attr->name = string_copy(sb_attr_name.buf);
					attr->value = 0;
					ys_list_append(attrs, attr);
				}
				else {
					attr = 0;
				}
				while ( is_space_char(ch) ) {
					ch = ys_freader_get_char(fp);
				}
				if ( ch == '=' ) {
					in_quote = 0;
					ys_string_reset( &sb_attr_value );
					attr_state = EXPECT_ATTRVALUE;
				}
				else if ( ch == '>' || ch == '/' ) {
					ys_freader_unget_char(fp, ch);	
					state = EXPECT_RBRACKET;
				}
				else {
					ys_freader_unget_char(fp, ch);	
					attr_state = EXPECT_ATTRNAME;
				}
				break;

			case EXPECT_ATTRVALUE:
				while ( is_space_char(ch) ) {
					ch = ys_freader_get_char(fp);
				}
				if ( ch == '"' || ch == '\'' ) {
					in_quote = 1;
					quote_char = ch;
				}
				else {
					ys_freader_unget_char(fp, ch);
				}
				while ( (ch = ys_freader_get_char(fp)) != EOF ) {
					if ( in_quote && ch == quote_char ) {
						in_quote = 0;
						attr_state = EXPECT_ATTRNAME;
						break;
					}
					else if (in_quote) {
						ys_string_addch( &sb_attr_value, ch );
					}
					else if ( ch == '>' ) {
						state = EXPECT_RBRACKET;
						ys_freader_unget_char(fp, ch);	
						break;
					}
					else if ( ch == '/' ) {
						int next = ys_freader_get_char(fp);
						ys_freader_unget_char(fp, next);
						if ( next == '>' ) {
							ys_freader_unget_char(fp, ch);	
							state = EXPECT_RBRACKET;
						}
						else {
							ys_string_addch( &sb_attr_value, ch );
						}
						break;
					}
					else if ( is_space_char(ch) ) {
						attr_state = EXPECT_ATTRNAME;
						break;
					}
					else {
						assert(!in_quote);
						if ( ch == '"' || ch == '\'' ) {
							fprintf(stderr, "Unexpected '%c' in attribute value\n", ch);
							goto unget_exit;
						}
						ys_string_addch( &sb_attr_value, ch );
					}
				}
				if (ys_string_length( &sb_attr_value ) > 0 ) {
					ys_string_nullterminate(&sb_attr_value);
					if (attr != 0) {
						attr->value = string_copy(sb_attr_value.buf);
					}
				}
			}
			break;

		case EXPECT_SPECIAL_TAG: 
			assert(expected_end_tag != 0);
			if ( ch != *expected_end_tag ) {
				ys_string_addch( sb_tag, ch );
				break;
			}
			if ( strlen(expected_end_tag) > 1 ) {
				if ( !ys_freader_peek(fp, expected_end_tag+1, strlen(expected_end_tag+1)) ) {
					ys_string_addch( sb_tag, ch );
					break;
				}
			}
			while ( ch != '>' && ch != EOF ) {
				ys_string_addch( sb_tag, ch );
				ch = ys_freader_get_char(fp);	
			}
			state = EXPECT_RBRACKET;
			ys_freader_unget_char(fp, ch);	
			break;	
			
		case EXPECT_RBRACKET:
			if ( is_space_char(ch) )
				break;
			if ( ch == '/' ) {
				ch = ys_freader_get_char(fp);
				tag->type = YS_XML_TAG_EMPTY;
			}
			if ( ch != '>' ) {
				fprintf(stderr, "Unexpected character '%c' in"
					" tag name, "
					"expecting '>'\n", ch);
				goto unget_exit;
			}
			state = DONE; 
			break;
		}

		if ( state != DONE ) {
			ch = ys_freader_get_char(fp);
		}
	}

	if ( state != DONE ) {
		fprintf(stderr, "Parse error\n");
		goto error_exit;
	}

	ys_string_nullterminate( sb_tag );
	ys_string_alltrim( sb_tag );

	if ( ys_string_length(sb_tag) == 0 ) {
		fprintf(stderr,
	 		"Tag name must be at least one character\n");
		goto error_exit;
	}

	tag->name = string_copy(sb_tag->buf);
#if 0
	dump_tag(tag, stdout);
#endif
	return 0;

unget_exit:
	if ( ch != EOF )
		ys_freader_unget_char(fp, ch);
error_exit:
	ys_xml_free_tag(tag);
	return -1;
}

int
ys_xml_parse_element( ys_freader_t *fp, ys_string *sb, ys_xml_tag_t *tag ) 
{
	int ch;
	int is_script = (tag->type == YS_XML_TAG_OPEN 
		&& strcasecmp(tag->name, "SCRIPT") == 0);
	
	charmap_init();
	ys_string_reset(sb);
	ch = ys_freader_get_char(fp);

	while ( ch != EOF ) {
		if ( ch == '<' ) {
			if (!is_script) {
				ys_freader_unget_char(fp, ch);
				break;
			}
			if (is_script) {
				if (ys_freader_peek(fp, "/SCRIPT>", 8)) {
					ys_freader_unget_char(fp, ch);
					break;
				}
			}
		}
		else if ( is_space_char(ch) ) {
			while ( ch != EOF && is_space_char(ch) ) {
				ch = ys_freader_get_char(fp);
			}
			if ( ch != EOF ) {
				ys_freader_unget_char(fp, ch);
			}
			ch = ' ';
		}
		ys_string_addch(sb, ch);
		ch = ys_freader_get_char(fp);
	}
	ys_string_nullterminate(sb);
	ys_string_alltrim(sb);
	return 0;
}	
			
int
ys_html_get_data(FILE *file, ys_html_data_t *htdata)
{
	static ys_string sb;
	static int first_time = 1;

	ys_freader_t fr;
	ys_freader_t *fp = &fr;
	int ch;
	ys_xml_tag_t tag = {0};
	int rc = 0;
	
	if (first_time) {
		first_time = 0;
		ys_string_init(&sb);
	}
	else {
		ys_string_reset(&sb);
	}
	ys_freader_open2(fp, file);
	memset(htdata, 0, sizeof *htdata);

	while ( rc == 0 && (ch = ys_freader_get_char(fp)) != EOF ) {

		if ( ch == '<' ) {
			ys_freader_unget_char(fp, ch);
			if ( ys_xml_parse_tag(fp, &tag) != 0 ) {
				fprintf(stderr, "Failed to parse html tag\n");
				rc = -1;
			}	
			if ( rc == 0 && tag.type == YS_XML_TAG_OPEN ) {
				if (strcasecmp(tag.name, "body") == 0) {
					rc = 1;
				}
				else if (strcasecmp(tag.name, "title") == 0) {
					if (ys_xml_parse_element(fp, &sb, &tag) != 0) {
						fprintf(stderr, "Failed to parse <title> element\n");
						rc = -1;
					}	
					else {
						htdata->title = string_copy(sb.buf);
					}
				}
				else if (strcasecmp(tag.name, "html") == 0) {
					ys_xml_attr_t *attr;

					for (attr = (ys_xml_attr_t *) ys_list_first(&tag.attrs);
						attr != 0;
						attr = (ys_xml_attr_t *) ys_list_next(&tag.attrs, attr)) {
						if (strcasecmp(attr->name, "lang") == 0) {
							htdata->lang = string_copy(attr->value);
							break;
						}
					}
				}
				else if (strcasecmp(tag.name, "meta") == 0) {
					ys_xml_attr_t *attr;
					enum { ATTR_CONTENT_TYPE = 1, ATTR_KEYWORDS, ATTR_AUTHOR, 
						ATTR_DESC, ATTR_ROBOTS, ATTR_COPYRIGHT, ATTR_DATE };
					int attr_type = 0;

					for (attr = (ys_xml_attr_t *) ys_list_first(&tag.attrs);
						attr != 0;
						attr = (ys_xml_attr_t *) ys_list_next(&tag.attrs, attr)) {
						if (strcasecmp(attr->name, "http-equiv") == 0) {
							if (strcasecmp(attr->value, "content-type") == 0) {
								attr_type = ATTR_CONTENT_TYPE;
							}
							else
								attr_type = 0;	
						}
						else if (strcasecmp(attr->name, "name") == 0) {
							if (strcasecmp(attr->value, "keywords") == 0) {
								attr_type = ATTR_KEYWORDS;
							}
							else if (strcasecmp(attr->value, "author") == 0) {
								attr_type = ATTR_AUTHOR;
							}
							else if (strcasecmp(attr->value, "description") == 0) {
								attr_type = ATTR_DESC;
							}
							else if (strcasecmp(attr->value, "copyright") == 0) {
								attr_type = ATTR_COPYRIGHT;
							}
							else if (strcasecmp(attr->value, "date") == 0) {
								attr_type = ATTR_DATE;
							}
							else if (strcasecmp(attr->value, "robots") == 0) {
								attr_type = ATTR_ROBOTS;
							}
							else
								attr_type = 0;
						}
						else if (strcasecmp(attr->name, "content") == 0) {
							if (attr_type == 0)
								continue;
							switch (attr_type) {
							case ATTR_KEYWORDS:
								htdata->keywords = string_copy(attr->value);
								break;
							case ATTR_AUTHOR:
								htdata->author = string_copy(attr->value);
								break;
							case ATTR_DESC:
								htdata->description = string_copy(attr->value);
								break;
							case ATTR_COPYRIGHT:
								htdata->copyright = string_copy(attr->value);
								break;
							case ATTR_DATE:
								htdata->date = string_copy(attr->value);
								break;
							case ATTR_ROBOTS:
								htdata->robots = string_copy(attr->value);
								break;
							case ATTR_CONTENT_TYPE: {
									char charset[64];
									const char *cp;
									int len;
									cp = strstr(attr->value, "charset=");
									if (cp != 0) 
										cp += 8;
									else
										break;
									len = 0;
									while (*cp != 0 && *cp != ';' && len < sizeof charset-1) {
										charset[len++] = *cp++;
									}
									charset[len] = 0;		
									htdata->charset = string_copy(charset);
									break;
								}
							}
						}
					}
				}
			}
			else if ( tag.type == YS_XML_TAG_CLOSE ) {
				if ( strcasecmp(tag.name, "head") == 0 ) {
					rc = 1;
				}
			}
			ys_xml_free_tag(&tag);
		}
	}

	return 0;
}

#ifdef _STANDALONE
typedef struct tag {
	ys_link_t link;
	char name[64];
} tagitem;
				
int
print_title( FILE *file )
{
	ys_html_data_t htdata = {0};
	ys_html_get_data(file, &htdata);
	if (htdata.title != 0)
		printf("title = '%s'\n", htdata.title);
	if (htdata.charset != 0)
		printf("charset = '%s'\n", htdata.charset);
	if (htdata.author != 0)
		printf("author = '%s'\n", htdata.author);
	if (htdata.description != 0)
		printf("description = '%s'\n", htdata.description);
	if (htdata.keywords != 0)
		printf("keywords = '%s'\n", htdata.keywords);
	ys_xml_release_memory();
	return 0;
}

int
parse_file( FILE *file )
{
	int ch;
	ys_list_t stack;
	ys_xml_tag_t tag = {0};
	ys_string sb;
	ys_freader_t fr;
	ys_freader_t *fp = &fr;

	ys_string_init(&sb);
	ys_list_init(&stack);
	ys_freader_open2(fp, file);
	while ( (ch = ys_freader_get_char(fp)) != EOF ) {
		int rc;

		ys_freader_unget_char(fp, ch);
		if (ch == '<') {
			ys_xml_free_tag(&tag);
			rc = ys_xml_parse_tag(fp, &tag);
			if (rc == 0) {
				dump_tag(&tag, stdout);
#if 0
				if (tag.type == YS_XML_TAG_OPEN)  {
					tagitem *t = calloc(1, sizeof(tagitem));
					if (t == 0) {
						fprintf(stderr, "out of memory\n");
						return -1;
					}
					strncpy(t->name, tag.name, 
						sizeof t->name);
					ys_list_push(&stack, t);
				}
				else if (tag.type == YS_XML_TAG_CLOSE) {
					tagitem *t = (tagitem *)ys_list_pop(&stack);
					if (t == 0 || strcmp(t->name, tag.name) != 0) {
						fprintf(stderr, "End tag '%s' does not match "
							"start tag '%s'\n", tag.name, t->name);
						if (t != 0)
							free(t);
						return -1;
					}
					if (t != 0)
						free(t);
				}
				else {
				}
#endif
			}
		}
		else {
			rc = ys_xml_parse_element(fp, &sb, &tag);
		}		
		if (rc != 0)
			break;
	}
	ys_string_destroy(&sb);
	ys_xml_release_memory();
	return 0;
}

int 
main(int argc, char *argv[]) 
{
	FILE *fp;

	if (argc != 2) {
		fprintf(stderr, "usage: xmlparser <file>\n");
		exit(1);
	}

	fp = fopen(argv[1], "r");
	if (fp != 0) {
		/* parse_file(fp); */
		print_title(fp);
		fclose(fp);
	}
	return 0;
}		
			
#endif	

