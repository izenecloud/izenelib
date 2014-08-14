
#ifndef IZENELIB_IR_TERMENUM_H_
#define IZENELIB_IR_TERMENUM_H_
#include <ir/index_manager/index/rtype/Compare.h>
#include <types.h>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <3rdparty/am/stx/btree_map.h>
#include "InMemoryBTreeCache.h"
#include <util/ClockTimer.h>

// #define TE_DEBUG

NS_IZENELIB_IR_BEGIN
namespace indexmanager {


template <class KeyType, class ValueType>
class TermEnum
{
    public:
    virtual ~TermEnum()
    {
    }
    virtual bool next(std::pair<KeyType, ValueType>& kvp)
    {
        return false;
    }
    // use this interface, you should not modify the content of value returned outside.
    virtual bool get(KeyType& key, const ValueType*& value)
    {
        return false;
    }
    void move()
    {
    }
};


template <class K, class V, class AMType>
class BTTermEnum : public TermEnum<K, V>
{
    
public:
    typedef K KeyType;
    typedef V ValueType;
    //typedef std::map<KeyType, ValueType> AMType;
    typedef typename AMType::iterator iterator;
    
    
    BTTermEnum(AMType& am)
    :it_(am.begin()), it_end_(am.end())
    {
    }
    
    BTTermEnum(AMType& am, const KeyType& key)
    :it_(am.lower_bound(key)), it_end_(am.end())
    {
    }

    bool next(std::pair<KeyType, ValueType>& kvp)
    {
#ifdef TE_DEBUG
//         std::cout<<"BTTermEnum next"<<std::endl;
#endif
        if(it_==it_end_) return false;
        else
        {
            kvp = *it_;
            kvp.second.sort();
            //std::cerr<<"BT in-memory"<<std::endl;
            ++it_;
#ifdef TE_DEBUG
//             std::cout<<"BTTermEnum key:"<<kvp.first<<", value: "<<kvp.second<<std::endl;
#endif
            return true;
        }
    }
  
  
private:
    iterator it_;
    iterator it_end_;
};

template <class K, class V, class AMType>
class PreLoadTermEnum : public TermEnum<K, V>
{
public:
    typedef K KeyType;
    typedef V ValueType;
    typedef typename AMType::iterator iterator;
    
    PreLoadTermEnum(AMType& am)
    :it_(am.begin()), it_end_(am.end())
    {
    }
    
    PreLoadTermEnum(AMType& am, const KeyType& key)
    :it_(am.lower_bound(key)), it_end_(am.end())
    {
    }

    bool get(KeyType& key, const ValueType*& value)
    {
        if(it_==it_end_) return false;
        else
        {
            key = it_->first;
            value = &(it_->second);
            return true;
        }
    }

    void move()
    {
        if(it_==it_end_) return;
        ++it_;
    }

    bool next(std::pair<KeyType, ValueType>& kvp)
    {
        if(it_==it_end_) return false;
        else
        {
            kvp = *it_;
            ++it_;
            return true;
        }
    }

private:
    iterator it_;
    iterator it_end_;
};

template <class AmType>
class AMTermEnum : public TermEnum<typename AmType::key_type, typename AmType::value_type>
{
    
public:
    typedef AmType AMType;
    typedef izenelib::am::AMIterator<AMType> iterator;
    typedef typename AMType::key_type KeyType;
    typedef typename AMType::value_type ValueType;
    
    AMTermEnum(AMType& am)
    :it_(am), it_end_()
    {
#ifdef TE_DEBUG
//         std::cout<<"AMTermEnum constructor1 "<<am.size()<<std::endl;
//         std::cout<<"is end : "<<(int)(it_==it_end_)<<std::endl;
#endif
    }
    
    AMTermEnum(AMType& am, const KeyType& key)
    :it_(am, key), it_end_()
    {
    }
    
