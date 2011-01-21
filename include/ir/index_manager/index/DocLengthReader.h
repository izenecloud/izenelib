/**
* @file        DocLengthReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Document(property) length is stored alone, it is loaded into
* bitmap when requested
*/
#ifndef DOCLENGTH_READER_H
#define DOCLENGTH_READER_H

#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/utility/system.h>

#include <boost/thread.hpp>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/*
*@brief DocLengthReader
*/
class DocLengthReader{
public:
    DocLengthReader(
        const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, 
        Directory* pDirectory
    );

    ~DocLengthReader();

public:
    ///Load bitmap into memory
    void load(docid_t maxDocId);

    ///get property length for a document
    size_t docLength(docid_t docId, fieldid_t fid);

    ///get average property length of total documents
    double averagePropertyLength(fieldid_t fid);

private:
    Directory* pDirectory_;

    unsigned char* propertyOffsetMap_;

    ///used to store indexed property that does not need to store doc length value
    unsigned char* propertyDocLenMap_;

    ///used to store bitmap, each property length occupy 16bit, which indicates the
    ///max property length is limited to 65535
    doclen_t* data_;

    size_t numIndexedProperties_;

    size_t size_;

    mutable boost::mutex mutex_;
};

}

NS_IZENELIB_IR_END

#endif


