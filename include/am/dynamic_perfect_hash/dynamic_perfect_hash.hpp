/**
   @file dynamic_perfect_hash.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef DYNAMIC_PERFECT_HASHING_HPP
#define DYNAMIC_PERFECT_HASHING_HPP

#include <types.h>
#include <util/log.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <time.h>
#include <am/am.h>

using namespace std;

NS_IZENELIB_AM_BEGIN
/**
 *@class DynamicPerfectHash
 * This work is based on Martin's paper 'Dynammic Perfect Hashing: Upper and Lower Bonds' in 1990.
 *
 **/
template<
  class KeyType = uint32_t,
  class ValueType = uint32_t,//string,
  uint32_t c = 2500000,
  uint32_t SM_O = 1
  >
class DynamicPerfectHash : public AccessMethod<KeyType, ValueType>
{
  typedef boost::archive::text_iarchive iarchive;
  typedef boost::archive::text_oarchive oarchive;
  
  typedef DynamicPerfectHash<KeyType, ValueType, c, SM_O> SelfType;
  
  class subtable_cell;
  
  class table_cell
  {
  public:
    uint64_t b_;
    uint64_t s_;
    uint64_t m_;
    uint64_t k_;
    uint64_t u_;
    uint64_t p_;
    subtable_cell* pSub_;

    table_cell()
    {
      b_ = s_ = m_ = 0;
      pSub_ = NULL;
      SelfType::random_param(k_, u_, p_);
    }
    
  friend ostream& operator << ( ostream& os, const table_cell& cell)
    {
      os<<"["<<cell.b_<<","<<cell.s_<<","<<cell.m_<<","<<cell.k_<<", "<<cell.u_<<", "<<cell.p_<<"]";
      return os;
    }

    bool save(FILE* f)
    {
      uint64_t arr[6] = 
        { b_, s_, m_ ,k_, u_, p_
        };
      
      
      if (fwrite(arr, sizeof(arr), 1, f) != 1)
        return false;

      return true;
    }

    bool load(FILE* f)
    {
      uint64_t arr[6];
      
      if (fread(arr, sizeof(arr), 1, f) != 1)
        return false;

      b_ = arr[0];
      s_ = arr[1];
      m_ = arr[2];
      k_ = arr[3];
      u_ = arr[4];
      p_ = arr[5];
      
      return true;
    }
    
    
    
  }
    ;

  class subtable_cell
  {
  public:
    uint64_t data_;
    uint64_t valueIndex_;

    subtable_cell()
    {
      data_ = 0;
      valueIndex_ = -1;
    }

    subtable_cell(uint64_t data, uint64_t valueIdx)
    {
      data_ = data;
      valueIndex_ = valueIdx;
    }
    
  friend ostream& operator << ( ostream& os, const subtable_cell& cell)
    {
      os<<"("<<cell.data_<<"|"<<cell.valueIndex_<<")";
      return os;
    }

    bool save(FILE* f)
    {
      if (fwrite(&data_, sizeof(KeyType), 1, f) != 1)
        return false;

      if (fwrite(&valueIndex_, sizeof(uint64_t), 1, f) != 1)
        return false;

      return true;
    }

    bool load(FILE* f)
    {
      if (fread(&data_, sizeof(KeyType), 1, f) != 1)
        return false;

      if (fread(&valueIndex_, sizeof(uint64_t), 1, f) != 1)
        return false;

      return true;
    }
    
  }
    ;
public:
  
  inline DynamicPerfectHash()
  {
    sum_s_ = M_ = count_ = update_count_ = mainT_size_ = 0;
    pMainT_ = NULL;
    //condition_time_ = insert_time1_ = insert_time2_ = adjust_time_ = rehash_time_ = 0;
  }

  inline ~DynamicPerfectHash()
  {
    deleteAll();
  }

  uint64_t getCount() const 
  {
    return count_;
  }

  uint64_t getUpdateCount() const
  {
    return update_count_;
  }

