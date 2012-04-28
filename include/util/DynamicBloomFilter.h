#ifndef IZENELIB_UTIL_DYNAMIC_BLOOM_FILTER_H
#define IZENELIB_UTIL_DYNAMIC_BLOOM_FILTER_H

#include "BloomFilter.h"

#include <vector>

namespace izenelib { namespace util {

/**
 * Implements a Dynamic Bloom filter(DBF), as defined in the INFOCOM 2006 paper:
 * Theory and Network Applications of Dynamic Bloom Filters
 *
 * A dynamic Bloom filter (DBF) makes use of a s * m bit matrix but
 * each of the s rows is a standard Bloom filter. The creation
 * process of a DBF is iterative. At the start, the DBF is a 1 * m
 * bit matrix, i.e., it is composed of a single standard Bloom filter.
 * It assumes that n_r elements are recorded in the initial bit vector,
 * where n_r <= n (n is the cardinality of the set A to record in the filter).
 *
 * As the size of A grows during the execution of the application,
 * several keys must be inserted in the DBF.  When inserting a key into the DBF,
 * one must first get an active Bloom filter in the matrix.  A Bloom filter is
 * active when the number of recorded keys, n_r, is strictly less than the current
 * cardinality of A, n.
 * If an active Bloom filter is found, the key is inserted and n_r is incremented
 * by one. On the other hand, if there is no active Bloom filter, a new one is created
 * (i.e., a new row is added to the matrix) according to the current size of A and the element
 * is added in this new Bloom filter and the n_r value of this new Bloom filter is set to one.
 * A given key is said to belong to the DBF if the k positions are set to one in one of the matrix rows.
 */
template<typename KeyType, typename HashType = uint32_t>
class DynamicBloomFilter
{
    typedef BloomFilter<KeyType,HashType> StandardBloomFilterType;
public:
    DynamicBloomFilter()
        :items_estimate_(0)
        ,false_positive_prob_(0)
        ,n_0_(0)
        ,n_r_(0)
    {}

    DynamicBloomFilter(size_t items_estimate, float false_positive_prob, size_t n_0)
        :items_estimate_(items_estimate)
        ,false_positive_prob_(false_positive_prob)
        ,n_0_(n_0)
        ,n_r_(0)
    {}

    ~DynamicBloomFilter()
    {
        for(unsigned i = 0; i < bloom_filters_.size(); ++i)
        {
            delete bloom_filters_[i];
        }
    }

    void Insert(const KeyType& key)
    {
        if(n_r_ % n_0_ == 0)
        {
            bloom_filters_.push_back(
                new BloomFilter<KeyType,HashType>(items_estimate_,false_positive_prob_));
        }
        bloom_filters_.back()->Insert(key);
        ++n_r_;
    }

    bool Get(const KeyType& key) const
    {
        for(unsigned i = 0; i < bloom_filters_.size(); ++i)
        {
            if(bloom_filters_[i]->Get(key))
                return true;
        }
        return false;
    }

    template<class DataIO> friend
    void DataIO_loadObject(DataIO& dio, DynamicBloomFilter& x)
    {
        dio & x.items_estimate_;
        dio & x.false_positive_prob_;
        dio & x.n_0_;
        dio & x.n_r_;
        size_t num_bfs = 0;
        dio & num_bfs;
        for(unsigned i = 0; i < num_bfs; ++i)
        {
            typename DynamicBloomFilter::StandardBloomFilterType* bloomfilter
                = new typename DynamicBloomFilter::StandardBloomFilterType;
            dio & *bloomfilter;
            x.bloom_filters_.push_back(bloomfilter);
        }
    }

    template<class DataIO> friend
    void DataIO_saveObject(DataIO& dio, const DynamicBloomFilter& x)
    {
        dio & x.items_estimate_;
        dio & x.false_positive_prob_;
        dio & x.n_0_;
        dio & x.n_r_;
        size_t num_bfs = x.bloom_filters_.size();
        dio & num_bfs;
        for(unsigned i = 0; i < num_bfs; ++i)
        {
            dio & *(x.bloom_filters_[i]);
        }
    }

    void save(std::ostream& ostr) const
    {
        ostr.write((const char *)&items_estimate_, sizeof(items_estimate_));
        ostr.write((const char *)&false_positive_prob_, sizeof(false_positive_prob_));
        ostr.write((const char *)&n_0_, sizeof(n_0_));
        ostr.write((const char *)&n_r_, sizeof(n_r_));
        size_t num_bfs = bloom_filters_.size();
        ostr.write((const char *)&num_bfs, sizeof(num_bfs));
        for (size_t i = 0; i < num_bfs; i++)
        {
            bloom_filters_[i]->save(ostr);
        }
    }

    void load(std::istream& istr)
    {
        istr.read((char *)&items_estimate_, sizeof(items_estimate_));
        istr.read((char *)&false_positive_prob_, sizeof(false_positive_prob_));
        istr.read((char *)&n_0_, sizeof(n_0_));
        istr.read((char *)&n_r_, sizeof(n_r_));
        size_t num_bfs = 0;
        istr.read((char *)&num_bfs, sizeof(num_bfs));
        bloom_filters_.resize(num_bfs);
        for (size_t i = 0; i < num_bfs; i++)
        {
            bloom_filters_[i] = new typename DynamicBloomFilter::StandardBloomFilterType;
            bloom_filters_[i]->load(istr);
        }
    }

private:
    DISALLOW_COPY_AND_ASSIGN(DynamicBloomFilter);

    size_t items_estimate_;
    float false_positive_prob_;
    size_t n_0_;
    size_t n_r_;
    std::vector<StandardBloomFilterType* >  bloom_filters_;
};

}}

#endif