    bool get(KeyType& key, const ValueType*& value)
    {
#ifdef TE_DEBUG
//         std::cout<<"AMTermEnum next"<<std::endl;
#endif
        if(it_==it_end_) return false;
        else
        {
            key = it_->first;
            value = &(it_->second);
#ifdef TE_DEBUG
//             std::cout<<"AMTermEnum key:"<<kvp.first<<std::endl;
#endif
            return true;
        }
    }
    void move()
    {
        if(it_==it_end_) return;
        ++it_;
    }

    bool next(std::pair<KeyType, ValueType>& kvp)
    {
#ifdef TE_DEBUG
//         std::cout<<"AMTermEnum next"<<std::endl;
#endif
        if(it_==it_end_) return false;
        else
        {
            kvp.first = it_->first;
            kvp.second = it_->second;
            ++it_;
#ifdef TE_DEBUG
//             std::cout<<"AMTermEnum key:"<<kvp.first<<std::endl;
#endif
            return true;
        }
    }
  
  
private:
    iterator it_;
    iterator it_end_;
};


// template <class KeyType, class ValueType1, class AMType2>
// class TwoWayTermEnum : public TermEnum<KeyType, typename AMType2::ValueType>
// {
//     typedef BTTermEnum<KeyType, ValueType1> EnumType1;
//     typedef AMTermEnum<AMType2> EnumType2;
//     typedef typename EnumType1::AMType AMType1;
//     typedef typename AMType2::ValueType ValueType;
//     typedef std::pair<KeyType, ValueType1> DataType1;
//     typedef std::pair<KeyType, ValueType> DataType;
//     typedef boost::function<void (const ValueType1&, ValueType&) > FuncType;
//     
//     public:
//         TwoWayTermEnum(AMType1& am1, AMType2& am2, const FuncType& func)
//         : enum1_(am1), enum2_(am2), func_(func)
//         {
//         }
//         
//         TwoWayTermEnum(AMType1& am1, AMType2& am2, const KeyType& key, const FuncType& func)
//         : enum1_(am1, key), enum2_(am2, key), func_(func)
//         {
//         }
//         
//         
//         bool next(DataType& kvp)
//         {
// #ifdef TE_DEBUG
//             std::cout<<"[TE] next ";
// #endif
// //             boost::optional<DataType1> data1;
// //             boost::optional<DataType2> data2;
//             if(!data1_)
//             {
//                 DataType1 rdata1;
//                 if(enum1_.next(rdata1))
//                 {
//                     data1_ = rdata1;
// #ifdef TE_DEBUG
//                     std::cout<<"get key1 : "<<data1_.get().first<<",";
// #endif
//                 }
//                 else
//                 {
//                     
// #ifdef TE_DEBUG
//                     std::cout<<"not get key1,";
// #endif
//                 }
//             }
//             else
//             {
// #ifdef TE_DEBUG
//                 std::cout<<"exist key1 : "<<data1_.get().first<<",";
// #endif                
//             }
//             if(!data2_)
//             {
//                 DataType2 rdata2;
//                 if(enum2_.next(rdata2))
//                 {
//                     data2_ = rdata2;
// #ifdef TE_DEBUG
//                     std::cout<<"get key2 : "<<data2_.get().first<<",";
// #endif
//                 }
//                 else
//                 {
// #ifdef TE_DEBUG
//                     std::cout<<"not get key2,";
// #endif
//                 }
//             }
//             else
//             {
// #ifdef TE_DEBUG
//                 std::cout<<"exist key2 : "<<data2_.get().first<<",";
// #endif                
//             }
// #ifdef TE_DEBUG
//             std::cout<<std::endl;
// #endif             
//             
//             
//             KeyType key;
//             bool select[2];
//             select[0] = false;
//             select[1] = false;
//             if(!data1_ && data2_)
//             {
//                 select[1] = true;
//                 key = data2_.get().first;
//             }
//             else if(data1_ && !data2_)
//             {
//                 select[0] = true;
//                 key = data1_.get().first;
//             }
//             else if(data1_ && data2_)
//             {
//                 KeyType& key1 = data1_.get().first;
//                 KeyType& key2 = data2_.get().first;
//                 int comp = Compare<KeyType>::compare(key1, key2);
//                 if(comp==0)
//                 {
//                     select[0] = true;
//                     select[1] = true;
//                     key = data1_.get().first;
//                 }
//                 else if(comp<0)
//                 {
//                     select[0] = true;
//                     key = data1_.get().first;
//                 }
//                 else
//                 {
//                     select[1] = true;
//                     key = data2_.get().first;
//                 }
//             }
//             
//             if(!select[0] && !select[1] ) return false;
//             ValueType1 value1;
//             if(select[0])
//             {
//                 value1 = data1_.get().second;
//                 data1_.reset();
//             }
//             if(select[1])
//             {
//                 kvp.second = data2_.get().second;
//                 data2_.reset();
//             }
//             kvp.first = key;
//             func_(value1, kvp.second);
//             return true;
//             
//         }
//         
//     private:
//         EnumType1 enum1_;
//         EnumType2 enum2_;
//         FuncType func_;
//         boost::optional<DataType1> data1_;
//         boost::optional<DataType2> data2_;
// };


template <class KeyType, class ValueType1, class AMType1, class AMType2, class EnumType2, class ValueType>
class TwoWayTermEnumBase : public TermEnum<KeyType, ValueType>
{
    typedef BTTermEnum<KeyType, ValueType1, AMType1> EnumType1;
    //typedef AMTermEnum<AMType2> EnumType2;
    //typedef typename EnumType1::AMType AMType1;
    typedef typename EnumType2::ValueType ValueType2;
    typedef std::pair<KeyType, ValueType1> DataType1;
    typedef std::pair<KeyType, ValueType2> DataType2;
    typedef std::pair<KeyType, ValueType> DataType;
    
public:
    typedef boost::function<void (const ValueType1&,const ValueType2&, ValueType&) > FuncType;

