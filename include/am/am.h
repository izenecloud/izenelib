#ifndef IZENELIB_AM_H
#define IZENELIB_AM_H

#include <types.h>
#include <am/concept/DataType.h>
#include <util/ThreadModel.h>
#include <memory>

NS_IZENELIB_AM_BEGIN

using namespace izenelib::util;

template<typename KeyType, typename ValueType,
typename LockType=NullLock, typename Alloc=std::allocator<DataType<KeyType,ValueType> > >
class AccessMethod
{
public:
	virtual bool insert(const KeyType& key, const ValueType& value){
		DataType<KeyType,ValueType> data(key, value);
    	return insert(data);
	}

	virtual bool insert(const DataType<KeyType,ValueType>& data) = 0;

	virtual bool update(const KeyType& key, const ValueType& value){
		DataType<KeyType,ValueType> data(key, value);
    	return update(data);
	}

	virtual bool update(const DataType<KeyType,ValueType>& data){return false;}

	virtual ValueType* find(const KeyType& key) = 0;

	virtual bool del(const KeyType& key){return false;}

	virtual ~AccessMethod() {};
};

template<typename KeyType, typename LockType, typename Alloc >
class AccessMethod<KeyType, NullType, LockType, Alloc>
{
public:
	virtual bool insert(const KeyType& key, const NullType val=NullType()){
		DataType<KeyType> data(key);
    	return insert(data);
	}

	virtual bool insert(const DataType<KeyType>& data) = 0;

	virtual bool update(const KeyType& key){
		DataType<KeyType> data(key);
    	return update(data);
	}

	virtual bool update(const DataType<KeyType>& data) {return false;}


	virtual bool del(const KeyType& key){return false;}

	virtual ~AccessMethod() {};
};



template<typename KeyType, typename ValueType, typename AM, bool open=false> class AMOBJ {
	AM am_;
public:
	AMOBJ() :
		am_() {
	}
	AM& getInstance() {
		return am_;
	}
	bool insert(const KeyType& key, const ValueType& value) {
		return am_.insert(key, value);
	}
	bool insert(const KeyType& value) {
		return am_.insert(value);
	}
	bool del(const KeyType& key) {
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
	ValueType* find(const KeyType& key) {
		return am_.find(key);
	}
	int num_items() {
		return am_.num_items();
	}	
};

template<typename KeyType, typename ValueType, typename AM> class AMOBJ<
		KeyType, ValueType, AM, true> {
	AM am_;
	string getStr() {
		char p[1000];
		sprintf(p, "%s_%s_%s.dat", typeid(AM).name(), typeid(KeyType).name(), typeid(ValueType).name());
		assert(strlen(p) < 1000 );
		return p;
	}
public:
	AMOBJ() :
		am_( getStr() ) {
		am_.open();
	}
	~AMOBJ() {
		am_.close();
	}
	AM& getInstance() {
		return am_;
	}
	bool insert(const KeyType& key, const ValueType& value) {
		return am_.insert(key, value);
	}
	bool insert(const KeyType& value) {
		return am_.insert(value);
	}
	bool del(const KeyType& key) {
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
	ValueType* find(const KeyType& key) {
		return am_.find(key);
	}
	int num_items() {
		return am_.num_items();
	}
};


NS_IZENELIB_AM_END

#endif //End of IZENELIB_AM_H
