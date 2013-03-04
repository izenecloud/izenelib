#include <compression/compressor.h>
#include <compression/minilzo/minilzo.h>
#include <compression/lz4hc/lz4.h>
#include <compression/lz4hc/lz4hc.h>
#include <compression/snappy/snappy.h>

#include <iostream>
using namespace std;

int Compressor::compressBound(int size){
    return (size + (size / 16) + 64 + 3);
}

bool LzoCompressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    static lzo_align_t __LZO_MMODEL
    wrkmem [((LZO1X_1_MEM_COMPRESS) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t)];
    int re = lzo1x_1_compress(src, srcLen, dest, &destLen, wrkmem);
    if(re != LZO_E_OK)
        return false;
    return true;
}

bool LzoCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    size_t tmpLen;
    int re = lzo1x_decompress(src, srcLen, dest, &tmpLen, NULL);
    if(re != LZO_E_OK || tmpLen != destLen)
        return false;
    return true;
}

bool Lz4Compressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    destLen = LZ4_compress((const char*)src, (char*)dest, srcLen);
    if(destLen == 0)
        return false;
    return true;
}

bool Lz4Compressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    int tmpLen;
    tmpLen = LZ4_uncompress((const char*)src, (char*)dest, destLen);
    if(tmpLen < 0 || (size_t)tmpLen != srcLen){
        return false;
    }
    return true;
}

int Lz4Compressor::compressBound(int size){
    return LZ4_compressBound(size);
}

bool Lz4hcCompressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    destLen = LZ4_compressHC((const char*)src, (char*)dest, srcLen);
    if(destLen == 0)
        return false;
    return true;
}

bool Lz4hcCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    int tmpLen;
    tmpLen = LZ4_uncompress((const char*)src, (char*)dest, destLen);
    if(tmpLen < 0 || (size_t)tmpLen != srcLen){
        return false;
    }
    return true;
}

int Lz4hcCompressor::compressBound(int size){
    return LZ4_compressBound(size);
}

bool SnappyCompressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    snappy::RawCompress((const char*)src, srcLen, (char*)dest, &destLen);
    if(destLen == 0)
        return false;
    return true;
}

bool SnappyCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    bool ret = snappy::RawUncompress((const char*)src, srcLen, (char*)dest);
    if (!ret)
    {
        destLen = 0;
        return false;
    }
    ret = snappy::GetUncompressedLength((const char*)src, srcLen, &destLen);
    if(!ret || destLen == 0){
        return false;
    }
    return true;
}

int SnappyCompressor::compressBound(int size){
    return (int)snappy::MaxCompressedLength(size);
}

