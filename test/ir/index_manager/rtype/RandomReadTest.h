#ifndef IZENELIB_IR_RANDOMREADTEST_H_
#define IZENELIB_IR_RANDOMREADTEST_H_
#include <ir/index_manager/index/rtype/BTreeIndexer.h>
#include <ir/index_manager/index/rtype/InMemoryBTreeIndexer.h>

using namespace izenelib::ir::indexmanager;

template <class KeyType>
class RandomReadTest
{
typedef CBTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:

    static bool Test(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        uint32_t func_count = 7;
        uint32_t func_num = 0;
        RandomGenerator<uint32_t>::Gen(0, func_count, func_num);
        bool result = false;
        BitVector docs1;
        BitVector docs2;
        switch (func_num) 
        {
            case 0:
                result = indexer.seek(key) == ref.seek(key);
                break;
            case 1:
                indexer.getValue(key, docs1);
                ref.getValue(key, docs2);
                result = (docs1==docs2);
                break;
            case 2:
                indexer.getValueLess(key, docs1);
                ref.getValueLess(key, docs2);
                result = (docs1==docs2);
                break;
            case 3:
                indexer.getValueLessEqual(key, docs1);
                ref.getValueLessEqual(key, docs2);
                result = (docs1==docs2);
                break;
            case 4:
                indexer.getValueGreat(key, docs1);
                ref.getValueGreat(key, docs2);
                result = (docs1==docs2);
                break;
            case 5:
                indexer.getValueGreatEqual(key, docs1);
                ref.getValueGreatEqual(key, docs2);
                result = (docs1==docs2);
                break;
            case 6:
            {
                KeyType key2;
                RandomGenerator<KeyType>::Gen(key2);
                indexer.getValueBetween(key, key2, docs1);
                ref.getValueBetween(key, key2, docs2);
                result = (docs1==docs2);
            }
                break;
            default:
                result = false;
        }
        return result;
    }
    
};


template <>
class RandomReadTest<izenelib::util::UString>
{
typedef izenelib::util::UString KeyType;
typedef CBTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:

    static bool Test(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        uint32_t func_count = 10;
        uint32_t func_num = 0;
        RandomGenerator<uint32_t>::Gen(0, func_count, func_num);
        bool result = false;
        BitVector docs1;
        BitVector docs2;
        switch (func_num) 
        {
            case 0:
                result = indexer.seek(key) == ref.seek(key);
                break;
            case 1:
                indexer.getValue(key, docs1);
                ref.getValue(key, docs2);
                result = (docs1==docs2);
                break;
            case 2:
                indexer.getValueLess(key, docs1);
                ref.getValueLess(key, docs2);
                result = (docs1==docs2);
                break;
            case 3:
                indexer.getValueLessEqual(key, docs1);
                ref.getValueLessEqual(key, docs2);
                result = (docs1==docs2);
                break;
            case 4:
                indexer.getValueGreat(key, docs1);
                ref.getValueGreat(key, docs2);
                result = (docs1==docs2);
                break;
            case 5:
                indexer.getValueGreatEqual(key, docs1);
                ref.getValueGreatEqual(key, docs2);
                result = (docs1==docs2);
                break;
            case 6:
            {
                KeyType key2;
                RandomGenerator<KeyType>::Gen(key2);
                indexer.getValueBetween(key, key2, docs1);
                ref.getValueBetween(key, key2, docs2);
                result = (docs1==docs2);
            }
                break;
            case 7:
                indexer.getValueStart(key, docs1);
                ref.getValueStart(key, docs2);
                result = (docs1==docs2);
                break;
            case 8:
                indexer.getValueEnd(key, docs1);
                ref.getValueEnd(key, docs2);
                result = (docs1==docs2);
                break;
            case 9:
                indexer.getValueSubString(key, docs1);
                ref.getValueSubString(key, docs2);
                result = (docs1==docs2);
                break;
            default:
                result = false;
        }
        return result;
    }
    
};

#endif


