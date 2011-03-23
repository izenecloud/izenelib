#ifndef IZENELIB_AM_SPARSEVECTOR_H_
#define IZENELIB_AM_SPARSEVECTOR_H_


#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <types.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/utility.hpp>//for std pair
NS_IZENELIB_AM_BEGIN

template <typename V, typename K = uint16_t, template <typename VV> class Container = std::vector>
class SparseVector
{

  public:
    typedef Container<std::pair<K, V> > ValueType;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & value;
    }
    ValueType value;
};

   
NS_IZENELIB_AM_END

#endif 
