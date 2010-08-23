#include <iostream>
#include "pfordelta_unpack.h"
/**

 * Implementation of the p4delta algorithm for sorted integer arrays based on


 * 1. Original Algorithm from

 * http://homepages.cwi.nl/~heman/downloads/msthesis.pdf 2. Optimization and

 * variation from http://www2008.org/papers/pdf/p387-zhangA.pdf 3. Further optimization

 * http://www2009.org/proceedings/pdf/p401.pdf

 *

 */
using namespace std;
int MASK[] =

	{0x00000000,
	 0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f,
	 0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	 0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff, 0x0001ffff, 0x0003ffff,
	 0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
	 0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff,
	 0x7fffffff, 0xffffffff
	};

int S16_NUMSIZE = 16;

int S16_BITSSIZE = 28;

int S16_NUM[] =

	{28, 21, 21, 21, 14, 9, 8, 7, 6, 6, 5, 5, 4, 3, 2, 1};

int S16_BITS[16][28] =

{ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,0,0,0,0,0,0,0},
	{2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{4,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{3,4,4,4,4,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{5,5,5,5,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{4,4,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{6,6,6,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{5,5,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{7,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{10,9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{14,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

class  pfordelta_mix_s16_compressor
{
    int INVALID;
    // Maximum bits that can be used = 32
    // 32 bits for retaining base value

    int BASE_MASK;
    // Header size

    int HEADER_MASK;

    // hy: Header number

    int HEADER_NUM;
    // Parameters for the compressed set

    int _b;

    int _batchSize;

    int _expNum;

    int _compressedBitSize;

    int MAX_BATCH_SIZE;

    int MAX_POSSIBLE_COMPRESSED_BITSIZE;

    int* _myExpPos;

    int* _myExpHighBits;

public:
    pfordelta_mix_s16_compressor()
    {

        INVALID = -1;
        BASE_MASK = 32;
        HEADER_MASK = BASE_MASK;
        HEADER_NUM = 2;
        _b = INVALID;
        _batchSize = INVALID;
        _expNum = INVALID;
        _compressedBitSize = 0;
        MAX_BATCH_SIZE = 128;
        MAX_POSSIBLE_COMPRESSED_BITSIZE =  HEADER_MASK *HEADER_NUM + MAX_BATCH_SIZE * _b  + MAX_BATCH_SIZE * BASE_MASK + MAX_BATCH_SIZE * BASE_MASK + 32;
        _myExpPos = new int[MAX_BATCH_SIZE];
        _myExpHighBits = new int[MAX_BATCH_SIZE];
		
    }

    ~pfordelta_mix_s16_compressor()
    {
        delete[] _myExpPos;
        delete[] _myExpHighBits;
    }
    //hy: set param for one block to be prepared for compression

    /**

     * Alternate implementation for compress

     *

     * @param input

     * @return comprssed set in long array form

     * @throws IllegalArgumentException

     */

    void compressOneBlock(int* inputBlock)
    {
        //if (_b == INVALID)
           // throw std::runtime_error("codec not initialized correctly ");

        int BATCH_MAX = 1<<_b;
        // hy: the following allocated memory should be enough for storing the compressed data, the last 32 bits is for the aligned word
        // header (_b|_expNum and base for skipping) + _batchSize b-bit slots + expPos + expHighBits + alignment
        //("_batchSize: " + _batchSize);

        int MAX_POSSIBLE_COMPRESSED_BITSIZE =  HEADER_MASK *HEADER_NUM + _batchSize * _b  + _batchSize * BASE_MASK + _batchSize * BASE_MASK + 32;

        int* compressedBlock = new int[(MAX_POSSIBLE_COMPRESSED_BITSIZE>>5)];
        int* expPos = new int[_batchSize];

        int* expHighBits = new int[_batchSize];
        // hy: the first int of the header records the value of _b and the number of exps, the second one records the base

        // hy: record compressedBlock[0] after compression

        //compressedBlock[0] = _b << 10 | _expNum;

        compressedBlock[1] = inputBlock[_batchSize-1];
        // Offset is the offset of the next location to place the value

        int offset = HEADER_MASK *  HEADER_NUM;

        int expIndex = 0;

        for (int i = 0; i < _batchSize; i++)
        {
            if (inputBlock[i] < BATCH_MAX) // not exp
            {
                writeBits(compressedBlock, inputBlock[i], offset, _b);
            }
            else // exp
            {
                //System.out.println("exp input[" + i + "]: " + input[i]);
                writeBits(compressedBlock, inputBlock[i] & MASK[_b], offset, _b); // hy: copy lower b-bits

                expPos[expIndex] = i; // hy: write the position array

                expHighBits[expIndex] = (inputBlock[i] >> _b) & MASK[32-_b];   // hy: write the higher 32-b bits

                expIndex++;
            }
            offset += _b;
        }
        _expNum = expIndex;

        // hy: assuming expNum < 2^10

        compressedBlock[0] = (_b << 10) | (_expNum & 0x3ff);
        if (_expNum>0)
        {
            // System.out.println("comp " + ", expPos" + expPos[0] + ", expHighBits:" + expHighBits[0]);
            int compressedExpPosBitSize = compressExpPosByS16(compressedBlock, offset, expPos, expIndex);

            //int compressedExpPosBitSize = compressExpPosByVbyte(compressedBlock, offset, expPos, expIndex);

            offset += compressedExpPosBitSize;
            int compressedExpHighBits = compressExpHighBitsByS16(compressedBlock, offset, expHighBits, expIndex, 32-_b);

            offset += compressedExpHighBits;
        }
        else
        {
            //System.out.println("en: no exp");
        }

//    if(offset > MAX_POSSIBLE_COMPRESSED_BITSIZE)
//
//        {
//       System.err.println("ERROR: compressed buffer overflow");

//    }
        _compressedBitSize = offset;
    }

    int getCompressedBitSize()
    {
        return _compressedBitSize;
    }


    void writeBits(int* out, int val, int outOffset, int bits)
    {
        // hy: length must > 0

        int index = outOffset >> 5;


        int skip = outOffset & 0x1f;

        val &= (0xffffffff >> (32 - bits));

        out[index] |= (val << skip);

        if (32 - skip < bits)
        {
            out[index + 1] |= (val >> (32 - skip));
        }
    }


    int readBits(int* in,  int inOffset,  int bits)
    {

        int index = inOffset >> 5;


        int skip = inOffset & 0x1f;

        int val = (int)(in[index] >> skip);

        if (32 - skip < bits)
        {
            val |= (int)(in[index + 1] << (32 - skip));
        }
        return val & (0xffffffff >> (32 - bits));
    }

    int* decompress(int* compBlock)
    {
        return decompressOneBlock(compBlock);
    }

    // hy: call this function after setParam() is called

    int* decompressOneBlock(int* compBlock)
    {
        // hy: first decompress the _b and _expNum
        _expNum = compBlock[0] & 0x3ff;
        _b = (compBlock[0]>>10) & (0x1f);

        int* decompBlock = new int[_batchSize];
        int* expPos = new int[_expNum];
        int* expHighBits =  new int[_expNum];
        int offset = HEADER_MASK*HEADER_NUM;

        int compressedBits = 0;

        //System.out.println("decompressOneBlock(): _batchSize:" + _batchSize + "_b:" + _b + "expNum" + _expNum);
        compressedBits = decompressBBitSlots(decompBlock, compBlock, _batchSize, _b);

        offset += compressedBits;
        //compressedBits = decompressExpPosByS16(expPos, compBlock, _expNum, offset);

        if (_expNum>0)
        {
            compressedBits = decompressExpPosByS16(expPos, compBlock, _expNum, offset);

            offset += compressedBits;
            compressedBits = decompressExpHighBitsByS16(expHighBits, compBlock, _expNum, offset, 32-_b);

            offset += compressedBits;
            int i=0;
            for (i = 0; i < _expNum; i++)
            {
                //System.out.println("i: " + i + ", expPos" + expPos[i] + ", expHighBits:" + expHighBits[i]);

                decompBlock[expPos[i]] = (decompBlock[expPos[i]] & MASK[_b]) | (expHighBits[i] << _b);
            }
        }

        //System.out.println("tomtom decompOneBlock");

        // printBlock(decompBlock, _batchSize);

        return decompBlock;
    }



//hy: call this function after setParam() is called

    int decompressOneBlockFast(int* decompBlock, int* compBlock)
    {
        // hy: first decompress the _b and _expNum
        _expNum = compBlock[0] & 0x3ff;

        _b = (compBlock[0]>>10) & (0x1f);
        // int* decompBlock = new int[_batchSize];

        int offset = HEADER_MASK*HEADER_NUM;

        int compressedBits = 0;

        //System.out.println("decompressOneBlock(): _batchSize:" + _batchSize + "_b:" + _b + "expNum" + _expNum);
        compressedBits = decompressBBitSlots(decompBlock, compBlock, _batchSize, _b);

        offset += compressedBits;
        //compressedBits = decompressExpPosByS16(expPos, compBlock, _expNum, offset);

        if (_expNum>0)
        {
            compressedBits = decompressExpPosByS16(_myExpPos, compBlock, _expNum, offset);

            offset += compressedBits;

            compressedBits = decompressExpHighBitsByS16(_myExpHighBits, compBlock, _expNum, offset, 32-_b);

            offset += compressedBits;

            int i=0;

            for (i = 0; i < _expNum; i++)
            {
                //System.out.println("i: " + i + ", expPos" + expPos[i] + ", expHighBits:" + expHighBits[i]);

                decompBlock[_myExpPos[i]] = (decompBlock[_myExpPos[i]] & MASK[_b]) | (_myExpHighBits[i] << _b);
            }

        }

        //System.out.println("tomtom decompOneBlock");

        // printBlock(decompBlock, _batchSize);

        return offset;

    }


    int compressExpPosByS16(int* compBlock, int offset, int* expPos, int expSize)
    {
        // hy: preprocess
        if (offset%32 !=0)
        {
            std::cout<<"comp: offset should have been the multiple of 32"<<std::endl;;
            return -1;
        }
        int outOffset  = offset>>5;
        int num, inOffset=0, numLeft;

        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Compress(compBlock, outOffset, expPos, inOffset, numLeft);

            outOffset++;

            inOffset += num;

        }
        int compressedBitSize = (outOffset - (offset>>5))<<5;

        return compressedBitSize;
    }

    int decompressExpPosByS16(int* expPos, int * compBlock, int expSize, int offset)
    {
        int inOffset  = offset>>5;
        int num, outOffset=0, numLeft;

        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Decompress(expPos, outOffset, compBlock, inOffset, numLeft);

            outOffset += num;

            inOffset++;
        }

        int compressedIntNum = inOffset-(offset>>5);

        //postProcess

//    for(int i=1; i<decompressedNum; ++i)
//
//                {
//      expPos[i] = expPos[i] + expPos[i-1] + 1;
//    }

        int compressedBitSize = compressedIntNum << 5;

        return compressedBitSize;

    }

    int compressExpHighBitsByS16(int* compBlock, int offset, int* expHighBits, int expSize, int bits)
    {
        for (int i =0; i<expSize; i++)
        {
            expHighBits[i] &= MASK[bits];
        }

        int compressedBitSize = 0;

        int outOffset  = offset>>5;

        //System.out.println("compExpPos: startPos" + startPos + ", expSize:" + expSize);

        int num, inOffset=0, numLeft;

        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Compress(compBlock, outOffset, expHighBits, inOffset, numLeft);
            outOffset++;
            inOffset += num;
        }

        compressedBitSize = outOffset - (offset>>5);
        return compressedBitSize;

    }

    int decompressExpHighBitsByS16(int* expHighBits,  int* compBlock,  int expSize, int offset, int bits)
    {
        int inOffset  = offset>>5;
        int num, outOffset=0, numLeft;

        for (numLeft=expSize; numLeft>0; numLeft -= num)
        {
            num = s16Decompress(expHighBits, outOffset, compBlock, inOffset, numLeft);
            outOffset += num;
            inOffset++;
        }
        int compressedBitSize = outOffset << 5;

        return compressedBitSize;
    }

    // hy: no compression (just use int to represent them)

    int compressExpPosBasic(int* compBlock, int offset, int* expPos, int expSize)
    {
        // hy: currently no compression for pos

        if (offset%32 !=0)
        {
            std::cout<<"comp: offset should have been the multiple of 32"<<std::endl;;
            return -1;
        }

        int startPos  = offset>>5;

        //System.out.println("compExpPos: startPos" + startPos + ", expSize:" + expSize);

        for (int i=0; i<expSize; i++)
        {

            compBlock[startPos+i] = expPos[i];

            //System.out.print(expPos[i] + " ");

        }

        int compressedBitSize = expSize << 5;

        return compressedBitSize;
    }

    int decompressExpPosBasic(int* expPos, int* compBlock, int expSize, int offset)
    {
        // hy: currently no compression for pos
        if (offset%32 !=0)
        {
            std::cout<<"comp: offset should have been the multiple of 32"<<std::endl;;
            return -1;
        }
        int startPos = offset>>5;

        //System.out.println("decompExpPos: startPos" + startPos + ", expSize:" + expSize);

        for (int i=0; i<expSize; i++)
        {
            expPos[i] = compBlock[startPos+i];

            //System.out.print(expPos[i] + " ");
        }

        int compressedBitSize = expSize << 5;

        return compressedBitSize;

    }

    // hy: just use 32-b to represent the high bits of exps

    int compressExpHighBitsBasic(int* compBlock, int offset, int* expHighBits, int expSize, int bits)
    {

        int compressedBitSize = 0;

        for (int i =0; i<expSize; i++)
        {
            //System.out.println("comp: i:" + i + "b:" + b + "expHighBits" + expHighBits[i]);

            writeBits(compBlock, expHighBits[i] & MASK[bits], offset, bits);

            offset += bits;

        }

        compressedBitSize = bits * expSize;

        return compressedBitSize;

    }


    int decompressExpHighBitsBasic(int* expHighBits, int* compBlock, int expSize, int offset, int bits)
    {
        if (offset%32 !=0)
        {
            std::cout<<"comp: offset should have been the multiple of 32"<<std::endl;;
            return -1;
        }

        int compressedBitSize = 0;

        for (int i =0; i<expSize; i++)
        {

            //System.out.println("b:" + b + "decomp: i:" + i + "expHighBits" + expHighBits[i]);

            expHighBits[i] = readBits(compBlock, offset, bits);

            offset +=  bits;

            //expHighBits[i] = readBits(compBlock, offset +i*32, 32) & BATCH_HIGHER_MASK;

        }

        compressedBitSize = bits * expSize;
        return compressedBitSize;
    }

    int decompressBBitSlots(int* decompressedSlots, int* compBlock, int blockSize, int bits)
    {
        int compressedBitSize = 0;

        int offset = HEADER_MASK *HEADER_NUM;

        for (int i =0; i<blockSize; i++)
        {
            decompressedSlots[i] = readBits(compBlock, offset, bits);

            offset += bits;
        }

        compressedBitSize = bits * blockSize;
        return compressedBitSize;
    }

    int decompressBBitSlotsFast(int* decompressedSlots, int* compBlock, int blockSize, int bits)
    {

        int compressedBitSize = 0;

        PForDeltaUnpack::unpack((uint32_t*)decompressedSlots, (uint32_t*)compBlock, bits);

        compressedBitSize = bits * blockSize;
        return compressedBitSize;
    }




    // hy: n is the number to be compressed, after the call, inOffset+=returnValue, outOffset++;

    int s16Compress(int* out, int outOffset, int* in, int inOffset, int n)
    {

        int numIdx, j, num, bits;

        for (numIdx = 0; numIdx < S16_NUMSIZE; numIdx++)
        {

            out[outOffset] = numIdx<<S16_BITSSIZE;

            num = (S16_NUM[numIdx] < n) ? S16_NUM[numIdx] : n;
            for (j = 0, bits = 0; (j < num) && in[inOffset+j] < (1<<S16_BITS[numIdx][j]); )
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


    int s16Decompress(int* out, int outOffset, int* in, int inOffset, int n)
    {
        int numIdx, j=0, bits=0;

        numIdx = in[inOffset]>>S16_BITSSIZE;

        int num = S16_NUM[numIdx] < n ? S16_NUM[numIdx] : n;

        for (j=0, bits=0; j<num; j++)
        {
            out[outOffset+j] = (in[inOffset] >> bits) & (MASK[S16_BITS[numIdx][j]]) ;

            bits += S16_BITS[numIdx][j];
        }

        return num;
    }



    int binaryEncode(int* out, int outOffset,  int val,  int lo, int hi)
    {

        int w=0, bits=0;

        w = hi-lo+1; /*the number of values*/

        /* w-1 is the maximum value , so this loop caculate how many bits are needed for the max value */

        for (w=w-1, bits=0; w>0; w>>=1) bits++;

        if (bits>0)

            writeBits(out, (val-lo) & MASK[bits], outOffset, bits);

        return bits;
    }


    int ipcRecur(int* buf,  int outOffset, int* L, int inIdx, int f, int lo, int hi)
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



    int ipcCompress(int* out, int outOffset, int* in, int n, int prevLast, int hi)
    {

        if (n<1) return 0;

        int lo = prevLast+1;

        int compressedBitsNum =  ipcRecur(out, outOffset, in, 0, n, lo, hi);

        return compressedBitsNum;
    }



    // hy: return bitsNum

    int vbyteCompress(int* out, int outOffset, int val)
    {
        int* barray = new int[5];

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

                //System.out.println("come on: barray[i]" + barray[i]);

                writeBits(out, (barray[i]  | 0x1) & MASK[8], newOutOffset, 8);

                newOutOffset+=8;
            }
        }

        //System.out.println("barray[0]" + barray[0]);

        writeBits(out, (barray[0] | 0x0) & MASK[8], newOutOffset, 8);

        //System.out.println("read: barray[0]" + readBits(out, newOutOffset, 8));

        newOutOffset+=8;

        return newOutOffset-outOffset;

    }



    // hy: return bitsNum

    int vbyteDecompress(int* out, int outIdx, int* in, int inOffset)
    {

        int newInOffset = inOffset;

        int nextByte =  readBits(in, newInOffset, 8);

        //System.out.println("nextByte: barray[0]" + (nextByte & MASK[7]));

        int flag = nextByte & 0x1;

        int val = nextByte>>1;

        //System.out.println("val: barray[0]" + val);

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


    int compressExpPosByVbyte(int* compBlock, int outOffset, int* expPos, int expSize)
    {
        // hy: currently no compression for pos

        if (outOffset%32 !=0)
        {
            std::cout<<"comp: offset should have been the multiple of 32"<<std::endl;;
            return -1;
        }

        int newOutOffset = outOffset;

        int bitsNum;


        for (int i=0; i<expSize; i++)
        {

            bitsNum = vbyteCompress(compBlock, newOutOffset, expPos[i]);

            newOutOffset += bitsNum;

            //System.out.print(expPos[i] + " ");

        }
        //System.out.println("en vbyte: compExpPos:  expSize:" + expSize + "bitSize:" + (newOutOffset-outOffset));
        return newOutOffset-outOffset;

    }


    int decompressExpPosByVbyte(int* expPos, int* compBlock, int expSize, int inOffset)
    {
        // hy: currently no compression for pos
        //System.out.println("inOffset: " + inOffset);
        int newInOffset = inOffset;

        int bitsNum;

        //System.out.println("decompExpPos: startPos" + startPos + ", expSize:" + expSize);

        for (int i=0; i<expSize; i++)
        {

            bitsNum = vbyteDecompress(expPos, i, compBlock, newInOffset);

            newInOffset += bitsNum;

            //System.out.print(expPos[i] + " ");
        }
        //System.out.println("de vbyte: compExpPos:  expSize:" + expSize + "bitSize:" + (newInOffset-inOffset));

        return newInOffset-inOffset;

    }


};