  uint64_t getSize() const
  {
    return mainT_size_;
  }

  
  int num_items() const 
  {
    return count_;
  }

  //  void print_time()
  // {
//     cout<<"Adjusting subtable takes "<<(double)adjust_time_ / CLOCKS_PER_SEC<<" seconds!\n";
//     cout<<"Rehashing all takes "<<(double)rehash_time_ / CLOCKS_PER_SEC<<" seconds!\n";
//     cout<<"Inserting1 takes "<<(double)insert_time1_ / CLOCKS_PER_SEC<<" seconds!\n";
//     cout<<"Inserting2 takes "<<(double)insert_time2_ / CLOCKS_PER_SEC<<" seconds!\n";
//     cout<<"condition takes "<<(double)condition_time_ / CLOCKS_PER_SEC<<" seconds!\n";
    
  // }
  
  
  //bool insert(const KeyType& x, const ValueType& data)
//   virtual bool insert(const KeyType& key, const ValueType& value){
// 		DataType<KeyType,ValueType> data(key, value);
//     	return insert(data);
// 	}
  using AccessMethod<KeyType, ValueType>::insert;
  
  virtual bool insert(const DataType<KeyType,ValueType>& data)
  {
    uint64_t x = str_hash(data.key);

    uint64_t valueIdx = dataVec_.size();
    dataVec_.push_back(data.value);
    
    update_count_++;
    
    if (update_count_>M_)
    {
      //      finish = clock();
      //      insert_time1_ += finish-start;
      return rehashAll(x, valueIdx);
    }
    
    
    uint64_t j = hashFunc(main_k_, main_u_, main_p_, mainT_size_, x);
    if (pMainT_[j].pSub_!=NULL)
    {
      //cout<<"insert: if (pMainT_[j].pSub_!=NULL)\n";
      uint64_t sub_j = hashFunc(pMainT_[j].k_, pMainT_[j].u_, pMainT_[j].p_, pMainT_[j].s_, x);

      if (pMainT_[j].pSub_!=NULL && pMainT_[j].pSub_[sub_j].data_==x)
      {
        //cout<<"insert: if (pMainT_[j].pSub_!=NULL && pMainT_[j].pSub_[sub_j].data_==x)\n";
        pMainT_[j].pSub_[sub_j].valueIndex_ = valueIdx;
        //        finish = clock();
        //        insert_time1_ += finish-start;
        return true;
      }
    
      if (pMainT_[j].b_ + 1<=pMainT_[j].m_)// size of Tj sufficient
      {
        if (pMainT_[j].pSub_[sub_j].valueIndex_ == (uint64_t)-1)
        {
          pMainT_[j].pSub_[sub_j] = subtable_cell(x, valueIdx);
          count_ ++;
          pMainT_[j].b_ ++;
          return true;
        }
        
        adjustSubTable(j, subtable_cell(x, valueIdx));
        //finish = clock();
        //insert_time1_ += finish-start;
        return true;
      }
    }

    //start = clock();
    //// size of Tj is not sufficient
    //cout<<"insert: size of Tj is not sufficient\n";
    pMainT_[j].m_ = 2* (1<pMainT_[j].m_? pMainT_[j].m_ : 1);
    uint64_t tmp_s = pMainT_[j].s_;
    pMainT_[j].s_ = 2*pMainT_[j].m_*(pMainT_[j].m_ -1);
    sum_s_ = sum_s_ - tmp_s + pMainT_[j].s_;
    
    if (condition())
    {
      //double capaciy of Tj
      subtable_cell* sub = pMainT_[j].pSub_;
      pMainT_[j].pSub_ = new subtable_cell[pMainT_[j].s_];
      
      memcpy(pMainT_[j].pSub_, sub, tmp_s*sizeof(subtable_cell));
      delete sub;
      adjustSubTable(j, subtable_cell(x, valueIdx));
      //      finish = clock();
      //      insert_time2_ += finish-start;
      return true;
    }
    else
    {// level-1 hash function is bad.
      //cout<<"insert: level-1 hash function is bad.\n";
      //      finish = clock();
      //      insert_time2_ += finish-start;
      return rehashAll(x, valueIdx);
    }
  }

