#include <ir/index_manager/index/DocLengthReader.h>

#include <iostream>

#define MAX_PROPERTIES    100

using namespace izenelib::ir::indexmanager;

DocLengthReader::DocLengthReader(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory)
    :pDirectory_(pDirectory)
    ,data_(NULL)
    ,numIndexedProperties_(0)
{
    propertyOffsetMap_ = new unsigned char[MAX_PROPERTIES];
    memset(propertyOffsetMap_, 0, MAX_PROPERTIES);
    size_t i = 0;
    size_t offset = 0;
    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator iter = schema.begin(); iter != schema.end(); ++iter, ++i)
    {
        if(iter->isForward()&&iter->isIndex())
        {
            numIndexedProperties_++;
            propertyOffsetMap_[iter->getPropertyId()] = offset++;
        }
    }
}

DocLengthReader::~DocLengthReader()
{
    delete propertyOffsetMap_;
    if(data_) delete data_;
}

void DocLengthReader::load(docid_t maxDocId)
{
    size_ = maxDocId * numIndexedProperties_;
    data_ = new uint16_t[size_];
    memset(data_, 0, size_*2);
    IndexInput* pInput = pDirectory_->openInput("doclen.map");
    pInput->readBytes((unsigned char*)data_, size_*2);
}

size_t DocLengthReader::docLength(docid_t docId, fieldid_t fid)
{
    if(docId*numIndexedProperties_ > size_)
        return 0;
    return data_[docId*numIndexedProperties_+propertyOffsetMap_[fid]];
}

