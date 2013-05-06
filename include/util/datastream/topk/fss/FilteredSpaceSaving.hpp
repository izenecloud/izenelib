/*
 *     @file   FilteredSpaceSaving.hpp
 *     @author Xiaoxing Wu
 *     @date   2013.05.06
 *     @       filtered space saving for topk
 */

#ifndef IZENELIB_UTIL_FILTEREDSPACESAVING_H_
#define IZENELIB_UTIL_FILTEREDSPACESAVING_H_

#include <iostream>
#include <list>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>

namespace izenelib{ namespace util {

template <typename ElemType, typename IntType>
class Counter {
public:
    Counter(const ElemType & id, const IntType & err)
        : id_(id), err_(err)
    {
    }

    ElemType id_;
    IntType err_;
};

template <typename ElemType, typename IntType>
class LessEqual {
public:
    LessEqual(Counter<ElemType, IntType> *c)
        : c_(c)
    {
    }

    bool operator()(Counter<ElemType, IntType> *const & lhs)
    {
        return lhs->err_ <= c_->err_;
    }
private:
    Counter<ElemType, IntType> *c_;
};

template <typename ElemType, typename IntType>
class MonitoredList {
public:
    typedef std::list<std::pair<IntType, std::list<Counter<ElemType, IntType> *> *> > BucketsT;
    typedef std::list<Counter<ElemType, IntType> *> CountersT;
    typedef boost::unordered_map<ElemType, std::pair<typename BucketsT::iterator, typename CountersT::iterator> > GpsT;

    MonitoredList(int h, int max_size)
        : current_size_(0), max_size_(max_size), h_(h), alpha_(h, 0), c_(h, 0), hash_()
    {
    }

    ~MonitoredList()
    {
        for (typename GpsT::iterator i = gps_.begin(); i != gps_.end(); ++i) {
            delete *((i->second).second);
        }
        for (typename BucketsT::iterator i = ss_.begin(); i != ss_.end(); ++i) {
            delete i->second;
        }
    }

    void insert(const ElemType & id)
    {
        // if the id is monitored already
        typename GpsT::iterator i = gps_.find(id);
        if (i != gps_.end()) {
            typename CountersT::iterator col = (i->second).second;
            typename BucketsT::iterator row = (i->second).first;
            typename BucketsT::iterator next_row = row; ++next_row;
            IntType count = row->first;
            CountersT & l = *(row->second);

            if (next_row == ss_.end() || next_row->first > count + 1) {
                typename BucketsT::iterator new_row = ss_.insert(next_row, make_pair(count + 1, new CountersT));
                CountersT & new_l = *(new_row->second);
                new_l.splice(new_l.begin(), l, col);

                (i->second).first = new_row;
                (i->second).second = new_l.begin();
            } else {
                CountersT & next_l = *(next_row->second);
                typename CountersT::iterator insert_pos = std::find_if(next_l.begin(), next_l.end(), LessEqual<ElemType, IntType>(*col));
                next_l.splice(insert_pos, l, col);

                (i->second).first = next_row;
                (i->second).second = --insert_pos;
            }

            if (l.empty()) {
                delete row->second;
                ss_.erase(row);
            }

            return;
        }

        // filter out low count element
        int target_bucket = hash_(id) % h_;
        IntType min = ((current_size_ == 0) ? 0 : ((ss_.begin())->first));

        if (alpha_[target_bucket] + 1 < min) {
            alpha_[target_bucket] += 1;
            return;
        }

        if (current_size_ < max_size_) {
            ++current_size_;

            typename BucketsT::iterator insert_pos = ss_.begin();
            while (insert_pos != ss_.end() && insert_pos->first < alpha_[target_bucket] + 1) {
                ++insert_pos;
            }
            if (insert_pos == ss_.end() || insert_pos->first > alpha_[target_bucket] + 1) {
                typename BucketsT::iterator newBucket = ss_.insert(insert_pos, make_pair(alpha_[target_bucket] + 1, new CountersT));
                CountersT & new_l = *(newBucket->second);
                gps_.insert(make_pair(id, make_pair(newBucket, new_l.insert(new_l.begin(), new Counter<ElemType, IntType>(id, alpha_[target_bucket])))));
            } else {
                CountersT & match_l = *(insert_pos->second);
                gps_.insert(make_pair(id, make_pair(insert_pos, match_l.insert(match_l.begin(), new Counter<ElemType, IntType>(id, alpha_[target_bucket])))));
            }
            c_[target_bucket] += 1;
        } else {
            typename BucketsT::iterator firstRow = ss_.begin();
            CountersT & first_l = *(firstRow->second);
            Counter<ElemType, IntType> *c = *(first_l.begin());

            int replace_bucket = hash_(c->id_) % h_;
            c_[replace_bucket] -= 1;
            alpha_[replace_bucket] = firstRow->first;

            typename BucketsT::iterator insert_pos = firstRow;
            while (insert_pos != ss_.end() && insert_pos->first < alpha_[target_bucket] + 1) {
                ++insert_pos;
            }

            gps_.erase(c->id_);

            c->id_ = id;
            c->err_ = alpha_[target_bucket];

            if (insert_pos == ss_.end() || insert_pos->first > alpha_[target_bucket] + 1) {
                typename BucketsT::iterator newRow = ss_.insert(insert_pos, make_pair(alpha_[target_bucket] + 1, new CountersT));
                CountersT & new_l = *(newRow->second);
                new_l.splice(new_l.begin(), first_l, first_l.begin());

                gps_.insert(make_pair(id, make_pair(newRow, new_l.begin())));
            } else {
                CountersT & match_l = *(insert_pos->second);
                match_l.splice(match_l.begin(), first_l, first_l.begin());

                gps_.insert(make_pair(id, make_pair(insert_pos, match_l.begin())));
            }
            c_[target_bucket] += 1;

            if (first_l.empty()) {
                delete firstRow->second;
                ss_.erase(firstRow);
            }
        }
    }

    void show()
    {
        for (typename BucketsT::reverse_iterator i = ss_.rbegin(); i != ss_.rend(); ++i) {
            std::cout << i->first << ':' << i->second->size() << std::endl;
            for (typename CountersT::iterator j = i->second->begin(); j != i->second->end(); ++j) {
                std::cout << (*j)->id_ << '\t' << (*j)->err_ << std::endl;
            }
        }
        std::cout << "size:" << current_size_ << std::endl;

        for (int i = 0; i < h_; ++i) {
            std::cout << alpha_[i] << ' ';
        }
        std::cout << std::endl;

        for (int i = 0; i < h_; ++i) {
            std::cout << c_[i] << ' ';
        }
        std::cout << std::endl;

        std::cout << std::endl;
    }

private:
    BucketsT ss_;
    int current_size_;
    int max_size_;

    GpsT gps_;

    int h_;
    std::vector<IntType> alpha_;
    std::vector<int> c_;

    boost::hash<ElemType> hash_;
};

}
}

#endif
