#ifndef COMPRESSION_COMPRESSOR_H
#define COMPRESSION_COMPRESSOR_H

#include <cstddef>

class Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen) = 0;

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen) = 0;

    virtual int compressBound(int size);
};

class LzoCompressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);
};

class Lz4Compressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);

    virtual int compressBound(int size);
};

class Lz4hcCompressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);

    virtual int compressBound(int size);
};

class Lz77Compressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);
};

class LzendCompressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);
};

class LzfxCompressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);
};

class SnappyCompressor : public Compressor{
public:
    virtual bool compress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t& destLen);

    virtual bool decompress(const unsigned char* src, size_t srcLen, unsigned char* dest, size_t destLen);

    virtual int compressBound(int size);
};

typedef Lz4Compressor DocumentCompressor;

#endif
