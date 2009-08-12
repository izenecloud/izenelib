/**
 * @file	HashFunctionTraits.h
 * @brief	Select suitable hash functions for each type of NameID
 * @author	Wei
 * @date 2009-08-12
 */

#ifndef _HASH_FUNCTION_TRAITS_H_
#define _HASH_FUNCTION_TRAITS_H_

template <typename NameID>
class HashFunctionTraits
{
  template<typename NameString>
  static NameID hash(const NameString&)
  {
    NameID value;
    return value;
  }
};

template <>
class HashFunctionTraits<int32_t>
{
  template<typename NameString>
  static int32_t hash(const NameString& key)
  {
    return (int32_t)HashFunction<NameString>::generateHash32(key);
  }
};

template <>
class HashFunctionTraits<uint32_t>
{
  template<typename NameString>
  static uint32_t hash(const NameString& key)
  {
    return HashFunction<NameString>::generateHash32(key);
  }
};

template <>
class HashFunctionTraits<int64_t>
{
  template<typename NameString>
  static int64_t hash(const NameString& key)
  {
    return (int64_t)HashFunction<NameString>::generateHash64(key);
  }
};

template <>
class HashFunctionTraits<uint64_t>
{
  template<typename NameString>
  static uint64_t hash(const NameString& key)
  {
    return HashFunction<NameString>::generateHash64(key);
  }
};
#endif
