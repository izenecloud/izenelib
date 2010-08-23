#ifndef IZENE_UTIL_COMPRESSION_INT_VBYTE_COMPRESSOR_H
#define IZENE_UTIL_COMPRESSION_INT_VBYTE_COMPRESSOR_H


namespace izenelib{namespace util{namespace compression{
///just for comparing
class vbyte_compressor
{
public:
    vbyte_compressor();

    ~vbyte_compressor();

    int _compress(unsigned int* input, unsigned int* output, int size);

    int _decompress(unsigned int* input, unsigned int* output, int size);
};

}}}

#endif
