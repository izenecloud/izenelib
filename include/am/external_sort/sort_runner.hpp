/**
   @file multi_pass_sort.hpp
   @author Kevin Hu
   @date 2010.01.20
 */
#ifndef SORT_RUNNER_HPP
#define SORT_RUNNER_HPP

#include <util/log.h>
#include <vector>
#include <string>
#include <types.h>
#include <stdio.h>
#include <time.h>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include <am/dyn_array.hpp>
#include <math.h>
#include <sys/time.h>

NS_IZENELIB_AM_BEGIN

struct DirectIOPolicy
{
public:
    DirectIOPolicy(FILE* fd, const std::string& mode = "r")
      :fd_(fd),length_(0)
    {
        if (mode.compare("r") == 0)
        {
            fseek(fd_, 0, SEEK_END);
            length_ = ftell(fd_);
            fseek(fd_, 0, SEEK_SET);
        }
    }

    bool _isCompression() { return false;}

    size_t _write(char* const data, size_t length)
    {
        return fwrite(data, length, 1, fd_);
    }

    size_t _read(char* data, size_t length)
    {
        return fread(data, 1, length, fd_);
    }

    void _readBytes(char* data, size_t len)
    {
        IASSERT(fread(data, len, 1, fd_) == 1);
    }
    size_t _tell()
    {
        return ftell(fd_);
    }

    void _seek(size_t pos, int origin = SEEK_SET)
    {
        fseek(fd_,pos,origin);
    }

    size_t _length()
    {
        return length_;
    }

    FILE* fd_;
    size_t length_;
};

template<typename SortIOPolicy>
class SortIO:public SortIOPolicy
{
public:
    SortIO(FILE* fd,const std::string& mode = "r")
      :SortIOPolicy(fd,mode)
    {}

    bool isCompression()
    {
        return this->_isCompression();
    }

    size_t write(char* const data, int length)
    {
        return this->_write(data, length);
    }

    size_t read(char* data, int length)
    {
        return this->_read(data, length);
    }

    void readBytes(char* data, size_t len)
    {
        this->_readBytes(data, len);
    }

    size_t tell()
    {
        return this->_tell();
    }

    void seek(size_t pos,  int origin = SEEK_SET)
    {
        this->_seek(pos, origin);
    }

    size_t length()
    {
        return this->_length();
    }

};

typedef SortIO<DirectIOPolicy> DirectIO;

/**
   @class MultiPassSort
 **/
template<
  class KEY_TYPE = uint32_t,//pre-key type, indicate the length of the pre-key.
  class LEN_TYPE = uint8_t,//
  bool  COMPARE_ALL = false,
  typename IO_TYPE = DirectIO
>
class SortRunner
{
typedef SortRunner<KEY_TYPE, LEN_TYPE, COMPARE_ALL, IO_TYPE> self_t;

  struct KEY_PTR;
  std::string filenm_;
  char* pre_buf_;
  char* run_buf_;
  KEY_PTR* key_buf_;
  char* out_buf_;

  uint32_t RUN_BUF_SIZE_;

  boost::mutex pre_buf_mtx_;
  boost::condition pre_buf_con_;
  boost::mutex out_buf_mtx_;
  boost::condition out_buf_con_;

  uint32_t pre_buf_size_;
  uint32_t pre_buf_num_;
  uint32_t out_buf_size_;
  uint32_t out_buf_num_;
  uint64_t count_;
  uint32_t run_num_;

  uint32_t max_record_len_;
  uint32_t max_record_len_of_this_run_;
  uint32_t min_run_buff_size_for_merger_;
    /**
     @brief pre-key used for sorting
   */
  struct KEY_PTR
  {
    uint32_t pos;

    KEY_TYPE& KEY_(char* buf)
    {
      return *(KEY_TYPE*)(buf+pos+sizeof(LEN_TYPE));
    }

    KEY_TYPE KEY(const char* buf)const
    {
      return *(KEY_TYPE*)(buf+pos+sizeof(LEN_TYPE));
    }

    LEN_TYPE& LEN_(char* buf)
    {
      return *(LEN_TYPE*)(buf+pos);
    }

    LEN_TYPE LEN(const char* buf)const
    {
      return *(LEN_TYPE*)(buf+pos);
    }

    inline KEY_PTR(uint32_t p)
    {
      pos = p;
    }

    inline KEY_PTR()
    {
      pos = -1;
    }

    KEY_PTR& operator = (const KEY_PTR& other)
    {
      pos = other.pos;
      return *this;
    }