  virtual bool del(const KeyType& k)
  {
    uint64_t x = str_hash(k);
    
    update_count_++;
    uint64_t j = hashFunc(main_k_, main_u_,main_p_, mainT_size_, x);
    if (pMainT_[j].pSub_ ==NULL)
      return false;

    uint64_t sub_j = hashFunc(pMainT_[j].k_, pMainT_[j].u_, pMainT_[j].p_, pMainT_[j].s_, x);

    if (pMainT_[j].pSub_!=NULL && pMainT_[j].pSub_[sub_j].data_==x)
    {
      pMainT_[j].pSub_[sub_j].valueIndex_ = -1;
      count_--;
      pMainT_[j].b_--;
    }
    else
      return false;

    if (update_count_>M_)
      return rehashAll(0,-1);//(subtable_cell(0, -1));

    return true;
  }

  
  using AccessMethod<KeyType, ValueType>::update;//bool update(const KeyType& x, const ValueType& data)
  virtual bool update(const DataType<KeyType,ValueType>& data)
  {
    uint64_t x = str_hash(data.key);
    
    uint64_t j = hashFunc(main_k_, main_u_,main_p_, mainT_size_, x);
    
    if (pMainT_[j].pSub_ ==NULL)
      return false;
    
    uint64_t sub_j = hashFunc(pMainT_[j].k_, pMainT_[j].u_, pMainT_[j].p_, pMainT_[j].s_, x);
    
    if (pMainT_[j].pSub_!=NULL && pMainT_[j].pSub_[sub_j].data_==x)
    {
      dataVec_[pMainT_[j].pSub_[sub_j].valueIndex_] = data.value;
      return true;
    }
    

    return false;
  }

  virtual ValueType* find(const KeyType& k)
  {
    uint64_t x = str_hash(k);
    uint64_t j = hashFunc(main_k_, main_u_,main_p_, mainT_size_, x);

    if (pMainT_[j].pSub_ ==NULL || pMainT_[j].b_ == 0)
      return NULL;
    
    uint64_t sub_j = hashFunc(pMainT_[j].k_, pMainT_[j].u_, pMainT_[j].p_, pMainT_[j].s_, x);
    
    if (pMainT_[j].pSub_!=NULL && pMainT_[j].pSub_[sub_j].data_==x && pMainT_[j].pSub_[sub_j].valueIndex_!=(uint64_t) -1)
      return &(dataVec_[pMainT_[j].pSub_[sub_j].valueIndex_]);

    return NULL;
  }
  
friend ostream& operator << ( ostream& os, const SelfType& node)
  {
    for (uint64_t i=0; i<node.mainT_size_; i++)
    {
      os<<"\n-------- "<<i<<" --------\n";
      os<<node.pMainT_[i]<<"==>";
      if (node.pMainT_[i].pSub_ == NULL)
        continue;

      subtable_cell* sub = node.pMainT_[i].pSub_;
      for (uint64_t j=0; j<node.pMainT_[i].s_; j++)
      {
        if (sub[j].valueIndex_ == (uint64_t)-1)
          continue;
        os<<j<<":"<<sub[j]<<", ";
      }

      os<<endl;
    }

    os<<"\ncount_: "<<node.count_<<"  " ;
    os<<"update_count_: "<<node.update_count_<<"  ";
    os<<"M_: "<<node.M_<<"  ";
    os<<"mainT_size_: "<<node.mainT_size_<<"   ";
    os<<"main_k_: "<<node.main_k_<<endl;
    os<<"main_u_: "<<node.main_u_<<endl;

    return os;
  }

