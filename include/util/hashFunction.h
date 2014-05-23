//-*-C++-*-
/**
 * @file  HashFunction.h
 * @brief The definitions of hashing functions used in
 *        hashing methods.
 */
#ifndef HashFunction_h
#define HashFunction_h 1
#include <stdlib.h>
#include "../am/util/DbObj.h"
//#include "../am/util/Wrapper.h"
#include "izene_serialization.h"
#include <openssl/evp.h>

NS_IZENELIB_UTIL_BEGIN

typedef uint64_t ub8; /** unsigned 8-byte quantities */
typedef uint32_t ub4; /** unsigned 4-byte quantities */
typedef uint16_t ub2; /** unsigned 2-byte quantities */
typedef unsigned char ub1; /** unsigned 1-byte quantities */
#define hashsize(n) ((ub8)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}
#define mix64(a,b,c) \
{ \
  a=a-b;  a=a-c;  a=a^(c>>43); \
  b=b-c;  b=b-a;  b=b^(a<<9); \
  c=c-a;  c=c-b;  c=c^(b>>8); \
  a=a-b;  a=a-c;  a=a^(c>>38); \
  b=b-c;  b=b-a;  b=b^(a<<23); \
  c=c-a;  c=c-b;  c=c^(b>>5); \
  a=a-b;  a=a-c;  a=a^(c>>35); \
  b=b-c;  b=b-a;  b=b^(a<<49); \
  c=c-a;  c=c-b;  c=c^(b>>11); \
  a=a-b;  a=a-c;  a=a^(c>>12); \
  b=b-c;  b=b-a;  b=b^(a<<18); \
  c=c-a;  c=c-b;  c=c^(b>>22); \
}

#if defined(_MSC_VER)

#include <stdlib.h>

#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

