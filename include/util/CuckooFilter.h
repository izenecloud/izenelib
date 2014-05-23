/******************************************************
https://github.com/efficient/cuckoofilter
Cuckoo filter is a Bloom filter replacement for approximated set-membership queries. 
While Bloom filters are well-known space-efficient data structures to serve queries like 
"if item x is in a set?", they do not support deletion. Their variances to enable deletion 
(like counting Bloom filters) usually require much more space.

Cuckoo filters provide the flexibility to add and remove items dynamically. A cuckoo filter
is based on cuckoo hashing (and therefore named as cuckoo filter). It is essentially a cuckoo
hash table storing each key's fingerprint. Cuckoo hash tables can be highly compact, 
thus a cuckoo filter could use less space than conventional Bloom filters, for applications 
that require low false positive rates (< 3%).

For details about the algorithm and citations please use this article for now
"Cuckoo Filter: Better Than Bloom" by Bin Fan, Dave Andersen and Michael Kaminsky

A cuckoo filter supports following operations:

    Add(key): insert key to the filter
    Contain(key): return if key is already inserted, it may return false positive results like Bloom filters
    Delete(key): delete the given key from the filter
    Size(): return the total number of keys existing in the filter
    SizeInBytes(): return the filter size in bytes

*******************************************************/
#ifndef IZENELIB_UTIL_CUCKOO_FILTER_H
#define IZENELIB_UTIL_CUCKOO_FILTER_H

#include <cmath>
#include <limits.h>
#include <stdexcept>
#include <types.h>
#include <xmmintrin.h>

#include <util/hashFunction.h>

namespace izenelib
{
namespace util
{

#define haszero4(x) (((x) - 0x1111ULL) & (~(x)) & 0x8888ULL)
#define hasvalue4(x,n) (haszero4((x) ^ (0x1111ULL * (n))))

#define haszero8(x) (((x) - 0x01010101ULL) & (~(x)) & 0x80808080ULL)
#define hasvalue8(x,n) (haszero8((x) ^ (0x01010101ULL * (n))))

#define haszero12(x) (((x) - 0x001001001001ULL) & (~(x)) & 0x800800800800ULL)
#define hasvalue12(x,n) (haszero12((x) ^ (0x001001001001ULL * (n))))

#define haszero16(x) (((x) - 0x0001000100010001ULL) & (~(x)) & 0x8000800080008000ULL)
#define hasvalue16(x,n) (haszero16((x) ^ (0x0001000100010001ULL * (n))))

inline uint64_t upperpower2(uint64_t x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}


// the most naive table implementation: one huge bit array
template <size_t bits_per_tag>
class SingleTable
{
    static const size_t tags_per_bucket  = 4;
    static const size_t bytes_per_bucket = (bits_per_tag * tags_per_bucket + 7) >> 3;

    size_t num_buckets;

    struct Bucket
    {
        char bits_[bytes_per_bucket];
    } __attribute__((__packed__));

    // using a pointer adds one more indirection
    Bucket *buckets_;

public:
    static const uint32_t TAGMASK = (1ULL << bits_per_tag) - 1;

    explicit
    SingleTable(size_t num)
    {
        num_buckets = num;
        buckets_ = new Bucket[num_buckets];
        CleanupTags();
    }

    ~SingleTable()
    {
        delete [] buckets_;
    }

    size_t IndexHash(uint32_t hv)
    {
        size_t index = hv % num_buckets;
        return index;
    }

    uint32_t TagHash(uint32_t hv)
    {
        uint32_t tag;
        tag  = hv & TAGMASK;
        tag += (tag == 0);
        return tag;
    }

    void CleanupTags()
    {
        memset(buckets_, 0, bytes_per_bucket * num_buckets);
    }

    size_t SizeInBytes() const
    {
        return bytes_per_bucket * num_buckets;
    }

    size_t SizeInTags() const
    {
        return tags_per_bucket * num_buckets;
    }

    std::string Info() const
    {
        std::stringstream ss;
        ss << "SingleHashtable with tag size: " << bits_per_tag << " bits \n";
        ss << "\t\tAssociativity: " << tags_per_bucket << "\n";
        ss << "\t\tTotal # of rows: " << num_buckets << "\n";
        ss << "\t\tTotal # slots: " << SizeInTags() << "\n";
        return ss.str();
    }

