/**
   @file str_str_table.hpp
   @author Kevin Hu
   @date 2010.01.08
*/
#ifndef STR_STR_TABLE_HPP
#define STR_STR_TABLE_HPP

#include <types.h>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <wiselib/ustring/UString.h>
#include <util/hashFunction.h>
#include <string>
#include "id_str_table.hpp"
#include "id_table.hpp"
#include <boost/filesystem/operations.hpp>

NS_IZENELIB_IR_BEGIN
/**
 * @class StrStrTable
 * @brief A string to string hash table.
 *
 **/

template<
  wiselib::UString::EncodingType ENCODE_TYPE = wiselib::UString::UTF_8,
  uint8_t ENTRY_POW = 17
  >
class StrStrTable
{
  typedef IdStringTable<uint32_t, uint32_t, ENTRY_POW> id_str_t;
  typedef IdTable<uint32_t, ENTRY_POW> id_id_t;
  
protected:
  //id_str_t id1_str_table_;
  id_str_t id2_str_table_;
  id_id_t  id_table_;
  std::string filenm_;

  /** 
     * @brief hash 32 bit int
     * 
     * @param key source
     * 
     * @return hashed value
     */
    inline uint32_t hash32shift(uint32_t key) {
        key = ~key + (key << 15); // key = (key << 15) - key - 1;
        key = key ^ (key >> 12);
        key = key + (key << 2);
        key = key ^ (key >> 4);
        key = key * 2057; // key = (key + (key << 3)) + (key << 11);
        key = key ^ (key >> 16);
        return key;
    }

public:
  /**
     @brief a constructor
  */
  StrStrTable(const char* filenm)
    :filenm_(filenm)
  {
  }
    
  /**
     @brief a destructor
  */
  ~StrStrTable()
  {
  }

  uint32_t num_items() const
  {
    return id_table_.num_items();
  }

  bool insert(const std::string& str1, const std::string& str2)
  {
    if (str1.length() == 0 || str2.length() == 0)
      return false;
    
    uint32_t id1 = izenelib::util::HashFunction<std::string>::generateHash32(str1);
    uint32_t id2 = hash32shift(id1);

    uint32_t id1_2 = id_table_.find(id1);
    
    if (id1_2 == (uint32_t)-1)
    {
      IASSERT(id_table_.insert(id1, id2));
      IASSERT(id2_str_table_.insert(id2, str2.length(), str2.c_str()));
      return true;
    }

    IASSERT(id1_2 == id2);
    uint32_t len = 0;
    const char* tmp = id2_str_table_.find(id1_2, len);
    IASSERT(tmp != NULL);
    std::string str1_2(tmp, len);

    //original includes current one. 
    if (str1_2.find(str2)!=std::size_t(-1))
      return false;
    
    //current one does't include original one. 
    if (str2.find(str1_2)==std::size_t(-1))
    {
      str1_2 += str2;
    }
    else
    {
      str1_2 = str2;
    }
    

    IASSERT(id_table_.update(id1, id2));
    IASSERT(str1_2.length()>0);
    IASSERT(id2_str_table_.update(id2, str1_2.length(), str1_2.c_str()));

    return true;
  }

  std::string find(const std::string& str1)const
  {
    if (str1.length()==0)
      return std::string();
    
    uint32_t id1 = izenelib::util::HashFunction<std::string>::generateHash32(str1);
    uint32_t id1_2 = id_table_.find(id1);

    if (id1_2 == (uint32_t)-1)
      return std::string();

    uint32_t len;
    const char* tmp = id2_str_table_.find(id1_2, len);
    IASSERT(tmp != NULL);
    return std::string (tmp, len);
  }

  bool update(const std::string& str1, const std::string& str2)
  {
    if (str1.length()==0)
      return false;
    if (str2.length()==0)
      return remove(str1);
    
    uint32_t id1 = izenelib::util::HashFunction<std::string>::generateHash32(str1);

    uint32_t id2 = id_table_.find(id1);
    if (id2 == (uint32_t)-1)
      return false;
    
    id_table_.update(id1, id2);
    return id2_str_table_.update(id2, str2.length(), str2.c_str());
  }

  bool remove(const std::string& str1)
  {
    uint32_t id1 = izenelib::util::HashFunction<std::string>::generateHash32(str1);

    uint32_t id1_2 = id_table_.find(id1);
    if (id1_2 == (uint32_t)-1)
      return false;

    id2_str_table_.remove(id1_2);
    id_table_.remove(id1);
    
    return true;
  }
  
  void flush()const
  {
    boost::filesystem::remove(std::string(std::string("rm -f ")+std::string(filenm_+".over")).c_str());
        
    FILE* f = fopen(std::string(filenm_+".id2.str").c_str(), "w+");
    id2_str_table_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".id.id").c_str(), "w+");
    id_table_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".over").c_str(), "w+");
    fclose(f);
  }
    
  bool load()
  {
    FILE* f = fopen(std::string(filenm_+".over").c_str(), "r");
    if (f == NULL)
      return false;
    fclose(f);
    
    f = fopen(std::string(filenm_+".id2.str").c_str(), "r");
    id2_str_table_.load(f);
    fclose(f);

    f = fopen(std::string(filenm_+".id.id").c_str(), "r");
    id_table_.load(f);
    fclose(f);

    return true;
  }
  
};

NS_IZENELIB_IR_END

#endif
