
#ifndef IZENELIB_IR_TERMENUM_H_
#define IZENELIB_IR_TERMENUM_H_
#include <ir/index_manager/index/rtype/Compare.h>
#include <types.h>
#include <boost/function.hpp>
#include <3rdparty/am/stx/btree_map.h>

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
    
    
};

template <class K, class V>
class BTTermEnum : public TermEnum<K, V>
{
    
public:
    typedef K KeyType;
    typedef V ValueType;
    typedef stx::btree_map<KeyType, ValueType> AMType;
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

template <class KeyType, class ValueType1, class AMType2, class ValueType>
class TwoWayTermEnum : public TermEnum<KeyType, ValueType>
{
    typedef BTTermEnum<KeyType, ValueType1> EnumType1;
    typedef AMTermEnum<AMType2> EnumType2;
    typedef typename EnumType1::AMType AMType1;
    typedef typename EnumType2::ValueType ValueType2;
    typedef std::pair<KeyType, ValueType1> DataType1;
    typedef std::pair<KeyType, ValueType2> DataType2;
    typedef std::pair<KeyType, ValueType> DataType;
    typedef boost::function<void (const ValueType1&,const ValueType2&, ValueType&) > FuncType;
    
    public:
        TwoWayTermEnum(AMType1& am1, AMType2& am2, const FuncType& func)
        : enum1_(am1), enum2_(am2), func_(func)
        {
        }
        
        TwoWayTermEnum(AMType1& am1, AMType2& am2, const KeyType& key, const FuncType& func)
        : enum1_(am1, key), enum2_(am2, key), func_(func)
        {
        }
        
        
        bool next(DataType& kvp)
        {
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
            if(!data2_)
            {
                DataType2 rdata2;
                if(enum2_.next(rdata2))
                {
                    data2_ = rdata2;
#ifdef TE_DEBUG
                    std::cout<<"get key2 : "<<data2_.get().first<<",";
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
#ifdef TE_DEBUG
//                 std::cout<<"comp : "<<comp<<std::endl;
#endif
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
            ValueType2 value2;
            if(select[0])
            {
                value1 = data1_.get().second;
                data1_.reset();
            }
            if(select[1])
            {
                value2 = data2_.get().second;
                data2_.reset();
            }
            kvp.first = key;
            func_(value1, value2, kvp.second);
            return true;
            
        }
        
    private:
        EnumType1 enum1_;
        EnumType2 enum2_;
        FuncType func_;
        boost::optional<DataType1> data1_;
        boost::optional<DataType2> data2_;
};
}
NS_IZENELIB_IR_END

#endif


