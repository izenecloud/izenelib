/*
 * keyFile.hpp
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


#ifndef KEYFILE_HPP__
#define KEYFILE_HPP__

#include <util/izene_serialization.h>

#include <fstream>
#include <string>
#include <vector>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

template <class KeyType, class ValueType>
class KeyFile
{
    enum
    {
        BLOCKSIZE = 4096
    };

public:
    KeyFile(const char* fn);
    ~KeyFile();

    int initWorkingFile(const char* fn);
    void initMaxID(const uint64_t maxID);
    int clear();
    int write(const uint64_t id, const KeyType& key, const ValueType& value);
    int read(const uint64_t id, std::vector<std::pair<KeyType, ValueType> >& kvs);
    size_t getNum() const;

private:
    std::string fns_;
    std::vector<std::vector<std::pair<KeyType, ValueType> > > buffers_;
    std::vector<uint64_t> nextPointers_;
    std::vector<uint64_t> firstPointers_;
    size_t num_;
    uint64_t maxID_;

};

template <class KeyType, class ValueType>
KeyFile<KeyType, ValueType>::KeyFile(const char* fn) :  fns_(fn), num_(0), maxID_(KEYBLOCK)
{
    initWorkingFile(fns_.c_str());
}

template <class KeyType, class ValueType>
KeyFile<KeyType, ValueType>::~KeyFile()
{
}

template <class KeyType, class ValueType>
size_t KeyFile<KeyType, ValueType>::getNum() const
{
    return num_;
}

template <class KeyType, class ValueType>
int KeyFile<KeyType, ValueType>::clear()
{
    num_ = 0;
    std::vector<std::vector<std::pair<KeyType, ValueType> > >().swap(buffers_);
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

template <class KeyType, class ValueType>
int KeyFile<KeyType, ValueType>::initWorkingFile(const char* fn)
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

template <class KeyType, class ValueType>
void KeyFile<KeyType, ValueType>::initMaxID(const uint64_t maxID)
{
    maxID_ = maxID;
    if (!fns_.empty())
    {
        ofstream ofs(fns_.c_str());
        ofs.write((const char*)(&maxID_), sizeof(uint64_t));
    }
    buffers_.resize(maxID_);
    nextPointers_.resize(maxID_);
    firstPointers_.resize(maxID_);
}

template <class KeyType, class ValueType>
int KeyFile<KeyType, ValueType>::write(const uint64_t id, const KeyType& key, const ValueType& value)
{
    assert(id < maxID_);
    std::vector<std::pair<KeyType, ValueType> >& v(buffers_[id]);
    v.push_back(std::make_pair(key, value));

    if (v.size() >= BLOCKSIZE)
    {
        fstream ofs(fns_.c_str(), std::ios::in | std::ios::out);
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

        char* vbuf;
        std::size_t vlen = 0;
        izenelib::util::izene_serialization<std::vector<std::pair<KeyType, ValueType> > > izsKey(v);
        izsKey.write_image(vbuf, vlen);
        ofs.write((const char*)(&vlen), sizeof(std::size_t));
        ofs.write((const char*)vbuf, vlen);

        nextPointers_[id] = static_cast<uint64_t>(ofs.tellp());
        static const uint64_t dummy = 0;
        ofs.write((const char*)(&dummy), sizeof(uint64_t));
        if (!ofs)
        {
            return -1;
        }

        v.clear();
    }
    ++num_;
    return 0;
}

template <class KeyType, class ValueType>
int KeyFile<KeyType, ValueType>::read(const uint64_t id, std::vector<std::pair<KeyType, ValueType> >& kvs)
{
    uint64_t readPos = firstPointers_[id];
    std::ifstream ifs(fns_.c_str());
    size_t blockNum_ = 0;
    while (readPos != 0)
    {
        blockNum_++;
        ifs.seekg(readPos, ios::beg);

        std::size_t vlen = 0;
        ifs.read((char*)(&vlen), sizeof(std::size_t));
        std::string vstr;
        vstr.resize(vlen);
        ifs.read((char*)(&vstr[0]), vlen);
        izenelib::util::izene_deserialization<std::vector<std::pair<KeyType, ValueType> > > izsKey(vstr.c_str(), vlen);
        std::vector<std::pair<KeyType, ValueType> > kvs_buffer;
        izsKey.read_image(kvs_buffer);
        kvs.insert(kvs.end(), kvs_buffer.begin(), kvs_buffer.end());

        ifs.read((char*)(&readPos), sizeof(uint64_t));
        if (!ifs)
        {
            cerr << "read error" << endl;
            return -1;
        }

        if (!ifs) return -1;
    }

    kvs.insert(kvs.end(), buffers_[id].begin(), buffers_[id].end());
    return 0;
}

}}

NS_IZENELIB_AM_END

#endif // KEY_FILE_HPP__
