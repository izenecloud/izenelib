/**
   @file alpha_sort.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef ALPHA_SORT_HPP
#define ALPHA_SORT_HPP

#include <util/log.h>
#include <vector>
#include <string>
#include <types.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <time.h>
#include <am/graph_index/addr_bucket.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
   @class AlphaSort
   This work is based on Chris Nyberg’s work in 1995. It uses quick sort to generate runs, and tournament tree sort to merge runs.
   Both quick sort and tournament sort is made as cache-sensitive algorithms.
 **/
template<
  class PRE_KEY_TYPE = uint32_t,//pre-key type, indicate the length of the pre-key.
  bool  COMPARE_ALL = true,
  uint64_t TOTAL_BUFFER_SIZE = 3000000,//300000000,//at leat 20000
  uint32_t CACHE_BLOCK_SIZE = 2880//CPU cache size
  >
class AlphaSort
{

  /**
     @brief pre-key used for sorting
   */
  struct PRE_KEY_POINTER_
  {
    PRE_KEY_TYPE pre_key_;//!< pre-key, used to compare
    uint64_t rcrd_addr_;//!< record address
    uint16_t len_;
    FILE* f_;//!< record file handler

    inline PRE_KEY_POINTER_(PRE_KEY_TYPE pre_key,uint64_t rcrd_addr, uint16_t len, FILE* f)
    {
      pre_key_ = pre_key;
      rcrd_addr_ = rcrd_addr;
      len_ = len;
      f_ = f;
    }

    inline PRE_KEY_POINTER_()
    {
      pre_key_ = -1;
      rcrd_addr_ = -1;
      len_ = 0;
      f_ = NULL;
    }

    
    bool operator == (const PRE_KEY_POINTER_& p) const
    {
      return rcrd_addr_ == p.rcrd_addr_;
    }

    /**
       If pre-key is the same, it loads next 4 bytes data of the record to compare
     **/
    bool operator < (const PRE_KEY_POINTER_& p) const
    {
      if (COMPARE_ALL && pre_key_== p.pre_key_)
      {
        if (f_==NULL || p.f_==NULL)
          return false;
        
        PRE_KEY_TYPE k=pre_key_, pk=p.pre_key_;
        uint32_t shift = sizeof(PRE_KEY_TYPE);
        
        while (k == pk && len_>shift && p.len_>shift)
        {
          fseek(f_, rcrd_addr_+sizeof(uint16_t)+shift, SEEK_SET);
          uint32_t s = shift+sizeof(PRE_KEY_TYPE)>len_? len_-shift:sizeof(PRE_KEY_TYPE);
          
          if (fread(&k,s, 1, f_ )!=1)
          {
            LDBG_<<"Operator <: can't read";
            return false;
          }

          fseek(p.f_, p.rcrd_addr_+sizeof(uint16_t)+shift, SEEK_SET);
          s = shift+sizeof(PRE_KEY_TYPE)>p.len_? p.len_-shift:sizeof(PRE_KEY_TYPE);
          
          if (fread(&pk,s, 1, p.f_ )!=1)
          {
            LDBG_<<"Operator <: can't read";
            return false;
          }

          shift += sizeof(PRE_KEY_TYPE);
        }
        
        return k< pk;
      }
      
      return pre_key_ < p.pre_key_;
    }

    
    /**
       If pre-key is the same, it loads next 4 bytes data of the record to compare
     **/
    bool operator > (const PRE_KEY_POINTER_& p) const
    {
      
      if (COMPARE_ALL && pre_key_== p.pre_key_)
      {
        if (f_==NULL || p.f_==NULL)
          return false;
        
        PRE_KEY_TYPE k=pre_key_, pk=p.pre_key_;
        uint32_t shift = sizeof(PRE_KEY_TYPE);
        
        while (k == pk && len_>shift && p.len_>shift)
        {
          fseek(f_, rcrd_addr_+sizeof(uint16_t)+shift, SEEK_SET);
          uint32_t s = shift+sizeof(PRE_KEY_TYPE)>len_? len_-shift:sizeof(PRE_KEY_TYPE);
          
          if (fread(&k,s, 1, f_ )!=1)
          {
            LDBG_<<"Operator <: can't read";
            return false;
          }

          fseek(p.f_, p.rcrd_addr_+sizeof(uint16_t)+shift, SEEK_SET);
          s = shift+sizeof(PRE_KEY_TYPE)>p.len_? p.len_-shift:sizeof(PRE_KEY_TYPE);
          
          if (fread(&pk,s, 1, p.f_ )!=1)
          {
            LDBG_<<"Operator <: can't read";
            return false;
          }

          shift += sizeof(PRE_KEY_TYPE);
        }

        return k> pk;
      }

      return pre_key_ > p.pre_key_;
    }

    bool operator >= (const PRE_KEY_POINTER_& p) const
    {
      return *this>p || *this==p;
    }

    bool operator <= (const PRE_KEY_POINTER_& p) const
    {
      return *this<p || *this==p;
    }
    

  friend ostream& operator <<(ostream& os, const PRE_KEY_POINTER_& v)
    {
      os<<"["<<v.pre_key_<<","<<v.rcrd_addr_<<","<<v.f_<<"]";
      return os;
    }
    
    
  };

