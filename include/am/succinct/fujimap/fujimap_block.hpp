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

#include "keyedge.hpp"
#include "bitvec.hpp"

#include <queue>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fujimap
{

/*
 * Minimum Perfect Associative Array
 * used in Fujimap
 */
template <class ValueType>
class FujimapBlock
{
    static const double C_R ; ///< Redundancy for bit array (>1.3)
    static const uint64_t intercept = 10;

public:
    FujimapBlock(); ///< Default Constructor
    ~FujimapBlock(); ///< Default Destructor

    int build(std::vector<KeyEdge<ValueType> >& keyEdges,
              const uint64_t seed, const uint64_t fpLen,
              const EncodeType et); ///< build an associative map
    ValueType getVal(const KeyEdge<ValueType>& ke) const; ///< return a value corresponding to the given KeyEdge
    void save(std::ofstream& ofs) const; ///< save the status in ofs
    void load(std::ifstream& ifs); ///< load the status from ifs

    uint64_t getSeed() const;

    size_t getKeyNum() const; ///<return the number of registered keys
    size_t getWorkingSize() const; ///<return the current working size

private:
    void test();

    BitVec<ValueType> B_;

    uint64_t keyNum_;
    ValueType minCodeVal_;
    ValueType maxCodeVal_;
    uint64_t maxCodeLen_;
    uint64_t fpLen_;
    uint64_t seed_;
    uint64_t bn_;
    EncodeType et_;
};

template <class ValueType>
const double FujimapBlock<ValueType>::C_R = 1.3;

template <class ValueType>
FujimapBlock<ValueType>::FujimapBlock()
    : keyNum_(0)
    , minCodeVal_(0)
    , maxCodeVal_(0)
    , maxCodeLen_(0)
    , fpLen_(FPLEN)
    , seed_(0x12345678)
    , bn_(0)
    , et_(BINARY)
{
}

template <class ValueType>
FujimapBlock<ValueType>::~FujimapBlock()
{
}

template <class ValueType>
ValueType FujimapBlock<ValueType>::getVal(const KeyEdge<ValueType>& ke) const
{
    if (B_.bvBitSize() == 0)
    {
        return (ValueType)NOTFOUND;
    }

    uint64_t blockSize = (et_ == BINARY) ? (maxCodeLen_ + fpLen_) : 1;
    if (fpLen_ != 0)
    {
        uint64_t fpCheck = 0;
        for (uint64_t i = 0; i < R; ++i)
        {
            fpCheck ^= B_.get64Bits(ke.get(i, bn_) * blockSize, fpLen_);
        }
        if (fpCheck != FujimapCommon::maskCheckLen(ke.v[0] ^ ke.v[1], fpLen_))
        {
            return (ValueType)NOTFOUND;
        }
    }

    ValueType code = 0;
    for (uint64_t i = 0; i < R; ++i)
    {
        code ^= B_.getBits(ke.get(i, bn_) * blockSize + fpLen_, maxCodeLen_);
    }

    if (et_ == GAMMA)
    {
        code = FujimapCommon::gammaDecode(code);
    }

    if (code > maxCodeVal_)
    {
        return (ValueType)NOTFOUND;
    }

    return code + minCodeVal_;
}

template <class ValueType>
uint64_t FujimapBlock<ValueType>::getSeed() const
{
    return seed_;
}

template <class ValueType>
int FujimapBlock<ValueType>::build(
    std::vector<KeyEdge<ValueType> >& keyEdges,
    const uint64_t seed, const uint64_t fpLen,
    const EncodeType et)
{
    keyNum_ = static_cast<uint64_t>(keyEdges.size());
    seed_   = seed;
    fpLen_  = fpLen;
    et_     = et;

    minCodeVal_ = (ValueType)-1;
    maxCodeVal_ = 0;
    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        if (keyEdges[i].code < minCodeVal_)
        {
            minCodeVal_ = keyEdges[i].code;
        }
    }

    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        keyEdges[i].code -= minCodeVal_;
        if (keyEdges[i].code > maxCodeVal_)
        {
            maxCodeVal_ = keyEdges[i].code;
        }
    }

    uint64_t totalCodeLen = 0;
    if (et_ == BINARY)
    {
        maxCodeLen_  = FujimapCommon::log2(maxCodeVal_);
        totalCodeLen = (maxCodeLen_ + fpLen_) * keyNum_;
        bn_          = (uint64_t)(keyNum_ * C_R / (double)R + intercept);
    }
    else if (et_ == GAMMA)
    {
        for (size_t i = 0; i < keyEdges.size(); ++i)
        {
            totalCodeLen += FujimapCommon::gammaLen(keyEdges[i].code);
        }
        totalCodeLen += fpLen_ * keyNum_;
        maxCodeLen_   = FujimapCommon::gammaLen(maxCodeVal_);
        bn_           = totalCodeLen * C_R / R + intercept; // <= keyNum_ * maxCodeLen_ * C_R / R + 10
    }
    else
    {
        assert(false);
    }

    uint64_t maxCodeFPLen = maxCodeLen_ + fpLen_;
    uint64_t pointsPerKey = (et_ == BINARY) ? 1 : maxCodeFPLen;

    // set_ keyEdge
    uint64_t space = (et_ == BINARY) ? 0 : maxCodeFPLen;
    std::vector<uint8_t> degs(bn_ * R + space);
    std::vector<uint64_t> offset_s(bn_ * R + space + 1);

    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        const KeyEdge<ValueType>& ke(keyEdges[i]);
        uint64_t len = (et_ == BINARY) ? 1 : FujimapCommon::gammaLen(ke.code) + fpLen_;
        for (uint64_t j = 0; j < R; ++j)
        {
            uint64_t t = ke.get(j, bn_);
            for (uint64_t k = 0; k < len; ++k)
            {
                if (degs[t + k] == 0xFF)
                {
                    return -1;
                }
                ++degs[t + k];
            }
        }
    }

    // set_ offset_s
    uint64_t sum = 0;
    for (size_t i = 0; i < degs.size(); ++i)
    {
        offset_s[i] = sum;
        sum += degs[i];
        degs[i] = 0;
    }
    offset_s.back() = sum;

    // set_ edges
    uint64_t totalEdgeNum = (et_ == BINARY) ? keyNum_ * R : totalCodeLen * R;
    std::vector<uint64_t> edges(totalEdgeNum);
    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        const KeyEdge<ValueType>& ke(keyEdges[i]);
        uint64_t len = (et_ == BINARY) ? 1 : FujimapCommon::gammaLen(ke.code) + fpLen_;
        for (uint64_t j = 0; j < R; ++j)
        {
            uint64_t t = ke.get(j, bn_);
            for (uint64_t k = 0; k < len; ++k)
            {
                edges[offset_s[t + k] + degs[t + k]++] = i * pointsPerKey + k;
            }
        }
    }

    // init q
    std::queue<uint64_t> q;
    for (size_t i = 0; i < degs.size(); ++i)
    {
        if (degs[i] == 1)
        {
            q.push(edges[offset_s[i]]);
        }
    }

    std::vector<std::pair<uint64_t, uint8_t> > extractedEdges;
    uint64_t assignNum = keyNum_ * pointsPerKey;

    BitVec<uint64_t> visitedEdges(assignNum);
    uint64_t deletedNum = 0;
    while (!q.empty())
    {
        uint64_t v = q.front();
        q.pop();

        if (visitedEdges.getBit(v)) continue;
        visitedEdges.setBit(v);
        ++deletedNum;

        uint64_t keyID  = v / pointsPerKey;
        uint64_t offset = v % pointsPerKey;

        const KeyEdge<ValueType>& ke(keyEdges[keyID]);
        int choosed = -1;
        for (uint64_t i = 0; i < R; ++i)
        {
            const uint64_t t = ke.get(i, bn_);
            --degs[t + offset];

            if (degs[t + offset] == 0)
            {
                choosed = i;
                continue;
            }
            else if (degs[t + offset] >= 2)
            {
                continue;
            }
            // degs[t] == 1
            const uint64_t end = offset_s[t + offset + 1];
            for (uint64_t j = offset_s[t + offset]; j < end; ++j)
            {
                if (!visitedEdges.getBit(edges[j]))
                {
                    q.push(edges[j]);
                    break;
                }
            }
        }
        assert(choosed != -1);
        extractedEdges.push_back(std::make_pair(v, choosed));
    }

    if (et_ == BINARY && deletedNum != keyNum_)
    {
        return -1;
    }
    else if (et_ == GAMMA && deletedNum != totalCodeLen)
    {
        return -1;
    }

    assert(q.empty());

    if (et_ == BINARY)
    {
        B_.resize(bn_ * maxCodeFPLen * R);
    }
    else if (et_ == GAMMA)
    {
        B_.resize(bn_ * R + space);
    }
    else
    {
        assert(false);
    }

    uint64_t blockSize = (et_ == BINARY ) ? maxCodeFPLen : 1;
    BitVec<uint64_t> visitedVerticies(assignNum * R);
    std::reverse(extractedEdges.begin(), extractedEdges.end());
    for (std::vector<std::pair<uint64_t, uint8_t> >::const_iterator it =
                extractedEdges.begin(); it != extractedEdges.end(); ++it)
    {
        const uint64_t v = it->first;

        uint64_t keyID  = v / pointsPerKey;
        uint64_t offset = v % pointsPerKey;

        const KeyEdge<ValueType>& ke(keyEdges[keyID]);

        uint64_t signature = FujimapCommon::maskCheckLen(ke.v[0] ^ ke.v[1], fpLen_);
        if (et_ == BINARY)
        {
            ValueType bits = ke.code;
            for (uint64_t i = 0; i < R; ++i)
            {
                const uint64_t t = ke.get(i, bn_);
                if (!(visitedVerticies.getBit(t + offset)))
                {
                    continue;
                }
                signature ^= B_.get64Bits(t * blockSize + offset, fpLen_);
                bits ^= B_.getBits(t * blockSize + offset + fpLen_, maxCodeLen_);
            }

            const uint64_t set_Pos = ke.get(it->second, bn_);
            B_.set64Bits(set_Pos * blockSize + offset, fpLen_, signature);
            B_.setBits(set_Pos * blockSize + offset + fpLen_, maxCodeLen_, bits);
            visitedVerticies.setBit(set_Pos + offset);
        }
        else if (et_ == GAMMA)
        {
            bool bit;
            if (offset < fpLen_)
            {
                bit = (signature >> offset) & 1;
            }
            else
            {
                bit = FujimapCommon::gammaEncodeBit(offset - fpLen_, ke.code);
            }
            for (uint64_t i = 0; i < R; ++i)
            {
                const uint64_t t = ke.get(i, bn_);
                if (!(visitedVerticies.getBit(t + offset)))
                {
                    continue;
                }
                bit ^= B_.getBit(t * blockSize + offset);
            }

            const uint64_t set_Pos = ke.get(it->second, bn_);
            B_.set64Bits(set_Pos * blockSize + offset, 1, bit);
            visitedVerticies.setBit(set_Pos + offset);
        }
        else
        {
            assert(false);
        }
    }

    return 0;
}

