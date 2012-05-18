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

#ifndef IZENELIB_AM_SUCCINCT_DAG_VECTOR_SPARSE_MAP_HPP_
#define IZENELIB_AM_SUCCINCT_DAG_VECTOR_SPARSE_MAP_HPP_

#include "sparse_set.hpp"

NS_IZENELIB_AM_BEGIN

namespace succinct{

/**
 * Sparse Map
 */
template<class T>
class sparse_map
{
public:

    /**
     * Constructor
     */
    sparse_map()
    {
    }

    /**
     * Constructor given low_width
     * @param low_width the width of low array
     */
    sparse_map(uint64_t low_width) :
            indicies_(low_width)
    {
    }

    /**
     * Constructor given one_num and num
     * @param one_num the number of expected ones in the array
     * @param num the number of expected bits in the array
     */
    sparse_map(uint64_t num, uint64_t size) :
            indicies_(num, size)
    {
    }

    /**
     * Desctructor
     */
    ~sparse_map()
    {
    }

    /**
     * Clear sparse vector
     */
    void clear()
    {
        indicies_.clear();
        std::vector<T>().swap(values_);
    }

    /**
     * Add element to the end of the array
     * @param val an element to be added
     */
    void push_back(uint64_t pos, const T& t)
    {
        indicies_.push_back(pos);
        values_.push_back(t);
    }

    /**
     * Return the position of (rank+1)-th one in the array
     * @param rank the rank of the position
     * @return the position of (ran+1)-th one in the array
     */
    T operator[] (uint64_t pos) const
    {
        return values_[indicies_.rank(pos)];
    }

    uint64_t get_pos(uint64_t ind) const
    {
        return indicies_[ind];
    }

    /**
     * Return the position of (rank+1)-th one in the array
     * @param rank the rank of the position
     * @return the position of (ran+1)-th one in the array
     */
    T loopup_with_check (uint64_t pos, const T& t) const
    {
        if (!indicies_.exist(pos))
        {
            return t;
        }
        return values_[indicies_.rank(pos)];
    }


    /**
     * Return the pos-th bit
     * @param pos the position in the array
     * @return the bit at the pos-th position.
     */
    bool exist(uint64_t pos) const
    {
        return indicies_.exist(pos);
    }

    /**
     *
     */
    uint64_t num() const
    {
        return values_.size();
    }

    uint64_t size() const
    {
        return indicies_.size();
    }

    uint64_t get_alloc_byte_num() const
    {
        return indicies_.get_alloc_byte_num()
               + sizeof(T) * values_.size();
    }

    void swap(sparse_map& sm)
    {
        indicies_.swap(sm.indicies_);
        values_.swap(sm.values_);
    }

    class const_iterator : public std::iterator<std::random_access_iterator_tag, T, size_t>
    {
    public:
        const_iterator(const sparse_map<T>& sm) :
                ssit_(sm.indicies_.begin()),
                vit_(sm.values_.begin())
        {
        }

        const_iterator(const sparse_map<T>& sm, uint64_t) :
                ssit_(sm.indicies_.end()),
                vit_(sm.values_.begin())
        {
        }

        const_iterator& operator++()
        {
            ++ssit_;
            ++vit_;
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
            --ssit_;
            --vit_;
            return *this;
        }

        const_iterator operator--(int)
        {
            const_iterator tmp(*this);
            --*this;
            return tmp;
        }

        size_t operator=(const const_iterator& it) const
        {
            return vit_ - it.vit_;
        }

        bool operator==(const const_iterator& it) const
        {
            return vit_ == it.vit_;
        }

        bool operator!=(const const_iterator& it) const
        {
            return !(*this == it);
        }

        std::pair<uint64_t, T> operator*() const
        {
            return std::make_pair(*ssit_, *vit_);
        }

    private:
        sparse_set::const_iterator ssit_;
        typename std::vector<T>::const_iterator vit_;
    };

    const_iterator begin() const
    {
        return const_iterator(*this);
    }

    const_iterator end() const
    {
        return const_iterator(*this, 0);
    }

private:
    sparse_set indicies_;
    std::vector<T> values_;
};

}
NS_IZENELIB_AM_END

#endif // IZENELIB_AM_SUCCINCT_DAG_VECTOR_SPARSE_MAP_HPP_
