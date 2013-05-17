#ifndef IZENELIB_UTIL_DATASTREAM_LOSSY_COUNTER_HPP
#define IZENELIB_UTIL_DATASTREAM_LOSSY_COUNTER_HPP

#include <iostream>
#include <cstdlib>
#include <map>
#include <string>

namespace izenelib { namespace util { 

class LossyCounter
{
    std::map<std::string,std::pair<int,int> > S_;
    int N_;
    double gamma_, epsilon_;
    int b_current_;

    void next_backet()
    {
        std::map<std::string,std::pair<int,int> >::iterator it = S_.begin();
        while(it!=S_.end())
        {
            double f = ((*it).second).first;
            double d = ((*it).second).second;
            if(f <= b_current_ - d)
            {
                S_.erase(it++);
            }
            else
            {
                ++it;
            }
        }
        b_current_++;
    }
public:
    LossyCounter(double gamma, double epsilon):gamma_(gamma),epsilon_(epsilon)
    {
        N_ = 0;
        b_current_ = 1;
    }
    void add(const std::string &str)
    {
        if(S_.count(str)==0)
        {
            S_[str] = std::make_pair(1,b_current_-1);
        }
        else
        {
            S_[str].first++;
        }
        N_++;
        if(N_%((int)(1.0/epsilon_))==0)
        {
            next_backet();
        }
    }
    int get(const std::string &str)
    {
        if(S_.count(str)==0) return -1;
        double f = S_[str].first;
        if(f < (gamma_ - epsilon_) * N_) return -1;
        return S_[str].first;
    }
    void show()
    {
        std::map<std::string,std::pair<int,int> >::iterator it = S_.begin();
        while(it!=S_.end())
        {
            std::cout << (*it).first << "\t" << ((*it).second).first << std::endl;
            ++it;
        }
    }
    int size()
    {
        return S_.size();
    }
};

}}
#endif
