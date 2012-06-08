/**
   @file multi_pass_sort.hpp
   @author Kevin Hu
   @date 2010.01.20
 */
#ifndef SORT_MERGER_HPP
#define SORT_MERGER_HPP

#include <util/izene_log.h>
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
#include <queue>

NS_IZENELIB_AM_BEGIN

/**
   @class MultiPassSort
 **/
template<
  class KEY_TYPE = uint32_t,//pre-key type, indicate the length of the pre-key.
  class LEN_TYPE = uint8_t,//
  bool  COMPARE_ALL = false,
  typename IO_TYPE = DirectIO
>
class SortMerger
{
typedef SortMerger<KEY_TYPE, LEN_TYPE, COMPARE_ALL, IO_TYPE> self_t;

  struct KEY_ADDR;
  std::string filenm_;
  const uint32_t MAX_GROUP_SIZE_;//!< max group size
  const uint32_t BS_SIZE_;//!< in fact it equals to memory size
  uint32_t PRE_BUF_SIZE_;//!< max predict buffer size
  uint32_t RUN_BUF_SIZE_;//!< max run buffer size
  uint32_t OUT_BUF_SIZE_;//!< max size of output buffer
  const uint32_t OUT_BUF_NUM_;//!< output threads number

  std::priority_queue<struct KEY_ADDR> pre_heap_;//!<predict heap
  std::priority_queue<struct KEY_ADDR> merge_heap_;//!< merge heap

  uint32_t* micro_run_idx_;//!< the access index of each microruns
  uint32_t* micro_run_pos_;//!< the access position within each microruns
  uint32_t* num_micro_run_;//!< the records number of each microruns
  uint32_t* size_micro_run_;//!< the size of entire microrun
  uint32_t* num_run_;//!< records number of each runs
  uint32_t* size_run_;//!< size of each entire runs
  uint32_t* size_loaded_run_;//!<size of data that have been read within each entire runs 
  uint64_t* run_addr_;//!< start file address of each runs
  uint64_t* run_curr_addr_;//!<current file address of each runs

  char** micro_buf_;//!< address of every microrun channel buffer
  char** sub_out_buf_;//!< addresses of each output buffer
  
  char* pre_buf_;//!< predict buffer
  char* run_buf_;//!< entire run buffer
  char* out_buf_;//!< the entire output buffer

  boost::mutex pre_buf_mtx_;//!< mutex and condition for predict buffer accessed by merge and predict thread
  boost::condition pre_buf_con_;
  
  boost::mutex* in_out_mtx_;//!<mutex and condition for output buffers accessed by output threads and merge threads
  boost::condition* in_out_con_;//!<
  
  boost::mutex out_out_mtx_;//!< mutex and condition to ensure the seqence of file writing of all the output threads
  boost::condition out_out_con_;

  uint32_t pre_buf_size_;//!< the current size of microrun that has been loaded onto prediect buffer
  uint32_t pre_buf_num_;//!< the current records number of microrun that has been loaded onto prediect buffer
  //uint32_t pre_idx_;//!< the index of microrun channel right in the predict buffer
  uint32_t out_buf_in_idx_;//!< used by merge to get the current available output buffer
  uint32_t out_buf_out_idx_;//!< used by output threads to get the index of the turn of outputting
  uint32_t* out_buf_size_;//!< data size of each output buffer
  bool* out_buf_full_;//!< a flag to ensure if the output buffer is full or not
  

  uint64_t count_;//!< records number
  uint32_t group_size_;//!< the real run number that can get from the input file.

  uint64_t FILE_LEN_;  
    /**
     @brief pre-key used for sorting
   */
  struct KEY_ADDR
  {
    char* data;
    uint64_t addr;
    uint32_t idx;

    KEY_TYPE& KEY_()
    {
      return *(KEY_TYPE*)(data+sizeof(LEN_TYPE));
    }

    KEY_TYPE KEY()const
    {
      return *(KEY_TYPE*)(data+sizeof(LEN_TYPE));
    }    

    LEN_TYPE LEN()const 
    {
      return *(LEN_TYPE*)data;
    }

    LEN_TYPE& LEN_()const 
    {
      return *(LEN_TYPE*)data;
    }
    
    uint64_t& ADDR_()
    {
      return addr;
    }

    uint64_t ADDR()const
    {
      return addr;
    }    

    uint32_t IDX()const
    {
      return idx;
    }

    uint32_t& IDX_()
    {
      return idx;
    }
    
