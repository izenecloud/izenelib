/**
 * @file	compressor.h
 * @brief	Header file of integer compression algorithm suits
 * @author	Yingfeng Zhang
 * @date    2010-08-11
 * @details
 * ==============
 */
#ifndef IZENE_UTIL_COMPRESSION_INT_COMPRESSOR_H
#define IZENE_UTIL_COMPRESSION_INT_COMPRESSOR_H

#include <util/compression/int/pfordelta_mix_compressor.h>
#include <util/compression/int/vbyte_compressor.h>
#include <util/compression/int/s16_compressor.h>
#include <util/compression/int/pfordelta_compressor.h>

namespace izenelib{namespace util{namespace compression{

template<typename CompressorPolicy>
class Compressor:public CompressorPolicy
{
public:
    int compress(unsigned int* input, unsigned int* output, int size)
    {
        return upcast()->compress(input,output,size);
    }
    /*
    * param: size: size of umpressed data
    * @return compressed size
    */
    int decompress(unsigned int* input, unsigned int* output, int size)
    {
        return upcast()->decompress(input,output,size);
    }

    ///////////////////////////////////////////////////////////
    ///There are two kinds of compress and decompress interfaces.
    ///The major difference is the decompress
    ///////////////////////////////////////////////////////////
    int compress(uint32_t* const src, char* des, int length)
    {
        return upcast()->compress(src, des, length);
    }
    /*
    * param: length: length of compressed data
    * @return decompressed size
    */
    int decompress(char* const src, uint32_t* des, int length)
    {
        return upcast()->decompress(src, des, length);
    }

protected:
    inline CompressorPolicy * upcast()
    {
        return static_cast<CompressorPolicy *>(this);
    };
    inline const CompressorPolicy * upcast()const
    {
        return static_cast<const CompressorPolicy *>(this);
    };

};

typedef Compressor<vbyte_compressor> VByte_Compressor;
typedef Compressor<pfordelta_mix_compressor> PForDeltaMix_Compressor;
typedef Compressor<s16_compressor> S16_Compressor;
typedef Compressor<pfordelta_compressor> PForDelta_Compressor;

}}}
#endif

