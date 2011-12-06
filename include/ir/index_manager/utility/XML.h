#ifndef XML_H
#define XML_H
// Copyright (c) Shareaza Development Team, 2002-2005.
// This file is part of SHAREAZA (www.shareaza.com)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Utilities.h>

#include <boost/algorithm/string/case_conv.hpp>

#include <list>
#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class XMLNode;
class XMLElement;
class XMLAttribute;
class XMLNode
{
public:
    XMLNode(XMLElement* pParent = NULL, const char* pszName = NULL);
    virtual ~XMLNode();

protected:
    int nNode;
    XMLElement* pParent;
    std::string sName;
    std::string sValue;

    enum { xmlNode, xmlElement, xmlAttribute };

public:
    inline int getType() ;
    inline XMLNode* asNode() ;
    inline XMLElement* asElement() ;
    inline XMLAttribute* asAttribute() ;
public:
    inline XMLElement* getParent() ;
    inline void Delete();
public:
    inline std::string getName() ;
    inline void setName(const char* pszValue);
    inline bool isNamed(const char* pszName) ;
    inline std::string getValue() ;
    inline void setValue(const char* pszValue);
protected:
    static bool parseMatch(const char*& pszXML, const char* pszToken);
    static bool parseIdentifier(const char*& pszXML, std::string& strIdentifier);
public:
    static std::string	stringToValue(const char*& pszXML, int nLength);
    static void valueToString(const char* pszValue, std::string& strXML);
    static void uniformString(std::string& str);

    friend class XMLElement;
};

typedef std::list<XMLElement*> ElementList;
typedef ElementList::iterator ElementIterator;
typedef std::map<std::string,XMLAttribute*> AttributeMap;
typedef AttributeMap::iterator AttributeIterator;
typedef std::pair<std::string,XMLAttribute*> AttributePair;
class XMLElement : public XMLNode
{
    // Construction
public:
    XMLElement(XMLElement* pParent = NULL, const char* pszName = NULL);
    virtual ~XMLElement();

protected:
    ElementList pElements;
    AttributeMap pAttributes;

public:
    XMLElement* clone(XMLElement* pParent = NULL);
    inline XMLElement* detach();
public:
    inline XMLElement* addElement(const char* pszName);
    inline XMLElement* addElement(XMLElement* pElement);
    inline int getElementCount() ;
    inline XMLElement* getFirstElement() ;
    inline ElementIterator getElementIterator() ;
    inline XMLElement* getNextElement(ElementIterator& iter) ;
    inline bool hasNextElement(ElementIterator& iter) ;
    inline XMLElement* getElementByName(const char* pszName) ;
    inline XMLElement* getElementByName(const char* pszName, bool bCreate);
    inline void removeElement(XMLElement* pElement);
    void deleteAllElements();
public:
    inline XMLAttribute* addAttribute(const char* pszName, const char* pszValue = NULL);
    inline XMLAttribute* addAttribute(XMLAttribute* pAttribute);
    inline int getAttributeCount() const;

    inline AttributeIterator	getAttributeIterator() ;
    inline XMLAttribute* getNextAttribute(AttributeIterator& iter) ;
    inline bool hasNextAttribute(AttributeIterator& iter) ;
    inline XMLAttribute* getAttribute(const char* pszName) ;
    inline std::string getAttributeValue(const char* pszName, const char* pszDefault = NULL) ;
    inline void removeAttribute(XMLAttribute* pAttribute);
    inline void deleteAttribute(const char* pszName);
    void deleteAllAttributes();
public:
    std::string toString(bool bHeader = false, bool bNewline = false);
    void toString(std::string& strXML, bool bNewline = false,int nDepth = 0);
    void toFile(const std::string& sPath, bool bHeader = false);
    bool parseString(const char*& strXML);
    bool equals(XMLElement* pXML) ;

    static XMLElement* fromString(const char* pszXML, bool bHeader = false);
    static XMLElement* fromBytes(uint8_t* pByte, size_t nByte, bool bHeader = false);
    static XMLElement* fromFile(const std::string& sPath, bool bHeader = false);


};


class XMLAttribute : public XMLNode
{
public:
    XMLAttribute(XMLElement* pParent, const char* pszName = NULL);
    virtual ~XMLAttribute();


public:
    static const char* xmlnsSchema;
    static const char* xmlnsInstance;
    static const char* schemaName;

    // Operations
public:
    XMLAttribute* clone(XMLElement* pParent = NULL);
    void toString(std::string& strXML);
    bool parseString(const char*& strXML);
    bool equals(XMLAttribute* pXML) ;
};


//////////////////////////////////////////////////////////////////////
// XMLNode node type and casting access

int XMLNode::getType()
{
    return nNode;
}

XMLNode* XMLNode::asNode()
{
    return (XMLNode*)this;
}

XMLElement* XMLNode::asElement()
{
    return ( nNode == xmlElement ) ? (XMLElement*)this : NULL;
}

XMLAttribute* XMLNode::asAttribute()
{
    return ( nNode == xmlAttribute ) ? (XMLAttribute*)this : NULL;
}

//////////////////////////////////////////////////////////////////////
// XMLNode parent access and delete

XMLElement* XMLNode::getParent()
{
    return pParent;
}

void XMLNode::Delete()
{
    if ( this == NULL ) return;

    if ( pParent != NULL )
    {
        if ( nNode == xmlElement ) pParent->removeElement( (XMLElement*)this );
        else if ( nNode == xmlAttribute ) pParent->removeAttribute( (XMLAttribute*)this );
    }

    delete this;
}

