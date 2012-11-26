#ifndef _IZENELIB_IR_COMMONSET_SIMD_TOOLBOX_
#define _IZENELIB_IR_COMMONSET_SIMD_TOOLBOX_

#include <xmmintrin.h>
#include <emmintrin.h>

namespace izenelib
{
namespace ir
{
namespace commonset
{

class SIMDToolBox
{
public:

    SIMDToolBox( unsigned int nregisters, unsigned int nbuffer = 1024*1024 ) :
        bitmap_masks_128bits_(NULL),
        bitmap_masks_32bits_(NULL),
        hitsptr_data_( NULL ),
        hitsptr_start_( NULL ),
        hitsptr_( NULL ),
        buffer_int_( NULL ),
        zero_( _mm_setzero_si128() ),
        nregisters_(nregisters),
        nbytes_(nregisters/32),
        nbuffer_(nbuffer)
    {
        bitmap_masks_128bits_ = new __m128i[nregisters_];

        unsigned int* intmasks = new unsigned int[nregisters_/32];

        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte ) intmasks[ibyte] = 0;

        for( unsigned int ibyte = 0, imask = 0 ; ibyte < nbytes_ ; ++ibyte )
        {
            for( int ibit = 31 ; ibit >= 0 ; --ibit, ++imask )
            {
                intmasks[ibyte] = 1;
                intmasks[ibyte] <<= ibit;
                bitmap_masks_128bits_[ imask ] = _mm_set_epi32( intmasks[3], intmasks[2], intmasks[1], intmasks[0] );
                intmasks[ibyte] = 0;
                //std::cout << "mask " << imask << " : " << toString(bitmap_masks_[imask]) << "\n";
            }
        }

        delete[] intmasks;

        bitmap_masks_32bits_ = new __m128i[32];

        unsigned int intmask;

        for( int ibit = 31, imask = 0 ; ibit >= 0 ; --ibit, ++imask )
        {
            intmask = 1;
            intmask <<= ibit;
            bitmap_masks_32bits_[ imask ] = _mm_set_epi32( intmask, intmask, intmask, intmask );
        }

        hitsptr_data_ = new unsigned int[ nregisters_ ];

        hitsptr_ = new unsigned int*[ nbytes_ ];

        hitsptr_start_ = new unsigned int*[ nbytes_ ];

        for( unsigned int ibyte = 0, offset = 0 ; ibyte < nbytes_ ; ++ibyte, offset+=32 )
        {
            hitsptr_[ibyte] = hitsptr_start_[ibyte] = hitsptr_data_ + offset;
        }

        buffer_int_ = new unsigned int[ nbuffer_ ];

    }

    ~SIMDToolBox()
    {
        delete[] bitmap_masks_128bits_;
        delete[] bitmap_masks_32bits_;
        delete[] hitsptr_data_;
        delete[] hitsptr_;
        delete[] hitsptr_start_;
        delete[] buffer_int_;
    }

    unsigned int getNRegisters()
    {
        return nregisters_;
    }

    unsigned int getNBytes()
    {
        return nbytes_;
    }

    bool areEqual( __m128i& x, __m128i& y )
    {
//std::cout << " a = " << toString(x) << "\n";
//std::cout << " b = " << toString(y) << "\n";
//__m128i aeqb = _mm_cmpeq_epi32(x,y);
//std::cout << " aeqb = " << toString(aeqb) << "\n";
//int i = _mm_movemask_epi8(_mm_cmpeq_epi32(x,y));
//std::cout << " movemask = " << i << "\n";

        __m128i xx = _mm_load_si128( &x );
        __m128i yy = _mm_load_si128( &y );
        return ( _mm_movemask_epi8(_mm_cmpeq_epi32(xx,yy)) == 65535 );
    }

    void turnOnBit( __m128i* x, unsigned int ibit )
    {
        unsigned int im128i = ibit / nregisters_;
        unsigned int iregister = ibit % nregisters_;

//std::cout << "\nturn on bit " << ibit << ", im128i = " << im128i << ", iregister = " << iregister << "\n";
//std::cout << "  mask   = " << toString(bitmap_masks_[iregister]) << "\n";
//std::cout << "  before = " << toString(x[im128i]) << "\n";
        //__m128i a = _mm_load_si128( x + im128i );
        //__m128i b = _mm_load_si128( bitmap_masks_128bits_ + iregister );
        //x[im128i] = _mm_or_si128( a, b );
        //_mm_store_si128( x+im128i, ab );

        x[im128i] = _mm_or_si128(x[im128i],bitmap_masks_128bits_[iregister]);

//std::cout << "  after  = " << toString(x[im128i]) << "\n";
//std::cout << "  a  = " << toString(a) << "\n";
//std::cout << "  b  = " << toString(b) << "\n";
//std::cout << "  ab = " << toString(ab) << "\n\n";
    }

