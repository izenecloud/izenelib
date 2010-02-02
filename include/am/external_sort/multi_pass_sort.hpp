/**
   @file multi_pass_sort.hpp
   @author Kevin Hu
   @date 2010.01.20
 */
#ifndef MULTI_PASS_SORT_HPP
#define MULTI_PASS_SORT_HPP

#include <util/log.h>
#include <vector>
#include <string>
#include <types.h>
#include <stdio.h>
#include <time.h>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <am/graph_index/dyn_array.hpp>
#include <math.h>
#include <sys/time.h>

NS_IZENELIB_AM_BEGIN

/**
   @class MultiPassSort
 **/
template<
  class PRE_KEY_TYPE = uint32_t,//pre-key type, indicate the length of the pre-key.
  class LEN_TYPE = uint8_t,//
  bool  COMPARE_ALL = false
>
class MultiPassSort
{

  typedef MultiPassSort<PRE_KEY_TYPE, LEN_TYPE, COMPARE_ALL> self_t;
  typedef izenelib::am::DynArray<boost::thread*> threads_t;
  typedef izenelib::am::DynArray<boost::mutex*> mutexs_t;
  
  std::string filenm_;
  uint32_t buf_size_;
  char* buffer_;  
  const uint32_t MAX_GROUP_SIZE;
  
  uint32_t pos_;
  FILE* original_f_;
  FILE* key_f_;
  boost::mutex keys_mtx_;
  boost::mutex join_mtx_;

  //for adding data
  char* tmp_buffer_;
  uint32_t tmp_pos_;
  uint64_t end_pos_;
  enum 
    {
      TMP_BUF_SIZE = 100000000
    };

  uint64_t count_;
  uint32_t thre_num_;
  /**
     @brief pre-key used for sorting
   */
  struct PRE_KEY_STRUCT
  {
    char pre_key_[sizeof(PRE_KEY_TYPE)];//!< pre-key, used to compare
    char rcrd_addr_[sizeof(uint64_t)];//!< record address
    char len_[sizeof(LEN_TYPE)];

    PRE_KEY_TYPE& PRE_KEY_()
    {
      return *(PRE_KEY_TYPE*)pre_key_;
    }

    PRE_KEY_TYPE PRE_KEY()const
    {
      return *(PRE_KEY_TYPE*)pre_key_ ;
    }    

    uint64_t& RCRD_ADDR_()
    {
      return *(uint64_t*)rcrd_addr_;
    }

    uint64_t RCRD_ADDR()const
    {
      return *(uint64_t*)rcrd_addr_ ;
    }    

    LEN_TYPE& LEN_()
    {
      return *(LEN_TYPE*)len_;
    }

    LEN_TYPE LEN()const
    {
      return *(LEN_TYPE*)len_ ;
    }    

    inline PRE_KEY_STRUCT(PRE_KEY_TYPE pre_key,uint64_t rcrd_addr, LEN_TYPE len)
    {
      PRE_KEY_() = pre_key;
      RCRD_ADDR_() = rcrd_addr;
      LEN_() = len;
    }

    inline PRE_KEY_STRUCT()
    {
      RCRD_ADDR_() = -1;
    }

    PRE_KEY_STRUCT& operator = (const PRE_KEY_STRUCT& other)
    {
      PRE_KEY_() = other.PRE_KEY();
      RCRD_ADDR_() = other.RCRD_ADDR();
      LEN_() = other.LEN();
      return *this;
    }
    
    int compare(const PRE_KEY_STRUCT& p, FILE* f) const
    {
      if (RCRD_ADDR() == p.RCRD_ADDR())
        return 0;

      if (PRE_KEY()> p.PRE_KEY())
        return 1;
      if (PRE_KEY()< p.PRE_KEY())
        return -1;

      if (!COMPARE_ALL)
        return 0;
      
      PRE_KEY_TYPE k=PRE_KEY(), pk=p.PRE_KEY();

      fseek(f, RCRD_ADDR(), SEEK_SET);
      LEN_TYPE len1 = 0;
      IASSERT(fread(&len1, sizeof(LEN_TYPE), 1, f)==1);
      
      fseek(f, p.RCRD_ADDR(), SEEK_SET);
      LEN_TYPE len2 = 0;
      IASSERT(fread(&len2, sizeof(LEN_TYPE), 1, f)==1);
      
      
      uint32_t shift = sizeof(PRE_KEY_TYPE);
        
      while (k == pk && len1>shift && len2>shift)
      {
        fseek(f, RCRD_ADDR()+sizeof(LEN_TYPE)+shift, SEEK_SET);
        uint32_t s = shift+sizeof(PRE_KEY_TYPE)>len1? len1-shift:sizeof(PRE_KEY_TYPE);
          
        IASSERT(fread(&k,s, 1, f)==1);

        fseek(f, p.RCRD_ADDR()+sizeof(LEN_TYPE)+shift, SEEK_SET);
        s = shift+sizeof(PRE_KEY_TYPE)>len2? len2-shift:sizeof(PRE_KEY_TYPE);

        IASSERT(fread(&k,s, 1, f)==1);

        shift += sizeof(PRE_KEY_TYPE);
      }

      return k< pk;
    }