  typedef FileDataBucket<PRE_KEY_POINTER_, 10000000> data_bucket_t;
  /**
     Tournament tree node.
   **/
  struct T_TREE_NODE_
  {
    PRE_KEY_POINTER_ key_;
    size_t idx_;
    uint32_t idx_in_block_;

    inline T_TREE_NODE_(const PRE_KEY_POINTER_& key, size_t idx, uint32_t idx_in_block)
    {
      key_ = key;
      idx_ = idx;
      idx_in_block_ = idx_in_block;
    }

    inline T_TREE_NODE_()
    {
      idx_ = -1;
      idx_in_block_ = -1;
    }

    bool operator == (const T_TREE_NODE_& p) const
    {
      return key_.rcrd_addr_ == p.key_.rcrd_addr_;
    }
    
    bool operator < (const T_TREE_NODE_& p) const
    {
//       if (key_.pre_key_ == p.key_.pre_key_)
//       {
//         if (idx_ == p.idx_)
//           return idx_in_block_<p.idx_in_block_;
//         return idx_ < p.idx_;
//       }
      
      return key_ < p.key_;
    }
    
//     bool operator <= (const T_TREE_NODE_& p) const
//     {      
//       return key_ <= p.key_;
//     }

    bool operator > (const T_TREE_NODE_& p) const
    {      
//       if (key_.pre_key_ == p.key_.pre_key_)
//       {
//         if (idx_ == p.idx_)
//           return idx_in_block_ > p.idx_in_block_;
//         return idx_ > p.idx_;
//       }

      return key_ > p.key_;
    }

    bool isMax() const
    {
      return key_.pre_key_==(PRE_KEY_TYPE)-1 && key_.rcrd_addr_==(uint64_t)-1;
    }

  friend ostream& operator <<(ostream& os, const T_TREE_NODE_& v)
    {
      os<<"<"<<v.key_<<","<<v.idx_<<","<<v.idx_in_block_<<">";
      return os;
    }
    
    
  };
  
#define ND_CACHE_SIZE ((CACHE_BLOCK_SIZE-sizeof(uint32_t))/sizeof(T_TREE_NODE_))
#define ND_CACHE_MAX_LEN ((ND_CACHE_SIZE+1)/2)

  /**
     Tree is splitted into sub-trees. This is the container of the sub-tree.
   **/
  class TREE_BUCKET_
  {
    T_TREE_NODE_ pool_[ND_CACHE_SIZE];
    uint32_t data_c_;

  public:
    TREE_BUCKET_()
    {
      data_c_ = 0;
    }

    bool newNode(const T_TREE_NODE_& n)
    {
      if (data_c_ >= ND_CACHE_MAX_LEN)
        return false;

      pool_[data_c_] = n;
      data_c_++;
      return true;
    }

    bool isFull()
    {
      return (data_c_ >= ND_CACHE_MAX_LEN);
    }

    bool isEmpty()
    {
      return (data_c_ == 0);
    }
    
    void setNodeOf(uint32_t idx, const T_TREE_NODE_& n)
    {
      IF_DLOG(idx>=ND_CACHE_SIZE)<<"TREE_BUCKET_::setNodeOf";
      pool_[idx] = n;
    }

    uint32_t find(const T_TREE_NODE_& n)
    {
      for(uint32_t i=0; i<data_c_; i++)
      {
        if (pool_[i]==n)
          return i;
      }

      return (uint32_t)-1;
    }

    T_TREE_NODE_* replace(const T_TREE_NODE_& src, const T_TREE_NODE_& des )
    {
      uint32_t idx = find(src);
      if (idx == (uint32_t)-1 || idx>=data_c_)
        return NULL;

      pool_[idx] = des;

      idx = idx%2==0? idx: idx-1;
      
      uint32_t i = 0;
      uint32_t c = data_c_;
      while (i+1<c)
      {
        if (i == idx)
        {
          pool_[c] = pool_[i]<pool_[i+1]? pool_[i]: pool_[i+1];
          idx = c%2==0? c: c-1;
        }
        
        c++;
        i += 2;
      }
      
      return &pool_[idx];
    }

    T_TREE_NODE_* getTop()
    {
      if (data_c_==0)
        return NULL;

      if (data_c_==1)
        return &pool_[0];

      return &pool_[data_c_*2-2];
    }

    T_TREE_NODE_* grow()
    {
      if (data_c_==0)
        return NULL;

      if (data_c_==1)
        return &pool_[0];

      uint32_t i = 0;
      uint32_t c = data_c_;
      while (i+1<c)
      {
        pool_[c] = pool_[i]<pool_[i+1]? pool_[i]: pool_[i+1];
        c++;
        i += 2;
      }

      return &pool_[data_c_*2-2];
    }

