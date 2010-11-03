#ifndef IZENE_UTIL_COMPRESSION_INT_NEWPFORDELTA_DECOMPRESS_H
#define IZENE_UTIL_COMPRESSION_INT_NEWPFORDELTA_DECOMPRESS_H

namespace izenelib{namespace util{namespace compression{

class NewPForDecompress
{
public:
    static int fastDeCompressFor1Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val = encodedValue[ offSet ++ ];
            decode[ decodeOffset ++ ] = ( val << 31 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 30 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 29 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 28 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 27 ) >> 31 ; //5
            decode[ decodeOffset ++ ] = ( val << 26 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 25 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 24 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 23 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 22 ) >> 31 ; //10
            decode[ decodeOffset ++ ] = ( val << 21 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 20 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 19 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 18 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 17 ) >> 31 ; //15
            decode[ decodeOffset ++ ] = ( val << 16 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 15 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 14 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 13 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 12 ) >> 31 ; //20
            decode[ decodeOffset ++ ] = ( val << 11 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 10 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 9 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 8 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 7 ) >> 31 ; //25
            decode[ decodeOffset ++ ] = ( val << 6 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 5 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 4 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 3 ) >> 31 ;
            decode[ decodeOffset ++ ] = ( val << 2 ) >> 31 ; //30
            decode[ decodeOffset ++ ] = ( val << 1 ) >> 31 ;
            decode[ decodeOffset ++ ] = val >> 31 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val << 31 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 30 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 29 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 28 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 27 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 26 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 25 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 24 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 23 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 22 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 21 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 20 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 19 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 18 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 17 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 16 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 15 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 14 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 13 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 12 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 11 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 10 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 9 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 8 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 7 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 6 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 5 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 4 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 3 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 2 ) >> 31 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 1 ) >> 31 ;
        return offSet;
    }