  friend std::ostream& operator <<(std::ostream& os, const PRE_KEY_STRUCT& v)
    {
      os<<"["<<v.PRE_KEY()<<","<<v.RCRD_ADDR()<<"]";
      return os;
    }    
  };

  inline bool is_buffer_full_(uint32_t len)const
  {
    return pos_+sizeof(struct PRE_KEY_STRUCT)>buf_size_
      || tmp_pos_+sizeof(LEN_TYPE)+len>TMP_BUF_SIZE;
  }
 
  inline void flush_buffer_()
  {
    if (key_f_ == NULL)
    {
      key_f_ = fopen((filenm_+".key").c_str(), "w+");
      IASSERT(key_f_!=NULL);
    }
    
    fseek(original_f_, 0, SEEK_END);
    if (tmp_pos_!=0)
      IASSERT(fwrite(tmp_buffer_, tmp_pos_, 1, original_f_)==1);
    end_pos_ = ftell(original_f_);
    tmp_pos_ = 0;
    
    if (pos_ >0)
    {
      //std::cout<<std::endl<<pos_/sizeof(PRE_KEY_STRUCT)-1<<std::endl;
      quick_sort_(0, pos_/sizeof(PRE_KEY_STRUCT)-1);
      assert(t_check_quick_sort_());
      IASSERT(fwrite(buffer_, pos_, 1, key_f_)==1);
    }
    pos_ = 0;
  }
  
  inline void new_buffer_()
  {
    if (buffer_==NULL)
    {
      buffer_ = (char*)malloc(buf_size_);
      pos_ = 0;
    }

    if (tmp_buffer_==NULL)
    {
      tmp_buffer_ = (char*)malloc(TMP_BUF_SIZE);
      tmp_pos_ = 0;
    }
  }
  
  void output_keys_(FILE* f, const char* buf, uint32_t s, boost::thread* thre, boost::mutex* mt)
  {
    if (thre)
    {
      thre->join();
      delete thre;
    }
    IASSERT(fwrite(buf, s,1, f)==1);
    mt->unlock();
  }

