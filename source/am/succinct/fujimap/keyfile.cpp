/*
 * keyFile.cpp
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

#include <iostream>
#include <cassert>
#include <am/succinct/fujimap/keyfile.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

static const char* TMPKF = "tmp.kf";


KeyFile::KeyFile() :  fns_(TMPKF), num_(0), maxID_(0)
{
    initWorkingFile(fns_.c_str());
}

KeyFile::~KeyFile()
{
}

size_t KeyFile::getNum() const
{
    return num_;
}

int KeyFile::clear()
{
    num_ = 0;
    buffers_.clear();
    nextPointers_.clear();
    firstPointers_.clear();
    buffers_.resize(maxID_);
    nextPointers_.resize(maxID_);
    firstPointers_.resize(maxID_);

    ofstream ofs(fns_.c_str());
    ofs.write((const char*)(&maxID_), sizeof(uint64_t));
    if (!ofs)
    {
        return -1;
    }

    return 0;
}



int KeyFile::initWorkingFile(const char* fn)
{
    fns_ = fn;
    ofstream ofs(fns_.c_str());
    ofs.write((const char*)(&maxID_), sizeof(uint64_t));
    if (!ofs)
    {
        return -1;
    }
    return 0;
}

void KeyFile::initMaxID(const uint64_t maxID__)
{
    maxID_ = maxID__;
    buffers_.resize(maxID_);
    nextPointers_.resize(maxID_);
    firstPointers_.resize(maxID_);
}


int KeyFile::write(const uint64_t id, const char* kbuf, const size_t klen,
                   const uint64_t value)
{
    assert(id < maxID_);
    vector<pair<string, uint64_t> >& v(buffers_[id]);
    string s;
    s.assign(kbuf, klen);
    v.push_back(make_pair(s, value));

    if (v.size() >= BLOCKSIZE)
    {
        fstream ofs(fns_.c_str(),  std::ios::in | std::ios::out);
        ofs.seekp(0, ios::end);
        uint64_t curPos = ofs.tellp();
        if (firstPointers_[id] == 0)
        {
            firstPointers_[id] = curPos;
        }
        else
        {
            ofs.seekp(nextPointers_[id], ios::beg);
            ofs.write((const char*)(&curPos), sizeof(uint64_t));
            if (!ofs)
            {
                return -1;
            }
            ofs.seekp(curPos, ios::beg);
            if (!ofs)
            {
                return -1;
            }
        }

        uint64_t vnum_ = static_cast<uint64_t>(v.size());
        ofs.write((const char*)(&vnum_), sizeof(uint64_t));
        for (size_t i = 0; i < v.size(); ++i)
        {
            uint64_t klen = v[i].first.size();
            ofs.write((const char*)(&klen), sizeof(uint64_t));
            ofs.write((const char*)(v[i].first.c_str()), klen);
            ofs.write((const char*)(&v[i].second), sizeof(uint64_t));
        }

        nextPointers_[id] = static_cast<uint64_t>(ofs.tellp());
        uint64_t dummy = 0;
        ofs.write((const char*)(&dummy), sizeof(uint64_t));
        if (!ofs)
        {
            return -1;
        }

        vector<pair<string, uint64_t> >().swap(v);
    }
    ++num_;
    return 0;
}

int KeyFile::read(const uint64_t id, vector<pair<std::string, uint64_t> >& kvs)
{
    uint64_t readPos = firstPointers_[id];
    ifstream ifs(fns_.c_str());
    size_t blockNum_ = 0;
    while (readPos != 0)
    {
        blockNum_++;
        ifs.seekg(readPos, ios::beg);
        uint64_t vnum_ = 0;
        ifs.read((char*)(&vnum_), sizeof(uint64_t));
        if (!ifs) return -1;
        for (uint64_t i = 0; i < vnum_; ++i)
        {
            uint64_t klen = 0;
            ifs.read((char*)(&klen), sizeof(uint64_t));
            if (!ifs) return -1;
            std::string s;
            s.resize(klen);
            ifs.read((char*)(&s[0]), klen);
            if (!ifs) return -1;
            uint64_t value = 0;
            ifs.read((char*)(&value), sizeof(uint64_t));
            if (!ifs) return -1;
            kvs.push_back(make_pair(s, value));
        }

        ifs.read((char*)(&readPos), sizeof(uint64_t));
        if (!ifs)
        {
            cerr << "read error" << endl;
            return -1;
        }

        if (!ifs) return -1;
    }

    for (size_t i = 0; i < buffers_[id].size(); ++i)
    {
        kvs.push_back(buffers_[id][i]);
    }
    return 0;
}





}}

NS_IZENELIB_AM_END

