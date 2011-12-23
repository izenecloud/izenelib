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
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

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
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    }

};

//When ValueType is NullType, it is equivatent to unary DataType.
template<typename KeyType, typename ValueType=NullType>
class DataType
{
public:
    DataType()
    {
    }

    DataType(const KeyType& key, const ValueType& value)
        : key(key), value(value)
    {
    }

    int compare(const DataType& other) const
    {
        return _compare(other, static_cast<boost::is_arithmetic<KeyType>*>(0));
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & key;
        ar & value;
    }

    const KeyType& get_key() const
    {
        return key;
    }

    const ValueType& get_value() const
    {
        return value;
    }

    KeyType& get_key()
    {
        return key;
    }

    ValueType& get_value()
    {
        return value;
    }

private:
    int _compare(const DataType& other, const boost::mpl::true_*) const
    {
        return key-other.key;
    }

    int _compare(const DataType& other, const boost::mpl::false_*) const
    {
        BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
        return key.compare(other.key);
    }

public:
    KeyType key;
    ValueType value;
};

template<typename KeyType>
class DataType<KeyType, NullType>
{
public:
    DataType()
    {
    }

    DataType(const KeyType& key)
        : key(key)
    {
    }

    DataType(const KeyType& key, const NullType&)
        : key(key)
    {
    }

    int compare(const DataType& other) const
    {
        return _compare(other, static_cast<boost::is_arithmetic<KeyType>*>(0));
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & key;
    }

    const KeyType& get_key() const
    {
        return key;
    }

    const NullType& get_value() const
    {
        return value;
    }

    KeyType& get_key()
    {
        return key;
    }

    NullType& get_value()
    {
        return value;
    }

private:
    int _compare(const DataType& other, const boost::mpl::true_*) const
    {
        return key-other.key;
    }

    int _compare(const DataType& other, const boost::mpl::false_*) const
    {
        BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
        return key.compare(other.key);
    }

public:
    KeyType key;
    NullType value;
};

template <typename T>
struct has_compare_operator{
    enum { value = boost::is_arithmetic<T>::value };
};

#if !defined(WIN32) || defined(__MINGW32__)
template <>
struct has_compare_operator<int128_t>
{
    enum { value = 1 };
};

template <>
struct has_compare_operator<uint128_t>
{
    enum { value = 1 };
};
#endif

template <typename T1, typename T2>
struct has_compare_operator<std::pair<T1, T2> >
{
    enum { value = 1 };
};

template <typename T1, typename T2>
struct has_compare_operator<boost::tuple<T1, T2> >
{
    enum { value = 1 };
};

template <typename T1, typename T2, typename T3>
struct has_compare_operator<boost::tuple<T1, T2, T3> >
{
    enum { value = 1 };
};

template <typename T1, typename T2, typename T3, typename T4>
struct has_compare_operator<boost::tuple<T1, T2, T3, T4> >
{
    enum { value = 1 };
};

template<int>
struct indidator{};

template<class KeyType>
class CompareFunctor : public binary_function<KeyType, KeyType, int>
{
public:
    int operator()(const KeyType& key1, const KeyType& key2) const
    {
        return _compare(key1,
                        key2,
                        indidator<has_compare_operator<KeyType>::value>()
                        );
    }

private:
    int _compare(const KeyType& key1,
                 const KeyType& key2,
                 indidator<1>) const
    {
        if (key1 > key2) return 1;
        if (key1 < key2) return -1;
        return 0;
    }

    int _compare(const KeyType& key1,
                 const KeyType& key2,
                 indidator<0>) const
    {
        BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
        return key1.compare(key2);
    }
};

NS_IZENELIB_AM_END

#endif //End of AM_CONCEPT_DATATYPE_H