  void shrink_key_()
  {
    /*flag 1: if address is larger than 2^32*//*address*/
    const uint32_t KEY_SIZE = 1+sizeof(uint32_t)+sizeof(LEN_TYPE);
    
    const uint32_t KEY_BUF_SIZE = buf_size_/2/sizeof(PRE_KEY_STRUCT)*sizeof(PRE_KEY_STRUCT);
    const uint32_t OUT_BUF_SIZE = buf_size_ - KEY_BUF_SIZE;
    
    FILE* f = fopen((filenm_+".key.out").c_str(), "w+");
    char* out_b = buffer_+KEY_BUF_SIZE;
    uint32_t out_p = 0;

    //get key file size
    fseek(key_f_, 0, SEEK_END);
    const uint64_t KEY_FILE_SIZE = ftell(key_f_);
    IASSERT(KEY_FILE_SIZE%sizeof(PRE_KEY_STRUCT)==0);
    
    const uint32_t KEY_LOAD_TIMES = KEY_FILE_SIZE%KEY_BUF_SIZE==0? KEY_FILE_SIZE/KEY_BUF_SIZE: KEY_FILE_SIZE/KEY_BUF_SIZE+1;

    count_ = 0;
    fseek(key_f_, 0, SEEK_SET);
    for (uint32_t i=0; i<KEY_LOAD_TIMES; ++i)
    {
      uint32_t ss = KEY_FILE_SIZE - ftell(key_f_)>KEY_BUF_SIZE? KEY_BUF_SIZE: KEY_FILE_SIZE - ftell(key_f_);//loading size
      IASSERT(ss%sizeof(PRE_KEY_STRUCT)==0);
      uint32_t nn = ss/sizeof(PRE_KEY_STRUCT);
      IASSERT(fread(buffer_, ss, 1, key_f_)==1);
      
      for (uint32_t j=0; j<nn; ++j)
      {
        if (out_p+KEY_SIZE > OUT_BUF_SIZE)
        {
          IASSERT(fwrite(out_b, out_p, 1, f)==1);
          out_p = 0;
        }

        *(out_b+out_p) = 0;
        if (((PRE_KEY_STRUCT*)buffer_)[j].RCRD_ADDR()>(uint64_t)((uint32_t)-1))
          *(out_b+out_p) = 1;

        ++out_p;
        *(uint32_t*)(out_b+out_p) = (uint32_t)((PRE_KEY_STRUCT*)buffer_)[j].RCRD_ADDR();
        if (((PRE_KEY_STRUCT*)buffer_)[j].RCRD_ADDR()>(uint64_t)((uint32_t)-1))
          *(uint32_t*)(out_b+out_p) = (uint32_t)((((PRE_KEY_STRUCT*)buffer_)[j].RCRD_ADDR())>>32);

        out_p += sizeof(uint32_t);
        *(LEN_TYPE*)(out_b+out_p) = ((PRE_KEY_STRUCT*)buffer_)[j].LEN();

        out_p += sizeof(LEN_TYPE);
        ++count_;
      }
    }

    if (out_p>0)
      IASSERT(fwrite(out_b, out_p, 1, f)==1);
    fclose(key_f_);
    fclose(f);

    boost::filesystem::remove(filenm_+".key");
    boost::filesystem::rename(filenm_+".key.out", filenm_+".key");
    key_f_ = fopen((filenm_+".key").c_str(), "r");
    IASSERT(key_f_!= NULL);
    //std::cout<<"\n-Count: "<<count_<<std::endl;
  }
  
