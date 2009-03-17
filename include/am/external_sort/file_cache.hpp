#ifndef FILE_CACHE_HPP
#define FILE_CACHE_HPP

#include <types.h>
#include <string>
#include <stdio.h>
#include <util/log.h>

using namespace std;

template<
  uint32_t SIZE = 1000000
  >
class FileCache
{
public:
  FileCache(const string& fileName)
  {
    f_ = fopen(fileName.c_str(), "r");
    if (f_==NULL)
    {
      LDBG_<<"Can not open file: "<<fileName;
      return;
    }
    fseek(f_, 0 ,SEEK_END);
    len_ = ftell(f_);
  }

  FileCache(FILE* f)
  {
    f_ = f;
    fseek(f_, 0 ,SEEK_END);
    len_ = ftell(f_);
  }
  

  bool read(uint64_t start, uint64_t size, char* buf)
  {
    if (f_ == NULL)
      return false;

    if (start+size > len_)
    {
      LDBG_<<"Required file block exceed the file size\n";
      return false;
    }

    if (start>=start_ && start+size<=end_)
    {
      memcpy(buf, buffer_+start-start_, size);
      return true;
    }

    //pre-loading
    uint64_t s = start+SIZE>len_? len_-start: SIZE;

    fseek(f_, start ,SEEK_SET);
    if(fread(buffer_, s, 1, f_)!=1)
    {
      LDBG_<<"Can't read file for file cache.";
      return false;
    }

    start_ = start;
    end_ = start_ + s;

    memcpy(buf, buffer_, size);
    return true;
  }
  
protected:
  FILE* f_;
  char buffer_[SIZE];
  uint64_t start_;
  uint64_t end_;
  uint64_t len_;
}
  ;
#endif
