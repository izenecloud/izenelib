#include <ir/index_manager/index/DocLengthWriter.h>

#define MAX_PROPERTIES    100

using namespace izenelib::ir::indexmanager;

DocLengthWriter::DocLengthWriter(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory)
    :numIndexedProperties_(0)
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
    size_t buffersize = 1024*1024;
    pOutput_ = pDirectory->createOutput("doclen.map", buffersize, "r+");
}

DocLengthWriter::~DocLengthWriter()
{
    delete propertyOffsetMap_;
    delete pOutput_;
}

void DocLengthWriter::fill(fieldid_t fid, size_t len, uint16_t* docLength)
{
    size_t offset = propertyOffsetMap_[fid];
    docLength[offset] = (uint16_t) len;
}

void DocLengthWriter::add(docid_t docID, const uint16_t* docLength)
{
    pOutput_->seek(docID*numIndexedProperties_*sizeof(uint16_t));
    pOutput_->writeBytes((unsigned char*)docLength,numIndexedProperties_*sizeof(uint16_t));
}

void DocLengthWriter::flush()
{
    pOutput_->flush();
}