  friend ostream& operator <<(ostream& os, const TREE_BUCKET_& v)
    {
      os<<"["<<v.data_c_<<"]";
      for(uint32_t i=0; i<2*v.data_c_-1; i++)
        os<<v.pool_[i]<<",";
      return os;
    }
    

  };
  
#define UNIT_SIZE sizeof(PRE_KEY_POINTER_)
#define TREE_NODE_SIZE sizeof(T_TREE_NODE_)
  
public:
  AlphaSort()
    :in_mem_blk_amnt_(TOTAL_BUFFER_SIZE/(2*CACHE_BLOCK_SIZE+2*TREE_NODE_SIZE)-1),
     per_block_(CACHE_BLOCK_SIZE/UNIT_SIZE), output_buffer_size_(in_mem_blk_amnt_*CACHE_BLOCK_SIZE/UNIT_SIZE)
  {
//     cout<<"in_mem_blk_amnt_: "<<in_mem_blk_amnt_<<endl;
//     cout<<"per_block_: "<<per_block_<<endl;
//     cout<<"output_buffer_size_: "<<output_buffer_size_<<endl;
//     cout<<"ND_CACHE_SIZE: "<<ND_CACHE_SIZE<<endl;
//     cout<<"ND_CACHE_MAX_LEN: "<<ND_CACHE_MAX_LEN<<endl;
    
    output_buffer_ = new PRE_KEY_POINTER_[output_buffer_size_];
    blk_amnt_ = data_amount_ = 0;
    isInputed_ = false;

    stringstream ss("./.");
    ss<< getpid();
    string tmp;
    ss >> tmp;

    tempFile_ = fopen(tmp.c_str(), "w+");
    if (tempFile_ == NULL)
    {
      LDBG_<<"can't create temple file.";
    }
    
    mediaBlock_ = new PRE_KEY_POINTER_[output_buffer_size_];
    mediaIdx_ = -1;

    read_output_idx_ = write_output_idx_ = 0;
  }

  ~AlphaSort()
  {
    for(typename vector<PRE_KEY_POINTER_*>::iterator i=sorted_pool_.begin(); i!=sorted_pool_.end();i++)
    {
       if (*i != NULL)
        delete *i;
    }

    for(typename vector<vector<TREE_BUCKET_* >* >::iterator i=cs_tree_pool_.begin(); i!=cs_tree_pool_.end();i++)
    {
      for (typename vector<TREE_BUCKET_* >::iterator j=(*i)->begin(); j!=(*i)->end(); ++j)
      {
        delete (*j);
      }
      
      delete  (*i);
    }

    //for(typename vector<FileCache_*>::iterator i=inputFileCacheVctr_.begin(); i!=inputFileCacheVctr_.end();i++)
    //if (*i != NULL)
    //  delete *i;
    
    if(output_buffer_!=NULL)
      delete output_buffer_;
    
    if (mediaBlock_!=NULL)
      delete mediaBlock_;

    stringstream ss("./.");
    ss<< getpid();
    string tmp;
    ss >> tmp;

    remove(tmp.c_str());
  }

  /**
     Add input file. This should be called before sorting.
   **/
  void addInputFile(const string& fileName)
  {
    input_files_.push_back(fileName);
  }

  
  /**
     Add input file. This should be called before sorting.
   **/
  void addInputFiles(const vector<string>& fileNames)
  {
    input_files_.insert(input_files_.begin(), fileNames.begin(),fileNames.end());
  }

  /**
     Start to sort, the result will be put into output file.
   **/
  void sort(const string& outputFile)
  {
     outputFileNm_ = outputFile;
    
//     boost::function0< void> f =  boost::bind(&AlphaSort::inputThread,this);
//     boost::thread inputting( f );

//     boost::function0< void> f1 =  boost::bind(&AlphaSort::sortThread,this);
//     boost::thread sorting( f1 );

//     boost::function0< void> f2 =  boost::bind(&AlphaSort::outputThread,this);
//     boost::thread outputting( f2 );
    
//     inputting.join();
    
//     sorting.join();
    
//     outputting.join();

     //clock_t start, finish;
     //start = clock();
    inputThread();
    //finish = clock();
    //cout<<"\nInput Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;

//     start = clock();
//     sortThread();
//     finish = clock();
//     cout<<"\nSort Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;

//     start = clock();
//     outputThread();
//     finish = clock();
//     cout<<"\nOutput Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;
    
  }

  /**
     This is used to debug.
   **/
  void printRawPool()
  {
    for(typename vector<PRE_KEY_POINTER_*>::iterator i=raw_pool_.begin(); i<raw_pool_.end();i++)
    {
      cout<<"\n/////////////////////////////////\n";
      for (uint32_t j=0; j<per_block_; j++)
      {
        cout<<((*i)[j]);
      }
    }
  }

  
  /**
     This is used to debug.
   **/
  bool t_checkSortedPool()
  {
    for(typename vector<PRE_KEY_POINTER_*>::iterator i=sorted_pool_.begin(); i<sorted_pool_.end();i++)
    {
      for (uint32_t j=0; j<per_block_; j++)
      {
        if (j>1 && (*i)[j]<(*i)[j-1])
        {
          cout<<"\nFaild!\n";
          return false;
        }
      }
    }

    return true;
  }
  
