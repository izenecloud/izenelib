#ifndef IZENELIB_IR_RANDOMREADTEST_H_
#define IZENELIB_IR_RANDOMREADTEST_H_
#include <ir/index_manager/index/rtype/BTreeIndexer.h>
#include <ir/index_manager/index/rtype/InMemoryBTreeIndexer.h>

#define TEST_DEBUG

using namespace izenelib::ir::indexmanager;

template <class KeyType>
class RandomReadTest
{
typedef BTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:

    static bool SimpleTest(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        bool result = false;
        Bitset docs1;
        Bitset docs2;
#ifdef TEST_DEBUG
        //LOG(ERROR)<<"getValueGreatEqual "<<key<<std::endl;
#endif
        indexer.getValueGreatEqual(key, docs1);
        ref.getValueGreatEqual(key, docs2);
        result = docs1.equal_ignore_size(docs2);

#ifdef TEST_DEBUG
        if(!result)
        {
            LOG(ERROR)<<"failed reason:"<<std::endl;
            LOG(ERROR)<<docs1<<std::endl;
            LOG(ERROR)<<docs2<<std::endl;
            std::size_t csize = std::min(docs1.size(), docs2.size());
            for(std::size_t i=0;i<csize;i++)
            {
                if(docs1.test(i)!=docs2.test(i))
                {
                    LOG(ERROR)<<"failed bits : "<<i<<std::endl;
                }
            }
        }
#endif
        //LOG(ERROR)<<"[docs count]"<<docs1.count()<<","<<docs2.count()<<std::endl;

        return result;
    }

