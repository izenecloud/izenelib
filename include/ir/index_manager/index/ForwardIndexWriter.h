#ifndef FORWARDINDEXWRITER_H
#define FORWARDINDEXWRITER_H

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/LAInput.h>


#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexOutput.h>

#include <util/DynamicArray.h>

#include <deque>

using namespace izenelib::util;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef std::deque<std::pair<unsigned int, unsigned int> > ForwardIndex;
typedef DynamicArray<ForwardIndex*, Const_NullValue<ForwardIndex*> > DynForwardIndexArray;

class ForwardIndexWriter{
public:
    ForwardIndexWriter(Directory* pDirectory);

    ~ForwardIndexWriter();

public:
    void addDocument(docid_t docID);

    void addProperty(fieldid_t fid, boost::shared_ptr<LAInput> laInput);
	
    void close();

private:
    Directory* pDirectory_;

    IndexOutput* pDOCOutput_;

    IndexOutput* pFDIOutput_;

    IndexOutput* pVOCOutput_;

    IndexOutput* pPOSOutput_;

    DynForwardIndexArray* forwardIndexArray_;
};

}

NS_IZENELIB_IR_END

#endif