  bool save(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "w+");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }

    uint64_t k = 0;
    uint64_t arr[8] = 
      {
        k, count_, update_count_, mainT_size_, M_, main_k_, main_u_, main_p_
      }
    ;

    if (fwrite(arr, sizeof (arr), 1, f)!=1)
      return false;
    
    
    for (uint64_t i=0; i<mainT_size_; i++)
    {
      if (pMainT_[i].pSub_ == NULL)
        continue;
      k++;
      
      if (fwrite(&i, sizeof (uint64_t), 1, f)!=1)
        return false;
      
      if (!pMainT_[i].save(f))
        return false;

      subtable_cell* sub = pMainT_[i].pSub_;
      for (uint64_t j=0; j<pMainT_[i].s_; j++)
      {
        if (sub[j].valueIndex_ == (uint64_t)-1)
          continue;
        if (!sub[j].save(f))
          return false;
      }
      
    }

    fseek(f, 0, SEEK_SET);
    if (fwrite(&k, sizeof (uint64_t), 1, f)!=1)
      return false;
    
    fclose(f);

    ofstream of(valueFilename.c_str());
    oarchive oa(of);
    size_t size = dataVec_.size();
    oa << size;
    
    for(typename vector<ValueType>::iterator i =dataVec_.begin(); i!=dataVec_.end(); i++)
    {
      oa<<(*i);
    }

    of.close();
    
    return true;
  }
  
  bool load(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "r");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }

    uint64_t k = 0;
    uint64_t arr[8];
    if(fread(arr, sizeof(arr), 1, f)!=1)
      return false;

    k = arr[0];
    count_ = arr[1];
    update_count_ = arr[2];
    mainT_size_ = arr[3];
    M_ = arr[4];
    main_k_ = arr[5];
    main_u_ = arr[6];
    main_p_ = arr[7];

    if (pMainT_ != NULL)
      deleteAll();
    pMainT_ = new table_cell[mainT_size_];
    
    for (uint64_t i=0; i<k; i++)
    {
      uint64_t u = 0;
      if (fread(&u, sizeof (uint64_t), 1, f)!=1)
      {
        cout<<1<<endl;
        return false;
      }

      if (!pMainT_[u].load(f))
      {
        cout<<2<<endl;
        return false;
      }

      pMainT_[u].pSub_ = new subtable_cell[pMainT_[u].s_];
      
      for (uint64_t j=0; j<pMainT_[u].b_; j++)
      {
        subtable_cell d;
        if (!d.load(f))
        {
          cout<<3<<endl;
          return false;
        }
        
        uint64_t y = hashFunc(pMainT_[u].k_, pMainT_[u].u_, pMainT_[u].p_, pMainT_[u].s_, d.data_);
        pMainT_[u].pSub_[y] = d;
      }
      //cout<<*this;
    }
    
    fclose(f);

    dataVec_.clear();
    
    ifstream ifs(valueFilename.c_str());
    iarchive ia(ifs);
    size_t size;
    ia>>size;

    for (size_t i =0; i<size; i++)
    {
      ValueType v;
      ia>>v;
      //cout<<v;
      dataVec_.push_back(v);
    }
        
    ifs.close();
    return true;
  }
  