        TwoWayTermEnumBase(AMType1& am1, AMType2& am2, const FuncType& func)
        : enum1_(am1), enum2_(am2), func_(func)
        {
        }
        
        TwoWayTermEnumBase(AMType1& am1, AMType2& am2, const KeyType& key, const FuncType& func)
        : enum1_(am1, key), enum2_(am2, key), func_(func)
        {
        }
        
        bool next(DataType& kvp)
        {
            //izenelib::util::ClockTimer timer;
            //timer.restart();
            //kvp.second.clear();
            //ValueType().swap(kvp.second);
#ifdef TE_DEBUG
            std::cout<<"[TE] next ";
#endif
//             boost::optional<DataType1> data1;
//             boost::optional<DataType2> data2;
            if(!data1_)
            {
                DataType1 rdata1;
                if(enum1_.next(rdata1))
                {
                    data1_ = rdata1;
#ifdef TE_DEBUG
                    std::cout<<"get key1 : "<<data1_.get().first<<",";
#endif
                }
                else
                {
                    
#ifdef TE_DEBUG
                    std::cout<<"not get key1,";
#endif
                }
            }
            else
            {
#ifdef TE_DEBUG
                std::cout<<"exist key1 : "<<data1_.get().first<<",";
#endif                
            }
            //double t1 = timer.elapsed();
            //double t2 = t1;
            if(!data2_)
            {
                DataType2 rdata2;
                const ValueType2* tmp = NULL;
                if(enum2_.get(rdata2.first, tmp))
                {
                    //t2 = timer.elapsed();
                    data2_ = rdata2;
                    data2_.get().second = *tmp;
                    enum2_.move();
#ifdef TE_DEBUG
                    std::cout<<"get key2 : {"<<data2_.get().first<<"} :"<<data2_.get().second<<",";
#endif
                }
                else
                {
#ifdef TE_DEBUG
                    std::cout<<"not get key2,";
#endif
                }
            }
            else
            {
#ifdef TE_DEBUG
                std::cout<<"exist key2 : "<<data2_.get().first<<",";
#endif                
            }
#ifdef TE_DEBUG
            std::cout<<std::endl;
#endif             
            
            
            //double t3 = timer.elapsed();
            KeyType key;
            bool select[2];
            select[0] = false;
            select[1] = false;
            if(!data1_ && data2_)
            {
                select[1] = true;
                key = data2_.get().first;
            }
            else if(data1_ && !data2_)
            {
                select[0] = true;
                key = data1_.get().first;
            }
            else if(data1_ && data2_)
            {
                KeyType& key1 = data1_.get().first;
                KeyType& key2 = data2_.get().first;
                int comp = Compare<KeyType>::compare(key1, key2);
                if(comp==0)
                {
                    select[0] = true;
                    select[1] = true;
                    key = data1_.get().first;
                }
                else if(comp<0)
                {
                    select[0] = true;
                    key = data1_.get().first;
                }
                else
                {
                    select[1] = true;
                    key = data2_.get().first;
                }
            }
            
            if(!select[0] && !select[1] ) return false;
            ValueType1 value1;
            static const ValueType2 emptyvalue;
            const ValueType2* value2 = &emptyvalue;
            if(select[0])
            {
                value1 = data1_.get().second;
                data1_.reset();
            }
            if(select[1])
            {
                value2 = &data2_.get().second;
            }
            kvp.first = key;

            //double t4 = timer.elapsed();
            //if (t4 > 0.04)
            //    std::cerr << "next cost: " << t1 << "," << t2 << "," << t3 << "," << t4 << std::endl;
            func_(value1, *value2, kvp.second);

            //double t5 = timer.elapsed();
            //if (t5 > 0.08)
            //    std::cerr << "combine cost: " << t5 << std::endl;

            if(select[1])
                data2_.reset();
            return true;
            
        }
        