  /**
     This is used to debug.
   **/
  void cs_printTournamentTree()
  {
    size_t l = 0;
    for (typename vector<vector<TREE_BUCKET_* >* >::iterator i=cs_tree_pool_.begin(); i!=cs_tree_pool_.end(); i++,l++)
    {
      cout<<"\n++++++++++++++++++"<<l<<"++++++++++++++++++\n";
      
      for (typename vector<TREE_BUCKET_*>::iterator j=(*i)->begin(); j!=(*i)->end(); j++)
      {
        cout<<endl;
        cout<<*(*j);
      }
    }
    cout<<endl;
  }

  
  /**
     This is used to debug.
   **/
  void printOutputBuffer()
  {
    cout<<endl;
    for (uint64_t i=0; i<write_output_idx_; i++)
    {
      cout<<output_buffer_[i]<<endl;
    }
    
  }
  
  /**
     This is used to debug.
   **/
  bool t_checkOutputBuffer()
  {
    cout<<endl;
    for (uint64_t i=0; i<write_output_idx_; i++)
    {
      if (i>0 && output_buffer_[i]<output_buffer_[i-1])
      {
        cout<<"Faild!\n";
        return false;
      }
      
    }
    return true;
    
  }

  
  /**
     This is used to test if the output is correct.
   **/
  bool t_checkOutputFile()
  {
    const uint16_t read_size = sizeof(uint16_t)+sizeof(PRE_KEY_TYPE);
    
    FILE* f = fopen(outputFileNm_.c_str(), "r");
    if (f ==NULL)
    {
      LDBG_<<"Can not open output file: "<<outputFileNm_;
      return false;
    }

    uint64_t count = 0;
    if(fread(&count, sizeof(uint64_t), 1, f)!=1)
    {
      LDBG_<<"Reading count faild in file: "<<outputFileNm_;
      fclose(f);
      return false;
    }

    //cout<<count<<endl;
    
    PRE_KEY_TYPE last_key = 0;
    for (uint64_t j=0; j<count; j++)
    {
      char rec[read_size];
        
      if(fread(rec, read_size, 1, f)!=1)
      {
        LDBG_<<"Reading record faild in output file: "<<outputFileNm_<<j;
        fclose(f);
        return false;
      }

      PRE_KEY_TYPE k = *(PRE_KEY_TYPE*)(rec+sizeof(uint16_t));

      //cout<<k<<" "<<last_key<<" "<<count<<" "<<j<<endl;
      if (k<last_key)
      {
        cout<<k<<" "<<last_key<<endl;
        cout<<"\nOutput checking faild!\n";
        return false;
      }
      last_key = k;
      
      fseek(f,(*(uint16_t*)rec)-sizeof(PRE_KEY_TYPE) ,SEEK_CUR);
    }

    return true;
  }
  
protected:
  size_t push2rawPool(PRE_KEY_POINTER_* p)
  {
    bool f = true;
    while (f)
    {
      boost::mutex::scoped_lock lock(input_mutex_);  
      f = raw_pool_.size()>=in_mem_blk_amnt_;
    }
    
    {
      boost::mutex::scoped_lock lock(input_mutex_);  
      raw_pool_.push_back(p);
      
      return raw_pool_.size();
    }
  }

  PRE_KEY_POINTER_* popFromRawPool()
  {
    bool f = true;
    while (f)
    {
      {
        boost::mutex::scoped_lock lock(input_mutex_);
        f = (raw_pool_.size()==0);
      }
      if (f && isInputDone())
        return NULL;
    }

    {
      boost::mutex::scoped_lock lock(input_mutex_);  
      PRE_KEY_POINTER_* tmp = raw_pool_.back();
      raw_pool_.pop_back();
      return tmp;
    }    
  }

  void inputDone()
  {
    boost::mutex::scoped_lock lock(is_input_mutex_);
    isInputed_ = true;
  }

  bool isInputDone()
  {
    boost::mutex::scoped_lock lock(is_input_mutex_);
    if (isInputed_)
      return true;

    return false;
  }

  /**
     Input thread, read records from file into memory.
   **/
  uint64_t inputThread()
  {
    PRE_KEY_POINTER_* block = new PRE_KEY_POINTER_[per_block_];
    uint32_t c = 0;
    const uint16_t read_size = sizeof(uint16_t)+sizeof(PRE_KEY_TYPE);
    
    for (vector<string>::iterator i=input_files_.begin(); i!=input_files_.end();i++)
    {
      FILE* f = fopen((*i).c_str(), "r");
      if (f == NULL)
      {
        LDBG_<<"Can not open file: "<<(*i);
        return (uint64_t)-1;
      }
      fs_.push_back(f);

      uint64_t count = 0;
      if (fread(&count, sizeof(uint64_t), 1, f)!=1)
      {
        LDBG_<<"Reading count faild in file: "<<*i;
        continue;
      }
      
      for (uint64_t j=0; j<count; j++)
      {
        char rec[read_size];
        uint64_t pos = ftell(f);
        
        if(fread(rec, read_size, 1, f)!=1)
        {
          LDBG_<<"Reading "<<j<<" record faild in file: "<<*i;
          continue;
        }
        
        if (c == per_block_)
        {
          uint64_t addr = ftell(f);
          sort(block);
          fseek(f, addr, SEEK_SET);
          //push2rawPool(block);
          blk_amnt_++;
          block = new PRE_KEY_POINTER_[per_block_];            
          c = 0;
        }

        block[c] = PRE_KEY_POINTER_ (*(PRE_KEY_TYPE*)(rec+sizeof(uint16_t)), pos, *(uint16_t*)rec, f);
        c++;

        data_amount_++;
        fseek(f, (*(uint16_t*)rec)-sizeof(PRE_KEY_TYPE), SEEK_CUR);
        //start += (*(uint16_t*)rec)+sizeof(PRE_KEY_TYPE);
      }
    }

    
    if (c <= per_block_)
    {
      while(c < per_block_)
      {
        block[c] = PRE_KEY_POINTER_ ();
        c++;
      }
      blk_amnt_++;
      sort(block);
      //push2rawPool(block);
    }

    inputDone();

    ///////////////////////
    fflush(tempFile_);

    cs_makeTournamentTree();

    std::cout<<"----------\n";
    
    cs_treeGrow();
    
    return data_amount_;
  }

