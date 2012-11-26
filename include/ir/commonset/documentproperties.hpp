#ifndef _IZENELIB_IR_COMMONSET_DOCUMENT_PROPERTIES_HPP_
#define _IZENELIB_IR_COMMONSET_DOCUMENT_PROPERTIES_HPP_

namespace izenelib
{
namespace ir
{
namespace commonset
{

template<typename DocID = unsigned int>
class DocumentProperties
{
public:

    DocumentProperties( unsigned int ndocids, bool rank_by_docid = false ) :
        ndocids_(ndocids),
        scores_(NULL)
    {
        scores_ = new double[ ndocids_ ];

        if( rank_by_docid )
        {
            for( unsigned int idoc = 0 ; idoc < ndocids_ ; ++idoc ) scores_[ idoc ] = 1.0/(idoc+1.0);
        }
    }

    virtual ~DocumentProperties()
    {
        delete[] scores_;
    }

    void setScore(unsigned int doc, double score)
    {
        scores_[doc] = score;
    }

    double* const getScores() const
    {
        return scores_;
    }

    unsigned int getNScores() const
    {
        return ndocids_;
    }

private:
    unsigned int ndocids_;
    double* scores_;
};

}
}
}

#endif
