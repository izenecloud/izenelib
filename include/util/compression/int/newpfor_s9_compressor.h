#ifndef IZENE_UTIL_COMPRESSION_INT_NEWPFORDELTA_MIX_S9_COMPRESSOR_H
#define IZENE_UTIL_COMPRESSION_INT_NEWPFORDELTA_MIX_S9_COMPRESSOR_H

#include <iostream>
#include <math.h>
#include <stdexcept>
#include <algorithm>

#include "newpfor_decompress.h"

namespace izenelib{namespace util{namespace compression{

static uint32_t basicMask[] =
    {0x00000000,
     0x00000001, 0x00000003, 0x00000007, 0x0000000f, 0x0000001f, 0x0000003f,
     0x0000007f, 0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
     0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff, 0x0001ffff, 0x0003ffff,
     0x0007ffff, 0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
     0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff, 0x1fffffff, 0x3fffffff,
     0x7fffffff, 0xffffffff
    };

static int bitLength[] = { 1, 2, 3, 4, 5, 7, 9, 14, 28 };

static int codeNum[] = { 28, 14, 9, 7, 5, 4, 3, 2, 1 };

class newpfor_mix_s9_compressor
{
    class S9Compressor
    {
    public:
        int encode( uint32_t* numbers, uint32_t* output, int length )
        {
            int currentPos = 0;
            int outputPos = 0;
            while ( currentPos < length )
            {
                for ( int selector = 0 ; selector < 9 ; selector++ )
                {
                    uint32_t res = 0;
                    int compressedNum = codeNum[selector];
                    if ( length <= currentPos + compressedNum -1 )
                        continue;
                    int b = bitLength[selector];
                    uint32_t max = 1 << b ;
                    int i = 0;
                    for ( ; i < compressedNum ; i++ )
                        if ( max <= numbers[currentPos + i] )
                            break;
                        else
                            res = ( res << b ) + numbers[currentPos + i];
                    if ( i == compressedNum )
                    {
                        res |= selector << 28;
                        output[outputPos++] = res;
                        currentPos += compressedNum;
                        break;
                    }
                }
            }
            return outputPos;
        }
        void decode( uint32_t* encodedValue, int length, uint32_t* decode, int firstExceptionPos )
        {
            int correntPos = firstExceptionPos;
            uint32_t head = 0;
            for ( int i = 0 ; i < length ; i++ )
            {
                uint32_t val = encodedValue[i] ;
                uint32_t header = ( val >> 28 ) + head;
				
                switch ( header )
                {
                case 0 :
                { //code num : 28, bitwidth : 1
                    decode[ correntPos ] = ( val << 4 ) >> 31 ;
                    correntPos += ( ( val << 5 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 6 ) >> 31 ;
                    correntPos += ( ( val << 7 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 8 ) >> 31 ;
                    correntPos += ( ( val << 9 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 10 ) >> 31 ;
                    correntPos += ( ( val << 11 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 12 ) >> 31 ;
                    correntPos += ( ( val << 13 ) >> 31 ) + 1 ; //10
                    decode[ correntPos ] = ( val << 14 ) >> 31 ;
                    correntPos += ( ( val << 15 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 16 ) >> 31 ;
                    correntPos += ( ( val << 17 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 18 ) >> 31 ;
                    correntPos += ( ( val << 19 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 20 ) >> 31 ;
                    correntPos += ( ( val << 21 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 22 ) >> 31 ;
                    correntPos += ( ( val << 23 ) >> 31 ) + 1 ; //20
                    decode[ correntPos ] = ( val << 24 ) >> 31 ;
                    correntPos += ( ( val << 25 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 26 ) >> 31 ;
                    correntPos += ( ( val << 27 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 28 ) >> 31 ;
                    correntPos += ( ( val << 29 ) >> 31 ) + 1 ;
                    decode[ correntPos ] = ( val << 30 ) >> 31 ;
                    correntPos += ( ( val << 31 ) >> 31 ) + 1 ;
                    head = 0;
                    break;
                }
                case 1 :
                { //code num : 14, bitwidth : 2
                    decode[ correntPos ] = ( val << 4 ) >> 30 ;
                    correntPos += ( ( val << 6 ) >> 30 ) + 1 ;
                    decode[ correntPos ] = ( val << 8 ) >> 30 ;
                    correntPos += ( ( val << 10 ) >> 30 ) + 1 ;
                    decode[ correntPos ] = ( val << 12 ) >> 30 ;
                    correntPos += ( ( val << 14 ) >> 30 ) + 1 ;
                    decode[ correntPos ] = ( val << 16 ) >> 30 ;
                    correntPos += ( ( val << 18 ) >> 30 ) + 1 ;
                    decode[ correntPos ] = ( val << 20 ) >> 30 ;
                    correntPos += ( ( val << 22 ) >> 30 ) + 1 ; //10
                    decode[ correntPos ] = ( val << 24 ) >> 30 ;
                    correntPos += ( ( val << 26 ) >> 30 ) + 1 ;
                    decode[ correntPos ] = ( val << 28 ) >> 30 ;
                    correntPos += ( ( val << 30 ) >> 30 ) + 1 ;
                    head = 0;
                    break;
                }
                case 2 :
                { //code num : 9, bitwidth : 3
                    decode[ correntPos ] = ( val << 5 ) >> 29 ;
                    correntPos += ( ( val << 8 ) >> 29 ) + 1 ;
                    decode[ correntPos ] = ( val << 11 ) >> 29 ;
                    correntPos += ( ( val << 14 ) >> 29 ) + 1 ;
                    decode[ correntPos ] = ( val << 17 ) >> 29 ;
                    correntPos += ( ( val << 20 ) >> 29 ) + 1 ;
                    decode[ correntPos ] = ( val << 23 ) >> 29 ;
                    correntPos += ( ( val << 26 ) >> 29 ) + 1 ;
                    decode[ correntPos ] = ( val << 29 ) >> 29 ;
                    head = 16;
                    break;
                }
                case 3 :
                { //code num : 7, bitwidth : 4
                    decode[ correntPos ] = ( val << 4 ) >> 28 ;
                    correntPos += ( ( val << 8 ) >> 28 ) + 1 ;
                    decode[ correntPos ] = ( val << 12 ) >> 28 ;
                    correntPos += ( ( val << 16 ) >> 28 ) + 1 ;
                    decode[ correntPos ] = ( val << 20 ) >> 28 ;
                    correntPos += ( ( val << 24 ) >> 28 ) + 1 ;
                    decode[ correntPos ] = ( val << 28 ) >> 28 ;
                    head = 16;
                    break;
                }
                case 4 :
                { //code num : 5, bitwidth : 5
                    decode[ correntPos ] = ( val << 7 ) >> 27 ;
                    correntPos += ( ( val << 12 ) >> 27 ) + 1 ;
                    decode[ correntPos ] = ( val << 17 ) >> 27 ;
                    correntPos += ( ( val << 22 ) >> 27 ) + 1 ;
                    decode[ correntPos ] = ( val << 27 ) >> 27 ;
                    head = 16;
                    break;
                }
                case 5 :
                { //code num : 4, bitwidth : 7
                    decode[ correntPos ] = ( val << 4 ) >> 25 ;
                    correntPos += ( ( val << 11 ) >> 25 ) + 1 ;
                    decode[ correntPos ] = ( val << 18 ) >> 25 ;
                    correntPos += ( ( val << 25 ) >> 25 ) + 1 ;
                    head = 0;
                    break;
                }
                case 6 :
                { //code num : 3, bitwidth : 9
                    decode[ correntPos ] = ( val << 5 ) >> 23 ;
                    correntPos += ( ( val << 14 ) >> 23 ) + 1 ;
                    decode[ correntPos ] = ( val << 23 ) >> 23 ;
                    head = 16;
                    break;
                }
                case 7 :
                { //code num : 2, bitwidth : 14
                    decode[ correntPos ] = ( val << 4 ) >> 18 ;
                    correntPos += ( ( val << 18 ) >> 18 ) +1 ;
                    head = 0;
                    break;
                }
                case 8 :
                { //code num : 1, bitwidth : 28
                    decode[ correntPos ] = ( val << 4 ) >> 4;
                    head = 16;
                    break;
                }
                case 16 :
                { //code num : 28, bitwidth : 1
                    correntPos += ( val << 4 ) >> 31 ;
                    decode[ correntPos ] = ( val << 5 ) >> 31 ;
                    correntPos += ( val << 6 ) >> 31 ;
                    decode[ correntPos ] = ( val << 7 ) >> 31 ;
                    correntPos += ( val << 8 ) >> 31 ;
                    decode[ correntPos ] = ( val << 9 ) >> 31 ;
                    correntPos += ( val << 10 ) >> 31 ;
                    decode[ correntPos ] = ( val << 11 ) >> 31 ;
                    correntPos += ( val << 12 ) >> 31 ;
                    decode[ correntPos ] = ( val << 13 ) >> 31 ; //10
                    correntPos += ( val << 14 ) >> 31 ;
                    decode[ correntPos ] = ( val << 15 ) >> 31 ;
                    correntPos += ( val << 16 ) >> 31 ;
                    decode[ correntPos ] = ( val << 17 ) >> 31 ;
                    correntPos += ( val << 18 ) >> 31 ;
                    decode[ correntPos ] = ( val << 19 ) >> 31 ;
                    correntPos += ( val << 20 ) >> 31 ;
                    decode[ correntPos ] = ( val << 21 ) >> 31 ;
                    correntPos += ( val << 22 ) >> 31 ;
                    decode[ correntPos ] = ( val << 23 ) >> 31 ; //20
                    correntPos += ( val << 24 ) >> 31 ;
                    decode[ correntPos ] = ( val << 25 ) >> 31 ;
                    correntPos += ( val << 26 ) >> 31 ;
                    decode[ correntPos ] = ( val << 27 ) >> 31 ;
                    correntPos += ( val << 28 ) >> 31 ;
                    decode[ correntPos ] = ( val << 29 ) >> 31 ;
                    correntPos += ( val << 30 ) >> 31 ;
                    decode[ correntPos ] = ( val << 31 ) >> 31 ;
                    head = 16;
                    break;
                }
                case 17 :
                { //code num : 14, bitwidth : 2
                    correntPos += ( val << 4 ) >> 30 ;
                    decode[ correntPos ] = ( val << 6 ) >> 30 ;
                    correntPos += ( val << 8 ) >> 30 ;
                    decode[ correntPos ] = ( val << 10 ) >> 30 ;
                    correntPos += ( val << 12 ) >> 30 ;
                    decode[ correntPos ] = ( val << 14 ) >> 30 ;
                    correntPos += ( val << 16 ) >> 30 ;
                    decode[ correntPos ] = ( val << 18 ) >> 30 ;
                    correntPos += ( val << 20 ) >> 30 ;
                    decode[ correntPos ] = ( val << 22 ) >> 30 ; //10
                    correntPos += ( val << 24 ) >> 30 ;
                    decode[ correntPos ] = ( val << 26 ) >> 30 ;
                    correntPos += ( val << 28 ) >> 30 ;
                    decode[ correntPos ] = ( val << 30 ) >> 30 ;
                    head = 16;
                    break;
                }
                case 18 :
                { //code num : 9, bitwidth : 3
                    correntPos += ( val << 5 ) >> 29 ;
                    decode[ correntPos ] = ( val << 8 ) >> 29 ;
                    correntPos += ( val << 11 ) >> 29 ;
                    decode[ correntPos ] = ( val << 14 ) >> 29 ;
                    correntPos += ( val << 17 ) >> 29 ;
                    decode[ correntPos ] = ( val << 20 ) >> 29 ;
                    correntPos += ( val << 23 ) >> 29 ;
                    decode[ correntPos ] = ( val << 26 ) >> 29 ;
                    correntPos += ( val << 29 ) >> 29 ;
                    head = 0;
                    break;
                }
                case 19 :
                { //code num : 7, bitwidth : 4
                    correntPos += ( val << 4 ) >> 28 ;
                    decode[ correntPos ] = ( val << 8 ) >> 28 ;
                    correntPos += ( val << 12 ) >> 28 ;
                    decode[ correntPos ] = ( val << 16 ) >> 28 ;
                    correntPos += ( val << 20 ) >> 28 ;
                    decode[ correntPos ] = ( val << 24 ) >> 28 ;
                    correntPos += ( val << 28 ) >> 28 ;
                    head = 0;
                    break;
                }
                case 20 :
                { //code num : 5, bitwidth : 5
                    correntPos += ( val << 7 ) >> 27 ;
                    decode[ correntPos ] = ( val << 12 ) >> 27 ;
                    correntPos += ( val << 17 ) >> 27 ;
                    decode[ correntPos ] = ( val << 22 ) >> 27 ;
                    correntPos += ( val << 27 ) >> 27 ;
                    head = 0;
                    break;
                }
                case 21 :
                { //code num : 4, bitwidth : 7
                    correntPos += ( val << 4 ) >> 25 ;
                    decode[ correntPos ] = ( val << 11 ) >> 25 ;
                    correntPos += ( val << 18 ) >> 25 ;
                    decode[ correntPos ] = ( val << 25 ) >> 25 ;
                    head = 16;
                    break;
                }
                case 22 :
                { //code num : 3, bitwidth : 9
                    correntPos += ( val << 5 ) >> 23 ;
                    decode[ correntPos ] = ( val << 14 ) >> 23 ;
                    correntPos += ( val << 23 ) >> 23 ;
                    head = 0;
                    break;
                }
                case 23 :
                { //code num : 2, bitwidth : 14
                    correntPos += ( val << 4 ) >> 18 ;
                    decode[ correntPos ] = ( val << 18 ) >> 18 ;
                    head = 16;
                    break;
                }
                case 24 :
                { //code num : 1, bitwidth : 28
                    correntPos += ( val << 4 ) >> 4;
                    head = 0;
                    break;
                }
                default :
                    throw std::runtime_error("illegal argument");
                }
            }
        }
    };


    static const double exceptionThresholdRate_ ;
    static const double exceptionRate_ ;
    static const int blockSize_ = 128;
    static const int headerSize_ = 1;
    static const int MAX_EXPECTED_BLOCKSIZE = 1024;
    uint32_t exceptionList_[MAX_EXPECTED_BLOCKSIZE];
    uint32_t exceptionOffset_[MAX_EXPECTED_BLOCKSIZE];
    uint32_t exceptionDatum_[MAX_EXPECTED_BLOCKSIZE*2];	
    uint32_t miss_[MAX_EXPECTED_BLOCKSIZE];
    uint32_t code_[MAX_EXPECTED_BLOCKSIZE];

    S9Compressor s9compressor_;

public:
    int compress(unsigned int* input, unsigned int* output, int size)
    {
        int total_comp_len = 0;

        int b = 0; // numFrameBits
        int exceptionNum = 0;

        for (int i = 0; i < size; i += blockSize_)
        {
            int curr_len = 0;
            if ((size - i) >= blockSize_)
            {
                curr_len = blockSize_;
            }
            else
            {
                curr_len = size - i;
            }
            estimateCompressBits( input, curr_len, b, exceptionNum);
            b = 20;
            exceptionNum = 0;
            int compressed_len = compress_block(input, curr_len, output, b, exceptionNum);
		
            output += compressed_len;
            input += curr_len;
		
            total_comp_len += compressed_len;
        }
        return total_comp_len;
    }

private:
    int compress_block(uint32_t* numbers, int length, uint32_t* outputBlock, int bitFrame, int exceptionNum)
    {
        int exceptionIntOffset = headerSize_ + ( length * bitFrame + 31 ) / 32;

        if ( exceptionNum == 0 )
        {
            outputBlock[0] = makeHeader( numbers, length, bitFrame, 0, 0, 0 );
            for ( int i = 0 ; i < length ; i++ )
            {
                encodeCompressedValue( i, numbers[i], bitFrame, outputBlock ); // normal encoding
            }
            return exceptionIntOffset;
        }
        int pre = 0;
        uint32_t max = 1 << bitFrame;
        // loop1: find exception
        int j = 0;
        for ( int i = 0 ; i < length ; i++ )
        {
            uint32_t val = numbers[i];
            code_[i] = val;
            miss_[j] = i;
            if( max <= val ) j++;
        }
        // loop2: create offset and upper-bit value list for patch .
        int cur = miss_[ 0 ];
        exceptionList_[0] = code_[cur] >> bitFrame ;
        code_[cur] = code_[cur] & basicMask[bitFrame];
        pre = cur ;
        for ( int i = 1 ; i < j ; i++ )
        {
            cur = miss_[ i ];
            exceptionList_[i] = code_[cur] >> bitFrame;
            code_[cur] = code_[cur] & basicMask[bitFrame];
            exceptionOffset_[i-1] = cur - pre - 1;
            pre = cur;
        }
        int firstExceptionPos = miss_[0] + 1;
	
        //make exception region
        for ( int i = 0 ; i < exceptionNum ; i++ )
            exceptionDatum_[ 2 * i ] = exceptionList_[i];
        int exceptionOffsetNum = exceptionNum-1;
        for ( int i = 0 ; i < exceptionOffsetNum; i++ )
            exceptionDatum_[ 2 * i + 1 ] = exceptionOffset_[i];
        int exceptionRange = s9compressor_.encode( exceptionDatum_, outputBlock+exceptionIntOffset, exceptionNum+exceptionOffsetNum);
        int intDataSize = exceptionIntOffset + exceptionRange;

        // 1: make header
        outputBlock[0] = makeHeader( code_, length, bitFrame, firstExceptionPos, exceptionNum, exceptionRange );
        // 2: make encoded value
        for ( int i = 0 ; i < length ; i++ )
        {
            encodeCompressedValue( i, code_[i], bitFrame, outputBlock ); // normal encoding
        }
        return intDataSize;
    }

private:
    /**
     * encode normal value
     *
     * @param i
     * @param val
     * @param b
     * @param frame
     */
    void encodeCompressedValue( int i, uint32_t val, int b, uint32_t* frame )
    {
        uint32_t totalBitCount = b * i;
        uint32_t intPos = totalBitCount >> 5;
        uint32_t firstBit = totalBitCount % 32;
        uint32_t endBit = firstBit + b;
        uint32_t mask  = 0;
        mask = ~( basicMask[b] << firstBit );
        uint32_t _val = val << firstBit;
        frame[ intPos + headerSize_ ] = (frame[ intPos + headerSize_ ] & mask) | _val;
		
        // over bit-width of integer
        if ( 32 < endBit )
        {
            uint32_t shiftBit = b - ( endBit - 32 );
            mask = ~( basicMask[b] >> shiftBit );
            _val = val >> shiftBit;
            frame[ intPos + headerSize_ + 1] = (frame[ intPos + headerSize_ + 1] & mask) | _val;
        }
    }
    /**
     * Header is consist of 1 byte and the construction is as follow
     *
     * 7bit : dataNum - 1
     * 8bit : first exceptionPos
     * 5bit : numFramebit -1
     * 11bit : exception byte range
     * 1bit : has next frame or not
     *
     * @param b
     * @param exceptionCode
     * @param firstExceptionPos
     * @param code
     * @param exception
     * @return
     */
    int makeHeader( uint32_t* code, int length, int b, int firstExceptionPos, int exceptionNum,
                    int exceptionIntRange)
    {
        int lastOrNot =  0;
        return (length-1) << 25
               | firstExceptionPos << 17
               | ( b - 1 ) << 12
               | exceptionIntRange << 1
               | lastOrNot;
    }

    /**
     * calculate estimate bit number of frame
     * it is estimated by prediction of 10% exception
     *
     * @param numbers
     * @param length : data length of this "For"
     * @return 2 value int
     *  ( bitFrame, exceptionNum )
     */
    void estimateCompressBits( uint32_t* numbers, int length, int& bitFrame, int& exceptionNum )
    {
        memcpy(code_,numbers, length*sizeof(uint32_t));
        std::sort(code_, code_+ length);
        uint32_t maxValue = code_[ length - 1 ];
        if ( maxValue <= 1 )
        {
            // bitFrame, exceptionNum, exceptionCode :
            bitFrame = 1;
            exceptionNum = 0;
            return;
        }
        int exceptionCode = ( maxValue < ( 1 << 8 ) ) ? 0 : (maxValue < (1 << 16 )) ? 1 : 2;
        int bytesPerException = 1 << exceptionCode;
        int frameBits = 1;
        int bytesForFrame = (length * frameBits + 7 ) / 8; // cut up byte
        // initially assume all inputs are exceptions.
        int totalBytes = bytesForFrame + length * bytesPerException; // excluding the header.
        int bestBytes = totalBytes;
        int bestFrameBits = frameBits;
        int bestExceptions = length;
        for (int i = 0; i < length; i++)
        {
            // determine frameBits so that code_[i] is no more exception
            while ( code_[i] >= (uint32_t)(1 << frameBits) )
            {
                if ( frameBits == 30 )
                { // no point to increase further.
                    return rebuild( code_, length, bestFrameBits, length - i - 1, bitFrame, exceptionNum);
                }
                ++frameBits;
                // increase bytesForFrame and totalBytes to correspond to frameBits
                int newBytesForFrame = (length * frameBits + 7 ) / 8;
                totalBytes += newBytesForFrame - bytesForFrame;
                bytesForFrame = newBytesForFrame;
            }
            totalBytes -= bytesPerException; // no more need to store code_[i] as exception
            if ( totalBytes <= bestBytes )
            { // <= : prefer fewer exceptions at higher number of frame bits.
                bestBytes = totalBytes;
                bestFrameBits = frameBits;
                bestExceptions = length - i - 1;
            }
        }
        rebuild( code_, length, bestFrameBits, bestExceptions, bitFrame, exceptionNum);
    }

    void rebuild( uint32_t* copy, int length, int bestFrameBits, int bestExceptions, int& bitFrame, int& exceptionNum)
    {
        if ( bestExceptions <= blockSize_ * exceptionThresholdRate_ )
        {
            bitFrame = bestFrameBits;
            exceptionNum = bestExceptions;
            return;
        }
        int searchPos = (int) floor( length * ( 1- exceptionRate_ ) );
        searchPos = searchPos == 0 ? 1 : searchPos;
        uint32_t currentVal = copy[ searchPos - 1 ];
        int i = 1;
        uint32_t max = 0 ;
        for ( ; i < 32 ; i++ )
        {
            max = basicMask[i];
            if ( currentVal <= max )
                break;
        }
        int candidateBit = i;
        // search exception num
        for ( int j = 0 ; j < length ; j++ )
        {
            if ( max < copy[j] )
            {
                bitFrame = candidateBit;
                exceptionNum = length - j;
                return;
            }
        }

        bitFrame = candidateBit;
        exceptionNum = 0;
    }

public:
    int decompress(unsigned int* input, unsigned int* output, int size)
    {
        int total_decom_len = 0;

        for (int i = 0; i < size; i += blockSize_)
        {
            int curr_len = 0;
            if ((size - i) >= blockSize_)
            {
                curr_len = blockSize_;
            }
            else
            {
                curr_len = size - i;
            }

            int decomp_len = decompress_block(output, input);
            input += decomp_len;
            output += curr_len;
            total_decom_len += decomp_len;
        }
        return total_decom_len;
    }

private:
    int decompress_block(uint32_t* decode, uint32_t* encodedValue)
    {
        /****************************************************************
         * decode header value
         * header component is as follow
         *
         * 7bit : dataNum - 1
         * 8bit : first exceptionPos
         * 5bit : numFramebit -1
         * 11bit : exception byte range
         * 1bit : has next frame or not
         *
         *****************************************************************/
         
        uint32_t headerValue = encodedValue[0];
        int dataNum  = ( headerValue >> 25 ) + 1 ;
        int firstExceptionPos  = ( headerValue << 7 ) >> 24 ;  // miss[0] + 1 or 0
        int numFrameBit  = ( ( headerValue << 15) >> 27 ) + 1 ; // 1 < numFramebit < 32
        int exceptionIntRange = ( headerValue << 20 ) >> 21 ;
        /***************************************************************/
        // first loop
        int encodeOffset = headerSize_ ;
        int intOffsetForExceptionRange;
        switch ( numFrameBit )
        {
        case 1 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor1Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 2 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor2Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 3 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor3Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 4 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor4Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 5 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor5Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 6 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor6Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 7 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor7Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 8 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor8Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 9 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor9Bit ( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 10 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor10Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 11 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor11Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 12 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor12Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 13 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor13Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 14 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor14Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 15 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor15Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 16 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor16Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 17 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor17Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 18 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor18Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 19 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor19Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 20 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor20Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 21 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor21Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 22 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor22Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 23 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor23Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 24 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor24Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 25 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor25Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 26 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor26Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 27 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor27Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 28 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor28Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 29 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor29Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 30 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor30Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 31 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor31Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        case 32 :
            intOffsetForExceptionRange = NewPForDecompress::fastDeCompressFor32Bit( encodeOffset, encodedValue, dataNum, 0, decode );
            break;
        default :
            throw std::runtime_error("numFramBit is too high ! " + numFrameBit);
        }
		
        //exception loop
        if ( firstExceptionPos != 0 )
            s9compressor_.decode( encodedValue+intOffsetForExceptionRange, exceptionIntRange, decode, firstExceptionPos - 1 );


        return intOffsetForExceptionRange + exceptionIntRange;
    }

};


}}}
#endif

