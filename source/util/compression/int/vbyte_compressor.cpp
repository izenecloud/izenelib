#include <util/compression/int/vbyte_compressor.h>

using namespace std;

namespace izenelib{namespace util{namespace compression{

vbyte_compressor::vbyte_compressor()
{}

vbyte_compressor::~vbyte_compressor()
{}

int vbyte_compressor::compress(unsigned int* input, unsigned int* output, int size)
{
    unsigned char* curr_byte = reinterpret_cast<unsigned char*> (output);

    unsigned int bp = 0;  // Current byte pointer into the 'output' array which we use to set each word in the 'output' array to 0 before decoding to it.
    // This which prevents uninitialized data errors in Valgrind.
    unsigned int n;
    for (int i = 0; i < size; ++i)
    {
        n = input[i];
//        assert(n <= 0x10204080);  // Max integer we can encode using 5 bytes is ((128**4)+(128**3)+(128**2)+(128**1)).
        unsigned char _barray[5];
        for (int j = 0; j < 5; ++j)
        {
            _barray[j] = (n & 0x7F) << 1;
            n = n >> 7;
        }

        bool started = false;
        for (int k = 4; k > 0; --k)
        {
            if (_barray[k] != 0 || started == true)
            {
                started = true;
                if ((bp & 3) == 0)
                    output[bp >> 2] = 0;
                *curr_byte = _barray[k] | 0x1;
                ++curr_byte;
                ++bp;
            }
        }

        if ((bp & 3) == 0)
            output[bp >> 2] = 0;
        *curr_byte = _barray[0];
        ++curr_byte;
        ++bp;
    }

    return (bp >> 2) + ((bp & 3) != 0 ? 1 : 0);
}

int vbyte_compressor::decompress(unsigned int* input, unsigned int* output, int size)
{
    unsigned char* curr_byte = reinterpret_cast<unsigned char*> (input);
    unsigned int n;
    for (int i = 0; i < size; ++i)
    {
        n = ((*curr_byte >> 1));
        if ((*curr_byte & 0x1) != 0)
        {
            ++curr_byte;
            n = (n << 7) | (*curr_byte >> 1);
            if ((*curr_byte & 0x1) != 0)
            {
                ++curr_byte;
                n = (n << 7) | (*curr_byte >> 1);
                if ((*curr_byte & 0x1) != 0)
                {
                    ++curr_byte;
                    n = (n << 7) | (*curr_byte >> 1);
                }
            }
        }
        ++curr_byte;
        output[i] = n;
    }

    int num_bytes_consumed = (curr_byte - reinterpret_cast<unsigned char*> (input));
    return (num_bytes_consumed >> 2) + ((num_bytes_consumed & 3) != 0 ? 1 : 0);
}

}}}
