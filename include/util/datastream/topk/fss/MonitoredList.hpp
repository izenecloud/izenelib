/*
 *     @file   MonitoredList.hpp
 *     @author Kuilong Liu
 *     @date   2013.04.23
 *     @       filtered space saving for topk
 */

#ifndef IZENELIB_UTIL_MONITOREDLIST_H_
#define IZENELIB_UTIL_MONITOREDLIST_H_

#include <list>
#include <stdint.h>
#include <iterator>
#include <vector>
#include <map>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace izenelib{ namespace util {

template<typename ElemType, typename CountType, typename ErrorType>
class MonitoredBucket;

template<typename ElemType, typename CountType, typename ErrorType>
class MonitoredItem
{
public:
    MonitoredItem(ElemType elem, ErrorType err)
        :element_(elem),
        error_(err)
    {
    }
    ~MonitoredItem()
    {
    }

    ElemType element_;
    ErrorType error_;
    MonitoredBucket<ElemType, CountType, ErrorType>* bucket_;
};

template<typename ElemType, typename CountType, typename ErrorType>
class MonitoredBucket
{
public:
    typedef MonitoredItem<ElemType, CountType, ErrorType> MonitoredItemT;
    MonitoredBucket(CountType c)
        :count_(c),
        bucket_size_(0)
    {
    }
    ~MonitoredBucket()
    {
    }

    bool insert(ElemType elem, ErrorType err)
    {
        bucket_size_++;
        typename std::list<MonitoredItemT >::iterator iter = items_.begin();
        do
        {
            if(iter == items_.end() || iter->error_ >= err)
            {
                MonitoredItemT mi(elem, err);
//                mi->element_ = elem;
//                mi->error_ = err;
//                mi->bucket_ = this;
                items_.insert(iter, mi);
                break;
            }
            else iter++;
        }while(true);
        return true;
    }
    bool erase()
    {
        bucket_size_--;
        items_.pop_back();
        return true;
    }

    bool erase(ElemType elem)
    {
        bucket_size_--;
        typename std::list<MonitoredItemT>::iterator iter = items_.begin();
        for(;iter!=items_.end();iter++)
        {
            if(iter->element_ == elem)
            {
                items_.erase(iter);
            }
        }
        return true;
    }

    bool del(ElemType elem, ErrorType& error)
    {
        typename std::list<MonitoredItemT>::iterator iter = items_.begin();
        for(;iter!=items_.end();iter++)
        {
            if(iter->element_ == elem)
            {
                error = iter->error_;
                items_.erase(iter);
                bucket_size_--;
                return true;
            }
        }
        return false;
    }
    CountType count_;
    CountType bucket_size_;
    std::list<MonitoredItemT> items_;
};

template<typename ElemType, typename CountType, typename ErrorType>
class MonitoredList
{
public:
    typedef MonitoredBucket<ElemType, CountType, ErrorType> MonitoredBucketT;
    typedef MonitoredItem<ElemType, CountType, ErrorType> MonitoredItemT;

    MonitoredList(CountType ms)
        :MAXSIZE(ms),
        current_size_(0)
    {
    }

    bool insert(ElemType elem, CountType count, ErrorType error)
    {
        if(current_size_ < MAXSIZE)
        {
            current_size_++;
        }
        else
        {
            buckets_.begin()->erase();
            if(buckets_.begin()->bucket_size_==0 && buckets_.begin()->count_ != count)
                buckets_.pop_front();
        }

        typename std::list<MonitoredBucketT >::iterator iter = buckets_.begin();
        do
        {
            if(iter == buckets_.end() || iter->count_ > count)
            {
                MonitoredBucketT mb(count);
//                mb.count_ = count;
//                mb.bucket_size_ = 0;
                mb.insert(elem, error);
                buckets_.insert(iter, mb);
                break;
            }
            else if(iter->count_ == count)
            {
                iter->insert(elem, error);
                break;
            }
            else if(iter->count_ == count-1)
            {
                if(iter->bucket_size_ == 1)
                {
                    iter->count_ = count;
                    break;
                }
                else
                {
                    iter->erase(elem);
                }
            }
            else iter++;
        }while(true);

        return true;
    }

    bool insert(ElemType elem)
    {
        typename std::list<MonitoredBucketT>::iterator iter, it;
        iter = buckets_.begin();
        for(;iter!=buckets_.end();iter++)
        {
            ErrorType error = 0;
            it = iter; it++;

            if(iter->bucket_size_ == 1 && iter->items_.begin()->element_ == elem &&(it == buckets_.end() || it->count_ != iter->count_+1))
            {
                iter->count_++;
                return true;
            }
            else if(iter->del(elem, error))
            {
                CountType count = iter->count_;count++;
                it = iter;
                it++;
                if(it == buckets_.end() || it->count_ != count)
                {
                    MonitoredBucketT mb(count);
                    mb.insert(elem, error);
                    buckets_.insert(it, mb);
                }
                else
                {
                    it->insert(elem, error);
                    if(iter->bucket_size_ == 0)
                        buckets_.erase(iter);
                }
                return true;
            }
        }

        CountType count = 1;
        ErrorType error = 0;

        if(current_size_ < MAXSIZE)
        {
            current_size_++;
        }
        else
        {
            error = buckets_.begin()->count_;
            count = error+1;
            buckets_.begin()->erase();
            if(buckets_.begin()->bucket_size_==0 && buckets_.begin()->count_ != count)
                buckets_.pop_front();
        }
        iter = buckets_.begin();
        do
        {
            if(iter == buckets_.end() || iter->count_ > count)
            {
                MonitoredBucketT mb(count);
//                mb.count_ = count;
//                mb.bucket_size_ = 0;
                mb.insert(elem, error);
                buckets_.insert(iter, mb);
                break;
            }
            else if(iter->count_ == count)
            {
                iter->insert(elem, error);
                break;
            }
            else iter++;
        }while(true);


        return true;
    }
    bool get(std::list<std::string>& elems,
            std::list<uint32_t>& count,
            std::list<uint32_t>& err)
    {
        typename std::list<MonitoredBucketT>::iterator iter = buckets_.end();
        typename std::list<MonitoredItemT>::iterator it;
        while(iter!=buckets_.begin())
        {
            iter--;
            it = iter->items_.begin();
            for(;it!=iter->items_.end();it++)
            {
                elems.push_back(it->element_);
                count.push_back(iter->count_);
                err.push_back(it->error_);
            }
        }
        return true;
    }
private:
    CountType MAXSIZE;
    CountType current_size_;
    std::list<MonitoredBucketT> buckets_;

    boost::unordered_map<ElemType, boost::shared_ptr<MonitoredItemT> > elem_pos_map_;
};

}
}

#endif
