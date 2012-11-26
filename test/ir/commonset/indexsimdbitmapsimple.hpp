#ifndef _SF1R_NEW_INDEX_SIMD_BITMAP_SIMPLE_HPP_
#define _SF1R_NEW_INDEX_SIMD_BITMAP_SIMPLE_HPP_

#include <xmmintrin.h>
#include <emmintrin.h>

#include "indexsimple.hpp"
#include  <ir/commonset/simdtoolbox.hpp>

namespace izenelib
{
namespace ir
{
namespace commonset
{

class IndexSIMDBitMapSimple : public IndexSimple, private SIMDToolBox
{
public:

    IndexSIMDBitMapSimple( unsigned int nblocks, unsigned int nregisters,unsigned int ndocids_total, unsigned int ntokens_cached, unsigned int ntokens_disk, unsigned int ndocuments_common, const std::vector<unsigned int>& ndocids, const std::string& directory_name = "./", bool verbose = false );

    ~IndexSIMDBitMapSimple()
    {
        for( unsigned int i = 0 ; i < ntokens_ ; ++i )
        {
            delete[] docids_m128i_[i];
            delete[] blocks_m128i_[i];
        }
        delete[] docids_m128i_;
        delete[] blocks_m128i_;
        delete[] ndocids_m128i_;
    }

    bool getBitMapBlocks( const unsigned int& tokenid, __m128i*& blocks_ptr, __m128i*& blocks_ptr_end ) const
    {
        if( tokenid >= ntokens_ ) return false;
        blocks_ptr = blocks_m128i_[tokenid];
        blocks_ptr_end = blocks_ptr + n_m128i_in_blocks_bitmap_;
        return true;
    }

    bool getBitMapDocIDs( const unsigned int& tokenid, __m128i*& docid_ptr, __m128i*& docid_ptr_end ) const
    {
        if( tokenid >= ntokens_ ) return false;
        docid_ptr = docids_m128i_[tokenid];
        docid_ptr_end = docid_ptr + ndocids_m128i_[tokenid];
        return true;
    }

    unsigned int getNM128iDocIDsMax()
    {
        return n_m128i_in_docids_bitmap_;
    }

    unsigned int getNM128iDocIDs(unsigned int itoken)
    {
        return itoken < ntokens_ ? ndocids_m128i_[itoken] : 0;
    }

    unsigned int getNBlocks()
    {
        return nblocks_;
    }

    unsigned int getNM128iPerBlock()
    {
        return n_m128i_per_block_;
    }

    unsigned int getNM128iBlocks()
    {
        return n_m128i_in_blocks_bitmap_;
    }

    unsigned int getNRegisters()
    {
        return SIMDToolBox::getNRegisters();
    }

private:

    __m128i** docids_m128i_;

    __m128i** blocks_m128i_;

    unsigned int* ndocids_m128i_;

    __m128i zero_;

    unsigned int nblocks_;

    unsigned int n_m128i_in_blocks_bitmap_;

    unsigned int n_m128i_per_block_;