  void output_()
  {
    IASSERT(t_check_sort_());

    shrink_key_();

    const uint32_t KEY_SIZE = 1/*flag 1: if address is larger than 2^32*/+sizeof(uint32_t)/*for address*/+sizeof(LEN_TYPE);
    const uint32_t DATA_BUF_SIZE = buf_size_/2/KEY_SIZE*KEY_SIZE;
    const uint32_t KEY_BUF_SIZE = (buf_size_ - DATA_BUF_SIZE)/KEY_SIZE*KEY_SIZE;
    char* data_buf = buffer_+KEY_BUF_SIZE;
    
    fseek(key_f_, 0, SEEK_END);
    const uint64_t KEY_FILE_SIZE = ftell(key_f_);
    IASSERT(KEY_FILE_SIZE%KEY_SIZE==0);
    
    const uint32_t KEY_LOAD_TIMES = KEY_FILE_SIZE%KEY_BUF_SIZE==0? KEY_FILE_SIZE/KEY_BUF_SIZE:KEY_FILE_SIZE/KEY_BUF_SIZE+1;

    fseek(original_f_, 0, SEEK_END);
    const uint64_t DATA_FILE_SIZE = ftell(original_f_);
    
    IASSERT(original_f_!=NULL);
    
    FILE* out_f = fopen((filenm_+".dat.out").c_str(), "w+");
    IASSERT(out_f!=NULL);

    uint64_t data_start_pos = 0;

    count_ = 0;
    fseek(original_f_, 0, SEEK_SET);
    while (data_start_pos<DATA_FILE_SIZE)
    {
      std::cout<<"\rOutputing... : "<<(double)data_start_pos/DATA_FILE_SIZE*100.<<"%"<<std::flush;
      //iteratly loading original data onto data buffer
      uint32_t s = DATA_FILE_SIZE-data_start_pos;
      if (s > DATA_BUF_SIZE)
        s = DATA_BUF_SIZE;
      uint64_t data_end_pos = data_start_pos + s;

      fseek(original_f_, data_start_pos, SEEK_SET);
      IASSERT(fread(data_buf, s, 1, original_f_)==1);
      
      fseek(key_f_, 0, SEEK_SET);
      fseek(out_f, 0, SEEK_SET);
      uint64_t seek_len = 0;
      //iterately get the keys
      for (uint32_t i=0; i<KEY_LOAD_TIMES; ++i)
      {
        uint32_t ss = KEY_FILE_SIZE - ftell(key_f_)>KEY_BUF_SIZE? KEY_BUF_SIZE: KEY_FILE_SIZE - ftell(key_f_);//loading size
        IASSERT(ss%KEY_SIZE==0);
        uint32_t nn = ss/KEY_SIZE;
        IASSERT(fread(buffer_, ss, 1, key_f_)==1);

        uint32_t p = 0;
        for (uint32_t j=0; j<nn; ++j)
        {
          uint8_t flag = *(buffer_+p);
          ++p;
          uint64_t addr = *(uint32_t*)(buffer_+p);
          p+= sizeof(uint32_t);
          LEN_TYPE len = *(LEN_TYPE*)(buffer_+p);
          p+= sizeof(LEN_TYPE);
          if (flag ==1)
          {
            addr <<=32;
            addr += (uint32_t)-1;
          }
          
          if (addr >= data_start_pos && addr < data_end_pos)
          {
            fseek(out_f, seek_len, SEEK_CUR);
            seek_len = 0;
            if (len+addr+sizeof(LEN_TYPE)>data_end_pos)
            {// if the data accross the buffer
              fseek(original_f_, addr, SEEK_SET);
              IASSERT(fread(&len, sizeof(LEN_TYPE), 1, original_f_)==1);
              
              char* b = new char[len];
              IASSERT(fread(b, len, 1, original_f_)==1);
              
              IASSERT(fwrite(&len, sizeof(LEN_TYPE), 1, out_f)==1);
              IASSERT(fwrite(b, len, 1, out_f)==1);
              ++count_;
              delete b;
              continue;
            }
            //std::cout<<"+"<<((PRE_KEY_STRUCT*)buffer_)[j].PRE_KEY()<<std::endl;

            ++count_;
            IASSERT(addr-data_start_pos+sizeof(LEN_TYPE)+len <= s);
            IASSERT(len == *(LEN_TYPE*)(data_buf+addr-data_start_pos));
            
            IASSERT(fwrite(data_buf+addr-data_start_pos, len+sizeof(LEN_TYPE), 1, out_f)==1);
            continue;
          }
          seek_len += len+sizeof(LEN_TYPE);
        }
      }

      data_start_pos = data_end_pos;
    }

    fclose(key_f_);
    fclose(out_f);
    fclose(original_f_);
  
    key_f_ = original_f_ = NULL;
    boost::filesystem::remove(filenm_+".dat");
    boost::filesystem::remove(filenm_+".key");
    boost::filesystem::remove(filenm_+".key.out");
    boost::filesystem::rename(filenm_+".dat.out", filenm_+".dat");

//     char* data;
//     LEN_TYPE len;
//     PRE_KEY_TYPE last = 0;

//     IASSERT(begin());
//     while (next_data(len, &data))
//     {
//       if (*(PRE_KEY_TYPE*)data-last!=1)
//         std::cout<<*(PRE_KEY_TYPE*)data<<"-"<<last<<std::endl;
//       IASSERT(*(PRE_KEY_TYPE*)data-last==1);
//       IASSERT(last<=*(PRE_KEY_TYPE*)data);
//       last = *(PRE_KEY_TYPE*)data;
//       free(data);
//     }
    //std::cout<<"\nCount: "<<count_<<std::endl;
  }

  void quick_sort_(int left, int right)
  {
    int i = left, j = right;
    PRE_KEY_STRUCT tmp;
    PRE_KEY_STRUCT pivot = ((PRE_KEY_STRUCT*)buffer_)[(left + right) / 2];
    
    /* partition */
    while (i <= j) {
      
      while (((PRE_KEY_STRUCT*)buffer_)[i].compare(pivot, original_f_)<0)
        i++;
      while (((PRE_KEY_STRUCT*)buffer_)[j].compare(pivot, original_f_)>0)
        j--;

      if (i <= j) {
        tmp = ((PRE_KEY_STRUCT*)buffer_)[i];
        ((PRE_KEY_STRUCT*)buffer_)[i] = ((PRE_KEY_STRUCT*)buffer_)[j];
        ((PRE_KEY_STRUCT*)buffer_)[j] = tmp;
        i++;
        j--;
      }
    };

    //IASSERT(i-1==j);
    /* recursion */
    if (left < j)
      quick_sort_(left, j);

    if (i < right)
      quick_sort_(i, right);

  }
  
  bool t_check_quick_sort_()
  {
    uint32_t s = pos_/sizeof(struct PRE_KEY_STRUCT)-1;
    
    for (uint32_t i=0; i<s-1; ++i)
      if (((PRE_KEY_STRUCT*)buffer_)[i].compare(((PRE_KEY_STRUCT*)buffer_)[i+1], original_f_)>0)
        //          || ((PRE_KEY_STRUCT*)buffer_)[i].PRE_KEY()+1!=((PRE_KEY_STRUCT*)buffer_)[i+1].PRE_KEY())
      {
        std::cout<<s<<"-"<<i<<std::endl;
        std::cout<<((PRE_KEY_STRUCT*)buffer_)[i]<<"--"<<((PRE_KEY_STRUCT*)buffer_)[i+1]<<std::endl;
        return false;
      }
    
    return true;
  }

