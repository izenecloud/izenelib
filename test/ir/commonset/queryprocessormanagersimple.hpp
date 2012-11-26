#ifndef _SF1R_NEW_QUERYPROCESSORMANAGERSIMPLE_HPP_
#define _SF1R_NEW_QUERYPROCESSORMANAGERSIMPLE_HPP_

#include  <ir/commonset/queryprocessormanager.hpp>
#include  <ir/commonset/queryprocessingalgorithms.hpp>

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class QueryProcessorManagerSimple : public QueryProcessorManager<DocID,TokenID>, QueryProcessingAlgorithms<DocID,TokenID>
{
public:

    QueryProcessorManagerSimple
    (
        Index<DocID,TokenID>& index,
        DocumentProperties<DocID>& documentproperties,
        unsigned int ntotal_buffer_docids = 10*1024*1024
    ) :
        QueryProcessorManager<DocID,TokenID>(index, documentproperties),
        QueryProcessingAlgorithms<DocID,TokenID>(),
        nbuffer_docids( ntotal_buffer_docids / 3 ),
        buffer0_(NULL),
        buffer1_(NULL),
        buffer2_(NULL)
    {
        buffer0_ = new DocID[ nbuffer_docids ];
        buffer1_ = new DocID[ nbuffer_docids ];
        buffer2_ = new DocID[ nbuffer_docids ];
    }

    ~QueryProcessorManagerSimple()
    {
        delete[] buffer0_;
        delete[] buffer1_;
        delete[] buffer2_;
    }


    virtual void computeCommonSet()
    {
        Query<TokenID>& query = QueryProcessorManager<DocID,TokenID>::getQuery();

        Index<DocID,TokenID>& index = QueryProcessorManager<DocID,TokenID>::getIndex();

        CommonSet<DocID>& commonset = QueryProcessorManager<DocID,TokenID>::getCommonSet();

        unsigned int ntokens = query.getNTokens();

        if( ntokens < 1 ) return;

        int* operators = query.getOperators();

        commonset.clear();

        DocID* docptr_answer = buffer0_;
        DocID* docptr_answer_end = buffer0_;
        DocID* docptr_a = NULL;
        DocID* docptr_a_end = NULL;
        DocID* docptr_b = NULL;
        DocID* docptr_b_end = NULL;

        index.getDocIDs( *query.getTokenID(0), buffer2_, nbuffer_docids, docptr_answer, docptr_answer_end );

        //QueryProcessingAlgorithms<DocID,TokenID>::filterDocIDs( docptr_a, docptr_a_end, query, docptr_answer, docptr_answer_end );

        for( unsigned int itoken = 1 ; itoken < ntokens ; ++itoken, ++operators )
        {
            docptr_a = docptr_answer;
            docptr_a_end = docptr_answer_end;

//std::cout << "\ncomputeCommonSet : answer = ";
//int n = (int)(docptr_answer_end-docptr_answer);
//for( unsigned int* ptr = docptr_answer ; ptr < docptr_answer_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n";

            index.getDocIDs( *query.getTokenID(itoken), buffer2_, nbuffer_docids, docptr_b, docptr_b_end );

//std::cout << "\ncomputeCommonSet : next token = ";
//n = (int)(docptr_b_end-docptr_b);
//for( unsigned int* ptr = docptr_b ; ptr < docptr_b_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n";

            docptr_answer = (docptr_a == buffer0_) ? buffer1_ : buffer0_;
            docptr_answer_end = docptr_answer;

            switch( *operators )
            {
            case Query<TokenID>::AND:
                QueryProcessingAlgorithms<DocID,TokenID>::getCommonSetAND( docptr_a, docptr_a_end, docptr_b, docptr_b_end, docptr_answer_end );
                break;
            case Query<TokenID>::OR:
                QueryProcessingAlgorithms<DocID,TokenID>::getCommonSetOR( docptr_a, docptr_a_end, docptr_b, docptr_b_end, docptr_answer_end );
                break;
            case Query<TokenID>::NOT:
                QueryProcessingAlgorithms<DocID,TokenID>::getCommonSetNOT( docptr_a, docptr_a_end, docptr_b, docptr_b_end, docptr_answer_end );
                break;
            case Query<TokenID>::WAND:
                QueryProcessingAlgorithms<DocID,TokenID>::getCommonSetWAND( docptr_a, docptr_a_end, docptr_b, docptr_b_end, docptr_answer_end );
                break;
            }

//std::cout << "\ncomputeCommonSet : new answer = ";
//n = (int)(docptr_answer_end-docptr_answer);
//for( unsigned int* ptr = docptr_answer ; ptr < docptr_answer_end ; ++ptr ) std::cout << *ptr << " ";
//std::cout << "\n";

        }

        commonset.add( docptr_answer, docptr_answer_end, QueryProcessorManager<DocID,TokenID>::getDocumentProperties().getScores() );
    }

private:

    unsigned int nbuffer_docids;
    DocID* buffer0_;
    DocID* buffer1_;
    DocID* buffer2_;

};

}
}
}

#endif
