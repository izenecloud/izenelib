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
            ///This judgement is necessary because aliased properties have the same property id
            if(0 == propertyOffsetMap_[iter->getPropertyId()])
                propertyOffsetMap_[iter->getPropertyId()] = offset++;
        }
    }
}

DocLengthReader::~DocLengthReader()
{
    delete propertyOffsetMap_;
    if(data_) {delete[] data_; data_ = NULL;}
}

void DocLengthReader::load(docid_t maxDocId)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if(data_) {delete[] data_; data_ = NULL;}

    size_ = maxDocId * numIndexedProperties_;
    data_ = new uint16_t[size_];
    memset(data_, 0, size_*2);
    IndexInput* pInput = 0;
    try{
    pInput = pDirectory_->openInput("doclen.map");
    pInput->readBytes((unsigned char*)data_, size_*2);
    delete pInput;
    }catch(std::exception& e)
    {
        if(pInput) delete pInput;
    }
}

size_t DocLengthReader::docLength(docid_t docId, fieldid_t fid)
{
    if(docId*numIndexedProperties_ > size_|| !data_)
        return 0;
    return data_[docId*numIndexedProperties_+propertyOffsetMap_[fid]];
}

double DocLengthReader::averagePropertyLength(fieldid_t fid)
{
    size_t totalLen = 0;
    size_t maxDoc = size_/numIndexedProperties_;
    unsigned char offset = propertyOffsetMap_[fid];
    for(size_t i = 0; i < maxDoc; ++i)
    {
       unsigned int docOffset = i*numIndexedProperties_;
       if(docOffset <= size_)
           totalLen+= data_[docOffset + offset];
    }
    return (double)totalLen/(double)maxDoc;
}


