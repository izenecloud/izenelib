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
#include "keyfile.hpp"

#include <util/hashFunction.h>

#include <boost/unordered_map.hpp>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/**
  * Succinct Associative Array
  * Support basic key/value store operations (set/get)
  * XXX The ValueType must be uint64_t or uint128_t
  */
template <class KeyType, class ValueType = uint64_t>
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
    void setInteger(const KeyType& key, const ValueType& value,
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
    ValueType getInteger(const KeyType& key) const;

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
    class BlockID
    {
    public:
        BlockID() : keyBlockN_(1), truncKeyBlockN_(0) {}
        ~BlockID() {}

        void setKeyBlockN(const uint64_t keyBlockN)
        {
            keyBlockN_ = keyBlockN;
            truncKeyBlockN_ = (1LLU << (FujimapCommon::log2(keyBlockN) - 1)) - 1;
        }

        template <class key_t>
        inline uint64_t operator()(const key_t& key) const
        {
            char* kbuf;
            std::size_t klen;
            izenelib::util::izene_serialization<key_t> izsKey(key);
            izsKey.write_image(kbuf, klen);
            return MurmurHash64A(kbuf, klen, 0) % keyBlockN_;
        }

        inline uint64_t operator()(const uint128_t& key) const
        {
            return (uint64_t)key & truncKeyBlockN_;
        }

    private:
        uint64_t keyBlockN_;
        uint64_t truncKeyBlockN_;
    };

    int build_(std::vector<std::pair<KeyType, ValueType> >& kvs, FujimapBlock<ValueType>& fb);

    ValueType getCode(const std::string& value); ///< Return corresponding code of a given value

    std::ostringstream what_; ///< Store a message

    boost::unordered_map<std::string, uint32_t> val2code_; ///< Map from value to code
    std::vector<std::string> code2val_; ///< Map from code to value

    KeyFile<KeyType, ValueType> kf_; ///< A set of non-searchable key/values
    boost::unordered_map<KeyType, ValueType> tmpEdges_; ///< A set of searchable key/values to be indexed

    std::vector<std::vector<FujimapBlock<ValueType> > > fbs_; ///< BitArrays
    size_t fbs_count_;

    uint64_t seed_; ///< A seed for hash
    uint64_t fpLen_; ///< A false positive rate (prob. of false positive is 2^{-fpLen})
    uint64_t tmpN_; ///< A size of temporary map
    uint64_t tmpC_; ///< Current size of temporary map
    uint64_t keyBlockN_; ///< A number of blocks
    BlockID blockID_;
    EncodeType et_; ///< An encode type of values
    bool changed_;
};

template <class KeyType, class ValueType>
Fujimap<KeyType, ValueType>::Fujimap(const char* fn)
    : kf_(fn)
    , fbs_count_(0)
    , seed_(0x123456)
    , fpLen_(FPLEN)
    , tmpN_(TMPN)
    , tmpC_(0)
    , keyBlockN_(KEYBLOCK)
    , et_(BINARY)
    , changed_(true)
{
    blockID_.setKeyBlockN(keyBlockN_);
    kf_.initMaxID(keyBlockN_);
}

