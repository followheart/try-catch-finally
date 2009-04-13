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
 * 04-01-03: New C++ classes to represent SaxParser, HtmlParser and XmlParser.
 * 05-01-03: Split of from getword.cpp
 */
#include "saxparser.h"

#if USE_LIBXML

static int 
saxp_is_standalone(void *ctx) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->isStandalone();
}

static int 
saxp_has_internal_subset(void *ctx) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->hasInternalSubset(); 
}

static int 
saxp_has_external_subset(void *ctx) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->hasExternalSubset(); 
}

static void 
saxp_internal_subset(void *ctx, const xmlChar *name,
	       const xmlChar *ExternalID, const xmlChar *SystemID)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->internalSubset(name, ExternalID, SystemID);
}

static xmlParserInputPtr
saxp_resolve_entity(void *ctx, const xmlChar *publicId, const xmlChar *systemId)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->resolveEntity(publicId, systemId);; 
}

static xmlEntityPtr 
saxp_get_entity(void *ctx, const xmlChar *name)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->getEntity(name); 
}

static xmlEntityPtr 
saxp_get_parameter_entity(void *ctx, const xmlChar *name)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	return sp->getParameterEntity(name); 
}

static void 
saxp_entity_decl(void *ctx, const xmlChar *name, int type,
          const xmlChar *publicId, const xmlChar *systemId, xmlChar *content)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->entityDecl(name, type, publicId, systemId, content);
}

static void 
saxp_attribute_decl(void *ctx, const xmlChar *elem, const xmlChar *name,
              int type, int def, const xmlChar *defaultValue,
	      xmlEnumerationPtr tree)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->attributeDecl(elem, name, type, def, defaultValue, tree);
}

static void 
saxp_element_decl(void *ctx, const xmlChar *name, int type,
	    xmlElementContentPtr content)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->elementDecl(name, type, content);
}

static void 
saxp_notation_decl(void *ctx, const xmlChar *name,
	     const xmlChar *publicId, const xmlChar *systemId)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->notationDecl(name, publicId, systemId);
}

static void 
saxp_unparsed_entity_decl(void *ctx, const xmlChar *name,
		   const xmlChar *publicId, const xmlChar *systemId,
		   const xmlChar *notationName)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->unparsedEntityDecl(name, publicId, systemId, notationName);
}

static void 
saxp_set_document_locator(void *ctx, xmlSAXLocatorPtr loc)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->setDocumentLocator(loc);
}

static void 
saxp_start_document(void *ctx) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->startDocument();
}

static void 
saxp_end_document(void *ctx) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->endDocument();
}

static void 
saxp_start_element_html(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->startElement(name, atts);
}

static void 
saxp_end_element_html(void *ctx, const xmlChar *name)
{
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->endElement(name);
}

static void 
saxp_characters_html(void *ctx, const xmlChar *ch, int len)
{
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->characters(ch, len);
}

static void 
saxp_do_reference(void *ctx, const xmlChar *name) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->doReference(name);
}

static void 
saxp_ignorable_whitespace(void *ctx, const xmlChar *ch, int len) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->ignorableWhitespace(ch, len);
}

static void 
saxp_processing_instruction(void *ctx, const xmlChar *target,
                      const xmlChar *data)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->processingInstruction(target, data);
}

static void 
saxp_do_comment(void *ctx, const xmlChar *value)
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
	sp->doComment(value);
}

static void 
saxp_warning(void *ctx, const char *msg, ...) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
}

static void 
saxp_error(void *ctx, const char *msg, ...) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
}

static void 
saxp_fatal_error(void *ctx, const char *msg, ...) 
{ 
	YASENS SaxParser *sp = (YASENS SaxParser *)ctx;
}

static xmlSAXHandler saxp_handler = {
    saxp_internal_subset, saxp_is_standalone, saxp_has_internal_subset,
    saxp_has_external_subset, saxp_resolve_entity, saxp_get_entity,
    saxp_entity_decl, saxp_notation_decl, saxp_attribute_decl,
    saxp_element_decl, saxp_unparsed_entity_decl, saxp_set_document_locator,
    saxp_start_document, saxp_end_document, saxp_start_element_html,
    saxp_end_element_html, saxp_do_reference, saxp_characters_html,
    saxp_ignorable_whitespace, saxp_processing_instruction,
    saxp_do_comment, saxp_warning, saxp_error, saxp_fatal_error, 
    saxp_get_parameter_entity,
};

extern xmlSAXHandler *
ys_get_saxp_handler(void)
{
	return &saxp_handler;
}

#endif

