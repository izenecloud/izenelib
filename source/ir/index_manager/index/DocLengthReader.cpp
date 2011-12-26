#include <ir/index_manager/index/DocLengthReader.h>
#include <util/izene_log.h>

#include <iostream>
#include <cassert>
#include <boost/scoped_ptr.hpp>

#define MAX_PROPERTIES    100

using namespace izenelib::ir::indexmanager;

DocLengthReader::DocLengthReader(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory)
    :pDirectory_(pDirectory)
    ,propertyOffsetMap_(NULL)
    ,propertyDocLenMap_(NULL)
    ,data_(NULL)
    ,numIndexedProperties_(0)
    ,size_(0)
{
    propertyOffsetMap_ = new unsigned char[MAX_PROPERTIES];
    propertyDocLenMap_  = new unsigned char[MAX_PROPERTIES];
    memset(propertyOffsetMap_, 0, MAX_PROPERTIES);
    memset(propertyDocLenMap_, 0, MAX_PROPERTIES);

    size_t i = 0;
    size_t offset = 0;
    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator iter = schema.begin(); iter != schema.end(); ++iter, ++i)
    {
        if(iter->isIndex() && iter->isAnalyzed() )
        {
            if(iter->isStoreDocLen())
            {
                ///This judgement is necessary because aliased properties have the same property id
                if(0 == propertyOffsetMap_[iter->getPropertyId()])
                {
                    numIndexedProperties_++;
                    propertyOffsetMap_[iter->getPropertyId()] = offset++;
                }
            }
            else
              propertyDocLenMap_[iter->getPropertyId()] = 1;
        }
    }
}

DocLengthReader::~DocLengthReader()
{
    delete[] propertyOffsetMap_;
    delete[] propertyDocLenMap_;
    if(data_) {delete[] data_; data_ = NULL;}
}

void DocLengthReader::load(docid_t maxDocId)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if(data_) {delete[] data_; data_ = NULL;}

    // although doc id 0 is not used, its space data_[0] is reserved,
    // so that for doc id i, we can access its data_ by data_[i]
    size_ = (maxDocId+1) * numIndexedProperties_;
    data_ = new doclen_t[size_];
    memset(data_, 0, size_*sizeof(doclen_t));
    try{
        boost::scoped_ptr<IndexInput> pInput(pDirectory_->openInput("doclen.map"));
        pInput->readBytes((unsigned char*)data_, size_*sizeof(doclen_t));
    }catch(std::exception& e)
    {
     //   LOG(WARNING) << e.what();
    }
}

size_t DocLengthReader::docLength(docid_t docId, fieldid_t fid)
{
    if(!propertyDocLenMap_[fid])
    {
        size_t pos = docId*numIndexedProperties_+propertyOffsetMap_[fid];
        if(pos >= size_ || !data_)
            return 0;
        return data_[pos];
    }
    else
        return propertyDocLenMap_[fid];
}

double DocLengthReader::averagePropertyLength(fieldid_t fid)
{
    if(0 == numIndexedProperties_ || size_ <= numIndexedProperties_)
        return 1;

    // count the doc num without id 0
    const size_t maxDoc = size_ / numIndexedProperties_ - 1;
    assert(maxDoc > 0);

    size_t totalLen = 0;
    unsigned int offset = numIndexedProperties_ + propertyOffsetMap_[fid];
    for(size_t i = 1; i <= maxDoc; ++i)
    {
        totalLen += data_[offset];
        offset += numIndexedProperties_;
    }

    return (double)totalLen / maxDoc;
}


