#ifndef AM_CONCEPT_DATATYPE_H
#define AM_CONCEPT_DATATYPE_H

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <functional>
#include <iostream>
using namespace std;

#include <types.h>

NS_IZENELIB_AM_BEGIN

template<typename KeyType>
struct KeyTypeConcept
{
	void constraints()
	{
		KeyType key1;
		KeyType key2;
		key1.compare(key2);
	}
};

struct NullType {
	//empty serialize function.
	template<class Archive> void serialize(Archive& ar,
			const unsigned int version) {		
	}

};

//When ValueType is NullType, it is equivatent to unary DataType.
template<typename KeyType, typename ValueType=NullType> class DataType {
public:
	DataType() {
	}
	DataType(const KeyType& key, const ValueType& value) :
		key(key), value(value) {
	}

	int compare(const DataType& other) const {
		return _compare(other, static_cast<boost::is_arithmetic<KeyType>*>(0));
	}

	template<class Archive> void serialize(Archive& ar,
			const unsigned int version) {
		ar & key;
		ar & value;
	}

	const KeyType& get_key() const {
		return key;
	}

	const ValueType& get_value() const {
		return value;
	}
private:
	int _compare(const DataType& other, const boost::mpl::true_*) const {
		return key-other.key;
	}

	int _compare(const DataType& other, const boost::mpl::false_*) const {
		BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
		return key.compare(other.key);
	}

public:
	KeyType key;
	ValueType value;
};

template<typename KeyType> class DataType<KeyType, NullType> {

public:
	DataType() {
	}
	DataType(const KeyType& key) :
		key(key) {
	}
	DataType(const KeyType& key, const NullType&) :
		key(key) {
	}

	int compare(const DataType& other) const {
		return _compare(other, static_cast<boost::is_arithmetic<KeyType>*>(0));
	}

	template<class Archive> void serialize(Archive& ar,
			const unsigned int version) {
		ar & key;
	}

	const KeyType& get_key() const {
		return key;
	}
	const NullType& get_value() const {
		return value;
	}

private:
	int _compare(const DataType& other, const boost::mpl::true_*) const {
		return key-other.key;
	}

	int _compare(const DataType& other, const boost::mpl::false_*) const {
		BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
		return key.compare(other.key);
	}

public:
	KeyType key;
	NullType value;
};

template<class KeyType> class CompareFunctor :
	public binary_function<KeyType, KeyType, int>

{
public:
	int operator()(const KeyType& key1, const KeyType& key2) const {
		return _compare(key1, key2,
				static_cast<boost::is_arithmetic<KeyType>*>(0));
	}
private:
	int _compare(const KeyType& key1, const KeyType& key2,
			const boost::mpl::true_*) const {	
		if(key1 > key2) return 1;
		else if(key1 <key2 )return -1;		
		else return 0;
	}

	int _compare(const KeyType& key1, const KeyType& key2,
			const boost::mpl::false_*) const {
		BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
		return key1.compare(key2);
	}

};

NS_IZENELIB_AM_END

#endif //End of AM_CONCEPT_DATATYPE_H