    static int fastDeCompressFor2Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        uint32_t val = 0;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            val = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val << 30 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 28 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 26 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 24 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 22 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 20 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 18 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 16 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 14 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 12 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 10 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 8 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 6 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 4 ) >> 30;
            decode[ decodeOffset++ ] = ( val << 2 ) >> 30;
            decode[ decodeOffset++ ] = val >> 30;
        }
        if ( rest == 0 )
            return offSet;
        val = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val << 30 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 28 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 26 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 24 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 22 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 20 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 18 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 16 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 14 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 12 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 10 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 8 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 6 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 4 ) >> 30;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val << 2 ) >> 30;
        return offSet;
    }

    static int fastDeCompressFor3Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            decode[ decodeOffset ++ ] = ( val1 << 29 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 26 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 23 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 20 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 17 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 14 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 11 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 8 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 5 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val1 << 2 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( (val2 << 31) >> 29 ) | (val1 >> 30) ;
            decode[ decodeOffset ++ ] = ( val2 << 28 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 25 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 22 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 19 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 16 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 13 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 10 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 7 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 4 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val2 << 1 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( (val3 << 30 ) >> 29 ) | (val2 >> 31);
            decode[ decodeOffset ++ ] = ( val3 << 27 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 24 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 21 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 18 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 15 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 12 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 9 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 6 ) >> 29 ;
            decode[ decodeOffset ++ ] = ( val3 << 3 ) >> 29 ;
            decode[ decodeOffset ++ ] = val3 >> 29 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 29 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 26 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 23 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 20 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 17 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 14 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 11 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 8 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 5 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 2 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( (val2 << 31) >> 29 ) | (val1 >> 30) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 28 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 25 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 22 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 19 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 16 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 13 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 10 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 7 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 1 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( (val3 << 30 ) >> 29 ) | (val2 >> 31);
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 27 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 24 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 21 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 18 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 15 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 12 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 9 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 29 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 3 ) >> 29 ;
        return offSet;
    }

    static int fastDeCompressFor4Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 3;
        int rest = dataNum % 8;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            decode[ decodeOffset ++ ] = ( val1 << 28 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 24 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 20 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 16 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 12 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 8 ) >> 28 ;
            decode[ decodeOffset ++ ] = ( val1 << 4 ) >> 28 ;
            decode[ decodeOffset ++ ] = val1 >> 28 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 28 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 24 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 20 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 16 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 12 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 8 ) >> 28 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 4 ) >> 28 ;
        return offSet;
    }
    static int fastDeCompressFor5Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet ++ ];
            uint32_t val2 = encodedValue[ offSet ++ ];
            uint32_t val3 = encodedValue[ offSet ++ ];
            uint32_t val4 = encodedValue[ offSet ++ ];
            uint32_t val5 = encodedValue[ offSet ++ ];
            decode[ decodeOffset ++ ] = ( val1 << 27 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val1 << 22 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val1 << 17 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val1 << 12 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val1 << 7 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val1 << 2 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( ( val2 << 29 ) >> 27 ) | ( val1 >> 30 ) ;
            decode[ decodeOffset ++ ] = ( val2 << 24 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val2 << 19 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val2 << 14 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val2 << 9 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val2 << 4 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( ( val3 << 31 ) >> 27 ) | ( val2 >> 28 ) ;
            decode[ decodeOffset ++ ] = ( val3 << 26 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val3 << 21 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val3 << 16 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val3 << 11 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val3 << 6 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val3 << 1 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( ( val4 << 28 ) >> 27 ) | ( val3 >> 31 ) ;
            decode[ decodeOffset ++ ] = ( val4 << 23 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val4 << 18 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val4 << 13 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val4 << 8 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val4 << 3 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( ( val5 << 30 ) >> 27 ) | ( val4 >> 29 ) ;
            decode[ decodeOffset ++ ] = ( val5 << 25 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val5 << 20 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val5 << 15 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val5 << 10 ) >> 27 ;
            decode[ decodeOffset ++ ] = ( val5 << 5 ) >> 27 ;
            decode[ decodeOffset ++ ] = val5 >> 27 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 27 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 22 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 17 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 12 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 7 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 2 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 29 ) >> 27 ) | ( val1 >> 30 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 24 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 19 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 14 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 9 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 31 ) >> 27 ) | ( val2 >> 28 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 26 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 21 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 16 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 11 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 1 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 28 ) >> 27 ) | ( val3 >> 31 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 23 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 18 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 13 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 8 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 3 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 30 ) >> 27 ) | ( val4 >> 29 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 25 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 20 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 15 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 10 ) >> 27 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 5 ) >> 27 ;
        return offSet;
    }

    static int fastDeCompressFor6Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 26 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val1 << 20 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val1 << 14 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val1 << 8 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val1 << 2 ) >> 26 ;
            decode[ decodeOffset++ ] = ( ( val2 << 28 ) >> 26 ) | ( val1 >> 30 );
            decode[ decodeOffset++ ] = ( val2 << 22 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val2 << 16 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val2 << 10 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val2 << 4 ) >> 26 ;
            decode[ decodeOffset++ ] = ( ( val3 << 30 ) >> 26 ) | ( val2 >> 28 );
            decode[ decodeOffset++ ] = ( val3 << 24 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val3 << 18 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val3 << 12 ) >> 26 ;
            decode[ decodeOffset++ ] = ( val3 << 6 ) >> 26 ;
            decode[ decodeOffset++ ] = val3 >> 26 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 26 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 20 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 14 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 8 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 2 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 28 ) >> 26 ) | ( val1 >> 30 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 22 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 16 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 10 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 30 ) >> 26 ) | ( val2 >> 28 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 24 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 18 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 12 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 26 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = val3 >> 26 ;
        return offSet;
    }

    static int fastDeCompressFor7Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet ++ ];
            uint32_t val2 = encodedValue[ offSet ++ ];
            uint32_t val3 = encodedValue[ offSet ++ ];
            uint32_t val4 = encodedValue[ offSet ++ ];
            uint32_t val5 = encodedValue[ offSet ++ ];
            uint32_t val6 = encodedValue[ offSet ++ ];
            uint32_t val7 = encodedValue[ offSet ++ ];
            decode[ decodeOffset++ ] = ( val1 << 25 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val1 << 18 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val1 << 11 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val1 << 4 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val2 << 29 ) >> 25 ) | ( val1 >> 28 ) ;
            decode[ decodeOffset++ ] = ( val2 << 22 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val2 << 15 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val2 << 8 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val2 << 1 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val3 << 26 ) >> 25 ) | ( val2 >> 31 ) ;
            decode[ decodeOffset++ ] = ( val3 << 19 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val3 << 12 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val3 << 5 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val4 << 30 ) >> 25 ) | ( val3 >> 27 ) ;
            decode[ decodeOffset++ ] = ( val4 << 23 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val4 << 16 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val4 << 9 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val4 << 2 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val5 << 27 ) >> 25 ) | ( val4 >> 30 ) ;
            decode[ decodeOffset++ ] = ( val5 << 20 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val5 << 13 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val5 << 6 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val6 << 31 ) >> 25 ) | ( val5 >> 26 ) ;
            decode[ decodeOffset++ ] = ( val6 << 24 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val6 << 17 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val6 << 10 ) >> 25 ;
            decode[ decodeOffset++ ] = ( val6 << 3 ) >> 25 ;
            decode[ decodeOffset++ ] = ( ( val7 << 28 ) >> 25 ) | ( val6 >> 29 ) ;
            decode[ decodeOffset++ ] = ( val7 << 21 ) >> 25;
            decode[ decodeOffset++ ] = ( val7 << 14 ) >> 25;
            decode[ decodeOffset++ ] = ( val7 << 7 ) >> 25;
            decode[ decodeOffset++ ] = val7 >> 25;
        }
        if ( rest == 0 )
            return offSet;

        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 25 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val1 << 18 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val1 << 11 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val1 << 4 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 29 ) >> 25 ) | ( val1 >> 28 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val2 << 22 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val2 << 15 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val2 << 8 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val2 << 1 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 26 ) >> 25 ) | ( val2 >> 31 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val3 << 19 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val3 << 12 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val3 << 5 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 30 ) >> 25 ) | ( val3 >> 27 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val4 << 23 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val4 << 16 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val4 << 9 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val4 << 2 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 27 ) >> 25 ) | ( val4 >> 30 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val5 << 20 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val5 << 13 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val5 << 6 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 31 ) >> 25 ) | ( val5 >> 26 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val6 << 24 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val6 << 17 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val6 << 10 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val6 << 3 ) >> 25 ;
        if ( --rest == 0 )return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 28 ) >> 25 ) | ( val6 >> 29 ) ;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val7 << 21 ) >> 25;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val7 << 14 ) >> 25;
        if ( --rest == 0 )return offSet;
        decode[ ++decodeOffset ] = ( val7 << 7 ) >> 25;
        return offSet;
    }

    static int fastDeCompressFor8Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 2;
        int rest = dataNum % 4;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            decode[ decodeOffset ++ ] = ( val1 << 24 ) >> 24 ;
            decode[ decodeOffset ++ ] = ( val1 << 16 ) >> 24 ;
            decode[ decodeOffset ++ ] = ( val1 << 8 ) >> 24 ;
            decode[ decodeOffset ++ ] = val1 >> 24 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 24 ) >> 24 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 16 ) >> 24 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 8 ) >> 24 ;
        return offSet;
    }

    static int fastDeCompressFor9Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet ++ ];
            uint32_t val2 = encodedValue[ offSet ++ ];
            uint32_t val3 = encodedValue[ offSet ++ ];
            uint32_t val4 = encodedValue[ offSet ++ ];
            uint32_t val5 = encodedValue[ offSet ++ ];
            uint32_t val6 = encodedValue[ offSet ++ ];
            uint32_t val7 = encodedValue[ offSet ++ ];
            uint32_t val8 = encodedValue[ offSet ++ ];
            uint32_t val9 = encodedValue[ offSet ++ ];
            decode[ decodeOffset ++ ] = ( val1 << 23 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val1 << 14 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val1 << 5 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val2 << 28 ) >> 23 ) | ( val1 >> 27 ) ;
            decode[ decodeOffset ++ ] = ( val2 << 19 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val2 << 10 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val2 << 1 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val3 << 24 ) >> 23 ) | ( val2 >> 31 ) ;
            decode[ decodeOffset ++ ] = ( val3 << 15 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val3 << 6 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val4 << 29 ) >> 23 ) | ( val3 >> 26 ) ;
            decode[ decodeOffset ++ ] = ( val4 << 20 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val4 << 11 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val4 << 2 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val5 << 25 ) >> 23 ) | ( val4 >> 30 ) ;
            decode[ decodeOffset ++ ] = ( val5 << 16 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val5 << 7 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val6 << 30 ) >> 23 ) | ( val5 >> 25 ) ;
            decode[ decodeOffset ++ ] = ( val6 << 21 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val6 << 12 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val6 << 3 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val7 << 26 ) >> 23 ) | ( val6 >> 29 ) ;
            decode[ decodeOffset ++ ] = ( val7 << 17 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val7 << 8 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val8 << 31 ) >> 23 ) | ( val7 >> 24 ) ;
            decode[ decodeOffset ++ ] = ( val8 << 22 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val8 << 13 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val8 << 4 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( ( val9 << 27 ) >> 23 ) | ( val8 >> 28 ) ;
            decode[ decodeOffset ++ ] = ( val9 << 18 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val9 << 9 ) >> 23 ;
            decode[ decodeOffset ++ ] = ( val9 ) >> 23 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 23 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 14 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 5 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 28 ) >> 23 ) | ( val1 >> 27 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 19 ) >> 23 ;  //5
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 10 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 1 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 24 ) >> 23 ) | ( val2 >> 31 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 15 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 23 ;  //10
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 29 ) >> 23 ) | ( val3 >> 26 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 20 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 11 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 2 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 25 ) >> 23 ) | ( val4 >> 30 ) ; //15
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 16 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 7 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 30 ) >> 23 ) | ( val5 >> 25 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 21 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 12 ) >> 23 ;  //20
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 3 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 26 ) >> 23 ) | ( val6 >> 29 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 17 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 8 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 31 ) >> 23 ) | ( val7 >> 24 ) ; //25
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 22 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 13 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 4 ) >> 23 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 27 ) >> 23 ) | ( val8 >> 28 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 18 ) >> 23 ;  //30
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 9 ) >> 23 ;
        return offSet;
    }
    static int fastDeCompressFor10Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode)
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 22 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val1 << 12 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val1 << 2 ) >> 22 ;
            decode[ decodeOffset++ ] = ( ( val2 << 24 ) >> 22 ) | ( val1 >> 30 ) ;
            decode[ decodeOffset++ ] = ( val2 << 14 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val2 << 4 ) >> 22 ;
            decode[ decodeOffset++ ] = ( ( val3 << 26 ) >> 22 ) | ( val2 >> 28 ) ;
            decode[ decodeOffset++ ] = ( val3 << 16 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val3 << 6 ) >> 22 ;
            decode[ decodeOffset++ ] = ( ( val4 << 28 ) >> 22 ) | ( val3 >> 26 ) ;
            decode[ decodeOffset++ ] = ( val4 << 18 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val4 << 8 ) >> 22 ;
            decode[ decodeOffset++ ] = ( ( val5 << 30 ) >> 22 ) | ( val4 >> 24 ) ;
            decode[ decodeOffset++ ] = ( val5 << 20 ) >> 22 ;
            decode[ decodeOffset++ ] = ( val5 << 10 ) >> 22 ;
            decode[ decodeOffset++ ] = val5 >> 22 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 22 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 12 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 2 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 24 ) >> 22 ) | ( val1 >> 30 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 14 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 26 ) >> 22 ) | ( val2 >> 28 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 16 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 28 ) >> 22 ) | ( val3 >> 26 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 18 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 8 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 30 ) >> 22 ) | ( val4 >> 24 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 20 ) >> 22 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 10 ) >> 22 ;
        return offSet;
    }

    static int fastDeCompressFor11Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet ++ ];
            uint32_t val2 = encodedValue[ offSet ++ ];
            uint32_t val3 = encodedValue[ offSet ++ ];
            uint32_t val4 = encodedValue[ offSet ++ ];
            uint32_t val5 = encodedValue[ offSet ++ ];
            uint32_t val6 = encodedValue[ offSet ++ ];
            uint32_t val7 = encodedValue[ offSet ++ ];
            uint32_t val8 = encodedValue[ offSet ++ ];
            uint32_t val9 = encodedValue[ offSet ++ ];
            uint32_t val10 = encodedValue[ offSet ++ ];
            uint32_t val11 = encodedValue[ offSet ++ ];
            decode[ decodeOffset++ ] = ( val1 << 21 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val1 << 10 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val2 << 31) >> 21 ) | ( val1 >> 22 ) ;
            decode[ decodeOffset++ ] = ( val2 << 20 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val2 << 9 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val3 << 30 ) >> 21 ) | ( val2 >> 23 ) ;
            decode[ decodeOffset++ ] = ( val3 << 19 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val3 << 8 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val4 << 29 ) >> 21 ) | ( val3 >> 24 ) ;
            decode[ decodeOffset++ ] = ( val4 << 18 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val4 << 7 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val5 << 28 ) >> 21 ) | ( val4 >> 25 ) ;
            decode[ decodeOffset++ ] = ( val5 << 17 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val5 << 6 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val6 << 27 ) >> 21 ) | ( val5 >> 26 ) ;
            decode[ decodeOffset++ ] = ( val6 << 16 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val6 << 5 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val7 << 26 ) >> 21 ) | ( val6 >> 27 ) ;
            decode[ decodeOffset++ ] = ( val7 << 15 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val7 << 4 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val8 << 25 ) >> 21 ) | ( val7 >> 28 ) ;
            decode[ decodeOffset++ ] = ( val8 << 14 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val8 << 3 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val9 << 24 ) >> 21 ) | ( val8 >> 29 ) ;
            decode[ decodeOffset++ ] = ( val9 << 13 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val9 << 2 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val10 << 23 ) >> 21 ) | ( val9 >> 30 ) ;
            decode[ decodeOffset++ ] = ( val10 << 12 ) >> 21 ;
            decode[ decodeOffset++ ] = ( val10 << 1 ) >> 21 ;
            decode[ decodeOffset++ ] = ( ( val11 << 22 ) >> 21 ) | ( val10 >> 31 ) ;
            decode[ decodeOffset++ ] = ( val11 << 11 ) >> 21 ;
            decode[ decodeOffset++ ] = val11 >> 21;
        }

        if ( rest == 0 )
            return offSet;;

        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 21 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 10 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 31) >> 21 ) | ( val1 >> 22 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 20 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 9 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 30 ) >> 21 ) | ( val2 >> 23 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 19 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 8 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 29 ) >> 21 ) | ( val3 >> 24 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 18 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 7 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 28 ) >> 21 ) | ( val4 >> 25 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 17 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 6 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 27 ) >> 21 ) | ( val5 >> 26 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 16 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 5 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 26 ) >> 21 ) | ( val6 >> 27 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 15 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 4 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 25 ) >> 21 ) | ( val7 >> 28 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 14 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 3 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 24 ) >> 21 ) | ( val8 >> 29 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 13 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 2 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 23 ) >> 21 ) | ( val9 >> 30 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 12 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 1 ) >> 21 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 22 ) >> 21 ) | ( val10 >> 31 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 11 ) >> 21 ;
        return offSet;
    }

    static int fastDeCompressFor12Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 3;
        int rest = dataNum % 8;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 20 ) >> 20;
            decode[ decodeOffset++ ] = ( val1 << 8 ) >> 20;
            decode[ decodeOffset++ ] = ( ( val2 << 28 ) >> 20 ) | ( val1 >> 24 );
            decode[ decodeOffset++ ] = ( val2 << 16 ) >> 20;
            decode[ decodeOffset++ ] = ( val2 << 4 ) >> 20;
            decode[ decodeOffset++ ] = ( ( val3 << 24 ) >> 20 ) | ( val2 >> 28 );
            decode[ decodeOffset++ ] = ( val3 << 12 ) >> 20;
            decode[ decodeOffset++ ] = val3 >> 20;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 20 ) >> 20;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 8 ) >> 20;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 28 ) >> 20 ) | ( val1 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 16 ) >> 20;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 20;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 24 ) >> 20 ) | ( val2 >> 28 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 12 ) >> 20;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = val3 >> 20;
        return offSet;
    }

    static int fastDeCompressFor13Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 19 ) >> 19;
            decode[ decodeOffset++ ] = ( val1 << 6 ) >> 19;
            decode[ decodeOffset++ ] = ( ( val2 << 25 ) >> 19 ) | ( val1 >> 26 );
            decode[ decodeOffset++ ] = ( val2 << 12 ) >> 19;
            decode[ decodeOffset++ ] = ( ( val3 << 31 ) >> 19 ) | ( val2 >> 20 );
            decode[ decodeOffset++ ] = ( val3 << 18 ) >> 19 ;
            decode[ decodeOffset++ ] = ( val3 << 5 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val4 << 24 ) >> 19 ) | ( val3 >> 27 );
            decode[ decodeOffset++ ] = ( val4 << 11 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val5 << 30 ) >> 19 ) | ( val4 >> 21 );
            decode[ decodeOffset++ ] = ( val5 << 17 ) >> 19 ;
            decode[ decodeOffset++ ] = ( val5 << 4 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val6 << 23 ) >> 19 ) | ( val5 >> 28 );
            decode[ decodeOffset++ ] = ( val6 << 10 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val7 << 29 ) >> 19 ) | ( val6 >> 22 );
            decode[ decodeOffset++ ] = ( val7 << 16 ) >> 19 ;
            decode[ decodeOffset++ ] = ( val7 << 3 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val8 << 22 ) >> 19 ) | ( val7 >> 29 );
            decode[ decodeOffset++ ] = ( val8 << 9 ) >> 19 ;
            decode[ decodeOffset++ ] = (( val9 << 28 ) >> 19 ) | ( val8 >> 23 );
            decode[ decodeOffset++ ] = ( val9 << 15 ) >> 19 ;
            decode[ decodeOffset++ ] = ( val9 << 2 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val10 << 21 ) >> 19 ) | ( val9 >> 30 );
            decode[ decodeOffset++ ] = ( val10 << 8 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val11 << 27 ) >> 19 ) | ( val10 >> 24 );
            decode[ decodeOffset++ ] = ( val11 << 14 ) >> 19 ;
            decode[ decodeOffset++ ] = ( val11 << 1 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val12 << 20 ) >> 19 ) | ( val11 >> 31 );
            decode[ decodeOffset++ ] = ( val12 << 7 ) >> 19 ;
            decode[ decodeOffset++ ] = ( ( val13 << 26 ) >> 19 ) | ( val12 >> 25 );
            decode[ decodeOffset++ ] = ( val13 << 13 ) >> 19 ;
            decode[ decodeOffset++ ] = val13 >> 19;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 19 ) >> 19;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 6 ) >> 19;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 25 ) >> 19 ) | ( val1 >> 26 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 12 ) >> 19;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 31 ) >> 19 ) | ( val2 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 18 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 5 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 24 ) >> 19 ) | ( val3 >> 27 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 11 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 30 ) >> 19 ) | ( val4 >> 21 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 17 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 4 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 23 ) >> 19 ) | ( val5 >> 28 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 10 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 29 ) >> 19 ) | ( val6 >> 22 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 16 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 3 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 22 ) >> 19 ) | ( val7 >> 29 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 9 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val9 << 28 ) >> 19 ) | ( val8 >> 23 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 15 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 2 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 21 ) >> 19 ) | ( val9 >> 30 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 8 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 27 ) >> 19 ) | ( val10 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 14 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 1 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 20 ) >> 19 ) | ( val11 >> 31 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val12 << 7 ) >> 19 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 26 ) >> 19 ) | ( val12 >> 25 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val13 << 13 ) >> 19 ;
        return offSet;
    }

    static int fastDeCompressFor14Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 18 ) >> 18;
            decode[ decodeOffset++ ] = ( val1 << 4 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val2 << 22 ) >> 18 ) | ( val1 >> 28 );
            decode[ decodeOffset++ ] = ( val2 << 8 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val3 << 26 ) >> 18 ) | ( val2 >> 24 );
            decode[ decodeOffset++ ] = ( val3 << 12 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val4 << 30 ) >> 18 ) | ( val3 >> 20 );
            decode[ decodeOffset++ ] = ( val4 << 16 ) >> 18;
            decode[ decodeOffset++ ] = ( val4 << 2 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val5 << 20 ) >> 18 ) | ( val4 >> 30 );
            decode[ decodeOffset++ ] = ( val5 << 6 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val6 << 24 ) >> 18 ) | ( val5 >> 26 );
            decode[ decodeOffset++ ] = ( val6 << 10 ) >> 18;
            decode[ decodeOffset++ ] = ( ( val7 << 28 ) >> 18 ) | ( val6 >> 22 );
            decode[ decodeOffset++ ] = ( val7 << 14 ) >> 18;
            decode[ decodeOffset++ ] = val7 >> 18;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 18 ) >> 18;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 4 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 22 ) >> 18 ) | ( val1 >> 28 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 8 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 26 ) >> 18 ) | ( val2 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 12 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 30 ) >> 18 ) | ( val3 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 16 ) >> 18;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 2 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 20 ) >> 18 ) | ( val4 >> 30 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 6 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 24 ) >> 18 ) | ( val5 >> 26 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 10 ) >> 18;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 28 ) >> 18 ) | ( val6 >> 22 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 14 ) >> 18;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = val7 >> 18;
        return offSet;
    }

    static int fastDeCompressFor15Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 17 ) >> 17;
            decode[ decodeOffset++ ] = ( val1 << 2 ) >> 17;
            decode[ decodeOffset++ ] = (( val2 << 19 ) >> 17 ) | ( val1 >> 30 );
            decode[ decodeOffset++ ] = ( val2 << 4 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val3 << 21 ) >> 17 ) | ( val2 >> 28 ) ;
            decode[ decodeOffset++ ] = ( val3 << 6 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val4 << 23 ) >> 17 ) | ( val3 >> 26 );
            decode[ decodeOffset++ ] = ( val4 << 8 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val5 << 25 ) >> 17 ) | ( val4 >> 24 );
            decode[ decodeOffset++ ] = ( val5 << 10 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val6 << 27 ) >> 17 ) | ( val5 >> 22 );
            decode[ decodeOffset++ ] = ( val6 << 12 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val7 << 29 ) >> 17 ) | ( val6 >> 20 );
            decode[ decodeOffset++ ] = ( val7 << 14 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val8 << 31 ) >> 17 ) | ( val7 >> 18 );
            decode[ decodeOffset++ ] = ( val8 << 16 ) >> 17;
            decode[ decodeOffset++ ] = ( val8 << 1 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val9 << 18 ) >> 17 ) | ( val8 >> 31 );
            decode[ decodeOffset++ ] = ( val9 << 3 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val10 << 20 ) >> 17 ) | ( val9 >> 29 );
            decode[ decodeOffset++ ] = ( val10 << 5 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val11 << 22 ) >> 17 ) | ( val10 >> 27 );
            decode[ decodeOffset++ ] = ( val11 << 7 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val12 << 24 ) >> 17 ) | ( val11 >> 25 );
            decode[ decodeOffset++ ] = ( val12 << 9 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val13 << 26 ) >> 17 ) | ( val12 >> 23 );
            decode[ decodeOffset++ ] = ( val13 << 11 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val14 << 28 ) >> 17 ) | ( val13 >> 21 );
            decode[ decodeOffset++ ] = ( val14 << 13 ) >> 17;
            decode[ decodeOffset++ ] = ( ( val15 << 30 ) >> 17 ) | ( val14 >> 19 );
            decode[ decodeOffset++ ] = ( val15 << 15 ) >> 17;
            decode[ decodeOffset++ ] = val15 >> 17 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 17 ) >> 17;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val1 << 2 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val2 << 19 ) >> 17 ) | ( val1 >> 30 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 21 ) >> 17 ) | ( val2 >> 28 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 23 ) >> 17 ) | ( val3 >> 26 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 8 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 25 ) >> 17 ) | ( val4 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 10 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 27 ) >> 17 ) | ( val5 >> 22 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 12 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 29 ) >> 17 ) | ( val6 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 14 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 31 ) >> 17 ) | ( val7 >> 18 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 16 ) >> 17;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 1 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 18 ) >> 17 ) | ( val8 >> 31 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 3 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 20 ) >> 17 ) | ( val9 >> 29 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 5 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 22 ) >> 17 ) | ( val10 >> 27 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 7 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 24 ) >> 17 ) | ( val11 >> 25 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val12 << 9 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 26 ) >> 17 ) | ( val12 >> 23 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val13 << 11 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val14 << 28 ) >> 17 ) | ( val13 >> 21 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val14 << 13 ) >> 17;
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val15 << 30 ) >> 17 ) | ( val14 >> 19 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val15 << 15 ) >> 17;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = val15 >> 17 ;
        return offSet;
    }

    static int fastDeCompressFor16Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {

        int maxBlocks = dataNum >> 1;
        int rest = dataNum % 2;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            decode[ decodeOffset ++ ] = ( val1 << 16 ) >> 16;
            decode[ decodeOffset ++ ] = ( val1 >> 16 ) ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 16 ) >> 16;
        return offSet;
    }

    static int fastDeCompressFor17Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 15 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val2 << 30 ) >> 15 ) | ( val1 >> 17 );
            decode[ decodeOffset++ ] = ( val2 << 13 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val3 << 28 ) >> 15 ) | ( val2 >> 19 );
            decode[ decodeOffset++ ] = ( val3 << 11 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val4 << 26 ) >> 15 ) | ( val3 >> 21 );
            decode[ decodeOffset++ ] = ( val4 << 9 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val5 << 24 ) >> 15 ) | ( val4 >> 23 );
            decode[ decodeOffset++ ] = ( val5 << 7 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val6 << 22 ) >> 15 ) | ( val5 >> 25 );
            decode[ decodeOffset++ ] = ( val6 << 5 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val7 << 20 ) >> 15 ) | ( val6 >> 27 );
            decode[ decodeOffset++ ] = ( val7 << 3 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val8 << 18 ) >> 15 ) | ( val7 >> 29 );
            decode[ decodeOffset++ ] = ( val8 << 1 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val9 << 16 ) >> 15 ) | ( val8 >> 31 );
            decode[ decodeOffset++ ] = ( ( val10 << 31 ) >> 15 ) | ( val9 >> 16 );
            decode[ decodeOffset++ ] = ( val10 << 14 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val11 << 29 ) >> 15 ) | ( val10 >> 18 );
            decode[ decodeOffset++ ] = ( val11 << 12 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val12 << 27 ) >> 15 ) | ( val11 >> 20 );
            decode[ decodeOffset++ ] = ( val12 << 10 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val13 << 25 ) >> 15 ) | ( val12 >> 22 );
            decode[ decodeOffset++ ] = ( val13 << 8 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val14 << 23 ) >> 15 ) | ( val13 >> 24 );
            decode[ decodeOffset++ ] = ( val14 << 6 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val15 << 21 ) >> 15 ) | ( val14 >> 26 );
            decode[ decodeOffset++ ] = ( val15 << 4 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val16 << 19 ) >> 15 ) | ( val15 >> 28 );
            decode[ decodeOffset++ ] = ( val16 >> 2 ) >> 15 ;
            decode[ decodeOffset++ ] = ( ( val17 << 17 ) >> 15 ) | ( val16 >> 30 );
            decode[ decodeOffset++ ] = val17 >> 15 ;
        }

        if ( rest == 0 )
            return offSet;

        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 15 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 30 ) >> 15 ) | ( val1 >> 17 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 13 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 28 ) >> 15 ) | ( val2 >> 19 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 11 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 26 ) >> 15 ) | ( val3 >> 21 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 9 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 24 ) >> 15 ) | ( val4 >> 23 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 7 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 22 ) >> 15 ) | ( val5 >> 25 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 5 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 20 ) >> 15 ) | ( val6 >> 27 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 3 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 18 ) >> 15 ) | ( val7 >> 29 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 1 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 16 ) >> 15 ) | ( val8 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 31 ) >> 15 ) | ( val9 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 14 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 29 ) >> 15 ) | ( val10 >> 18 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 12 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 27 ) >> 15 ) | ( val11 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val12 << 10 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 25 ) >> 15 ) | ( val12 >> 22 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val13 << 8 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val14 << 23 ) >> 15 ) | ( val13 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val14 << 6 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val15 << 21 ) >> 15 ) | ( val14 >> 26 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val15 << 4 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val16 << 19 ) >> 15 ) | ( val15 >> 28 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val16 >> 2 ) >> 15 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val17 << 17 ) >> 15 ) | ( val16 >> 30 );
        return offSet;
    }

    static int fastDeCompressFor18Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet ++ ];
            uint32_t val2 = encodedValue[ offSet ++ ];
            uint32_t val3 = encodedValue[ offSet ++ ];
            uint32_t val4 = encodedValue[ offSet ++ ];
            uint32_t val5 = encodedValue[ offSet ++ ];
            uint32_t val6 = encodedValue[ offSet ++ ];
            uint32_t val7 = encodedValue[ offSet ++ ];
            uint32_t val8 = encodedValue[ offSet ++ ];
            uint32_t val9 = encodedValue[ offSet ++ ];
            decode[ decodeOffset ++ ] = (val1 << 14 ) >> 14;
            decode[ decodeOffset ++ ] = ( ( val2 << 28 ) >> 14 ) | ( val1 >> 18 ) ;
            decode[ decodeOffset ++ ] = ( val2 << 10 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val3 << 24 ) >> 14 ) | ( val2 >> 22 ) ;
            decode[ decodeOffset ++ ] = ( val3 << 6 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val4 << 20 ) >> 14 ) | ( val3 >> 26 ) ;
            decode[ decodeOffset ++ ] = ( val4 << 2 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val5 << 16 ) >> 14 ) | ( val4 >> 30 ) ;
            decode[ decodeOffset ++ ] = ( ( val6 << 30 ) >> 14 ) | ( val5 >> 16 ) ;
            decode[ decodeOffset ++ ] = ( val6 << 12 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val7 << 26 ) >> 14 ) | ( val6 >> 20 ) ;
            decode[ decodeOffset ++ ] = ( val7 << 8 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val8 << 22 ) >> 14 ) | ( val7 >> 24 ) ;
            decode[ decodeOffset ++ ] = ( val8 << 4 ) >> 14 ;
            decode[ decodeOffset ++ ] = ( ( val9 << 18 ) >> 14 ) | ( val8 >> 28 ) ;
            decode[ decodeOffset ++ ] = val9 >> 14;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = (val1 << 14 ) >> 14;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 28 ) >> 14 ) | ( val1 >> 18 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 10 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 24 ) >> 14 ) | ( val2 >> 22 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 6 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 20 ) >> 14 ) | ( val3 >> 26 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 2 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 16 ) >> 14 ) | ( val4 >> 30 ) ;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 30 ) >> 14 ) | ( val5 >> 16 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 12 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 26 ) >> 14 ) | ( val6 >> 20 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val7 << 8 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 22 ) >> 14 ) | ( val7 >> 24 ) ;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 4 ) >> 14 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 18 ) >> 14 ) | ( val8 >> 28 ) ;
        return offSet;
    }

    static int fastDeCompressFor19Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 13) >> 13;
            decode[ decodeOffset++ ] = ( ( val2 << 26 ) >> 13 ) | ( val1 >> 19 );
            decode[ decodeOffset++ ] = ( val2 << 7 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val3 << 20 ) >> 13 ) | ( val2 >> 25 );
            decode[ decodeOffset++ ] = ( val3 << 1 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val4 << 14 ) >> 13 ) | ( val3 >> 31 );
            decode[ decodeOffset++ ] = ( ( val5 << 27 ) >> 13 ) | ( val4 >> 18 );
            decode[ decodeOffset++ ] = ( val5 << 8 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val6 << 8 ) >> 21 ) | ( val5 >> 24 );
            decode[ decodeOffset++ ] = ( val6 << 2 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val7 << 15 ) >> 13 ) | ( val6 >> 30 );
            decode[ decodeOffset++ ] = ( ( val8 << 28 ) >> 13 ) | ( val7 >> 17 );
            decode[ decodeOffset++ ] = ( val8 << 9 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val9 << 22 ) >> 13 ) | ( val8 >> 23 );
            decode[ decodeOffset++ ] = ( val9 << 3 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val10 << 16 ) >> 13 ) | ( val9 >> 29 );
            decode[ decodeOffset++ ] = ( ( val11 << 29 ) >> 13 ) | ( val10 >> 16 );
            decode[ decodeOffset++ ] = ( val11 << 10 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val12 << 23 ) >> 13 ) | ( val11 >> 22 );
            decode[ decodeOffset++ ] = ( val12 << 4 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val13 << 17 ) >> 13 ) | ( val12 >> 28 );
            decode[ decodeOffset++ ] = ( ( val14 << 30 ) >> 13 ) | ( val13 >> 15 );
            decode[ decodeOffset++ ] = ( val14 << 11 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val15 << 24 ) >> 13 ) | ( val14 >> 21 );
            decode[ decodeOffset++ ] = ( val15 << 5 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val16 << 18 ) >> 13 ) | ( val15 >> 27 );
            decode[ decodeOffset++ ] = ( ( val17 << 31 ) >> 13 ) | ( val16 >> 14 );
            decode[ decodeOffset++ ] = ( val17 << 12 ) >> 13;
            decode[ decodeOffset++ ] = ( ( val18 << 25 ) >> 13 ) | ( val17 >> 20 );
            decode[ decodeOffset++ ] = ( val18 << 6 ) >> 13;
            decode[ decodeOffset++ ] = (( val19 << 19 ) >> 13 ) | ( val18 >> 26 );
            decode[ decodeOffset++ ] = val19 >> 13 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 13) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 26 ) >> 13 ) | ( val1 >> 19 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 7 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 20 ) >> 13 ) | ( val2 >> 25 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 1 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 14 ) >> 13 ) | ( val3 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 27 ) >> 13 ) | ( val4 >> 18 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 8 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 8 ) >> 21 ) | ( val5 >> 24 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 2 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 15 ) >> 13 ) | ( val6 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 28 ) >> 13 ) | ( val7 >> 17 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 9 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 22 ) >> 13 ) | ( val8 >> 23 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 3 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 16 ) >> 13 ) | ( val9 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 29 ) >> 13 ) | ( val10 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 10 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 23 ) >> 13 ) | ( val11 >> 22 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val12 << 4 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 17 ) >> 13 ) | ( val12 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val14 << 30 ) >> 13 ) | ( val13 >> 15 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val14 << 11 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val15 << 24 ) >> 13 ) | ( val14 >> 21 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val15 << 5 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val16 << 18 ) >> 13 ) | ( val15 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val17 << 31 ) >> 13 ) | ( val16 >> 14 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val17 << 12 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val18 << 25 ) >> 13 ) | ( val17 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val18 << 6 ) >> 13;
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val19 << 19 ) >> 13 ) | ( val18 >> 26 );
        return offSet;
    }

    static int fastDeCompressFor20Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 3;
        int rest = dataNum % 8;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 12 ) >> 12 ;
            decode[ decodeOffset++ ] = ( ( val2 << 24 ) >> 12 ) | ( val1 >> 20 );
            decode[ decodeOffset++ ] = ( val2 << 4 ) >> 12 ;
            decode[ decodeOffset++ ] = ( ( val3 << 16 ) >> 12 ) | ( val2 >> 28 );
            decode[ decodeOffset++ ] = ( ( val4 << 28 ) >> 12 ) | ( val3 >> 16 );
            decode[ decodeOffset++ ] = ( val4 << 8 ) >> 12 ;
            decode[ decodeOffset++ ] = ( ( val5 << 20 ) >> 12 ) | ( val4 >> 24 );
            decode[ decodeOffset++ ] = val5 >> 12;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 12 ) >> 12 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 24 ) >> 12 ) | ( val1 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val2 << 4 ) >> 12 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 16 ) >> 12 ) | ( val2 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 28 ) >> 12 ) | ( val3 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 8 ) >> 12 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 20 ) >> 12 ) | ( val4 >> 24 );
        return offSet;
    }

    static int fastDeCompressFor21Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];

            decode[ decodeOffset++ ] = ( val1 << 11 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val2 << 22 ) >> 11 ) | ( val1 >> 21 );
            decode[ decodeOffset++ ] = ( val2 << 1 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val3 << 12 ) >> 11 ) | ( val2 >> 31 );
            decode[ decodeOffset++ ] = ( ( val4 << 23 ) >> 11 ) | ( val3 >> 20 );
            decode[ decodeOffset++ ] = ( val4 << 2 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val5 << 13 ) >> 11 ) | ( val4 >> 30 );
            decode[ decodeOffset++ ] = ( ( val6 << 24 ) >> 11 ) | ( val5 >> 19 );
            decode[ decodeOffset++ ] = ( val6 << 3 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val7 << 14 ) >> 11 ) | ( val6 >> 29 );
            decode[ decodeOffset++ ] = ( ( val8 << 25 ) >> 11 ) | ( val7 >> 18 );
            decode[ decodeOffset++ ] = ( val8 << 4 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val9 << 15 ) >> 11 ) | ( val8 >> 28 );
            decode[ decodeOffset++ ] = ( ( val10 << 26 ) >> 11 ) | ( val9 >> 17 );
            decode[ decodeOffset++ ] = ( val10 << 5 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val11 << 16 ) >> 11 ) | ( val10 >> 27 );
            decode[ decodeOffset++ ] = ( ( val12 << 27 ) >> 11 ) | ( val11 >> 16 );
            decode[ decodeOffset++ ] = ( val12 << 6 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val13 << 17 ) >> 11 ) | ( val12 >> 26 );
            decode[ decodeOffset++ ] = ( ( val14 << 28 ) >> 11 ) | ( val13 >> 15 );
            decode[ decodeOffset++ ] = ( val14 << 7 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val15 << 18 ) >> 11 ) | ( val14 >> 25 );
            decode[ decodeOffset++ ] = ( ( val16 << 29 ) >> 11 ) | ( val15 >> 14 );
            decode[ decodeOffset++ ] = ( val16 << 8 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val17 << 19 ) >> 11 ) | ( val16 >> 24 );
            decode[ decodeOffset++ ] = ( ( val18 << 30 ) >> 11 ) | ( val17 >> 13 );
            decode[ decodeOffset++ ] = ( val18 << 9 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val19 << 20 ) >> 11 ) | ( val18 >> 23 );
            decode[ decodeOffset++ ] = ( ( val20 << 31 ) >> 11 ) | ( val19 >> 12 );
            decode[ decodeOffset++ ] = ( val20 << 10 ) >> 11;
            decode[ decodeOffset++ ] = ( ( val21 << 21 ) >> 11 ) | ( val20 >> 22 );
            decode[ decodeOffset++ ] = val21 >> 11;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 11 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 22 ) >> 11 ) | ( val1 >> 21 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( val2 << 1 ) >> 11;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( ( val3 << 12 ) >> 11 ) | ( val2 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 23 ) >> 11 ) | ( val3 >> 20 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 2 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 13 ) >> 11 ) | ( val4 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 24 ) >> 11 ) | ( val5 >> 19 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 3 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 14 ) >> 11 ) | ( val6 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 25 ) >> 11 ) | ( val7 >> 18 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 4 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 15 ) >> 11 ) | ( val8 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 26 ) >> 11 ) | ( val9 >> 17 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 5 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 16 ) >> 11 ) | ( val10 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 27 ) >> 11 ) | ( val11 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val12 << 6 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 17 ) >> 11 ) | ( val12 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val14 << 28 ) >> 11 ) | ( val13 >> 15 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val14 << 7 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val15 << 18 ) >> 11 ) | ( val14 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val16 << 29 ) >> 11 ) | ( val15 >> 14 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val16 << 8 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val17 << 19 ) >> 11 ) | ( val16 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val18 << 30 ) >> 11 ) | ( val17 >> 13 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val18 << 9 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val19 << 20 ) >> 11 ) | ( val18 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val20 << 31 ) >> 11 ) | ( val19 >> 12 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val20 << 10 ) >> 11;
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val21 << 21 ) >> 11 ) | ( val20 >> 22 );
        return offSet;
    }

    static int fastDeCompressFor22Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 10 ) >> 10;
            decode[ decodeOffset++ ] = ( ( val2 << 20 ) >> 10 ) | ( val1 >> 22 );
            decode[ decodeOffset++ ] = ( ( val3 << 30 ) >> 10 ) | ( val2 >> 12 );
            decode[ decodeOffset++ ] = ( val3 << 8 ) >> 10;
            decode[ decodeOffset++ ] = ( ( val4 << 18 ) >> 10 ) | ( val3 >> 24 );
            decode[ decodeOffset++ ] = ( ( val5 << 28 ) >> 10 ) | ( val4 >> 14 );
            decode[ decodeOffset++ ] = ( val5 << 6 ) >> 10;
            decode[ decodeOffset++ ] = ( ( val6 << 16 ) >> 10 ) | ( val5 >> 26 );
            decode[ decodeOffset++ ] = ( ( val7 << 26 ) >> 10 ) | ( val6 >> 16 );
            decode[ decodeOffset++ ] = ( val7 << 4 ) >> 10;
            decode[ decodeOffset++ ] = ( ( val8 << 14 ) >> 10 ) | ( val7 >> 28 );
            decode[ decodeOffset++ ] = ( ( val9 << 24 ) >> 10 ) | ( val8 >> 18 );
            decode[ decodeOffset++ ] = ( val9 << 2 ) >> 10;
            decode[ decodeOffset++ ] = ( ( val10 << 12 ) >> 10 ) | ( val9 >> 30 );
            decode[ decodeOffset++ ] = ( ( val11 << 22 ) >> 10 ) | ( val10 >> 20 );
            decode[ decodeOffset++ ] = val11 >> 10;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( val1 << 10 ) >> 10;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val2 << 20 ) >> 10 ) | ( val1 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val3 << 30 ) >> 10 ) | ( val2 >> 12 );
        if ( --rest == 0 ) return offSet;
        decode[ decodeOffset++ ] = ( val3 << 8 ) >> 10;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val4 << 18 ) >> 10 ) | ( val3 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val5 << 28 ) >> 10 ) | ( val4 >> 14 );
        if ( --rest == 0 ) return offSet;
        decode[ decodeOffset++ ] = ( val5 << 6 ) >> 10;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val6 << 16 ) >> 10 ) | ( val5 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val7 << 26 ) >> 10 ) | ( val6 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ decodeOffset++ ] = ( val7 << 4 ) >> 10;
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val8 << 14 ) >> 10 ) | ( val7 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val9 << 24 ) >> 10 ) | ( val8 >> 18 );
        if ( --rest == 0 ) return offSet;
        decode[ decodeOffset++ ] = ( val9 << 2 ) >> 10;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val10 << 12 ) >> 10 ) | ( val9 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val11 << 22 ) >> 10 ) | ( val10 >> 20 );
        return offSet;
    }
    static int fastDeCompressFor23Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];
            uint32_t val22 = encodedValue[ offSet++ ];
            uint32_t val23 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 9 ) >> 9;
            decode[ decodeOffset++ ] = (( val2 << 18 ) >> 9 ) | ( val1 >> 23 );
            decode[ decodeOffset++ ] = (( val3 << 27 ) >> 9 ) | ( val2 >> 14 );
            decode[ decodeOffset++ ] = ( val3 << 4 ) >> 9;
            decode[ decodeOffset++ ] = (( val4 << 13 ) >> 9 ) | ( val3 >> 28 );
            decode[ decodeOffset++ ] = (( val5 << 22 ) >> 9 ) | ( val4 >> 19 );
            decode[ decodeOffset++ ] = (( val6 << 31 ) >> 9 ) | ( val5 >> 10 );
            decode[ decodeOffset++ ] = ( val6 << 8 ) >> 9;
            decode[ decodeOffset++ ] = (( val7 << 17 ) >> 9 ) | ( val6 >> 24 );
            decode[ decodeOffset++ ] = (( val8 << 26 ) >> 9 ) | ( val7 >> 15 );
            decode[ decodeOffset++ ] = ( val8 << 3 ) >> 9;
            decode[ decodeOffset++ ] = (( val9 << 12 ) >> 9 ) | ( val8 >> 29 );
            decode[ decodeOffset++ ] = (( val10 << 21 ) >> 9 ) | ( val9 >> 20 );
            decode[ decodeOffset++ ] = (( val11 << 30 ) >> 9 ) | ( val10 >> 11 );
            decode[ decodeOffset++ ] = ( val11 << 7 ) >> 9;
            decode[ decodeOffset++ ] = (( val12 << 16 ) >> 9 ) | ( val11 >> 25 );
            decode[ decodeOffset++ ] = (( val13 << 25 ) >> 9 ) | ( val12 >> 16 );
            decode[ decodeOffset++ ] = ( val13 << 2 ) >> 9;
            decode[ decodeOffset++ ] = (( val14 << 11 ) >> 9 ) | ( val13 >> 30 );
            decode[ decodeOffset++ ] = (( val15 << 20 ) >> 9 ) | ( val14 >> 21 );
            decode[ decodeOffset++ ] = (( val16 << 29 ) >> 9 ) | ( val15 >> 12 );
            decode[ decodeOffset++ ] = ( val16 << 6 ) >> 9 ;
            decode[ decodeOffset++ ] = (( val17 << 15 ) >> 9 ) | ( val16 >> 26 );
            decode[ decodeOffset++ ] = (( val18 << 24 ) >> 9 ) | ( val17 >> 17 );
            decode[ decodeOffset++ ] = ( val18 << 1 ) >> 9;
            decode[ decodeOffset++ ] = (( val19 << 10 ) >> 9 ) | ( val18 >> 31 );
            decode[ decodeOffset++ ] = (( val20 << 19 ) >> 9 ) | ( val19 >> 22 );
            decode[ decodeOffset++ ] = (( val21 << 28 ) >> 9 ) | ( val20 >> 13 );
            decode[ decodeOffset++ ] = ( val21 << 5 ) >> 9;
            decode[ decodeOffset++ ] = (( val22 << 14 ) >> 9 ) | ( val21 >> 27 );
            decode[ decodeOffset++ ] = (( val23 << 23 ) >> 9 ) | ( val22 >> 18 );
            decode[ decodeOffset++ ] = val23 >> 9;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 9 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val2 << 18 ) >> 9 ) | ( val1 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val3 << 27 ) >> 9 ) | ( val2 >> 14 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val3 << 4 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val4 << 13 ) >> 9 ) | ( val3 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val5 << 22 ) >> 9 ) | ( val4 >> 19 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val6 << 31 ) >> 9 ) | ( val5 >> 10 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 8 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val7 << 17 ) >> 9 ) | ( val6 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val8 << 26 ) >> 9 ) | ( val7 >> 15 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 3 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val9 << 12 ) >> 9 ) | ( val8 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val10 << 21 ) >> 9 ) | ( val9 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val11 << 30 ) >> 9 ) | ( val10 >> 11 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 7 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val12 << 16 ) >> 9 ) | ( val11 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val13 << 25 ) >> 9 ) | ( val12 >> 16 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val13 << 2 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val14 << 11 ) >> 9 ) | ( val13 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val15 << 20 ) >> 9 ) | ( val14 >> 21 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val16 << 29 ) >> 9 ) | ( val15 >> 12 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val16 << 6 ) >> 9 ;
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val17 << 15 ) >> 9 ) | ( val16 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val18 << 24 ) >> 9 ) | ( val17 >> 17 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val18 << 1 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val19 << 10 ) >> 9 ) | ( val18 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val20 << 19 ) >> 9 ) | ( val19 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val21 << 28 ) >> 9 ) | ( val20 >> 13 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val21 << 5 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val22 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val22 << 14 ) >> 9 ) | ( val21 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val23 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val23 << 23 ) >> 9 ) | ( val22 >> 18 );
        return offSet;
    }

    static int fastDeCompressFor24Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 2;
        int rest = dataNum % 4;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 8 ) >> 8;
            decode[ decodeOffset++ ] = ( ( val2 << 16 ) >> 8 ) | ( val1 >> 24 );
            decode[ decodeOffset++ ] = ( ( val3 << 24 ) >> 10 ) | ( val2 >> 16 );
            decode[ decodeOffset++ ] = val3 >> 8;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 8 ) >> 8;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 16 ) >> 8 ) | ( val1 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 24 ) >> 10 ) | ( val2 >> 16 );
        return offSet;
    }
    static int fastDeCompressFor25Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];
            uint32_t val22 = encodedValue[ offSet++ ];
            uint32_t val23 = encodedValue[ offSet++ ];
            uint32_t val24 = encodedValue[ offSet++ ];
            uint32_t val25 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 7 ) >> 7;
            decode[ decodeOffset++ ] = (( val2 << 14 ) >> 7 ) | ( val1 >> 25 );
            decode[ decodeOffset++ ] = (( val3 << 21 ) >> 7 ) | ( val2 >> 18 );
            decode[ decodeOffset++ ] = (( val4 << 28 ) >> 7 ) | ( val3 >> 11 );
            decode[ decodeOffset++ ] = ( val4 << 3 ) >> 9;
            decode[ decodeOffset++ ] = (( val5 << 10 ) >> 7 ) | ( val4 >> 29 );
            decode[ decodeOffset++ ] = (( val6 << 17 ) >> 7 ) | ( val5 >> 22 );
            decode[ decodeOffset++ ] = (( val7 << 24 ) >> 7 ) | ( val6 >> 15 );
            decode[ decodeOffset++ ] = (( val8 << 31 ) >> 7 ) | ( val7 >> 8 );
            decode[ decodeOffset++ ] = ( val8 << 6 ) >> 7;
            decode[ decodeOffset++ ] = (( val9 << 13 ) >> 7 ) | ( val8 >> 26 );
            decode[ decodeOffset++ ] = (( val10 << 20 ) >> 7 ) | ( val9 >> 19 );
            decode[ decodeOffset++ ] = (( val11 << 27 ) >> 7 ) | ( val10 >> 12 );
            decode[ decodeOffset++ ] = ( val11 << 2 ) >> 7;
            decode[ decodeOffset++ ] = (( val12 << 9 ) >> 7 ) | ( val11 >> 30 );
            decode[ decodeOffset++ ] = (( val13 << 16 ) >> 7 ) | ( val12 >> 23 );
            decode[ decodeOffset++ ] = (( val14 << 23 ) >> 7 ) | ( val13 >> 16 );
            decode[ decodeOffset++ ] = (( val15 << 30 ) >> 7 ) | ( val14 >> 9 );
            decode[ decodeOffset++ ] = (( val15 << 5 ) >> 7 );
            decode[ decodeOffset++ ] = (( val16 << 12 ) >> 7 ) | ( val15 >> 27 );
            decode[ decodeOffset++ ] = (( val17 << 19 ) >> 7 ) | ( val16 >> 20 );
            decode[ decodeOffset++ ] = (( val18 << 26 ) >> 7 ) | ( val17 >> 13 );
            decode[ decodeOffset++ ] = ( val18 << 1 ) >> 7;
            decode[ decodeOffset++ ] = (( val19 << 8 ) >> 7 ) | ( val18 >> 31 );
            decode[ decodeOffset++ ] = (( val20 << 15 ) >> 7 ) | ( val19 >> 24 );
            decode[ decodeOffset++ ] = (( val21 << 22 ) >> 7 ) | ( val20 >> 17 );
            decode[ decodeOffset++ ] = (( val22 << 29 ) >> 7 ) | ( val21 >> 10 );
            decode[ decodeOffset++ ] = ( val22 << 4 ) >> 7;
            decode[ decodeOffset++ ] = (( val23 << 11 ) >> 7 ) | ( val22 >> 28 );
            decode[ decodeOffset++ ] = (( val24 << 18 ) >> 7 ) | ( val23 >> 21 );
            decode[ decodeOffset++ ] = (( val25 << 25 ) >> 7 ) | ( val24 >> 14 );
            decode[ decodeOffset++ ] = val25 >> 7;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 7 ) >> 7;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val2 << 14 ) >> 7 ) | ( val1 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val3 << 21 ) >> 7 ) | ( val2 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val4 << 28 ) >> 7 ) | ( val3 >> 11 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val4 << 3 ) >> 9;
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val5 << 10 ) >> 7 ) | ( val4 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val6 << 17 ) >> 7 ) | ( val5 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val7 << 24 ) >> 7 ) | ( val6 >> 15 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val8 << 31 ) >> 7 ) | ( val7 >> 8 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val8 << 6 ) >> 7;
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val9 << 13 ) >> 7 ) | ( val8 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val10 << 20 ) >> 7 ) | ( val9 >> 19 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val11 << 27 ) >> 7 ) | ( val10 >> 12 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 2 ) >> 7;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val12 << 9 ) >> 7 ) | ( val11 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val13 << 16 ) >> 7 ) | ( val12 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val14 << 23 ) >> 7 ) | ( val13 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val15 << 30 ) >> 7 ) | ( val14 >> 9 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = (( val15 << 5 ) >> 7 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val16 << 12 ) >> 7 ) | ( val15 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val17 << 19 ) >> 7 ) | ( val16 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val18 << 26 ) >> 7 ) | ( val17 >> 13 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val18 << 1 ) >> 7;
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val19 << 8 ) >> 7 ) | ( val18 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val20 << 15 ) >> 7 ) | ( val19 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val21 << 22 ) >> 7 ) | ( val20 >> 17 );
        if ( --rest == 0 ) return offSet;
        uint32_t val22 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val22 << 29 ) >> 7 ) | ( val21 >> 10 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val22 << 4 ) >> 7;
        if ( --rest == 0 ) return offSet;
        uint32_t val23 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val23 << 11 ) >> 7 ) | ( val22 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val24 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val24 << 18 ) >> 7 ) | ( val23 >> 21 );
        if ( --rest == 0 ) return offSet;
        uint32_t val25 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val25 << 25 ) >> 7 ) | ( val24 >> 14 );
        return offSet;
    }

    static int fastDeCompressFor26Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 4;
        int rest = dataNum % 16;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 6 ) >> 6;
            decode[ decodeOffset++ ] = ( ( val2 << 12 ) >> 6 ) | ( val1 >> 26 );
            decode[ decodeOffset++ ] = ( ( val3 << 18 ) >> 6 ) | ( val2 >> 20 );
            decode[ decodeOffset++ ] = ( ( val4 << 24 ) >> 6 ) | ( val3 >> 14 );
            decode[ decodeOffset++ ] = ( ( val5 << 30 ) >> 6 ) | ( val4 >> 8 );
            decode[ decodeOffset++ ] = ( val5 << 4 ) >> 6;
            decode[ decodeOffset++ ] = ( ( val6 << 10 ) >> 6 ) | ( val5 >> 28 );
            decode[ decodeOffset++ ] = ( ( val7 << 16 ) >> 6 ) | ( val6 >> 22 );
            decode[ decodeOffset++ ] = ( ( val8 << 22 ) >> 6 ) | ( val7 >> 16 );
            decode[ decodeOffset++ ] = ( ( val9 << 28 ) >> 6 ) | ( val8 >> 10 );
            decode[ decodeOffset++ ] = ( val9 << 2 ) >> 6;
            decode[ decodeOffset++ ] = ( ( val10 << 8 ) >> 6 ) | ( val9 >> 30 );
            decode[ decodeOffset++ ] = ( ( val11 << 14 ) >> 6 ) | ( val10 >> 24 );
            decode[ decodeOffset++ ] = ( ( val12 << 20 ) >> 6 ) | ( val11 >> 18 );
            decode[ decodeOffset++ ] = ( ( val13 << 26 ) >> 6 ) | ( val12 >> 12 );
            decode[ decodeOffset++ ] = val13 >> 6;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 6 ) >> 6;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 12 ) >> 6 ) | ( val1 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 18 ) >> 6 ) | ( val2 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 24 ) >> 6 ) | ( val3 >> 14 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 30 ) >> 6 ) | ( val4 >> 8 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val5 << 4 ) >> 6;
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 10 ) >> 6 ) | ( val5 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 16 ) >> 6 ) | ( val6 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val8 << 22 ) >> 6 ) | ( val7 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val9 << 28 ) >> 6 ) | ( val8 >> 10 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val9 << 2 ) >> 6;
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val10 << 8 ) >> 6 ) | ( val9 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val11 << 14 ) >> 6 ) | ( val10 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val12 << 20 ) >> 6 ) | ( val11 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val13 << 26 ) >> 6 ) | ( val12 >> 12 );
        return offSet;
    }
    static int fastDeCompressFor27Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];
            uint32_t val22 = encodedValue[ offSet++ ];
            uint32_t val23 = encodedValue[ offSet++ ];
            uint32_t val24 = encodedValue[ offSet++ ];
            uint32_t val25 = encodedValue[ offSet++ ];
            uint32_t val26 = encodedValue[ offSet++ ];
            uint32_t val27 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 5 ) >> 5;
            decode[ decodeOffset++ ] = (( val2 << 10 ) >> 5 ) | ( val1 >> 27 );
            decode[ decodeOffset++ ] = (( val3 << 15 ) >> 5 ) | ( val2 >> 22 );
            decode[ decodeOffset++ ] = (( val4 << 20 ) >> 5 ) | ( val3 >> 17 );
            decode[ decodeOffset++ ] = (( val5 << 25 ) >> 5 ) | ( val4 >> 12 );
            decode[ decodeOffset++ ] = (( val6 << 30 ) >> 5 ) | ( val5 >> 7 );
            decode[ decodeOffset++ ] = ( val6 << 3 ) >> 5;
            decode[ decodeOffset++ ] = (( val7 << 8 ) >> 5 ) | ( val6 >> 29 );
            decode[ decodeOffset++ ] = (( val8 << 13 ) >> 5 ) | ( val7 >> 24 );
            decode[ decodeOffset++ ] = (( val9 << 18 ) >> 5 ) | ( val8 >> 19 );
            decode[ decodeOffset++ ] = (( val10 << 23 ) >> 5 ) | ( val9 >> 14 );
            decode[ decodeOffset++ ] = (( val11 << 28 ) >> 5 ) | ( val10 >> 9 );
            decode[ decodeOffset++ ] = ( val11 << 1 ) >> 5;
            decode[ decodeOffset++ ] = (( val12 << 6 ) >> 5 ) | ( val11 >> 31 );
            decode[ decodeOffset++ ] = (( val13 << 11 ) >> 5 ) | ( val12 >> 25 );
            decode[ decodeOffset++ ] = (( val14 << 16 ) >> 5 ) | ( val13 >> 20 );
            decode[ decodeOffset++ ] = (( val15 << 21 ) >> 5 ) | ( val14 >> 15 );
            decode[ decodeOffset++ ] = (( val16 << 26 ) >> 5 ) | ( val15 >> 10 );
            decode[ decodeOffset++ ] = (( val17 << 31 ) >> 5 ) | ( val16 >> 5 );
            decode[ decodeOffset++ ] = ( val18 << 4 ) >> 5;
            decode[ decodeOffset++ ] = (( val18 << 9 ) >> 5 ) | ( val17 >> 28 );
            decode[ decodeOffset++ ] = (( val19 << 14 ) >> 5 ) | ( val18 >> 23 );
            decode[ decodeOffset++ ] = (( val20 << 19 ) >> 5 ) | ( val19 >> 18 );
            decode[ decodeOffset++ ] = (( val21 << 24 ) >> 5 ) | ( val20 >> 13 );
            decode[ decodeOffset++ ] = (( val22 << 29 ) >> 5 ) | ( val21 >> 8 );
            decode[ decodeOffset++ ] = ( val22 << 2 ) >> 5;
            decode[ decodeOffset++ ] = (( val23 << 7 ) >> 5 ) | ( val22 >> 30 );
            decode[ decodeOffset++ ] = (( val24 << 12 ) >> 5 ) | ( val23 >> 25 );
            decode[ decodeOffset++ ] = (( val25 << 17 ) >> 5 ) | ( val24 >> 20 );
            decode[ decodeOffset++ ] = (( val26 << 22 ) >> 5 ) | ( val25 >> 15 );
            decode[ decodeOffset++ ] = (( val27 << 27 ) >> 5 ) | ( val26 >> 10 );
            decode[ decodeOffset++ ] = val27 >> 5;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 5 ) >> 5;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val2 << 10 ) >> 5 ) | ( val1 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val3 << 15 ) >> 5 ) | ( val2 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val4 << 20 ) >> 5 ) | ( val3 >> 17 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val5 << 25 ) >> 5 ) | ( val4 >> 12 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val6 << 30 ) >> 5 ) | ( val5 >> 7 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val6 << 3 ) >> 5;
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val7 << 8 ) >> 5 ) | ( val6 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val8 << 13 ) >> 5 ) | ( val7 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val9 << 18 ) >> 5 ) | ( val8 >> 19 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val10 << 23 ) >> 5 ) | ( val9 >> 14 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val11 << 28 ) >> 5 ) | ( val10 >> 9 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val11 << 1 ) >> 5;
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val12 << 6 ) >> 5 ) | ( val11 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val13 << 11 ) >> 5 ) | ( val12 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val14 << 16 ) >> 5 ) | ( val13 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val15 << 21 ) >> 5 ) | ( val14 >> 15 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val16 << 26 ) >> 5 ) | ( val15 >> 10 );
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val17 << 31 ) >> 5 ) | ( val16 >> 5 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( val18 << 4 ) >> 5;
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = (( val18 << 9 ) >> 5 ) | ( val17 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val19 << 14 ) >> 5 ) | ( val18 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val20 << 19 ) >> 5 ) | ( val19 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val21 << 24 ) >> 5 ) | ( val20 >> 13 );
        if ( --rest == 0 ) return offSet;
        uint32_t val22 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val22 << 29 ) >> 5 ) | ( val21 >> 8 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val22 << 2 ) >> 5;
        if ( --rest == 0 ) return offSet;
        uint32_t val23 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val23 << 7 ) >> 5 ) | ( val22 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val24 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val24 << 12 ) >> 5 ) | ( val23 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val25 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val25 << 17 ) >> 5 ) | ( val24 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val26 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val26 << 22 ) >> 5 ) | ( val25 >> 15 );
        if ( --rest == 0 ) return offSet;
        uint32_t val27 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val27 << 27 ) >> 5 ) | ( val26 >> 10 );
        return offSet;
    }
    static int fastDeCompressFor28Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 3;
        int rest = dataNum % 8;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 4 ) >> 4;
            decode[ decodeOffset++ ] = ( ( val2 << 8 ) >> 4 ) | ( val1 >> 28 );
            decode[ decodeOffset++ ] = ( ( val3 << 12 ) >> 4 ) | ( val2 >> 24 );
            decode[ decodeOffset++ ] = ( ( val4 << 16 ) >> 4 ) | ( val3 >> 20 );
            decode[ decodeOffset++ ] = ( ( val5 << 20 ) >> 4 ) | ( val4 >> 16 );
            decode[ decodeOffset++ ] = ( ( val6 << 24 ) >> 4 ) | ( val5 >> 12 );
            decode[ decodeOffset++ ] = ( ( val7 << 28 ) >> 4 ) | ( val6 >> 8 );
            decode[ decodeOffset++ ] = val7 >> 4 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 4 ) >> 4;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val2 << 8 ) >> 4 ) | ( val1 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val3 << 12 ) >> 4 ) | ( val2 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val4 << 16 ) >> 4 ) | ( val3 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val5 << 20 ) >> 4 ) | ( val4 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val6 << 24 ) >> 4 ) | ( val5 >> 12 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = ( ( val7 << 28 ) >> 4 ) | ( val6 >> 8 );
        return offSet;
    }
    static int fastDeCompressFor29Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];
            uint32_t val22 = encodedValue[ offSet++ ];
            uint32_t val23 = encodedValue[ offSet++ ];
            uint32_t val24 = encodedValue[ offSet++ ];
            uint32_t val25 = encodedValue[ offSet++ ];
            uint32_t val26 = encodedValue[ offSet++ ];
            uint32_t val27 = encodedValue[ offSet++ ];
            uint32_t val28 = encodedValue[ offSet++ ];
            uint32_t val29 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 3 ) >> 3;
            decode[ decodeOffset++ ] = (( val2 << 6 ) >> 3 ) | ( val1 >> 29 );
            decode[ decodeOffset++ ] = (( val3 << 9 ) >> 3 ) | ( val2 >> 26 );
            decode[ decodeOffset++ ] = (( val4 << 12 ) >> 3 ) | ( val3 >> 23 );
            decode[ decodeOffset++ ] = (( val5 << 15 ) >> 3 ) | ( val4 >> 20 );
            decode[ decodeOffset++ ] = (( val6 << 18 ) >> 3 ) | ( val5 >> 17 );
            decode[ decodeOffset++ ] = (( val7 << 21 ) >> 3 ) | ( val6 >> 14 );
            decode[ decodeOffset++ ] = (( val8 << 24 ) >> 3 ) | ( val7 >> 11 );
            decode[ decodeOffset++ ] = (( val9 << 27 ) >> 3 ) | ( val8 >> 8 );
            decode[ decodeOffset++ ] = (( val10 << 30 ) >> 3 ) | ( val9 >> 5 );
            decode[ decodeOffset++ ] = ( val10 << 1 ) >> 3;
            decode[ decodeOffset++ ] = (( val11 << 4 ) >> 3 ) | ( val10 >> 31 );
            decode[ decodeOffset++ ] = (( val12 << 7 ) >> 3 ) | ( val11 >> 28 );
            decode[ decodeOffset++ ] = (( val13 << 10 ) >> 3 ) | ( val12 >> 25 );
            decode[ decodeOffset++ ] = (( val14 << 13 ) >> 3 ) | ( val13 >> 22 );
            decode[ decodeOffset++ ] = (( val15 << 16 ) >> 3 ) | ( val14 >> 19 );
            decode[ decodeOffset++ ] = (( val16 << 19 ) >> 3 ) | ( val15 >> 16 );
            decode[ decodeOffset++ ] = (( val17 << 22 ) >> 3 ) | ( val16 >> 13 );
            decode[ decodeOffset++ ] = (( val18 << 25 ) >> 3 ) | ( val17 >> 10 );
            decode[ decodeOffset++ ] = (( val19 << 28 ) >> 3 ) | ( val18 >> 7 );
            decode[ decodeOffset++ ] = (( val20 << 31 ) >> 3 ) | ( val19 >> 4 );
            decode[ decodeOffset++ ] = ( val20 << 2 ) >> 3;
            decode[ decodeOffset++ ] = (( val21 << 5 ) >> 3 ) | ( val20 >> 30 );
            decode[ decodeOffset++ ] = (( val22 << 8 ) >> 3 ) | ( val21 >> 27 );
            decode[ decodeOffset++ ] = (( val23 << 11 ) >> 3 ) | ( val22 >> 24 );
            decode[ decodeOffset++ ] = (( val24 << 14 ) >> 3 ) | ( val23 >> 21 );
            decode[ decodeOffset++ ] = (( val25 << 17 ) >> 3 ) | ( val24 >> 18 );
            decode[ decodeOffset++ ] = (( val26 << 20 ) >> 3 ) | ( val25 >> 15 );
            decode[ decodeOffset++ ] = (( val27 << 23 ) >> 3 ) | ( val26 >> 12 );
            decode[ decodeOffset++ ] = (( val28 << 26 ) >> 3 ) | ( val27 >> 9 );
            decode[ decodeOffset++ ] = (( val29 << 29 ) >> 3 ) | ( val28 >> 6 );
            decode[ decodeOffset++ ] = val29 >> 3;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset ] = ( val1 << 3 ) >> 3;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val2 << 6 ) >> 3 ) | ( val1 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val3 << 9 ) >> 3 ) | ( val2 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val4 << 12 ) >> 3 ) | ( val3 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val5 << 15 ) >> 3 ) | ( val4 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val6 << 18 ) >> 3 ) | ( val5 >> 17 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val7 << 21 ) >> 3 ) | ( val6 >> 14 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val8 << 24 ) >> 3 ) | ( val7 >> 11 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val9 << 27 ) >> 3 ) | ( val8 >> 8 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val10 << 30 ) >> 3 ) | ( val9 >> 5 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val10 << 1 ) >> 3;
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val11 << 4 ) >> 3 ) | ( val10 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val12 << 7 ) >> 3 ) | ( val11 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val13 << 10 ) >> 3 ) | ( val12 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val14 << 13 ) >> 3 ) | ( val13 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val15 << 16 ) >> 3 ) | ( val14 >> 19 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val16 << 19 ) >> 3 ) | ( val15 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val17 << 22 ) >> 3 ) | ( val16 >> 13 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val18 << 25 ) >> 3 ) | ( val17 >> 10 );
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val19 << 28 ) >> 3 ) | ( val18 >> 7 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val20 << 31 ) >> 3 ) | ( val19 >> 4 );
        if ( --rest == 0 ) return offSet;
        decode[ ++decodeOffset ] = ( val20 << 2 ) >> 3;
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val21 << 5 ) >> 3 ) | ( val20 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val22 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val22 << 8 ) >> 3 ) | ( val21 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val23 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val23 << 11 ) >> 3 ) | ( val22 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val24 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val24 << 14 ) >> 3 ) | ( val23 >> 21 );
        if ( --rest == 0 ) return offSet;
        uint32_t val25 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val25 << 17 ) >> 3 ) | ( val24 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val26 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val26 << 20 ) >> 3 ) | ( val25 >> 15 );
        if ( --rest == 0 ) return offSet;
        uint32_t val27 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val27 << 23 ) >> 3 ) | ( val26 >> 12 );
        if ( --rest == 0 ) return offSet;
        uint32_t val28 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val28 << 26 ) >> 3 ) | ( val27 >> 9 );
        if ( --rest == 0 ) return offSet;
        uint32_t val29 = encodedValue[ offSet++ ];
        decode[ ++decodeOffset ] = (( val29 << 29 ) >> 3 ) | ( val28 >> 6 );
        return offSet;
    }

    static int fastDeCompressFor30Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 3;
        int rest = dataNum % 8;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 2 ) >> 2;
            decode[ decodeOffset++ ] = ( ( val2 << 4 ) >> 2 ) | ( val1 >> 30 );
            decode[ decodeOffset++ ] = ( ( val3 << 6 ) >> 2 ) | ( val2 >> 28 );
            decode[ decodeOffset++ ] = ( ( val4 << 8 ) >> 2 ) | ( val3 >> 26 );
            decode[ decodeOffset++ ] = ( ( val5 << 10 ) >> 2 ) | ( val4 >> 24 );
            decode[ decodeOffset++ ] = ( ( val6 << 12 ) >> 2 ) | ( val5 >> 22 );
            decode[ decodeOffset++ ] = ( ( val7 << 14 ) >> 2 ) | ( val6 >> 20 );
            decode[ decodeOffset++ ] = ( ( val8 << 16 ) >> 2 ) | ( val7 >> 18 );
            decode[ decodeOffset++ ] = ( ( val9 << 18 ) >> 2 ) | ( val8 >> 16 );
            decode[ decodeOffset++ ] = ( ( val10 << 20 ) >> 2 ) | ( val9 >> 14 );
            decode[ decodeOffset++ ] = ( ( val11 << 22 ) >> 2 ) | ( val10 >> 12 );
            decode[ decodeOffset++ ] = ( ( val12 << 24 ) >> 2 ) | ( val11 >> 10 );
            decode[ decodeOffset++ ] = ( ( val13 << 26 ) >> 2 ) | ( val12 >> 8 );
            decode[ decodeOffset++ ] = ( ( val14 << 28 ) >> 2 ) | ( val13 >> 8 );
            decode[ decodeOffset++ ] = ( ( val15 << 30 ) >> 2 ) | ( val14 >> 8 );
            decode[ decodeOffset++ ] = val15 >> 2 ;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( val1 << 2 ) >> 2;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val2 << 4 ) >> 2 ) | ( val1 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val3 << 6 ) >> 2 ) | ( val2 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val4 << 8 ) >> 2 ) | ( val3 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val5 << 10 ) >> 2 ) | ( val4 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val6 << 12 ) >> 2 ) | ( val5 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val7 << 14 ) >> 2 ) | ( val6 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val8 << 16 ) >> 2 ) | ( val7 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val9 << 18 ) >> 2 ) | ( val8 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val10 << 20 ) >> 2 ) | ( val9 >> 14 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val11 << 22 ) >> 2 ) | ( val10 >> 12 );
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val12 << 24 ) >> 2 ) | ( val11 >> 10 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val13 << 26 ) >> 2 ) | ( val12 >> 8 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val14 << 28 ) >> 2 ) | ( val13 >> 8 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( ( val15 << 30 ) >> 2 ) | ( val14 >> 8 );
        return offSet;
    }
    static int fastDeCompressFor31Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum >> 5;
        int rest = dataNum % 32;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
        {
            uint32_t val1 = encodedValue[ offSet++ ];
            uint32_t val2 = encodedValue[ offSet++ ];
            uint32_t val3 = encodedValue[ offSet++ ];
            uint32_t val4 = encodedValue[ offSet++ ];
            uint32_t val5 = encodedValue[ offSet++ ];
            uint32_t val6 = encodedValue[ offSet++ ];
            uint32_t val7 = encodedValue[ offSet++ ];
            uint32_t val8 = encodedValue[ offSet++ ];
            uint32_t val9 = encodedValue[ offSet++ ];
            uint32_t val10 = encodedValue[ offSet++ ];
            uint32_t val11 = encodedValue[ offSet++ ];
            uint32_t val12 = encodedValue[ offSet++ ];
            uint32_t val13 = encodedValue[ offSet++ ];
            uint32_t val14 = encodedValue[ offSet++ ];
            uint32_t val15 = encodedValue[ offSet++ ];
            uint32_t val16 = encodedValue[ offSet++ ];
            uint32_t val17 = encodedValue[ offSet++ ];
            uint32_t val18 = encodedValue[ offSet++ ];
            uint32_t val19 = encodedValue[ offSet++ ];
            uint32_t val20 = encodedValue[ offSet++ ];
            uint32_t val21 = encodedValue[ offSet++ ];
            uint32_t val22 = encodedValue[ offSet++ ];
            uint32_t val23 = encodedValue[ offSet++ ];
            uint32_t val24 = encodedValue[ offSet++ ];
            uint32_t val25 = encodedValue[ offSet++ ];
            uint32_t val26 = encodedValue[ offSet++ ];
            uint32_t val27 = encodedValue[ offSet++ ];
            uint32_t val28 = encodedValue[ offSet++ ];
            uint32_t val29 = encodedValue[ offSet++ ];
            uint32_t val30 = encodedValue[ offSet++ ];
            uint32_t val31 = encodedValue[ offSet++ ];
            decode[ decodeOffset++ ] = ( val1 << 1 ) >> 1;
            decode[ decodeOffset++ ] = (( val2 << 2 ) >> 1 ) | ( val1 >> 31 );
            decode[ decodeOffset++ ] = (( val3 << 3 ) >> 1 ) | ( val2 >> 30 );
            decode[ decodeOffset++ ] = (( val4 << 4 ) >> 1 ) | ( val3 >> 29 );
            decode[ decodeOffset++ ] = (( val5 << 5 ) >> 1 ) | ( val4 >> 28 );
            decode[ decodeOffset++ ] = (( val6 << 6 ) >> 1 ) | ( val5 >> 27 );
            decode[ decodeOffset++ ] = (( val7 << 7 ) >> 1 ) | ( val6 >> 26 );
            decode[ decodeOffset++ ] = (( val8 << 8 ) >> 1 ) | ( val7 >> 25 );
            decode[ decodeOffset++ ] = (( val9 << 9 ) >> 1 ) | ( val8 >> 24 );
            decode[ decodeOffset++ ] = (( val10 << 10 ) >> 1 ) | ( val9 >> 23 );
            decode[ decodeOffset++ ] = (( val11 << 11 ) >> 1 ) | ( val10 >> 22 );
            decode[ decodeOffset++ ] = (( val12 << 12 ) >> 1 ) | ( val11 >> 21 );
            decode[ decodeOffset++ ] = (( val13 << 13 ) >> 1 ) | ( val12 >> 20 );
            decode[ decodeOffset++ ] = (( val14 << 14 ) >> 1 ) | ( val13 >> 19 );
            decode[ decodeOffset++ ] = (( val15 << 15 ) >> 1 ) | ( val14 >> 18 );
            decode[ decodeOffset++ ] = (( val16 << 16 ) >> 1 ) | ( val15 >> 17 );
            decode[ decodeOffset++ ] = (( val17 << 17 ) >> 1 ) | ( val16 >> 16 );
            decode[ decodeOffset++ ] = (( val18 << 18 ) >> 1 ) | ( val17 >> 15 );
            decode[ decodeOffset++ ] = (( val19 << 19 ) >> 1 ) | ( val18 >> 14 );
            decode[ decodeOffset++ ] = (( val20 << 20 ) >> 1 ) | ( val19 >> 13 );
            decode[ decodeOffset++ ] = (( val21 << 21 ) >> 1 ) | ( val20 >> 12 );
            decode[ decodeOffset++ ] = (( val22 << 22 ) >> 1 ) | ( val21 >> 11 );
            decode[ decodeOffset++ ] = (( val23 << 23 ) >> 1 ) | ( val22 >> 10 );
            decode[ decodeOffset++ ] = (( val24 << 24 ) >> 1 ) | ( val23 >> 9 );
            decode[ decodeOffset++ ] = (( val25 << 25 ) >> 1 ) | ( val24 >> 8 );
            decode[ decodeOffset++ ] = (( val26 << 26 ) >> 1 ) | ( val25 >> 7 );
            decode[ decodeOffset++ ] = (( val27 << 27 ) >> 1 ) | ( val26 >> 6 );
            decode[ decodeOffset++ ] = (( val28 << 28 ) >> 1 ) | ( val27 >> 5 );
            decode[ decodeOffset++ ] = (( val29 << 29 ) >> 1 ) | ( val28 >> 4 );
            decode[ decodeOffset++ ] = (( val30 << 30 ) >> 1 ) | ( val28 >> 3 );
            decode[ decodeOffset++ ] = (( val31 << 31 ) >> 1 ) | ( val28 >> 2 );
            decode[ decodeOffset++ ] = val31 >> 1;
        }
        if ( rest == 0 )
            return offSet;
        uint32_t val1 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = ( val1 << 1 ) >> 1;
        if ( --rest == 0 ) return offSet;
        uint32_t val2 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val2 << 2 ) >> 1 ) | ( val1 >> 31 );
        if ( --rest == 0 ) return offSet;
        uint32_t val3 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val3 << 3 ) >> 1 ) | ( val2 >> 30 );
        if ( --rest == 0 ) return offSet;
        uint32_t val4 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val4 << 4 ) >> 1 ) | ( val3 >> 29 );
        if ( --rest == 0 ) return offSet;
        uint32_t val5 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val5 << 5 ) >> 1 ) | ( val4 >> 28 );
        if ( --rest == 0 ) return offSet;
        uint32_t val6 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val6 << 6 ) >> 1 ) | ( val5 >> 27 );
        if ( --rest == 0 ) return offSet;
        uint32_t val7 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val7 << 7 ) >> 1 ) | ( val6 >> 26 );
        if ( --rest == 0 ) return offSet;
        uint32_t val8 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val8 << 8 ) >> 1 ) | ( val7 >> 25 );
        if ( --rest == 0 ) return offSet;
        uint32_t val9 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val9 << 9 ) >> 1 ) | ( val8 >> 24 );
        if ( --rest == 0 ) return offSet;
        uint32_t val10 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val10 << 10 ) >> 1 ) | ( val9 >> 23 );
        if ( --rest == 0 ) return offSet;
        uint32_t val11 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val11 << 11 ) >> 1 ) | ( val10 >> 22 );
        if ( --rest == 0 ) return offSet;
        uint32_t val12 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val12 << 12 ) >> 1 ) | ( val11 >> 21 );
        if ( --rest == 0 ) return offSet;
        uint32_t val13 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val13 << 13 ) >> 1 ) | ( val12 >> 20 );
        if ( --rest == 0 ) return offSet;
        uint32_t val14 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val14 << 14 ) >> 1 ) | ( val13 >> 19 );
        if ( --rest == 0 ) return offSet;
        uint32_t val15 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val15 << 15 ) >> 1 ) | ( val14 >> 18 );
        if ( --rest == 0 ) return offSet;
        uint32_t val16 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val16 << 16 ) >> 1 ) | ( val15 >> 17 );
        if ( --rest == 0 ) return offSet;
        uint32_t val17 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val17 << 17 ) >> 1 ) | ( val16 >> 16 );
        if ( --rest == 0 ) return offSet;
        uint32_t val18 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val18 << 18 ) >> 1 ) | ( val17 >> 15 );
        if ( --rest == 0 ) return offSet;
        uint32_t val19 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val19 << 19 ) >> 1 ) | ( val18 >> 14 );
        if ( --rest == 0 ) return offSet;
        uint32_t val20 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val20 << 20 ) >> 1 ) | ( val19 >> 13 );
        if ( --rest == 0 ) return offSet;
        uint32_t val21 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val21 << 21 ) >> 1 ) | ( val20 >> 12 );
        if ( --rest == 0 ) return offSet;
        uint32_t val22 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val22 << 22 ) >> 1 ) | ( val21 >> 11 );
        if ( --rest == 0 ) return offSet;
        uint32_t val23 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val23 << 23 ) >> 1 ) | ( val22 >> 10 );
        if ( --rest == 0 ) return offSet;
        uint32_t val24 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val24 << 24 ) >> 1 ) | ( val23 >> 9 );
        if ( --rest == 0 ) return offSet;
        uint32_t val25 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val25 << 25 ) >> 1 ) | ( val24 >> 8 );
        if ( --rest == 0 ) return offSet;
        uint32_t val26 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val26 << 26 ) >> 1 ) | ( val25 >> 7 );
        if ( --rest == 0 ) return offSet;
        uint32_t val27 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val27 << 27 ) >> 1 ) | ( val26 >> 6 );
        if ( --rest == 0 ) return offSet;
        uint32_t val28 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val28 << 28 ) >> 1 ) | ( val27 >> 5 );
        if ( --rest == 0 ) return offSet;
        uint32_t val29 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val29 << 29 ) >> 1 ) | ( val28 >> 4 );
        if ( --rest == 0 ) return offSet;
        uint32_t val30 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val30 << 30 ) >> 1 ) | ( val28 >> 3 );
        if ( --rest == 0 ) return offSet;
        uint32_t val31 = encodedValue[ offSet++ ];
        decode[ decodeOffset++ ] = (( val31 << 31 ) >> 1 ) | ( val28 >> 2 );
        return offSet;
    }

    static int fastDeCompressFor32Bit( int offSet, uint32_t* encodedValue, int dataNum, int decodeOffset, uint32_t* decode )
    {
        int maxBlocks = dataNum;
        // block process
        for ( int block = 0 ; block < maxBlocks ; block++ )
            decode[ decodeOffset++ ] = encodedValue[ offSet++ ];
        return offSet;
    }
};

}}}

#endif