  /**
     Quick sort one block.
   **/
  void sort(PRE_KEY_POINTER_* block)
  {
    quickSort(block, 0, per_block_-1);
      
    if( sorted_pool_.size()<in_mem_blk_amnt_)
      sorted_pool_.push_back(block);
    else
    {
      //write into temple file
      add2TempFile(block);
    }
  }

  /**
     Sort thread, sort blocks while input thread is reading records.
   **/
  void sortThread()
  {
    PRE_KEY_POINTER_* block = popFromRawPool();
    while (block!=NULL)
    {
      quickSort(block, 0, per_block_-1);
      
      if( sorted_pool_.size()<in_mem_blk_amnt_)
        sorted_pool_.push_back(block);
      else
      {
        //write into temple file
        add2TempFile(block);
      }
      block = popFromRawPool();
    }
    
    fflush(tempFile_);
    // sorted all blocks, then use tournament tree to merge
    cs_makeTournamentTree();
    //cs_printTournamentTree();
    
    cs_treeGrow();
  }
  
  /**
     This is cache-sensitive algorithm. Replace tournament tree's root node with sorted block records one by one.
   **/
  void cs_treeGrow()
  {
    FILE* f = fopen(outputFileNm_.c_str(), "w+");
    data_bucket_t bucket( (outputFileNm_+".tmp").c_str() );
    bucket.ready4add();
    
    if (f==NULL)
    {
      LDBG_<<"Can not open file: "<<outputFileNm_;
      return;
    }

    if(fwrite(&data_amount_, sizeof(uint64_t), 1, f)!=1)
    {
      LDBG_<<"Can't write data amount: ";
      return;
    }
    
    T_TREE_NODE_* top = (*cs_tree_pool_.back())[0]->getTop();
    //writeOutputBuff(top->key_);
    //output(f, top->key_);
    bucket.push_back(top->key_);

    uint64_t c = 0;
    while (!top->isMax())
    {      
      PRE_KEY_POINTER_* block;
      T_TREE_NODE_ newNode;
      getSortedBlockOf(top->idx_, &block);
      
      if (top->idx_in_block_<per_block_-1)
        newNode = T_TREE_NODE_(block[top->idx_in_block_+1], top->idx_, top->idx_in_block_+1);

      //find the smallest at bottom line
      size_t i = cs_tree_pool_.size()-1;
      uint32_t j = 0;

      while (i>0)
      {
        TREE_BUCKET_* bu = (*cs_tree_pool_[i])[j];
        uint32_t j1 = bu->find(*top);
        if (j1 ==(uint32_t) -1)
        {
          j++;
          IF_DLOG(j==cs_tree_pool_[i]->size())<<"TREE_BUCKET_: 1";
          
          bu = (*cs_tree_pool_[i])[j];
          j1 = bu->find(*top);
          if (j1== (uint32_t)-1)
          {
            LDBG_<<"TREE_BUCKET_: 2";
            return ;
          }
          
        }
        
        j = (j*ND_CACHE_MAX_LEN + j1)*2;
        i--;
      }
      
      if ((*cs_tree_pool_[i])[j]->find(*top)==(uint32_t)-1)
      {
        j++;
        if ((*cs_tree_pool_[i])[j]->find(*top)==(uint32_t)-1)
        {
          LDBG_<<"TREE_BUCKET_: 3";
          return;
        }
      }
      
      //replace
      i = 0;
      while (i<cs_tree_pool_.size())
      {
        T_TREE_NODE_* newTop = (*cs_tree_pool_[i])[j]->replace(*top, newNode);

        if(newTop == NULL)
        {
          std::cout<<*top<<std::endl;
          std::cout<<newNode<<std::endl;
          std::cout<<(*cs_tree_pool_[i]).size()<<" "<<j<<std::endl;
          LDBG_<<"TREE_BUCKET_: 4";
          return ;
        }

        if (j%2==0)
        {
          if (j<cs_tree_pool_[i]->size()-1)
          {
            top = *top>*(*cs_tree_pool_[i])[j+1]->getTop()? (*cs_tree_pool_[i])[j+1]->getTop():top;
            newTop = *newTop<*(*cs_tree_pool_[i])[j+1]->getTop()? newTop: (*cs_tree_pool_[i])[j+1]->getTop();
          }
        }
        else
        {
          top = *top>*(*cs_tree_pool_[i])[j-1]->getTop()? (*cs_tree_pool_[i])[j-1]->getTop():top;
          newTop = *newTop<*(*cs_tree_pool_[i])[j-1]->getTop()? newTop: (*cs_tree_pool_[i])[j-1]->getTop();
        }
        
        newNode = *newTop;
        
        j = j/2/ND_CACHE_MAX_LEN;
        i++;
      }
      
      *top = newNode;c++;
      // if (c%10000==0)
//         std::cout<<"cc: "<<c<<std::endl;
      //writeOutputBuff(top->key_);
      //output(f, top->key_);
      if (top->key_.f_!=NULL)
        bucket.push_back(top->key_);
    }

    bucket.flush();
    //std::cout<<c<<" "<<bucket.num()<<" "<<data_amount_<<endl;
    
    IASSERT(bucket.num()==data_amount_);
    output(f, bucket);
    fclose(f);
    bucket.dump();
  }