  bool t_check_sort_()
  {
    fseek(key_f_, 0, SEEK_END);
    const uint64_t KEY_FILE_SIZE = ftell(key_f_);
    
    const uint32_t KEY_LOAD_TIMES = KEY_FILE_SIZE%buf_size_==0? KEY_FILE_SIZE/buf_size_: KEY_FILE_SIZE/buf_size_+1;

    fseek(key_f_, 0, SEEK_SET);
    for (uint32_t i=0; i<KEY_LOAD_TIMES; ++i)
    {
      uint32_t ss = KEY_FILE_SIZE - ftell(key_f_)>buf_size_? buf_size_: KEY_FILE_SIZE - ftell(key_f_);//loading size
      uint32_t nn = ss/sizeof(PRE_KEY_STRUCT);
      IASSERT(fread(buffer_, ss, 1, key_f_)==1);
      for (uint32_t j=0; j<nn-1; ++j)
      {
//         if (((PRE_KEY_STRUCT*)buffer_)[j].PRE_KEY()+1!=((PRE_KEY_STRUCT*)buffer_)[j+1].PRE_KEY())
//         {
//           std::cout<<"+"<<((PRE_KEY_STRUCT*)buffer_)[j].PRE_KEY()<<"-"<<((PRE_KEY_STRUCT*)buffer_)[j+1].PRE_KEY();
//           return false;
//         }
        if (((PRE_KEY_STRUCT*)buffer_)[j].compare(((PRE_KEY_STRUCT*)buffer_)[j+1], original_f_)>0)
        {
          std::cout<<((PRE_KEY_STRUCT*)buffer_)[j].PRE_KEY()<<"-"<<((PRE_KEY_STRUCT*)buffer_)[j+1].PRE_KEY()<<std::endl;
          return false;
        }
      }
    }

    IASSERT((uint64_t)ftell(key_f_)==KEY_FILE_SIZE);
    return true;
  }
  
public:
  MultiPassSort(const char* filenm, uint32_t buf_size = 100000000, uint32_t group_size=4)
    :filenm_(filenm), buf_size_(buf_size), buffer_(NULL), MAX_GROUP_SIZE(group_size)
  {
    tmp_buffer_ = buffer_ =  NULL;
    original_f_= key_f_ = NULL;
    pos_ = 0;

    buf_size_ = buf_size_/sizeof(struct PRE_KEY_STRUCT)*sizeof(struct PRE_KEY_STRUCT);
    end_pos_ = 0;
    count_ = 0;

    thre_num_ = 1;//100;
    assert(buf_size_>12);
    assert(MAX_GROUP_SIZE>1);
  }

  ~MultiPassSort()
  {
    if (buffer_ != NULL)
      free(buffer_);
  }

  void set_buffer_size(uint32_t size)
  {
    buf_size_ = size;
     buf_size_ = buf_size_/sizeof(struct PRE_KEY_STRUCT)*sizeof(struct PRE_KEY_STRUCT);
  }

  void set_threads_num(uint32_t num)
  {
    thre_num_ = num;
  }
  
  void add_data(LEN_TYPE len, const char* data)
  {
    if (len == 0)
      return;
    
    new_buffer_();
    if (is_buffer_full_(len))
      flush_buffer_();

    if (original_f_ == NULL)
    {
      original_f_ = fopen((filenm_+".dat").c_str(), "w+");
      IASSERT(original_f_!=NULL);
    }

    *(PRE_KEY_STRUCT*)(buffer_+pos_) = PRE_KEY_STRUCT(*(PRE_KEY_TYPE*)data,
                                                      end_pos_+tmp_pos_, len);
    
    pos_ += sizeof(PRE_KEY_STRUCT);
                                                      
    *(LEN_TYPE*)(tmp_buffer_+tmp_pos_) = len;
    tmp_pos_ += sizeof(LEN_TYPE);
    memcpy(tmp_buffer_+tmp_pos_, data, len);
    tmp_pos_ += len;

    ++count_;
  }

  void flush()
  {
    flush_buffer_();
    fflush(key_f_);
  }
  