    inline KEY_ADDR(char* p, uint64_t ad, uint32_t i)
    {
      data = p;
      addr = ad;
      idx = i;
    }

    inline KEY_ADDR()
    {
      data = NULL;
      addr = -1;
      idx = -1;
    }

    KEY_ADDR& operator = (const KEY_ADDR& other)
    {
      data = other.data;
      addr = other.addr;
      idx = other.IDX();
      
      return *this;
    }
    
    int compare(const KEY_ADDR& p)const
    {
      if (KEY() == p.KEY() && !COMPARE_ALL)
        return 0;

      if (KEY()> p.KEY())
        return 1;
      if (KEY()< p.KEY())
        return -1;

      if (!COMPARE_ALL)
        return 0;
      
      LEN_TYPE len1 = LEN()/sizeof(KEY_TYPE);
      LEN_TYPE len2 = p.LEN()/sizeof(KEY_TYPE);

      for (LEN_TYPE i=1; i<len1 && i<len2; ++i)
      {
        if (((KEY_TYPE*)(data+sizeof(LEN_TYPE)))[i]>((KEY_TYPE*)(p.data+sizeof(LEN_TYPE)))[i])
          return 1;
        if (((KEY_TYPE*)(data+sizeof(LEN_TYPE)))[i]<((KEY_TYPE*)(p.data+sizeof(LEN_TYPE)))[i])
          return -1;
      }

      if (len1 == len2)
        return 0;

      if (len1>len2)
        return 1;
      return -1;
      
    }

    bool operator == (const KEY_ADDR& other)const
    {
      return compare(other)==0;
    }

    bool operator > (const KEY_ADDR& other)const
    {
      return compare(other)<0;
    }

    bool operator < (const KEY_ADDR& other)const
    {
      return compare(other)>0;
    }
    
  };

  
  inline void new_buffer_()
  {
    if (!pre_buf_)
      pre_buf_ = (char*)malloc(PRE_BUF_SIZE_);

    if (!run_buf_)
      run_buf_ = (char*)malloc(RUN_BUF_SIZE_);

    if (!out_buf_)
      out_buf_ = (char*)malloc(OUT_BUF_SIZE_);
  }
  
