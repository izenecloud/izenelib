/**
   @file cache_strategy.h
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef CACHE_STRATEGY_H
#define CACHE_STRATEGY_H

#include <time.h>

/**
 *@class CachePolicyLRU Latest rare used out
 **/
class CachePolicyLRU
{
    time_t time_;
public:
    CachePolicyLRU()
    {
        time_ = time(NULL);
    }

    int compare(const CachePolicyLRU& t)const
    {
        return time_ - t.time_;
    }

    void visit()
    {
        time_ = time(NULL);
    }


    friend ostream& operator << ( ostream& os, const CachePolicyLRU& inf)
    {
        os<<"time: "<<inf.time_<<endl;
        return os;
    }

}
;


/**
 *@class CachePolicyLU Latest used out
 **/
class CachePolicyLU//least used
{
    uint64_t visit_count_;

public:
    CachePolicyLU()
    {
        visit_count_ = 0;
    }

    int compare(const CachePolicyLU& t)const
    {
        return visit_count_ - t.visit_count_;
    }

    void visit()
    {
        visit_count_++;
    }

    friend ostream& operator << ( ostream& os, const CachePolicyLU& inf)
    {
        os<<"visited: "<<inf.visit_count_<<endl;
        return os;
    }


}
;


/**
 *@class CachePolicyLARU least and rarest used out
 **/
class CachePolicyLARU//least and rarest used
{
    uint64_t visit_count_;
    time_t time_;

public:
    CachePolicyLARU()
    {
        visit_count_ = 1;
        time_ = time(NULL);
    }

    int compare(const CachePolicyLARU& t)const
    {
        //cout<<time_<<" "<<t.time_<<"  "<<visit_count_<<"  "<<t.visit_count_<<endl;

        return (visit_count_+time_*1000) - (t.visit_count_ + t.time_*1000);
    }

    void visit()
    {
        time_ = time(NULL);
        visit_count_++;
    }

    friend ostream& operator << ( ostream& os, const CachePolicyLARU& inf)
    {
        os<<"time: "<<inf.time_<<"  visited: "<<inf.visit_count_<<endl;
        return os;
    }


}
;
#endif
