#include <ir/index_manager/index/PostingReader.h>
#include <ir/index_manager/index/CompressParameters.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{
void PostingReader::ensurePosBufferUpperBound(uint32_t* &pPPosting, int32_t& posBufLength, int32_t minBufLength)
{
    const int32_t upperBound = UncompressedOutBufferUpperbound(minBufLength);
    if(posBufLength < upperBound)
        growPosBuffer(pPPosting, posBufLength, upperBound);
}
}

NS_IZENELIB_IR_END