  inline void init_(IO_TYPE& ioStream)
  {
    //initialize three buffers
    new_buffer_();

    //initiate output buffers
    out_buf_size_[0] = 0;
    sub_out_buf_[0] = out_buf_;
    out_buf_full_[0] = false;
    for (uint32_t i = 1; i<OUT_BUF_NUM_; ++i)
    {
      sub_out_buf_[i] = sub_out_buf_[i-1]+ OUT_BUF_SIZE_/OUT_BUF_NUM_;
      out_buf_size_[i] = 0;
      out_buf_full_[i] = false;
    }
    out_buf_in_idx_ = 0;
    out_buf_out_idx_ = 0;


    //initiate the microrun buffer
    micro_buf_[0] = run_buf_;
    for (uint32_t i = 1; i<MAX_GROUP_SIZE_; ++i)
      micro_buf_[i] = micro_buf_[i-1]+PRE_BUF_SIZE_;

    //
    group_size_ = 0;
    uint64_t nextRunPos = 0;
    for (uint32_t i = 0; i<MAX_GROUP_SIZE_ && (uint64_t)ioStream.tell()<FILE_LEN_; ++i, ++group_size_)
    {
      //get the size of run
      //IASSERT(fread(size_run_+i, sizeof(uint32_t), 1, f)==1);
      ioStream.readBytes((char*)(size_run_+i),sizeof(uint32_t));
      //get the records number of a run
      //IASSERT(fread(num_run_+i, sizeof(uint32_t), 1, f)==1);
      ioStream.readBytes((char*)(num_run_+i),sizeof(uint32_t));
      //IASSERT(fread(&nextRunPos, sizeof(uint64_t), 1, f)==1);
      ioStream.readBytes((char*)(&nextRunPos),sizeof(uint64_t));

      run_addr_[i] = ioStream.tell();//ftell(f);

      //loading size of a microrun
      uint32_t s = size_run_[i]>PRE_BUF_SIZE_? PRE_BUF_SIZE_:size_run_[i];
      //IASSERT(fread(micro_buf_[i], s, 1, f)==1);
      size_t ret = ioStream.read(micro_buf_[i], s);
      size_micro_run_[i] = ret;
      size_loaded_run_[i] = ret;
      run_curr_addr_[i] = ioStream.tell();

      if(!ioStream.isCompression())
      {
        ///it is not needed for compression, validation will be made within IOStream in that case
        //if a record can fit in microrun buffer
        bool flag = false;
        while (*(LEN_TYPE*)(micro_buf_[i])+sizeof(LEN_TYPE) > s)
        {
          size_micro_run_[i] = 0;
          --count_;
          LOG(WARNING) << "[Warning]: A record is too long, it will be ignored";
          ioStream.seek(*(LEN_TYPE*)(micro_buf_[i])+sizeof(LEN_TYPE)-s, SEEK_CUR);

          if (ioStream.tell()-run_addr_[i] >= (uint64_t)size_run_[i])
          {
            flag = true;
            break;
          }
        
          s = (uint32_t)((uint64_t)size_run_[i]-(ioStream.tell()-run_addr_[i])>PRE_BUF_SIZE_? PRE_BUF_SIZE_:(uint64_t)size_run_[i]-(ioStream.tell()-run_addr_[i]));
          size_micro_run_[i] = s;
          //IASSERT(fread(micro_buf_[i], s, 1, f)==1);
          ioStream.read(micro_buf_[i], s);
        }
        if (flag)
          continue;
      }
      merge_heap_.push(KEY_ADDR(micro_buf_[i], -1, i));
      micro_run_idx_[i] = 1;
      micro_run_pos_[i] = KEY_ADDR(micro_buf_[i], -1, i).LEN()+sizeof(LEN_TYPE);
      num_micro_run_[i] = 0;

      //if size_run_[i]>PRE_BUF_SIZE_, it needs to the end of a run and turn to the next run
      //fseek(f, size_run_[i]-s, SEEK_CUR);
      //fseek(f, nextRunPos, SEEK_SET);
      ioStream.seek(nextRunPos);
    }
    LOG(INFO) << "Run number: " << group_size_;
    
    //initialize predict heap and records number of every microrun
    for (uint32_t i = 0; i<group_size_; ++i)
    {
      uint32_t pos = 0;
      uint32_t last_pos = -1;
      assert(i < MAX_GROUP_SIZE_);
      while (size_micro_run_[i])
      {
        LEN_TYPE len = *(LEN_TYPE*)(micro_buf_[i]+pos);
        if(pos+ sizeof(LEN_TYPE)+ len > size_micro_run_[i])
        {
          IASSERT(last_pos != (uint32_t)-1);//buffer too small that can't hold one record
          
          len = *(LEN_TYPE*)(micro_buf_[i]+last_pos)+sizeof(LEN_TYPE);
          char* tmp = (char*)malloc(len);
          memcpy(tmp, micro_buf_[i]+last_pos, len);

          if(ioStream.isCompression())		  
            pre_heap_.push(KEY_ADDR(tmp, run_curr_addr_[i], i));
          else
          {
            pre_heap_.push(KEY_ADDR(tmp, run_addr_[i]+pos, i));
            //
            //std::cout<<i<<":"<<KEY_ADDR(tmp, run_addr_[i]+pos, i).KEY()<<std::endl;
            //std::cout<<i<<":"<<run_addr_[i]+pos<<":"<<run_curr_addr_[i]<<std::endl;
            size_micro_run_[i] = pos;
          }
          break;
        }
        
        num_micro_run_[i]++;
        last_pos = pos;
        pos += sizeof(LEN_TYPE)+len;
      }
    }
    //pre_idx_ = -1;
  }

