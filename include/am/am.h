#ifndef IZENELIB_AM_H
#define IZENELIB_AM_H

#include <types.h>
#include <am/concept/DataType.h>
#include <util/ThreadModel.h>
#include <memory>
#include <string.h>
#include <cstdio>

NS_IZENELIB_AM_BEGIN

using namespace izenelib::util;

template<typename KeyType, typename ValueType, typename LockType = NullLock,
         typename Alloc = std::allocator<DataType<KeyType, ValueType> > >
class AccessMethod
{
public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;

    virtual bool insert(const KeyType& key, const ValueType& value)
    {
        DataType<KeyType,ValueType> data(key, value);
        return insert(data);
    }

    virtual bool insert(const DataType<KeyType,ValueType>& data) = 0;

    virtual bool get(const KeyType&key, ValueType& value) = 0;

    virtual bool update(const KeyType& key, const ValueType& value)
    {
        DataType<KeyType,ValueType> data(key, value);
        return update(data);
    }

    virtual bool update(const DataType<KeyType,ValueType>& data)
    {
        return false;
    }

    virtual ValueType* find(const KeyType& key)
    {
        return NULL;
    }
    ;

    virtual bool del(const KeyType& key)
    {
        return false;
    }

    virtual ~AccessMethod()
    {
    }
    ;
};

template <typename KeyType, typename LockType, typename Alloc>
class AccessMethod<KeyType, NullType, LockType, Alloc>
{
public:
    typedef KeyType key_type;
    typedef NullType value_type;
    typedef DataType<KeyType> data_type;

    virtual bool insert(const KeyType& key, const NullType val=NullType())
    {
        DataType<KeyType> data(key);
        return insert(data);
    }

    virtual bool insert(const DataType<KeyType>& data) = 0;

    virtual bool get(const KeyType&key, value_type& value) = 0;

    virtual bool update(const KeyType& key)
    {
        DataType<KeyType> data(key);
        return update(data);
    }

    virtual bool update(const DataType<KeyType>& data)
    {
        return false;
    }

    virtual bool del(const KeyType& key)
    {
        return false;
    }

    virtual value_type* find(const KeyType& key)
    {
        return NULL;
    }

    virtual ~AccessMethod()
    {
    }
};

template <typename KeyType, typename ValueType, typename AM, bool open = false>
class AMOBJ
{
    AM am_;
public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;

    AMOBJ() :
        am_()
    {
    }
    AM& getInstance()
    {
        return am_;
    }
    bool insert(const KeyType& key, const ValueType& value)
    {
        return am_.insert(key, value);
    }
    bool insert(const KeyType& value)
    {
        return am_.insert(value);
    }
    bool del(const KeyType& key)
    {
        return am_.del(key);
    }
    bool get(const KeyType& key, ValueType& value)
    {
        return am_.get(key, value);
    }
    bool update(const KeyType& key, const ValueType& value)
    {
        return am_.update(key, value);
    }
    ValueType* find(const KeyType& key)
    {
        return am_.find(key);
    }

//  SDBCursor get_first_locn()
//  {
//      return am_.get_first_locn();
//  }
//
//  bool seq(SDBCursor& locn)
//  {
//      typedef typename AM::SDBCursor SDBCursor;
//      return am_.seq(locn);
//  }
//  bool get(const SDBCursor&locn,  KeyType& key, ValueType& value)
//  {
//      return am_.get(locn, key, value);
//  }

    int num_items()
    {
        return am_.num_items();
    }
};

template<typename KeyType, typename ValueType, typename AM>
class AMOBJ<KeyType, ValueType, AM, true>
{
    AM am_;
    string getStr()
    {
        char p[1000];
        sprintf(p, "%s_%s_%s.dat", typeid(AM).name(), typeid(KeyType).name(), typeid(ValueType).name());
        assert(strlen(p) < 1000);
        return p;
    }
public:
    typedef KeyType key_type;
    typedef ValueType value_type;
    typedef DataType<KeyType, ValueType> data_type;
    typedef typename AM::SDBCursor SDBCursor;

    AMOBJ()
        : am_(getStr())
    {
        am_.open();
    }
    ~AMOBJ()
    {
        am_.close();
    }
    AM& getInstance()
    {
        return am_;
    }
    bool insert(const KeyType& key, const ValueType& value)
    {
        return am_.insert(key, value);
    }
    bool insert(const KeyType& value)
    {
        return am_.insert(value);
    }
    bool del(const KeyType& key)
    {
        return am_.del(key);
    }
    bool get(const KeyType& key, ValueType& value)
    {
        return am_.get(key, value);
    }
    bool get(const SDBCursor&locn,  KeyType& key, ValueType& value)
    {
        return am_.get(locn, key, value);
    }

    bool update(const KeyType& key, const ValueType& value)
    {
        return am_.update(key, value);
    }
    ValueType* find(const KeyType& key)
    {
        return am_.find(key);
    }

    SDBCursor get_first_locn()
    {
        return am_.get_first_locn();
    }

    bool seq(SDBCursor& locn)
    {
        return am_.seq(locn);
    }

    int num_items()
    {
        return am_.num_items();
    }
};

NS_IZENELIB_AM_END

#endif //End of IZENELIB_AM_H
