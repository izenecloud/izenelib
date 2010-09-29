#ifndef IZENE_UTIL_COMPRESSION_INT_PFORDELTA_MIX_S16_COMPRESSOR_H
#define IZENE_UTIL_COMPRESSION_INT_PFORDELTA_MIX_S16_COMPRESSOR_H

#include <stdint.h>
#include <string.h>

#include <iostream>
#include "pfordelta_unpack.h"

namespace izenelib{namespace util{namespace compression{
/**
 * Implementation of the p4delta algorithm for sorted integer arrays based on
 * 1. Original Algorithm from http://homepages.cwi.nl/~heman/downloads/msthesis.pdf
 * 2. Optimization and variation from http://www2008.org/papers/pdf/p387-zhangA.pdf
 * 3. Further optimization http://www2009.org/proceedings/pdf/p401.pdf
 * @author Hao Yan
 *
 */
using namespace std;

class  pfordelta_mix_s16_compressor
{
    // Max number of  bits to store an uncompressed value
    static const int MAX_BITS = 32;
    // Header number. The header of each block contains 2 integers, the first one stores the parameters to decompress the block;

    // the second integer stores the last (uncompressed) docId of the block to speed up query processing by the skipping techniques.

    static const int HEADER_NUM = 2;
    // Header size

    static const  int HEADER_SIZE = MAX_BITS * HEADER_NUM;
    // Default block size, must be X * 32

    static const int MAX_EXPECTED_BLOCKSIZE = 1024;
    //The number of bits to store exception positions if fixedBitEncoding is chosen

    static const int POSBITS = 8;
    // Auxiliary spaces to store temporary compressed/decompressed results
    static const int DEFAULT_BATCH_SIZE = 128; // default block size

    static const int MASK[33];
	
    static const int S16_NUMSIZE = 16;

    static const int S16_BITSSIZE = 28;

    static const int S16_NUM[16];
	
    static const uint32_t S16_BITS[16][28];
	
    static const int POSSIBLE_B[16];// all possible values of b in PForDelta algorithm

    uint32_t _expPos[MAX_EXPECTED_BLOCKSIZE];

    uint32_t _expHighBits[MAX_EXPECTED_BLOCKSIZE];

    uint32_t _expAux[MAX_EXPECTED_BLOCKSIZE * 2];

    uint32_t  _EstCompBlock[MAX_EXPECTED_BLOCKSIZE * 2];

    bool _flagFixedBitExpPos; // indicating if the expPos (positions of excpetions) is encoded in fixed number of bits

    int _blockSize;

public:
    pfordelta_mix_s16_compressor()
            :_flagFixedBitExpPos(false)
            ,_blockSize(DEFAULT_BATCH_SIZE)
    {}

