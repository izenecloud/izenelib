#ifndef IZENELIB_UTIL_VECTORJOINER_H_
#define IZENELIB_UTIL_VECTORJOINER_H_

#include <types.h>
#include <vector>
namespace izenelib{
namespace util{

template <class value_type>
class VectorJoiner 
{
    
typedef std::vector<value_type> vec_type;
public:
    VectorJoiner(vec_type* vec):vec_(vec), b_(false), index_(0)
    {
        if(!vec_->empty())
        {
            b_ = true;
            cache_ = vec_->front();
            index_ = 1;
        }
    }

    bool Next(vec_type& vec)
    {
        if(!b_) return false;
        value_type key = cache_;
        //if(cache_.empty()) return false;
        vec.resize(0);
        while(b_)
        {
            if( !(key<cache_) )
            {
                vec.push_back(cache_);
                if(index_>=vec_->size())
                {
                    b_ = false;
                    break;
                }
                cache_ = (*vec_)[index_++];
            }
            else
            {
                break;
            }
        }
        return true;

    }

private:
    vec_type* vec_;
    bool b_;
    value_type cache_;
    std::size_t index_;
        
};

}
}
#endif