    // read tag from pos(i,j)
    inline uint32_t ReadTag(const size_t i, const size_t j) const
    {
        const char *p = buckets_[i].bits_;
        uint32_t tag;
        /* following code only works for little-endian */
        if (bits_per_tag == 2)
        {
            tag = *((uint8_t*) p) >> (j * 2);
        }
        else if (bits_per_tag == 4)
        {
            p += (j >> 1);
            tag = *((uint8_t*) p) >> ((j & 1) << 2);
        }
        else if (bits_per_tag == 8)
        {
            p += j;
            tag = *((uint8_t*) p);
        }
        else if (bits_per_tag == 12)
        {
            p += j + (j >> 1);
            tag = *((uint16_t*) p) >> ((j & 1) << 2);
        }
        else if (bits_per_tag == 16)
        {
            p += (j << 1);
            tag = *((uint16_t*) p);
        }
        else if (bits_per_tag == 32)
        {
            tag = ((uint32_t*) p)[j];
        }
        return tag & TAGMASK;
    }

    // write tag to pos(i,j)
    inline void  WriteTag(const size_t i, const size_t j, const uint32_t t)
    {
        char *p = buckets_[i].bits_;
        uint32_t tag = t & TAGMASK;
        /* following code only works for little-endian */
        if (bits_per_tag == 2)
        {
            *((uint8_t*) p) |= tag << (2*j);
        }
        else if (bits_per_tag == 4)
        {
            p += (j >> 1);
            if ( (j & 1) == 0)
            {
                *((uint8_t*) p)  &= 0xf0;
                *((uint8_t*) p)  |= tag;
            }
            else
            {
                *((uint8_t*) p)  &= 0x0f;
                *((uint8_t*) p)  |= (tag << 4);
            }
        }
        else if (bits_per_tag == 8)
        {
            ((uint8_t*) p)[j] =  tag;
        }
        else if (bits_per_tag == 12 )
        {
            p += (j + (j >> 1));
            if ( (j & 1) == 0)
            {
                ((uint16_t*) p)[0] &= 0xf000;
                ((uint16_t*) p)[0] |= tag;
            }
            else
            {
                ((uint16_t*) p)[0] &= 0x000f;
                ((uint16_t*) p)[0] |= (tag << 4);
            }
        }
        else if (bits_per_tag == 16)
        {
            ((uint16_t*) p)[j] = tag;
        }
        else if (bits_per_tag == 32)
        {
            ((uint32_t*) p)[j] = tag;
        }

        return;
    }

    inline bool  FindTagInBuckets(const size_t i1,
                                  const size_t i2,
                                  const uint32_t tag) const
    {
        const char* p1 = buckets_[i1].bits_;
        const char* p2 = buckets_[i2].bits_;

        uint64_t v1 =  *((uint64_t*) p1);
        uint64_t v2 =  *((uint64_t*) p2);

        // caution: unaligned access & assuming little endian
        if (bits_per_tag == 4 && tags_per_bucket == 4)
        {
            return hasvalue4(v1, tag) || hasvalue4(v2, tag);
        }
        else if (bits_per_tag == 8 && tags_per_bucket == 4)
        {
            return hasvalue8(v1, tag) || hasvalue8(v2, tag);
        }
        else if (bits_per_tag == 12 && tags_per_bucket == 4)
        {
            return hasvalue12(v1, tag) || hasvalue12(v2, tag);
        }
        else if (bits_per_tag == 16 && tags_per_bucket == 4)
        {
            return hasvalue16(v1, tag) || hasvalue16(v2, tag);
        }
        else
        {
            for (size_t j = 0; j < tags_per_bucket; j++ )
            {
                if ((ReadTag(i1, j) == tag) || (ReadTag(i2,j) == tag))
                    return true;
            }
            return false;
        }

    }

