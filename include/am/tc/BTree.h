#ifndef AM_TC_BTREE_H
#define AM_TC_BTREE_H
/**
 * @file am/tc/BTree.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:37:24>
 * @date Updated <2009-09-10 20:50:44>
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

        izenelib::am::CompareFunctor<KeyType> _comp;
        KeyType keyA, keyB;
        izene_deserialization<KeyType> izdA(dataA, sizeA);
        izene_deserialization<KeyType> izdB(dataB, sizeB);
        izdA.read_image(keyA);
        izdB.read_image(keyB);

        return _comp(keyA, keyB);
    }
};

template<typename KeyType,
         typename ValueType,
         typename Comp=BTreeLessCmp<KeyType> >
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
    : btree_(file), comp_()
    {
        if (!boost::is_same<Comp, BTreeLexicalCmp>::value)
        {
            btree_.setcmpfunc(&self_type::compare,
                             static_cast<void*>(&comp_));
        }
    }

    //@{
    //@brief tuning functions used before opening, refer to the manual of
    //tokyo cabinet

    bool setmutex()
    {
        return btree_.setmutex();
    }
    bool setcache(int32_t lcnum, int32_t ncnum)
    {
        return btree_.setcache(lcnum, ncnum);
    }
    bool tune(int32_t lmemb, int32_t nmemb,
              int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts)
    {
        return btree_.tune(lmemb, nmemb, bnum, apow, fpow, opts);
    }
    bool setxmsiz(int64_t xmsiz)
    {
        return btree_.setxmsiz(xmsiz);
    }
    bool setdfunit(int32_t dfunit)
    {
        return btree_.setdfunit(dfunit);
    }
    //@}

    const char* errorMessage() const
    {
        return btree_.errorMessage();
    }
    static inline const char* errorMessage(int code)
    {
        return raw::BTree::errorMessage(code);
    }
    int errorCode() const
    {
        return btree_.errorCode();
    }

    static int compare(const char* dataA, int sizeA,
                       const char* dataB, int sizeB,
                       void* p)
    {
        Comp* pHash = static_cast<Comp*>(p);
        return pHash->compare(dataA, sizeA, dataB, sizeB);
    }

private:
    raw::BTree btree_;
    Comp comp_;

    raw::BTree& rawAm()
    {
        return btree_;
    }
    const raw::BTree& rawAm() const
    {
        return btree_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::tc

#endif // AM_TC_BTREE_H
