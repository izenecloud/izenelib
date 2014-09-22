/*
 *  Copyright (c) 2011 Daisuke Okanohara
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */


#ifndef IZENELIB_AM_SUCCINCT_DAG_VECTOR_COMP_VECTOR_HPP_
#define IZENELIB_AM_SUCCINCT_DAG_VECTOR_COMP_VECTOR_HPP_

#include <map>
#include "dag_vector.hpp"

NS_IZENELIB_AM_BEGIN

namespace succinct{

/**
 * Compressed Vector using Direct Accessible Gamma code Vector
 */
template <class T>
class comp_vector
{
public:

    /**
     * Constructor
     */
    comp_vector() : auto_remap_(true)
    {
    }

    /**
     * Destructor
     */
    ~comp_vector()
    {
    }

    /**
     * Return the number of elements
     * @return the number of elements.
     */
    size_t size() const
    {
        return dagv_.size();
    }

    /**
     * Return the number of distict elements
     * @return the number of distinct elements.
     */
    size_t alpha_num() const
    {
        return val2t_.size();
    }

    /**
     * Add an element to the end of the vector
     * @param t an element to be added
     */
    void push_back(const T& t)
    {
        dagv_.push_back(get_val(t));
        std::vector<std::pair<uint64_t, uint64_t> > counter;
        if (need_remap(counter))
        {
            remap(counter);
        }
    }

    /**
     * Lookup the ind-th element
     * @param ind index
     * @return the ind-th element
     */
    const T& operator[] (uint64_t ind) const
    {
        return val2t_[dagv_[ind]];
    }

    /**
     * Reassign the internal gamma code to shrink the working space.
     * The elements in the vector are not changed after this operation.
     */
    void remap(std::vector<std::pair<uint64_t, uint64_t> >& counter)
    {
        std::vector<uint64_t> old2new;
        get_old2new(counter, old2new);

        remap_dagv(old2new);
        remap_t2val(old2new);
        remap_val2t(old2new);
    }

    /**
     * Get the current auto_remap configuration.
     * @return auto_remap status.
     */
    bool auto_remap() const
    {
        return auto_remap_;
    }

    /**
     * Set the auto_remap configuration.
     * @param auto_remap to be set
     */
    void set_auto_remap(bool auto_remap)
    {
        auto_remap_ = auto_remap;
    }

    /**
     * Return the working space in bytes
     */
    uint64_t get_alloc_byte_num() const
    {
        return dagv_.get_alloc_byte_num();
    }

    /**
     * Return the distinct elements
     */
    const std::vector<T>& val2t() const
    {
        return val2t_;
    }

    class const_iterator : public std::iterator<std::random_access_iterator_tag, T, size_t>
    {
    public:
        const_iterator(const comp_vector& v) :
                dvit_(v.dagv_.begin()), val2t_(v.val2t_)
        {
        }

        const_iterator(const comp_vector& v, size_t ) :
                dvit_(v.dagv_.end()), val2t_(v.val2t_)
        {
        }

        const_iterator& operator++()
        {
            ++dvit_;
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator tmp(*this);
            ++*this;
            return tmp;
        }

        const_iterator& operator--()
        {
            --dvit_;
            return *this;
        }

        const_iterator operator--(int)
        {
            const_iterator tmp(*this);
            --*this;
            return tmp;
        }

        bool operator==(const const_iterator& it) const
        {
            return dvit_ == it.dvit_;
        }

        bool operator!=(const const_iterator& it) const
        {
            return !(*this == it);
        }

        size_t operator-(const const_iterator& it) const
        {
            return dvit_ - it.dvit_;
        }

        const T& operator*() const
        {
            return val2t_[*dvit_];
        }

    private:
        typename dag_vector::const_iterator dvit_;
        const std::vector<T>& val2t_;
    };

    const_iterator begin() const
    {
        return const_iterator(*this);
    }

    const_iterator end() const
    {
        return const_iterator(*this, size());
    }

private:
    static constexpr uint64_t min_need_remap_size_ = 4096;
    static constexpr float shrink_rate_ = 0.7;

    bool need_remap(std::vector<std::pair<uint64_t, uint64_t> >& counter)
    {
        if (!auto_remap_) return false;
        uint64_t size = dagv_.size();
        if (size < min_need_remap_size_)
        {
            return false;
        }
        if (size != (size & -size))
        {
            return false;
        }

        get_counter(counter);
        uint64_t old_len = 0;
        uint64_t new_len = 0;
        for (size_t i = 0; i < counter.size(); ++i)
        {
            new_len += counter[i].first * (2 * dag_vector::binary_len(i+1) - 1);
            old_len += counter[i].first * (2 * dag_vector::binary_len(counter[i].second+1) - 1);
        }

        if ((float)new_len >= (float)old_len * shrink_rate_ )
        {
            return false;
        }

        return true;
    }

    void get_counter(std::vector<std::pair<uint64_t, uint64_t> >& counter)
    {
        counter.clear();
        counter.resize(t2val_.size());
        for (size_t i = 0; i < counter.size(); ++i)
        {
            counter[i].second = i;
        }

        for (dag_vector::const_iterator it = dagv_.begin();
                it != dagv_.end(); ++it)
        {
            counter[*it].first++;

        }
        sort(counter.rbegin(), counter.rend());
    }



    uint64_t get_val(const T& t)
    {
        typename std::map<T, uint64_t>::const_iterator it = t2val_.find(t);
        if (it != t2val_.end())
        {
            return it->second;
        }
        uint64_t newID = static_cast<uint64_t>(t2val_.size());
        t2val_[t] = newID;
        val2t_.push_back(t);
        return newID;
    }

    void get_old2new(const std::vector<std::pair<uint64_t, uint64_t> >& counter,
                     std::vector<uint64_t>& old2new)
    {
        old2new.resize(t2val_.size());
        for (size_t i = 0; i < counter.size(); ++i)
        {
            old2new[counter[i].second] = i;
        }
    }

    void remap_dagv(const std::vector<uint64_t>& old2new)
    {
        dag_vector new_dagv;
        for (dag_vector::const_iterator it = dagv_.begin();
                it != dagv_.end(); ++it)
        {
            new_dagv.push_back(old2new[*it]);
        }
        dagv_.swap(new_dagv);
    }

    void remap_t2val(const std::vector<uint64_t>& old2new)
    {
        for (typename std::map<T, uint64_t>::iterator it = t2val_.begin();
                it != t2val_.end(); ++it)
        {
            it->second = old2new[it->second];
        }
    }

    void remap_val2t(const std::vector<uint64_t>& old2new)
    {
        std::vector<T> new_val2t(val2t_.size());
        for (size_t i = 0; i < val2t_.size(); ++i)
        {
            new_val2t[old2new[i]] = val2t_[i];
        }
        val2t_.swap(new_val2t);
    }

    dag_vector dagv_;             /// direct accessible gamma code vector
    std::map<T, uint64_t> t2val_; /// mapping from an element to an internal code
    std::vector<T> val2t_;        /// mapping from an internal code to an element
    bool auto_remap_;             /// if auto_remap_ = true, then remap() will be called every time the size is doubled
};


}
NS_IZENELIB_AM_END

#endif // IZENELIB_AM_SUCCINCT_DAG_VECTOR_COMP_VECTOR_HPP_
