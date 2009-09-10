#ifndef AM_TC_BTREE_H
#define AM_TC_BTREE_H
/**
 * @file am/tc/BTree.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:37:24>
 * @date Updated <2009-09-10 20:30:08>
 * @brief TokyoCabinet BTree DB wrapper
 */

#include "raw/BTree.h"

#include <am/raw/AmWrapper.h>
#include <am/range/IterNextRange.h>
#include <am/range/GetNextRange.h>

#include <functional>

#include <boost/assert.hpp>

namespace izenelib {
namespace am {
namespace tc {

struct BTreeLexicalCmp
{
    int compare(const char* dataA, int sizeA,
                const char* dataB, int sizeB) const
    {
        // never go here
        BOOST_ASSERT(false);
        return 0;
    }
};

template<typename KeyType>
struct BTreeLessCmp
{
    int compare(const char* dataA, int sizeA,
                const char* dataB, int sizeB) const
    {
        KeyType a;
        izenelib::util::izene_deserialization<KeyType> izdA(
            dataA, sizeA
        );
        izdA.read_image(a);

        KeyType b;
        izenelib::util::izene_deserialization<KeyType> izdB(
            dataB, sizeB
        );
        izdB.read_image(b);

        if (a < b)
        {
            return -1;
        }
        else if (b < a)
        {
            return 1;
        }

        return 0;
    }
};

template<typename KeyType,
         typename ValueType,
         typename Comp=BTreeLexicalCmp >
class BTree
    : public izenelib::am::raw::AmWrapper<BTree<KeyType, ValueType, Comp>,
                                          raw::BTree,
                                          KeyType,
                                          ValueType>
{
    typedef BTree<KeyType, ValueType, Comp> self_type;

public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename raw::BTree::size_type size_type;

    typedef raw::BTree raw_am_type;

    enum {
        READER = raw::BTree::READER,
        WRITER = raw::BTree::WRITER,
        CREAT  = raw::BTree::CREAT,
        TRUNC  = raw::BTree::TRUNC,
        NOLCK  = raw::BTree::NOLCK,
        LCKNB  = raw::BTree::LCKNB,
        TSYNC  = raw::BTree::TSYNC,
    };

    enum { DEFAULT_OPEN_MODE = raw::BTree::DEFAULT_OPEN_MODE };

    explicit BTree(const std::string& file = "")
    : hash_(file), comp_()
    {
        if (!boost::is_same<Comp, BTreeLexicalCmp>::value)
        {
            hash_.setcmpfunc(&self_type::compare,
                             static_cast<void*>(&comp_));
        }
    }

    //@{
    //@brief tuning functions used before opening, refer to the manual of
    //tokyo cabinet

    bool setmutex()
    {
        return hash_.setmutex();
    }
    bool setcache(int32_t lcnum, int32_t ncnum)
    {
        return hash_.setcache(lcnum, ncnum);
    }
    bool tune(int32_t lmemb, int32_t nmemb,
              int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return hash_.tune(lmemb, nmemb, bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return hash_.setxmsiz(xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return hash_.setdfunit(dfunit);
    }
    //@}

    const char* errorMessage() const
    {
        return hash_.errorMessage();
    }
    static inline const char* errorMessage(int code)
    {
        return raw::BTree::errorMessage(code);
    }
    int errorCode() const
    {
        return hash_.errorCode();
    }

    static int compare(const char* dataA, int sizeA,
                       const char* dataB, int sizeB,
                       void* p)
    {
        Comp* pHash = static_cast<Comp*>(p);
        return pHash->compare(dataA, sizeA, dataB, sizeB);
    }

private:
    raw::BTree hash_;
    Comp comp_;

    raw::BTree& rawAm()
    {
        return hash_;
    }
    const raw::BTree& rawAm() const
    {
        return hash_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::tc

#endif // AM_TC_BTREE_H
