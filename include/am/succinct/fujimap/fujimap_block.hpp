/*
 * fujimapBlock.hpp
 * Copyright (c) 2010 Daisuke Okanohara All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FUJIMAP_BLOCK_HPP__
#define FUJIMAP_BLOCK_HPP__

#include <vector>
#include <string>
#include <fstream>

#include "keyedge.hpp"
#include "bitvec.hpp"
#include "fujimap_common.hpp"

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/*
 * Minimum Perfect Associative Array
 * used in Fujimap
 */
class FujimapBlock
{
    static const double   C_R; ///< Redundancy for bit array (>1.3)
    static const uint64_t intercept;

public:
    FujimapBlock(); ///< Default Constructor
    ~FujimapBlock(); ///< Default Destructor

    int build(std::vector<KeyEdge>& keyEdges,
              const uint64_t seed, const uint64_t fpLen,
              const EncodeType et); ///< build an associative map
    uint64_t getVal(const KeyEdge& ke) const; ///< return a value corresponding to the given KeyEdge
    void save(std::ofstream& ofs); ///< save the status in ofs
    void load(std::ifstream& ifs); ///< load the status from ifs

    uint64_t getSeed() const;

    size_t getKeyNum() const; ///<return the number of registered keys
    size_t getBSize() const;
    size_t getWorkingSize() const; ///<return the current working size

private:
    void test();

    BitVec B_;

    uint64_t keyNum_;
    uint64_t minCodeVal_;
    uint64_t maxCodeVal_;
    uint64_t maxCodeLen_;
    uint64_t fpLen_;
    uint64_t seed_;
    uint64_t bn_;
    EncodeType et_;
};

}}

NS_IZENELIB_AM_END

#endif // FUJIMAP_BLOCK_HPP__
