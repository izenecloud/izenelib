/*
 *  Copyright (c) 2010 Daisuke Okanohara
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

#ifndef SDARRAY_SDARRAY_HPP__
#define SDARRAY_SDARRAY_HPP__

#include <types.h>

#include <vector>
#include <iostream>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace sdarray
{

class SDArray
{
    static const size_t BLOCK_SIZE;

public:
    enum
    {
        NOTFOUND = 0xFFFFFFFFFFFFFFFFLLU
    };

    SDArray();
    ~SDArray();

    /*
     * Add new item to the last
     * val >= 0
     */
    void add(size_t val);

    /*
     * Build an index. This build should be called before prefixSum(), prefixSumLookup(), and find().
     */
    void build();

    void clear();

    /*
     * @ret vals_[0]+vals_[1]+...+vals_[pos-1]
     */
    size_t prefixSum(size_t pos) const;

    /*
     * @ret vals_[0]+vals_[1]+...+vals_[pos-1] and set vals_[pos] to val
     */
    size_t prefixSumLookup(size_t pos, size_t& val) const;

    size_t getVal(size_t pos) const;

    /*
     * @ret Return ind s.t. prefixSum(ind) <= val < prefixSum(ind+1) or NOTFOUND if not exist
     */
    size_t find(size_t val) const;

    size_t getSum() const;

    size_t size() const;
    size_t allocSize() const;

    void save(std::ostream& os) const;
    void load(std::istream& is);
    void swap(SDArray& other)
    {
        std::swap(size_, other.size_);
        std::swap(sum_, other.sum_);
        Ltable_.swap( other.Ltable_ );
        B_.swap( other.B_ );
        vals_.swap( other.vals_ );
    }

private:
    void packHighs_(size_t begPos, size_t width);
    void packLows_(size_t begPos, size_t width);

    size_t selectBlock_(size_t rank, size_t header) const;
    size_t rankBlock_(size_t val, size_t header) const;
    size_t getLow_(size_t begPos, size_t num, size_t width) const;
    bool getBit_(size_t pos) const;
    size_t getBits_(size_t pos, size_t num) const;

private:
    size_t size_;
    size_t sum_;

    std::vector<size_t> Ltable_;
    std::vector<size_t> B_;
    std::vector<size_t> vals_;
};

}
}

NS_IZENELIB_AM_END

#endif // SDARRAY_SDARRAY_HPP__
