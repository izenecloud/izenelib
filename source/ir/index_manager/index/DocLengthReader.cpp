#include <ir/index_manager/index/DocLengthReader.h>

using namespace izenelib::ir::indexmanager;

DocLengthReader::DocLengthReader(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory)
    :pDirectory_(pDirectory)
    ,data_(NULL)
{
    size_t len = schema.size();
    propertyOffsetMap_ = new unsigned char[len];
    memset(propertyOffsetMap_, 0, len);
    size_t i = 0, numIndexedProperties = 0;
    size_t offset = 0;
    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator iter = schema.begin(); iter != schema.end(); ++iter, ++i)
    {
        if(iter->isForward()&&iter->isIndex())
        {
            numIndexedProperties++;
            propertyOffsetMap_[i] = offset++;
        }
    }
    width_ = numIndexedProperties*sizeof(uint16_t);
}

DocLengthReader::~DocLengthReader()
{
    delete propertyOffsetMap_;
    if(data_) delete data_;
}

void DocLengthReader::load(docid_t maxDocId)
{
    size_ = maxDocId*width_;
    data_ = new uint16_t[size_];
    IndexInput* pInput = pDirectory_->openInput("doclen.map");
    pInput->readBytes((unsigned char*)data_, maxDocId*width_*2);
}

size_t DocLengthReader::docLength(docid_t docId, fieldid_t fid)
{
    if(docId*width_ > size_)
        return 0;
    return (uint16_t)data_[docId*width_+propertyOffsetMap_[fid]];
}
