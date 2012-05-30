#include <ir/index_manager/utility/XML.h>

#include <util/string/StringUtils.h>

#include <fstream>
#include <assert.h>


using namespace std;

using namespace izenelib::ir::indexmanager;
using namespace izenelib::util;

//////////////////////////////////////////////////////////////////////
// XMLNode construction

XMLNode::XMLNode(XMLElement* pParent, const char* pszName)
{
    nNode = xmlNode;
    pParent = pParent;
    if ( pszName ) sName = pszName;
}

XMLNode::~XMLNode()
{
}

//////////////////////////////////////////////////////////////////////
// XMLNode parsing

bool XMLNode::parseMatch(const char*& pszBase, const char* pszToken)
{
    const char* pszXML = pszBase;
    int nParse = 0;

    for ( ; *pszXML == ' ' || *pszXML == '\t' || *pszXML == '\r' || *pszXML == '\n' ; pszXML++, nParse++ );
    if ( ! *pszXML ) return false;

    for ( ; *pszXML && *pszToken ; pszXML++, pszToken++, nParse++ )
    {
        if ( *pszXML != *pszToken ) return false;
    }

    pszBase += nParse;

    return true;
}

bool XMLNode::parseIdentifier(const char*& pszBase, string& strIdentifier)
{
    const char* pszXML = pszBase;
    int nParse = 0;

    for ( ; *pszXML == ' ' || *pszXML == '\t' || *pszXML == '\r' || *pszXML == '\n' ; pszXML++, nParse++ );
    if ( ! *pszXML ) return false;

    int nIdentifier = 0;
    for ( ; *pszXML && ( isalnum( *pszXML ) || *pszXML == ':' || *pszXML == '_' ) ; pszXML++, nIdentifier++ );
    if ( ! nIdentifier ) return false;

    pszBase += nParse;
    strncpy( Utilities::getBuffer(strIdentifier,nIdentifier), pszBase, nIdentifier );
    pszBase += nIdentifier;

    return true;
}

//////////////////////////////////////////////////////////////////////
// XMLNode string to value

string XMLNode::stringToValue(const char*& pszXML, int nLength)
{
    string strValue;

    if ( ! nLength || ! *pszXML ) return strValue;

    char* pszValue = Utilities::getBuffer(strValue,nLength);
    char* pszOut = pszValue;

    char* pszNull = (char*)pszXML + nLength;
    char cNull = *pszNull;
    *pszNull = 0;

    while ( *pszXML && pszXML < pszNull )
    {
        if ( isspace( *pszXML ) )
        {
            if ( pszValue != pszOut ) *pszOut++ = ' ';
            pszXML++;
            while ( *pszXML && isspace( *pszXML ) ) pszXML++;
            if ( ! *pszXML || pszXML >= pszNull ) break;
        }

        if ( *pszXML == '&' )
        {
            pszXML++;
            if ( ! *pszXML || pszXML >= pszNull ) break;

            if ( strncasecmp( pszXML, "amp;", 4 ) == 0 )
            {
                *pszOut++ = '&';
                pszXML += 4;
            }
            else if ( strncasecmp( pszXML, "lt;", 3 ) == 0 )
            {
                *pszOut++ = '<';
                pszXML += 3;
            }
            else if ( strncasecmp( pszXML, "gt;", 3 ) == 0 )
            {
                *pszOut++ = '>';
                pszXML += 3;
            }
            else if ( strncasecmp( pszXML, "quot;", 5 ) == 0 )
            {
                *pszOut++ = '\"';
                pszXML += 5;
            }
            else if ( strncasecmp( pszXML, "apos;", 5 ) == 0 )
            {
                *pszOut++ = '\'';
                pszXML += 5;
            }
            else if ( strncasecmp( pszXML, "nbsp;", 5 ) == 0 )
            {
                *pszOut++ = ' ';
                pszXML += 5;
            }
            else if ( *pszXML == '#' )
            {
                union
                {
                    long unsigned int ptr;
                    char ret;
                }nChar;
                pszXML++;
                if ( ! *pszXML || pszXML >= pszNull || ! isdigit( *pszXML ) ) break;

                if ( sscanf( pszXML, "%lu;", &nChar.ptr ) == 1 )
                {
                    *pszOut++ = nChar.ret;
                    while ( *pszXML && *pszXML != ';' ) pszXML++;
                    if ( ! *pszXML || pszXML >= pszNull ) break;
                    pszXML++;
                }
            }
            else
            {
                *pszOut++ = '&';
            }
        }
        else
        {
            *pszOut++ = *pszXML++;
        }
    }

    //ASSERT( pszNull == pszXML );
    *pszNull = cNull;

    //ASSERT( pszOut - pszValue <= nLength );
    //strValue.ReleaseBuffer( (int)( pszOut - pszValue ) );

    return strValue;
}

