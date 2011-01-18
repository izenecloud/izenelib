#include <ir/index_manager/index/DocLengthReader.h>
#include <util/izene_log.h>

#include <iostream>
#include <boost/scoped_ptr.hpp>

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
        if(iter->isIndex() && iter->isAnalyzed())
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
    delete[] propertyOffsetMap_;
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
        LOG(WARNING) << e.what();
    }
}

size_t DocLengthReader::docLength(docid_t docId, fieldid_t fid)
{
    size_t pos = docId*numIndexedProperties_+propertyOffsetMap_[fid];
    if(pos >= size_ || !data_)
        return 0;
    return data_[pos];
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


