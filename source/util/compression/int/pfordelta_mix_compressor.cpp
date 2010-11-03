#include <util/compression/int/pfordelta_mix_compressor.h>

#include <vector>
#include <cassert>
#include <iostream>
#include <math.h>

using namespace std;

namespace izenelib{namespace util{namespace compression{

char pop_table[256] =  {
0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, 
};

#define lm1_idx_m(r, x) \
do  {\
   (x) = (x) | ((x) >> 1); \
   (x) = (x) | ((x) >> 2); \
   (x) = (x) | ((x) >> 4); \
   (x) = (x) | ((x) >> 8); \
   (x) = (x) | ((x) >>16); \
    \
   (x) = ~(x); \
   *(r) = pop_table[(x)         & 0xFF] + \
          pop_table[((x) >>  8) & 0xFF] + \
          pop_table[((x) >> 16) & 0xFF] + \
          pop_table[((x) >> 24)]; \
    \
    *(r) = 32 - *(r);  \
}while(0);


const static char VINT_MASK = 0xff; // using the first 6bit as mark

pfordelta_mix_compressor::pfordelta_mix_compressor()
{
    _division = static_cast<float>(0.9);
    _exps_comp_rate = static_cast<float>(0.3);
}

pfordelta_mix_compressor::~pfordelta_mix_compressor()
{
}

int pfordelta_mix_compressor::compress(uint32_t* const src, char* des, int length)
{
    char* ptr_des = des;
    uint32_t* ptr_src = src;

    int total_comp_len = 0;

    // compress one block each time
    for (int i = 0; i < length; i += PFORDELTA_BATCH_NUMBER)
    {

        int curr_len = 0;
        if ((length - i) >= PFORDELTA_BATCH_NUMBER)
        {
            curr_len = PFORDELTA_BATCH_NUMBER;
        }
        else
        {
            curr_len = length - i;
        }

        int compressed_len = this->encode_p(ptr_des, ptr_src, curr_len);
        ptr_des += compressed_len;
        ptr_src += curr_len;

        total_comp_len += compressed_len;
    }
    return total_comp_len;
}

int pfordelta_mix_compressor::decompress(char* const src, uint32_t* des, int length)
{
    uint32_t* ptr_des = des;
    char* ptr_src = src;
    char* ptr_src_end = ptr_src + length;

    // make sure the data is complete
    assert(length > 0);

    int ptr_src_length = 0;
    while (ptr_src < ptr_src_end)
    {
        int decomp_len = decode_p(ptr_des, ptr_src, ptr_src_length);
        ptr_des += decomp_len;
        ptr_src += ptr_src_length;
    }

    return static_cast<int>(ptr_des - des);
}

int pfordelta_mix_compressor::compress(unsigned int* input, unsigned int* output, int size)
{
    int total_comp_len = 0;

    for(int i = 0; i < size; i += PFORDELTA_BATCH_NUMBER)
    {
        int curr_len = 0;
        if ((size - i) >= PFORDELTA_BATCH_NUMBER)
        {
            curr_len = PFORDELTA_BATCH_NUMBER;
        }
        else
        {
            curr_len = size - i;
        }

        int compressed_len = encode_block(output, input, curr_len);
        output += compressed_len;
        input += curr_len;

        total_comp_len += compressed_len;
    
    }
    return total_comp_len;
}

int pfordelta_mix_compressor::decompress(unsigned int* input, unsigned int* output, int size)
{
    int total_decom_len = 0;

    for(int i = 0; i < size; i += PFORDELTA_BATCH_NUMBER)
    {
        int curr_len = 0;
        if ((size - i) >= PFORDELTA_BATCH_NUMBER)
        {
            curr_len = PFORDELTA_BATCH_NUMBER;
        }
        else
        {
            curr_len = size - i;
        }

        int decomp_len = decode_block(output, input);
        input += decomp_len;
        output += curr_len;
        total_decom_len += decomp_len;
    
    }
    return total_decom_len;	
}


//  VINT Module
//
//  Header 2 byte (16 bit)
//
//  |      8 bit       |       8 bit       |
//      magic number       length in byte
//
//         0xff
//
//  use magic number to avoid conflict with PForDelta
//

template<typename T>
int
pfordelta_mix_compressor::encode_p(char* des, T* const src, int length)
{
    assert(length > 0);

    int result = 0;

    if (length > PFORDELTAMIX_THRESHOLD)
    {
        // use normal pfordelta to compress
        result = pfor_encode_p(des, src, length);
    }
    else
    {
        // use vint to compress
        char* ptr_control = des;
        *ptr_control = VINT_MASK;
        ++ptr_control;

        char* ptr_des = des;
        ptr_des += 2;

        *ptr_control = vint_encode_p(ptr_des, src, length);

        result = *ptr_control + 2;
    }

    return result;
}

template<typename T>
int
pfordelta_mix_compressor::decode_p(T* des, char* const src, int& srcLen)
{
    int result = 0;

    if (*src != VINT_MASK)
    {
        result = pfor_decode_p(des, src, srcLen);
    }
    else
    {
        char* ptr_src = src;
        ++ptr_src;

        int length = *ptr_src;
        ++ptr_src;

        assert(length > 0);

        // start the data offset
        result = vint_decode_p(des, ptr_src, length);
        srcLen = length + 2;
    }

    return result;
}


template<typename T>
int
pfordelta_mix_compressor::vint_encode_p(char* des, T* const src, int length)
{
    char* ptr_des = des;
    T* ptr_src = src;

    unsigned int comp_len = 0;
    for (int i=0; i<length; ++i)
    {
        compressFunc<T>((*ptr_src), ptr_des, comp_len);
        ++ptr_src;
        ptr_des += comp_len;
    }
    return ptr_des - des;
}

template<typename T>
inline int
pfordelta_mix_compressor::vint_decode_p(T* des, char* const src, int length)
{
    char* ptr_src = src;
    T* ptr_des = des;
    while ((ptr_src - src) < length)
    {
        *ptr_des = decompressFunc<T>(ptr_src);
        ++ptr_des;
    }

    return static_cast<int>(ptr_des - des);
}

template<typename T>
unsigned int
pfordelta_mix_compressor::compressFunc(T value, char * buf, unsigned int & len)
{
    // special value, special treatment
    if (value == 0)
    {
        *buf = 0;
        len  = 1;
        return 1;
    }
    int bits = sizeof(value) * 8;
    bits = bits - bits % 7;
    // shrink and removing leading zero(s)
    while ((value >> bits) == 0)
    {
        bits -= 7;
    }

    len = 0;
    while (bits > 0)
    {
        // semi-byte(s) moved into buffer
        buf[len++] = (char)(0x80 | ((value >> bits) & 0x7F));
        bits -= 7;
    }

    // terminating semi-byte
    buf[len++] = char(value & 0x7F);
    return len;
}

template<typename T>
inline T
pfordelta_mix_compressor::decompressFunc(char*&  buf)
{
    register T result = 0;
    while (*buf & 0x80)
    {
        // sign bit indicate that this byte is not the endding one
        result = (result << 7) | ((*buf) & 0x7F);
        buf ++;
        // left shift 7 bits, and append the right 7 bits to the result
    }
    // terminating byte, last 7 bits appended.
    result = (result << 7) | (*buf);
    buf ++;
    return result;
}

int pfordelta_mix_compressor::vint_encode_p(uint32_t* const input, uint32_t* output, int size)
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

int pfordelta_mix_compressor::vint_decode_p(uint32_t* const input, uint32_t* output, int size)
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


int pfordelta_mix_compressor::encode_block(uint32_t* des, uint32_t* const src, int length)
{
    uint32_t* ptr_des = des;
    uint32_t* ptr_src = src;

    int num_bit_array[32] = {0};

    // 8bit                         |    6bit         |                  2bit              | 8bit                                 | 8bit
    // length for data block |  data's bit   |interval for exceptions     |length for exceptional data | index for first exceptional data
    // and word                 |                   |                                    |compressed using vint        |
    //
    uint32_t control_num = 0;

    // find the bit range of all the data
    int highestBitIdx = 0;
    for (int i = 0; i < length; ++i)
    {
        //int tempHighBitIdx = this->highBitIdx(*ptr_src);
        //int tempHighBitIdx = highBitIdx(*ptr_src);

        int tempHighBitIdx;
        uint32_t x = *ptr_src;
        lm1_idx_m(&tempHighBitIdx, x);

        ++num_bit_array[tempHighBitIdx];
        if (tempHighBitIdx > highestBitIdx) highestBitIdx = tempHighBitIdx;
        ++ptr_src;
    }

    ptr_src = src;

    // look for the best bit_of_num
    int temp_exps_num = length - num_bit_array[0];
    int bit_of_num = 0;
    int least_comp_len = length;
    for (int i= 1; i <= highestBitIdx; ++i)
    {
        if (num_bit_array[i] == 0) continue;
        temp_exps_num -= num_bit_array[i];
        if (temp_exps_num > MAX_EXPS_NUM)
            continue;
        int temp_num_per_word = 32 / i;
        int temp_comp_len = (length + temp_num_per_word - 1)/ temp_num_per_word
                            + (int)ceil(temp_exps_num * _exps_comp_rate);
        if (temp_comp_len <= least_comp_len)
        {
            least_comp_len = temp_comp_len;
            bit_of_num = i;
        }
    }


    // look for the max gap for exceptional data, the first index for exceptional data, and number for exceptional data
    int max_gap = 0;
    uint32_t max_num = (1 << bit_of_num) - 1;
    int last_exception = -1;
    int first_exception = 128;
    int num_of_exception = 0;
    for (int i = 0; i < length; ++i)
    {
        if (*ptr_src > max_num)
        {
            if (last_exception != -1)
            {
                if (i - last_exception > max_gap)
                {
                    // find max gap
                    max_gap = i - last_exception;
                }
                last_exception = i;
            }
            else
            {
                last_exception = i;
                first_exception = i;
            }

            ++num_of_exception;
        }
        ++ptr_src;
    }

    int exps_handle_mode = EXPS_HANDLE_MODE_DEFAULT;
    // if exceptional gap is too long, chooses suitable processing way
    if (bit_of_num < highBitIdx(max_gap))
    {
        if (getCostofAddBit(bit_of_num) <= (num_of_exception + 3) / 4)
        {

            //method 1: adding one mark bit, to indicate it is an exception when it's valued as "1"
            ++bit_of_num;
            exps_handle_mode = EXPS_HANDLE_MODE_FIRSTBIT;
        }
        else
        {
            //method 2: putting index for all exceptional data after data block.
            exps_handle_mode = EXPS_HANDLE_MODE_AFTERDATA;
        }
    }

    assert(bit_of_num != 0);

    // make sure the length
    int num_per_word = 32 / bit_of_num;
    int size_of_data_block = (length + num_per_word - 1) / num_per_word;

    uint32_t* ptr_control = ptr_des;

    ++ptr_des;

    // writing data(different with approach in the paper)
    //   1 get exceptional gap sequence
    //   2 writing data
    uint32_t idx_vec[num_of_exception];
    int index = 0;
    int last_exception_write = -1;

    ptr_src = src;

    if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
    {
        for (int i = 0; i < length; ++i)
        {
            if (*ptr_src > max_num)
            {
                if (last_exception_write != -1)
                {
                    idx_vec[index++] = (i - last_exception_write);
                }
                last_exception_write = i;
            }
            ++ptr_src;
        }
        // set the last as mark
        idx_vec[index] = 0;
    }

    // warning: fix bug here! not initialized
    ptr_src = src;

    // 2
    index = 0;
    uint32_t* ptr_exception = reinterpret_cast<uint32_t*>(_buffer);
    uint32_t* ptr_exception_index = ptr_exception;
    // for recompress
    uint32_t* ptr_exception_begin = ptr_exception;
    if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
    {
        ptr_exception += num_of_exception;
    }

    uint32_t* ptr_src_end = ptr_src + length;
    for (int i = 0; i < size_of_data_block; ++i)
    {
        uint32_t current_num = 0;
        for (int j = 0; j < num_per_word; ++j)
        {
            if (ptr_src == ptr_src_end)
            {
                break;
            }

            // from low to high
            if (*ptr_src <= max_num)
            {
                current_num |= *ptr_src << (bit_of_num * j);
            }
            else
            {
                if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
                {
                    current_num |= idx_vec[index++] << (bit_of_num * j);
                    // the number of exceptions
                    *ptr_exception = *ptr_src;
                    ++ptr_exception;
                }
                else if (exps_handle_mode == EXPS_HANDLE_MODE_FIRSTBIT)
                {
                    int mask;
                    getMaskNumber(mask, bit_of_num - 1);
                    int lower_bits_data = *ptr_src & mask;
                    lower_bits_data |= (1 << (bit_of_num - 1));
                    current_num |= lower_bits_data << (bit_of_num * j);
                    int higher_bits_data = *ptr_src >> (bit_of_num-1);
                    *ptr_exception = higher_bits_data;
                    ++ptr_exception;
                }
                else if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
                {
                    int mask;
                    getMaskNumber(mask, bit_of_num);
                    int lower_bits_data = *ptr_src & mask;
                    current_num |= lower_bits_data << (bit_of_num * j);
                    int higher_bits_data = *ptr_src >> (bit_of_num);
                    *ptr_exception = higher_bits_data;
                    ++ptr_exception;
                    *ptr_exception_index = i * num_per_word + j;
                    ++ptr_exception_index;
                }
            }
            ++ptr_src;
        }
        *ptr_des = current_num;
        ++ptr_des;
    }
    int comp_size_of_exception = 0;
    if (num_of_exception)
    {
        if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
        {
            num_of_exception *= 2;
        }
        comp_size_of_exception = vint_encode_p(ptr_exception_begin, ptr_des, num_of_exception);
        _exps_comp_rate = static_cast<float>(comp_size_of_exception* sizeof(uint32_t))
                          / (num_of_exception * sizeof(uint32_t));
    }
    // control number
    control_num = size_of_data_block << 24;
    control_num |= ((bit_of_num << 2) | exps_handle_mode) << 16;
    control_num |= num_of_exception << 8;
    control_num |= first_exception;
    *ptr_control = control_num;

    // XXX different with origin algorithm
    return (1 + size_of_data_block) + comp_size_of_exception;

}


template<typename T>
int
pfordelta_mix_compressor::pfor_encode_p(char* des, T* const src, int length)
{
    uint32_t* ptr_des = reinterpret_cast<uint32_t*>(des);
    T* ptr_src = src;

    int num_bit_array[32] = {0};

    // 8bit                         |    6bit         |                  2bit              | 8bit                                 | 8bit
    // length for data block |  data's bit   |interval for exceptions     |length for exceptional data | index for first exceptional data
    // and word                 |                   |                                    |compressed using vint        |
    //
    uint32_t control_num = 0;

    // find the bit range of all the data
    int highestBitIdx = 0;
    for (int i = 0; i < length; ++i)
    {
        //int tempHighBitIdx = this->highBitIdx(*ptr_src);
        //int tempHighBitIdx = highBitIdx(*ptr_src);

        int tempHighBitIdx;
        uint32_t x = *ptr_src;
        lm1_idx_m(&tempHighBitIdx, x);

        ++num_bit_array[tempHighBitIdx];
        if (tempHighBitIdx > highestBitIdx) highestBitIdx = tempHighBitIdx;
        ++ptr_src;
    }

    ptr_src = src;

    // look for the best bit_of_num
    int temp_exps_num = length - num_bit_array[0];
    int bit_of_num = 0;
    int least_comp_len = length;
    for (int i= 1; i <= highestBitIdx; ++i)
    {
        if (num_bit_array[i] == 0) continue;
        temp_exps_num -= num_bit_array[i];
        if (temp_exps_num > MAX_EXPS_NUM)
            continue;
        int temp_num_per_word = 32 / i;
        int temp_comp_len = (length + temp_num_per_word - 1)/ temp_num_per_word
                            + (int)ceil(temp_exps_num * _exps_comp_rate);
        if (temp_comp_len <= least_comp_len)
        {
            least_comp_len = temp_comp_len;
            bit_of_num = i;
        }
    }


    // look for the max gap for exceptional data, the first index for exceptional data, and number for exceptional data
    int max_gap = 0;
    T max_num = (1 << bit_of_num) - 1;
    int last_exception = -1;
    int first_exception = 128;
    int num_of_exception = 0;
    for (int i = 0; i < length; ++i)
    {
        if (*ptr_src > max_num)
        {
            if (last_exception != -1)
            {
                if (i - last_exception > max_gap)
                {
                    // find max gap
                    max_gap = i - last_exception;
                }
                last_exception = i;
            }
            else
            {
                last_exception = i;
                first_exception = i;
            }

            ++num_of_exception;
        }
        ++ptr_src;
    }

    int exps_handle_mode = EXPS_HANDLE_MODE_DEFAULT;
    // if exceptional gap is too long, chooses suitable processing way
    if (bit_of_num < highBitIdx(max_gap))
    {
        if (getCostofAddBit(bit_of_num) <= (num_of_exception + 3) / 4)
        {

            //method 1: adding one mark bit, to indicate it is an exception when it's valued as "1"
            ++bit_of_num;
            exps_handle_mode = EXPS_HANDLE_MODE_FIRSTBIT;
        }
        else
        {
            //method 2: putting index for all exceptional data after data block.
            exps_handle_mode = EXPS_HANDLE_MODE_AFTERDATA;
        }
    }

    assert(bit_of_num != 0);

    // make sure the length
    int num_per_word = 32 / bit_of_num;
    int size_of_data_block = (length + num_per_word - 1) / num_per_word;

    uint32_t* ptr_control = ptr_des;

    ++ptr_des;

    // writing data(different with approach in the paper)
    //   1 get exceptional gap sequence
    //   2 writing data
    uint32_t* idx_vec = new uint32_t[num_of_exception];
    int index = 0;
    int last_exception_write = -1;

    ptr_src = src;

    if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
    {
        for (int i = 0; i < length; ++i)
        {
            if (*ptr_src > max_num)
            {
                if (last_exception_write != -1)
                {
                    idx_vec[index++] = (i - last_exception_write);
                }
                last_exception_write = i;
            }
            ++ptr_src;
        }
        // set the last as mark
        idx_vec[index] = 0;
    }

    // warning: fix bug here! not initialized
    ptr_src = src;

    // 2
    index = 0;
    uint32_t* ptr_exception = reinterpret_cast<uint32_t*>(_buffer);
    uint32_t* ptr_exception_index = ptr_exception;
    // for recompress
    uint32_t* ptr_exception_begin = ptr_exception;
    if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
    {
        ptr_exception += num_of_exception;
    }

    T* ptr_src_end = ptr_src + length;
    for (int i = 0; i < size_of_data_block; ++i)
    {
        uint32_t current_num = 0;
        for (int j = 0; j < num_per_word; ++j)
        {
            if (ptr_src == ptr_src_end)
            {
                break;
            }

            // from low to high
            if (*ptr_src <= max_num)
            {
                current_num |= *ptr_src << (bit_of_num * j);
            }
            else
            {
                if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
                {
                    current_num |= idx_vec[index++] << (bit_of_num * j);
                    // the number of exceptions
                    *ptr_exception = *ptr_src;
                    ++ptr_exception;
                }
                else if (exps_handle_mode == EXPS_HANDLE_MODE_FIRSTBIT)
                {
                    int mask;
                    getMaskNumber(mask, bit_of_num - 1);
                    int lower_bits_data = *ptr_src & mask;
                    lower_bits_data |= (1 << (bit_of_num - 1));
                    current_num |= lower_bits_data << (bit_of_num * j);
                    int higher_bits_data = *ptr_src >> (bit_of_num-1);
                    *ptr_exception = higher_bits_data;
                    ++ptr_exception;
                }
                else if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
                {
                    int mask;
                    getMaskNumber(mask, bit_of_num);
                    int lower_bits_data = *ptr_src & mask;
                    current_num |= lower_bits_data << (bit_of_num * j);
                    int higher_bits_data = *ptr_src >> (bit_of_num);
                    *ptr_exception = higher_bits_data;
                    ++ptr_exception;
                    *ptr_exception_index = i * num_per_word + j;
                    ++ptr_exception_index;
                }
            }
            ++ptr_src;
        }
        *ptr_des = current_num;
        ++ptr_des;
    }
    delete[] idx_vec;
    int comp_size_of_exception = 0;
    if (num_of_exception)
    {
        if (exps_handle_mode == EXPS_HANDLE_MODE_AFTERDATA)
        {
            num_of_exception *= 2;
        }
        comp_size_of_exception = vint_encode_p(reinterpret_cast<char*>(ptr_des),
                                               ptr_exception_begin, num_of_exception);
        _exps_comp_rate = static_cast<float>(comp_size_of_exception)
                          / (num_of_exception * sizeof(uint32_t));
    }
    // control number
    control_num = size_of_data_block << 24;
    control_num |= ((bit_of_num << 2) | exps_handle_mode) << 16;
    control_num |= comp_size_of_exception << 8;
    control_num |= first_exception;

    *ptr_control = control_num;

    // XXX different with origin algorithm
    return (1 + size_of_data_block) * sizeof(uint32_t) + comp_size_of_exception;
}

int pfordelta_mix_compressor::decode_block(uint32_t* des, uint32_t* const src)
{
    uint32_t* ptr_des = des;
    uint32_t* ptr_des_copy = des;
    uint32_t* ptr_src = src;

    // control number
    uint32_t control_num = *ptr_src;
    ++ptr_src;

    int mask;
    getMaskNumber(mask, 8);

    int first_exception = control_num & mask;
    control_num = control_num >> 8;
    int num_of_exception = control_num & mask;
    control_num = control_num >> 8;
    int bit_of_num = control_num & mask;
    int exps_handle_mode = bit_of_num & 0x3;
    bit_of_num >>= 2;
    control_num = control_num >> 8;
    int size_of_data_block = control_num & mask;

    // get length of current block
    //
    uint32_t* ptr_exception = NULL; // ptr_src + size_of_data_block;

    // handle the exception
    int comp_size_of_exception = 0;
    if (num_of_exception != 0)
    {
        ptr_exception = reinterpret_cast<uint32_t*>(_buffer);
        comp_size_of_exception = vint_decode_p(ptr_src + size_of_data_block, 
                                         ptr_exception,
                                         num_of_exception);
    }

    int num_per_word = 32 / bit_of_num;

    int mask_number;
    getMaskNumber(mask_number, bit_of_num);

    uint32_t* ptr_src_end = ptr_src + size_of_data_block;
    if (exps_handle_mode == EXPS_HANDLE_MODE_FIRSTBIT)
    {
        int valid_bit_of_num = bit_of_num - 1;
        int valid_mask_number;
        getMaskNumber(valid_mask_number, valid_bit_of_num);
        while (ptr_src < ptr_src_end)
        {
            unsigned current_word = *ptr_src;
            uint32_t* ptr_des_end = ptr_des + num_per_word;
            while (ptr_des < ptr_des_end)
            {
                int value = current_word & mask_number;
                if (value >> valid_bit_of_num)
                {
                    value &= valid_mask_number;
                    value += (*(ptr_exception++) << valid_bit_of_num);
                }
                *(ptr_des++) = value;
                current_word = current_word >> bit_of_num;
            }
            ++ptr_src;
        };
    }
    else
    {
        while (ptr_src < ptr_src_end)
        {
            unsigned current_word = *ptr_src;
            uint32_t* ptr_des_end = ptr_des + num_per_word;
            while (ptr_des < ptr_des_end)
            {
                *(ptr_des++) = current_word & mask_number;
                current_word = current_word >> bit_of_num;
            }
            ++ptr_src;
        }

        //handle exception
        if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
        {
            int cur_excep_pos = first_exception;
            for (int i = 0; i < num_of_exception; ++i)
            {
                int next_pos = ptr_des_copy[cur_excep_pos];
                ptr_des_copy[cur_excep_pos] = *ptr_exception;
                cur_excep_pos += next_pos;
                ++ptr_exception;
            }
        }
        else
        {
            num_of_exception /= 2;
            uint32_t* ptr_exception_index = ptr_exception;
            ptr_exception += num_of_exception;
            for (int i = 0; i < num_of_exception; ++i)
            {
                ptr_des_copy[*(ptr_exception_index++)] +=
                    (*(ptr_exception++) << bit_of_num);
            }
        }
    }

    // remove the last 0
    while (ptr_des > des)
    {
        if (*(ptr_des - 1) > 0)
        {
            break;
        }
        --ptr_des;
    }
	
    //return (ptr_des - des);
    return (1 + size_of_data_block) + comp_size_of_exception;
}

template<typename T>
inline int
pfordelta_mix_compressor::pfor_decode_p(T* des, char* const src, int& srcLen)
{
    T* ptr_des = des;
    T* ptr_des_copy = des;
    uint32_t* ptr_src = reinterpret_cast<uint32_t*>(src);

    // control number
    uint32_t control_num = *ptr_src;
    ++ptr_src;

    int mask;
    getMaskNumber(mask, 8);

    int first_exception = control_num & mask;
    control_num = control_num >> 8;
    int num_of_exception = control_num & mask;
    control_num = control_num >> 8;
    int bit_of_num = control_num & mask;
    int exps_handle_mode = bit_of_num & 0x3;
    bit_of_num >>= 2;
    control_num = control_num >> 8;
    int size_of_data_block = control_num & mask;

    // get length of current block
    //
    srcLen = (1 + size_of_data_block) * sizeof(uint32_t) + num_of_exception;

    uint32_t* ptr_exception = NULL; // ptr_src + size_of_data_block;

    // handle the exception
    if (num_of_exception != 0)
    {
        ptr_exception = reinterpret_cast<uint32_t*>(_buffer);
        num_of_exception = vint_decode_p(ptr_exception,
                                         reinterpret_cast<char*>(ptr_src + size_of_data_block),
                                         num_of_exception);
    }

    int num_per_word = 32 / bit_of_num;

    int mask_number;
    getMaskNumber(mask_number, bit_of_num);

    uint32_t* ptr_src_end = ptr_src + size_of_data_block;
    if (exps_handle_mode == EXPS_HANDLE_MODE_FIRSTBIT)
    {
        int valid_bit_of_num = bit_of_num - 1;
        int valid_mask_number;
        getMaskNumber(valid_mask_number, valid_bit_of_num);
        while (ptr_src < ptr_src_end)
        {
            unsigned current_word = *ptr_src;
            T* ptr_des_end = ptr_des + num_per_word;
            while (ptr_des < ptr_des_end)
            {
                int value = current_word & mask_number;
                if (value >> valid_bit_of_num)
                {
                    value &= valid_mask_number;
                    value += (*(ptr_exception++) << valid_bit_of_num);
                }
                *(ptr_des++) = value;
                current_word = current_word >> bit_of_num;
            }
            ++ptr_src;
        };
    }
    else
    {
        while (ptr_src < ptr_src_end)
        {
            unsigned current_word = *ptr_src;
            T* ptr_des_end = ptr_des + num_per_word;
            while (ptr_des < ptr_des_end)
            {
                *(ptr_des++) = current_word & mask_number;
                current_word = current_word >> bit_of_num;
            }
            ++ptr_src;
        }

        //handle exception
        if (exps_handle_mode == EXPS_HANDLE_MODE_DEFAULT)
        {
            int cur_excep_pos = first_exception;
            for (int i = 0; i < num_of_exception; ++i)
            {
                int next_pos = ptr_des_copy[cur_excep_pos];
                ptr_des_copy[cur_excep_pos] = *ptr_exception;
                cur_excep_pos += next_pos;
                ++ptr_exception;
            }
        }
        else
        {
            num_of_exception /= 2;
            uint32_t* ptr_exception_index = ptr_exception;
            ptr_exception += num_of_exception;
            for (int i = 0; i < num_of_exception; ++i)
            {
                ptr_des_copy[*(ptr_exception_index++)] +=
                    (*(ptr_exception++) << bit_of_num);
            }
        }
    }

    // remove the last 0
    while (ptr_des > des)
    {
        if (*(ptr_des - 1) > 0)
        {
            break;
        }
        --ptr_des;
    }
    return (ptr_des - des);
}

}}}