//////////////////////////////////////////////////////////////////////
// XMLNode value to string

#define V2S_APPEND(x,y)	\
        if ( (x) > nOut ) \
            { \
            nOut += (x) + 16; \
            pszOut = Utilities::getBuffer(strXML, nLen + nOut ) + nLen; \
            } \
            { for ( const char* pszIn = (y) ; *pszIn ; nOut--, nLen++ ) *pszOut++ = *pszIn++; }

void XMLNode::valueToString(const char* pszValue, string& strXML)
{
    int nLen = (int)strXML.length();
    int nOut = (int)strlen( pszValue );
    char* pszOut = Utilities::getBuffer(strXML, nLen + nOut ) + nLen;

    for ( ; *pszValue ; pszValue++ )
    {
        int nChar = (int)(unsigned char)*pszValue;

        switch ( nChar )
        {
        case '&':
            V2S_APPEND( 5, "&amp;" );
            break;
        case '<':
            V2S_APPEND( 4, "&lt;" );
            break;
        case '>':
            V2S_APPEND( 4, "&gt;" );
            break;
        case '\"':
            V2S_APPEND( 6, "&quot;" );
            break;
        case '\'':
            V2S_APPEND( 6, "&apos;" );
            break;
        default:
            if ( nChar > 127 )
            {
                char item[20];
                sprintf(item,"&#%lu;",(long unsigned int)nChar);
                string strItem = item;
                //V2S_APPEND( (int)strItem.length(), strItem.c_str());
                *pszOut++=nChar;
            }
            else if ( nOut > 0 )
            {
                *pszOut++ = nChar;
                nOut--;
                nLen++;
            }
            else
            {
                nOut += 16;
                pszOut = Utilities::getBuffer(strXML, nLen + nOut ) + nLen;
                *pszOut++ = nChar;
                nOut--;
                nLen++;
            }
            break;
        }
    }

    //strXML.resize( nLen );
}

//////////////////////////////////////////////////////////////////////
// XMLNode string helper

