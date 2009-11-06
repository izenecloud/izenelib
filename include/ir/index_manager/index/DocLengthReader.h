#ifndef DOCLENGTH_READER_H
#define DOCLENGTH_READER_H

#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class DocLengthReader{
public:
    DocLengthReader(const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, Directory* pDirectory);

    ~DocLengthReader();

public:
    void load(docid_t maxDocId);

    size_t docLength(docid_t docId, fieldid_t fid);

    double averagePropertyLength(fieldid_t fid);

private:
    Directory* pDirectory_;

    unsigned char* propertyOffsetMap_;

    uint16_t* data_;

    size_t numIndexedProperties_;

    size_t size_;
};

}

NS_IZENELIB_IR_END

#endif