    void bitmapAND( const __m128i& a, const __m128i& b, __m128i& a_and_b )
    {
        //__m128i aa = _mm_load_si128( &a );
        //__m128i bb = _mm_load_si128( &b );
        //a_and_b =_mm_and_si128( aa, bb );

        a_and_b =_mm_and_si128( a, b );
    }

    void getIndicesOfNonZeroBits
    (
        __m128i* block_bitmaps_ptr,
        __m128i* block_bitmaps_ptr_end,
        unsigned int*& nonzero_block_indices_ptr,
        unsigned int initial_offset = 0
    )
    {
        unsigned int offset_increment = nregisters_;

        for( unsigned int offset = initial_offset ; block_bitmaps_ptr < block_bitmaps_ptr_end ; ++block_bitmaps_ptr, offset+=offset_increment )
        {
            getIndicesOfNonZeroBits( block_bitmaps_ptr, offset, nonzero_block_indices_ptr );
        }
    }

    void getIndicesOfNonZeroBits( __m128i* bitmap, unsigned int offset, unsigned int*& hits_result_ptr )
    {
        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte ) hitsptr_[ibyte] = hitsptr_start_[ibyte];

        Converter bitmap_and_masks;

//std::cout<< ">>>>>> indices : offset = " << offset << "\n";

        for( register unsigned int ibit = 0, ihit_offset = offset ; ibit < 32 ; ++ibit, ++ihit_offset )
        {
            bitmap_and_masks.i128 = _mm_and_si128( bitmap_masks_32bits_[ibit], *bitmap ); // bitmap AND [ mask, mask, mask, mask ]

//std::cout << "  bitmap   = " << toString(*bitmap) <<"\n";
//std::cout << "  mask     = " << toString(bitmap_masks_32bits_[ibit]) <<"\n";
//std::cout << "  and result   = " << toString(bitmap_and_masks.i128) << "\n";

//std::cout << "ibyte hits are : ";
            for( unsigned int ibyte = 0, shiftoffset = 0 ; ibyte < nbytes_ ; ++ibyte, shiftoffset+=32 )
            {
                if( bitmap_and_masks.i4[ibyte] != 0 )
                {
//std::cout<<ibyte<<" ";
                    *(hitsptr_[ibyte]++) = ihit_offset + shiftoffset;
                }
            }
//std::cout << "\n";
        }

        // write to output in docid numeric order

        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte )
        {
            for( unsigned int* hitsptr = hitsptr_start_[ibyte] ; hitsptr < hitsptr_[ibyte] ; ++hitsptr, ++hits_result_ptr )
            {
                *hits_result_ptr = *hitsptr;
            }
        }

//std::cout << "hits: ";
//for(unsigned int* ptr = start_ptr ; ptr<hits_result_ptr; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n\n";

    }

    void getIndicesOfNonZeroBits( __m128i* bitmap, unsigned int offset, CommonSet<unsigned int>& commonset )
    {
        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte ) hitsptr_[ibyte] = hitsptr_start_[ibyte];

        Converter bitmap_and_masks;

        for( register unsigned int ibit = 0, ihit_offset = offset ; ibit < 32 ; ++ibit, ++ihit_offset )
        {
            bitmap_and_masks.i128 = _mm_and_si128( bitmap_masks_32bits_[ibit], *bitmap ); // bitmap AND [ mask, mask, mask, mask ]

            for( unsigned int ibyte = 0, shiftoffset = 0 ; ibyte < nbytes_ ; ++ibyte, shiftoffset+=32 )
            {
                if( bitmap_and_masks.i4[ibyte] != 0 )
                {
                    *(hitsptr_[ibyte]++) = ihit_offset + shiftoffset;
                }
            }
        }

