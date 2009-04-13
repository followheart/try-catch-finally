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
#ifndef saxparser_h
#define saxparser_h

#include "yase.h"

#if USE_LIBXML

#include <libxml/xmlmemory.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/debugXML.h>

YASE_NS_BEGIN

class SaxParser {
public:
	virtual ~SaxParser()
	{
	}

	virtual int 
	isStandalone() 
	{ 
		return(0); 
	}

	virtual int 
	hasInternalSubset() 
	{ 
		return(0); 
	}

	virtual int 
	hasExternalSubset() 
	{ 
		return(0); 
	}

	virtual void 
	internalSubset(const xmlChar *name, const xmlChar *ExternalID, 
		const xmlChar *SystemID)
	{ 
	}

	virtual xmlParserInputPtr
	resolveEntity(const xmlChar *publicId, const xmlChar *systemId)
	{ 
		return(NULL); 
	}

	virtual xmlEntityPtr 
	getEntity(const xmlChar *name)
	{ 
		return(NULL); 
	}

	virtual xmlEntityPtr 
	getParameterEntity(const xmlChar *name)
	{ 
		return(NULL); 
	}

	virtual void 
	entityDecl(const xmlChar *name, int type,
          const xmlChar *publicId, const xmlChar *systemId, xmlChar *content)
	{ 
	}

	virtual void 
	attributeDecl(const xmlChar *elem, const xmlChar *name,
		int type, int def, const xmlChar *defaultValue,
		xmlEnumerationPtr tree)
	{ 
	}

	virtual void 
	elementDecl(const xmlChar *name, int type,
	    xmlElementContentPtr content)
	{ 
	}

	virtual void 
	notationDecl(const xmlChar *name,
	     const xmlChar *publicId, const xmlChar *systemId)
	{ 
	}

	virtual void 
	unparsedEntityDecl(const xmlChar *name,
		const xmlChar *publicId, const xmlChar *systemId,
		const xmlChar *notationName)
	{ 
	}

	virtual void 
	setDocumentLocator(xmlSAXLocatorPtr loc)
	{ 
	}

	virtual void 
	startDocument() 
	{ 
	}

	virtual void 
	endDocument() 
	{ 
	}

	virtual void 
	startElement(const xmlChar *name, const xmlChar **atts)
	{
	}

	virtual void 
	endElement(const xmlChar *name)
	{
	}

	virtual void 
	characters(const xmlChar *ch, int len)
	{
	}

	virtual void 
	doReference(const xmlChar *name) 
	{ 
	}

	virtual void 
	ignorableWhitespace(const xmlChar *ch, int len) 
	{ 
	}

	virtual void 
	processingInstruction(const xmlChar *target,
                      const xmlChar *data)
	{ 
	}

	virtual void 
	doComment(const xmlChar *value)
	{ 
	}

	virtual void 
	warning(const char *msg, ...) 
	{ 
	}

	virtual void 
	error(const char *msg, ...) 
	{ 
	}

	virtual void 
	fatalError(const char *msg, ...) 
	{ 
	}
};

YASE_NS_END

extern xmlSAXHandler *
ys_get_saxp_handler(void);

#endif

#endif

