#ifndef FORWARDINDEXREADER_H
#define FORWARDINDEXREADER_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class ForwardIndexReader
{
public:
    ForwardIndexReader(Directory* pDirectory);

    ~ForwardIndexReader();
public:

private:
    IndexInput* pDOCInput_;

    IndexInput* pFDIInput_;

    IndexInput* pVOCInput_;

    IndexInput* pPOSInput_;
};

}
NS_IZENELIB_IR_END

#endif
