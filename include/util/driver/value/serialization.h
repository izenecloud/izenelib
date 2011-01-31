#ifndef IZENELIB_DRIVER_VALUE_SERIALIZATION_H
#define IZENELIB_DRIVER_VALUE_SERIALIZATION_H
/**
 * @file izenelib/driver/value/serialization.h
 * @author Ian Yang
 * @date Created <2010-06-08 15:09:50>
 */

#include "Value.h"

#include <boost/variant.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/serialization/variant.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

namespace izenelib {
namespace driver {
//////////////////////////////////////////////////
// Febird Serialization

template<class DataIO>
struct DataIO_ValueSaveVisitor : boost::static_visitor<>
{
    DataIO_ValueSaveVisitor(DataIO& io)
    : io_(io)
    {}

    template<class T>
    void operator()(const T& value) const
    {
        io_ & value;
    }

private:
    DataIO& io_;
};

template<class DataIO>
void DataIO_saveObject(DataIO& ar, const Value& v)
{
    int which = v.type();
    ar & which;
    boost::apply_visitor(DataIO_ValueSaveVisitor<DataIO>(ar), v.variant());
}

template<class Types, class Enable = void>
struct DataIO_ValueLoader
{
    template<typename DataIO>
    static void load(DataIO& ar, int which, Value& v)
    {
        if (which == 0) {
            typedef typename boost::mpl::front<Types>::type head_type;
            head_type value;
            ar & value;
            v = value;
            return;
        }
        typedef typename boost::mpl::pop_front<Types>::type type;
        DataIO_ValueLoader<type>::load(ar, which - 1, v);
    }
};

template<class Types>
struct DataIO_ValueLoader<
    Types,
    typename boost::enable_if<boost::mpl::empty<Types> >::type
>
{
    template<typename DataIO>
    static void load(DataIO&, int, Value&)
    {}
};

template<class DataIO>
void DataIO_loadObject(DataIO& ar, Value& v)
{
    int which;
    ar & which;

    if (which >= boost::mpl::size<Value::type_list>::value)
    {
        throw std::runtime_error(
            "Failed to load data"
        );
    }

    DataIO_ValueLoader<Value::type_list>::load(ar, which, v);
}

}} // namespace izenelib::driver

//////////////////////////////////////////////////
// Boost Serialization

namespace boost {
namespace serialization {

template<class Archive>
inline void serialize(Archive & ar,
               ::izenelib::driver::Value& v,
               const unsigned int version)
{
    ar & v.variant();
}

template<class Archive>
inline void serialize(Archive & ar,
                      ::izenelib::driver::NullTypeTag& v,
                      const unsigned int version)
{}

}} // namespace boost::serialization

#endif // IZENELIB_DRIVER_VALUE_SERIALIZATION_H
