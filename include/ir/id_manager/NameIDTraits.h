/**
 * @file	NameIDTraits.h
 * @brief	Select suitable parameters for each type of NameID, including:
 *              1. hash functions.
 *              2. minimum and maximum values.
 * @author	Wei Cao
 * @date 2009-08-12
 */

#ifndef _NAMEID_TRAITS_H_
#define _NAMEID_TRAITS_H_

#include <limits.h>
#include <types.h>
#include <stdint.h>

#include <util/hashFunction.h>

NS_IZENELIB_IR_BEGIN

namespace idmanager {

/**
  * Common case not implmented.
  * Four kinds of NameID are supported: int32, uint32, int64, uint64.
  */
template <typename NameID> class NameIDTraits;

template <>
class NameIDTraits<uint16_t>
{
public:
  enum{MinValue = 1, MaxValue = USHRT_MAX};

  template<typename NameString>
  static uint16_t hash(const typename NameString::value_type * buffer, const size_t length)
  {
    return (uint16_t)izenelib::util::HashFunction<NameString>::generateHash32((const char*)buffer,length*sizeof(typename NameString::value_type));
  }

  template<typename NameString>
  static uint16_t hash(const NameString& key)
  {
    return (uint16_t)izenelib::util::HashFunction<NameString>::generateHash32(key);
  }
};

template <>
class NameIDTraits<int32_t>
{
public:
  enum{MinValue = 1, MaxValue = INT_MAX };

  template<typename NameString>
  static int32_t hash(const typename NameString::value_type * buffer, const size_t length)
  {
    return (int32_t)izenelib::util::HashFunction<NameString>::generateHash32((const char*)buffer,length*sizeof(typename NameString::value_type));
  }

  template<typename NameString>
  static int32_t hash(const NameString& key)
  {
    return (int32_t)izenelib::util::HashFunction<NameString>::generateHash32(key);
  }
};

template <>
class NameIDTraits<uint32_t>
{
public:
  enum{MinValue = 1, MaxValue = UINT_MAX };

  template<typename NameString>
  static uint32_t hash(const typename NameString::value_type * buffer, const size_t length)
  {
    return izenelib::util::HashFunction<NameString>::generateHash32((const char*)buffer,length*sizeof(typename NameString::value_type));
  }

  template<typename NameString>
  static uint32_t hash(const NameString& key)
  {
    return izenelib::util::HashFunction<NameString>::generateHash32(key);
  }
};

template <>
class NameIDTraits<int64_t>
{
public:
  enum{MinValue = 1, MaxValue = LONG_MAX };

  template<typename NameString>
  static int64_t hash(const typename NameString::value_type * buffer, const size_t length)
  {
    return (int64_t)izenelib::util::HashFunction<NameString>::generateHash64((const char*)buffer,length*sizeof(typename NameString::value_type));
  }

  template<typename NameString>
  static int64_t hash(const NameString& key)
  {
    return (int64_t)izenelib::util::HashFunction<NameString>::generateHash64(key);
  }
};

template <>
class NameIDTraits<uint64_t>
{
public:
  enum{MinValue = 1, MaxValue = ULONG_MAX };

  template<typename NameString>
  static uint64_t hash(const typename NameString::value_type * buffer, const size_t length)
  {
    return izenelib::util::HashFunction<NameString>::generateHash64((const char*)buffer,length*sizeof(typename NameString::value_type));
  }


  template<typename NameString>
  static uint64_t hash(const NameString& key)
  {
    return izenelib::util::HashFunction<NameString>::generateHash64(key);
  }
};

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif
