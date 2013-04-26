/*
 *  @file    SpaceSaving
 *  @author  Kuilong Liu
 *  @date    2013.04.26
 *           Space Saving algorithm,
 *           which is used as a monitored list in filtered space saving
 *
 *           The space saving algorithm has been improved.
 *           Now, each operation(insert, get) is O(1)
 *           And when we fetch top 1000 frequent items, we can guarantee that top
 *           100 frequent items in the results have no error.
 */
#ifndef IZENELIB_UTIL_SPACESAVING_H_
#define IZENELIB_UTIL_SPACESAVING_H_

#include <list>
#include <iostream>
#include <boost/unordered_map.hpp>
using namespace std;

namespace izenelib { namespace util {

template<typename ElemType, typename CountType, typename ErrorType>
class MonBucket;

template<typename ElemType, typename CountType, typename ErrorType>
class MonItem
{
public:
    typedef MonBucket<ElemType, CountType, ErrorType> BucketT;
    typedef MonItem<ElemType, CountType, ErrorType> ItemT;

    MonItem(ElemType elem, ErrorType err, BucketT* b, ItemT* p, ItemT* n)
        :elem_(elem), error_(err), b_(b), prev_(p), next_(n)
    {
    }
    ~MonItem()
    {
        b_=NULL;
        if(next_)delete next_;
    }

    ElemType elem_;
    ErrorType error_;
    BucketT* b_;
    ItemT* prev_;
    ItemT* next_;
};

template<typename ElemType, typename CountType, typename ErrorType>
class MonBucket
{
public:
    typedef MonItem<ElemType, CountType, ErrorType> ItemT;
    typedef MonBucket<ElemType, CountType, ErrorType> BucketT;

    MonBucket(CountType c, BucketT* p, BucketT* n)
        :size_(0), c_(c), head_(NULL), end_(NULL), prev_(p), next_(n)
    {
    }
    ~MonBucket()
    {
        if(head_) delete head_;
        if(next_) delete next_;
    }

    bool insert(ItemT* i)
    {
        i->b_=this;
        size_++;
        if(size_==1)
        {
            head_=end_=i;
            i->prev_=i->next_=NULL;
            return true;
        }
        if(head_->error_ >= i->error_)
        {
            i->next_=head_;
            i->prev_=NULL;
            head_->prev_=i;
            head_=i;
            return true;
        }

        if(end_->error_ < i->error_)
        {
            i->prev_=end_;
            i->next_=NULL;
            end_->next_=i;
            end_=i;
            return true;
        }

        ItemT* p = head_;
        do
        {
            if(i->error_ <= p->next_->error_)
            {
                i->next_=p->next_;
                p->next_->prev_=i;
                i->prev_=p;
                p->next_=i;
                return true;
            }
            p=p->next_;
        }while(true);
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

template<typename ElemType, typename CountType, typename ErrorType>
class MonitoredList
{
public:
    typedef MonBucket<ElemType, CountType, ErrorType> BucketT;
    typedef MonItem<ElemType, CountType, ErrorType> ItemT;

    MonitoredList(CountType m)
        :MAXSIZE_(m),
        size_(0)
    {
        bs_ = new BucketT(0, NULL, NULL);
    }
    ~MonitoredList()
    {
        if(bs_)delete bs_;
        gps_.clear();
    }
    bool reset()
    {
        if(bs_->next_) delete bs_->next_;
        gps_.clear();
        size_=0;
        return true;
    }
    bool insert(ElemType elem)
    {
        if(gps_.find(elem) != gps_.end())
        {
            return update(elem);
        }
        if(size_ < MAXSIZE_)
        {
            return additem(elem);
        }
        return  replace(elem);
    }

    bool get(std::list<ElemType>& elems, std::list<CountType>& counts, std::list<ErrorType>& errs)
    {
        BucketT* bp = bs_->next_;
        while(bp)
        {
            ItemT* i=bp->end_;
            while(i)
            {
                elems.push_front(i->elem_);
                counts.push_front(i->b_->c_);
                errs.push_front(i->error_);
                i=i->prev_;
            }
            bp=bp->next_;
        }
        return true;
    }
    void show()
    {
        BucketT* bp = bs_->next_;
        cout <<"*********************************************************"<<endl;
        while(bp)
        {
            cout <<" Bucket[ " <<bp->c_<<"]: ";
            ItemT* i=bp->head_;
            while(i)
            {
                cout<<" (" <<i->elem_ <<", " <<i->error_<<") ";
                i=i->next_;
            }
            cout << endl;
            bp=bp->next_;
        }
    }
private:
    bool update(ElemType elem)
    {
        ItemT* i = gps_[elem];
        BucketT* bp = i->b_;
        CountType count = bp->c_+1;
        if(bp->size_==1 && (!(bp->next_) || bp->next_->c_ > count))
        {
            bp->c_++;
            return true;
        }

        bp->erase(i);
        if(!(bp->next_))
            bp->next_=new BucketT(count, bp, NULL);
        else if(bp->next_->c_ > count)
        {
            BucketT* tp = new BucketT(count, bp, bp->next_);
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
    bool replace(ElemType elem)
    {
        ItemT* i = bs_->next_->end_;
        gps_.erase(gps_.find(i->elem_));
        gps_[elem] = i;
        i->elem_=elem;
        i->error_=i->b_->c_;
        return update(elem);
    }
    bool additem(ElemType elem)
    {
        CountType count = 1;
        if(bs_->next_==NULL)
        {
            bs_->next_=new BucketT(count, bs_, NULL);
        }

        BucketT* bp=bs_->next_;
        ItemT* i = new ItemT(elem, 0, bp, NULL, NULL);

        if(bp->c_==count)
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
        return true;
    }

    CountType MAXSIZE_;
    //current size
    CountType size_;

    BucketT* bs_;
    boost::unordered_map<ElemType, ItemT* > gps_;
};

} //end of namespace util
} //end of namespace izenelib
#endif