  void output(FILE* f, data_bucket_t& bucket)
  {
    const uint32_t SIZE = 1000000000;
    char* buf = (char*)malloc(SIZE);
    char record[4048];

    IASSERT(input_files_.size()==1);

    fseek(f, sizeof(uint64_t), SEEK_SET);

    for (uint32_t w=0; w<fs_.size(); ++w)
    {
      
      FILE* ff = fs_[w];
      fseek(ff, 0, SEEK_END);
      uint64_t fs = ftell(ff);
    
      uint32_t t = fs%SIZE==0?fs/SIZE:fs/SIZE+1;
      for (uint32_t k= 0; k<t; ++k)
      {
        fseek(ff, SIZE*k, SEEK_SET);
      
        fread(buf, SIZE, 1, ff);
      
        uint64_t start = k*SIZE;
        uint64_t end = fs>(k+1)*SIZE? (k+1)*SIZE: fs;
      
        fseek(f, sizeof(uint64_t), SEEK_SET);
        bucket.ready4fetch();
        for (uint64_t i=0; i<bucket.num(); ++i)
        {
          PRE_KEY_POINTER_ addr = bucket.next();
          //std::cout<<addr<<std::endl;

          if (addr.f_!=ff || addr.rcrd_addr_ < start || addr.rcrd_addr_ >= end)
          {
            fseek(f, addr.len_+sizeof(uint16_t), SEEK_CUR);
            continue;
          }

          //data is in the middle of two buffer
          if (addr.rcrd_addr_+sizeof(uint16_t)+addr.len_>end)
          {
            fseek(ff, addr.rcrd_addr_+sizeof(uint16_t), SEEK_SET);

            IASSERT(addr.len_+sizeof(uint16_t)<=4048);
            *(uint16_t*)record = addr.len_;
            IASSERT(fread(record+sizeof(uint16_t), addr.len_, 1, ff)==1);

            IASSERT(*(PRE_KEY_TYPE*)(record+sizeof(uint16_t)) == addr.pre_key_);
            
            IASSERT(fwrite(record, addr.len_+sizeof(uint16_t), 1, f)==1);
            continue;
          }

          IASSERT(*(uint16_t*)(buf+addr.rcrd_addr_-start) == addr.len_);
          IASSERT(*(PRE_KEY_TYPE*)(buf+addr.rcrd_addr_-start+sizeof(uint16_t)) == addr.pre_key_);
        
          IASSERT(fwrite(buf+addr.rcrd_addr_-start, addr.len_+sizeof(uint16_t), 1, f)==1);

        }
      }
    }

    free(buf);
  }
  
  /**
     Read record from input file then write it into output file.
   **/
  void output(FILE* f, const PRE_KEY_POINTER_& k)
  {
    if (f==NULL || k.f_==NULL)
      return;
        
    uint16_t len;

    fseek(k.f_, k.rcrd_addr_, SEEK_SET);
    if (fread(&len, sizeof(uint16_t), 1, k.f_)!=1)
    {
      LDBG_<<"can't read len of record!";
      return;
    }

    if (fwrite(&len, sizeof(uint16_t), 1, f)!=1)
    {
      LDBG_<<"can't write len of record!";
      return;
    }
      
    char buf[len];
    if (fread(buf, len, 1, k.f_)!=1)
    {
      LDBG_<<"can't read record!";
      return;
    }
      
    if (fwrite(buf, len, 1, f)!=1)
    {
      LDBG_<<"can't write record!";
      return;
    }

    return;
    
  }
  
