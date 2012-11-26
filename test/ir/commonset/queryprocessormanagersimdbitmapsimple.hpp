#ifndef _SF1R_NEW_QUERYPROCESSORMANAGER_SIMD_BITMAP_SIMPLE_HPP_
#define _SF1R_NEW_QUERYPROCESSORMANAGER_SIMD_BITMAP_SIMPLE_HPP_

#include  <ir/commonset/queryprocessormanager.hpp>
#include  <ir/commonset/simdtoolbox.hpp>
#include "indexsimdbitmapsimple.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class QueryProcessorManagerSIMDBitMapSimple : public QueryProcessorManager<DocID,TokenID>, private SIMDToolBox
{
public:

    QueryProcessorManagerSIMDBitMapSimple
    (
        IndexSIMDBitMapSimple& index,
        DocumentProperties<DocID>& documentproperties,
        unsigned int nbuffer = 10*1024*1024
    ) :
        QueryProcessorManager<DocID,TokenID>( dynamic_cast<Index<DocID,TokenID>&>(index), documentproperties),
        SIMDToolBox( index.getNRegisters() ),
        simd_index_( index ),
        buffer0_int_( NULL ),
        buffer1_int_( NULL ),
        buffer0_m128i_( NULL ),
        buffer1_m128i_( NULL ),
        buffer2_m128i_( NULL ),
        nbuffer_( nbuffer )
    {
        buffer0_int_ = new unsigned int[ nbuffer_ ];
        buffer1_int_ = new unsigned int[ nbuffer_ ];
        buffer0_m128i_ = new __m128i[ nbuffer_ ];
        buffer1_m128i_ = new __m128i*[ nbuffer_ ];
        buffer2_m128i_ = new __m128i*[ nbuffer_ ];
    }

    ~QueryProcessorManagerSIMDBitMapSimple()
    {
        delete[] buffer0_int_;
        delete[] buffer1_int_;
        delete[] buffer0_m128i_;
        delete[] buffer1_m128i_;
        delete[] buffer2_m128i_;
    }

    // note functionality limited to a two-token AND query in this prototype implementation

    virtual void computeCommonSet()
    {
        // get block bitmaps for both tokens

        __m128i* blockbitmaps_ptr_a = NULL;
        __m128i* blockbitmaps_ptr_b = NULL;
        __m128i* blockbitmaps_ptr_a_end = NULL;
        __m128i* blockbitmaps_ptr_b_end = NULL;

        Query<TokenID>& query = QueryProcessorManager<DocID,TokenID>::getQuery();

        simd_index_.getBitMapBlocks( *query.getTokenID(0), blockbitmaps_ptr_a, blockbitmaps_ptr_a_end );

        simd_index_.getBitMapBlocks( *query.getTokenID(1), blockbitmaps_ptr_b, blockbitmaps_ptr_b_end );

//std::cout << "\n\ntoken 0 block bitmap = " << toString( blockbitmaps_ptr_a, blockbitmaps_ptr_a_end ) <<"\n";
//std::cout << "token 1 block bitmap = " << toString( blockbitmaps_ptr_b, blockbitmaps_ptr_b_end ) <<"\n\n";


        // AND the block bitmaps for both tokens

        __m128i* blockbitmaps_answer_ptr = buffer0_m128i_;
        __m128i* blockbitmaps_answer_ptr_end = buffer0_m128i_;

        for( __m128i *ptr_a = blockbitmaps_ptr_a, *ptr_b = blockbitmaps_ptr_b ; ptr_a < blockbitmaps_ptr_a_end && ptr_b < blockbitmaps_ptr_b_end ; ++ptr_a, ++ptr_b, ++blockbitmaps_answer_ptr_end )
        {
            bitmapAND( *ptr_a, *ptr_b, *blockbitmaps_answer_ptr_end );
        }

//std::cout << "block bitmap and = " << toString( blockbitmaps_answer_ptr, blockbitmaps_answer_ptr_end ) << "\n\n";



        // convert answer into a list of indices, i.e. the indices of the blocks with hits in the common set

        unsigned int* nonzero_block_indices_ptr = buffer0_int_;
        unsigned int* nonzero_block_indices_ptr_end = buffer0_int_;

        getIndicesOfNonZeroBits( blockbitmaps_answer_ptr, blockbitmaps_answer_ptr_end, nonzero_block_indices_ptr_end );

//std::cout << "block bitmap AND result, nonzero blocks are ";
//for( unsigned int* ptr = nonzero_block_indices_ptr ; ptr < nonzero_block_indices_ptr_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n\n";

        // convert the indices of these nonzero blocks into pointers to the corresponding docid __m128i, for each token

        __m128i* docidbitmaps_ptr_a = NULL;
        __m128i* docidbitmaps_ptr_b = NULL;
        __m128i* docidbitmaps_ptr_a_end = NULL;
        __m128i* docidbitmaps_ptr_b_end = NULL;

        simd_index_.getBitMapDocIDs( *query.getTokenID(0), docidbitmaps_ptr_a, docidbitmaps_ptr_a_end );

        simd_index_.getBitMapDocIDs( *query.getTokenID(1), docidbitmaps_ptr_b, docidbitmaps_ptr_b_end );

//std::cout << "token 0 docid bitmap =\n" << toString( docidbitmaps_ptr_a, docidbitmaps_ptr_a_end, "\n" ) <<"\n";
//std::cout << "token 1 docid bitmap =\n" << toString( docidbitmaps_ptr_b, docidbitmaps_ptr_b_end, "\n" ) <<"\n\n";

        __m128i** nonzerodocidbitmaps_ptr_a = buffer1_m128i_;
        __m128i** nonzerodocidbitmaps_ptr_a_end = buffer1_m128i_;
        __m128i** nonzerodocidbitmaps_ptr_b = buffer2_m128i_;
        __m128i** nonzerodocidbitmaps_ptr_b_end = buffer2_m128i_;

        unsigned int nm128iperblock = simd_index_.getNM128iPerBlock();

        getDocIDBitMapsForNonZeroBlocks( nonzero_block_indices_ptr, nonzero_block_indices_ptr_end, blockbitmaps_ptr_a, blockbitmaps_ptr_a_end, nm128iperblock, docidbitmaps_ptr_a, nonzerodocidbitmaps_ptr_a_end );

        getDocIDBitMapsForNonZeroBlocks( nonzero_block_indices_ptr, nonzero_block_indices_ptr_end, blockbitmaps_ptr_b, blockbitmaps_ptr_b_end, nm128iperblock, docidbitmaps_ptr_b, nonzerodocidbitmaps_ptr_b_end );

//std::cout << "token 0 docid bitmaps for AND =\n" << toString( nonzerodocidbitmaps_ptr_a, nonzerodocidbitmaps_ptr_a_end, "\n" ) <<"\n";
//std::cout << "token 1 docid bitmaps for AND =\n" << toString( nonzerodocidbitmaps_ptr_b, nonzerodocidbitmaps_ptr_b_end, "\n" ) <<"\n\n";



        // carry out the AND operation on the docid __m128i bitmaps, and convert the result into docids

        //unsigned int* docids_ptr = buffer1_int_;
        //unsigned int* docids_ptr_end = buffer1_int_;

        unsigned int nregisters = simd_index_.getNRegisters();

        unsigned int ndocids_per_block = nm128iperblock * nregisters;

        unsigned int ndocidbitmaps = nm128iperblock * (unsigned int) ( nonzero_block_indices_ptr_end-nonzero_block_indices_ptr );

        unsigned int docid_offset_for_block = *nonzero_block_indices_ptr * ndocids_per_block;

        CommonSet<DocID>& commonset = QueryProcessorManager<DocID,TokenID>::getCommonSet();

        commonset.clear();

        __m128i result;

//std::cout<< "\n";

        for( unsigned int idocidbitmap = 0 ; idocidbitmap < ndocidbitmaps ; ++idocidbitmap, docid_offset_for_block+=nregisters )
        {
            bitmapAND( *(nonzerodocidbitmaps_ptr_a[idocidbitmap]), *(nonzerodocidbitmaps_ptr_b[idocidbitmap]), result );

//std::cout << "a     = " << toString( *(nonzerodocidbitmaps_ptr_a[idocidbitmap])) << "\n";
//std::cout << "b     = " << toString( *(nonzerodocidbitmaps_ptr_b[idocidbitmap])) << "\n";
//std::cout << "\na & b = " << toString(result) << "\n";

            if( idocidbitmap != 0 && idocidbitmap%nm128iperblock==0 )
            {
                ++nonzero_block_indices_ptr;
                docid_offset_for_block = *nonzero_block_indices_ptr * ndocids_per_block;
            }

//std::cout << "idocidbitmap = " << idocidbitmap << ", offset = " << docid_offset_for_block << "\n";

//unsigned int* tmp = docids_ptr_end;

            //getIndicesOfNonZeroBits( &result, docid_offset_for_block, docids_ptr_end );

            getIndicesOfNonZeroBits( &result, docid_offset_for_block, commonset );

//if( (unsigned int)(docids_ptr_end-tmp) == 0 ) std::cout << "pointer hasnt moved\n";
//std::cout<<"new docids = ";
//for( ; tmp < docids_ptr_end ; ++tmp ) std::cout << *tmp << " ";
//std::cout<<"\n\n";
        }

//std::cout<< "\ncommon set docids = ";
//for( unsigned int* ptr = docids_ptr ; ptr < docids_ptr_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout<<"\n\n\n\n\n";



        // put the docids in commonset

        //CommonSet<DocID>& commonset = QueryProcessorManager<DocID,TokenID>::getCommonSet();

        //commonset.clear( query.getNResultsEnd() );

        //commonset.add( docids_ptr, docids_ptr_end, QueryProcessorManager<DocID,TokenID>::getDocumentProperties().getScores() );

    }

private:

    IndexSIMDBitMapSimple& simd_index_;

    unsigned int* buffer0_int_;

    unsigned int* buffer1_int_;

    __m128i* buffer0_m128i_;

    __m128i** buffer1_m128i_;

    __m128i** buffer2_m128i_;

    unsigned int nbuffer_;

};

}
}
}

#endif
