///
/// @file ssfr.h
/// @brief A new refactored version of simple sequence file.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-03-21
/// @date Updated 2011-03-21
///

#ifndef _IZENELIB_AM_SSF_H_
#define _IZENELIB_AM_SSF_H_

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <util/izene_serialization.h>
#include <am/external_sort/izene_sort.hpp>
// #include <util/Exception.h>

NS_IZENELIB_AM_BEGIN

namespace ssf
{
template <typename LenType=uint32_t,
template <typename T> class Deserializer = izenelib::util::izene_deserialization_boost_binary>
class Reader {
public:
    Reader( const std::string& file ):
    file_(file), stream_(), count_(0), index_(0), fail_(false)
    {
    }
    
    ~Reader()
    {
        Close();
    }
    
    bool Failed()
    {
      if(fail_) return true;
      return stream_.fail();
    }
    
    bool Open()
    {
        Close();
      stream_.open(file_.c_str());
      if(Failed()) 
      {
        std::cerr<<"open "<<file_<<" failed"<<std::endl;
        perror("reason: ") ;
        return false;
      }
      LoadCount_();
      if(Failed()) 
      {
        std::cerr<<"open "<<file_<<" failed"<<std::endl;
        perror("reason: ") ;
        return false;
      }
      return true;
    }
    
    const std::string& GetPath() const
    {
        return file_;
    }
    
    uint64_t Count() const
    {
      return count_;
    }
    
    template <class T>
    bool Next(T& t)
    {
      if(index_ >= Count() ) 
      {
        return false;
      }
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      //std::cerr<<"recode size "<<recodeSize<<std::endl;
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      typename Deserializer<T>::type izd_value(data,recodeSize);
      izd_value.read_image(t);
      delete[] data;
      index_++;
      return true;
    }
    
    template <class T>
    bool NextOne(T& t)
    {
      if(index_ >= Count() ) 
      {
        return false;
      }
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      //std::cerr<<"recode size "<<recodeSize<<std::endl;
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      typename Deserializer<T>::type izd_value(data,recodeSize);
      izd_value.read_image(t);
      delete[] data;
      index_++;
      return true;
    }

    template <typename K, class V>
    bool Next(K& key, V& value)
    {
      if(index_ >= Count() ) 
      {
        return false;
      }
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      
      char* pData = data;
      memcpy( &key, pData, sizeof(K) );
      pData += sizeof(K);
      std::size_t valueSize = recodeSize - sizeof(K);
      typename Deserializer<V>::type izd_value(pData,valueSize);
      izd_value.read_image(value);
      delete[] data;
      index_++;
      return true;
    }
    
    template <class T>
    bool Next(std::vector<T>& t_list)
    {
      if(index_ >= Count() ) 
      {
          return false;
      }
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      if( recodeSize % sizeof(T) != 0 )
      {
        fail_ = true;
        return false;
      }
      std::size_t t_count = recodeSize / sizeof(T);
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      t_list.resize(t_count);
      char* pData = data;
      for( uint32_t i=0;i<t_list.size();i++)
      {
          memcpy( &t_list[i], pData, sizeof(T) );
          pData += sizeof(T);
      }
      delete[] data;
      index_++;
      return true;
    }
    
    template <typename K, class V>
    bool Next(std::pair<K,V>& t)
    {
      if(index_ >= Count() ) 
      {
        return false;
      }
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( stream_.fail() )
      {
        delete[] data;
        return false;
      }
      
      char* pData = data;
      memcpy( &t.first, pData, sizeof(K) );
      pData += sizeof(K);
      std::size_t valueSize = recodeSize - sizeof(K);
      typename Deserializer<V>::type izd_value(pData,valueSize);
      izd_value.read_image(t.second);
      delete[] data;
      index_++;
      return true;
    }
    
