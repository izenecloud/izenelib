#ifndef UTIL_BOOST_VARIANT_FEBIRD_H
#define UTIL_BOOST_VARIANT_FEBIRD_H
/**
 * @file util/BoostVariantFebird.h
 * @author Ian Yang
 * @date Created <2009-10-22 15:29:52>
 * @date Updated <2009-10-22 15:45:20>
 * @brief support serializing boost::varaint using febird
 */

#ifndef BOOST_VARIANT_UTIL_H
#define BOOST_VARIANT_UTIL_H

#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/empty.hpp>

#include <boost/variant.hpp>

namespace boost {

template<class DataIO>
struct FerbirdVariantSaveVisitor : boost::static_visitor<>
{
    FerbirdVariantSaveVisitor(DataIO& io)
    : io_(io)
    {}

    template<class T>
    void operator()(T const & value) const
    {
        io_ & value;
    }

private:
    DataIO& io_;
};

template<class DataIO, BOOST_VARIANT_ENUM_PARAMS(class T)>
void DataIO_saveObject(DataIO& io,
                       const boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& v)
{
    int which = v.which();
    io & which;
    FerbirdVariantSaveVisitor<DataIO> visitor(ar);
    v.apply_visitor(visitor);
}

template<class Types, class Enable = void>
struct FerbirdVariantLoader
{
    template<class DataIO, BOOST_VARIANT_ENUM_PARAMS(class T)>
    static void load(DataIO& ar,
                     int which,
                     boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& v)
    {
        if (which == 0) {
            typedef typename boost::mpl::front<Types>::type head_type;
            head_type value;
            ar & value;
            v = value;
            return;
        }
        typedef typename boost::mpl::pop_front<Types>::type type;
        DataIO_propertyValueLoader<type>::load(ar, which - 1, v);
    }
};

/**
 * Program never goes here, add it to let the code pass the compiler check.
 */
template<class Types>
struct FerbirdVariantLoader
<
    Types,
    typename boost::enable_if<boost::mpl::empty<Types> >::type
>
{
    template<typename DataIO, typename T>
    static void load(DataIO&, int, T&)
    {}
};

template<class DataIO, BOOST_VARIANT_ENUM_PARAMS(class T)>
void DataIO_loadObject(DataIO& io,
                       boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& v)
{
    int which = 0;
    ar & which;

    typedef typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types types;
    if (which >= boost::mpl::size<types>::value)
    {
        throw std::runtime_error(
            "Failed to load data"
        );
    }

    FerbirdVariantLoader<types>::load(io, which, v);
}


} // namespace boost

#endif // UTIL_BOOST_VARIANT_FEBIRD_H
