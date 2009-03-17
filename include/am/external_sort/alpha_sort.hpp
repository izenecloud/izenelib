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

using namespace std;

NS_IZENELIB_AM_BEGIN

template<
  uint64_t TOTAL_BUFFER_SIZE = 100000,//300000000,//at leat 20000
  uint32_t CACHE_BLOCK_SIZE = 2880
  >
class AlphaSort
{
  
  struct PRE_KEY_POINTER_
  {
    uint32_t pre_key_;
    uint64_t rcrd_addr_;
    FILE* f_;

    inline PRE_KEY_POINTER_(uint32_t pre_key,uint64_t rcrd_addr, FILE* f)
    {
      pre_key_ = pre_key;
      rcrd_addr_ = rcrd_addr;
      f_ = f;
    }

    inline PRE_KEY_POINTER_()
    {
      pre_key_ = -1;
      rcrd_addr_ = -1;
      f_ = NULL;
    }

    
    bool operator == (const PRE_KEY_POINTER_& p) const
    {
      return rcrd_addr_ == p.rcrd_addr_;
    }
    
    bool operator < (const PRE_KEY_POINTER_& p) const
    {
      return pre_key_ < p.pre_key_;
    }

    bool operator > (const PRE_KEY_POINTER_& p) const
    {
      return pre_key_ > p.pre_key_;
    }

  friend ostream& operator <<(ostream& os, const PRE_KEY_POINTER_& v)
    {
      os<<"["<<v.pre_key_<<","<<v.rcrd_addr_<<","<<v.f_<<"]";
      return os;
    }
    
    
  };

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
      if (key_.pre_key_ == p.key_.pre_key_)
      {
        if (idx_ == p.idx_)
          return idx_in_block_<p.idx_in_block_;
        return idx_ < p.idx_;
      }
      
      return key_.pre_key_ < p.key_.pre_key_;
    }

    bool operator > (const T_TREE_NODE_& p) const
    {      
      if (key_.pre_key_ == p.key_.pre_key_)
      {
        if (idx_ == p.idx_)
          return idx_in_block_ > p.idx_in_block_;
        return idx_ > p.idx_;
      }

      return key_.pre_key_ > p.key_.pre_key_;
    }

    bool isMax() const
    {
      return key_.pre_key_==(uint32_t)-1 && key_.rcrd_addr_==(uint64_t)-1;
    }

  friend ostream& operator <<(ostream& os, const T_TREE_NODE_& v)
    {
      os<<"<"<<v.key_.pre_key_<<","<<v.idx_<<","<<v.idx_in_block_<<">";
      return os;
    }
    
    
  };
  
