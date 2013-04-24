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

using namespace std;
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
    }

    ~TopKEstimation()
    {
    }

    bool update(ElemType elem, CountType count)
    {
//        cout << "update elem: " <<elem <<" count: " << count << endl;
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
            bp->next_=new BucketT(count, bp, NULL);
        bp->next_->insert(i);

        if(bp->size_==0)
        {
            bp->prev_->next_=bp->next_;
            bp->next_->prev_=bp->prev_;
            delete bp;
        }
        return true;
    }
    bool replace(ElemType elem, CountType count)
    {
        count = bs_->next_->c_+1;
        ItemT* i = bs_->next_->end_;
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
                elem.push_back(i->elem_);
                count.push_back(i->b_->c_);
                i=i->next_;
            }
            bp=bp->next_;
        }
        return true;
    }
    CountType MAXSIZE_;
    //current size
    CountType size_;
    //threshold
    CountType th_;
    BucketT* bs_;
    boost::unordered_map<ElemType, ItemT* > gps_;
};

} //end of namespace util
} //end of namespace izenelib

#endif
