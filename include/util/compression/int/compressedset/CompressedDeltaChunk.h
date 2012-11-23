#ifndef  IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_COMPRESSED_DELTA_CHUNK_H__
#define  IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_COMPRESSED_DELTA_CHUNK_H__
#include <vector>
#include "Common.h"
#include "Sink.h"
#include "Source.h"
#include <util/compression/int/fastpfor/memutil.h>

namespace izenelib
{
namespace util
{
namespace compression
{

typedef  AlignedSTLAllocator<uint8, 256> CacheAllocator;
class CompressedDeltaChunk
{
private:
    vector<uint8,CacheAllocator> data_;
    size_t compressedSize_;
    //disable copy constructor
    CompressedDeltaChunk(const CompressedDeltaChunk& other);
    CompressedDeltaChunk& operator=(const CompressedDeltaChunk& other);
public:
    CompressedDeltaChunk();
    CompressedDeltaChunk(size_t compressedSize);
    CompressedDeltaChunk(istream & in);
    void resize(size_t newsize);
    vector<uint8,CacheAllocator>& getVector();
    ~CompressedDeltaChunk();
    size_t getCompressedSize();
    Sink getSink();
    Source getSource() const;
    void write(ostream & out) const;

} __attribute__ ((aligned (256)));

}
}
}
#endif // IZENELIB_UTIL_COMPRESSION_COMPRESSED_SET_COMPRESSED_DELTA_CHUNK_H__
