#include "ScdParser.h"
#include <iostream>
#include <cassert>
#include <cstdio>

#include <wiselib/ustring/UString.h>
#include <wiselib/profiler/ProfilerGroup.h>
#include <boost/filesystem/path.hpp>

using namespace boost::filesystem;


using namespace std;

ScdParser::ScdParser()
    :size_(0)
{
}

ScdParser::ScdParser( const wiselib::UString::EncodingType & encodingType )
    :size_(0), encodingType_(encodingType), docOffsetList_(NULL)
{
}

ScdParser::~ScdParser()
{
    if(fs_.is_open())
        fs_.close();
    if( docOffsetList_ != NULL )
    {
        delete docOffsetList_;
    }
}

bool ScdParser::checkSCDFormat( const string & file )
{
    //B-<document collector ID(2Bytes)>-<YYYYMMDDHHmm(12Bytes)>-
    //    <SSsss(second, millisecond, 5Bytes)>-<I/U/D/R(1Byte)>-
    //    <C/F(1Byte)>.SCD

    if( boost::filesystem::path(file).extension() != ".SCD" 
            && boost::filesystem::path(file).extension() != ".scd"  )
        return false;

    string fileName = boost::filesystem::path(file).stem();

    size_t pos = 0;
    string strVal;
    int val;

    if( fileName.length() != 27 )
        return false;
    if( fileName[pos] != 'B' && fileName[0] != 'b' )
        return false;
    if( fileName[++pos] != '-' ) 
        return false;

    pos++;
    strVal = fileName.substr( pos, 2 );        //year
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;

    pos += 3;
    strVal = fileName.substr( pos, 4 );        //year
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 1970 )
        return false;

    pos += 4;
    strVal = fileName.substr( pos, 2 );        //month
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 1 || val > 12 )
        return false;

    pos += 2;
    strVal = fileName.substr( pos, 2 );        //day
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 1 || val > 31 )
        return false;

    pos += 2;
    strVal = fileName.substr( pos, 2 );        //hour
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 0 || val > 24 )
        return false;

    pos += 2;
    strVal = fileName.substr( pos, 2 );        //minute
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 0 || val >= 60 )
        return false;

    pos += 3;
    strVal = fileName.substr( pos, 2 );        //second
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 0 || val >= 60 )
        return false;

    pos += 2;
    strVal = fileName.substr( pos, 3 );        //millisecond
    sscanf( strVal.c_str(), "%d", &val );
    //cout << val << endl;
    if( val < 0 || val > 999 )
        return false;

    pos += 4;
    strVal = fileName.substr( pos, 1 );        //c
    char c = tolower( strVal[0] );
    //cout << strVal << endl;
    if( c != 'i' && c != 'd' && c != 'u' && c != 'r' )
        return false;

    pos += 2;
    strVal = fileName.substr( pos, 1 );        //d
    c = tolower( strVal[0] );
    //cout << strVal << endl;
    if( c != 'c' && c != 'f' )
        return false;

    
    return true;
}

SCD_TYPE ScdParser::checkSCDType( const string & file )
{
    //B-<document collector ID(2Bytes)>-<YYYYMMDDHHmm(12Bytes)>-
    //    <SSsss(second, millisecond, 5Bytes)>-<I/U/D/R(1Byte)>-
    //    <C/F(1Byte)>.SCD

    string fileName = boost::filesystem::path(file).stem();

    SCD_TYPE type;

    switch( tolower(fileName[24]) )
    {
        case 'i':
            type = INSERT_SCD;           
            break;
        case 'd':
            type = DELETE_SCD;
            break;
        case 'u':
            type = UPDATE_SCD;
            break;
        case 'r':
            type = REPLACE_SCD;
            break;
        default:
            type = NOT_SCD;
            break;
    }
    return type;
}

bool ScdParser::compareSCD (const string & file1, const string & file2)
{
    return (checkSCDType(file1) < checkSCDType(file2));
}


bool ScdParser::load(const string& path)
{
    if(fs_.is_open())
    {
        fs_.close();
    }
    fs_.open(path.c_str(), ios::in);
    if (!fs_)
        return false;
    fs_.seekg(0, ios::end);    
    size_ = fs_.tellg();  
    fs_.seekg(0, ios::beg);
    return true;
}

bool ScdParser::getDocIdList( std::vector<wiselib::UString> & list )
{
    if( docOffsetList_ != NULL )
    {
        delete docOffsetList_;
    }
    docOffsetList_ = new izenelib::am::cccr_hash<wiselib::UString, long>();
    if( !fs_.is_open() )
        return false;

    ScdParser::iterator it = this->begin();
    for( ; it != this->end(); it++ )
    {
        //if the document is NULL 
        if ( (*it) == NULL  )
        {
            return false;
        }

        list.push_back( (*it)->at(0).second );
        docOffsetList_->insert( (*it)->at(0).second, it.getOffset() );
    }
    
    return true;
}

bool ScdParser::getDocIdList( std::vector<DocIdPair > & list )
{
    if( docOffsetList_ != NULL )
    {
        delete docOffsetList_;
    }
    docOffsetList_ = new izenelib::am::cccr_hash<wiselib::UString, long>();
    if( !fs_.is_open() )
        return false;

    ScdParser::iterator it = this->begin();

    for( ; it != this->end(); it++ )
    {
        //if the document is NULL
        if ( (*it) == NULL  )
        {
            return false;
        }
        list.push_back( make_pair((*it)->at(0).second, 0 ));
        docOffsetList_->insert( (*it)->at(0).second, it.getOffset() );
    }
    
    return true;
}