    int compare(KEY_PTR& p, const char* buf)const
    {
      if (KEY(buf) == p.KEY(buf) && !COMPARE_ALL)
        return 0;

      if (KEY(buf)> p.KEY(buf))
        return 1;
      if (KEY(buf)< p.KEY(buf))
        return -1;

      if (!COMPARE_ALL)
        return 0;

      LEN_TYPE len1 = LEN(buf)/sizeof(KEY_TYPE);
      LEN_TYPE len2 = p.LEN(buf)/sizeof(KEY_TYPE);

      for (LEN_TYPE i=1; i<len1 && i<len2; ++i)
      {
        if (((KEY_TYPE*)(buf+pos+sizeof(LEN_TYPE)))[i]>((KEY_TYPE*)(buf+p.pos+sizeof(LEN_TYPE)))[i])
          return 1;
        if (((KEY_TYPE*)(buf+pos+sizeof(LEN_TYPE)))[i]<((KEY_TYPE*)(buf+p.pos+sizeof(LEN_TYPE)))[i])
          return -1;
      }

      if (len1 == len2)
        return 0;

      if (len1>len2)
        return 1;
      return -1;

    }
  };

  inline void new_buffer_()
  {
    if (!pre_buf_)
      pre_buf_ = (char*)malloc(RUN_BUF_SIZE_);

    if (!run_buf_)
      run_buf_ = (char*)malloc(RUN_BUF_SIZE_);

    if (!out_buf_)
      out_buf_ = (char*)malloc(RUN_BUF_SIZE_);
  }

  void quick_sort_(int left, int right, int limit)
  {
    int i = left, j = right;
    KEY_PTR tmp;
    KEY_PTR pivot = key_buf_[(left + right) / 2];

    /* partition */
    while (i <= j) {

      while (i < limit && key_buf_[i].compare(pivot, run_buf_)<0)
        i++;
      while (j>=0 && key_buf_[j].compare(pivot, run_buf_)>0)
        j--;

      if (i <= j) {
        tmp = key_buf_[i];
        key_buf_[i] = key_buf_[j];
        key_buf_[j] = tmp;
        i++;
        j--;
      }
    };

    //IASSERT(i-1==j);
    /* recursion */
    if (left < j)
      quick_sort_(left, j, limit);

    if (i < right)
      quick_sort_(i, right, limit);

  }

  bool t_check_quick_sort_()
  {
    // uint32_t s = pos_/sizeof(struct KEY_PTR)-1;

//     for (uint32_t i=0; i<s-1; ++i)
//       if (key_buf_[i].compare(key_buf_[i+1], original_f_)>0)
//         //          || key_buf_[i].KEY(run_buf_)+1!=key_buf_[i+1].KEY(run_buf_))
//       {
//         std::cout<<s<<"-"<<i<<std::endl;
//         std::cout<<key_buf_[i]<<"--"<<key_buf_[i+1]<<std::endl;
//         return false;
//       }

    return true;
  }

  void prefetch_(FILE* f)
  {
    IO_TYPE ioStream(f);
    const uint64_t FILE_LEN = ioStream.length();
    run_num_ = 0;
    uint64_t pos = sizeof(uint64_t);
    std::cout<<std::endl;
    ioStream.seek(pos);

    while(pos < FILE_LEN)
    {
      std::cout<<"\rA runner is processing "<<pos*1./FILE_LEN<<std::flush;
      ++run_num_;
      boost::mutex::scoped_lock lock(pre_buf_mtx_);

      while (pre_buf_size_!=0)
        pre_buf_con_.wait(lock);

      //uint32_t s = (uint32_t)(FILE_LEN-pos>RUN_BUF_SIZE_? RUN_BUF_SIZE_: FILE_LEN-pos);
      uint32_t s;
      if(!ioStream.isCompression())
         s = (uint32_t)(FILE_LEN-pos>RUN_BUF_SIZE_? RUN_BUF_SIZE_: FILE_LEN-pos);
      else
         s = RUN_BUF_SIZE_;
      //std::cout<<std::endl<<pos<<"-"<<FILE_LEN<<"-"<<RUN_BUF_SIZE_<<"-"<<s<<std::endl;
      if(!ioStream.isCompression())
        ioStream.seek(pos);
      //IASSERT(fread(pre_buf_, s, 1, f)==1);
      s = ioStream.read(pre_buf_, s);
      if(!ioStream.isCompression())
        pos += (uint64_t)s;
      else
        pos = ioStream.tell();

      //check the position of the last record
      pre_buf_size_ = 0;
      pre_buf_num_ = 0;
      for(; pre_buf_size_<s; ++pre_buf_num_)
      {
        if (pre_buf_size_+*(LEN_TYPE*)(pre_buf_+pre_buf_size_)+sizeof(LEN_TYPE)>s)
          break;
        pre_buf_size_ += *(LEN_TYPE*)(pre_buf_+pre_buf_size_)+sizeof(LEN_TYPE);
      }
      pos -= (uint64_t)(s- pre_buf_size_);

      //std::cout<<"pre_buf_size_ "<<pre_buf_size_<<" pre_buf_num_ "<<pre_buf_num_<<" ret "<<s<<" pos "<<pos<<std::endl;

      if (pre_buf_num_ == 0)
      {
        std::cout<<"\n[Warning]: A record is too long, and has been ignored!\n";
        //pos += *(LEN_TYPE*)(pre_buf_+pre_buf_size_) + sizeof(LEN_TYPE);
        --count_;
        RUN_BUF_SIZE_ = (uint32_t)((*(LEN_TYPE*)(pre_buf_+pre_buf_size_) + sizeof(LEN_TYPE))*1.1);
        pre_buf_ = (char*)realloc(pre_buf_, RUN_BUF_SIZE_);
        continue;
      }

      //IASSERT(pre_buf_size_ <= RUN_BUF_SIZE_);
      pre_buf_con_.notify_one();
    }
    std::cout<<"Prefetching is over...\n";
  }

