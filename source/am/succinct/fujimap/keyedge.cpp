/*
 * keyEdge.cpp
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

#include <am/succinct/fujimap/keyedge.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

uint64_t get64bit(const uint8_t* v)
{
    return (uint64_t)v[0] | ((uint64_t)v[1] << 8) | ((uint64_t)v[2] << 16) | ((uint64_t)v[3] << 24) |
           ((uint64_t)v[4] << 32) | ((uint64_t)v[5] << 40) | ((uint64_t)v[6] << 48) | ((uint64_t)v[7] << 56);
}

// MurmurHash 2.0
// http://murmurhash.googlepages.com/

uint64_t hash (const char* data_, size_t len)
{
    const uint8_t* data = (const uint8_t*)data_;
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = len * m;

    while (len >= 8)
    {
        uint64_t k = get64bit(data);

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
        data += 8;
        len -= 8;
    }

    switch(len & 7)
    {
    case 7:
        h ^= uint64_t(data[6]) << 48;
    case 6:
        h ^= uint64_t(data[5]) << 40;
    case 5:
        h ^= uint64_t(data[4]) << 32;
    case 4:
        h ^= uint64_t(data[3]) << 24;
    case 3:
        h ^= uint64_t(data[2]) << 16;
    case 2:
        h ^= uint64_t(data[1]) << 8;
    case 1:
        h ^= uint64_t(data[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

// Bob Jenkins's Hash
// http://burtleburtle.net/bob/hash/doobs.html

/*
#define bob_mix(a, b, c) \
a -= b; a -= c; a ^= (c >> 13);      \
b -= c; b -= a; b ^= (a << 8);        \
c -= a; c -= b; c ^= (b >> 13);       \
a -= b; a -= c; a ^= (c >> 12);       \
b -= c; b -= a; b ^= (a << 16);       \
c -= a; c -= b; c ^= (b >> 5);        \
a -= b; a -= c; a ^= (c >> 3);        \
b -= c; b -= a; b ^= (a << 10);       \
c -= a; c -= b; c ^= (b >> 15);
*/


#define bob_mix(a, b, c) \
    a -= b; a -= c; a ^= (c>>43); \
    b -= c; b -= a; b ^= (a<<9); \
    c -= a; c -= b; c ^= (b>>8); \
    a -= b; a -= c; a ^= (c>>38); \
    b -= c; b -= a; b ^= (a<<23); \
    c -= a; c -= b; c ^= (b>>5); \
    a -= b; a -= c; a ^= (c>>35); \
    b -= c; b -= a; b ^= (a<<49); \
    c -= a; c -= b; c ^= (b>>11); \
    a -= b; a -= c; a ^= (c>>12); \
    b -= c; b -= a; b ^= (a<<18); \
    c -= a; c -= b; c ^= (b>>22);

/*
#define bob_mix(a, b, c) \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8);
*/

void hash(const char* str, size_t len, uint64_t seed,
          uint64_t& a, uint64_t& b, uint64_t& c)
{
    const uint8_t* p = (const uint8_t*)(str);
    a = 0x9e3779b97f4a7c13LL;
    b = seed;
    c = seed;
    while (len >= 24)
    {
        a += get64bit(p + 0);
        b += get64bit(p + 8);
        c += get64bit(p + 16);
        bob_mix(a, b, c);
        p += 24;
        len -= 24;
    }

    c += static_cast<uint64_t>(len);
    switch(len)
        /* all the case statements fall through */
    {
    case 23:
        c+=((uint64_t)p[22]<<56);
    case 22:
        c+=((uint64_t)p[21]<<48);
    case 21:
        c+=((uint64_t)p[20]<<40);
    case 20:
        c+=((uint64_t)p[19]<<32);
    case 19:
        c+=((uint64_t)p[18]<<24);
    case 18:
        c+=((uint64_t)p[17]<<16);
    case 17:
        c+=((uint64_t)p[16]<<8);
        /* the first byte of c is reserved for the length */
    case 16:
        b+=((uint64_t)p[15]<<56);
    case 15:
        b+=((uint64_t)p[14]<<48);
    case 14:
        b+=((uint64_t)p[13]<<40);
    case 13:
        b+=((uint64_t)p[12]<<32);
    case 12:
        b+=((uint64_t)p[11]<<24);
    case 11:
        b+=((uint64_t)p[10]<<16);
    case 10:
        b+=((uint64_t)p[ 9]<<8);
    case  9:
        b+=((uint64_t)p[ 8]);
    case  8:
        a+=((uint64_t)p[ 7]<<56);
    case  7:
        a+=((uint64_t)p[ 6]<<48);
    case  6:
        a+=((uint64_t)p[ 5]<<40);
    case  5:
        a+=((uint64_t)p[ 4]<<32);
    case  4:
        a+=((uint64_t)p[ 3]<<24);
    case  3:
        a+=((uint64_t)p[ 2]<<16);
    case  2:
        a+=((uint64_t)p[ 1]<<8);
    case  1:
        a+=((uint64_t)p[ 0]);
    default :
        ;
    }

    bob_mix(a, b, c);
}


KeyEdge::KeyEdge(const char* str, const size_t len,
                 const uint64_t code, const uint64_t seed) : code(code)
{
    hash(str, len, seed, v[0], v[1], v[2]);
}

KeyEdge::KeyEdge() :  code(0)
{
}

void KeyEdge::save(ofstream& ofs)
{
    ofs.write((const char*)(&code), sizeof(code));
    ofs.write((const char*)(&v[0]), sizeof(v[0]) * R);
}

void KeyEdge::load(ifstream& ifs)
{
    ifs.read((char*)(&code), sizeof(code));
    ifs.read((char*)(&v[0]), sizeof(v[0]) * R);

}

}}

NS_IZENELIB_AM_END