//////////////////////////////////////////////////////////////////////
// XMLNode name access

std::string XMLNode::getName()
{
    return sName;
}

void XMLNode::setName(const char* pszValue)
{
    sName = pszValue;
}

bool XMLNode::isNamed(const char* pszName)
{
    if ( this == NULL ) return false;
    return strcasecmp(sName.c_str(),pszName ) == 0;
}

//////////////////////////////////////////////////////////////////////
// XMLNode value access

std::string XMLNode::getValue()
{
    return sValue;
}

void XMLNode::setValue(const char* pszValue)
{
    sValue = pszValue;
}

//////////////////////////////////////////////////////////////////////
// XMLElement detach

XMLElement* XMLElement::detach()
{
    if ( pParent ) pParent->removeElement( this );
    pParent = NULL;
    return this;
}

//////////////////////////////////////////////////////////////////////
// XMLElement element access

XMLElement* XMLElement::addElement(const char* pszName)
{
    XMLElement* pElement = new XMLElement( this, pszName );
    pElements.push_back( pElement );
    return pElement;
}

XMLElement* XMLElement::addElement(XMLElement* pElement)
{
    if ( pElement->pParent ) return NULL;
    pElements.push_back( pElement );
    pElement->pParent = this;
    return pElement;
}

int XMLElement::getElementCount()
{
    return (int)pElements.size();
}

XMLElement* XMLElement::getFirstElement()
{
    if ( this == NULL ) return NULL;
    return (pElements.size() > 0)? pElements.front() : NULL;
}

ElementIterator XMLElement::getElementIterator()
{
    return pElements.begin();
}
bool XMLElement::hasNextElement(ElementIterator& iter)
{
    return (pElements.end() != iter);
}

XMLElement* XMLElement::getNextElement(ElementIterator& iter)
{
    return *iter++;
}

XMLElement* XMLElement::getElementByName(const char* pszName)
{
    for ( ElementIterator iter = getElementIterator() ; hasNextElement(iter) ; )
    {
        XMLElement* pElement = getNextElement( iter );
        if ( strcasecmp(pElement->getName().c_str(),pszName ) == 0 ) return pElement;

    }
    return NULL;
}

XMLElement* XMLElement::getElementByName(const char* pszName, bool bCreate)
{
    for ( ElementIterator iter = getElementIterator() ; hasNextElement(iter) ; )
    {
        XMLElement* pElement = getNextElement( iter );
        if ( strcasecmp(pElement->getName().c_str(),pszName ) == 0 ) return pElement;
    }

    return bCreate ? addElement( pszName ) : NULL;
}

void XMLElement::removeElement(XMLElement* pElement)
{
    pElements.remove(pElement);
}

//////////////////////////////////////////////////////////////////////
// XMLElement attribute access

XMLAttribute* XMLElement::addAttribute(const char* pszName, const char* pszValue)
{
    XMLAttribute* pAttribute = getAttribute( pszName );

    if ( ! pAttribute )
    {
        pAttribute = new XMLAttribute( this, pszName );
        std::string strName( pszName );
        boost::to_lower(strName);
        pAttributes.insert(AttributePair(strName, pAttribute ));
    }

    if ( pszValue ) pAttribute->setValue( pszValue );

    return pAttribute;
}

XMLAttribute* XMLElement::addAttribute(XMLAttribute* pAttribute)
{
    if ( pAttribute->pParent ) return NULL;
    std::string strName( pAttribute->sName );
    boost::to_lower(strName);
    pAttributes.insert(AttributePair(pAttribute->sName, pAttribute) );
    pAttribute->pParent = this;
    return pAttribute;
}

int XMLElement::getAttributeCount() const
{
    return (int)pAttributes.size();
}

AttributeIterator XMLElement::getAttributeIterator()
{
    return pAttributes.begin();
}
bool XMLElement::hasNextAttribute(AttributeIterator& iter)
{
    return (pAttributes.end() != iter);
}

XMLAttribute* XMLElement::getNextAttribute(AttributeIterator& iter)
{
    XMLAttribute* pAttribute = iter->second;
    iter++;
    return pAttribute;
}

XMLAttribute* XMLElement::getAttribute(const char* pszName)
{
    std::string strName( pszName );
    boost::to_lower(strName);
    AttributeIterator iter = pAttributes.find(strName);
    if (iter != pAttributes.end())
        return iter->second;
    return NULL;
}

std::string XMLElement::getAttributeValue(const char* pszName, const char* pszDefault)
{
    XMLAttribute* pAttribute = getAttribute( pszName );
    std::string strResult;
    if ( pAttribute ) strResult = pAttribute->sValue;
    else if ( pszDefault ) strResult = pszDefault;
    return strResult;
}

void XMLElement::removeAttribute(XMLAttribute* pAttribute)
{
    std::string strName( pAttribute->sName );
    boost::to_lower(strName);
    AttributeIterator iter = pAttributes.find (strName);
    if (iter != pAttributes.end())
        pAttributes.erase(iter);
}

void XMLElement::deleteAttribute(const char* pszName)
{
    XMLAttribute* pAttribute = getAttribute( pszName );
    if ( pAttribute ) pAttribute->Delete();
}


}

NS_IZENELIB_IR_END

#endif
