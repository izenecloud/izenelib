/*
 * fujimapBlock.cpp
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

#include <fstream>
#include <queue>
#include <cassert>
#include <algorithm>
#include <am/succinct/fujimap/fujimap_block.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

const double   FujimapBlock::C_R = 1.3;
const uint64_t FujimapBlock::intercept = 10;

FujimapBlock::FujimapBlock()
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

FujimapBlock::~FujimapBlock()
{
}

size_t FujimapBlock::getBSize() const
{
    return B_.bvSize();
}

uint64_t FujimapBlock::getVal(const KeyEdge& ke) const
{
    if (B_.bvSize() == 0)
    {
        return NOTFOUND;
    }

    uint64_t blockSize = (et_ == BINARY) ? (maxCodeLen_ + fpLen_) : 1;
    if (fpLen_ != 0)
    {
        uint64_t fpCheck = 0;
        for (uint64_t i = 0; i < R; ++i)
        {
            fpCheck ^= B_.getBits(ke.get(i, bn_) * blockSize, fpLen_);
        }
        if (fpCheck != mask(ke.v[0] ^ ke.v[1], fpLen_))
        {
            return NOTFOUND;
        }
    }

    uint64_t code = 0;
    for (uint64_t i = 0; i < R; ++i)
    {
        code ^= B_.getBits(ke.get(i, bn_) * blockSize + fpLen_, maxCodeLen_);
    }

    if (et_ == GAMMA)
    {
        code = gammaDecode(code);
    }

    if (code > maxCodeVal_)
    {
        return NOTFOUND;
    }

    return code + minCodeVal_;
}

uint64_t FujimapBlock::getSeed() const
{
    return seed_;
}

int FujimapBlock::build(vector<KeyEdge>& keyEdges,
                        const uint64_t seed, const uint64_t fpLen,
                        const EncodeType et)
{
    keyNum_   = static_cast<uint64_t>(keyEdges.size());
    seed_     = seed;
    fpLen_    = fpLen;
    et_       = et;

    minCodeVal_ = 0xFFFFFFFFFFFFFFFFLLU;
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
        maxCodeLen_  = log2(maxCodeVal_);
        totalCodeLen = (maxCodeLen_ + fpLen_) * keyNum_;
        bn_          = (uint64_t)(keyNum_ * C_R / (double)R + intercept);
    }
    else if (et_ == GAMMA)
    {
        for (size_t i = 0; i < keyEdges.size(); ++i)
        {
            totalCodeLen += gammaLen(keyEdges[i].code);
        }
        totalCodeLen += fpLen_ * keyNum_;
        maxCodeLen_   = gammaLen(maxCodeVal_);
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
    vector<uint8_t> degs(bn_ * R + space);
    vector<uint64_t> offset_s(bn_ * R + space + 1);

    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        const KeyEdge& ke(keyEdges[i]);
        uint64_t len = (et_ == BINARY) ? 1 : gammaLen(ke.code) + fpLen_;
        for (uint64_t j = 0; j < R; ++j)
        {
            uint64_t t = ke.get(j, bn_);
            for (uint64_t k = 0; k < len; ++k)
            {
                if (degs[t+k] == 0xFF)
                {
                    return -1;
                }
                degs[t+k]++;
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
    vector<uint64_t> edges(totalEdgeNum);
    for (size_t i = 0; i < keyEdges.size(); ++i)
    {
        const KeyEdge& ke(keyEdges[i]);
        uint64_t len = (et_ == BINARY) ? 1 : gammaLen(ke.code) + fpLen_;
        for (uint64_t j = 0; j < R; ++j)
        {
            uint64_t t = ke.get(j, bn_);
            for (uint64_t k = 0; k < len; ++k)
            {
                edges[offset_s[t+k] + degs[t+k]++] = i * pointsPerKey + k;
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

    vector<pair<uint64_t, uint8_t> > extractedEdges;
    uint64_t assignNum = keyNum_ * pointsPerKey;

    BitVec visitedEdges(assignNum);
    uint64_t delet_edNum = 0;
    while (!q.empty())
    {
        uint64_t v = q.front();
        q.pop();

        if (visitedEdges.getBit(v)) continue;
        visitedEdges.setBit(v);
        ++delet_edNum;

        uint64_t keyID  = v / pointsPerKey;
        uint64_t offset_ = v % pointsPerKey;

        const KeyEdge& ke(keyEdges[keyID]);
        int choosed = -1;
        for (uint64_t i = 0; i < R; ++i)
        {
            const uint64_t t = ke.get(i, bn_);
            --degs[t + offset_];

            if (degs[t + offset_] == 0)
            {
                choosed = i;
                continue;
            }
            else if (degs[t + offset_] >= 2)
            {
                continue;
            }
            // degs[t] == 1
            const uint64_t end = offset_s[t+offset_+1];
            for (uint64_t j = offset_s[t+offset_]; j < end; ++j)
            {
                if (!visitedEdges.getBit(edges[j]))
                {
                    q.push(edges[j]);
                    break;
                }
            }
        }
        assert(choosed != -1);
        extractedEdges.push_back(make_pair(v, choosed));
    }

    if (et_ == BINARY && delet_edNum != keyNum_)
    {
        return -1;
    }
    else if (et_ == GAMMA && delet_edNum != totalCodeLen)
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
    BitVec visitedVerticies(assignNum * R);
    reverse(extractedEdges.begin(), extractedEdges.end());
    for (vector<pair<uint64_t, uint8_t> >::const_iterator it =
            extractedEdges.begin(); it != extractedEdges.end(); ++it)
    {
        const uint64_t v = it->first;

        uint64_t keyID = v / pointsPerKey;
        uint64_t offset_ = v % pointsPerKey;

        const KeyEdge& ke(keyEdges[keyID]);

        uint64_t signature    = mask(ke.v[0] ^ ke.v[1], fpLen_);
        uint64_t writeCodeLen = 0;
        uint64_t bits         = 0;
        if (et_ == BINARY)
        {
            writeCodeLen = maxCodeFPLen;
            bits = (ke.code << fpLen_) + signature; // ke.code + 1111
        }
        else if (et_ == GAMMA)
        {
            writeCodeLen = 1;
            if (offset_ < fpLen_)
            {
                bits = (signature >> offset_) & 1LLU;
            }
            else
            {
                bits = gammaEncodeBit(offset_ - fpLen_, ke.code);
            }
        }
        else
        {
            assert(false);
        }

        for (uint64_t i = 0; i < R; ++i)
        {
            const uint64_t t = ke.get(i, bn_);
            if (!(visitedVerticies.getBit(t+offset_)))
            {
                continue;
            }
            bits ^= B_.getBits(t * blockSize + offset_, writeCodeLen);
        }

        const uint64_t set_Pos = ke.get(it->second, bn_);
        B_.setBits(set_Pos * blockSize + offset_, writeCodeLen, bits);
        visitedVerticies.setBit(set_Pos + offset_);
    }

    return 0;
}

size_t FujimapBlock::getKeyNum() const
{
    return keyNum_;
}

size_t FujimapBlock::getWorkingSize() const
{
    return B_.bvSize() * BITNUM;
}

void FujimapBlock::save(ofstream& ofs)
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

void FujimapBlock::load(ifstream& ifs)
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

}}

NS_IZENELIB_AM_END
