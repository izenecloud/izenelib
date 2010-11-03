#include <util/compression/int/pfordelta_compressor.h>

using namespace std;

namespace izenelib{namespace util{namespace compression{

pfordelta_compressor::pfordelta_compressor() :
        FRAC(0.1)
{
    cnum_[0] = 0;
    cnum_[1] = 1;
    cnum_[2] = 2;
    cnum_[3] = 3;
    cnum_[4] = 4;
    cnum_[5] = 5;
    cnum_[6] = 6;
    cnum_[7] = 7;
    cnum_[8] = 8;
    cnum_[9] = 9;
    cnum_[10] = 10;
    cnum_[11] = 11;
    cnum_[12] = 12;
    cnum_[13] = 13;
    cnum_[14] = 16;
    cnum_[15] = 20;
    cnum_[16] = 32;

    block_size_ = 128;
}

void pfordelta_compressor::set_blocksize(int blockSize)
{
    if (blockSize < 64)
        block_size_ = 32;
    else if (blockSize < 128)
        block_size_ = 64;
    else if (blockSize < 256)
        block_size_ = 128;
    else
        block_size_ = 256;
}


int pfordelta_compressor::compress(unsigned int* input, unsigned int* output, int size)
{
    int total_comp_len = 0;

    for(int i = 0; i < size; i += block_size_)
    {
        int curr_len = 0;
        if ((size - i) >= block_size_)
        {
            curr_len = block_size_;
        }
        else
        {
            curr_len = size - i;
        }

        int compressed_len = _compress_block(input, output);
        output += compressed_len;
        input += curr_len;

        total_comp_len += compressed_len;
    
    }
    return total_comp_len;
}

int pfordelta_compressor::_compress_block(unsigned int* input, unsigned int* output)
{
    int flag = -1;
    unsigned int* w;
    for (int k = 0; flag < 0; k++)
    {
        w = output + 1;
        flag = pfor_encode(&w, input, k);
    }
    *output = flag;
    return w - output;
}

int pfordelta_compressor::pfor_encode(unsigned int** w, unsigned int* p, int num)
{
    int i, l, n, bb, t, s;
    unsigned int m;
    int b = cnum_[num + 2];
    int start;

    unsigned int out[block_size_];
    unsigned int ex[block_size_];

    if (b == 32)
    {
        for (i = 0; i < block_size_; i++)
        {
            (*w)[i] = p[i];
        }
        *w += block_size_;
        return ((num << 12) + (2 << 10) + block_size_);
    }

    // Find the largest number we're encoding.
    for (m = 0, i = 0; i < block_size_; i++)
    {
        if (p[i] > m)
            m = p[i];
    }

    if (m < 256)
    {
        bb = 8;
        t = 0;
    }
    else if (m < 65536)
    {
        bb = 16;
        t = 1;
    }
    else
    {
        bb = 32;
        t = 2;
    }

    for (start = 0, n = 0, l = -1, i = 0; i < block_size_; i++)
    {
        if ((p[i] >= static_cast<unsigned> (1 << b)) || ((l >= 0) && (i - l == (1 << b))))
        {
            if (l < 0)
                start = i;
            else
                out[l] = i - l - 1;

            ex[n++] = p[i];
            l = i;
        }
        else
        {
            out[i] = p[i];
        }
    }

    if (l >= 0)
        out[l] = (1 << b) - 1;
    else
        start = block_size_;

    if (static_cast<double> (n) <= FRAC * static_cast<double> (block_size_))
    {
        s = ((b * block_size_) >> 5);
        for (i = 0; i < s; i++)
        {
            (*w)[i] = 0;
        }
        pack(out, b, block_size_, *w);
        *w += s;

        s = ((bb * n) >> 5) + ((((bb * n) & 31) > 0) ? 1 : 0);
        for (i = 0; i < s; i++)
        {
            (*w)[i] = 0;
        }
        pack(ex, bb, n, *w);
        *w += s;
        return ((num << 12) + (t << 10) + start);
    }

    return -1;
}

int pfordelta_compressor::decompress(unsigned int* input, unsigned int* output, int size)
{
    int total_decom_len = 0;

    for(int i = 0; i < size; i += block_size_)
    {
        int curr_len = 0;
        if ((size - i) >= block_size_)
        {
            curr_len = block_size_;
        }
        else
        {
            curr_len = size - i;
        }

        int decomp_len = _decompress_block(input, output);
        input += decomp_len;
        output += curr_len;

        total_decom_len += decomp_len;
    
    }
    return total_decom_len;
}

int pfordelta_compressor::_decompress_block(unsigned int* input, unsigned int* output)
{
    unsigned int* tmp = input;
    int flag = *tmp;
    b = cnum_[((flag >> 12) & 15) + 2];
    unpack_count = ((flag >> 12) & 15) + 2;
    t = (flag >> 10) & 3;
    start = flag & 1023;

    tmp++;
    tmp = pfor_decode(output, tmp, flag);
    return tmp - input;
}

unsigned* pfordelta_compressor::pfor_decode(unsigned int* _p, unsigned int* _w, int flag)
{
    int i, s;
    unsigned int x;
    (unpack[unpack_count])(_p, _w, block_size_);
    _w += ((b * block_size_) >> 5);

    switch (t)
    {
    case 0:
        for (s = start, i = 0; s < block_size_; i++)
        {
            x = _p[s] + 1;
            _p[s] = (_w[i >> 2] >> (24 - ((i & 3) << 3))) & 255;
            s += x;
        }
        _w += (i >> 2);

        if ((i & 3) > 0)
            _w++;
        break;

    case 1:
        for (s = start, i = 0; s < block_size_; i++)
        {
            x = _p[s] + 1;
            _p[s] = (_w[i >> 1] >> (16 - ((i & 1) << 4))) & 65535;
            s += x;
        }
        _w += (i >> 1);
        if ((i & 1) > 0)
            _w++;
        break;

    case 2:
        for (s = start, i = 0; s < block_size_; i++)
        {
            x = _p[s] + 1;
            _p[s] = _w[i];
            s += x;
        }
        _w += i;
        break;
    }
    return _w;
}

}}}

