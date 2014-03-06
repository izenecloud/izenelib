/*
 *  Copyright (c) 2012 Daisuke Okanohara
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

#ifndef RSDIC_ENUM_CODER_HPP_
#define RSDIC_ENUM_CODER_HPP_

#include <types.h>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace rsdic
{

class EnumCoder
{
public:
    static uint64_t Encode(uint64_t val, size_t rank_sb);
    static uint64_t Decode(uint64_t code, size_t rank_sb);
    static bool GetBit(uint64_t code, size_t rank_sb, size_t pos);
    static bool GetBit(uint64_t code, size_t rank_sb, size_t pos, size_t& rank);
    static size_t Rank(uint64_t code, size_t rank_sb, size_t pos);
    static size_t Select(uint64_t code, size_t rank_sb, size_t num, bool bit);

    static inline uint8_t Len(size_t rank_sb)
    {
        return kEnumCodeLength_[rank_sb];
    }

private:
    static size_t Select0Enum_(uint64_t code, size_t rank_sb, size_t num);
    static size_t Select1Enum_(uint64_t code, size_t rank_sb, size_t num);

    static const uint8_t kEnumCodeLength_[65];
    static const uint64_t kCombinationTable64_[33][64];
};

}
}

NS_IZENELIB_AM_END

#endif // RSDIC_ENUM_CODER_HPP_
