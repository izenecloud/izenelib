
#ifndef IZENELIB_IR_FILTERINGSEARCHBASE_H
#define IZENELIB_IR_FILTERINGSEARCHBASE_H

#include <util/BoostVariantUtil.h>
#include <am/bitmap/Ewah.h>
#include <am/luxio/BTree.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/store/Directory.h>

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/bind.hpp>
#include <functional>

#include <algorithm>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{
    
template <class From>
class FilteringConvert
{
public:
    static void convert(const From& from, 
    
};
    
template <typename KeyType, typename ValueType, template<class K, class V> class Container>
class FilteringSearchBase
{
    
public:
    typedef Container<KeyType, ValueType> AMType;
    typedef izenelib::am::AmForwardIterator<AMType> AMIterator;
    FilteringSearchBase(AMType* am):am_(am)
    {
    }
    
    void getValue(const KeyType& key, BitVector& docs)
    {
        container
        AMIterator it(*container_, key);
        getValueImpl_(key, docs);
    }
    
    
private:
    AMType* am_;
};
    
}

NS_IZENELIB_IR_END

