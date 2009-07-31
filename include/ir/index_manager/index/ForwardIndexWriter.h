#ifndef FORWARDINDEXWRITER_H
#define FORWARDINDEXWRITER_H

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/ForwardIndex.h>

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <deque>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class ForwardIndexWriter{
public:
    ForwardIndexWriter(Directory* pDirectory);

    ~ForwardIndexWriter();

public:
    void addDocument(docid_t docID);

    void addProperty(fieldid_t fid, boost::shared_ptr<LAInput> laInput);
	
    void flush();

private:
    Directory* pDirectory_;

    IndexOutput* pDOCOutput_;

    IndexOutput* pFDIOutput_;

    IndexOutput* pVOCOutput_;

    IndexOutput* pPOSOutput_;

    ForwardIndex* forwardIndex_;
};

}

NS_IZENELIB_IR_END

#endif

