#include "ScdParser.h"
#include <iostream>
#include <cassert>
#include <cstdio>

#include <wiselib/ustring/UString.h>
using namespace std;

ScdParser::ScdParser()
    :size_(0), encodingType_(wiselib::UString::CP949)
{
}

ScdParser::ScdParser(wiselib::UString::EncodingType & encodingType)
    :size_(0), encodingType_(encodingType)
{
}

ScdParser::~ScdParser()
{
    if(fs_.is_open())
        fs_.close();
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
    return true;
}

ScdParser::iterator ScdParser::begin(unsigned int start_doc)
{
    fs_.seekg(0, ios::beg);
    return iterator(this, start_doc);
}

ScdParser::iterator ScdParser::end()
{
    //return iterator(size_);
    return iterator(-1);
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
    offset_ = pfs_->tellg();
    line_.clear();
    doc_.reset(getDoc());
    if(start_doc > 0)
    {
        for(unsigned int i = 0; i < start_doc; ++ i)
            operator++();    
    }
}

ScdParser::iterator::iterator(const iterator& other)
    :pfs_(other.pfs_)
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
    return(offset_ != other.offset_);
}

void ScdParser::iterator::operator++()
{
    doc_.reset(getDoc());
    offset_ = pfs_->tellg();
    //return iterator(*this);
}

void ScdParser::iterator::operator++(int)
{
    doc_.reset(getDoc());
    offset_ = pfs_->tellg();
    //return iterator(*this);
}

SCDDocPtr ScdParser::iterator::operator*()
{
    return doc_;
}


SCDDoc* ScdParser::iterator::getDoc()
{
    SCDDoc* doc = NULL; 
    string name("");
    string value("");

    if(line_.empty())
        getNextLine();
        
    do
    {
        if (hasField(line_) == false)
        {
            value.append(line_);
            continue;
        }

       // value.erase(value.find_last_not_of("\r\n") + 1);

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
	
    return doc;
}

bool ScdParser::iterator::getNextLine()
{
    line_.clear();
    //SCD file might contain blank line
    while((line_.length() == 0) && (!( pfs_->eof())))
        getline(*pfs_, line_);
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