  /**
     Output thread, read record from input file then write it into output file.
   **/
  void outputThread()
  {
    FILE* f = fopen(outputFileNm_.c_str(), "w+");
    if (f==NULL)
    {
      LDBG_<<"Can not open file: "<<outputFileNm_;
      return;
    }

    if(fwrite(&data_amount_, sizeof(uint64_t), 1, f)!=1)
    {
      LDBG_<<"Can't write data amount: ";
      return;
    }
    
    while(read_output_idx_<data_amount_)
    {
      PRE_KEY_POINTER_ k;
      readOutputBuff(&k);
      uint16_t len;

      fseek(k.f_, k.rcrd_addr_, SEEK_SET);
      if (fread(&len, sizeof(uint16_t), 1, k.f_)!=1)
      {
        LDBG_<<"can't read len of record!";
        return;
      }

      if (fwrite(&len, sizeof(uint16_t), 1, f)!=1)
      {
        LDBG_<<"can't write len of record!";
        return;
      }
      
      char buf[len];
      if (fread(buf, len, 1, k.f_)!=1)
      {
        LDBG_<<"can't read record!";
        return;
      }
      
      if (fwrite(buf, len, 1, f)!=1)
      {
        LDBG_<<"can't write record!";
        return;
      }
    }

    fclose(f);
  }

  /**
     Put item into output buffer
   **/
  void writeOutputBuff(const PRE_KEY_POINTER_& k)
  {
    bool f = true;
    while(f)
      {
        boost::mutex::scoped_lock lock(output_mutex_);
        f = write_output_idx_ - read_output_idx_ >= output_buffer_size_;
      }
    
    
      boost::mutex::scoped_lock lock(output_mutex_);
      output_buffer_[write_output_idx_%output_buffer_size_] = k;
      
    write_output_idx_++;
  }

  /**
     Read on item from output buffer.
   **/
  void readOutputBuff(PRE_KEY_POINTER_* p)
  {
    bool f = true;
    while(f)
    {
      boost::mutex::scoped_lock lock(output_mutex_);
      f = read_output_idx_>=write_output_idx_;
    }
    
    
      boost::mutex::scoped_lock lock(output_mutex_);
      *p = output_buffer_[read_output_idx_%output_buffer_size_];
      
    read_output_idx_++;
  }

  /**
     When memory is full, it put sorted block in a temperary file.
   **/
  bool add2TempFile(PRE_KEY_POINTER_* block)
  {
    if (tempFile_ == NULL)
      return false;

    if(fwrite(block, CACHE_BLOCK_SIZE, 1, tempFile_)!=1)
    {
      LDBG_<<"can't write to temple file.";
      return false;
    }

    delete[] block;
    return true;
  }

  /**
     It return sorted block of specific index.
   **/
  void getSortedBlockOf(uint64_t idx, PRE_KEY_POINTER_** block)
  {
    if (idx<sorted_pool_.size())
    {
      *block = sorted_pool_[idx];
      return;
    }

    readFromTempFile(idx);
    *block = mediaBlock_;
  }

  /**
     Read block into "mediaBlock_" from temperory file. 
   **/
  bool readFromTempFile(uint64_t idx)
  {
    if (tempFile_ == NULL)
      return false;
    
    if (sorted_pool_.size() != in_mem_blk_amnt_)
      LDBG_<<"sorted pool size is different from block amount."<<sorted_pool_.size()<<in_mem_blk_amnt_;
    
    if (tempFile_ == NULL)
      return false;

    if (idx == mediaIdx_)
      return true;

    mediaIdx_ = idx;
    if (idx< in_mem_blk_amnt_)
    {
      LDBG_<<"Index error while reading temple file.";
      return false;
    }
    
    fseek(tempFile_, (idx-in_mem_blk_amnt_)*CACHE_BLOCK_SIZE ,SEEK_SET);

    if(fread(mediaBlock_, CACHE_BLOCK_SIZE, 1, tempFile_)!=1)
    {
      LDBG_<<"can't read temple file.";
      return false;
    }

    return true;
  }
  
  /**
     Swap two elements.
  **/
  void swap(PRE_KEY_POINTER_* a, PRE_KEY_POINTER_* b)
  {
    PRE_KEY_POINTER_ temp;
    temp = *a;
    *a = *b;
    *b = temp;
  }

  /**
     When quick sort, it return the index of media element.
   **/
  uint32_t findMedianIndex(PRE_KEY_POINTER_* array, uint32_t left, uint32_t right)
  {
    return (left+right)/2;
    
    PRE_KEY_POINTER_ min = array[left];
    PRE_KEY_POINTER_ max = array[left];
    uint32_t shift = 5;

    if (right-left<2*shift)
      return (left+right)/2;
    
    for (uint32_t i=left+shift; i<=right; i+=shift)
    {
      if (array[i]<min)
      {
        min = array[i];
        continue;
      }

      if (array[i] > max)
      {
        max = array[i];
        continue;
      }
    }

    PRE_KEY_TYPE average = (min.pre_key_+max.pre_key_)/2;
    uint32_t idx = left;
    PRE_KEY_TYPE minGap = average>array[idx].pre_key_? average-array[idx].pre_key_ : array[idx].pre_key_-average;
    
    for (uint32_t i=left+shift; i<=right; i+=shift)
    {
      PRE_KEY_TYPE gap = average>array[i].pre_key_? average-array[i].pre_key_ : array[i].pre_key_-average;
      if (gap<minGap)
      {
        minGap = gap;
        idx = i;
      }
    }

    return idx;
  }
  