    void Close()
    {
      stream_.close();
    }
    
private:
    void LoadCount_()
    {
      stream_.seekg(0, ios::end);
      streampos size = stream_.tellg();
      if (size == (streampos) 0)
      {
          count_ = 0;
      }
      else
      {
          stream_.seekg(0, ios::beg);
          stream_.read((char*) &count_, sizeof(count_));
      }
    }    
    
private:
    std::string file_;
    std::ifstream stream_;
    uint64_t count_;
    uint64_t index_;
    bool fail_;
};

template <typename LenType=uint32_t,
template <typename T> class Deserializer = izenelib::util::izene_deserialization_boost_binary>
class Reader2 {
public:
    Reader2( const std::string& file ):
    file_(file), stream_(), count_(0), index_(0), fail_(false)
    {
    }
    
    ~Reader2()
    {
        Close();
    }
    
    bool Failed()
    {
      if(fail_) return true;
      return stream_.fail();
    }
    
    bool Open()
    {
        Close();
      stream_.open(file_.c_str());
      if(Failed()) 
      {
        std::cerr<<"open "<<file_<<" failed"<<std::endl;
        perror("reason: ") ;
        return false;
      }
      LoadCount_();
      if(Failed()) 
      {
        std::cerr<<"open "<<file_<<" failed"<<std::endl;
        perror("reason: ") ;
        return false;
      }
      return true;
    }
    
    const std::string& GetPath() const
    {
        return file_;
    }
    
    uint64_t Count() const
    {
      return count_;
    }
    
    template <class T>
    bool Next(T& t)
    {
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      typename Deserializer<T>::type izd_value(data,recodeSize);
      izd_value.read_image(t);
      delete[] data;
      index_++;
      return true;
    }
    
    template <typename K, class V>
    bool Next(K& key, V& value)
    {
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      
      char* pData = data;
      memcpy( &key, pData, sizeof(K) );
      pData += sizeof(K);
      std::size_t valueSize = recodeSize - sizeof(K);
      typename Deserializer<V>::type izd_value(pData,valueSize);
      izd_value.read_image(value);
      delete[] data;
      index_++;
      return true;
    }
    
    template <class T>
    bool Next(std::vector<T>& t_list)
    {
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      if( recodeSize % sizeof(T) != 0 )
      {
        fail_ = true;
        return false;
      }
      std::size_t t_count = recodeSize / sizeof(T);
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( Failed() )
      {
        delete[] data;
        return false;
      }
      t_list.resize(t_count);
      char* pData = data;
      for( uint32_t i=0;i<t_list.size();i++)
      {
          memcpy( &t_list[i], pData, sizeof(T) );
          pData += sizeof(T);
      }
      delete[] data;
      index_++;
      return true;
    }
    
    template <typename K, class V>
    bool Next(std::pair<K,V>& t)
    {
      LenType recodeSize = 0;
      stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
      if ( Failed() )
      {
        return false;
      }
      char* data = new char[recodeSize];
      stream_.read( data, recodeSize );
      if ( stream_.fail() )
      {
        delete[] data;
        return false;
      }
      
      char* pData = data;
      memcpy( &t.first, pData, sizeof(K) );
      pData += sizeof(K);
      std::size_t valueSize = recodeSize - sizeof(K);
      typename Deserializer<V>::type izd_value(pData,valueSize);
      izd_value.read_image(t.second);
      delete[] data;
      index_++;
      return true;
    }
    
    void Close()
    {
      stream_.close();
    }
    
private:
    void LoadCount_()
    {
      stream_.seekg(0, ios::end);
      streampos size = stream_.tellg();
      if (size == (streampos) 0)
      {
          count_ = 0;
      }
      else
      {
          stream_.seekg(0, ios::beg);
          stream_.read((char*) &count_, sizeof(count_));
      }
    }    
    
private:
    std::string file_;
    std::ifstream stream_;
    uint64_t count_;
    uint64_t index_;
    bool fail_;
};

template <typename LenType=uint32_t,
template <typename T> class Serializer = izenelib::util::izene_serialization_boost_binary>
class Writer {
public:
    Writer( const std::string& file ):
    file_(file), stream_(), count_(0)
    {
    }
    
    ~Writer()
    {
        Close();
    }
    
    bool Failed()
    {
      return stream_.fail();
    }
    