    inline bool  FindTagInBucket(const size_t i,  const uint32_t tag) const
    {
        // caution: unaligned access & assuming little endian
        if (bits_per_tag == 4 && tags_per_bucket == 4)
        {
            const char* p = buckets_[i].bits_;
            uint64_t v = *(uint64_t*)p; // uint16_t may suffice
            return hasvalue4(v, tag);
        }
        else if (bits_per_tag == 8 && tags_per_bucket == 4)
        {
            const char* p = buckets_[i].bits_;
            uint64_t v = *(uint64_t*)p; // uint32_t may suffice
            return hasvalue8(v, tag);
        }
        else if (bits_per_tag == 12 && tags_per_bucket == 4)
        {
            const char* p = buckets_[i].bits_;
            uint64_t v = *(uint64_t*)p;
            return hasvalue12(v, tag);
        }
        else if (bits_per_tag == 16 && tags_per_bucket == 4)
        {
            const char* p = buckets_[i].bits_;
            uint64_t v = *(uint64_t*)p;
            return hasvalue16(v, tag);
        }
        else
        {
            for (size_t j = 0; j < tags_per_bucket; j++ )
            {
                if (ReadTag(i, j) == tag)
                    return true;
            }
            return false;
        }
    }// FindTagInBucket

    inline  bool  DeleteTagFromBucket(const size_t i,  const uint32_t tag)
    {
        for (size_t j = 0; j < tags_per_bucket; j++ )
        {
            if (ReadTag(i, j) == tag)
            {
                assert (FindTagInBucket(i, tag) == true);
                WriteTag(i, j, 0);
                return true;
            }
        }
        return false;
    }// DeleteTagFromBucket

    inline  bool  InsertTagToBucket(const size_t i,  const uint32_t tag,
                                    const bool kickout, uint32_t& oldtag)
    {
        for (size_t j = 0; j < tags_per_bucket; j++ )
        {
            if (ReadTag(i, j) == 0)
            {
                WriteTag(i, j, tag);
                return true;
            }
        }
        if (kickout)
        {
            size_t r = rand() % tags_per_bucket;
            oldtag = ReadTag(i, r);
            WriteTag(i, r, tag);
        }
        return false;
    }// InsertTagToBucket


    inline size_t NumTagsInBucket(const size_t i)
    {
        size_t num = 0;
        for (size_t j = 0; j < tags_per_bucket; j++ )
        {
            if (ReadTag(i, j) != 0)
            {
                num ++;
            }
        }
        return num;
    } // NumTagsInBucket

};// SingleTable

// The logic to do partial-key cuckoo hashing.
// to cope with different hashtables, e.g.
// cache partitioned, or permutation-encoded,
// subclass Table and pass it to the constructor.
template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType = SingleTable>
class CuckooFilter
{

    TableType<bits_per_key> *table_;
    size_t      num_keys;

    static const size_t MAX_CUCKOO_COUNT = 500;

    inline void IndexTagHash(const KeyType &key, size_t &index, uint32_t &tag) const
    {

        string hashed_key = HashFunction<size_t>::generateSHA1((const char*) &key, sizeof(key));
        uint64_t hv = *((uint64_t*) hashed_key.c_str());

        index = table_->IndexHash((uint32_t) (hv >> 32));
        tag   = table_->TagHash((uint32_t) (hv & 0xFFFFFFFF));
    }

    inline size_t AltIndex(const size_t index, const uint32_t tag) const
    {
        //
        // originally we use:
        // index ^ HashUtil::BobHash((const void*) (&tag), 4)) & table_->INDEXMASK;
        // now doing a quick-n-dirty way:
        // 0x5bd1e995 is the hash constant from MurmurHash2
        //
        return table_->IndexHash((uint32_t) (index ^ (tag * 0x5bd1e995)));
    }

    struct
    {
        size_t index;
        uint32_t tag;
        bool used;
    } victim;

    // load factor is the fraction of occupancy
    double LoadFactor_() const
    {
        return 1.0 * Size()  / table_->SizeInTags();
    }

    double BitsPerKey_() const
    {
        return 8.0 * table_->SizeInBytes() / Size();
    }

    int Add_(const size_t i, const uint32_t tag);

public:
    enum Status
    {
        Ok = 0,
        NotFound = 1,
        NotEnoughSpace = 2,
        NotSupported = 3,
    };

    explicit CuckooFilter(size_t num_keys): num_keys(0)
    {
        size_t assoc       = 4;
        size_t num_buckets = upperpower2(num_keys / assoc);
        double frac        = (double) num_keys / num_buckets / assoc;
        if (frac > 0.96)
        {
            num_buckets <<= 1;
        }
        victim.used = false;
        table_      = new TableType<bits_per_key>(num_buckets);
    }

