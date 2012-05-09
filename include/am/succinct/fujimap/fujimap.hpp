/*
  * fujimap.hpp
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

#ifndef FUJIMAP_HPP__
#define FUJIMAP_HPP__

#include "fujimap_block.hpp"
#include "keyedge.hpp"
#include "keyfile.hpp"

#include <util/hashFunction.h>

#include <boost/unordered_map.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/**
  * Succinct Associative Array
  * Support basic key/value store operations (set/get)
  */
template <class KeyType>
class Fujimap
{
public:
    /**
     * Constructor
     */
    Fujimap(const char* fn = "tmp.kf");

    /**
     * Destructor
     */
    ~Fujimap();

    /**
     * Initialize a seed for hash function
     * @param seed A seed value
     */
    void initSeed(const uint64_t seed);

    /**
     * Initialize a false positive rate
     * @param fpLen A negative of false positive rate power (prob. of false positive rate is 2^{-fpLen_})
     */
    void initFP(const uint64_t fpLen);

    /**
     * Initialize a size of temporary map size. A succinct and static map will be constructured after every tmpN_ key/values are added.
     * @param tmpN A size of temporary map.
     */
    void initTmpN(const uint64_t tmpN);

    /**
     * Initialize a number of blocks in hash. This would be log(number of key/values).
     * @param keyBlockN A number of blocks.
     */
    void initKeyBlockN(const uint64_t keyBlockN);

    /**
     * Initialize a working file.
     * @param fn A name of working file which stores temporary data.
     */
    int initWorkingFile(const char* fn);

    /**
     * Initialize an encode type
     * @param et A type of encoding
     */
    void initEncodeType(const EncodeType et);

    /**
     * Set a record of key/value.
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param vbuf the pointer to the value region.
     * @param vlen the length of the value.
     * @param searchable true if this record will be searchable immediately after this operation false
     * if this record will be searchable after build() is called (default: false).
     */
    void setString(const KeyType& key, const char* vbuf, const size_t vlen,
                   const bool searchable = false);

    /**
     * Set a record of key/value. This record will be searchable immediately after this operation.
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param value the interger value.
     * @param searchable true if this record will be searchable immediately after this operation or false
     * if this record will be searchable after build() is called (defalut: false).
     */
    void setInteger(const KeyType& key, const uint64_t value,
                    const bool searchable = false);

    /**
     * Build an index for registered key/value pairs which are not indexed.
     * @return true on success, or false on failure.
     : @note when build() failed, a user specify new seed funciton by initSeed(), and retry build().
     */
    int build();

    /**
     * Retrieve the string value for a key
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param vlen the length of the value.
     * @return the pointer to the value region of the corresponding record, or NULL  on failure.
     * @note Because the pointer of the returned value is a member of fm,
     * a user should copy the returned value if using the returned value.
    */
    const char* getString(const KeyType& key, size_t& vlen) const;

    /**
     * Retrieve the integer value for a key
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @return the interge value for a key, or fujimap::NOTFOUND on failure.
    */
    uint64_t getInteger(const KeyType& key) const;

    /**
     * Load the previous status from a file
     * @param index the file name where the index is stored.
     * @return 0 on success, -1 on failure
     */
    int load(const char* index);

    /**
     * Save the current status in a file
     * @param index the file name where the index is stored.
     * @return 0 on success, -1 on failure
     */
    int save(const char* index); ///< Load a map from index

    /**
     * Clear the stage
     */
    void clear(bool kf_clear = false);

    /**
     * Report the current status when some error occured.
     * @return the current status message
     */
    std::string what() const;

    /**
     * Get the registered number of key/values.
     * @return the number of registered keys.
     */
    size_t getKeyNum() const;

    /**
     * Get the size of working space (estimated)
     * @return the size of working space in bits
     */
    size_t getWorkingSize() const;

    /**
     * Get the fpLen
     * @return fpLen
     */
    uint64_t getFpLen() const;

    /**
     * Get the current EncodeType
     * @return Current Encoding Type
     */
    EncodeType getEncodeType() const;

    /**
     * Get the current EncodeType in string
     * @return Current Encoding Type
     */
    std::string getEncodeTypeStr() const;

    bool empty() const;

private:
    int build_(std::vector<std::pair<KeyType, uint64_t> >& kvs, FujimapBlock& fb);

    uint64_t getBlockID(const KeyType& key) const;
    uint64_t getCode(const std::string& value); ///< Return corresponding code of a given value

    std::ostringstream what_; ///< Store a message

    boost::unordered_map<std::string, uint64_t> val2code_; ///< Map from value to code
    std::vector<std::string> code2val_; ///< Map from code to value

    KeyFile<KeyType> kf_; ///< A set of non-searchable key/values
    boost::unordered_map<KeyType, uint64_t> tmpEdges_; ///< A set of searchable key/values to be indexed

    std::vector<std::vector<FujimapBlock> > fbs_; ///< BitArrays