    bool Open()
    {
      if (!boost::filesystem::exists(file_))
      {
          std::ofstream ostream(file_.c_str());
          ostream << std::flush;
          ostream.close();
      }
      Close();
      stream_.open(file_.c_str());
      if (Failed())
      {
        return false;
      }
      LoadCount_();
      stream_.flush();
      stream_.seekg(0, ios::end);
      if (Failed())
      {
        return false;
      }
      return true;
    }
    
    
    const std::string& GetPath() const
    {
        return file_;
    }
    
    uint64_t Count() const
    {
      return count_;
    }

    std::fstream& GetRaw()
    {
        return stream_;
    }
    
    template <class T>
    bool Append(const T& t)
    {
      char* ptr;
      std::size_t valueSize;
      typename Serializer<T>::type izs(t);
      izs.write_image(ptr, valueSize);
      LenType recodeSize = valueSize;
      char* data = new char[recodeSize + sizeof(recodeSize)];
      char* pData = data;
      memcpy( pData, &recodeSize, sizeof(recodeSize) );
      pData += sizeof(recodeSize);
      memcpy( pData, ptr, valueSize );
      stream_.write(data, recodeSize+ sizeof(recodeSize));
      delete[] data;
      if( Failed() )
      {
        return false;
      }
      count_++;
      return true;
    }
    template <class T>
    bool AppendOne(const T& t)
    {
      char* ptr;
      std::size_t valueSize;
      typename Serializer<T>::type izs(t);
      izs.write_image(ptr, valueSize);
      LenType recodeSize = valueSize;
      char* data = new char[recodeSize + sizeof(recodeSize)];
      char* pData = data;
      memcpy( pData, &recodeSize, sizeof(recodeSize) );
      pData += sizeof(recodeSize);
      memcpy( pData, ptr, valueSize );
      stream_.write(data, recodeSize+ sizeof(recodeSize));
      delete[] data;
      if( Failed() )
      {
        return false;
      }
      count_++;
      return true;
    }
    
    template <typename K, class V>
    bool Append(const K& key, const V& value)
    {
      char* ptr;
      std::size_t valueSize;
      typename Serializer<V>::type izs(value);
      izs.write_image(ptr, valueSize);
      LenType keySize = sizeof(K);
      LenType recodeSize = keySize + valueSize;
      char* data = new char[recodeSize + sizeof(recodeSize)];
      char* pData = data;
      memcpy( pData, &recodeSize, sizeof(recodeSize) );
      pData += sizeof(recodeSize);
      memcpy( pData, &key, keySize );
      pData += keySize;
      memcpy( pData, ptr, valueSize );
      stream_.write(data, recodeSize+ sizeof(recodeSize));
      delete[] data;
      if( Failed() )
      {
        return false;
      }
      count_++;
      return true;
    }
    
    template <typename K, class V>
    bool Append(const std::pair<K,V>& t)
    {
      return Append(t.first, t.second);
    }
    
    
    template <typename T>
    bool Append(const std::vector<T>& t_list)
    {
      LenType keySize = sizeof(T);
      LenType recodeSize = keySize * t_list.size();
      char* data = new char[recodeSize + sizeof(recodeSize)];
      char* pData = data;
      memcpy( pData, &recodeSize, sizeof(recodeSize) );
      pData += sizeof(recodeSize);
      for( uint32_t i=0;i<t_list.size();i++)
      {
          memcpy( pData, &t_list[i], keySize );
          pData += keySize;
      }
      
      stream_.write(data, recodeSize+ sizeof(recodeSize));
      delete[] data;
      if( Failed() )
      {
        return false;
      }
      count_++;
      return true;
    }
    