  void sort_()
  {
    uint64_t count = 0;
    while (count< count_)
    {
      uint32_t pre_buf_size = 0;
      uint32_t pre_buf_num = 0;
      {
        boost::mutex::scoped_lock lock(pre_buf_mtx_);
        while (pre_buf_size_==0)
          pre_buf_con_.wait(lock);

        assert(pre_buf_size_ <= RUN_BUF_SIZE_);
        memcpy(run_buf_, pre_buf_, pre_buf_size_);
        pre_buf_size = pre_buf_size_;
        pre_buf_num = pre_buf_num_;
        count += pre_buf_num_;
        pre_buf_num_ = pre_buf_size_ = 0;
        pre_buf_con_.notify_one();
      }

      key_buf_ = (struct KEY_PTR*)realloc(key_buf_, pre_buf_num*sizeof(struct KEY_PTR));

      uint32_t pos = 0;
      for (uint32_t i = 0; i<pre_buf_num; ++i)
      {
        key_buf_[i] = KEY_PTR(pos);
        assert(pos <= RUN_BUF_SIZE_);
        pos += *(LEN_TYPE*)(run_buf_ + pos)+sizeof(LEN_TYPE);
        IASSERT(pos<=pre_buf_size);
      }
      IASSERT(pos==pre_buf_size);

      quick_sort_(0, pre_buf_num-1, pre_buf_num);

      boost::mutex::scoped_lock lock(out_buf_mtx_);
      while (out_buf_size_ != 0)
        out_buf_con_.wait(lock);

      out_buf_size_ = 0;
      out_buf_num_ = 0;
      LEN_TYPE max_len_of_this_run = (LEN_TYPE)0;
      for (uint32_t i=0; i<pre_buf_num; ++i, ++out_buf_num_)
      {
        assert(key_buf_[i].pos <= RUN_BUF_SIZE_);
        assert(out_buf_size_+key_buf_[i].LEN(run_buf_)+ sizeof(LEN_TYPE) <= RUN_BUF_SIZE_);
        memcpy(out_buf_+out_buf_size_, run_buf_+ key_buf_[i].pos, key_buf_[i].LEN(run_buf_)+ sizeof(LEN_TYPE));
        LEN_TYPE len = key_buf_[i].LEN(run_buf_) + sizeof(LEN_TYPE);
        out_buf_size_ += len;
        if(len > max_len_of_this_run) max_len_of_this_run = len;
      }
      max_record_len_of_this_run_ = max_len_of_this_run;
      min_run_buff_size_for_merger_ += max_record_len_of_this_run_;
      if(max_len_of_this_run > max_record_len_) max_record_len_ = (uint32_t)max_len_of_this_run;

      IASSERT(out_buf_num_ == pre_buf_num);
      IASSERT(out_buf_size_ == pre_buf_size);

      out_buf_con_.notify_one();
    }

    std::cout<<"Sorting is over...\n";
  }