    uint64_t seed_; ///< A seed for hash
    uint64_t fpLen_; ///< A false positive rate (prob. of false positive is 2^{-fpLen})
    uint64_t tmpN_; ///< A size of temporary map
    uint64_t tmpC_; ///< Current size of temporary map
    uint64_t keyBlockN_; ///< A number of blocks
    EncodeType et_; ///< An encode type of values
};

template <class KeyType>
Fujimap<KeyType>::Fujimap(const char* fn)
    : kf_(fn)
    , seed_(0x123456)
    , fpLen_(FPLEN)
    , tmpN_(TMPN)
    , tmpC_(0)
    , keyBlockN_(KEYBLOCK)
    , et_(BINARY)
{
    kf_.initMaxID(keyBlockN_);
}

template <class KeyType>
Fujimap<KeyType>::~Fujimap()
{
}

template <class KeyType>
void Fujimap<KeyType>::initSeed(const uint64_t seed)
{
    seed_ = seed;
}

template <class KeyType>
void Fujimap<KeyType>::initFP(const uint64_t fpLen)
{
    fpLen_ = fpLen;
}

template <class KeyType>
void Fujimap<KeyType>::initTmpN(const uint64_t tmpN)
{
    tmpN_ = tmpN;
}

template <class KeyType>
void Fujimap<KeyType>::initKeyBlockN(const uint64_t keyBlockN)
{
    keyBlockN_ = keyBlockN;
    kf_.initMaxID(keyBlockN_);
}

template <class KeyType>
int Fujimap<KeyType>::initWorkingFile(const char* fn)
{
    if (kf_.initWorkingFile(fn) == -1)
    {
        what_ << "initWorkingFile erorr: " << fn;
        return -1;
    }
    return  0;
}

template <class KeyType>
void Fujimap<KeyType>::initEncodeType(const EncodeType et)
{
    et_ = et;
}

template <class KeyType>
void Fujimap<KeyType>::setString(const KeyType& key, const char* vbuf, const size_t vlen,
                        const bool searchable)
{
    setInteger(key, getCode(std::string(vbuf, vlen)), searchable);
}

template <class KeyType>
void Fujimap<KeyType>::setInteger(const KeyType& key, const uint64_t code, const bool searchable)
{
    if (searchable)
    {
        tmpEdges_[key] = code;
    }
    else
    {
        uint64_t id = getBlockID(key);
        kf_.write(id, key, code);
    }
    if (++tmpC_ == tmpN_)
    {
        build();
    }
}