  void sort()
  {
    if (count_ == 0)
      return;
    
    struct timeval tvafter, tvpre;
    struct timezone tz;

    uint32_t seek_times = 0;
    threads_t threads;
    mutexs_t mutexs;
    threads.reserve(thre_num_);
    for (uint32_t i=0; i<thre_num_; ++i)
    {
      threads.push_back(NULL);
      mutexs.push_back(new boost::mutex());
    }
    
    flush_buffer_();
    fflush(key_f_);
    
    fseek(key_f_, 0, SEEK_END);
    const uint64_t FILE_SIZE = ftell(key_f_);
    if (FILE_SIZE==0)
      return;
    IASSERT(count_*sizeof(struct PRE_KEY_STRUCT) == FILE_SIZE);
    IASSERT(FILE_SIZE%sizeof(struct PRE_KEY_STRUCT)==0);

    std::cout<<"\nStart to sort "<<count_<<" items.\n";
    count_ = 0;

    if (FILE_SIZE<=buf_size_)
    {
      std::cout<<"\nNo merging.\n";
      fseek(key_f_, 0, SEEK_SET);
      IASSERT(fread(buffer_, FILE_SIZE, 1, key_f_)==1);
      quick_sort_(0, FILE_SIZE/sizeof(struct PRE_KEY_STRUCT)-1);
      fseek(key_f_, 0, SEEK_SET);
      IASSERT(fwrite(buffer_, FILE_SIZE, 1, key_f_)==1);
      
      gettimeofday (&tvpre , &tz);
      output_();
      gettimeofday (&tvafter , &tz);
      std::cout<<"\nIt takes "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000
               <<" minutes to output";
      return;
    }
    
    uint64_t file_pos[MAX_GROUP_SIZE];
    uint64_t file_end_pos[MAX_GROUP_SIZE];
    uint32_t buf_idx[MAX_GROUP_SIZE+1];//the last one is for output
    uint32_t buf_end_idx[MAX_GROUP_SIZE];//the last one is for output
    uint32_t buf_start_pos[MAX_GROUP_SIZE+1];//the last one is for output
    double   _times = (log((double)FILE_SIZE/buf_size_)/log((double)MAX_GROUP_SIZE));
    if ((double)((uint32_t)_times) != _times)
      _times += 1;
    const uint32_t TIMES = (uint32_t)_times;
    
    uint32_t key_out_buf_pos[thre_num_];

    gettimeofday (&tvpre , &tz);

    count_ = 0;
    uint32_t times = 0;
    while (times < TIMES)
    {
      FILE* key_out_f = fopen((filenm_+".key.out").c_str(), "w+");
      IASSERT(key_out_f!=NULL);
    
      const uint32_t CHUNK_SIZE = buf_size_/sizeof(struct PRE_KEY_STRUCT)*(uint32_t)pow((float)MAX_GROUP_SIZE, (float)times);
      uint64_t start = 0;
      while (start<FILE_SIZE)
      {
        std::cout<<"\r"<<((double)times/TIMES+1./TIMES*start/FILE_SIZE)*100.<<"%"<<std::flush;

        double gs = ((double)(FILE_SIZE-start))/(CHUNK_SIZE*sizeof(struct PRE_KEY_STRUCT));
        if ((double)((uint32_t)gs) != gs)
          gs += 1;
        uint32_t GROUP_SIZE = (uint32_t)gs;
        if (GROUP_SIZE > MAX_GROUP_SIZE)
          GROUP_SIZE = MAX_GROUP_SIZE;
        IASSERT(GROUP_SIZE>=1);
      
        const uint32_t DATA_NUM_IN_BUF = ((buf_size_/(GROUP_SIZE+1))/sizeof(struct PRE_KEY_STRUCT));//data number that chunk buffer can contain
        const uint32_t CHUNK_BUF_SIZE = sizeof(struct PRE_KEY_STRUCT) * DATA_NUM_IN_BUF;
        IASSERT(DATA_NUM_IN_BUF>=1);
        IASSERT(DATA_NUM_IN_BUF/thre_num_>=1);
      
        //get start and end position in file of every chunk
        for (uint32_t t=0; t<GROUP_SIZE; ++t)
        {
          file_pos[t] = start + t*CHUNK_SIZE*sizeof(struct PRE_KEY_STRUCT);
          file_end_pos[t] = file_pos[t] + CHUNK_SIZE*sizeof(struct PRE_KEY_STRUCT);
          if (file_end_pos[t]>FILE_SIZE)
            file_end_pos[t] = FILE_SIZE;
        }
        start = file_end_pos[GROUP_SIZE-1];
    
        //get start position within buffer of every chunk
        for (uint32_t t=0; t<GROUP_SIZE+1; ++t)
          buf_start_pos[t] = t*CHUNK_BUF_SIZE;

        IASSERT(DATA_NUM_IN_BUF/thre_num_>1);
        for (uint32_t t=0; t<thre_num_; ++t)
          key_out_buf_pos[t] = buf_start_pos[GROUP_SIZE] + t*DATA_NUM_IN_BUF/thre_num_*sizeof(struct PRE_KEY_STRUCT);;

        //initiate every chunk buffer
        for (uint32_t t=0; t<GROUP_SIZE; ++t)
        {
          buf_end_idx[t] = buf_idx[t] = 0;
          assert(file_pos[t]%sizeof(struct PRE_KEY_STRUCT)==0);
          uint32_t s = CHUNK_BUF_SIZE;
          
          if (file_pos[t] + s >= FILE_SIZE)
            s = FILE_SIZE - file_pos[t];
          if (s == 0)
            break;
          
          assert(s%sizeof(struct PRE_KEY_STRUCT)==0);

          ++seek_times;
          fseek(key_f_, file_pos[t], SEEK_SET);
          IASSERT(fread(buffer_+buf_start_pos[t], s, 1, key_f_)==1);
          file_pos[t] += s;

          IASSERT(s%sizeof(struct PRE_KEY_STRUCT)==0);
          buf_end_idx[t] = s/sizeof(struct PRE_KEY_STRUCT);
        }
        buf_end_idx[GROUP_SIZE] = DATA_NUM_IN_BUF;
        buf_idx[GROUP_SIZE] = 0;

        for (uint32_t g=0; g<threads.length(); ++g)
          threads[g] = NULL;
        
        uint32_t min_i = 0;
        PRE_KEY_STRUCT min;
        uint32_t sorted_ch = 0;
        uint32_t threads_i = 0;
        //std::cout<<GROUP_SIZE<<" Merging ...*******************\n";
        //merge group
        while (sorted_ch < GROUP_SIZE)
        {
          //std::cout<<sorted_ch<<"+"<<GROUP_SIZE<<std::endl;
          uint32_t t=0;
          //get the first one
          for (; t<GROUP_SIZE; ++t)
          {
            if (buf_idx[t]>=buf_end_idx[t])
              continue;

            min_i = t;
            min = ((PRE_KEY_STRUCT*)(buffer_+buf_start_pos[t]))[buf_idx[t]];
            ++t;
            break;
          }
          IASSERT(t<=GROUP_SIZE);

          //find the minimun
          for (; t<GROUP_SIZE; ++t)
          {
            IASSERT(buf_idx[t]<DATA_NUM_IN_BUF);
            if (buf_idx[t]>=buf_end_idx[t]|| min.compare(((PRE_KEY_STRUCT*)(buffer_+buf_start_pos[t]))[buf_idx[t]], original_f_)<=0)
              continue;
          
            min = ((PRE_KEY_STRUCT*)(buffer_+buf_start_pos[t]))[buf_idx[t]];
            min_i = t;
          }
          
          //assign it to output buffer
          if(buf_idx[GROUP_SIZE]==0)
          {
            mutexs.at(threads_i)->lock();
            mutexs.at(threads_i)->unlock();
          }
                    
          ((PRE_KEY_STRUCT*)(buffer_+key_out_buf_pos[threads_i]))[buf_idx[GROUP_SIZE]] = min;
          ++count_;
          //std::cout<<min<<std::endl;

          ++(buf_idx[GROUP_SIZE]);
          if (buf_idx[GROUP_SIZE]>=DATA_NUM_IN_BUF/thre_num_)
          {
            //output buffer full
            mutexs.at(threads_i)->lock();
            threads[threads_i] = new boost::thread(boost::bind(&self_t::output_keys_, this, key_out_f,
                                                            buffer_+key_out_buf_pos[threads_i],
                                                               buf_idx[GROUP_SIZE]*sizeof(PRE_KEY_STRUCT),
                                                               (threads_i==0? threads[thre_num_-1]: threads[threads_i-1]),
                                                               mutexs.at(threads_i)));
            ++threads_i;
            if (threads_i==thre_num_)
              threads_i = 0;
            buf_idx[GROUP_SIZE] = 0;
          }
        
          ++(buf_idx[min_i]);
          //load input if its buffer is empty
          if (buf_idx[min_i] == buf_end_idx[min_i])
          {
            buf_idx[min_i] = buf_end_idx[min_i] = 0;
            
            uint32_t s = CHUNK_BUF_SIZE;
            if (file_pos[min_i]+s >= file_end_pos[min_i])
              s = file_end_pos[min_i] - file_pos[min_i];
            //std::cout<<"s-"<<s<<std::endl;
            if (s == 0)
            {
              //std::cout<<"sorted ---------------------"<<min_i<<std::endl;
              ++sorted_ch;
              continue;
            }

            assert(s%sizeof(struct PRE_KEY_STRUCT)==0);

            ++seek_times;
            fseek(key_f_, file_pos[min_i], SEEK_SET);
            IASSERT(fread(buffer_+buf_start_pos[min_i], s, 1, key_f_)==1);
            file_pos[min_i] += s;
            IASSERT(file_end_pos[min_i]>= file_pos[min_i]);
            
            buf_end_idx[min_i] = s/sizeof(struct PRE_KEY_STRUCT);
            IASSERT(buf_end_idx[min_i]<=DATA_NUM_IN_BUF);
          }
        }
      
        if (buf_idx[GROUP_SIZE]!=0)
        {
          IASSERT(buf_idx[GROUP_SIZE]<DATA_NUM_IN_BUF/thre_num_);
          mutexs.at(threads_i)->lock();
          output_keys_(key_out_f, buffer_+key_out_buf_pos[threads_i],
                       buf_idx[GROUP_SIZE]*sizeof(PRE_KEY_STRUCT),
                       (threads_i==0? threads[thre_num_-1]: threads[threads_i-1]),
                       mutexs.at(threads_i));
          
          buf_idx[GROUP_SIZE] = 0;
        }
      }

      ++times;
      
      fseek(key_f_, 0, SEEK_END);
      fseek(key_out_f, 0, SEEK_END);
      IASSERT(ftell(key_f_) == ftell(key_out_f));
      IASSERT(FILE_SIZE == (uint64_t)ftell(key_out_f));
      
      fclose(key_f_);
      fclose(key_out_f);
      boost::filesystem::remove(filenm_+".key");
      boost::filesystem::rename(filenm_+".key.out", filenm_+".key");
      key_f_ = fopen((filenm_+".key").c_str(), "r");
      IASSERT(key_f_!= NULL);
    }

    gettimeofday (&tvafter , &tz);
    std::cout<<"\nIt takes "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000
             <<" minutes to merge("<<count_/TIMES<<")\n";
    
    std::cout<<"\nSorting is over, seeking times is "<<seek_times<<", begin to output ...\n";
    
    gettimeofday (&tvpre , &tz);
    output_();
    gettimeofday (&tvafter , &tz);
    std::cout<<"\nIt takes "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000
             <<" minutes to output";

    for (uint32_t g=0; g<mutexs.length(); ++g)
      delete mutexs.at(g);

  }