#define ND_CACHE_SIZE ((CACHE_BLOCK_SIZE-sizeof(uint32_t))/sizeof(T_TREE_NODE_))
#define ND_CACHE_MAX_LEN ((ND_CACHE_SIZE+1)/2)

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
    cout<<"in_mem_blk_amnt_: "<<in_mem_blk_amnt_<<endl;
    cout<<"per_block_: "<<per_block_<<endl;
    cout<<"output_buffer_size_: "<<output_buffer_size_<<endl;
    cout<<"ND_CACHE_SIZE: "<<ND_CACHE_SIZE<<endl;
    cout<<"ND_CACHE_MAX_LEN: "<<ND_CACHE_MAX_LEN<<endl;
    
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

    //for(typename vector<FileCache_*>::iterator i=inputFileCacheVctr_.begin(); i!=inputFileCacheVctr_.end();i++)
    //if (*i != NULL)
    //  delete *i;

    if(output_buffer_!=NULL)
      delete output_buffer_;

    delete mediaBlock_;
  }

  void addInputFile(const string& fileName)
  {
    input_files_.push_back(fileName);
  }

  void addInputFiles(const vector<string>& fileNames)
  {
    input_files_.insert(input_files_.begin(), fileNames.begin(),fileNames.end());
  }
  
  void sort(const string& outputFile)
  {
     outputFileNm_ = outputFile;
    
    boost::function0< void> f =  boost::bind(&AlphaSort::inputThread,this);
    boost::thread inputting( f );

    boost::function0< void> f1 =  boost::bind(&AlphaSort::sortThread,this);
    boost::thread sorting( f1 );

    boost::function0< void> f2 =  boost::bind(&AlphaSort::outputThread,this);
    boost::thread outputting( f2 );
    
    inputting.join();
    sorting.join();
    outputting.join();

//     clock_t start, finish;
//     start = clock();
//     inputThread();
//     finish = clock();
//     cout<<"\nInput Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;

//     start = clock();
//     sortThread();
//     finish = clock();
//     cout<<"\nSort Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;

//     start = clock();
//     outputThread();
//     finish = clock();
//     cout<<"\nOutput Thread: "<<(double)(finish - start) / CLOCKS_PER_SEC;
    
  }

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

  void printTournamentTree()
  {
    uint32_t c = blk_amnt_%2==0? blk_amnt_: blk_amnt_+1;
    
    for(typename vector<T_TREE_NODE_*>::iterator i=t_tree_pool_.begin(); i!=t_tree_pool_.end();i++)
    {
      T_TREE_NODE_* b = *i;
      cout<<"\n-------------"<<c<<"----------------\n";
      for (uint32_t j=0; j < c; j++)
      {
        cout<<b[j];
      }
      c = c/2;
      c = (c%2==0||c==1)? c: c+1;
    }
  }

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

  void printOutputBuffer()
  {
    cout<<endl;
    for (uint64_t i=0; i<write_output_idx_; i++)
    {
      cout<<output_buffer_[i]<<endl;
    }
    
  }

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
  
  bool t_checkOutputFile()
  {
    const uint16_t read_size = sizeof(uint16_t)+sizeof(uint32_t);
    
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

    uint32_t last_key = 0;
    for (uint64_t j=0; j<count; j++)
    {
      char rec[read_size];
        
      if(fread(rec, read_size, 1, f)!=1)
      {
        LDBG_<<"Reading record faild in output file: "<<outputFileNm_;
        fclose(f);
        return false;
      }

      uint32_t k = *(uint32_t*)(rec+sizeof(uint16_t));

      if (k<last_key)
      {
        cout<<"\nOutput checking faild!\n";
        return false;
      }
      last_key = k;
      
      fseek(f,(*(uint16_t*)rec)-sizeof(uint32_t) ,SEEK_CUR);
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
        f = raw_pool_.size()==0;
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
  
  uint64_t inputThread()
  {
    PRE_KEY_POINTER_* block = new PRE_KEY_POINTER_[per_block_];
    uint32_t c = 0;
    const uint16_t read_size = sizeof(uint16_t)+sizeof(uint32_t);
    
    for (vector<string>::iterator i=input_files_.begin(); i!=input_files_.end();i++)
    {
      FILE* f = fopen((*i).c_str(), "r");
      if (f == NULL)
      {
        LDBG_<<"Can not open file: "<<(*i);
        return (uint64_t)-1;
      }

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
          push2rawPool(block);
          blk_amnt_++;
          block = new PRE_KEY_POINTER_[per_block_];            
          c = 0;
        }

        block[c] = PRE_KEY_POINTER_ (*(uint32_t*)(rec+sizeof(uint16_t)), pos,f);
        c++;

        data_amount_++;
        fseek(f, (*(uint16_t*)rec)-sizeof(uint32_t), SEEK_CUR);
        //start += (*(uint16_t*)rec)+sizeof(uint32_t);
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
      push2rawPool(block);
    }

    inputDone();
    
    return data_amount_;
  }

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

  void treeGrow()
  {
    T_TREE_NODE_* top = t_tree_pool_.back();
    T_TREE_NODE_* leaf = t_tree_pool_.front();
    
    while (!top->isMax())
    {
      PRE_KEY_POINTER_* block;
      T_TREE_NODE_ newNode;
      getSortedBlockOf(top->idx_, &block);
      
      if (top->idx_in_block_<per_block_-1)
        newNode = T_TREE_NODE_(block[top->idx_in_block_+1], top->idx_, top->idx_in_block_+1);
      
      uint64_t i=0;
      
      for (; i<blk_amnt_; i++)
      {
        if (leaf[i]==*top)
        {
          leaf[i] = newNode;
          break;
        }
        
      }
      //cout<<endl;
      
      writeOutputBuff(top->key_);
      top = getTopOfTree(i);
    }
  }

  void cs_treeGrow()
  {
    T_TREE_NODE_* top = (*cs_tree_pool_.back())[0]->getTop();
    writeOutputBuff(top->key_);
    
    while (!top->isMax())
    {
      //cout<<"\n-----------------------\n";
//       cout<<*top<<endl;
//       cs_printTournamentTree();
      
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
            cout<<i<<"  "<<j<<endl;
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
//           cs_printTournamentTree();
//           cout<<i<<"--"<<j<<endl;
//           cout<<newNode<<endl;
//           cout<<*top<<endl;
          
          LDBG_<<"TREE_BUCKET_: 4";
          return;
        }

//         if (newNode.key_.pre_key_ == 1418754838 && newNode.idx_ == 6891)
//         {
//           cout<<i<<"--"<<j<<endl;
//           cout<<*top<<endl;
//         }
        
        
//         if (i==3 && j==3 && newNode.idx_ == 7070)
//         {
//           cs_printTournamentTree();
//           cout<<"ooeoeoeo\n";
//           cout<<newNode<<endl;
//           cout<<*top<<endl;
//         }

        if (j%2==0)
        {
          if (j<cs_tree_pool_[i]->size()-1)
          {
            top = *top<*(*cs_tree_pool_[i])[j+1]->getTop()? top:(*cs_tree_pool_[i])[j+1]->getTop();
            newTop = *newTop<*(*cs_tree_pool_[i])[j+1]->getTop()? newTop: (*cs_tree_pool_[i])[j+1]->getTop();
          }
        }
        else
        {
          top = *top<*(*cs_tree_pool_[i])[j-1]->getTop()? top:(*cs_tree_pool_[i])[j-1]->getTop();
          newTop = *newTop<*(*cs_tree_pool_[i])[j-1]->getTop()? newTop: (*cs_tree_pool_[i])[j-1]->getTop();
        }
        
        newNode = *newTop;
        
        j = j/2/ND_CACHE_MAX_LEN;
        i++;
        //cs_printTournamentTree();
      }
      
      *top = newNode;
      writeOutputBuff(top->key_);
      //cout<<"\n..................\n";
      //cs_printTournamentTree();
    }
  }
  
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
  
  T_TREE_NODE_* getTopOfTree(uint64_t idx)
  {
    for (size_t i = 0;i<t_tree_pool_.size()-1;i++)
    {
      uint64_t up_idx = idx%2==0? idx/2: (idx-1)/2;
      if (idx%2==0)
        t_tree_pool_[i+1][up_idx] = t_tree_pool_[i][idx]<t_tree_pool_[i][idx+1]? t_tree_pool_[i][idx]: t_tree_pool_[i][idx+1];
      else
        t_tree_pool_[i+1][up_idx] = t_tree_pool_[i][idx]<t_tree_pool_[i][idx-1]? t_tree_pool_[i][idx]: t_tree_pool_[i][idx-1];

      idx = up_idx;
    }

    return t_tree_pool_.back();
    
  }
  
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
  
  bool add2TempFile(PRE_KEY_POINTER_* block)
  {
    if (tempFile_ == NULL)
      return false;

    if(fwrite(block, CACHE_BLOCK_SIZE, 1, tempFile_)!=1)
    {
      LDBG_<<"can't write to temple file.";
      return false;
    }
    
    return true;
  }

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
  
  //Swap two elements
  void swap(PRE_KEY_POINTER_* a, PRE_KEY_POINTER_* b)
  {
    PRE_KEY_POINTER_ temp;
    temp = *a;
    *a = *b;
    *b = temp;
  }

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

    uint32_t average = (min.pre_key_+max.pre_key_)/2;
    uint32_t idx = left;
    uint32_t minGap = average>array[idx].pre_key_? average-array[idx].pre_key_ : array[idx].pre_key_-average;
    
    for (uint32_t i=left+shift; i<=right; i+=shift)
    {
      uint32_t gap = average>array[i].pre_key_? average-array[i].pre_key_ : array[i].pre_key_-average;
      if (gap<minGap)
      {
        minGap = gap;
        idx = i;
      }
    }

    return idx;
  }
  
  //Partition the array into two halves and return the
  //index about which the array is partitioned
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
  
  void makeTournamentTree()
  {
    T_TREE_NODE_* leaf;
    uint64_t level_c = blk_amnt_;

    level_c = level_c%2==0? level_c: level_c+1;
    leaf = new T_TREE_NODE_[level_c];
    
    uint64_t c=0;
    for(typename vector<PRE_KEY_POINTER_*>::iterator i=sorted_pool_.begin(); i!=sorted_pool_.end()&&c<blk_amnt_;i++,c++)
    {
      leaf[c]  = T_TREE_NODE_((*i)[0], c, 0);
    }

    for(;c<blk_amnt_;c++)
    {
      PRE_KEY_POINTER_* b;
      getSortedBlockOf(c, &b);
      leaf[c] = T_TREE_NODE_(*b, c, 0);
    }

    t_tree_pool_.push_back(leaf);

        
    while ((uint64_t)(level_c/2.+0.6)!=1)
    {
      T_TREE_NODE_* last_leaf = t_tree_pool_.back();
      uint64_t last_lvl_c = level_c;

      level_c = (uint64_t)(level_c/2.+0.6);
      level_c = level_c%2==0? level_c: level_c+1;
      
      leaf = new  T_TREE_NODE_[level_c];

      for (uint64_t i=0, j=0; i<last_lvl_c-1; i+=2,j++)
      {
        leaf[j] = last_leaf[i]<last_leaf[i+1]? last_leaf[i]: last_leaf[i+1];
      }
      
      t_tree_pool_.push_back(leaf);
    }

    t_tree_pool_.push_back(new T_TREE_NODE_(leaf[0]<leaf[1]?leaf[0]: leaf[1]));
  }
  
  

protected:
  string outputFileNm_;
  
  const uint64_t in_mem_blk_amnt_;
  const uint32_t per_block_;
  const uint64_t output_buffer_size_;
  uint64_t blk_amnt_;

  vector<PRE_KEY_POINTER_*> raw_pool_;
  vector<PRE_KEY_POINTER_*> sorted_pool_;
  vector<T_TREE_NODE_* > t_tree_pool_;
  vector<vector<TREE_BUCKET_* >* > cs_tree_pool_;

  vector<string> input_files_;

  PRE_KEY_POINTER_* output_buffer_;
  uint64_t data_amount_;

  bool isInputed_;

  FILE* tempFile_;

  PRE_KEY_POINTER_* mediaBlock_;
  uint64_t mediaIdx_;

  uint64_t read_output_idx_;
  uint64_t write_output_idx_;

  boost::mutex input_mutex_;
  boost::mutex is_input_mutex_;
  boost::mutex output_mutex_;

  //  vector<FileCache_*> inputFileCacheVctr_;
}
  ;

NS_IZENELIB_AM_END

#endif