    private:
        EnumType1 enum1_;
        EnumType2 enum2_;
        FuncType func_;
        boost::optional<DataType1> data1_;
        boost::optional<DataType2> data2_;
};

template <class KeyType, class ValueType1, class AMType1, class AMType2, class ValueType>
class TwoWayTermEnum: public TwoWayTermEnumBase<KeyType, ValueType1, AMType1, AMType2, AMTermEnum<AMType2>, ValueType> 
{
    typedef TwoWayTermEnumBase<KeyType, ValueType1, AMType1, AMType2, AMTermEnum<AMType2>, ValueType> BaseType;
public:
    TwoWayTermEnum(AMType1& am1, AMType2& am2, const typename BaseType::FuncType& func)
        : BaseType(am1, am2, func)
    {
    }

    TwoWayTermEnum(AMType1& am1, AMType2& am2, const KeyType& key, const typename BaseType::FuncType& func)
        : BaseType(am1, am2, key, func)
    {
    }

};

template <class KeyType, class ValueType1, class AMType1, class AMType2, class ValueType>
class TwoWayPreLoadTermEnum: public TwoWayTermEnumBase<KeyType, ValueType1, AMType1, AMType2, PreLoadTermEnum<KeyType, ValueType, AMType2>, ValueType>
{
    typedef TwoWayTermEnumBase<KeyType, ValueType1, AMType1, AMType2, PreLoadTermEnum<KeyType, ValueType, AMType2>, ValueType> BaseType;
public:
    TwoWayPreLoadTermEnum(AMType1& am1, AMType2& am2, const typename BaseType::FuncType& func)
        : BaseType(am1, am2, func)
    {
    }

    TwoWayPreLoadTermEnum(AMType1& am1, AMType2& am2, const KeyType& key, const typename BaseType::FuncType& func)
        : BaseType(am1, am2, key, func)
    {
    }

};


}
NS_IZENELIB_IR_END

#endif


