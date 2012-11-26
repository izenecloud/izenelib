#ifndef _IZENELIB_IR_COMMONSET_QUERY_HPP_
#define _IZENELIB_IR_COMMONSET_QUERY_HPP_

#include <vector>

namespace izenelib
{
namespace ir
{
namespace commonset
{

template<typename TokenID = unsigned int>
class Query
{
public:

    Query() :
        tokens_(NULL),
        operators_(NULL),
        ntokens_( 0 ),
        nresults_( 0 ),
        nresults_start_( 0 ),
        nresults_end_( 0 )
    {}

    Query( const std::vector<TokenID>& tokens, const std::vector<std::string>& operators, unsigned int nresults_start, unsigned int nresults_end ) :
        tokens_(NULL),
        operators_(NULL),
        ntokens_( tokens.size() ),
        nresults_( nresults_end - nresults_start ),
        nresults_start_( nresults_start ),
        nresults_end_( nresults_end )
    {
        tokens_ = new TokenID[ntokens_];
        for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken ) tokens_[itoken] = tokens[itoken];

        operators_ = new int[ntokens_-1];
        for( unsigned int iop = 0 ; iop < ntokens_-1 ; ++iop )
        {
            if( operators[iop] == "OR" ) operators_[iop] = OR;
            else if ( operators[iop] == "NOT" ) operators_[iop] = NOT;
            else if ( operators[iop] == "WAND" ) operators_[iop] = WAND;
            else operators_[iop] = AND;
        }

//ntokens_=1;
    }

    virtual ~Query()
    {
        delete[] tokens_;
        delete[] operators_;
    }
    unsigned int getNTokens() const
    {
        return ntokens_;
    }

    unsigned int getNResults() const
    {
        return nresults_;
    }

    unsigned int getNResultsStart() const
    {
        return nresults_start_;
    }

    unsigned int getNResultsEnd() const
    {
        return nresults_end_;
    }

    TokenID* getTokenIDs() const
    {
        return tokens_;
    }

    TokenID* getTokenID(unsigned int itoken)
    {
        return itoken < ntokens_ ? tokens_ + itoken : NULL;
    }

    int* getOperators() const
    {
        return operators_;
    }

    int getOperator(unsigned int ioperator)
    {
        return ioperator < ntokens_-1 ? operators_ + ioperator : INVALID_OPERATOR;
    }

    enum Operators { INVALID_OPERATOR, AND, OR, NOT, WAND };

private:

    TokenID* tokens_;
    int* operators_;
    unsigned int ntokens_;
    unsigned int nresults_;
    unsigned int nresults_start_;
    unsigned int nresults_end_;

};

}
}
}

#endif