  void predict_(IO_TYPE& ioStream)
  {
    while(pre_heap_.size()>0)
    {
      KEY_ADDR top = pre_heap_.top();
      pre_heap_.pop();
      uint64_t addr = top.ADDR();
      uint32_t idx = top.IDX();
      //std::cout<<top.KEY()<<std::endl;
      free(top.data);

      boost::mutex::scoped_lock lock(pre_buf_mtx_);

      while (pre_buf_size_!=0)
        pre_buf_con_.wait(lock);

      assert(idx < MAX_GROUP_SIZE_);
      //get loading size of a microrun
      uint32_t s;
      if(ioStream.isCompression())
	 s = (uint32_t)((uint64_t)size_run_[idx] - size_loaded_run_[idx]);        
      else
	 s = (uint32_t)((uint64_t)size_run_[idx] - (addr - run_addr_[idx]));


      if (s == 0)
      {
        //std::cout<<"==================\n";
        continue;
      }
      s = s > PRE_BUF_SIZE_? PRE_BUF_SIZE_:s;

      //load microrun
      //fseek(f, addr, SEEK_SET);
      ioStream.seek(addr);
      //IASSERT(fread(pre_buf_, s, 1, f)==1);
      s = ioStream.read(pre_buf_, s);
      size_loaded_run_[idx] += s;
      run_curr_addr_[idx] = ioStream.tell();;

      uint32_t pos = 0;
      uint32_t last_pos = -1;
      pre_buf_num_ = 0;
      while (1)
      {
        LEN_TYPE len = *(LEN_TYPE*)(pre_buf_+pos);
        if(pos+sizeof(LEN_TYPE)+len > s)
        {
          //the last record of this microrun
          IASSERT(last_pos != (uint32_t)-1);//buffer too small that can't hold one record
          len = *(LEN_TYPE*)(pre_buf_+last_pos)+sizeof(LEN_TYPE);
          char* tmp = (char*)malloc(len);
          memcpy(tmp, pre_buf_+last_pos, len);
          if(ioStream.isCompression())
              pre_heap_.push(KEY_ADDR(tmp, run_curr_addr_[idx], idx));
          else
              pre_heap_.push(KEY_ADDR(tmp, addr+(uint64_t)pos, idx));
          break;
        }

        ++pre_buf_num_;
        last_pos = pos;
        pos += sizeof(LEN_TYPE)+len;
      }
      pre_buf_size_ = pos;
      //pre_idx_ = idx;
      pre_buf_con_.notify_one();
    }
    {
      boost::mutex::scoped_lock lock(pre_buf_mtx_);
      pre_buf_size_ = -1;
      //pre_idx_ = -1;
      pre_buf_con_.notify_one();
    }

    LOG(INFO) << "Predicting is over...";
  }

  void merge_()
  {
    while(merge_heap_.size()>0)
    {
      KEY_ADDR top = merge_heap_.top();
      merge_heap_.pop();
      uint32_t idx = top.IDX();

      //output
      while(1)
      {
        assert(out_buf_in_idx_ < OUT_BUF_NUM_);
        boost::mutex::scoped_lock lock(in_out_mtx_[out_buf_in_idx_]);
        while (out_buf_full_[out_buf_in_idx_])
          in_out_con_[out_buf_in_idx_].wait(lock);

        //if buffer is full
        if (top.LEN()+sizeof(LEN_TYPE)+out_buf_size_[out_buf_in_idx_]>OUT_BUF_SIZE_/OUT_BUF_NUM_)
        {
          IASSERT(out_buf_size_[out_buf_in_idx_]!=0);//output buffer chanel is smaller than size of a record
          out_buf_full_[out_buf_in_idx_] = true;
          uint32_t tmp = out_buf_in_idx_;
          ++out_buf_in_idx_;
          out_buf_in_idx_ %= OUT_BUF_NUM_;
          in_out_con_[tmp].notify_one();
          //std::cout<<idx<<"-"<<out_buf_size_[tmp]<<std::endl;
          continue; 
        }

        assert(out_buf_in_idx_ < OUT_BUF_NUM_);
        memcpy(sub_out_buf_[out_buf_in_idx_]+out_buf_size_[out_buf_in_idx_],
                 top.data, top.LEN()+sizeof(LEN_TYPE));
        out_buf_size_[out_buf_in_idx_] += top.LEN()+sizeof(LEN_TYPE);

        //std::cout<<idx<<":"<<out_buf_size_[out_buf_in_idx_]<<std::endl;
        break;
      }

      assert(idx < MAX_GROUP_SIZE_);
      //reach the end of a microrun
      if (micro_run_idx_[idx] == num_micro_run_[idx])
      {
        IASSERT(micro_run_pos_[idx]<=size_micro_run_[idx]);
        boost::mutex::scoped_lock lock(pre_buf_mtx_);
        while (pre_buf_size_==0)
          pre_buf_con_.wait(lock);

        if (pre_buf_size_ == (uint32_t)-1)
          continue;
        // if (pre_idx_ != idx)
//         {
//           std::cout<<pre_idx_<<"="<<idx<<std::endl;
//           continue;
//         }

        assert(idx < MAX_GROUP_SIZE_);
        memcpy(micro_buf_[idx], pre_buf_, pre_buf_size_);
        size_micro_run_[idx] = pre_buf_size_;
        num_micro_run_[idx] = pre_buf_num_;
        micro_run_pos_[idx] = micro_run_idx_[idx] = pre_buf_num_ = pre_buf_size_ = 0;
        //pre_idx_ = -1;
        pre_buf_con_.notify_one();
      }

      assert(idx < MAX_GROUP_SIZE_);
      merge_heap_.push(KEY_ADDR(micro_buf_[idx]+micro_run_pos_[idx], -1, idx));
      ++micro_run_idx_[idx];
      micro_run_pos_[idx] += KEY_ADDR(micro_buf_[idx]+micro_run_pos_[idx], -1, idx).LEN()+sizeof(LEN_TYPE);
    }
    { 
      assert(out_buf_in_idx_ < OUT_BUF_NUM_);
      boost::mutex::scoped_lock lock(in_out_mtx_[out_buf_in_idx_]);
      if (!out_buf_full_[out_buf_in_idx_]
          &&out_buf_size_[out_buf_in_idx_]>0)
      {
        out_buf_full_[out_buf_in_idx_] = true;
        in_out_con_[out_buf_in_idx_].notify_one();
      }
    }
    

    LOG(INFO) << "Merging is over...";
  }