void XMLNode::uniformString(string& str)
{
    // non-alphanumeric characters which will not be ignored
    static const char* pszOK = "'-&/,;#()";

    TrimLeft(str);
    TrimRight(str);

    bool bSpace = true;

    for ( size_t nPos = 0 ; nPos < str.length() ; nPos++ )
    {
        int nChar = (int)(unsigned char)str.at( nPos );

        if ( nChar <= 32 )
        {
            if ( bSpace )
            {
                str = str.substr(0,nPos) + str.substr(nPos + 1 );
                //str = str.Left( nPos ) + str.Mid( nPos + 1 );
                nPos--;
            }
            else
            {
                if ( nChar != 32 ) str[nPos] = 32;
                bSpace = true;
            }
        }
        else if ( ! isalnum( nChar ) && nChar < 0xC0 && strchr( pszOK, nChar ) == NULL )
        {
            //str = str.Left( nPos ) + str.Mid( nPos + 1 );
            str = str.substr(0,nPos) + str.substr(nPos + 1 );
            nPos--;
        }
        else
        {
            bSpace = false;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// XMLElement construction

XMLElement::XMLElement(XMLElement* pParent, const char* pszName) : XMLNode( pParent, pszName )
{
    nNode = xmlElement;
}

XMLElement::~XMLElement()
{
    deleteAllElements();
    deleteAllAttributes();
}

//////////////////////////////////////////////////////////////////////
// XMLElement clone

XMLElement* XMLElement::clone(XMLElement* pParent)
{
    XMLElement* pClone = new XMLElement( pParent, sName.c_str() );

    for ( AttributeIterator iter = getAttributeIterator() ; hasNextAttribute(iter) ; )
    {
        XMLAttribute* pAttribute = getNextAttribute( iter )->clone( pClone );
        string strName( pAttribute->sName );

        boost::to_lower(strName);
        pClone->pAttributes.insert(AttributePair(strName, pAttribute ));
    }

    for ( ElementIterator iter = getElementIterator() ; hasNextElement(iter) ; )
    {
        XMLElement* pElement = getNextElement( iter );
        pClone->pElements.push_back( pElement->clone( pClone ) );
    }

    pClone->sValue = sValue;

    return pClone;
}

//////////////////////////////////////////////////////////////////////
// XMLElement delete

void XMLElement::deleteAllElements()
{
    ElementIterator iter = pElements.begin();
    while (iter != pElements.end())
    {
        delete (*iter);
        iter++;
    }
    pElements.clear();
}

void XMLElement::deleteAllAttributes()
{
    AttributeIterator iter = pAttributes.begin();
    while (iter != pAttributes.end())
    {
        delete iter->second;
        iter++;
    }
    pAttributes.clear();
}

//////////////////////////////////////////////////////////////////////
// XMLElement to string

string XMLElement::toString(bool bHeader, bool bNewline)
{
    string strXML;
    if ( bHeader ) strXML = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
    if ( bNewline ) strXML += "\r\n";
    toString( strXML, bNewline );
    //ASSERT( strXML.GetLength() == strlen(strXML) );
    return strXML;
}

#define WRITE_TABS for(nTab = 0;nTab < nDepth;nTab++){strXML += "\t";}
void XMLElement::toString(string& strXML, bool bNewline,int nDepth)
{
    int nTab = 0;
    if (bNewline)
        WRITE_TABS;

    strXML += '<' + sName;

    for ( AttributeIterator iter1 = getAttributeIterator(); hasNextAttribute(iter1) ; )
    {
        strXML += ' ';
        XMLAttribute* pAttribute = getNextAttribute( iter1 );
        pAttribute->toString( strXML );
    }

    ElementIterator iter = getElementIterator();

    if ( (hasNextElement(iter) == false) && sValue.empty() )
    {
        strXML += "/>";
        if ( bNewline ) strXML += "\r\n";
        return;
    }

    strXML += '>';
    if ( bNewline && hasNextElement(iter) )
    {
        strXML += "\r\n";
    }

    while ( hasNextElement(iter) )
    {
        XMLElement* pElement = getNextElement( iter );
        pElement->toString( strXML, bNewline, nDepth + 1);
    }

    valueToString( sValue.c_str(), strXML );
    if ((getElementCount() > 0) && bNewline)
        WRITE_TABS;
    strXML += "</" + sName + '>';
    if ( bNewline ) strXML += "\r\n";
}

//////////////////////////////////////////////////////////////////////
// XMLElement from string

XMLElement* XMLElement::fromString(const char* pszXML, bool bHeader)
{
    XMLElement* pElement	= NULL;
    const char* pszElement		= NULL;

    try
    {
        if ( parseMatch( pszXML, "<?xml version=\"" ) )
        {
            pszElement = strstr( pszXML, "?>" );
            if ( ! pszElement ) return false;
            pszXML = pszElement + 2;
        }
        else if ( bHeader ) return NULL;

        while ( parseMatch( pszXML, "<!--" ) )
        {
            pszElement = strstr( pszXML, "-->" );
            if ( ! pszElement || *pszElement != '-' ) return false;
            pszXML = pszElement + 3;
        }

        if ( parseMatch( pszXML, "<!DOCTYPE" ) )
        {
            pszElement = strstr( pszXML, ">" );
            if ( ! pszElement ) return false;
            pszXML = pszElement + 1;
        }

        while ( parseMatch( pszXML, "<!--" ) )
        {
            pszElement = strstr( pszXML, "-->" );
            if ( ! pszElement || *pszElement != '-' ) return false;
            pszXML = pszElement + 3;
        }

        pElement = new XMLElement();

        if ( ! pElement->parseString( pszXML ) )
        {
            delete pElement;
            pElement = NULL;
        }
    }
    catch (...)
    {
    }

    return pElement;
}

bool XMLElement::parseString(const char*& strXML)
{
    if ( ! parseMatch( strXML, "<" ) ) return false;

    if ( ! parseIdentifier( strXML, sName ) ) return false;

    const char* pszEnd = strXML + strlen( strXML );

    while ( ! parseMatch( strXML, ">" ) )
    {
        if ( parseMatch( strXML, "/" ) )
        {
            return parseMatch( strXML, ">" );
        }

        if ( ! *strXML || strXML >= pszEnd ) return false;

        XMLAttribute* pAttribute = new XMLAttribute( this );

        if ( pAttribute->parseString( strXML ) )
        {
            string strName( pAttribute->sName );
            boost::to_lower(strName);
            AttributeIterator iter = pAttributes.find(strName);
            if (iter != pAttributes.end())
            {
                delete iter->second;
                pAttributes.erase(iter);
            }
            pAttributes.insert(AttributePair(strName,pAttribute));
        }
        else
        {
            delete pAttribute;
            return false;
        }
    }

    string strClose = "</";
    strClose += sName + '>';

    while ( true )
    {
        if ( ! *strXML || strXML >= pszEnd ) return false;

        const char* pszElement = strchr( strXML, '<' );
        if ( ! pszElement || *pszElement != '<' ) return false;

        if ( pszElement > strXML )
        {
            //if ( sValue.length() && sValue.substr(0,1) != ' ' ) sValue += ' ';
            sValue += stringToValue( strXML, (int)( pszElement - strXML ) );
            assert( strXML == pszElement );
            if ( strXML != pszElement ) return false;
        }

        if ( parseMatch( strXML, strClose.c_str() ) )
        {
            break;
        }
        else if ( parseMatch( strXML, "<!--" ) )
        {
            pszElement = strstr( strXML, "-->") ;
            if ( ! pszElement || *pszElement != '-' ) return false;
            strXML = pszElement + 3;
        }
        else
        {
            XMLElement* pElement = new XMLElement( this );

            if ( pElement->parseString( strXML ) )
            {
                pElements.push_back( pElement );
            }
            else
            {
                delete pElement;
                return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// XMLElement from bytes

XMLElement* XMLElement::fromBytes(uint8_t* pByte, size_t nByte, bool bHeader)
{
    string strXML;

    if ( nByte >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
    {
        nByte = nByte / 2 - 1;

        if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
        {
            pByte += 2;

            for ( uint32_t nSwap = 0 ; nSwap < (uint32_t)nByte ; nSwap ++ )
            {
                register char nTemp = pByte[ ( nSwap << 1 ) + 0 ];
                pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
                pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
            }
        }
        else
        {
            pByte += 2;
        }


        memcpy(Utilities::getBuffer(strXML,nByte),pByte,nByte*sizeof(char));
    }
    else
    {
        if ( nByte >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF )
        {
            pByte += 3;
            nByte -= 3;
        }

        size_t nWide = Utilities::utf8towcs(NULL,0,(const char*)pByte,nByte);
        wchar_t* buf = new wchar_t[nWide + 1];
        Utilities::utf8towcs(buf,nWide,(const char*)pByte,nByte);
        buf[nWide] = 0;
        size_t nLen = Utilities::_wcstombs(NULL,0,buf,nWide);
        Utilities::_wcstombs(Utilities::getBuffer(strXML,nLen),nLen,buf,nWide);
        delete[] buf;

    }

    return fromString( strXML.c_str(), bHeader );
}

//////////////////////////////////////////////////////////////////////
// XMLElement from file

XMLElement* XMLElement::fromFile(const string& sPath, bool bHeader /* = false */)
{
    FILE* hFile = NULL;
    uint8_t* buf = NULL;
    try
    {
        hFile = fopen(sPath.c_str(),"rb");
        size_t len = 0;
        if (hFile)
        {
            fseek(hFile,0,SEEK_END);
            len = ftell(hFile);
            fseek(hFile,0,SEEK_SET);
            if (len <= 0)
            {
                fclose(hFile);
                return NULL;
            }
            buf = new uint8_t[len + 1];
            IASSERT(fread(buf,sizeof(char),len,hFile) == len);
            buf[len] = '\0';
            XMLElement* pXML = fromBytes(buf,len,bHeader);
            delete[] buf;
            fclose(hFile);
            hFile = NULL;
            return pXML;
        }
    }
    catch (...)
    {
        if (hFile)
            fclose(hFile);
        if (buf)
            delete[] buf;
    }
    return NULL;
}

void XMLElement::toFile(const string& sPath, bool bHeader)
{
    FILE* hFile = NULL;
    try
    {
        string sXML = toString(bHeader,true);
        hFile = fopen(sPath.c_str(),"wb");
        if (hFile)
        {
            size_t nWide = Utilities::_mbstowcs(NULL,0,sXML.c_str(),sXML.length());
            wchar_t* wide = new wchar_t[nWide + 1];
            Utilities::_mbstowcs(wide,nWide,sXML.c_str(),sXML.length());
            wide[nWide] = 0;

            size_t nMulti = Utilities::wcstoutf8(NULL,0,wide,nWide);
            char* multi = new char[nMulti + 1];
            Utilities::wcstoutf8(multi,nMulti,wide,nWide);
            multi[nMulti] = 0;
            IASSERT(fwrite(multi,sizeof(char),nMulti,hFile) == nMulti);
            delete[] wide;
            delete[] multi;

            fclose(hFile);
            hFile = NULL;
        }
    }
    catch (...)
    {
        if (hFile)
        {
            fclose(hFile);
        }
    }
}
//////////////////////////////////////////////////////////////////////
// XMLElement equality

bool XMLElement::equals(XMLElement* pXML)
{
    if ( this == NULL || pXML == NULL ) return false;
    if ( pXML == this ) return true;

    if ( sName != pXML->sName ) return false;
    if ( sValue != pXML->sValue ) return false;

    if ( getAttributeCount() != pXML->getAttributeCount() ) return false;
    if ( getElementCount() != pXML->getElementCount() ) return false;

    for ( AttributeIterator iter = getAttributeIterator() ; hasNextAttribute(iter); )
    {
        XMLAttribute* pAttribute1 = getNextAttribute( iter );
        XMLAttribute* pAttribute2 = pXML->getAttribute( pAttribute1->sName.c_str() );
        if ( pAttribute2 == NULL ) return false;
        if ( ! pAttribute1->equals( pAttribute2 ) ) return false;
    }

    ElementIterator iter1 = getElementIterator();
    ElementIterator iter2 = pXML->getElementIterator();

    for ( ; hasNextElement(iter1) && pXML->hasNextElement(iter2) ; )
    {
        XMLElement* pElement1 = getNextElement( iter1 );
        XMLElement* pElement2 = pXML->getNextElement( iter2 );
        if ( pElement1 == NULL || pElement2 == NULL ) return false;
        if ( ! pElement1->equals( pElement2 ) ) return false;
    }

    if ( (hasNextElement(iter1) != false) || (pXML->hasNextElement(iter2) != false) ) return false;

    return true;
}


//////////////////////////////////////////////////////////////////////
// XMLAttribute construction

const char* XMLAttribute::xmlnsSchema		= "http://www.w3.org/2001/XMLSchema";
const char* XMLAttribute::xmlnsInstance	= "http://www.w3.org/2001/XMLSchema-instance";
const char* XMLAttribute::schemaName		= "xsi:noNamespaceSchemaLocation";

XMLAttribute::XMLAttribute(XMLElement* pParent, const char* pszName) : XMLNode( pParent, pszName )
{
    nNode = xmlAttribute;
}

XMLAttribute::~XMLAttribute()
{
}

//////////////////////////////////////////////////////////////////////
// XMLAttribute clone

XMLAttribute* XMLAttribute::clone(XMLElement* pParent)
{
    XMLAttribute* pClone = new XMLAttribute( pParent, sName.c_str() );
    pClone->sValue = sValue;
    return pClone;
}

//////////////////////////////////////////////////////////////////////
// XMLAttribute to string

void XMLAttribute::toString(string& strXML)
{
    strXML += sName + "=\"";
    valueToString( sValue.c_str(), strXML );
    strXML += '\"';
}

//////////////////////////////////////////////////////////////////////
// XMLAttribute from string

bool XMLAttribute::parseString(const char*& strXML)
{
    if ( ! parseIdentifier( strXML, sName ) ) return false;
    if ( ! parseMatch( strXML, "=") ) return false;

    if ( parseMatch( strXML, "\"") )
    {
        const char* pszQuote = strchr( strXML,  '\"' );
        if ( ! pszQuote || *pszQuote != '\"' ) return false;

        sValue = stringToValue( strXML, (int)( pszQuote - strXML ) );

        return parseMatch( strXML, "\"" );
    }
    else if ( parseMatch( strXML, "'") )
    {
        const char* pszQuote = strchr( strXML,  '\'' );
        if ( ! pszQuote || *pszQuote != '\'' ) return false;

        sValue = stringToValue( strXML, (int)( pszQuote - strXML ) );

        return parseMatch( strXML, "\'" );
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////
// XMLAttribute equality

bool XMLAttribute::equals(XMLAttribute* pXML)
{
    if ( this == NULL || pXML == NULL ) return false;
    if ( pXML == this ) return true;

    if ( sName != pXML->sName ) return false;
    if ( sValue != pXML->sValue ) return false;

    return true;
}
