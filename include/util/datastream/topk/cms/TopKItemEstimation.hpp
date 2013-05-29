/*
 *  @file   TopKItemEstimation.hpp
 *  @author Kuilong Liu
 *  @date   2013.04.24
 *  @       TopKItem by count min sketch
 */
#ifndef IZENELIB_UTIL_TOPKITEM_ESTIMATION_H_
#define IZENELIB_UTIL_TOPKITEM_ESTIMATION_H_

#include <list>
#include <iterator>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

namespace izenelib { namespace util {

template<typename ElemType, typename CountType>
class Bucket;

template<typename ElemType, typename CountType>
class Item
{
public:
    typedef Bucket<ElemType, CountType> BucketT;
    typedef Item<ElemType, CountType> ItemT;

    Item(ElemType e, BucketT* b, ItemT* p, ItemT* n)
        :elem_(e), b_(b), prev_(p), next_(n)
    {
    }
    ~Item()
    {
        b_ = NULL;
        if(next_)delete next_;
    }

    ElemType elem_;
    BucketT* b_;
    ItemT* prev_;
    ItemT* next_;
};

template<typename ElemType, typename CountType>
class Bucket
{
public:
    typedef Item<ElemType, CountType> ItemT;
    typedef Bucket<ElemType, CountType> BucketT;
    Bucket(CountType c, BucketT* p, BucketT* n)
        :size_(0), c_(c), head_(NULL), end_(NULL), prev_(p), next_(n)
    {
    }
    ~Bucket()
    {
        if(head_)delete head_;
        if(next_)delete next_;
    }

    bool insert(ItemT* i)
    {
        if(size_==0)
        {
            head_=end_=i;
            i->prev_=NULL;
        }
        else
        {
            end_->next_=i;
            i->prev_=end_;
            end_=i;
        }
        i->b_=this;
        i->next_=NULL;
        size_++;
        return true;
    }

    bool erase(ItemT* i)
    {
        if(size_==1)
        {
            head_=end_=NULL;
        }
        else if(i->next_==NULL)
        {
            end_=end_->prev_;
            end_->next_=NULL;
        }
        else if(i->prev_==NULL)
        {
            head_=i->next_;
            head_->prev_=NULL;
        }
        else
        {
            i->prev_->next_=i->next_;
            i->next_->prev_=i->prev_;
        }
        size_--;
        i->prev_=i->next_=NULL;
        i->b_=NULL;
        return true;
    }
    CountType size_;
    CountType c_;
    ItemT* head_;
    ItemT* end_;
    BucketT* prev_;
    BucketT* next_;
};

template<typename ElemType, typename CountType>
class TopKEstimation
{
public:
    typedef Bucket<ElemType, CountType> BucketT;
    typedef Item<ElemType, CountType> ItemT;

    TopKEstimation(CountType m)
        :MAXSIZE_(m),
        size_(0),
        th_(0)
    {
        bs_ = new BucketT(0, NULL, NULL);
        end_=bs_;
    }

    ~TopKEstimation()
    {
        if(bs_)delete bs_;
        gps_.clear();
    }

    bool reset()
    {
        if(bs_) delete bs_;
        bs_=new BucketT(0, NULL, NULL);
        end_=bs_;
        gps_.clear();
        size_=th_=0;
        return true;
    }
    bool insert(ElemType elem, CountType count)
    {
        if(gps_.find(elem) != gps_.end())
        {
            return update(elem, count);
        }
        else if(size_ >= MAXSIZE_ && count <= th_)
            return true;
        else if(size_ >= MAXSIZE_)
        {
            return replace(elem, count);
        }
        else
        {
            return additem(elem, count);
        }
    }

    bool get(std::list<ElemType>& elem, std::list<CountType>& count)
    {
        BucketT* bp = bs_->next_;
        while(bp)
        {
            ItemT* i=bp->head_;
            while(i)
            {
                elem.push_front(i->elem_);
                count.push_front(i->b_->c_);
                i=i->next_;
            }
            bp=bp->next_;
        }
        return true;
    }
    bool get(std::list<std::pair<ElemType, CountType> >& topk)
    {
        BucketT* bp = bs_->next_;
        while(bp)
        {
            ItemT* i=bp->head_;
            while(i)
            {
                topk.push_front(make_pair(i->elem_, i->b_->c_));
                i=i->next_;
            }
            bp=bp->next_;
        }
        return true;
    }
    bool get(CountType k, std::list<std::pair<ElemType, CountType> >& topk)
    {
        BucketT* bp = end_;
        CountType tempk=0;
        while(bp!=bs_)
        {
            ItemT* i=bp->head_;
            while(i)
            {
                topk.push_back(make_pair(i->elem_, i->b_->c_));
                tempk++;
                if(tempk >= k || tempk >= size_)break;
                i=i->next_;
            }
            if(tempk >= k || tempk >= size_)break;
            bp=bp->prev_;
        }
        return true;
    }
private:
    bool update(ElemType elem, CountType count)
    {
        ItemT* i = gps_[elem];
        BucketT* bp = i->b_;
        count = bp->c_+1;

        if(bp->size_==1 && (!(bp->next_) || bp->next_->c_ > count))
        {
            bp->c_++;
            th_ = bs_->next_->c_;
            return true;
        }
        bp->erase(i);
        if(!(bp->next_))
        {
            bp->next_=new BucketT(count, bp, NULL);
            end_=bp->next_;
        }
        else if(bp->next_->c_ > count)
        {
            BucketT* tp=new BucketT(count, bp, bp->next_);
            bp->next_=tp;
            tp->next_->prev_=tp;
        }
        bp->next_->insert(i);

        if(bp->size_==0)
        {
            bp->prev_->next_=bp->next_;
            bp->next_->prev_=bp->prev_;
            bp->next_=bp->prev_=NULL;
            delete bp;
        }
        return true;
    }
    bool replace(ElemType elem, CountType count)
    {
        count = bs_->next_->c_+1;
        ItemT* i = bs_->next_->end_;
        gps_.erase(gps_.find(i->elem_));
        gps_[elem] = i;
        i->elem_=elem;
        return update(elem,count);
    }
    bool additem(ElemType elem, CountType count)
    {
        count=1;
        if(bs_->next_==NULL)
        {
            bs_->next_=new BucketT(count, bs_, NULL);
            end_=bs_->next_;
        }

        BucketT* bp=bs_->next_;
        ItemT* i=new ItemT(elem, bp, NULL, NULL);

        if(bp->c_ == count)
        {
            bp->insert(i);
        }
        else
        {
            BucketT* nbp = new BucketT(count, bs_, bs_->next_);
            bs_->next_ = nbp;
            nbp->next_->prev_=nbp;
            nbp->insert(i);
        }

        size_++;
        gps_[elem]=i;
        th_=bs_->next_->c_;
        return true;
    }

    CountType MAXSIZE_;
    //current size
    CountType size_;
    //threshold
    CountType th_;
    BucketT* bs_;
    BucketT* end_;
    boost::unordered_map<ElemType, ItemT* > gps_;
};

} //end of namespace util
} //end of namespace izenelib

#endif