    int compress(unsigned int* input, unsigned int* output, int size)
    {
        int total_comp_len = 0;

        for (int i = 0; i < size; i += _blockSize)
        {
            int curr_len = 0;
            if ((size - i) >= _blockSize)
            {
                curr_len = _blockSize;
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

    int decompress(unsigned int* input, unsigned int* output, int size)
    {
        int total_decom_len = 0;

        for (int i = 0; i < size; i += _blockSize)
        {
            int curr_len = 0;
            if ((size - i) >= _blockSize)
            {
                curr_len = _blockSize;
            }
            else
            {
                curr_len = size - i;
            }

            int decomp_len = decompressOneBlock(output, input, curr_len);
            input += decomp_len;
            output += curr_len;
            total_decom_len += decomp_len;
        }
        return total_decom_len;
    }

    int encode_block(uint32_t* des, uint32_t* const src, int blockSize)
    {
        int currentB = POSSIBLE_B[0];   
        int tmpB = currentB;
        _flagFixedBitExpPos = false;
        int compressedBitSize = estimateCompSize(src, blockSize, tmpB, _flagFixedBitExpPos);

        for (int i = 1; i < 16; ++i)
        {
            tmpB = POSSIBLE_B[i];
            bool currflagFixedBitExpPos = false;
            int curCompressedBitSize = estimateCompSize(src, blockSize, tmpB, currflagFixedBitExpPos);
            if(curCompressedBitSize < compressedBitSize)
            {
                currentB = tmpB;
                _flagFixedBitExpPos = currflagFixedBitExpPos;
                compressedBitSize = curCompressedBitSize;
            }
        }
        // return the compressed data achieved from the best b
        return compressOneBlock(src, des, currentB, blockSize);
    }

    /**
     * Estimate the compressed size of a block
     *
     * @param inputBlock a block of non-negative integers to be compressed
     * @return compressed bit size
     */
    int estimateCompSize(uint32_t* inputBlock, int blockSize, int bits, bool& flagFixedBitExpPos)
    {
        // The header and the bits-bit slots have the deterministic size. However, the compressed size for expPos and expHighBits are estimated.
        uint32_t maxNoExp = (1<<bits)-1;

        int outputOffset = HEADER_SIZE + bits * blockSize; //  Size of the header and the bits-bit slots

        int expNum = 0;
        // get expPos and expHighBits

        for (int i = 0; i < blockSize; ++i)
        {
            if (inputBlock[i] > maxNoExp)
            {
                _expPos[expNum] = i; //  the position of the exception
                _expHighBits[expNum] = (inputBlock[i] >> bits) & MASK[32-bits];   //  the higher 32-bits bits of the exception
                expNum++;
            }
        }

        if (expNum>0)
        {
            memcpy(_expAux, _expPos, expNum*sizeof(uint32_t));
            memcpy(_expAux + expNum, _expHighBits, expNum*sizeof(uint32_t));
            int compressedBitSize = compressBlockByS16(_EstCompBlock, 0, _expAux, expNum*2); // 2 blocks of expPos and expHighBits
            int compressedHighBitSize = compressBlockByS16(_EstCompBlock, 0, _expHighBits, expNum);
            // choose the better one of using Simple16 or using FixedBitSize to compress expPos
            if (compressedBitSize > (POSBITS *expNum + compressedHighBitSize))
            {
                flagFixedBitExpPos = true;
                outputOffset += (POSBITS*expNum + compressedHighBitSize);
            }
            else
            {
                outputOffset += compressedBitSize;
            }
        }
        return outputOffset;
    }

    /**
     * Compress an integer array
     *
     * @param inputBlock the integer input array
     * @param outputBlock the integer output array
     * @param bits the value of b in the PForDelta algorithm
     * @return the compressed size in number of words and the reference to the compressed data
     */
    int compressOneBlock(uint32_t* inputBlock, uint32_t* outputBlock, int bits, int blockSize)
    {
        // hy: compress a sequence of gaps except the first element (which is the original docId)
        uint32_t maxNoExp = 1<<bits;
        //int maxCompBitSize =  HEADER_SIZE + blockSize * (bits  + MAX_BITS + MAX_BITS) + 32;
        //uint32_t* compressedBlock = new uint32_t[(maxCompBitSize>>5)];

        // The second HEADER: the last (uncompressed) docId of the block
        outputBlock[1] = inputBlock[blockSize-1];
        int outputOffset = HEADER_SIZE;

        int expNum = 0;
        for (int i = 0; i<blockSize; ++i)
        {
            if (inputBlock[i] < maxNoExp)
            {
                writeBits(outputBlock, inputBlock[i], outputOffset, bits);
            }

            else // exp
            {
                writeBits(outputBlock, inputBlock[i] & MASK[bits], outputOffset, bits); // store the lower bits-bits of the exception
                _expPos[expNum] = i; // write the position of exception
                _expHighBits[expNum] = (inputBlock[i] >> bits) & MASK[32-bits];   // write the higher 32-bits bits of the exception
                expNum++;
            }

            outputOffset += bits;

        }
        // The first HEADER stores:  flagFixedBitExpPos | bits | expNum, assuming expNum < 2^10 and bits<2^10

        if (_flagFixedBitExpPos) // using fixed bits to encode expPos
        {
            outputBlock[0] = (1<<20) | ((bits & MASK[10]) << 10) | (expNum & 0x3ff);
        }
        else
        {
            outputBlock[0] = ((bits & MASK[10]) << 10) | (expNum & 0x3ff);
        }
        int compressedBitSize;
        if (expNum>0)
        {
            memcpy(_expAux, _expPos, expNum*sizeof(uint32_t));
            memcpy(_expAux + expNum, _expHighBits, expNum*sizeof(uint32_t));
            if (_flagFixedBitExpPos)
            {
                compressedBitSize = compressExpPosFixedBits(outputBlock, outputOffset, _expPos, expNum, POSBITS);
                outputOffset += compressedBitSize;
                compressedBitSize = compressBlockByS16(outputBlock, outputOffset, _expHighBits, expNum);
                outputOffset += compressedBitSize;
            }
            else
            {
                compressedBitSize = compressBlockByS16(outputBlock, outputOffset, _expAux, expNum * 2);
                outputOffset += compressedBitSize;
            }
        }
        return (outputOffset+31)>>5;
    }

    /**
     * Decompress a compressed block into an integer array
     *
     * @param outDecompBlock the decompressed output block
     * @param compBlock the compressed input block
     * @param blockSize
     * @return the processed data size (the number of words in the compressed form)
     */

    int decompressOneBlock(uint32_t* outDecompBlock, uint32_t* compBlock, int blockSize)
    {
        int expNum = compBlock[0] & 0x3ff;
        int bits = (compBlock[0]>>10) & (0x1f);

        bool flagFixedBitExpPos = ((compBlock[0]>>20) & (0x1)) > 0 ? true : false;
        // decompress the b-bit slots
        int offset = HEADER_SIZE;
        int compressedBits = 0;
        if (bits == 0)
        {
            memset(outDecompBlock,0,blockSize*sizeof(uint32_t));
        }
        else
        {
            compressedBits = decompressBBitSlotsWithHardCodes(outDecompBlock, compBlock, blockSize, bits);
        }

        offset += compressedBits;
        // decompress the expPos and expHighBits and assemble them with the above b-bit slots values into the  results
        if (expNum>0)
        {
            if (flagFixedBitExpPos)
            {
                compressedBits = decompressExpPosFixedBits(_expPos, compBlock, expNum, offset, 8);
                offset += compressedBits;
                compressedBits = decompressBlockByS16(_expHighBits, compBlock, expNum, offset);
                offset += compressedBits;
            }
            else
            {
                compressedBits = decompressBlockByS16(_expAux, compBlock, expNum*2, offset);
                offset += compressedBits;
            }
            int i=0;
            int curExpPos;
            int curHighBits;
            for (i = 0; i < expNum; i++)
            {
                if (flagFixedBitExpPos)
                {
                    curExpPos = _expPos[i]; // this makes sense since expNum is > 0
                    curHighBits = _expHighBits[i];
                }
                else
                {
                    curExpPos = _expAux[i]  ;
                    curHighBits = _expAux[i+expNum];
                }
                outDecompBlock[curExpPos] = (outDecompBlock[curExpPos] & MASK[bits]) | ((curHighBits & MASK[32-bits] ) << bits);
            }
        }
		
        return (offset+31)>>5;
    }
    /**
     * Decompress the b-bit slots
     *
     * @param decompressedSlots the decompressed output
     * @param compBlock the compressed input block
     * @param bits the value of b
     * @return the processed data size (the number of bits in the compressed form)
     */
    int decompressBBitSlots(uint32_t* decompressedSlots, uint32_t* compBlock, int blockSize, int bits)
    {
        int compressedBitSize = 0;
        int offset = HEADER_SIZE;
        for (int i =0; i<blockSize; i++)
        {
            decompressedSlots[i] = readBits(compBlock, offset, bits);
            offset += bits;
        }
        compressedBitSize = bits * blockSize;
        return compressedBitSize;
    }
    /**
     * Decompress the b-bit slots using hardcoded unpack methods
     *
     * @param decompressedSlots the decompressed output
     * @param compBlock the compressed input block
     * @param bits the value of b
     * @return the processed data size (the number of bits in the compressed form)
     */
    int decompressBBitSlotsWithHardCodes(uint32_t* decompressedSlots, uint32_t* compBlock, int blockSize, int bits)
    {
        int compressedBitSize = 0;
        PForDeltaUnpack::unpack(decompressedSlots, compBlock, bits);
        compressedBitSize = bits * blockSize;
        return compressedBitSize;
    }
    /**
     * Compress an integer array using Simple16 (used to compress positions and highBits of exceptions)
     *
    * @param outCompBlock the compressed output
     * @param offset the bit offset in the compressed input block
     * @param inBlock the compressed input block
     * @param expsize the number of exceptions
     * @return the compressed data size (the number of bits in the compressed form)
     */

    int compressBlockByS16(uint32_t* outCompBlock, int offset, uint32_t* inBlock, int expSize)
    {
        int outOffset  = (offset+31)>>5;
        int num, inOffset=0, numLeft;
        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Compress(outCompBlock, outOffset, inBlock, inOffset, numLeft);
            outOffset++;
            inOffset += num;
        }
        int compressedBitSize = (outOffset<<5)-offset;
        return compressedBitSize;
    }
    /**
     * Decompress an integer array using Simple16 (used to decompress positions and highBits of exceptions)
     *
     * @param outDecompBlock the decompressed output
     * @param inCompBlock the compressed input block
     * @param expsize the number of exceptions
     * @param offset the bit offset in the compressed input block
     * @return the processed data size (the number of bits in the compressed form)
     */
    int decompressBlockByS16(uint32_t* outDecompBlock, uint32_t* inCompBlock, int expSize, int offset)
    {
        int inOffset  = (offset+31)>>5;
        int num, outOffset=0, numLeft;
        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Decompress(outDecompBlock, outOffset, inCompBlock, inOffset, numLeft);
            outOffset += num;
            inOffset++;
        }

        int compressedBitSize = (inOffset<<5)-offset;
        return compressedBitSize;
    }
    /**
     * Compress the exception positions using fixed number of bits
     *
     * @param compBlock the compressed output
     * @param offset the bit offset in the compressed output block
     * @param expPos the input which is an integer array of exception positions
     * @param expsize the number of exceptions
     * @param bits the value of b in the PForDelta algorithm
    * @return the compressed data size (the number of bits in the compressed form)
     */

    int compressExpPosFixedBits(uint32_t* compBlock, int offset, uint32_t* expPos, int expSize, int bits)
    {
        int outputOffset = offset;
        for (int i=0; i<expSize; i++)
        {
            writeBits(compBlock, expPos[i] & MASK[bits], outputOffset, bits); // copy lower bits-bits
            outputOffset += bits;
        }
        int compressedBitSize = outputOffset - offset;
        return compressedBitSize;
    }
    /**
     * Decompress the exception positions using fixed number of bits
     *
     * @param expPos the output which is an integer array of exception positions
     * @param compBlock the compressed input
     * @param expsize the number of exceptions
     * @param offset the bit offset in the compressed input block
     * @param bits the value of b in the PForDelta algorithm
     * @return the processed data size (the number of bits in the compressed form)
     */
    int decompressExpPosFixedBits(uint32_t* expPos, uint32_t* compBlock, int expSize, int offset, int bits)
    {

        int outputOffset = offset;
        for (int i=0; i<expSize; i++)
        {
            expPos[i] = readBits(compBlock, outputOffset, bits);
            outputOffset += bits;
        }
        int compressedBitSize = outputOffset - offset;
        return compressedBitSize;
    }
    /**
     * Write certain number of bits of an integer into an integer array starting from the given offset
     *
     * @param out the output array
     * @param val the integer to be written
     * @param outOffset the offset in the number of bits in the output array, where the integer will be written
     * @param bits the number of bits to be written
     */
    static void writeBits(uint32_t* out, uint32_t val, int outOffset, int bits)
    {
        if (bits == 0)
            return;

        int index = outOffset >> 5;
        int skip = outOffset & 0x1f;
        val &= (0xffffffff >> (32 - bits));
        out[index] |= (val << skip);
        if (32 - skip < bits)
        {
            out[index + 1] |= (val >> (32 - skip));
        }
    }
    /**
     * Read certain number of bits of an integer into an integer array starting from the given offset
     *
     * @param in the input array
     * @param val the integer to be written
     * @param inOffset the offset in the number of bits in the input array, where the integer will be read
     * @param bits the number of bits to be read, unlike writeBits(), readBits() does not deal with bits==0 and thus bits must > 0. When bits ==0, the calling functions will just skip the entire bits-bit slots without decoding them
    */
    static uint32_t readBits(uint32_t* in,  int inOffset, int bits)
    {
        int index = inOffset >> 5;
        int skip = inOffset & 0x1f;
        uint32_t val = in[index] >> skip;
        if (32 - skip < bits)
        {
            val |= in[index + 1] << (32 - skip);
        }
        return val & (0xffffffff >> (32 - bits));
    }

    static  int readBitsForS16(uint32_t* in,  int inIntOffset, int inWithIntOffset, int bits)
    {
        int val = (int)(in[inIntOffset] >> inWithIntOffset);
        return val & (0xffffffff >> (32 - bits));
    }
    /**
     * Codes for encoding/decoding of the Simple16 compression algorithm
     *
     */
    /**
     * Compress an integer array using Simple16
     *
     * @param out the compressed output
     * @param outOffset the offset of the output in the number of integers
     * @param in the integer input array

    * @param inOffset the offset of the input in the number of integers
     * @param n the number of elements to be compressed
     * @return the number of compressed integers
     */
    static int s16Compress(uint32_t* out, int outOffset, uint32_t* in, int inOffset, int n)
    {
        uint32_t j, num, bits;
        for (int numIdx = 0; numIdx < S16_NUMSIZE; numIdx++)
        {
            out[outOffset] = numIdx<<S16_BITSSIZE;
            num = (S16_NUM[numIdx] < n) ? S16_NUM[numIdx] : n;
            for (j = 0, bits = 0; (j < num) && in[inOffset+j] < static_cast<uint32_t>(1<<S16_BITS[numIdx][j]); )
            {
                out[outOffset] |= (in[inOffset+j]<<bits);
                bits += S16_BITS[numIdx][j];
                j++;
            }
            if (j == num)
            {
                return num;
            }
        }
        return -1;
    }
    /**
     * Decompress an integer array using Simple16
     *
     * @param out the decompressed output
     * @param outOffset the offset of the output in the number of integers
     * @param in the compressed input array
     * @param inOffset the offset of the input in the number of integers
     * @param n the number of elements to be compressed
     * @return the number of processed integers
     */
    static int s16Decompress(uint32_t* out, int outOffset, uint32_t* in, int inOffset, int n)
    {
        int numIdx, j=0, bits=0;
        numIdx = in[inOffset]>>S16_BITSSIZE;

        int num = S16_NUM[numIdx] < n ? S16_NUM[numIdx] : n;
        for (j=0, bits=0; j<num; j++)
        {
            out[outOffset+j]  = readBitsForS16(in, inOffset, bits,  S16_BITS[numIdx][j]);
            bits += S16_BITS[numIdx][j];
        }
        return num;
    }
    /**
     * Using integer to represent exception positions, no compression
     *
     */
    int compressExpPosBasic(uint32_t* compBlock, int offset, uint32_t* expPos, int expSize)
    {
        int startPos  = offset>>5;
        for (int i=0; i<expSize; i++)
        {
            compBlock[startPos+i] = expPos[i];
        }
        int compressedBitSize = expSize << 5;
        return compressedBitSize;
    }

    int decompressExpPosBasic(uint32_t* expPos, uint32_t* compBlock, int expSize, int offset)
    {
        int startPos = offset>>5;
        for (int i=0; i<expSize; i++)
        {
            expPos[i] = compBlock[startPos+i];
        }
        int compressedBitSize = expSize << 5;
        return compressedBitSize;
    }

    /**
     * Using integer to represent exception highBits, no compression
     *
     */
    int compressExpHighBitsBasic(uint32_t* compBlock, int offset, uint32_t* expHighBits, int expSize, int bits)
    {
        int compressedBitSize = 0;
        for (int i =0; i<expSize; i++)
        {
            writeBits(compBlock, expHighBits[i] & MASK[bits], offset, bits);
            offset += bits;
        }
        compressedBitSize = bits * expSize;
        return compressedBitSize;
    }

    int decompressExpHighBitsBasic(uint32_t* expHighBits, uint32_t* compBlock, int expSize, int offset, int bits)
    {
        int compressedBitSize = 0;
        for (int i =0; i<expSize; i++)
        {
            expHighBits[i] = readBits(compBlock, offset, bits);
            offset +=  bits;
        }
        compressedBitSize = bits * expSize;
        return compressedBitSize;
    }

    int binaryEncode(uint32_t* out, int outOffset,  int val,  int lo, int hi)
    {
        int w=0, bits=0;
        w = hi-lo+1; /*the number of values*/
        /* w-1 is the maximum value , so this loop caculate how many bits are needed for the max value */
        for (w=w-1, bits=0; w>0; w>>=1) bits++;
        if (bits>0)
            writeBits(out, (val-lo) & MASK[bits], outOffset, bits);

        return bits;
    }

    int ipcRecur(uint32_t* buf, int outOffset, uint32_t* L, int inIdx, int f, int lo, int hi)
    {
        int cost=0;
        int h=0, m=0, f1=0, f2=0;
        int inIdx1, inIdx2;
        if (f == 0)
            return 0;

        if (f == 1)
        {
            return binaryEncode(buf, outOffset, L[0], lo, hi);
        }

        /* attention here, when f=2, h=1, L[h] then is the 2nd value of 2 values, L[0] is the first one, in such case,

          interpolative first encode 2nd value and then 1st value (instead of like in other cases, always first first-half and then second half */

        h = (f-1)>>1;
        m = L[inIdx+h];
        f1 = h;
        f2 = f - h - 1;
        inIdx1 = 0;
        inIdx2 = h+1;
        int bits = binaryEncode(buf, outOffset, m, lo+f1, hi-f2);
        outOffset += bits;
        cost += bits;
        cost += ipcRecur(buf, outOffset, L, inIdx1, f1,lo,m-1);
        cost += ipcRecur(buf, outOffset, L, inIdx2, f2,m+1,hi);
        return cost;
    }

    int ipcCompress(uint32_t* out, int outOffset, uint32_t* in, int n, int prevLast, int hi)
    {
        if (n<1) return 0;
        int lo = prevLast+1;
        int compressedBitsNum =  ipcRecur(out, outOffset, in, 0, n, lo, hi);
        return compressedBitsNum;
    }

// hy: return bitsNum

    int vbyteCompress(uint32_t* out, int outOffset, int val)
    {
        uint32_t barray[5];
        bool started = false;
        int newOutOffset = outOffset;
        int i;
        for (i = 0; i < 5; i++)
        {
            barray[i] = (val & MASK[7])<<1;
            val = val>>7; // hy: divided by 128
        }
        for (i = 4; i > 0; i--)
        {
            if ((barray[i] != 0) || (started == true))
            {
                started = true;
                writeBits(out, (barray[i]  | 0x1) & MASK[8], newOutOffset, 8);
                newOutOffset+=8;
            }
        }

        writeBits(out, (barray[0] | 0x0) & MASK[8], newOutOffset, 8);
        newOutOffset+=8;

        return newOutOffset-outOffset;
    }


// hy: return bitsNum
    int vbyteDecompress(uint32_t* out, int outIdx, uint32_t* in, int inOffset)
    {
        int newInOffset = inOffset;
        int nextByte = readBits(in, newInOffset, 8);
        int flag = nextByte & 0x1;
        int val = nextByte>>1;
        newInOffset+=8;
        while (flag != 0)
        {
            nextByte = readBits(in, newInOffset, 8);
            newInOffset+=8;
            val = (val<<7) + (nextByte>>1);
            flag = nextByte & 0x1;
        }
        out[outIdx] = val;
        return newInOffset - inOffset;
    }

    int compressExpPosByVbyte(uint32_t* compBlock, int outOffset, uint32_t* expPos, int expSize)
    {
        int newOutOffset = outOffset;
        int bitsNum;

        for (int i=0; i<expSize; i++)
        {
            bitsNum = vbyteCompress(compBlock, newOutOffset, expPos[i]);
            newOutOffset += bitsNum;
        }
        return newOutOffset-outOffset;
    }


    int decompressExpPosByVbyte(uint32_t* expPos, uint32_t* compBlock, int expSize, int inOffset)
    {
        // hy: currently no compression for pos
        int newInOffset = inOffset;
        int bitsNum;

        for (int i=0; i<expSize; i++)
        {
            bitsNum = vbyteDecompress(expPos, i, compBlock, newInOffset);
            newInOffset += bitsNum;
        }
        return newInOffset-inOffset;
    }

};

}}}

#endif

