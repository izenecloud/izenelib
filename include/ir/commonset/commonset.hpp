#ifndef _IZENELIB_IR_COMMONSET_COMMONSET_HPP_
#define _IZENELIB_IR_COMMONSET_COMMONSET_HPP_

#include <vector>
#include <algorithm>
#include <cstring>
#include <limits>

namespace izenelib
{
namespace ir
{
namespace commonset
{

template<typename DocID = unsigned int>
class CommonSet
{
public:

    CommonSet( unsigned int nresults_max = 10, unsigned int nhistogram_bins = 1000000 ) :
        nresults_(0),
        nresults_max_(nresults_max),
        score_min_(0),
        scores_(NULL),
        nhistogram_bins_(nhistogram_bins),
        max_score_possible_(0),
        min_score_possible_(0),
        score_range_(0),
        histogram_bin_width_(0),
        histogram_bin_width_div_one_(0),
        histogram_(NULL)
    {
        histogram_ = new unsigned int[nhistogram_bins_];
        clear();
    }

    virtual ~CommonSet()
    {
        delete[] histogram_;
    }

    bool add( const DocID& docid, double score )
    {
        if( docids_.size() < nresults_max_ || (docids_.size() >= nresults_max_ && score > score_min_) )
        {
            docids_.push_back( std::pair<DocID,double>(docid,score) );
            ++nresults_;
            addScoreToHistogram(score);
            return true;
        }
        return false;
    }

    bool add( const std::pair<DocID,double>& docid_score )
    {
        return add( docid_score.first, docid_score.second );
    }

    void add( DocID* docptr, DocID* docptr_end, double* scores )
    {
        while( docptr < docptr_end )
        {
            add( *docptr, scores[*docptr] );
            ++docptr;
        }
    }

    void add( DocID* docptr, DocID* docptr_end )
    {
        add( docptr, docptr_end, scores_ );
    }

    bool add( const DocID& docid )
    {
        return add( docid, scores_[docid] );
    }

    bool add( DocID* docid )
    {
        return add( *docid, scores_[*docid] );
    }

    void add( const CommonSet<DocID>& commonset )
    {
        std::vector<std::pair<DocID,double> >& docids = commonset.getCommonSet();

        typename std::vector<std::pair<DocID,double> >::iterator docid = docids.begin();

        for( ; docid != docids.end() ; ++docid ) add( *docid );
    }

    void rankDocIDs()
    {
        // use histogram to estimate the lowest score within the top k results
        // run through the docids, selecting docids with score > than this score, until the top k results are found
        // just store docids on the fly. docid / score is only used when computing top k
        // sort using score
    }

    DocID* getDocID(unsigned int idoc)
    {
        return idoc < nresults_ ? &( docids_[idoc].first ) : NULL ;
    }

    unsigned int getDocIDs( DocID* docids, unsigned int ndocids_max )
    {
        typename std::vector<std::pair<DocID,double> >::iterator docid = docids_.begin();

        unsigned int idocid = 0;

        for( ; docid != docids_.end() && idocid < ndocids_max ; ++docid, ++idocid ) docids[idocid] = docid->first;

        return idocid;
    }

    unsigned int getNResults()
    {
        return nresults_;
    }

    unsigned int getNResultsMax()
    {
        return nresults_max_;
    }

    void clear()
    {
        score_min_ = std::numeric_limits<double>::max();
        docids_.clear();
        nresults_ = 0;
        resetHistogram();
    }

    void setScoresPointer( double* scores, unsigned int nscores )
    {
        scores_ = scores;

        max_score_possible_ = std::numeric_limits<double>::min();
        min_score_possible_ = std::numeric_limits<double>::max();

        for( double* ptr = scores_ ; ptr < scores_ + nscores ; ++ptr )
        {
            max_score_possible_ = std::max( max_score_possible_, *ptr );
            min_score_possible_ = std::min( min_score_possible_, *ptr );
        }

        score_range_ = max_score_possible_ - min_score_possible_;

        histogram_bin_width_ = score_range_ / nhistogram_bins_;

        histogram_bin_width_div_one_ = (double) nhistogram_bins_ / score_range_;

        resetHistogram();
    }

private:

    std::vector<std::pair<DocID,double> > docids_;

    unsigned int nresults_;

    unsigned int nresults_max_;

    double score_min_;

    double* scores_;

    unsigned int nhistogram_bins_;

    double max_score_possible_;

    double min_score_possible_;

    double score_range_;

    double histogram_bin_width_;

    double histogram_bin_width_div_one_;

    unsigned int* histogram_;

    void resetHistogram()
    {
        memset( histogram_, 0, nhistogram_bins_ * sizeof(unsigned int) );
    }

    void addScoreToHistogram(double score)
    {
        int ibin = ( score - min_score_possible_ ) * histogram_bin_width_div_one_;
        if( ibin < 0 ) ibin = 0;
        if( (unsigned)ibin >= nhistogram_bins_ ) ibin = nhistogram_bins_-1;
        score_min_ = std::min( ibin*histogram_bin_width_, score_min_ );
        ++histogram_[ibin];
    }


    struct Comparator
    {
        bool operator()( const std::pair<unsigned int,double>& p0, const std::pair<unsigned int,double>& p1 )
        {
            if( p0.second == p1.second ) return p0.first < p1.first ? true : false ;
            return p0.second < p1.second ? true : false ;
        }
    } comparator_;

};

}
}
}

#endif
