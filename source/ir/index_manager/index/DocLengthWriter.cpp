#include <ir/index_manager/index/DocLengthWriter.h>

using namespace izenelib::ir::indexmanager;

DocLengthWriter::DocLengthWriter(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory)
    :numIndexedProperties_(0)
{
    size_t len = schema.size();
    propertyOffsetMap_ = new unsigned char[len];
    memset(propertyOffsetMap_, 0, len);
    size_t i = 0;
    size_t offset = 0;
    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator iter = schema.begin(); iter != schema.end(); ++iter, ++i)
    {
        if(iter->isForward()&&iter->isIndex())
        {
            numIndexedProperties_++;
            propertyOffsetMap_[i] = offset++;
        }
    }
    width_ = numIndexedProperties_*sizeof(uint16_t);
    size_t buffersize = 1024*1024;
    pOutput_ = pDirectory->createOutput("doclen.map", buffersize, "r+");
}

DocLengthWriter::~DocLengthWriter()
{
    delete propertyOffsetMap_;
    delete pOutput_;
}

void DocLengthWriter::fillData(fieldid_t fid, size_t len, unsigned char* docLength)
{
    size_t offset = propertyOffsetMap_[fid];
    docLength[offset] = (uint16_t) len;
}

void DocLengthWriter::add(docid_t docID, const unsigned char* docLength)
{
    pOutput_->seek(docID*sizeof(width_));
    pOutput_->writeBytes(docLength,width_);
}

void DocLengthWriter::flush()
{
    pOutPut_->flush();
}
