#ifndef DOCLENGTH_WRITER_H
#define DOCLENGTH_WRITER_H

#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class DocLengthWriter{
public:
    DocLengthWriter(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory);

    ~DocLengthWriter();

public:
    void fillData(fieldid_t fid, size_t len, unsigned char* docLength);

    void add(docid_t docID, const unsigned char* docLength);

    void flush();

    size_t getWidth() {return width_;}

private:
    Directory* pDirectory_;

    IndexOutput* pOutput_;

    size_t numIndexedProperties_;

    size_t width_;

    unsigned char* propertyOffsetMap_;
};

}

NS_IZENELIB_IR_END

#endif

