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

#ifndef WATARRAY_FM_INDEX_HPP_
#define WATARRAY_FM_INDEX_HPP_

#include "wat_array.hpp"

#include <util/ustring/UString.h>
#include <string>
#include <map>
#include <fstream>


namespace wat_array
{

class FMIndex
{
public:
    FMIndex(size_t length);
    ~FMIndex();
    void clear();
    void push_back(const izenelib::util::UString &doc);
    void build(char end_marker = 1, uint64_t ddic = 64, bool is_msg = false);
    uint64_t length() const
    {
        return wt_.length();
    }
    uint64_t docLength() const
    {
        return doctails_.length();
    }
    uint64_t GetRows(const izenelib::util::UString &key) const;
    uint64_t GetRows(const izenelib::util::UString &key, uint64_t &first, uint64_t &last) const;
    uint64_t GetPosition(uint64_t i) const;
    const izenelib::util::UString &GetSubstring(uint64_t pos, uint64_t len);
    uint64_t GetDocumentId(uint64_t pos) const;
    void Search(const izenelib::util::UString &key, std::map<uint64_t, uint64_t> &dids) const;
    const izenelib::util::UString &GetDocument(uint64_t did);
    void Write(std::ofstream &ofs) const;
    void Write(const char *filename) const;
    void Read(std::ifstream &ifs);
    void Read(const char *filename);

private:
    WatArray wt_;
    BitArray doctails_;
    std::vector<uint64_t> posdic_;
    std::vector<uint64_t> idic_;
    uint64_t ddic_;
    uint64_t head_;
    uint64_t rlt_[65536];
    izenelib::util::UString substr_;
};

class BWTransform
{
public:
    BWTransform();
    ~BWTransform();
    void clear();
    void build(const uint16_t *str, size_t len);
    uint64_t length() const
    {
        return length_;
    }
    uint64_t head() const
    {
        return head_;
    }
    uint16_t get(uint64_t i) const;
    void get(izenelib::util::UString &str) const;

private:
    void sort_(int64_t begin, int64_t end, uint64_t depth);
    uint16_t sa2char_(uint64_t i, uint64_t depth) const;

private:
    const uint16_t *str_;
    uint64_t length_;
    uint64_t head_;
    std::vector<uint32_t> sa_;
};

}

#endif // WASEQ_WASEQ_HPP_
