#include <compression/compressor.h>
#include <compression/minilzo/minilzo.h>
#include <compression/lz4/lz4.h>
#include <compression/lz4/lz4hc.h>
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
    return re == LZO_E_OK;
}

bool LzoCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    size_t tmpLen;
    int re = lzo1x_decompress(src, srcLen, dest, &tmpLen, NULL);
    return re == LZO_E_OK && tmpLen == destLen;
}

bool Lz4Compressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    destLen = LZ4_compress((const char*)src, (char*)dest, srcLen);
    return destLen != 0;
}

bool Lz4Compressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    int tmpLen = LZ4_uncompress((const char*)src, (char*)dest, destLen);
    return tmpLen >= 0 && (size_t)tmpLen == srcLen;
}

int Lz4Compressor::compressBound(int size){
    return LZ4_compressBound(size);
}

bool Lz4hcCompressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    destLen = LZ4_compressHC((const char*)src, (char*)dest, srcLen);
    return destLen != 0;
}

bool Lz4hcCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    int tmpLen = LZ4_uncompress((const char*)src, (char*)dest, destLen);
    return tmpLen >= 0 && (size_t)tmpLen != srcLen;
}

int Lz4hcCompressor::compressBound(int size){
    return LZ4_compressBound(size);
}

bool SnappyCompressor::compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen){
    snappy::RawCompress((const char*)src, srcLen, (char*)dest, &destLen);
    return destLen != 0;
}

bool SnappyCompressor::decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen){
    bool ret = snappy::RawUncompress((const char*)src, srcLen, (char*)dest);
    if (!ret)
    {
        destLen = 0;
        return false;
    }
    ret = snappy::GetUncompressedLength((const char*)src, srcLen, &destLen);
    return ret != 0 && destLen != 0;
}

int SnappyCompressor::compressBound(int size){
    return (int)snappy::MaxCompressedLength(size);
}
