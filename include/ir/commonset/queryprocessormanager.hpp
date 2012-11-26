#ifndef _IZENELIB_IR_COMMONSET_QUERYPROCESSORMANAGER_HPP_
#define _IZENELIB_IR_COMMONSET_QUERYPROCESSORMANAGER_HPP_

#include "index.hpp"
#include "documentproperties.hpp"
#include "commonset.hpp"
#include "query.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class QueryProcessorManager
{
public:

    QueryProcessorManager
    (
        Index<DocID,TokenID>& index,
        DocumentProperties<DocID>& documentproperties
    ) :
        index_(index),
        documentproperties_(documentproperties),
        commonset_(),
        query_( NULL )
    {
        commonset_.setScoresPointer( documentproperties_.getScores(), documentproperties_.getNScores() );
    }

    virtual void computeCommonSet() = 0;

    void setQuery( Query<TokenID>& query )
    {
        query_ = &query;
    }

    CommonSet<DocID>& getCommonSet()
    {
        return commonset_;
    }

    Query<TokenID>& getQuery() const
    {
        return *query_;
    }

    Index<DocID,TokenID>& getIndex() const
    {
        return index_;
    }

    DocumentProperties<DocID>& getDocumentProperties() const
    {
        return documentproperties_;
    }

private:

    Index<DocID,TokenID>& index_;

    DocumentProperties<DocID>& documentproperties_;

    CommonSet<DocID> commonset_;

    Query<TokenID>* query_;

};

}
}
}

#endif
