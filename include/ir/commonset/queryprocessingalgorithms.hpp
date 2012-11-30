#ifndef _IZENELIB_IR_COMMONSET_QUERY_PROCESSING_ALGORITHMS_HPP_
#define _IZENELIB_IR_COMMONSET_QUERY_PROCESSING_ALGORITHMS_HPP_

#include "query.hpp"
#include <types.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <x86intrin.h>

namespace izenelib
{
namespace ir
{
namespace commonset
{

template<typename DocID = unsigned int, typename TokenID = unsigned int>
class QueryProcessingAlgorithms
{
    __m128i shuffle_mask_[16];
    void prepare_shuffling_dictionary_()
    {
        for(int i = 0; i < 16; i++)
        {
            int counter = 0;
            char permutation[16];
            memset(permutation, 0xFF, sizeof(permutation));
            for(char b = 0; b < 4; b++)
            {
                if(getBit(i, b))
                {
                    permutation[counter++] = 4*b;
                    permutation[counter++] = 4*b + 1;
                    permutation[counter++] = 4*b + 2;
                    permutation[counter++] = 4*b + 3;
                }
            }
            __m128i mask = _mm_loadu_si128((const __m128i*)permutation);
            shuffle_mask_[i] = mask;
        }
    }

    int getBit(int value, int position)
    {
        return ( ( value & (1 << position) ) >> position);
    }
public:

    QueryProcessingAlgorithms()
    {
        prepare_shuffling_dictionary_();
    }

    virtual ~QueryProcessingAlgorithms() {}

    void getCommonSetAND
    (
        DocID* docptr_a, DocID* docptr_a_end,
        DocID* docptr_b, DocID* docptr_b_end,
        DocID*& docptr_answer
    )
    {
#if 1
        size_t counter = 0;
        while(docptr_a < docptr_a_end && docptr_b < docptr_b_end)
        {
            if(*docptr_a < *docptr_b)
            {
                ++docptr_a;
            }
            else if(*docptr_b < *docptr_a)
            {
                ++docptr_b;
            }
            else
            {
                docptr_answer[counter++] = *docptr_a;
                ++docptr_a;
                ++docptr_b;
            }
        }
        /*
            while( docptr_a < docptr_a_end )
            {
                while( docptr_b < docptr_b_end && *docptr_b < *docptr_a )
                {
                    ++docptr_b;
                }
                if( docptr_b == docptr_b_end ) return;
                if( *docptr_a == *docptr_b ) *(docptr_answer++) = *docptr_a;
                ++docptr_a;
            }*/
#else
        size_t count = 0;
        size_t i_a = 0, i_b = 0;
        size_t s_a = (docptr_a_end - docptr_a)/4;
        size_t s_b = (docptr_b_end - docptr_b)/4;		
        // trim lengths to be a multiple of 4
        size_t st_a = (s_a / 4) * 4;
        size_t st_b = (s_b / 4) * 4;

        while(i_a < s_a && i_b < s_b)
        {
            //[ load segments of four 32-bit elements
            __m128i v_a = _mm_load_si128((__m128i*)&docptr_a[i_a]);
            __m128i v_b = _mm_load_si128((__m128i*)&docptr_b[i_b]);
            //]

            //[ move pointers
#ifdef __SSE4__
            int32_t a_max = _mm_extract_epi32(v_a, 3);
            int32_t b_max = _mm_extract_epi32(v_b, 3);
#else /* !__SSE4__ */
            //*a = _mm_extract_epi16(r, 0) + (_mm_extract_epi16(r, 1) << 16);
            //*b = _mm_extract_epi16(r, 2) + (_mm_extract_epi16(r, 3) << 16);
            int32_t a_max = _mm_extract_epi16(v_a, 4) + (_mm_extract_epi16(v_a, 5) << 16);
            int32_t b_max = _mm_extract_epi16(v_b, 6) + (_mm_extract_epi16(v_b, 7) << 16);
#endif /* __SSE4__ */            
            i_a += (a_max <= b_max) * 4;
            i_b += (a_max >= b_max) * 4;
            //]

            //[ compute mask of common elements
            int32_t cyclic_shift = _MM_SHUFFLE(0,3,2,1);
            __m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b);    // pairwise comparison
            v_b = _mm_shuffle_epi32(v_b, cyclic_shift);       // shuffling
            __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, v_b);    // again...
            v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
            __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, v_b);    // and again...
            v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
            __m128i cmp_mask4 = _mm_cmpeq_epi32(v_a, v_b);    // and again.
            __m128i cmp_mask = _mm_or_si128(
                                   _mm_or_si128(cmp_mask1, cmp_mask2),
                                   _mm_or_si128(cmp_mask3, cmp_mask4)
                               ); // OR-ing of comparison masks
            // convert the 128-bit mask to the 4-bit mask
            int32_t mask = _mm_movemask_ps((__m128)cmp_mask);
            //]

            //[ copy out common elements
            __m128i p = _mm_shuffle_epi8(v_a, shuffle_mask_[mask]);
            _mm_storeu_si128((__m128i*)&docptr_answer[count], p);
            count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
            //]
        }
        // intersect the tail using scalar intersection	
        // todo
#endif
    }

    void getCommonSetOR
    (
        DocID* docptr_a, DocID* docptr_a_end,
        DocID* docptr_b, DocID* docptr_b_end,
        DocID*& docptr_answer
    )
    {}

    void getCommonSetNOT
    (
        DocID* docptr_a, DocID* docptr_a_end,
        DocID* docptr_b, DocID* docptr_b_end,
        DocID*& docptr_answer
    )
    {}

    void getCommonSetWAND
    (
        DocID* docptr_a, DocID* docptr_a_end,
        DocID* docptr_b, DocID* docptr_b_end,
        DocID*& docptr_answer
    )
    {}

    bool filterDocIDs
    (
        DocID* docids_ptr, DocID* docids_ptr_end,
        const Query<TokenID>& query,
        DocID*& docids_filtered_ptr, DocID*& docids_filtered_ptr_end
    )
    {
        docids_filtered_ptr = docids_ptr;
        docids_filtered_ptr_end = docids_ptr_end;
        return true;
        /*
            if( !query.hasFilters() )
            {
              docids_filtered_ptr = docids_ptr;
              docids_filtered_ptr_end = docids_ptr_end;
              return false;
            }

            docids_filtered_ptr_end = docids_filtered_ptr;

            if( query.hasCategoryFilter() && query.hasAttributeFilter() )
            {
              while( docids_ptr < docids_ptr_end )
              {
                if( document_attributes_.isACategoryMatch( *docids_ptr, query.getCategory() ) && document_attributes_.isAnAttributesMatch( *docids_ptr, query.getAttributes() )) *(docids_filtered_ptr_end++) = *docids_ptr;
                ++docids_ptr;
              }
              return true;
            }

            if( query.hasCategoryFilter() )
            {

              return true;
            }

            if( query.hasAttributeFilter() )
            {

              return true;
            }
        */
    }

};

}
}
}

#endif
