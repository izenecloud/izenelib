#ifndef IZENELIB_UTIL_TIMING_BLOOM_FILTER_H
#define IZENELIB_UTIL_TIMING_BLOOM_FILTER_H

#include <cmath>
#include <limits.h>
#include <stdexcept>
#include <types.h>
#include <time.h>

#include <util/hashFunction.h>
#include "boost/date_time/posix_time/posix_time.hpp"

namespace izenelib { namespace util {

/**
 * TimingBloomFilter
 */
template <typename KeyType, typename HashType = uint32_t>
class TimingBloomFilter
{
typedef uint32_t TimeType;
public:
    TimingBloomFilter()
        : num_hash_functions_(0)
        , num_items_(0)
        , vector_(NULL)
        , base_time_(0)
    {}

    TimingBloomFilter(size_t num_items, size_t num_hashes)
        : num_hash_functions_(num_hashes)
        , num_items_(num_items)
        , vector_(NULL)
        , base_time_(0)
    {
        if (num_items_ == 0)
        {
            throw std::runtime_error("Empty bloom filter");
        }
        
        num_items_ *= num_hash_functions_ / std::log(2);

        vector_ = new TimeType[num_items_];
        memset(vector_, 0, num_items_ * sizeof(TimeType));

        using namespace boost::posix_time;
        ptime t(second_clock::local_time());
        std::string year = to_simple_string(t).substr(0, 4) + "-01-01 00:00:00";
        t = time_from_string(year);
        struct tm tm = to_tm(t);
        base_time_ = mktime(&tm);
    }

    ~TimingBloomFilter()
    {
        if (NULL != vector_)
        {
            delete[] vector_;
            vector_ = NULL;
        }
    }

    void put(const KeyType& key, time_t exp)
    {
        uint32_t e = exp - base_time_;

        uint32_t hash = 0;
        for (size_t i = 0; i < num_hash_functions_; ++i)
        {
            hash = hasher_(key,hash) % num_items_;
            if (vector_[hash] < e)
                vector_[hash] = e;
        }
    }

    bool get(const KeyType& key, time_t time) const
    {
        uint32_t shed = time - base_time_;
        uint32_t hash = 0;
        for (size_t i = 0; i < num_hash_functions_; ++i)
        {
            hash = hasher_(key,hash) % num_items_;

            if (0 == vector_[hash] || vector_[hash] < shed )
            {
                return false;
            }
        }
        return true;
    }

    void save(std::ostream& ostr) const
    {
        ostr.write((const char *)&num_hash_functions_, sizeof(num_hash_functions_));
        ostr.write((const char *)&num_items_, sizeof(num_items_));
        ostr.write((const char *)vector_, num_items_ * sizeof(TimeType));
        ostr.write((const char *)&base_time_, sizeof(base_time_));
    }

    void load(std::istream& istr)
    {
        istr.read((char *)&num_hash_functions_, sizeof(num_hash_functions_));
        istr.read((char *)&num_items_, sizeof(num_items_));
        if (!vector_) 
            vector_ = new TimeType[num_items_];
        istr.read((char *)vector_, num_items_ * sizeof(TimeType));
        istr.read((char *)&base_time_, sizeof(base_time_));
    }

    size_t size()
    {
        return num_items_ * sizeof(TimeType);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(TimingBloomFilter);

    HashIDTraits<KeyType,HashType> hasher_;
    size_t num_hash_functions_;
    size_t num_items_;
    TimeType* vector_;
    time_t base_time_;
};

}}

#endif // IZENELIB_UTIL_TIMING_BLOOM_FILTER_H
