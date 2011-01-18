/**
* @file        DocLengthWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Document(property) length is stored alone, it is written to an alone
* bitmap file during indexing
*/
#ifndef DOCLENGTH_WRITER_H
#define DOCLENGTH_WRITER_H

#include <ir/index_manager/index/IndexerCollectionMeta.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/utility/system.h>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/*
*@brief DocLengthWriter
*/
class DocLengthWriter{
public:
    DocLengthWriter(
        const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema, 
        Directory* pDirectory
    );

    ~DocLengthWriter();

public:
    ///Since each document has multiple properties, when indexing a document, it's better
    ///to collect all property length of that document, and fill them into buffer property
    ///by property, and write to disk in the end. This api is used to fill the buffer
    void fill(fieldid_t fid, size_t len, doclen_t* docLength);
    ///Write the buffer that contains all property length of a document into disk.
    void add(docid_t docID, const doclen_t* docLength);

    void flush();

    size_t get_num_properties() {return numIndexedProperties_;}

private:
    Directory* pDirectory_;

    IndexOutput* pOutput_;

    size_t numIndexedProperties_;

    unsigned char* propertyOffsetMap_;
};

}

NS_IZENELIB_IR_END

#endif