    static bool Test(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        uint32_t func_count = 7;
        uint32_t func_num = 0;
        RandomGenerator<uint32_t>::Gen(0, func_count-1, func_num);
        bool result = false;
        Bitset docs1;
        Bitset docs2;
        switch (func_num)
        {
            case 0:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"seek "<<key<<std::endl;
#endif
                result = indexer.seek(key) == ref.seek(key);
                break;
            case 1:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValue "<<key<<std::endl;
#endif
                indexer.getValue(key, docs1);
                ref.getValue(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 2:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueLess "<<key<<std::endl;
#endif
                indexer.getValueLess(key, docs1);
                ref.getValueLess(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 3:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueLessEqual "<<key<<std::endl;
#endif
                indexer.getValueLessEqual(key, docs1);
                ref.getValueLessEqual(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 4:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueGreat "<<key<<std::endl;
#endif
                indexer.getValueGreat(key, docs1);
                ref.getValueGreat(key, docs2);
                result = docs1.equal_ignore_size(docs2);
// #ifdef TEST_DEBUG
//                 while(!result)
//                 {
//                     LOG(ERROR)<<"retring getValueGreat...."<<std::endl;
//                     docs1.clear();
//                     docs2.clear();
//                     indexer.getValueGreat(key-1, docs1);
//                     ref.getValueGreat(key-1, docs2);
//                     result = docs1.equal_ignore_size(docs2);
//                 }
// #endif
                break;
            case 5:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueGreatEqual "<<key<<std::endl;
#endif
                indexer.getValueGreatEqual(key, docs1);
                ref.getValueGreatEqual(key, docs2);
                result = docs1.equal_ignore_size(docs2);
// #ifdef TEST_DEBUG
//                 while(!result)
//                 {
//                     LOG(ERROR)<<"retring getValueGreatEqual...."<<std::endl;
//                     docs1.clear();
//                     docs2.clear();
//                     indexer.getValueGreatEqual(key-1, docs1);
//                     ref.getValueGreatEqual(key-1, docs2);
//                     result = docs1.equal_ignore_size(docs2);
//                 }
// #endif
                break;
            case 6:
            {
                KeyType key2;
                RandomGenerator<KeyType>::Gen(key2);
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueBetween "<<key<<","<<key2<<std::endl;
#endif
                indexer.getValueBetween(key, key2, docs1);
                ref.getValueBetween(key, key2, docs2);
                result = docs1.equal_ignore_size(docs2);
            }
                break;
            default:
                result = false;
        }
#ifdef TEST_DEBUG
        if(!result)
        {
            LOG(ERROR)<<"failed reason:"<<std::endl;
            LOG(ERROR)<<docs1<<std::endl;
            LOG(ERROR)<<docs2<<std::endl;
            std::size_t csize = std::min(docs1.size(), docs2.size());
            for(std::size_t i=0;i<csize;i++)
            {
                if(docs1.test(i)!=docs2.test(i))
                {
                    LOG(ERROR)<<"failed bits : "<<i<<std::endl;
                }
            }
        }
#endif
        //LOG(ERROR)<<"[docs count]"<<docs1.count()<<","<<docs2.count()<<std::endl;

        return result;
    }

};


template <>
class RandomReadTest<izenelib::util::UString>
{
typedef izenelib::util::UString KeyType;
typedef BTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:

    static bool SimpleTest(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        bool result = false;
        Bitset docs1;
        Bitset docs2;
#ifdef TEST_DEBUG
       // LOG(ERROR)<<"getValueGreatEqual "<<key<<std::endl;
#endif
        indexer.getValueGreatEqual(key, docs1);
        ref.getValueGreatEqual(key, docs2);
        result = docs1.equal_ignore_size(docs2);

#ifdef TEST_DEBUG
        if(!result)
        {
            LOG(ERROR)<<"failed reason:"<<std::endl;
            LOG(ERROR)<<docs1<<std::endl;
            LOG(ERROR)<<docs2<<std::endl;
            std::size_t csize = std::min(docs1.size(), docs2.size());
            for(std::size_t i=0;i<csize;i++)
            {
                if(docs1.test(i)!=docs2.test(i))
                {
                    LOG(ERROR)<<"failed bits : "<<i<<std::endl;
                }
            }
        }
#endif
        //LOG(ERROR)<<"[docs count]"<<docs1.count()<<","<<docs2.count()<<std::endl;

        return result;
    }

    static bool Test(IndexerType& indexer, RefType& ref)
    {
        KeyType key;
        RandomGenerator<KeyType>::Gen(key);
        uint32_t func_count = 10;
        uint32_t func_num = 0;
        RandomGenerator<uint32_t>::Gen(0, func_count-1, func_num);
        bool result = false;
        Bitset docs1;
        Bitset docs2;
        switch (func_num)
        {
            case 0:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"seek "<<key<<std::endl;
#endif
                result = indexer.seek(key) == ref.seek(key);
#ifdef TEST_DEBUG
                if(!result)
                {
                    LOG(ERROR)<<"seek failed : "<<(int)indexer.seek(key)<<","<<(int)ref.seek(key)<<std::endl;
                }
#endif
                break;
            case 1:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValue "<<key<<std::endl;
#endif
                indexer.getValue(key, docs1);
                ref.getValue(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 2:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueLess "<<key<<std::endl;
#endif
                indexer.getValueLess(key, docs1);
                ref.getValueLess(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 3:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueLessEqual "<<key<<std::endl;
#endif
                indexer.getValueLessEqual(key, docs1);
                ref.getValueLessEqual(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 4:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueGreat "<<key<<std::endl;
#endif
                indexer.getValueGreat(key, docs1);
                ref.getValueGreat(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 5:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueGreatEqual "<<key<<std::endl;
#endif
                indexer.getValueGreatEqual(key, docs1);
                ref.getValueGreatEqual(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 6:
            {
                KeyType key2;
                RandomGenerator<KeyType>::Gen(key2);
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueBetween "<<key<<","<<key2<<std::endl;
#endif
                indexer.getValueBetween(key, key2, docs1);
                ref.getValueBetween(key, key2, docs2);
                result = docs1.equal_ignore_size(docs2);
            }
                break;
            case 7:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueStart "<<key<<std::endl;
#endif
                indexer.getValueStart(key, docs1);
                ref.getValueStart(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 8:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueEnd "<<key<<std::endl;
#endif
                indexer.getValueEnd(key, docs1);
                ref.getValueEnd(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            case 9:
#ifdef TEST_DEBUG
                //LOG(ERROR)<<"getValueSubString "<<key<<std::endl;
#endif
                indexer.getValueSubString(key, docs1);
                ref.getValueSubString(key, docs2);
                result = docs1.equal_ignore_size(docs2);
                break;
            default:
                result = false;
        }
#ifdef TEST_DEBUG
        if(!result)
        {
            LOG(ERROR)<<"failed reason:"<<std::endl;
            LOG(ERROR)<<docs1<<std::endl;
            LOG(ERROR)<<docs2<<std::endl;
            std::size_t csize = std::min(docs1.size(), docs2.size());
            for(std::size_t i=0;i<csize;i++)
            {
                if(docs1.test(i)!=docs2.test(i))
                {
                    LOG(ERROR)<<"failed bits : "<<i<<std::endl;
                }
            }
        }
#endif
        //LOG(ERROR)<<"[docs count]"<<docs1.count()<<","<<docs2.count()<<std::endl;
        return result;
    }

};

#endif