protected:
  /**
   *RehashAll(x) is either called by insert(x) or del(x), and then x=-1. rehashAll(x) build a new table for all elements currently in table.
   **/
  bool rehashAll(uint64_t x, uint64_t valueIdx)
  {
    subtable_cell* L = NULL;
    uint64_t idx = 0;

    if (valueIdx != (uint64_t)-1)
      L = new subtable_cell[count_+1];
    else
      L = new subtable_cell[count_];

    //copy T to L
    for (uint64_t i=0; i<mainT_size_; i++)
    {
      if (pMainT_[i].b_== 0)
        continue;

      if (pMainT_[i].pSub_ ==NULL)
      {
        LDBG_<<"rehashAll() 1:"<<x<<valueIdx;
        return false;
      }
        

      subtable_cell* sub = pMainT_[i].pSub_;
      uint64_t s = pMainT_[i].s_;
      for (uint64_t j=0; j<s; j++)
      {
        if (sub[j].valueIndex_ == (uint64_t)-1)
          continue;

        L[idx] = sub[j];
        idx++;
      }
        
    }
      
    if (L==NULL || idx != count_)
    {
      LDBG_<<"rehashAll() 2:"<<x<<valueIdx;
      return false;
    }
    
    ////////////////////////////////////////////////////////////

    if (valueIdx != (uint64_t)-1)
    {
      L[idx] = subtable_cell(x, valueIdx);
      idx++;
    }
    update_count_ = count_ = idx;

    M_ = (1+c)*(count_>4? count_: 4);

    if (pMainT_ != NULL)
      deleteAll();
    mainT_size_ = M_*SM_O;
    pMainT_ = new table_cell[mainT_size_];

    
    int t = 0;
    do
    {
      t++;

      if (t>10)
      {
        LDBG_<<"rehashAll() 3:"<<x<<valueIdx;
        t = 0;//warnings:
      }

      random_param(main_k_, main_u_, main_p_);
      
      for (uint64_t i=0; i<mainT_size_; i++)
        pMainT_[i].b_ = 0;
      
      for (uint64_t i=0; i<count_; i++)
      {
        uint64_t j= hashFunc(main_k_, main_u_, main_p_, mainT_size_, L[i].data_);
        pMainT_[j].b_++;
        pMainT_[j].m_ = pMainT_[j].b_ *2;
        uint64_t s = pMainT_[j].s_;
        pMainT_[j].s_ = 2*pMainT_[j].m_*(pMainT_[j].m_ - 1);
        sum_s_ = sum_s_ - s + pMainT_[j].s_;
      }
    }while (!condition());

    for (uint64_t i=0; i<mainT_size_; i++)
        pMainT_[i].b_ = 0;

    uint64_t count = count_;
    count_ = 0;
    for (uint64_t i=0; i<count; i++)
    {
      uint64_t j= hashFunc(main_k_, main_u_, main_p_, mainT_size_, L[i].data_);

      if (pMainT_[j].pSub_ == NULL)
      {
        pMainT_[j].pSub_ = new subtable_cell[pMainT_[j].s_];
      }

      subtable_cell* sub = pMainT_[j].pSub_;
      uint64_t u = hashFunc(pMainT_[j].k_, pMainT_[j].u_,pMainT_[j].p_, pMainT_[j].s_, L[i].data_);
      if (sub[u].valueIndex_ != (uint64_t)-1)
      {
        adjustSubTable(j, L[i]);
      }
      else
      {
        sub[u] = L[i];
        pMainT_[j].b_++;
        count_++;
      }
      
    }

    if (count_ != count)
    {
      LDBG_<<"rehashAll() 4:"<<count<<count_;
      return false;
    }
    
    delete L;

//     finish = clock();
//     rehash_time_ += finish-start;
    
    return true;
  }

  bool condition()
  {
    //clock_t start, finish;
    //start= clock();
    
    if (pMainT_==NULL)return false;

//     uint64_t s = 0;
    
//     for (uint64_t i=0; i<mainT_size_; i++)
//       s += pMainT_[i].s_;

//     finish = clock();
//     if (sum_s_ != s)
//     {
//       LDBG_<<"condition ERROR: "<<sum_s_<<s;
//     }
    
    //condition_time_ += finish-start;
    
    return sum_s_<= (32*M_*M_/mainT_size_ + 4*M_);
  }

  uint64_t str_hash(const string& x)
  {
    uint32_t convkey = 0;
    const char* str = (const char*)x.c_str();
    for (size_t i = 0; i < x.size(); i++)
      convkey = 37*convkey + *str++;

    return convkey;
  }

  uint64_t str_hash(uint64_t x)
  {
    return x;
  }
  
  uint64_t hashFunc(uint64_t a,uint64_t b, uint64_t p, uint64_t s, const uint64_t& x)
  {
    return ((a*x+b)%p)%s;
  }

  /**
     @brief adjust table when collision happened.
   */
  void adjustSubTable(uint64_t idx, const subtable_cell& x = subtable_cell(0,-1))
  {
    //cout<<idx<<" adjustSubTable()\n";
    //    clock_t start, finish;
    //    start = clock();
  
    if (idx>=mainT_size_ || idx==(uint64_t)-1 || pMainT_[idx].pSub_ == NULL)
      return;

    subtable_cell* L;
    if (x.valueIndex_ != (uint64_t)-1)
      L = new subtable_cell[pMainT_[idx].b_ + 1];
    else
      L = new subtable_cell[pMainT_[idx].b_];
    
    uint64_t u = 0;
    subtable_cell* sub = pMainT_[idx].pSub_;
    //////////////copy sub_j to list L;
    for (uint64_t i=0; i<pMainT_[idx].s_; i++)
    {
      if (sub[i].valueIndex_ == (uint64_t)-1)
        continue;

      L[u] = sub[i];
      u++;
      sub[i] = subtable_cell(0, -1);
    }

    if (u != pMainT_[idx].b_)
    {
      LDBG_<<"adjustSubTable() 1:"<<pMainT_[idx].b_<<u;
      return;
    }

    if (x.valueIndex_ != (uint64_t)-1)
    {
      L[u] = x;
      u++;
      count_++;
      pMainT_[idx].b_++;
    }

    bool well = false;
    int t = 0;
    while (!well)
    {
      t++;
      if (t>10)
      {
        LDBG_<<"adjustSubTable() 2: Warning..."<<idx;
        t = 0;//there are warnings
      }
      
      random_param(pMainT_[idx].k_, pMainT_[idx].u_, pMainT_[idx].p_);
      
      for (uint64_t i=0; i<pMainT_[idx].s_; i++)
        sub[i] = subtable_cell(0, -1);

      uint64_t i=0;
      for (; i<u; i++)
      {
        uint64_t j= hashFunc(pMainT_[idx].k_, pMainT_[idx].u_, pMainT_[idx].p_, pMainT_[idx].s_, L[i].data_ );
        //cout<<u<<"  "<<pMainT_[idx].k_<<" "<<pMainT_[idx].u_<<" "<<sub[j].data_<<" "<<L[i].data_<<endl;
        if(sub[j].valueIndex_ != (uint64_t)-1)
        {
          break;
        }

        sub[j] = L[i];
      }
      
      if (i==u)
        well = true;
    }
    
    delete L;

    //    finish = clock();
    //    adjust_time_ += finish - start;
    
  }
  
  void deleteAll()
  {
    for (uint64_t i=0; i<mainT_size_; i++)
      deleteSubT(i);    

    delete pMainT_;
    pMainT_ = NULL;
  }

  void deleteSubT(uint64_t idx)
  {
    if( pMainT_[idx].pSub_!=NULL)
      {
        delete pMainT_[idx].pSub_;
        pMainT_[idx].pSub_ = NULL;
      }

  }  
    
  static void random_param(uint64_t& a, uint64_t& b, uint64_t& p)
  {
    p = rand();
    a = rand()%p;
    b = rand()%p;
  }

protected:
  table_cell* pMainT_;//!< entry pointer
  uint64_t count_; //!< record count
  uint64_t update_count_;//!< times of updates
  uint64_t M_; //!< used for upper bounds of update times 
  uint64_t mainT_size_;//!< main table size
  uint64_t main_k_;//!< 3 parameters for main table
  uint64_t main_u_;//!< 3 parameters for main table
  uint64_t main_p_;//!< 3 parameters for main table
  vector<ValueType> dataVec_;//!< used for storing value field
  uint64_t sum_s_;//!< sumary of 2 parameters.
  
//   clock_t adjust_time_;
//   clock_t rehash_time_;
//   clock_t insert_time1_;
//   clock_t insert_time2_;
//   clock_t condition_time_;
}
  ;

NS_IZENELIB_AM_END
#endif