        // write to output in docid numeric order

        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte )
        {
            for( unsigned int* hitsptr = hitsptr_start_[ibyte] ; hitsptr < hitsptr_[ibyte] ; ++hitsptr )
            {
                commonset.add( hitsptr );
            }
        }

    }

    /*

    inputs:

      nonzero_blocks_ptr  = array of block numbers

      blockbitmap_ptr     = bitmap which contains the nonzero_blocks_ptr blocks as hits, as well as other hits

      docidbitmaps_ptr    = docid bitmaps which are a pair with blockbitmap_ptr

    outputs:

      docidbitmaps_for_nonzero_blocks_ptr = array of pointers to the docid bitmaps corresponding to the nonzero_blocks_ptr block numbers

    */

    void getDocIDBitMapsForNonZeroBlocks
    (
        unsigned int* nonzero_blocks_ptr,
        unsigned int* nonzero_blocks_ptr_end,
        __m128i* blockbitmap_ptr,
        __m128i* blockbitmap_ptr_end,
        unsigned int n_m128i_per_block,
        __m128i* docidbitmaps_ptr,
        __m128i**& docidbitmaps_for_nonzero_blocks_ptr
    )
    {
        unsigned int* nonzero_block_indices_b_ptr = buffer_int_;
        unsigned int* nonzero_block_indices_b_ptr_end = buffer_int_;
        unsigned int index;

        __m128i* docids_ptr;
        __m128i* docids_ptr_end;

        getIndicesOfNonZeroBits( blockbitmap_ptr, blockbitmap_ptr_end, nonzero_block_indices_b_ptr_end );

//std::cout << "\n\n\n\n************ inside docidbitmapfornonzeroblocks*********\n\nblock bitmaps : " << toString( blockbitmap_ptr, blockbitmap_ptr_end ) << "\n\nnonzero indices : ";
//for( unsigned int* ptr = buffer_int_ ; ptr < nonzero_block_indices_b_ptr_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n\nnonzero indices to find : ";
//for( unsigned int* ptr = nonzero_blocks_ptr ; ptr <nonzero_blocks_ptr_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n\n";

        for( unsigned int* block_ptr = nonzero_blocks_ptr ; block_ptr < nonzero_blocks_ptr_end ; ++block_ptr )
        {
//std::cout<<"processing block " << *block_ptr << "\n";

            while( nonzero_block_indices_b_ptr < nonzero_block_indices_b_ptr_end && *nonzero_block_indices_b_ptr < *block_ptr )
            {
                ++nonzero_block_indices_b_ptr;
            }

//std::cout << "out of loop, nonzero block index = " << *nonzero_block_indices_b_ptr << ", nth index in b = " << (int)(nonzero_block_indices_b_ptr - buffer_int_) << "\n";

            if( nonzero_block_indices_b_ptr == nonzero_block_indices_b_ptr_end)
            {
//std::cout << "not found a block\n";
                break;
            }

            if( *nonzero_block_indices_b_ptr == *block_ptr )
            {
                index = (unsigned int) ( nonzero_block_indices_b_ptr - buffer_int_ );
                docids_ptr = docidbitmaps_ptr + index * n_m128i_per_block;
                docids_ptr_end = docids_ptr + n_m128i_per_block;
                for( ; docids_ptr < docids_ptr_end ; ++docids_ptr )
                {
//std::cout<<"add";
                    *(docidbitmaps_for_nonzero_blocks_ptr++) = docids_ptr;
                }

//std::cout << "n m128i per block = " << n_m128i_per_block << ", block index = " << index <<"\n";
//std::cout << "writing docids to array, from " << index * n_m128i_per_block << " to " << index * n_m128i_per_block + n_m128i_per_block << "\n";
//std::cout << "docid bitmaps for this hit = " << toString( docidbitmaps_ptr + index * n_m128i_per_block, docids_ptr_end ) << "\n";
            }
            else
            {
//std::cout << "filling with zeros\n";
                for( unsigned int i = 0 ; i < n_m128i_per_block ; ++i ) *(docidbitmaps_for_nonzero_blocks_ptr++) = &zero_;
            }

//std::cout<<"\n";
        }

//std::cout<<"******** finished docidbitmapfornonzeroblocks **********\n\n\n\n\n";
    }

    std::string toString( __m128i& i )
    {
        Converter converter;
        converter.i128 = i;
        std::stringstream str;
        str << " | ";
        for( unsigned int ibyte = 0 ; ibyte < nbytes_ ; ++ibyte ) str << toString( converter.i4[ibyte] ) << " | ";
        return str.str();
    }

    std::string toString( __m128i* ptr, __m128i* ptr_end, std::string separator = "" )
    {
        std::stringstream str;
        for( unsigned int i = 0 ; ptr < ptr_end ; ++ptr, ++i ) str << "[" << i << "] " << toString( *ptr ) << separator;
        return str.str();
    }

    std::string toString( __m128i** ptr, __m128i** ptr_end, std::string separator = "" )
    {
        std::stringstream str;
        for( unsigned int i = 0 ; ptr < ptr_end ; ++ptr, ++i ) str << "[" << i << "] " << toString( **ptr ) << separator;
        return str.str();
    }

    std::string toString( unsigned int i )
    {
        std::stringstream str;
        unsigned int a;

        for( int shift_r = 31 ; shift_r >= 0 ; --shift_r )
        {
            a = i;
            a >>= shift_r;
            if( (a&0x00000001) == 0x00000001 ) str << '1';
            else str << '0';
            if( shift_r == 31 || shift_r == 0 ) continue;
            if( (31-shift_r) % 8 == 7 ) str << ", ";
        }

        return str.str();
    }

// templetize this to include nregisters_
    union Converter
    {
        __m128i i128;
        unsigned int i4[4];
    };

private:

    __m128i* bitmap_masks_128bits_;

    __m128i* bitmap_masks_32bits_;

    unsigned int* hitsptr_data_;

    unsigned int** hitsptr_start_;

    unsigned int** hitsptr_;

    unsigned int* buffer_int_;

    __m128i zero_;

    unsigned int nregisters_;

    unsigned int nbytes_;

    unsigned int nbuffer_;

};

}
}
}

#endif