  void output_(FILE* f, uint32_t idx)
  {
    IO_TYPE ioStream(f,"w");

    const uint64_t c = count_;
    while (count_ > 0)
    {
      //wait its turn to output
      boost::mutex::scoped_lock out_lock(out_out_mtx_);
      while (out_buf_out_idx_ != idx)
        out_out_con_.wait(out_lock);

      if (count_ == 0)
      {
        ++out_buf_out_idx_;
        out_buf_out_idx_ %=OUT_BUF_NUM_;
        out_out_con_.notify_all();
        break;
      }
      
      assert(idx < OUT_BUF_NUM_);
      boost::mutex::scoped_lock in_lock(in_out_mtx_[idx]);
      while (!out_buf_full_[idx])
        in_out_con_[idx].wait(in_lock);

      //std::cout<<"\rMerging ..."<<1.*(c-count_)/c*100<<"%"<<std::flush;
      LOG(INFO) << "Merging ..." << 1.*(c-count_)/c*100 << "%";
      
      IASSERT(out_buf_size_[idx]!=0);
      assert(idx < OUT_BUF_NUM_);
      for (uint32_t pos=0; pos<out_buf_size_[idx];
           --count_, pos+=*(LEN_TYPE*)(sub_out_buf_[idx]+pos)+sizeof(LEN_TYPE));

      IASSERT(out_buf_size_[idx]<=OUT_BUF_SIZE_/OUT_BUF_NUM_);
      //IASSERT(fwrite(sub_out_buf_[idx], out_buf_size_[idx], 1, f)==1);
      ioStream.write(sub_out_buf_[idx], out_buf_size_[idx]);
      out_buf_full_[idx] = false;
      out_buf_size_[idx] = 0;
      ++out_buf_out_idx_;
      out_buf_out_idx_ %=OUT_BUF_NUM_;

      out_out_con_.notify_all();
      in_out_con_[idx].notify_one();
    }

    LOG(INFO) << "Outputting is over...";
  }

  bool t_check_sort_()
  {    
    return true;
  }
  
public:
  SortMerger(const char* filenm, uint32_t group_size = 4, uint32_t bs=100000000, uint32_t output_num = 2)
    :filenm_(filenm), MAX_GROUP_SIZE_(group_size),
     BS_SIZE_(bs),
     PRE_BUF_SIZE_((uint32_t)(1.*bs*0.8/(group_size+1))),
     RUN_BUF_SIZE_(PRE_BUF_SIZE_*group_size),
     OUT_BUF_SIZE_(bs - RUN_BUF_SIZE_ - PRE_BUF_SIZE_),
     OUT_BUF_NUM_(output_num)
  {
    pre_buf_ = run_buf_  = out_buf_ = NULL;
    

    pre_buf_size_ = pre_buf_num_ = count_ = 0;

    micro_run_idx_ = new uint32_t[MAX_GROUP_SIZE_];
    micro_run_pos_ = new uint32_t[MAX_GROUP_SIZE_];
    num_micro_run_ = new uint32_t[MAX_GROUP_SIZE_];
    size_micro_run_ = new uint32_t[MAX_GROUP_SIZE_];
    num_run_ = new uint32_t[MAX_GROUP_SIZE_];
    size_run_ = new uint32_t[MAX_GROUP_SIZE_];
    size_loaded_run_ = new uint32_t[MAX_GROUP_SIZE_];	
    run_addr_ = new uint64_t[MAX_GROUP_SIZE_];
    run_curr_addr_ = new uint64_t[MAX_GROUP_SIZE_];

    micro_buf_ = new char*[MAX_GROUP_SIZE_];
    sub_out_buf_ = new char*[OUT_BUF_NUM_];

    in_out_mtx_ = new boost::mutex[OUT_BUF_NUM_];
    in_out_con_ = new boost::condition[OUT_BUF_NUM_];

    out_buf_size_ = new uint32_t[OUT_BUF_NUM_];
    out_buf_full_ = new bool[OUT_BUF_NUM_];
  }