  void output_(FILE* f)
  {
    IO_TYPE ioStream(f,"w");

    uint64_t count = 0;
    uint64_t nextStart = 0;
    while (count< count_)
    {
      boost::mutex::scoped_lock lock(out_buf_mtx_);
      while (out_buf_size_ == 0)
        out_buf_con_.wait(lock);

      IASSERT(fwrite(&out_buf_size_, sizeof(uint32_t), 1, f)==1);
      IASSERT(fwrite(&out_buf_num_, sizeof(uint32_t), 1, f)==1);
      //IASSERT(fwrite(&max_record_len_of_this_run_, sizeof(uint32_t), 1, f)==1);	  //TODO
      uint64_t nextStartPos = ftell(f);
      IASSERT(fwrite(&nextStart, sizeof(uint64_t), 1, f)==1);
      //IASSERT(fwrite(out_buf_, out_buf_size_, 1, f)==1);
      ioStream.write(out_buf_, out_buf_size_);
      nextStart = ftell(f);
      fseek(f, nextStartPos, SEEK_SET);
      IASSERT(fwrite(&nextStart, sizeof(uint64_t), 1, f)==1);
      fseek(f, nextStart, SEEK_SET);

      IASSERT(t_check_sort_());
      count += out_buf_num_;
      out_buf_size_ = out_buf_num_ = 0;
      out_buf_con_.notify_one();
    }

    std::cout<<"Outputting is over...\n";
  }

  bool t_check_sort_()
  {
    KEY_PTR last(0);
    uint32_t pos= last.LEN(out_buf_)+sizeof(LEN_TYPE);
    for (; pos<out_buf_size_;)
    {
      KEY_PTR one(pos);
      if (one.compare(last, out_buf_)<0)
        return false;
      pos += one.LEN(out_buf_)+sizeof(LEN_TYPE);
    }

    return true;
  }

public:
  SortRunner(const char* filenm, uint32_t buf_size = 100000000)
    :filenm_(filenm), RUN_BUF_SIZE_((uint32_t)(1.*buf_size*0.3))
  {
    pre_buf_ = run_buf_  = out_buf_ = NULL;
    key_buf_ = NULL;

    pre_buf_size_ = pre_buf_num_ = out_buf_size_ = out_buf_num_ = count_ = run_num_ = 0;

    max_record_len_ = 0;
    max_record_len_of_this_run_ = 0;
    min_run_buff_size_for_merger_ = 0;
  }

  ~SortRunner()
  {
    if (pre_buf_)
      free(pre_buf_);

    if (run_buf_)
      free(run_buf_);

    if (key_buf_)
      free(key_buf_);

    if (out_buf_)
      free(out_buf_);
  }

  void run()
  {
    struct timeval tvafter, tvpre;
    struct timezone tz;

    new_buffer_();

    gettimeofday (&tvpre , &tz);

    FILE* f = fopen(filenm_.c_str(), "r");
    IASSERT(f);
    IASSERT(fread(&count_, sizeof(uint64_t), 1, f)==1);

    boost::thread prefetch_thre(boost::bind(&self_t::prefetch_, this, f));
    boost::thread sort_thre(boost::bind(&self_t::sort_, this));

    FILE* out_f = fopen((filenm_+".out").c_str(), "w+");
    IASSERT(out_f);
    IASSERT(fwrite(&count_, sizeof(uint64_t), 1, out_f)==1);
    boost::thread out_thre(boost::bind(&self_t::output_, this, out_f));

    prefetch_thre.join();
    sort_thre.join();
    out_thre.join();

//     fseek(f, 0, SEEK_END);
//     fseek(out_f, 0, SEEK_END);
//     IASSERT((uint64_t)ftell(f)+ run_num_*sizeof(uint32_t)*2== (uint64_t)ftell(out_f));

    fseek(out_f, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(uint64_t), 1, out_f)==1);
    fclose(f);
    fclose(out_f);

    if (boost::filesystem::exists(filenm_))
      boost::filesystem::remove(filenm_);
    if (boost::filesystem::exists(filenm_+".out"))
      boost::filesystem::rename(filenm_+".out", filenm_);

    gettimeofday (&tvafter , &tz);
    std::cout<<"A run is over("<<count_
             <<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000<<" min\n";
  }

  uint32_t run_num()const
  {
    return run_num_;
  }

  uint32_t max_record_len()const
  {
    return max_record_len_;
  }

  uint32_t min_run_buf_size_for_merger()const
  {
    return min_run_buff_size_for_merger_;
  }

}
  ;

NS_IZENELIB_AM_END

#endif
