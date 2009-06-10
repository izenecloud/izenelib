#ifndef FORWARDINDEXWRITER_H
#define FORWARDINDEXWRITER_H

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/ForwardIndex.h>


#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexOutput.h>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class ForwardIndexWriter{
public:
    ForwardIndexWriter(Directory* pDirectory);

    ~ForwardIndexWriter();

public:
    void addDocument(docid_t docID);

    void addField(fieldid_t fid, ForwardIndex& forwardIndex);
	
    void close();

private:
    Directory* pDirectory_;

    IndexOutput* pDOCOutput_;

    IndexOutput* pFDIOutput_;

    IndexOutput* pVOCOutput_;

    IndexOutput* pPOSOutput_;
};

}

NS_IZENELIB_IR_END

#endif