  ~SortMerger()
  {
    if (pre_buf_)
      free(pre_buf_);

    if (run_buf_)
      free(run_buf_);

    if (out_buf_)
      free(out_buf_);

    delete[] micro_run_idx_;
    delete[] micro_run_pos_;
    delete[] num_micro_run_;
    delete[] size_micro_run_;
    delete[] num_run_;
    delete[] size_run_ ;
    delete[] size_loaded_run_;
    delete[] run_addr_ ;
    delete[] run_curr_addr_;

    delete[] micro_buf_;
    delete[] sub_out_buf_;

    delete[] in_out_mtx_;
    delete[] in_out_con_;

    delete[] out_buf_size_;
    delete[] out_buf_full_;
  }

  void set_params(uint32_t max_record_len)
  {
      if(max_record_len > PRE_BUF_SIZE_)
      {
        PRE_BUF_SIZE_ = max_record_len;
        RUN_BUF_SIZE_ = PRE_BUF_SIZE_*MAX_GROUP_SIZE_;
        //OUT_BUF_SIZE_ = BS_SIZE_ - RUN_BUF_SIZE_ - PRE_BUF_SIZE_; ///we do not change OUT_BUF_SIZE_
      }
  }

  void set_params(uint32_t max_record_len, uint32_t min_buff_size_required)
  {
      if(max_record_len > PRE_BUF_SIZE_)
        PRE_BUF_SIZE_ = max_record_len;
      if(RUN_BUF_SIZE_ < min_buff_size_required)		
        RUN_BUF_SIZE_ = min_buff_size_required;
      if(RUN_BUF_SIZE_ < PRE_BUF_SIZE_*MAX_GROUP_SIZE_)
        RUN_BUF_SIZE_ = PRE_BUF_SIZE_*MAX_GROUP_SIZE_;
      if(OUT_BUF_SIZE_ < min_buff_size_required)
        OUT_BUF_SIZE_ = min_buff_size_required;
  }

  void run()
  {
    struct timeval tvafter, tvpre;
    struct timezone tz;
    
    gettimeofday (&tvpre , &tz);
    
    FILE* f = fopen(filenm_.c_str(), "r");
    IASSERT(f);
//    IASSERT(fread(&count_, sizeof(uint64_t), 1, f)==1);

    IO_TYPE ioStream(f);
    FILE_LEN_ = ioStream.length();

    ioStream.readBytes((char*)(&count_),sizeof(uint64_t));

    LOG(INFO) << "Count: " << count_;
	
    init_(ioStream);

    boost::thread predict_thre(boost::bind(&self_t::predict_, this, boost::ref(ioStream)));
    boost::thread merge_thre(boost::bind(&self_t::merge_, this));

    FILE* out_f = fopen((filenm_+".out").c_str(), "w+");
    IASSERT(out_f);
    IASSERT(fwrite(&count_, sizeof(uint64_t), 1, out_f)==1);

    boost::thread* out_thres[OUT_BUF_NUM_];
    for (uint32_t i=0; i<OUT_BUF_NUM_; ++i)
      out_thres[i] = new boost::thread (boost::bind(&self_t::output_, this, out_f, i));

    predict_thre.join();
    merge_thre.join();
    for (uint32_t i=0; i<OUT_BUF_NUM_; ++i)
    {
      out_thres[i]->join();
      //std::cout<<"\nJoin:"<<i<<std::endl;
      delete out_thres[i];
    }

    fclose(f);
    fclose(out_f);

    gettimeofday (&tvafter , &tz);
    LOG(INFO) << "Merging is done(" << count_
             << "): " << ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000 << " min";
    
    if (boost::filesystem::exists(filenm_))
      boost::filesystem::remove(filenm_);
    if (boost::filesystem::exists(filenm_+".out"))
      boost::filesystem::rename(filenm_+".out", filenm_);
  }
}
  ;

NS_IZENELIB_AM_END

#endif