template <class KeyType, class ValueType>
Fujimap<KeyType, ValueType>::~Fujimap()
{
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::initSeed(const uint64_t seed)
{
    seed_ = seed;
    changed_ = true;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::initFP(const uint64_t fpLen)
{
    fpLen_ = fpLen;
    changed_ = true;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::initTmpN(const uint64_t tmpN)
{
    tmpN_ = tmpN;
    changed_ = true;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::initKeyBlockN(const uint64_t keyBlockN)
{
    keyBlockN_ = keyBlockN;
    blockID_.setKeyBlockN(keyBlockN_);
    kf_.initMaxID(keyBlockN_);
    changed_ = true;
}

template <class KeyType, class ValueType>
int Fujimap<KeyType, ValueType>::initWorkingFile(const char* fn)
{
    changed_ = true;
    if (kf_.initWorkingFile(fn) == -1)
    {
        what_ << "initWorkingFile erorr: " << fn;
        return -1;
    }
    return  0;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::initEncodeType(const EncodeType et)
{
    et_ = et;
    changed_ = true;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::setString(const KeyType& key, const char* vbuf, const size_t vlen,
                        const bool searchable)
{
    setInteger(key, getCode(std::string(vbuf, vlen)), searchable);
    changed_ = true;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::setInteger(const KeyType& key, const ValueType& value, const bool searchable)
{
    changed_ = true;
    if (searchable)
    {
        tmpEdges_[key] = value;
    }
    else
    {
        uint64_t id = blockID_(key);
        kf_.write(id, key, value);
    }
    if (++tmpC_ == tmpN_)
    {
        build();
    }
}

template <class KeyType, class ValueType>
ValueType Fujimap<KeyType, ValueType>::getCode(const std::string& value)
{
    typename boost::unordered_map<std::string, uint32_t>::const_iterator it = val2code_.find(value);
    if (it != val2code_.end())
    {
        return it->second;
    }
    else
    {
        changed_ = true;
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

template <class KeyType, class ValueType>
int Fujimap<KeyType, ValueType>::build()
{
    if (tmpC_ == 0) return 0;

    changed_ = true;
    for (typename boost::unordered_map<KeyType, ValueType>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        uint64_t id = blockID_(it->first);
        kf_.write(id, it->first, it->second);
    }

    fbs_.push_back(std::vector<FujimapBlock<ValueType> >(keyBlockN_));
    std::vector<FujimapBlock<ValueType> >& cur = fbs_.back();
    for (uint64_t i = 0; i < keyBlockN_; ++i)
    {
        std::vector<std::pair<KeyType, ValueType> > kvs;
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
    ++fbs_count_;
    boost::unordered_map<KeyType, ValueType>().swap(tmpEdges_);
    return 0;
}

template <class KeyType, class ValueType>
int Fujimap<KeyType, ValueType>::build_(std::vector<std::pair<KeyType, ValueType> >& kvs,
                                        FujimapBlock<ValueType>& fb)
{
    if (kvs.size() == 0)
    {
        return 0; // not build
    }

    changed_ = true;
    std::reverse(kvs.begin(), kvs.end());
    std::stable_sort(kvs.begin(), kvs.end(), kvsComp<KeyType>);
    kvs.erase(std::unique(kvs.begin(), kvs.end(), kvsEq<KeyType>), kvs.end());

    for (int i = 0; i < 20; ++i)
    {
        std::vector<KeyEdge<ValueType> > keyEdges;
        keyEdges.reserve(kvs.size());
        for (size_t i = 0; i < kvs.size(); ++i)
        {
            char* kbuf;
            std::size_t klen;
            izenelib::util::izene_serialization<KeyType> izsKey(kvs[i].first);
            izsKey.write_image(kbuf, klen);
            keyEdges.push_back(KeyEdge<ValueType>(kbuf, klen, kvs[i].second, seed_));
        }
        if (fb.build(keyEdges, seed_, fpLen_, et_) == 0)
        {
            break;
        }
        seed_ += 777;
    }
    return 0;
}

template <class KeyType, class ValueType>
size_t Fujimap<KeyType, ValueType>::getKeyNum() const
{
    size_t keyNum = tmpEdges_.size() + kf_.getNum();

    for (size_t i = 0; i < fbs_count_; ++i)
    {
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            keyNum += fbs_[i][j].getKeyNum();
        }
    }

    return keyNum;
}

template <class KeyType, class ValueType>
size_t Fujimap<KeyType, ValueType>::getWorkingSize() const
{
    size_t workingSize = 0;
    for (size_t i = 0; i < fbs_count_; ++i)
    {
        for (size_t j = 0; j < fbs_[i].size(); ++j)
        {
            workingSize += fbs_[i][j].getWorkingSize();
        }
    }

    return workingSize;
}

template <class KeyType, class ValueType>
uint64_t Fujimap<KeyType, ValueType>::getFpLen() const
{
    return fpLen_;
}

template <class KeyType, class ValueType>
EncodeType Fujimap<KeyType, ValueType>::getEncodeType() const
{
    return et_;
}

template <class KeyType, class ValueType>
std::string Fujimap<KeyType, ValueType>::getEncodeTypeStr() const
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

template <class KeyType, class ValueType>
const char* Fujimap<KeyType, ValueType>::getString(const KeyType& key, size_t& vlen) const
{
    ValueType ret = getInteger(key);
    if (ret != (ValueType)NOTFOUND && ret < code2val_.size())
    {
        vlen = code2val_[ret].size();
        return code2val_[ret].c_str();
    }
    else
    {
        return NULL; // NOTFOUND;
    }
}

template <class KeyType, class ValueType>
ValueType Fujimap<KeyType, ValueType>::getInteger(const KeyType& key) const
{
    typename boost::unordered_map<KeyType, ValueType>::const_iterator it = tmpEdges_.find(key);
    if (it != tmpEdges_.end())
    {
        return it->second;
    }

    char* kbuf;
    std::size_t klen;
    izenelib::util::izene_serialization<KeyType> izsKey(key);
    izsKey.write_image(kbuf, klen);
    const uint64_t id = blockID_(key);
    for (size_t i = fbs_count_; i > 0; --i)
    {
        KeyEdge<ValueType> ke(kbuf, klen, 0, fbs_[i - 1][id].getSeed());
        ValueType ret = fbs_[i - 1][id].getVal(ke);
        if (ret != (ValueType)NOTFOUND)
        {
            return ret;
        }
    }

    return (ValueType)NOTFOUND;
}

template <class KeyType, class ValueType>
int Fujimap<KeyType, ValueType>::load(const char* index)
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
        std::pair<KeyType, ValueType> kv;
        izenelib::util::izene_deserialization<std::pair<KeyType, ValueType> > izsKey(s.c_str(), s.size());
        izsKey.read_image(kv);
        tmpEdges_.insert(kv);
    }

    ifs.read((char*)(&fbs_count_), sizeof(fbs_count_));
    fbs_.resize(fbs_count_);

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

    changed_ = true;
    return 0;
}

template <class KeyType, class ValueType>
int Fujimap<KeyType, ValueType>::save(const char* index)
{
    if (!changed_)
    {
        std::cout << "Fujimap has no change, ignore saving." << std::endl;
        return 0;
    }

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
    std::map<KeyType, ValueType> saved_tmp_map;
    for (typename boost::unordered_map<KeyType, ValueType>::const_iterator it = tmpEdges_.begin();
            it != tmpEdges_.end(); ++it)
    {
        saved_tmp_map[it->first] = it->second;
    }

    for (typename std::map<KeyType, ValueType>::const_iterator it = saved_tmp_map.begin();
            it != saved_tmp_map.end(); ++it)
    {
        char* kbuf;
        std::size_t klen;
        izenelib::util::izene_serialization<std::pair<KeyType, ValueType> > izsKey(*it);
        izsKey.write_image(kbuf, klen);
        ofs.write((const char*)(&klen), sizeof(klen));
        ofs.write(kbuf, klen);
    }

    ofs.write((const char*)(&fbs_count_), sizeof(fbs_count_));
    for (size_t i = 0; i < fbs_count_; ++i)
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

    changed_ = false;
    return 0;
}

template <class KeyType, class ValueType>
void Fujimap<KeyType, ValueType>::clear(bool kf_clear)
{
    changed_ = true;
    boost::unordered_map<std::string, uint32_t>().swap(val2code_);
    std::vector<std::string>().swap(code2val_);

    boost::unordered_map<KeyType, ValueType>().swap(tmpEdges_);
    std::vector<std::vector<FujimapBlock<ValueType> > >().swap(fbs_);

    if (kf_clear)
        kf_.clear();
}

template <class KeyType, class ValueType>
std::string Fujimap<KeyType, ValueType>::what() const
{
    return what_.str();
}

template <class KeyType, class ValueType>
bool Fujimap<KeyType, ValueType>::empty() const
{
    return fbs_.empty() && tmpEdges_.empty();
}

}}

NS_IZENELIB_AM_END

#endif // FUJIMAP_HPP__
