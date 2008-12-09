#ifndef AM_CONCEPT_DATATYPE_H
#define AM_CONCEPT_DATATYPE_H

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>

#include <types.h>

NS_IZENELIB_AM_BEGIN

template<typename KeyType, typename ValueType=void, typename Disable=void>
class DataType
{
public:
    DataType(const KeyType& key)
    :key(key)
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

private:
    int _compare(const DataType& other, const boost::mpl::true_*) const
    {
        return key-other.key;
    }

    int _compare(const DataType& other, const boost::mpl::false_*) const
    {
        return key.compare(other.key);
    }

public:
    KeyType key;
};



template<typename KeyType, typename ValueType>
class DataType<KeyType, ValueType, typename boost::disable_if<
                                       boost::is_void<ValueType> >::type>
{
public:
    DataType(const KeyType& key, const ValueType& value)
            :key(key),value(value)
    {}

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

private:
    int _compare(const DataType& other, const boost::mpl::true_*) const
    {
        return key-other.key;
    }

    int _compare(const DataType& other, const boost::mpl::false_*) const
    {
        return key.compare(other.key);
    }
public:
    KeyType key;

    ValueType value;
};



NS_IZENELIB_AM_END

#endif //End of AM_CONCEPT_DATATYPE_H