  void output()
  {
    output_();
  }

  void clear_files()
  {
    if (original_f_)
    {
      fclose(original_f_);
      original_f_ = NULL;
    }
    if (key_f_)
    {
      fclose(key_f_);
      key_f_ = NULL;
    }

    if (boost::filesystem::exists(filenm_+".dat"))
      boost::filesystem::remove(filenm_+".dat");

    if (boost::filesystem::exists(filenm_+".dat.out"))
      boost::filesystem::remove(filenm_+".dat.out");

    if (boost::filesystem::exists(filenm_+".key"))
      boost::filesystem::remove(filenm_+".key");

    if (boost::filesystem::exists(filenm_+".key.out"))
      boost::filesystem::remove(filenm_+".key.out");
  }
  
  bool begin()
  {
    if (buffer_==NULL)
    {
      buffer_ = (char*)malloc(buf_size_);
      pos_ = 0;
    }

    if (original_f_ == NULL)
    {
      original_f_ = fopen((filenm_+".dat").c_str(), "r");
      if(original_f_==NULL)
        return false;
    }
    fseek(original_f_, 0, SEEK_SET);
    
    return true;
  }

  bool next_data(LEN_TYPE& len, char** data)
  {
    if (!original_f_)
      return false;
    
    if (fread(&len, sizeof(LEN_TYPE), 1, original_f_)!=1)
    {
      fclose(original_f_);
      original_f_ = NULL;
      return false;
    }

    *data = (char*)malloc(len);
    IASSERT(fread(*data, len, 1, original_f_)==1);
    
    return true;
  }
}
  ;

NS_IZENELIB_AM_END

#endif
