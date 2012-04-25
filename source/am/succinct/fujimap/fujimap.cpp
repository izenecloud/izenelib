/*
 * fujimap.cpp
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

#include <algorithm>
#include <fstream>
#include <cassert>
#include <am/succinct/fujimap/fujimap.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

Fujimap::Fujimap() :  seed_(0x123456), fpLen_(FPLEN), tmpN_(TMPN),
    keyBlockN_(KEYBLOCK), et_(BINARY)
{
    kf_.initMaxID(keyBlockN_);
}

Fujimap::~Fujimap()
{
}

void Fujimap::initSeed(const uint64_t seed)
{
    seed_ = seed;
}

void Fujimap::initFP(const uint64_t fpLen)
{
    fpLen_ = fpLen;
}

void Fujimap::initTmpN(const uint64_t tmpN)
{
    tmpN_ = tmpN;
}

void Fujimap::initKeyBlockN(const uint64_t keyBlockN)
{
    keyBlockN_ = keyBlockN;
    kf_.initMaxID(keyBlockN_);
}

int Fujimap::initWorkingFile(const char* fn)
{
    if (kf_.initWorkingFile(fn) == -1)
    {
        what_ << "initWorkingFile erorr: " << fn;
        return -1;
    }
    return  0;
}

void Fujimap::initEncodeType(const EncodeType et)
{
    et_ = et;
}

void Fujimap::setString(const char* kbuf, const size_t klen,
                        const char* vbuf, const size_t vlen,
                        const bool searchable)
{
    setInteger(kbuf, klen, getCode(string(vbuf, vlen)), searchable);
}

void Fujimap::setInteger(const char* kbuf, const size_t klen,
                         const uint64_t code, const bool searchable)
{
    if (searchable)
    {
        tmpEdges_[string(kbuf, klen)] = code;
        if (tmpEdges_.size() == tmpN_)
        {
            build();
        }
    }
    else
    {
        uint64_t id = getBlockID(kbuf, klen);
        kf_.write(id, kbuf, klen, code);
    }
}

uint64_t Fujimap::getCode(const std::string& value)
{
    map<string, uint64_t>::const_iterator it = val2code_.find(value);
    if (it != val2code_.end())
    {
        return it->second;
    }
    else
    {
        uint32_t code = static_cast<uint32_t>(val2code_.size());
        val2code_[value] = code;
        code2val_.push_back(value);
        return code;
    }
}

bool kvsEq(const pair<string, uint64_t>& v1,
           const pair<string, uint64_t>& v2)
{
    return v1.first == v2.first; // ignore second
}

bool kvsComp(const pair<string, uint64_t>& v1,
             const pair<string, uint64_t>& v2)
{
    return v1.first < v2.first; // ignore second
}


uint64_t Fujimap::getBlockID(const char* kbuf, const size_t klen) const
{
    return hash(kbuf, klen) % keyBlockN_;
}

int Fujimap::build()
{
    for (map<string, uint64_t>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        uint64_t id = getBlockID(it->first.c_str(), it->first.size());
        kf_.write(id, it->first.c_str(), it->first.size(), it->second);
    }
    tmpEdges_.clear();


    vector<FujimapBlock> cur;
    for (uint64_t i = 0; i < keyBlockN_; ++i)
    {
        vector<pair<string, uint64_t> > kvs;
        if (kf_.read(i, kvs) == -1)
        {
            what_ << "kf read error" << endl;
            return -1;
        }

        FujimapBlock fb;
        if (build_(kvs, fb) == -1)
        {
            return -1;
        }

        cur.push_back(fb);
    }
    kf_.clear();
    fbs_.push_back(cur);
    return 0;
}

int Fujimap::build_(vector<pair<string, uint64_t> >& kvs,
                    FujimapBlock& fb)
{
    reverse(kvs.begin(), kvs.end());
    stable_sort(kvs.begin(), kvs.end(), kvsComp);
    kvs.erase(unique(kvs.begin(), kvs.end(), kvsEq), kvs.end());

    if (kvs.size() == 0)
    {
        return 0; // not build
    }

    for (int i = 0; i < 20; ++i)
    {
        vector<KeyEdge> keyEdges;
        for (size_t i = 0; i < kvs.size(); ++i)
        {
            keyEdges.push_back(KeyEdge(kvs[i].first.c_str(), kvs[i].first.size(),
                                       kvs[i].second, seed_));
        }
        if (fb.build(keyEdges, seed_, fpLen_, et_) == 0)
        {
            break;
        }
        seed_ += 777;
    }
    return 0;
}

size_t Fujimap::getKeyNum() const
{
    size_t keyNum = tmpEdges_.size() + kf_.getNum();

    for (size_t i = 0; i < fbs_.size(); ++i)
    {
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            keyNum += fbs_[i][j].getKeyNum();
        }
    }

    return keyNum;
}

size_t Fujimap::getWorkingSize() const
{
    size_t workingSize = 0;
    for (size_t i = 0; i < fbs_.size(); ++i)
    {
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            workingSize += fbs_[i][j].getWorkingSize();
        }
    }

    return workingSize;
}

uint64_t Fujimap::getFpLen() const
{
    return fpLen_;
}

EncodeType Fujimap::getEncodeType() const
{
    return et_;
}

string Fujimap::getEncodeTypeStr() const
{
    if (et_ == BINARY)
    {
        return string("binary");
    }
    else if (et_ == GAMMA)
    {
        return string("gamma");
    }
    else
    {
        return string("undefined");
    }
}

const char* Fujimap::getString(const char* kbuf, const size_t klen, size_t& vlen) const
{
    uint64_t ret = getInteger(kbuf, klen);
    if (ret != NOTFOUND && ret < code2val_.size())
    {
        vlen = code2val_[ret].size();
        return code2val_[ret].c_str();
    }
    else
    {
        return NULL; // NOTFOUND;
    }
}

uint64_t Fujimap::getInteger(const char* kbuf, const size_t klen) const
{
    string s(kbuf, klen);
    map<string, uint64_t>::const_iterator it = tmpEdges_.find(string(kbuf, klen));
    if (it != tmpEdges_.end())
    {
        return it->second;
    }

    const uint64_t id = getBlockID(kbuf, klen);
    for (vector< vector<FujimapBlock> >::const_reverse_iterator it2 = fbs_.rbegin();
            it2 != fbs_.rend(); ++it2)
    {
        KeyEdge ke(kbuf, klen, 0, (*it2)[id].getSeed());
        uint64_t ret = (*it2)[id].getVal(ke);
        if (ret != NOTFOUND)
        {
            return ret;
        }
    }

    return NOTFOUND;
}

int Fujimap::load(const char* index)
{
    ifstream ifs(index);
    if (!ifs)
    {
        what_ << "cannot open " << index << endl;
        return -1;
    }

    uint64_t code2valSize = 0;
    ifs.read((char*)(&code2valSize), sizeof(code2valSize));
    code2val_.resize(code2valSize);
    for (uint64_t i = 0; i < code2valSize; ++i)
    {
        loadString(code2val_[i], ifs);
    }
    for (size_t i = 0; i < code2val_.size(); ++i)
    {
        val2code_[code2val_[i]] = i;
    }

    ifs.read((char*)(&seed_), sizeof(seed_));
    ifs.read((char*)(&fpLen_), sizeof(fpLen_));
    ifs.read((char*)(&tmpN_), sizeof(tmpN_));


    uint64_t tmpEdgeSize = 0;
    ifs.read((char*)(&tmpEdgeSize), sizeof(tmpEdgeSize));

    for (uint64_t i = 0; i < tmpEdgeSize; ++i)
    {
        string s;
        loadString(s, ifs);
        uint32_t code = 0;
        ifs.read((char*)(&code), sizeof(uint32_t));
        tmpEdges_[s] = code;
    }

    uint64_t fbs_Size = 0;
    ifs.read((char*)(&fbs_Size), sizeof(fbs_Size));
    fbs_.resize(fbs_Size);

    for (size_t i = 0; i < fbs_.size(); ++i)
    {
        uint64_t fbs_InSize = 0;
        ifs.read((char*)(&fbs_InSize), sizeof(fbs_InSize));
        fbs_[i].resize(fbs_InSize);
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            fbs_[i][j].load(ifs);
        }
    }

    if (!ifs)
    {
        what_ << "read error " << index << endl;
        return -1;
    }

    return 0;
}

void Fujimap::saveString(const std::string& s, ofstream& ofs) const
{
    uint64_t len =  static_cast<uint64_t>(s.size());
    ofs.write((const char*)(&len), sizeof(len));
    ofs.write(s.c_str(), len);
}

void Fujimap::loadString(std::string& s, ifstream& ifs) const
{
    uint64_t len = 0;
    ifs.read((char*)(&len), sizeof(len));
    s.resize(len);
    ifs.read((char*)(&s[0]), len);
}


int Fujimap::save(const char* index)
{
    ofstream ofs(index);
    if (!ofs)
    {
        what_ << "cannot open " << index << endl;
        return -1;
    }

    uint64_t code2valSize = static_cast<uint64_t>(code2val_.size());
    ofs.write((const char*)(&code2valSize), sizeof(uint64_t));
    for (uint64_t i = 0; i < code2valSize; ++i)
    {
        saveString(code2val_[i], ofs);
    }

    ofs.write((const char*)(&seed_), sizeof(seed_));
    ofs.write((const char*)(&fpLen_), sizeof(fpLen_));
    ofs.write((const char*)(&tmpN_), sizeof(tmpN_));

    uint64_t tmpEdgeSize = static_cast<uint64_t>(tmpEdges_.size());
    ofs.write((const char*)(&tmpEdgeSize), sizeof(tmpEdgeSize));
    for (map<string, uint64_t>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        saveString(it->first, ofs);
        ofs.write((const char*)(&it->second), sizeof(it->second));
    }

    uint64_t fbs_Size = static_cast<uint64_t>(fbs_.size());
    ofs.write((const char*)(&fbs_Size), sizeof(fbs_Size));
    for (size_t i = 0; i < fbs_.size(); ++i)
    {
        uint64_t fbs_InSize = fbs_[i].size();
        ofs.write((const char*)(&fbs_InSize), sizeof(fbs_InSize));
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            fbs_[i][j].save(ofs);
        }
    }

    if (!ofs)
    {
        what_ << "write error " << index << endl;
        return -1;
    }

    return 0;
}

std::string Fujimap::what() const
{
    return what_.str();
}

}}

NS_IZENELIB_AM_END