    unsigned int n_m128i_in_docids_bitmap_;

};

IndexSIMDBitMapSimple::IndexSIMDBitMapSimple( unsigned int nblocks, unsigned int nregisters, unsigned int ndocids_total, unsigned int ntokens_cached, unsigned int ntokens_disk, unsigned int ndocuments_common, const std::vector<unsigned int>& ndocids, const std::string& directory, bool verbose ) :
    IndexSimple(ndocids_total, ntokens_cached, ntokens_disk, ndocuments_common, ndocids, directory, verbose),
    SIMDToolBox( nregisters ),
    docids_m128i_( NULL ),
    blocks_m128i_( NULL ),
    ndocids_m128i_( NULL ),
    zero_( _mm_setzero_si128() ),
    nblocks_( nblocks ),
    n_m128i_in_blocks_bitmap_( nblocks_ % nregisters == 0 ? nblocks_ / nregisters : 1 + nblocks_ / nregisters ),
    n_m128i_per_block_( nunique_docids_ % (nblocks_*nregisters) == 0 ? nunique_docids_ / (nblocks_*nregisters) : 1 + nunique_docids_ / (nblocks_*nregisters) ),
    n_m128i_in_docids_bitmap_( nunique_docids_ % nregisters == 0 ? nunique_docids_ / nregisters : 1 + nunique_docids_ / nregisters )
{
    std::cout << "IndexSIMDBitMapSimple::constructor : creating SIMD inverted lists\n";
    std::cout << "  n registers                         = " << getNRegisters() << "\n";
    std::cout << "  n unique document ids               = " << nunique_docids_ << "\n";
    std::cout << "  n blocks                            = " << nblocks_ << "\n";
    std::cout << "  n m128i variables per block         = " << n_m128i_per_block_ << "\n";
    std::cout << "  n m128i variables in docid bitmap   = " << n_m128i_in_docids_bitmap_ << "\n";



    // convert docid hits to a bitmap with nunique_docids_ bits

    docids_m128i_ = new __m128i*[ ntokens_ ];

    ndocids_m128i_ = new unsigned int[ ntokens_ ];

    for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
    {
        docids_m128i_[itoken] = new __m128i[ n_m128i_in_docids_bitmap_ ];

        for( unsigned int i = 0 ; i < n_m128i_in_docids_bitmap_ ; ++i ) docids_m128i_[itoken][i] = _mm_load_si128( &zero_ );

        for( unsigned int idocid = 0 ; idocid < ndocids_[itoken] ; ++idocid ) turnOnBit( docids_m128i_[itoken], docids_[itoken][idocid] );

        //std::cout << "\ndocids full bitmap : token = " << itoken << "\n";
        //for( unsigned int i = 0 ; i < n_m128i_in_docids_bitmap_ ; ++i ) std::cout << " [" << i << "," << i*128 << "] " << toString( docids_m128i_[itoken][i] ) << "\n";
    }



    // read through docid hits bitmap, removing empty m128i values, and recording that the m128i block is not empty in blocks_m128i_

    blocks_m128i_ = new __m128i*[ntokens_];
    bool block_is_empty;
    unsigned int nnonzero, im128i;

    for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
    {
        blocks_m128i_[itoken] = new __m128i[n_m128i_in_blocks_bitmap_];
        for( unsigned int i = 0 ; i < n_m128i_in_blocks_bitmap_ ; ++i ) blocks_m128i_[itoken][i] = _mm_load_si128( &zero_ );
        ndocids_m128i_[itoken] = nnonzero = im128i = 0;

        for( unsigned int iblock = 0 ; iblock < nblocks_ ; ++iblock, im128i+=n_m128i_per_block_ )
        {
            block_is_empty = true;

            for( unsigned int im128i_in_block = 0 ; im128i_in_block < n_m128i_per_block_ ; ++im128i_in_block )
            {
                if( !areEqual( docids_m128i_[itoken][im128i + im128i_in_block], zero_ ) )
                {
                    block_is_empty = false;
                    break;
                }
            }

//std::cout << "token " << itoken << " : " << im128i << "/" << n_m128i_in_docids_bitmap_ << "\n";

            if( !block_is_empty )
            {
                for( unsigned int im128i_in_block = 0 ; im128i_in_block < n_m128i_per_block_ ; ++im128i_in_block )
                {
//std::cout << "  doing " << im128i << " writing to " << ndocids_m128i_[itoken] << "\n";
                    docids_m128i_[itoken][ ndocids_m128i_[itoken]++ ] = docids_m128i_[itoken][im128i + im128i_in_block];
                }
                ++nnonzero;
                turnOnBit( blocks_m128i_[itoken], iblock );
            }

//std::cout << "done\n";
        }

        std::cout << "\n  token " << itoken << " docid bitmaps final\n";
        std::cout << "    n m128i variables in docid bitmaps = " << ndocids_m128i_[itoken] << "\n";
        std::cout << "    n nonzero blocks                   = " << nnonzero << "/" << nblocks_ << "\n";
        //std::cout << toString( docids_m128i_[itoken], docids_m128i_[itoken] + ndocids_m128i_[itoken] , "\n") <<"\n\n";
    }

    //for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
    {
        //std::cout << "\nblocks bitmap : token = " << itoken << "\n";
        //for( unsigned int i = 0 ; i < n_m128i_in_blocks_bitmap_ ; ++i ) std::cout << " [" << i << "," << i*128 << "] " << toString( blocks_m128i_[itoken][i] ) << "\n";
    }


// test indices function in simd
    /*
         unsigned int* indices = new unsigned int[nblocks_];
         unsigned int* indices_end = indices;

         for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
         {
           std::cout << "\nindices : token = " << itoken << " : " << std::flush;

           indices_end = indices;

           getIndicesOfNonZeroBits( blocks_m128i_[itoken], blocks_m128i_[itoken] + n_m128i_in_blocks_bitmap_, indices_end );

           for( unsigned int* ptr = indices ; ptr < indices_end ; ++ptr ) std::cout << *ptr << " ";

           std::cout<<std::endl;
         }

         delete[] indices;
    */
    std::cout << "\nIndexSIMDBitMapSimple::constructor : done.\n\n";
}

}
}
}

#endif