    template <typename T>
    bool BatchAppend(const std::vector<std::vector<T> >& t_list_list)
    {
        LenType keySize = sizeof(T);
        LenType plen = 0;
        for(std::size_t t=0;t<t_list_list.size();t++)
        {
            const std::vector<T>& t_list = t_list_list[t];
            LenType recodeSize = keySize * t_list.size();
            LenType line_plen = recodeSize+sizeof(recodeSize);
            plen+=line_plen;
        }
        char* data = new char[plen];
        char* pData = data;
        for(std::size_t t=0;t<t_list_list.size();t++)
        {
            const std::vector<T>& t_list = t_list_list[t];
            LenType recodeSize = keySize * t_list.size();
            //LenType line_plen = recodeSize+sizeof(recodeSize);
            memcpy( pData, &recodeSize, sizeof(recodeSize) );
            pData += sizeof(recodeSize);
            for( uint32_t i=0;i<t_list.size();i++)
            {
                memcpy( pData, &t_list[i], keySize );
                pData += keySize;
            }
        }

        stream_.write(data, plen);
        delete[] data;
        if( Failed() )
        {
            return false;
        }
        count_+=t_list_list.size();
        return true;
    }
    
    bool Flush()
    {
        WriteCount_();
        stream_.flush();
        if(Failed()) return false;
        return true;
    }
    
    void Close()
    {
        WriteCount_();
        stream_.close();
    }
    
private:
    
    void WriteCount_()
    {
      stream_.seekg(0, ios::beg);
      stream_.write((char*) &count_, sizeof(count_));
      stream_.seekg(0, ios::end);
    }
    
    void LoadCount_()
    {
      stream_.seekg(0, ios::end);
      streampos size = stream_.tellg();
      if (size == (streampos) 0)
      {
          count_ = 0;
          WriteCount_();
      }
      else
      {
          stream_.seekg(0, ios::beg);
          stream_.read((char*) &count_, sizeof(count_));
      }
    }
    
private:
    std::string file_;
    std::fstream stream_;
    uint64_t count_;
        
};

template <typename LenType = uint32_t, typename KeyType = uint32_t, bool COMPARE_ALL = false>
class Sorter {
    public:
        Sorter( )
        {
        }
        ~Sorter()
        {
        }
        
        static void Sort(const std::string& file)
        {
            izenelib::am::IzeneSort<KeyType, LenType, COMPARE_ALL> sorter("");
            sorter.sort(file);
        }
        

                
    
        
};

template <typename LenType=uint32_t>
class Util {
public:
  
  template <class T>
  static bool Load(const std::string& file, std::vector<T>& value)
  {
    Reader<LenType> reader(file);
    if(!reader.Open())
    {
      return false;
    }
    std::size_t count = reader.Count();
    value.resize(count);
    //value.reserve(count);
    std::size_t i=0;
    while(i<count)
    {
        //if(i%1==0)
        //{
            //std::cerr<<"loading "<<i<<","<<value.size()<<std::endl;
        //}
        //T v;
        //if(!reader.Next(v)) break;
        //value.push_back(v);
        //++i;
        if(!reader.Next(value[i++])) break;
    }
    //std::cerr<<"load i "<<i<<std::endl;
    return true;
  }
  
  template <class T>
  static bool Load(const std::string& file, T& value)
  {
    std::vector<T> value_list;
    if(!Load(file, value_list)) return false;
    if(value_list.size()!=1) return false;
    value = value_list[0];
    return true;
  }
  template <class T>
  static bool LoadOne(const std::string& file, std::vector<T>& value)
  {
    Reader<LenType> reader(file);
    if(!reader.Open())
    {
      return false;
    }
    std::size_t count = reader.Count();
    value.resize(count);
    //value.reserve(count);
    std::size_t i=0;
    while(i<count)
    {
        //if(i%1==0)
        //{
            //std::cerr<<"loading "<<i<<","<<value.size()<<std::endl;
        //}
        //T v;
        //if(!reader.Next(v)) break;
        //value.push_back(v);
        //++i;
        if(!reader.NextOne(value[i++])) break;
    }
    //std::cerr<<"load i "<<i<<std::endl;
    return true;
  }
  
  template <class T>
  static bool LoadOne(const std::string& file, T& value)
  {
    std::vector<T> value_list;
    if(!LoadOne(file, value_list)) return false;
    if(value_list.size()!=1) return false;
    value = value_list[0];
    return true;
  }

