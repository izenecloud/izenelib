/**
 * @file	compressor.h
 * @brief	Header file of compressor type
 * @author	Yingfeng Zhang
 * @date    2010-09-02
 * @details
 * ==============
 */
#ifndef POSTING_COMPRESSOR_H
#define POSTING_COMPRESSOR_H

#include <util/compression/int/compressor.h>

using namespace izenelib::util::compression;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template<typename PrimaryCompressor, typename SecondaryCompressor, int COMPRESS_BLOCK_SIZE, int MIN_PADDING_SIZE>
class CombinedCompressorType
{
public:
    CombinedCompressorType()
    {
        primary_coder_ = new PrimaryCompressor;
        secondary_coder_ = new SecondaryCompressor;
    }

    ~CombinedCompressorType()
    {
        delete primary_coder_;
        delete secondary_coder_;
    }

    int block_size() const { return COMPRESS_BLOCK_SIZE; }

    int compress(uint32_t* input, uint32_t* output, int num_input_elements) const
    {
        int compressed_len = 0;
        int num_whole_blocks = num_input_elements / COMPRESS_BLOCK_SIZE;
        int encoded_offset = 0;
        int unencoded_offset = 0;
        while (num_whole_blocks-- > 0)
        {
            encoded_offset += primary_coder_->compress(input + unencoded_offset, output + encoded_offset, COMPRESS_BLOCK_SIZE);
            unencoded_offset += COMPRESS_BLOCK_SIZE;
        }

        int left_to_encode = num_input_elements % COMPRESS_BLOCK_SIZE;
        if (left_to_encode == 0)
        {
            // Nothing to do here.
        }
        else if (left_to_encode < MIN_PADDING_SIZE)
        {
            // Encode leftover portion with a non-blockwise coder.
            encoded_offset += secondary_coder_->compress(input + unencoded_offset, output + encoded_offset, left_to_encode);
        }
        else
        {
            // Encode leftover portion with a blockwise coder, and pad it to the blocksize.
            // Assumption here is that the 'input' array size is at least an upper multiple of 'COMPRESS_BLOCK_SIZE'.
            int pad_until = COMPRESS_BLOCK_SIZE * ((num_input_elements / COMPRESS_BLOCK_SIZE) + 1);
            for (int i = num_input_elements; i < pad_until; ++i)
            {
                input[i] = 0;
            }
            encoded_offset += primary_coder_->compress(input + unencoded_offset, output + encoded_offset, COMPRESS_BLOCK_SIZE);
        }

        compressed_len = encoded_offset;

        return compressed_len;
    }

    int decompress(uint32_t* input, uint32_t* output, int num_input_elements) const
    {
        int compressed_len = 0;
        int num_whole_blocks = num_input_elements / COMPRESS_BLOCK_SIZE;
        int encoded_offset = 0;
        int unencoded_offset = 0;
        while (num_whole_blocks-- > 0)
        {
            encoded_offset += primary_coder_->decompress(input + encoded_offset, output + unencoded_offset, COMPRESS_BLOCK_SIZE);
            unencoded_offset += COMPRESS_BLOCK_SIZE;
        }

        int left_to_encode = num_input_elements % COMPRESS_BLOCK_SIZE;
        if (left_to_encode == 0)
        {
            // Nothing to do here.
        }
        else if (left_to_encode < MIN_PADDING_SIZE)
        {
            // Decode leftover portion with a non-blockwise coder.
            encoded_offset += secondary_coder_->decompress(input + encoded_offset, output + unencoded_offset, left_to_encode);
        }
        else
        {
            // Decode leftover portion with a blockwise coder, since it was padded to the blocksize.
            // Assumption here is that the 'output' array size is at least an upper multiple of 'COMPRESS_BLOCK_SIZE'.
            encoded_offset += primary_coder_->decompress(input + encoded_offset, output + unencoded_offset, COMPRESS_BLOCK_SIZE);
        }

        compressed_len = encoded_offset;

        return compressed_len;
    }

private:
    PrimaryCompressor* primary_coder_;
    SecondaryCompressor* secondary_coder_;
};

template<typename PrimaryCompressor>
class CompressorType
{
public:
    CompressorType()
    {
        coder_ = new PrimaryCompressor;
    }

    ~CompressorType()
    {
        delete coder_;
    }
	    
    int compress(uint32_t* input, uint32_t* output, int num_input_elements) const
    {
        int compressed_len = 0;
        compressed_len = coder_->compress(input, output, num_input_elements);

        return compressed_len;
    }

    int decompress(uint32_t* input, uint32_t* output, int num_input_elements) const
    {
        int compressed_len = 0;
        compressed_len = coder_->decompress(input, output, num_input_elements);

        return compressed_len;
    }
private:
    PrimaryCompressor* coder_;
};

//typedef CombinedCompressorType<PForDelta_Compressor, S16_Compressor, 128, 96> DocIDCompressor;
typedef CompressorType<PForDeltaMix_Compressor> DocIDCompressor;
typedef CompressorType<S16_Compressor> TermFreqCompressor;
typedef CompressorType<S16_Compressor> TermPosCompressor;
typedef CombinedCompressorType<PForDelta_Compressor, S16_Compressor, 256, 192> BlockHeadCompressor;

}
NS_IZENELIB_IR_END

#endif