template <class KeyType>
uint64_t Fujimap<KeyType>::getCode(const std::string& value)
{
    typename boost::unordered_map<std::string, uint64_t>::const_iterator it = val2code_.find(value);
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

template <class KeyType>
bool kvsEq(const pair<KeyType, uint64_t>& v1,
           const pair<KeyType, uint64_t>& v2)
{
    return v1.first == v2.first; // ignore second
}

template <class KeyType>
bool kvsComp(const pair<KeyType, uint64_t>& v1,
             const pair<KeyType, uint64_t>& v2)
{
    return v1.first < v2.first; // ignore second
}

template <class KeyType>
uint64_t Fujimap<KeyType>::getBlockID(const KeyType& key) const
{
    char* kbuf;
    std::size_t klen;
    izenelib::util::izene_serialization<KeyType> izsKey(key);
    izsKey.write_image(kbuf, klen);
    return MurmurHash64A(kbuf, klen, 0) % keyBlockN_;
}

template <>
inline uint64_t Fujimap<uint128_t>::getBlockID(const uint128_t& key) const
{
    return (uint64_t)key & (keyBlockN_ - 1);
}

template <class KeyType>
int Fujimap<KeyType>::build()
{
    if (tmpC_ == 0) return 0;

    for (typename boost::unordered_map<KeyType, uint64_t>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        uint64_t id = getBlockID(it->first);
        kf_.write(id, it->first, it->second);
    }
    boost::unordered_map<KeyType, uint64_t>().swap(tmpEdges_);

    fbs_.push_back(std::vector<FujimapBlock>(keyBlockN_));
    std::vector<FujimapBlock>& cur = fbs_.back();
    for (uint64_t i = 0; i < keyBlockN_; ++i)
    {
        std::vector<std::pair<KeyType, uint64_t> > kvs;
        if (kf_.read(i, kvs) == -1)
        {
            what_ << "kf read error" << endl;
            return -1;
        }

        if (build_(kvs, cur[i]) == -1)
        {
            fbs_.pop_back();
            return -1;
        }
    }
    kf_.clear();
    tmpC_ = 0;
    return 0;
}

template <class KeyType>
int Fujimap<KeyType>::build_(std::vector<std::pair<KeyType, uint64_t> >& kvs,
                    FujimapBlock& fb)
{
    if (kvs.size() == 0)
    {
        return 0; // not build
    }

    std::reverse(kvs.begin(), kvs.end());
    std::stable_sort(kvs.begin(), kvs.end(), kvsComp<KeyType>);
    kvs.erase(std::unique(kvs.begin(), kvs.end(), kvsEq<KeyType>), kvs.end());

    for (int i = 0; i < 20; ++i)
    {
        std::vector<KeyEdge> keyEdges;
        keyEdges.reserve(kvs.size());
        for (size_t i = 0; i < kvs.size(); ++i)
        {
            char* kbuf;
            std::size_t klen;
            izenelib::util::izene_serialization<KeyType> izsKey(kvs[i].first);
            izsKey.write_image(kbuf, klen);
            keyEdges.push_back(KeyEdge(kbuf, klen, kvs[i].second, seed_));
        }
        if (fb.build(keyEdges, seed_, fpLen_, et_) == 0)
        {
            break;
        }
        seed_ += 777;
    }
    return 0;
}

template <class KeyType>
size_t Fujimap<KeyType>::getKeyNum() const
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

template <class KeyType>
size_t Fujimap<KeyType>::getWorkingSize() const
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

template <class KeyType>
uint64_t Fujimap<KeyType>::getFpLen() const
{
    return fpLen_;
}

template <class KeyType>
EncodeType Fujimap<KeyType>::getEncodeType() const
{
    return et_;
}

template <class KeyType>
std::string Fujimap<KeyType>::getEncodeTypeStr() const
{
    if (et_ == BINARY)
    {
        return std::string("binary");
    }
    else if (et_ == GAMMA)
    {
        return std::string("gamma");
    }
    else
    {
        return std::string("undefined");
    }
}

template <class KeyType>
const char* Fujimap<KeyType>::getString(const KeyType& key, size_t& vlen) const
{
    uint64_t ret = getInteger(key);
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

template <class KeyType>
uint64_t Fujimap<KeyType>::getInteger(const KeyType& key) const
{
    typename boost::unordered_map<KeyType, uint64_t>::const_iterator it = tmpEdges_.find(key);
    if (it != tmpEdges_.end())
    {
        return it->second;
    }

    char* kbuf;
    std::size_t klen;
    izenelib::util::izene_serialization<KeyType> izsKey(key);
    izsKey.write_image(kbuf, klen);
    const uint64_t id = getBlockID(key);
    for (vector<vector<FujimapBlock> >::const_reverse_iterator it2 = fbs_.rbegin();
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

template <class KeyType>
int Fujimap<KeyType>::load(const char* index)
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
        std::size_t len = 0;
        ifs.read((char*)(&len), sizeof(len));
        code2val_[i].resize(len);
        ifs.read((char*)(&code2val_[i][0]), len);
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
        std::size_t len = 0;
        ifs.read((char*)(&len), sizeof(len));
        std::string s;
        s.resize(len);
        ifs.read((char*)(&s[0]), len);
        std::pair<KeyType, uint64_t> kv;
        izenelib::util::izene_deserialization<std::pair<KeyType, uint64_t> > izsKey(s.c_str(), s.size());
        izsKey.read_image(kv);
        tmpEdges_.insert(kv);
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

template <class KeyType>
int Fujimap<KeyType>::save(const char* index)
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
        const std::size_t len = code2val_[i].size();
        ofs.write((const char*)(&len), sizeof(len));
        ofs.write(code2val_[i].c_str(), len);
    }

    ofs.write((const char*)(&seed_), sizeof(seed_));
    ofs.write((const char*)(&fpLen_), sizeof(fpLen_));
    ofs.write((const char*)(&tmpN_), sizeof(tmpN_));

    uint64_t tmpEdgeSize = static_cast<uint64_t>(tmpEdges_.size());
    ofs.write((const char*)(&tmpEdgeSize), sizeof(tmpEdgeSize));
    for (typename boost::unordered_map<KeyType, uint64_t>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        char* kbuf;
        std::size_t klen;
        izenelib::util::izene_serialization<std::pair<KeyType, uint64_t> > izsKey(*it);
        izsKey.write_image(kbuf, klen);
        ofs.write((const char*)(&klen), sizeof(klen));
        ofs.write(kbuf, klen);
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

template <class KeyType>
void Fujimap<KeyType>::clear(bool kf_clear)
{
    boost::unordered_map<std::string, uint64_t>().swap(val2code_);
    std::vector<std::string>().swap(code2val_);

    boost::unordered_map<KeyType, uint64_t>().swap(tmpEdges_);
    std::vector<std::vector<FujimapBlock> >().swap(fbs_);

    if (kf_clear)
        kf_.clear();
}

template <class KeyType>
std::string Fujimap<KeyType>::what() const
{
    return what_.str();
}

template <class KeyType>
bool Fujimap<KeyType>::empty() const
{
    return fbs_.empty() && tmpEdges_.empty();
}

}}

NS_IZENELIB_AM_END

#endif // FUJIMAP_HPP__