inline uint64_t rotl64(uint64_t x, int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

inline uint32_t getblock ( const uint32_t * p, int i )
{
    return p[i];
}

inline uint64_t getblock ( const uint64_t * p, int i )
{
    return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

inline uint32_t fmix ( uint32_t h )
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

inline uint64_t fmix ( uint64_t k )
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

inline uint32_t MurmurHash2(const void* key, int32_t len, uint32_t seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const uint8_t * data = (const uint8_t*)key;

    static const uint32_t m = 0x5bd1e995;
    static const int32_t r = 24;

    // Initialize the hash to a 'random' value

    uint32_t h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    while (len >= 4)
    {
        uint32_t k = *(uint32_t *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch (len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

inline uint64_t MurmurHash64A(const void* key, int32_t len, uint64_t seed)
{
    static const uint64_t m = BIG_CONSTANT(0xc6a4a7935bd1e995);
    static const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while (data != end)
    {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
    case 7: h ^= uint64_t(data2[6]) << 48;
    case 6: h ^= uint64_t(data2[5]) << 40;
    case 5: h ^= uint64_t(data2[4]) << 32;
    case 4: h ^= uint64_t(data2[3]) << 24;
    case 3: h ^= uint64_t(data2[2]) << 16;
    case 2: h ^= uint64_t(data2[1]) << 8;
    case 1: h ^= uint64_t(data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

inline uint128_t MurmurHash3_x64_128(const void* key, const int32_t len, const uint32_t seed)
{
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    static const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    static const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body

    const uint64_t * blocks = (const uint64_t *)(data);

    for(int i = 0; i < nblocks; i++)
    {
        uint64_t k1 = blocks[i*2+0];
        uint64_t k2 = blocks[i*2+1];

        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;

        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    //----------
    // tail

    const uint8_t * tail = (const uint8_t*)(data + nblocks*16);

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch(len & 15)
    {
    case 15: k2 ^= uint64_t(tail[14]) << 48;
    case 14: k2 ^= uint64_t(tail[13]) << 40;
    case 13: k2 ^= uint64_t(tail[12]) << 32;
    case 12: k2 ^= uint64_t(tail[11]) << 24;
    case 11: k2 ^= uint64_t(tail[10]) << 16;
    case 10: k2 ^= uint64_t(tail[ 9]) << 8;
    case  9: k2 ^= uint64_t(tail[ 8]) << 0;
             k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

    case  8: k1 ^= uint64_t(tail[ 7]) << 56;
    case  7: k1 ^= uint64_t(tail[ 6]) << 48;
    case  6: k1 ^= uint64_t(tail[ 5]) << 40;
    case  5: k1 ^= uint64_t(tail[ 4]) << 32;
    case  4: k1 ^= uint64_t(tail[ 3]) << 24;
    case  3: k1 ^= uint64_t(tail[ 2]) << 16;
    case  2: k1 ^= uint64_t(tail[ 1]) << 8;
    case  1: k1 ^= uint64_t(tail[ 0]) << 0;
             k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix(h1);
    h2 = fmix(h2);

    h1 += h2;
    h2 += h1;

    return uint128_t(h1) | (uint128_t(h2) << 64);
}

inline uint32_t IncrementalMurmurHash2(uint8_t data, uint32_t h) {
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    static const uint32_t m = 0x5bd1e995;

    h ^= data;
    h *= m;

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
template <class KeyType>
/**
 * @brief The wrapper for all kinds of hashing functions.
 */
class HashFunction {
private:
    static const ub4 init_pattern_1 = 0x00000000;
    static const ub4 init_pattern_2 = 0x13572468;

    static ub4 calcHash(register const char *k, register uint32_t length,
            register uint32_t initval) {
        register uint32_t a, b, c, len;
        /* Set up the internal state */

        len = length;
        a = b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
        c = initval; /* the previous hash value */
        /*---------------------------------------- handle most of the key */
        while (len >= 12) {
            a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16)
                    +((uint32_t)k[3]<<24));
            b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16)
                    +((uint32_t)k[7]<<24));
            c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)
                    +((uint32_t)k[11]<<24));
            mix(a,b,c);
            k += 12;
            len -= 12;
        }
        /*------------------------------------- handle the last 11 bytes */
        c += length;
        switch (len) /* all the case statements fall through */
        {
        case 11:
            c+=((uint32_t)k[10]<<24);
        case 10:
            c+=((uint32_t)k[9]<<16);
        case 9:
            c+=((uint32_t)k[8]<<8);
            /* the first byte of c is reserved for the length */
        case 8:
            b+=((uint32_t)k[7]<<24);
        case 7:
            b+=((uint32_t)k[6]<<16);
        case 6:
            b+=((uint32_t)k[5]<<8);
        case 5:
            b+=k[4];
        case 4:
            a+=((uint32_t)k[3]<<24);
        case 3:
            a+=((uint32_t)k[2]<<16);
        case 2:
            a+=((uint32_t)k[1]<<8);
        case 1:
            a+=k[0];
            /* case 0: nothing left to add */
        }
        mix(a,b,c);

        /*-------------------------------------------- report the result */
        return c;
    }

public:
    static const int PRIME = 1048583; // often used in computing hash value. Big prime number.

    static const ub8 DEFAULT_SEED_VALUE = 0x9e3779b97f4a7c13LL;
    static ub8 hashFN64(const KeyType& key, ub8 seed = DEFAULT_SEED_VALUE) {
        register ub1 *k = (ub1 *)key.c_str();
        register ub4 length = key.size();
        register ub8 a, b, c;
        register ub4 len;

        /* Set up the internal state */
        len = length;
        a = b = seed; /* the golden ratio; an arbitrary value */
        c = 0x7902cac39f392d18LL;

        /*---------------------------------------- handle most of the key */
        while (len >= 24)
        {
            a += (k[0] +((ub8)k[1]<<8) +((ub8)k[2]<<16) +((ub8)k[3]<<24)
                    +((ub8)k[4]<<32)+((ub8)k[5]<<40)+((ub8)k[6]<<48)+((ub8)k[7]<<56));
            b += (k[8] +((ub8)k[9]<<8) +((ub8)k[10]<<16) +((ub8)k[11]<<24)
                    +((ub8)k[12]<<32)+((ub8)k[13]<<40)+((ub8)k[14]<<48)+((ub8)k[15]<<56));
            c += (k[16] +((ub8)k[17]<<8) +((ub8)k[18]<<16)+((ub8)k[19]<<24)
                    +((ub8)k[20]<<32)+((ub8)k[21]<<40)+((ub8)k[22]<<48)+((ub8)k[23]<<56));
            mix(a,b,c);
            k += 24; len -= 24;
        }

        /*------------------------------------- handle the last 11 bytes */
        c += length;
        switch(len) /* all the case statements fall through */
        {
            case 23 : c+=((ub8)k[22]<<56);
            case 22 : c+=((ub8)k[21]<<48);
            case 21 : c+=((ub8)k[20]<<40);
            case 20 : c+=((ub8)k[19]<<32);
            case 19 : c+=((ub8)k[18]<<24);
            case 18 : c+=((ub8)k[17]<<16);
            case 17 : c+=((ub8)k[16]<<8);
            /* the first byte of c is reserved for the length */
            case 16 : b+=((ub8)k[15]<<56);
            case 15 : b+=((ub8)k[14]<<48);
            case 14 : b+=((ub8)k[13]<<40);
            case 13 : b+=((ub8)k[12]<<32);
            case 12 : b+=((ub8)k[11]<<24);
            case 11 : b+=((ub8)k[10]<<16);
            case 10 : b+=((ub8)k[9]<<8);
            case 9 : b+=k[8];
            case 8 : a+=((ub8)k[7]<<56);
            case 7 : a+=((ub8)k[6]<<48);
            case 6 : a+=((ub8)k[5]<<40);
            case 5 : a+=((ub8)k[4]<<32);
            case 4 : a+=((ub8)k[3]<<24);
            case 3 : a+=((ub8)k[2]<<16);
            case 2 : a+=((ub8)k[1]<<8);
            case 1 : a+=k[0];
            /* case 0: nothing left to add */
        }
        mix(a,b,c);
        /*-------------------------------------------- report the result */
        return c;
    }

    static ub8 generateHash64 (const char* token, const size_t len ) {
        ub8 id = 0L;
        ub8 id1, id2;
        id1 = calcHash (token, len, init_pattern_1);
        id2 = calcHash (token, len, init_pattern_2);
        id = (id1 << 32) | id2;

        return id;
    }

    static ub8 generateHash64 (const KeyType& key) {
        const char *token = (const char *) key.c_str();
        const unsigned int len = key.size();
        return MurmurHash64A(token, len, 0);
    }

    static uint128_t generateHash128 (const KeyType& key) {
        const char *token = (const char *) key.c_str();
        const unsigned int len = key.size();
        return MurmurHash3_x64_128(token, len, 0);
    }

    static ub8 generateHash64BySer (const KeyType& key) {
        using namespace izenelib::am::util;
        char* token = 0;
        size_t len;
        izene_serialization<KeyType> izs(key);
        izs.write_image(token, len);
        ub8 id = 0L;
        ub8 id1, id2;
        id1 = calcHash (token, len, init_pattern_1);
        id2 = calcHash (token, len, init_pattern_2);
        id = (id1 << 32) | id2;

        return id;
    }

    static ub4 generateHash32 (const char* token, const size_t len ) {
        return calcHash (token, len, init_pattern_1);
    }

    static ub4 generateHash32 (const KeyType& key) {
        const char *token = (const char *) key.c_str();
        const unsigned int len = key.size();

        return calcHash (token, len, init_pattern_1);
    }

    static std::string generateMD5(const char* inbuf, size_t in_length) {
        EVP_MD_CTX mdctx;
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len;

        EVP_DigestInit(&mdctx, EVP_md5());
        EVP_DigestUpdate(&mdctx, (const void*) inbuf, in_length);
        EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
        EVP_MD_CTX_cleanup(&mdctx);

        return std::string((char*)md_value, (size_t)md_len);		
    }

    static std::string generateSHA1(const char* inbuf, size_t in_length) {
        EVP_MD_CTX mdctx;
        std::string ret;
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len;

        EVP_DigestInit(&mdctx, EVP_sha1());
        EVP_DigestUpdate(&mdctx, (const void*) inbuf, in_length);
        EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
        EVP_MD_CTX_cleanup(&mdctx);

        return std::string((char*)md_value, (size_t)md_len);
    }

    static ub4 convert_key(const KeyType& key) {
        register ub4 convkey = 0;
        register const char* str = (const char*)key.c_str();
        for (register size_t i = 0; i < key.size(); i++)
        convkey = 37*convkey + *str++;

        return convkey;
    }

    static ub4 djb2(const KeyType& key) {
        register ub4 h = 5381;
        register const char* str = (const char*)key.c_str();
        for (register size_t i = 0; i < key.size(); i++)
            h = ((h << 5) + h) + *str++;
        return h;
    }

    static ub4 sdbm(const KeyType& key) {
        register ub4 h = 0;
        register const char* str = (const char*)key.c_str();
        for (register size_t i = 0; i < key.size(); i++)
        h = *str++ + (h << 6) + (h << 16) - h;
        return h;
    }
};

template<typename KeyType> inline ub4 sdb_hashing(const KeyType& key) {
    using namespace izenelib::am::util;

    char* ptr = 0;
    size_t ksize;
    izene_serialization<KeyType> izs(key);
    izs.write_image(ptr, ksize);
    uint32_t convkey = 0;
    char* str = ptr;
    for (size_t i = 0; i < ksize; i++)
        convkey = 37*convkey + (uint8_t)*str++;
    return convkey;
}

template<class T> struct HashFunctor {
    size_t operator()(const T& key) const {
        using namespace izenelib::am::util;
        char* ptr = 0;
        size_t ksize;
        izene_serialization<T> izs(key);
        izs.write_image(ptr, ksize);

        char* str = ptr;
        uint32_t convkey = 0;
        for (size_t i = 0; i < ksize; i++)
            convkey = 37*convkey + (uint8_t)*str++;
        return convkey;
    }
};

inline uint32_t sdb_hash_fun(const void* kbuf, const size_t ksize) {
    uint32_t convkey = 0;
    const char* str = (const char*)kbuf;
    for (size_t i = 0; i < ksize; i++)
        convkey = 37*convkey + (uint8_t)*str++;
    return convkey;
}

template<typename KeyType> inline ub4 izene_hashing(const KeyType& key) {
    char* ptr = 0;
    size_t ksize;
    izene_serialization<KeyType> izs(key);
    izs.write_image(ptr, ksize);

    uint32_t convkey = 0;
    for (size_t i = 0; i < ksize; i++)
        convkey = 37*convkey + (uint8_t)*ptr++;
    return convkey % HashFunction<KeyType>::PRIME;
}


template<class T> struct izene_HashFunctor {
size_t operator()(const T& key) const {
    return izene_hashing(key);
}
};

template <typename KeyType, typename HashType> struct HashIDTraits;

template<typename KeyType>
struct HashIDTraits<KeyType, uint32_t>
{
    uint32_t operator()(const KeyType& key, uint32_t seed = 0) const {
        using namespace izenelib::am::util;
        char* ptr = 0;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        return MurmurHash2(ptr,ksize,seed);
    }
};

template<typename KeyType>
struct HashIDTraits<KeyType, uint64_t>
{
    uint64_t operator()(const KeyType& key, uint64_t seed = 0) const {
        using namespace izenelib::am::util;
        char* ptr = 0;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        return MurmurHash64A(ptr,ksize,seed);
    }
};

template<typename KeyType>
struct HashIDTraits<KeyType, uint128_t>
{
    uint128_t operator()(const KeyType& key, uint32_t seed = 0) const {
        using namespace izenelib::am::util;
        char* ptr = 0;
        size_t ksize;
        izene_serialization<KeyType> izs(key);
        izs.write_image(ptr, ksize);

        return MurmurHash3_x64_128(ptr,ksize,seed);
    }
};

NS_IZENELIB_UTIL_END
#endif
