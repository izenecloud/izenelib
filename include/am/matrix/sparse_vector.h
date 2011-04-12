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

template <typename V, typename K>
std::ostream& operator<<(std::ostream& stream, const SparseVector<V,K>& vec)
{
  for(uint32_t i=0;i<vec.value.size();i++)
  {
    if(i>0) stream<<" ";
    stream<<"["<<vec.value[i].first<<","<<vec.value[i].second<<"]";
  }
  return stream;
}

NS_IZENELIB_AM_END

#endif 