  template <class T>
  static bool Save(const std::string& file, const std::vector<T>& value)
  {
    try
    {
      boost::filesystem::remove_all(file);
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    Writer<LenType> writer(file);
    if(!writer.Open())
    {
      return false;
    }
    for(std::size_t i=0;i<value.size();i++)
    {
      if(!writer.Append(value[i])) return false;
    }
    writer.Close();
    return true;
  }
  
  template <class T>
  static bool Save(const std::string& file, const T& value)
  {
    std::vector<T> value_list(1, value);
    return Save(file, value_list);
  }
  
  template <class T>
  static bool SaveOne(const std::string& file, const std::vector<T>& value)
  {
    try
    {
      boost::filesystem::remove_all(file);
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    Writer<LenType> writer(file);
    if(!writer.Open())
    {
      return false;
    }
    for(std::size_t i=0;i<value.size();i++)
    {
      if(!writer.AppendOne(value[i])) return false;
    }
    writer.Close();
    return true;
  }
  
  template <class T>
  static bool SaveOne(const std::string& file, const T& value)
  {
    std::vector<T> value_list(1, value);
    return SaveOne(file, value_list);
  }
  
};

template <typename LenType, typename KeyType, class ValueType>
class Joiner {
public:
  Joiner(const std::string& file)
  :reader_(new Reader<LenType>(file)), b_(false)
  {
  }
  
  ~Joiner()
  {
    delete reader_;
  }
  
  bool Open()
  {
    if(!reader_->Open()) return false;
    if(reader_->Next(pair_.first, pair_.second))
    {
      b_ = true;
    }
    return true;
  }
  
  bool Next(KeyType& key, std::vector<ValueType>& value_list)
  {
    if(!b_) return false;
    key = pair_.first;
    value_list.resize(0);
    while(b_)
    {
      if( pair_.first == key )
      {
        value_list.push_back(pair_.second);
        if(!reader_->Next(pair_.first, pair_.second))
        {
          b_ = false;
          break;
        }
      }
      else
      {
        break;
      }
    }
    return true;
  }
  
private:
  Reader<LenType>* reader_;
  bool b_;
  std::pair<KeyType, ValueType> pair_;
//   std::vector<ValueType> value_list_;
  
  
};

template <typename LenType, typename KeyType, class ValueType1, class ValueType2>
class Merger {
public:
  Merger(Reader<LenType>* reader1, Reader<LenType>* reader2)
  :reader1_(reader1), reader2_(reader2), b1_(false), b2_(false)
  {
    if(reader1_->Next(pair1_.first, pair1_.second))
    {
      b1_ = true;
    }
    if(reader2_->Next(pair2_.first, pair2_.second))
    {
      b2_ = true;
    }
  }
  
  bool Next(KeyType& key, std::vector<ValueType1>& value_list1, std::vector<ValueType2>& value_list2)
  {
    bool key_set = false;
    if( b1_ )
    {
      key = pair1_.first;
      key_set = true;
    }
    if( b2_ )
    {
      if( key_set )
      {
        if( pair2_.first< key )
        {
          key = pair2_.first;
        }
      }
      else
      {
        key = pair2_.first;
        key_set = true;
      }
    }
    if( !key_set )
    {
      return false;
    }
    value_list1.resize(0);
    value_list2.resize(0);
    while( b1_ )
    {
      if( pair1_.first == key )
      {
        value_list1.push_back(pair1_.second);
        if(!reader1_->Next(pair1_.first, pair1_.second))
        {
          b1_ = false;
          break;
        }
      }
      else
      {
        break;
      }
    }
    while( b2_ )
    {
      if( pair2_.first == key )
      {
        value_list2.push_back(pair2_.second);
        if(!reader2_->Next(pair2_.first, pair2_.second))
        {
          b2_ = false;
          break;
        }
      }
      else
      {
        break;
      }
    }
    
    
    return true;
  }
  
private:
  
    
private:
  Reader<LenType>* reader1_;
  Reader<LenType>* reader2_;
  bool b1_;
  bool b2_;
  std::pair<KeyType, ValueType1> pair1_;
  std::pair<KeyType, ValueType2> pair2_;
};

}

NS_IZENELIB_AM_END

#endif 
