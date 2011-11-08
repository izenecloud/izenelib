#ifndef AM_LUXIO_BTREE_H
#define AM_LUXIO_BTREE_H
/**
 * @file am/luxio/BTree.h
 * @author Yingfeng Zhang
 * @date Created <2011-11-08 23:37:24>
 * @brief LuxIO BTree DB wrapper
 */

#include "raw/BTree.h"

#include <am/raw/AmWrapper.h>

#include <functional>

#include <boost/assert.hpp>

namespace izenelib {
namespace am {
namespace luxio {

template<typename KeyType>
struct LuxIOBTreeCmp
{
    int compare(Lux::IO::data_t& dataA,Lux::IO::data_t& dataB) const
    {
        izenelib::am::CompareFunctor<KeyType> _comp;
        KeyType keyA, keyB;
        izene_deserialization<KeyType> izdA((const char*)dataA.data, dataA.size);
        izene_deserialization<KeyType> izdB((const char*)dataB.data, dataB.size);
        izdA.read_image(keyA);
        izdB.read_image(keyB);

        return _comp(keyA, keyB);
    }
};

template<typename KeyType,
         typename ValueType,
         typename Comp=LuxIOBTreeCmp<KeyType> >
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

    enum IndexType{
        NON_CLUSTER_LINKING = raw::BTree::NON_CLUSTER_LINKING,
        NON_CLUSTER_PADDING = raw::BTree::NON_CLUSTER_PADDING,
        CLUSTER = raw::BTree::CLUSTER,
    };

    explicit BTree(const std::string& file = "")
    : db_(file)
    {
        db_.setcmpfunc(&self_type::compare);
    }

    void set_index_type(IndexType type = NON_CLUSTER_LINKING)
    {
        rawAm().set_index_type((raw::BTree::IndexType)type);
    }

    static int compare(Lux::IO::data_t& dataA,Lux::IO::data_t& dataB)
    {
        static Comp comp_;
        return comp_.compare(dataA, dataB);
    }

private:
    raw::BTree db_;

    raw::BTree& rawAm()
    {
        return db_;
    }
    const raw::BTree& rawAm() const
    {
        return db_;
    }

    friend struct izenelib::am::raw::detail::AmWrapperAccess;
};

}}} // namespace izenelib::am::luxio

#endif // AM_LUXIO_BTREE_H
