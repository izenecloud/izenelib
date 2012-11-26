#ifndef _IZENELIB_IR_COMMONSET_INDEX_HPP_
#define _IZENELIB_IR_COMMONSET_INDEX_HPP_

#include "indexmemorycached.hpp"
#include "indexdiskcached.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class Index : virtual protected IndexMemoryCached<DocID,TokenID>, virtual protected IndexDiskCached<DocID,TokenID>
{
public:

    Index() :
        IndexMemoryCached<DocID,TokenID>( "./" ),
        IndexDiskCached<DocID,TokenID>( "./" )
    {}

    Index( std::string directory ) :
        IndexMemoryCached<DocID,TokenID>( directory ),
        IndexDiskCached<DocID,TokenID>( directory )
    {}

    virtual ~Index() {};

    virtual bool getDocIDs( const TokenID& tokenid, DocID* buffer, unsigned int nbuffervalues_max, DocID*& docid_ptr, DocID*& docid_ptr_end )
    {
        if( IndexMemoryCached<DocID,TokenID>::getDocIDs( tokenid, docid_ptr, docid_ptr_end ) ) return true;

        if( IndexDiskCached<DocID,TokenID>::getDocIDs( tokenid, buffer, nbuffervalues_max, docid_ptr, docid_ptr_end ) ) return true;

        return false;
    }

    virtual bool getDocIDs( const Query<TokenID>& query, DocID* buffer, unsigned int nbuffervalues_max, DocID*& docid_ptr, DocID*& docid_ptr_end )
    {
        if( IndexMemoryCached<DocID,TokenID>::getDocIDs( query, docid_ptr, docid_ptr_end ) ) return true;

        if( IndexDiskCached<DocID,TokenID>::getDocIDs( query, buffer, nbuffervalues_max, docid_ptr, docid_ptr_end ) ) return true;

        return false;
    }

};

}
}
}

#endif
