#ifndef _IZENELIB_IR_COMMONSET_QUERY_PROCESSING_ALGORITHMS_HPP_
#define _IZENELIB_IR_COMMONSET_QUERY_PROCESSING_ALGORITHMS_HPP_

#include "query.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

template<typename DocID = unsigned int, typename TokenID = unsigned int>
class QueryProcessingAlgorithms
{
public:

    QueryProcessingAlgorithms() {}

    virtual ~QueryProcessingAlgorithms() {}

    void getCommonSetAND
    (
        DocID* docptr_a, DocID* docptr_a_end,
        DocID* docptr_b, DocID* docptr_b_end,
        DocID*& docptr_answer
    )
    {
        while( docptr_a < docptr_a_end )
        {
            while( docptr_b < docptr_b_end && *docptr_b < *docptr_a )
            {
                ++docptr_b;
            }
            if( docptr_b == docptr_b_end ) return;
            if( *docptr_a == *docptr_b ) *(docptr_answer++) = *docptr_a;
            ++docptr_a;
        }
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
