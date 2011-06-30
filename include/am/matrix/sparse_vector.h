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

template <typename V, typename K = uint16_t, template <typename VV, typename KK = std::allocator<VV> > class Container = std::vector>
class SparseVector
{

  public:
    typedef Container<std::pair<K, V> > ValueType;
    typedef SparseVector<V, K, Container> SelfType;
    
    SelfType& operator+=(const SelfType& from)
    {
      std::vector<std::pair<K, V> > new_value(value.size()+from.value.size());
      K p = 0;
      for(uint32_t i=0;i<value.size();i++)
      {
        new_value[p] = value[i];
        ++p;
      }
      for(uint32_t i=0;i<from.value.size();i++)
      {
        new_value[p] = from.value[i];
        ++p;
      }

      std::sort(new_value.begin(), new_value.end());
      K key = 0;
      V value = 0;
      value.resize(0);
      for(uint32_t i=0;i<new_value.size();i++)
      {
        K current_key = new_value[i].first;
        if(current_key!=key && value!=0)
        {
          value.push_back(std::make_pair(key, value) );
          value = 0;
        }
        value += new_value[i].second;
        key = current_key;
      }
      if(value!=0)
      {
        value.push_back(std::make_pair(key, value) );
      }
      return *this;
    }
    
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