template <class ValueType>
size_t FujimapBlock<ValueType>::getKeyNum() const
{
    return keyNum_;
}

template <class ValueType>
size_t FujimapBlock<ValueType>::getWorkingSize() const
{
    return B_.bvBitSize();
}

template <class ValueType>
void FujimapBlock<ValueType>::save(std::ofstream& ofs) const
{
    ofs.write((const char*)(&keyNum_),     sizeof(keyNum_));
    ofs.write((const char*)(&minCodeVal_), sizeof(minCodeVal_));
    ofs.write((const char*)(&maxCodeVal_), sizeof(maxCodeVal_));
    ofs.write((const char*)(&maxCodeLen_), sizeof(maxCodeLen_));
    ofs.write((const char*)(&fpLen_),      sizeof(fpLen_));
    ofs.write((const char*)(&seed_),       sizeof(seed_));
    ofs.write((const char*)(&bn_),         sizeof(bn_));
    ofs.write((const char*)(&et_),         sizeof(et_));

    B_.write(ofs);
}

template <class ValueType>
void FujimapBlock<ValueType>::load(std::ifstream& ifs)
{
    ifs.read((char*)(&keyNum_),     sizeof(keyNum_));
    ifs.read((char*)(&minCodeVal_), sizeof(minCodeVal_));
    ifs.read((char*)(&maxCodeVal_), sizeof(maxCodeVal_));
    ifs.read((char*)(&maxCodeLen_), sizeof(maxCodeLen_));
    ifs.read((char*)(&fpLen_),      sizeof(fpLen_));
    ifs.read((char*)(&seed_),       sizeof(seed_));
    ifs.read((char*)(&bn_),         sizeof(bn_));
    ifs.read((char*)(&et_),         sizeof(et_));

    B_.read(ifs);
}

}
}

NS_IZENELIB_AM_END

#endif // FUJIMAP_BLOCK_HPP__