  /**
     Partition the array into two halves and return the index about which the array is partitioned.
  **/
  uint32_t partition(PRE_KEY_POINTER_* array, uint32_t left, uint32_t right)
  {
    uint32_t pivotIndex = findMedianIndex(array, left, right), index = left, i;
    PRE_KEY_POINTER_ pivotValue = array[pivotIndex];
 
    swap(&array[pivotIndex], &array[right]);
    
    for(i = left; i < right; i++)
    {
      if(array[i] < pivotValue)
      {
        swap(&array[i], &array[index]);
        index += 1;
      }
    }
    swap(&array[right], &array[index]);
 
    return index;
  }

  /**
     A recursive function applying quick sort.
   **/
  void quickSort(PRE_KEY_POINTER_* array, uint32_t left, uint32_t right)
  {
    if(right-left<=1)
    {
      if (array[left]>array[right])
        swap(&array[left], &array[right]);
      return;
    }
    
    uint32_t idx = partition(array, left, right);
    
    if(idx>0 && left<idx-1)
      quickSort(array, left, idx - 1);
    if (idx<per_block_-1 && idx+1<right)
      quickSort(array, idx + 1, right);
  }

  /**
     Buid cache-sensitive tournament tree.
   **/
  void cs_makeTournamentTree()
  {
    TREE_BUCKET_* b = new TREE_BUCKET_();
    vector<TREE_BUCKET_*>* leaf = new vector<TREE_BUCKET_*>;

    /////initial the bottom of tree
    uint64_t c= 0;
    for(typename vector<PRE_KEY_POINTER_*>::iterator i=sorted_pool_.begin(); i!=sorted_pool_.end()&&c<blk_amnt_;i++,c++)
    {
      if (b->isFull())
      {
        b->grow();
        leaf->push_back(b);
        b = new TREE_BUCKET_();
      }
      b->newNode(T_TREE_NODE_((*i)[0], c, 0));
    }
    
    for(;c<blk_amnt_;c++)
    {
      if (b->isFull())
      {
        b->grow();
        leaf->push_back(b);
        b = new TREE_BUCKET_();
      }
      
      PRE_KEY_POINTER_* kb;
      getSortedBlockOf(c, &kb);
      b->newNode(T_TREE_NODE_(*kb, c, 0));
    }

    if (!b->isEmpty())
    {
      b->grow();
      leaf->push_back(b);
      cs_tree_pool_.push_back(leaf);
    }
    
    ///////////////////////////
    vector<TREE_BUCKET_*>* last = leaf;
    while(last->size()>1)
    {
      leaf = new vector<TREE_BUCKET_*>;
      b = new TREE_BUCKET_();

      //for (typename vector<TREE_BUCKET_*>::iterator i=last->begin(); i!=last->end();i++)
      for (size_t i=0; i<last->size()-1; i+=2)
      {
        if (b->isFull())
        {
          b->grow();
          leaf->push_back(b);
          b = new TREE_BUCKET_();
        }
        b->newNode(*(*last)[i]->getTop()<*(*last)[i+1]->getTop()?
                   *(*last)[i]->getTop(): *(*last)[i+1]->getTop());
      }

      if (last->size()%2==1)
      {
        if (b->isFull())
        {
          b->grow();
          leaf->push_back(b);
          b = new TREE_BUCKET_();
        }
        b->newNode(*(last->back()->getTop()));
      }
      

      if (b->isEmpty())
        continue;

      b->grow();
      leaf->push_back(b);
      cs_tree_pool_.push_back(leaf);

      last = leaf;
    }
    
  }

protected:
  string outputFileNm_;//!< Output file name.
  
  const uint64_t in_mem_blk_amnt_;//!< Amount of blocks in memory.
  const uint32_t per_block_;//!< Amount of element per block.
  const uint64_t output_buffer_size_;//!< Buffer size for output
  uint64_t blk_amnt_;//!< Total amount of block.

  vector<PRE_KEY_POINTER_*> raw_pool_;//!< A pool for unsorted block.
  vector<PRE_KEY_POINTER_*> sorted_pool_;//!< A pool for sorted block.

  vector<vector<TREE_BUCKET_* >* > cs_tree_pool_;//!< Container for tournament tree.

  vector<string> input_files_;//!< Vector of input file names.
  vector<FILE*> fs_;//!< Vector of input file handler.

  PRE_KEY_POINTER_* output_buffer_;//!< Buffer for output.
  uint64_t data_amount_;//!< Amount of elements.

  bool isInputed_;//!< A flag, indicate if input proccedure is over.

  FILE* tempFile_;//!< File handler of temperory file.

  PRE_KEY_POINTER_* mediaBlock_;//!< Sotre the block read from temperory file.
  uint64_t mediaIdx_;//!< The index of block sotred in mediaBlock_ currently.

  uint64_t read_output_idx_;//!< An index in output buffer, indicate where the read proccedure is reading.
  uint64_t write_output_idx_;//!< An index in output buffer, indicate where the write proccedure is writing.

  boost::mutex input_mutex_;
  boost::mutex is_input_mutex_;
  boost::mutex output_mutex_;
}
  ;

NS_IZENELIB_AM_END

#endif
