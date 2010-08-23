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

namespace izenelib{namespace util{namespace compression{

template<typename CompressorPolicy>
class Compressor:public CompressorPolicy
{
public:
    int compress(unsigned int* input, unsigned int* output, int size)
    {
        return this->_compress(input,output,size);
    }

    int decompress(unsigned int* input, unsigned int* output, int size)
    {
        return this->_decompress(input,output,size);
    }
    int compress(uint32_t* const src, char* des, int length)
    {
        return this->_compress(src, des, length);
    }

    int decompress(char* const src, uint32_t* des, int length)
    {
        return this->_decompress(src, des, length);
    }
};

typedef Compressor<vbyte_compressor> VByte_Compressor;
typedef Compressor<pfordelta_mix_compressor> PForDeltaMix_Compressor;
typedef Compressor<s16_compressor> S16_Compressor;

}}}
#endif

