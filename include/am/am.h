#ifndef IZENELIB_AM_H
#define IZENELIB_AM_H

#include <types.h>
#include <am/concept/DataType.h>
#include <am/concept/ThreadModel.h>
#include <memory>

NS_IZENELIB_AM_BEGIN

template<typename KeyType, typename ValueType,
         typename LockType=NullLock, typename Alloc=std::allocator<DataType<KeyType,ValueType> > >
class AccessMethod
{
public:
    virtual bool insert(const KeyType& key, const ValueType& value);

    virtual bool insert(const DataType<KeyType,ValueType>& data) = 0;

    virtual bool update(const KeyType& key, const ValueType& value);

    virtual bool update(const DataType<KeyType,ValueType>& data) = 0;

    virtual ValueType* find(const KeyType& key) = 0;

    virtual bool del(const KeyType& key) = 0;
};



template<typename KeyType, typename ValueType,
         typename LockType, typename Alloc>
  bool AccessMethod<KeyType, ValueType, LockType, Alloc>::insert(const KeyType& key, const ValueType& value)
{
    DataType<KeyType,ValueType> data(key,value);
    return insert(data);
}

template<typename KeyType, typename ValueType,
         typename LockType, typename Alloc >
bool AccessMethod<KeyType, ValueType, LockType, Alloc>::update(const KeyType& key, const ValueType& value)
{
    DataType<KeyType,ValueType> data(key,value);
    return update(data);
}


template<typename KeyType, typename LockType=NullLock,typename Alloc=std::allocator<DataType<KeyType> > >
class UnaryAccessMethod
{
public:
    virtual bool insert(const KeyType& key);

    virtual bool insert(const DataType<KeyType>& data) = 0;

    virtual bool update(const KeyType& key);

    virtual bool update(const DataType<KeyType>& data) = 0;

    virtual KeyType* find(const KeyType& key) = 0;

    virtual bool del(const KeyType& key) = 0;
};

template<typename KeyType, typename LockType,typename Alloc >
bool UnaryAccessMethod<KeyType, LockType, Alloc>::insert(const KeyType& key)
{
    DataType<KeyType> data(key);
    return insert(data);
}

template<typename KeyType, typename LockType,typename Alloc >
bool UnaryAccessMethod<KeyType, LockType, Alloc>::update(const KeyType& key)
{
    DataType<KeyType> data(key);
    return update(data);
}


NS_IZENELIB_AM_END

#endif //End of IZENELIB_AM_H