ScdParser::iterator ScdParser::begin(unsigned int start_doc)
{
    fs_.clear();
    fs_.seekg(0, ios::beg);
    return iterator(this, start_doc);
}

ScdParser::iterator ScdParser::end()
{
    //return iterator(size_);
    return iterator(-1);
}

bool ScdParser::getDoc( const wiselib::UString & docId, SCDDoc& doc )
{
    long * val = docOffsetList_->find( docId );
    if( val == NULL )
    {
        return false;
    }

    if( fs_.eof() )
    {
        fs_.clear();
    }
    fs_.seekg( *val, ios::beg );

    iterator it( this, 0 );
    doc = *(*it);

    return true;
}


ScdParser::iterator::iterator(long offset)
    :pfs_(NULL)
{
    offset_ = offset;
    line_.clear();
}

ScdParser::iterator::iterator(ScdParser* pScdParser, unsigned int start_doc)
{
    pfs_ = &(pScdParser->fs_);
    codingType_ = pScdParser->getEncodingType();
    offset_ = 0;
    line_.clear();
    doc_.reset(getDoc());

    if(start_doc > 0)
    {
        for(unsigned int i = 0; i < start_doc; ++i)
            operator++();    
    }
}

ScdParser::iterator::iterator(const iterator& other)
    :pfs_(other.pfs_)
    ,prevOffset_(other.prevOffset_)
    ,offset_(other.offset_)
    ,line_(other.line_)
    ,doc_(other.doc_)
{
}


ScdParser::iterator::~iterator()
{
}

bool ScdParser::iterator::operator==(const iterator& other)const
{
    return offset_ == other.offset_;
}

bool ScdParser::iterator::operator!=(const iterator& other)const
{
    return (offset_ != other.offset_);
}

void ScdParser::iterator::operator++()
{
    if( pfs_->eof() )
    {
        offset_ = -1;
    }
    else
    {
        offset_ = prevOffset_;
        doc_.reset(getDoc());
    }
    //return iterator(*this);
}

void ScdParser::iterator::operator++(int)
{
    if( pfs_->eof() )
    {
        offset_ = -1;
    }
    else
    {
        offset_ = prevOffset_;
        doc_.reset(getDoc());
    }
    //return iterator(*this);
}

SCDDocPtr ScdParser::iterator::operator*()
{
    return doc_;
}

long ScdParser::iterator::getOffset()
{
    return offset_;
}


SCDDoc* ScdParser::iterator::getDoc()
{
CREATE_PROFILER ( proScdParsing, "Index:SIAProcess", "Scd Parsing : parse SCD file");

START_PROFILER ( proScdParsing );

    SCDDoc* doc = NULL; 
    string name("");
    string value("");

    if(line_.empty())
        getNextLine();

    
    //check the first content of SCD to be valid doc
    string propertyName;
    string propertyValue;
    splitFieldNameValue(line_, propertyName, propertyValue);
    if ( (isDocumentBeginField(propertyName) == false) || (propertyValue.size() == 0) )
        return doc;

    do
    {
        if (hasField(line_) == false)
        {
            value.append(line_);
            continue;
        }

        if (name.length() > 0)
        {
            doc->push_back(FieldPair(wiselib::UString(name, codingType_), wiselib::UString(value, codingType_)));
            name = "";
            value = "";
        }

        string tempName;
        string tempValue;
        splitFieldNameValue(line_, tempName, tempValue);

        if (isDocumentBeginField(tempName) == true)
        {
            if(doc != NULL) 
               break;//It's the start line of next document

            doc = new SCDDoc;
        }

        name = tempName;
        value = tempValue;

    }while (getNextLine());

    if(doc != NULL)  
    {
        value.erase(value.find_last_not_of("\r\n") + 1);
        if (name.length() > 0)
        {
            doc->push_back(FieldPair(wiselib::UString(name, codingType_), wiselib::UString(value, codingType_)));
        }

    }

STOP_PROFILER ( proScdParsing );
	
    return doc;
}

bool ScdParser::iterator::getNextLine()
{
    prevOffset_ = pfs_->tellg();
    line_.clear();
    //SCD file might contain blank line
    while((line_.length() == 0) && (!( pfs_->eof())))
        getline(*pfs_, line_);
    //cout << line_ << endl;
    return ! pfs_->eof();//(line_.length() == 0)?false:true;
}

bool ScdParser::iterator::hasField(const std::string& str)
{
    if (str[0] != '<')
        return false;

    std::string::size_type offset = str.find('>', 1);
    if (offset == std::string::npos)
        return false;
    
    return true;
}


bool ScdParser::iterator::splitFieldNameValue(const std::string& str, std::string& name, std::string& value)
{
    if (hasField(str) == false)
        return false;

    std::string::size_type offset = str.find('>', 1);
    assert(offset != std::string::npos);
    name.assign(str, 1, offset - 1);

    value.assign(str.c_str() + offset + 1);
    return true;
}


bool ScdParser::iterator::isDocumentBeginField(const std::string& name)
{
    return (name == "DOCID")? true:false;
}

