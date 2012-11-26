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

        //std::random_device rd;
        //std::mt19937 generator(rd());
        //std::uniform_real_distribution<double> random_score(0,1.0);
        //for( unsigned int idoc = 0 ; idoc < ndocids_ ; ++idoc ) scores_[idoc] = random_score(generator);
    }

    virtual ~DocumentProperties()
    {
        delete[] scores_;
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
