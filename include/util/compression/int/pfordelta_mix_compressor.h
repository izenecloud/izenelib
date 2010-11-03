#ifndef IZENE_UTIL_COMPRESSION_INT_PFORDELTA_MIX_COMPRESSOR_H
#define IZENE_UTIL_COMPRESSION_INT_PFORDELTA_MIX_COMPRESSOR_H

#include <cassert>
#include <stdint.h>

namespace izenelib{namespace util{namespace compression{

/**
 * Optimize PForDelta<br>
 * 1 store exceptions using VINT<br>
 * 2 store the numbers fewer than PFORDELTAMIX_THRESHOLD with VINT
 */
class pfordelta_mix_compressor
{
public:
    /// numbers in each block
    const static int PFORDELTA_BATCH_NUMBER = 128;
    /// the threshold of using VINT
    const static int PFORDELTAMIX_THRESHOLD = 20;
    /// process mode when exception gap is too long
    const static int EXPS_HANDLE_MODE_DEFAULT = 0;
    const static int EXPS_HANDLE_MODE_FIRSTBIT = 1;
    const static int EXPS_HANDLE_MODE_AFTERDATA = 2;
    ///max number for exceptions. if too many exception happens, decompression performance will be affected
    const static int MAX_EXPS_NUM = 30;


public:
    pfordelta_mix_compressor();

    ~pfordelta_mix_compressor();

public:
    int compress(unsigned int* input, unsigned int* output, int size);

    int decompress(unsigned int* input, unsigned int* output, int size);
    /**
     * fixme: preserve "length * sizeof(uint32_t) * 2" space for des
     */
    int compress(uint32_t* const src, char* des, int length);

    /**
     * fixme: preserve more 32 word space for dest
     */
    int decompress(char* const src, uint32_t* des, int length);

public:
    void setDivision(float div)
    {
        if (div > 0.5 && div < 1)
        {
            _division = div;
        }
        else
        {
            div = 0.9;
        }
    }

    void setExpsCompRate(float rate)
    {
        if (rate > 1.0)
        {
            _exps_comp_rate = 0.3;
        }
        else
        {
            _exps_comp_rate = rate;
        }
    }

    ///for different bit_of_num, the overhead for adding an extra bit
    int getCostofAddBit(int bit_of_num)
    {
        static const int cost_of_add_bit[] =
        {
            4,  4,  5,  3,  6,  4,  6,  0,
            11, 0,  21, 0,  0,  0,  0,  0,
            64, 0,  0,  0,  0,  0,  0,  0,
            64, 0,  0,  0,  0,  0,  0,  0
        };

        return cost_of_add_bit[bit_of_num];
    }

protected:
    template<typename T>
    int highBitIdx(T number)
    {
        int idx = 0;
        while (number > 0)
        {
            ++idx;
            number = number >> 1;
        }
        return idx;
    }

    template<typename T>
    void getMaskNumber(T& mask, int maskBit)
    {
        static const int mask_map[] =
        {
            0,
            0x1,       0x3,       0x7,       0xf,       0x1f,       0x3f,       0x7f,       0xff,
            0x1ff,     0x3ff,     0x7ff,     0xfff,     0x1fff,     0x3fff,     0x7fff,     0xffff,
            0x1ffff,   0x3ffff,   0x7ffff,   0xfffff,   0x1fffff,   0x3fffff,   0x7fffff,   0xffffff,
            0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
        };

        mask = mask_map[maskBit];
    }

    template<typename T>
    int encode_p(char* des, T* const src, int length);

    template<typename T>
    int decode_p(T* des, char* const src, int& srcLen);

protected:
    // VINT function
    template<typename T>
    int vint_encode_p(char* des, T* const src, int length);

    template<typename T>
    int vint_decode_p(T* des, char* const src, int length);

    int vint_encode_p(uint32_t* const input, uint32_t* output, int size);

    int vint_decode_p(uint32_t* const input, uint32_t* output, int size);

    template<typename T>
    int pfor_encode_p(char* des, T* const src, int length);

    template<typename T>
    int pfor_decode_p(T* des, char* const src, int& srcLen);

    int encode_block(uint32_t* des, uint32_t* const src, int length);

    int decode_block(uint32_t* des, uint32_t* const src);

    template<typename T>
    unsigned int compressFunc(T value, char * buf, unsigned int & len);

    template<typename T>
    T decompressFunc(char*&  buf);

private:
    pfordelta_mix_compressor(const pfordelta_mix_compressor&);

    pfordelta_mix_compressor& operator=(const pfordelta_mix_compressor&);

protected:
    //caofx: don't use dynamic alloc now for the LocalMessageBlock alloc bug
    char _buffer[int(PFORDELTA_BATCH_NUMBER * (1 - 0.9)) * 4 + 256];
    float _division;
    float _exps_comp_rate;

};

}}}
#endif
