#ifndef FEBIRD_BOOST_VARIANT_H
#define FEBIRD_BOOST_VARIANT_H

#include <boost/variant.hpp>

#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/empty.hpp>

#include <3rdparty/febird/io/DataIO.h>
#include <3rdparty/febird/io/StreamBuffer.h>
#include <3rdparty/febird/io/FileStream.h>


namespace febird {

template<class DataIO>
struct variant_save_visitor : boost::static_visitor<> {
    variant_save_visitor(DataIO& ar) :
        ar_(ar)
    {}
    template<class T>
    void operator()(T const & value) const
    {
        ar_ & value;
    }
private:
    DataIO & ar_;
};

template<class DataIO, BOOST_VARIANT_ENUM_PARAMS(/* typename */ class T)>
void DataIO_saveObject(DataIO& dio, const boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& v)
{
    int which = v.which();
    dio & which;
    typedef BOOST_DEDUCED_TYPENAME  boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types types;
    variant_save_visitor<DataIO> visitor(dio);
    v.apply_visitor(visitor);
}

template<class S>
struct variant_impl {

    struct load_null {
        template<class DataIO, class V>
        static void invoke(
            DataIO & /*ar*/,
            int /*which*/,
            V & /*v*/
        ){}
    };

    struct load_impl {
        template<class DataIO, class V>
        static void invoke(
            DataIO & ar,
            int which,
            V & v
        ){
            if(which == 0){
                // note: A non-intrusive implementation (such as this one)
                // necessary has to copy the value.  This wouldn't be necessary
                // with an implementation that de-serialized to the address of the
                // aligned storage included in the variant.
                typedef BOOST_DEDUCED_TYPENAME mpl::front<S>::type head_type;
                head_type value;
                ar & value;
                v = value;
                //ar.reset_object_address(& boost::get<head_type>(v), & value);
                return;
            }
            typedef BOOST_DEDUCED_TYPENAME mpl::pop_front<S>::type type;
            variant_impl<type>::load(ar, which - 1, v);
        }
    };

    template<class DataIO, class V>
    static void load(
        DataIO & ar,
        int which,
        V & v
    ){
        typedef BOOST_DEDUCED_TYPENAME mpl::eval_if<mpl::empty<S>,
            mpl::identity<load_null>,
            mpl::identity<load_impl>
        >::type typex;
        typex::invoke(ar, which, v);
    }

};


template<class DataIO, BOOST_VARIANT_ENUM_PARAMS(/* typename */ class T)>
void DataIO_loadObject(DataIO & dio, boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& v)
{
    int which;
    typedef BOOST_DEDUCED_TYPENAME boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types types;
    dio & which;
    if(which >=  mpl::size<types>::value)
        // this might happen if a type was removed from the list of variant types
        throw IOException("unsupported");
    variant_impl<types>::load(dio, which, v);
}


}

#endif