    ~CuckooFilter()
    {
        delete table_;
    }

    /*
     * A Bloomier filter interface:
     *    Add, Contain, Delete
     */

    // Add a key to the filter.
    int Add(const KeyType& key);

    // Report if the key is inserted, with false positive rate.
    int Contain(const KeyType& key) const;

    // Delete a key from the hash table
    int Delete(const KeyType& key);


    /* methods for providing stats  */
    // summary infomation
    string Info() const;

    // number of current inserted keys;
    size_t Size() const
    {
        return num_keys;
    }

    // size of the filter in bytes.
    size_t SizeInBytes() const
    {
        return table_->SizeInBytes();
    }
}; // declaration of class CuckooFilter


template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType>
int CuckooFilter<KeyType, bits_per_key, TableType>::Add(const KeyType& key)
{
    if (victim.used)
    {
        return NotEnoughSpace;
    }
    size_t i;
    uint32_t tag;
    IndexTagHash(key, i, tag);

    return Add_(i, tag);
}

template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType>
int CuckooFilter<KeyType, bits_per_key, TableType>::Add_(const size_t i, const uint32_t tag)
{
    size_t curindex = i;
    uint32_t curtag = tag;
    uint32_t oldtag;

    for (uint32_t count = 0; count < MAX_CUCKOO_COUNT; count ++)
    {
        bool kickout = (count > 0);
        oldtag = 0;
        if (table_->InsertTagToBucket(curindex, curtag, kickout, oldtag))
        {
            num_keys ++;
            return Ok;
        }

        if (kickout)
        {
            curtag = oldtag;
        }
        curindex = AltIndex(curindex, curtag);
    }

    victim.index = curindex;
    victim.tag = curtag;
    victim.used = true;
    return Ok;
}

template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType>
int CuckooFilter<KeyType, bits_per_key, TableType>::Contain(const KeyType& key) const
{
    bool found = false;
    size_t i1, i2;
    uint32_t tag;

    IndexTagHash(key, i1, tag);
    i2 = AltIndex(i1, tag);

    assert(i1 == AltIndex(i2, tag));

    found = victim.used && (tag == victim.tag) && (i1 == victim.index || i2 == victim.index);

    if (found || table_->FindTagInBuckets(i1, i2, tag))
    {
        return Ok;
    }
    else
    {
        return NotFound;
    }
}

//template <typename KeyType>
template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType>
int CuckooFilter<KeyType, bits_per_key, TableType>::Delete(const KeyType& key)
{
    size_t i1, i2;
    uint32_t tag;

    IndexTagHash(key, i1, tag);
    i2 = AltIndex(i1, tag);

    if (table_->DeleteTagFromBucket(i1, tag))
    {
        num_keys--;
        goto TryEliminateVictim;
    }
    else if (table_->DeleteTagFromBucket(i2, tag))
    {
        num_keys--;
        goto TryEliminateVictim;
    }
    else if (victim.used && tag == victim.tag && (i1 == victim.index || i2 == victim.index))
    {
        //num_keys --;
        victim.used = false;
        return Ok;
    }
    else
    {
        return NotFound;
    }
TryEliminateVictim:
    if (victim.used)
    {
        victim.used = false;
        size_t i = victim.index;
        uint32_t tag = victim.tag;
        Add_(i, tag);
    }
    return Ok;
}

//template <typename KeyType>
template <typename KeyType,
         size_t bits_per_key,
         template<size_t> class TableType>
string CuckooFilter<KeyType, bits_per_key, TableType>::Info() const
{
    stringstream ss;
    ss << "CuckooFilter Status:\n";
#ifdef QUICK_N_DIRTY_HASHING
    ss << "\t\tQuick hashing used\n";
#else
    ss << "\t\tBob hashing used\n";
#endif
    ss << "\t\t" << table_->Info() << "\n";
    ss << "\t\tKeys stored: " << Size() << "\n";
    ss << "\t\tLoad facotr: " << LoadFactor_() << "\n";
    ss << "\t\tHashtable size: " << (table_->SizeInBytes() >> 10) << " KB\n";
    if (Size() > 0)
    {
        ss << "\t\tbit/key:   " << BitsPerKey_() << "\n";
    }
    else
    {
        ss << "\t\tbit/key:   N/A\n";
    }
    return ss.str();
}

}
}
#endif
