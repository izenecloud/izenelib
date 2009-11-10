#ifndef HASH_TRIE
#define HASH_TRIE

#include "term_hash_table.hpp"
#include <vector>
#include <string>
#include <math.h>

NS_IZENELIB_AM_BEGIN

template<
  uint32_t ENTRY_SIZE = 1000000,
  uint32_t VALUE_CACHE_SIZE = 500//MB
  >
class HashTrie
{
#define NODE_ENTRY_SIZE_TABLE_SIZE 4

  TermHashTable *root_;
  TermPropertyVector<VALUE_CACHE_SIZE>*   value_pool_;
  TermHashTable* buf_node_;
  FILE* f_;
  uint32_t node_entry_size_[NODE_ENTRY_SIZE_TABLE_SIZE];
  uint32_t node_num_;
  
  void insert_(const std::vector<std::string>& vs, std::size_t start,  uint32_t docid)
  {
    TermHashTable* node = root_;
    for (std::size_t i=start; i<vs.size(); ++i)
    {
      uint64_t value1 = value_pool_->touch_append();
      uint64_t value2 = node->insert(vs[i].c_str(), vs[i].length(), value1);
      if (value1 == value2)//new term
      {
        if (i == vs.size()-1)//the last one
        {
          value_pool_->append(1);
          continue;
        }
        
        node = new TermHashTable(node_entry_size_[i-start+1>=NODE_ENTRY_SIZE_TABLE_SIZE? NODE_ENTRY_SIZE_TABLE_SIZE-1: i-start+1]);
        ++node_num_;
        IASSERT(value_pool_->append(1, 1, (uint64_t)node)==value1);
        continue;
      }

      value_pool_->add_freq(value2);

      if (i == vs.size()-1)//the last one
      {
        continue;
      }
      
      node = (TermHashTable*)value_pool_->get_child(value2);
      if ((uint64_t)node == (uint64_t)-1)
      {
        node = new TermHashTable(node_entry_size_[i-start+1>=NODE_ENTRY_SIZE_TABLE_SIZE? NODE_ENTRY_SIZE_TABLE_SIZE-1: i-start+1]);
        ++node_num_;
        value_pool_->set_child(value2, (uint64_t)node);
        value_pool_->set_loaded(value2, 1);
      }
    }
  }

  uint64_t size_(TermHashTable* tb)
  {
    uint64_t r= tb->size();
    ++node_num_;

    //std::cout<<"*****************\n";
    //std::cout<<*tb<<std::endl;
    //std::cout<<tb->size()<<std::endl;
    
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      uint64_t value = (*i).value();
      //std::cout<<(*i).term()<<std::endl;
      
      if (!value_pool_->get_loaded(value) || value_pool_->get_child(value)==(uint64_t)-1)
        continue;
      r += size_((TermHashTable*)(value_pool_->get_child(value)));
    }
    //std::cout<<"-----------------\n";
    
    return r;
  }

  void release_(TermHashTable* tb)
  {
    if (tb == NULL)
      return;
    
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      uint64_t value = (*i).value();
      //std::cout<<(*i).term()<<std::endl;
      
      if (!value_pool_->get_loaded(value) || value_pool_->get_child(value)==(uint64_t)-1)
        continue;
      release_((TermHashTable*)(value_pool_->get_child(value)));
    }

    delete tb;
    ++node_num_;
  }

  void save_(TermHashTable* tb)
  {
    tb->save(f_);
    
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      uint64_t value = (*i).value();
      //std::cout<<(*i).term()<<std::endl;
      
      if (!value_pool_->get_loaded(value) || value_pool_->get_child(value)==(uint64_t)-1)
        continue;

      uint64_t pos = ftell(f_);
      save_((TermHashTable*)(value_pool_->get_child(value)));
      value_pool_->set_child(value, pos);
      value_pool_->set_loaded(value, 0);
    }

    delete tb;
  }
  
  void load_(TermHashTable* tb, uint64_t addr)
  {
    tb->load(f_, addr);
    
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      uint64_t value = (*i).value();
      //std::cout<<(*i).term()<<" "<<value<<std::endl;

      uint64_t child = value_pool_->get_child(value);
      
      if (value_pool_->get_loaded(value) || child ==(uint64_t)-1)
        continue;

      TermHashTable* tht = new TermHashTable();
      ++node_num_;
      load_(tht, child);
      value_pool_->set_child(value, (uint64_t)tht);
      value_pool_->set_loaded(value, 1);
    }
  }

  
  inline void load()
  {
    uint32_t nn = 0;
    node_num_ = 1;
    
    fseek(f_, 0, SEEK_SET);
    
    IASSERT(fread(&nn, sizeof(uint32_t), 1, f_)==1);
    
    root_ = new TermHashTable(ENTRY_SIZE);
    load_(root_, ftell(f_));
    IASSERT(node_num_ == nn);

    //std::cout<<"node num loaded: "<<node_num_<<std::endl;
  }

  
public:
  HashTrie(const char* nm)
  {
    node_entry_size_[0] = ENTRY_SIZE;
    for (uint32_t i=1; i<NODE_ENTRY_SIZE_TABLE_SIZE; ++i)
    {
      node_entry_size_[i] = (uint32_t)(sqrt((double)node_entry_size_[i-1])/2+0.5);
      //std::cout<<node_entry_size_[i]<<std::endl;
    }
    
    buf_node_ = new TermHashTable();
    
    std::string s = nm;
    s += ".val";
    value_pool_ = new TermPropertyVector<VALUE_CACHE_SIZE>(s.c_str());
    
    node_num_ = 1;

    f_ = fopen(nm, "r+");
    if (f_ == NULL)
    {
      f_ = fopen(nm, "w+");
      IASSERT(fwrite(&node_num_, sizeof(uint32_t), 1, f_)==1);
      root_ = new TermHashTable(ENTRY_SIZE);
    }
    else
      load();
    
    if (f_ == NULL)
    {
      std::cout<<"Can't create file: "<<nm<<std::endl;
      return;
    }
  }

  ~HashTrie()
  {
    //std::cout<<"deleting trie!\n";
    fclose(f_);
    uint32_t nn = node_num_;
    node_num_ = 0;
    release_(root_);
    IASSERT(nn == node_num_);
    //std::cout<<"deleting value pool!\n";
    delete value_pool_;
    delete buf_node_;
  }

  inline uint32_t node_num()const
  {
    return node_num_;
  }
  
  void insert(const std::vector<std::string>& vs, uint32_t docid)
  {
    for (std::size_t i=0; i<vs.size(); ++i)
    {
      insert_(vs, i, docid);
    }

    //std::cout<<*root_<<std::endl;
  }

  uint64_t size()
  {
    std::cout<<"Value pool size: "<<value_pool_->size()<<std::endl;
    node_num_ = 0;
    return size_(root_)+value_pool_->size();
  }
  
  uint32_t get_freq(const std::vector<std::string>& vs)
  {
    TermHashTable* node = root_;
    for (std::size_t i=0; i<vs.size(); ++i)
    {
      uint64_t value = node->find(vs[i].c_str(), vs[i].length());
      if (value == (uint64_t)-1)
        return 0;

      if (i == vs.size()-1)//last one
      {
        return value_pool_->get_freq(value);
      }
      
      if (value_pool_->get_loaded(value))
      {
        node = (TermHashTable* )value_pool_->get_child(value);
        continue;
      }
      node = buf_node_;
      node->load(f_, value_pool_->get_child(value));
    }

    return 0;
  }

  bool get_suffix(const std::vector<std::string>& terms, std::vector<std::string>& suffixs, std::vector<uint32_t>& counts)
  {
    suffixs.clear();
    counts.clear();
    
    TermHashTable* node = root_;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      uint64_t value = node->find(terms[i].c_str(), terms[i].length());
      if (value == (uint64_t)-1)
        return false;

      if (i == terms.size()-1)//last one
      {
        if (value_pool_->get_loaded(value))
          node = (TermHashTable* )value_pool_->get_child(value);
        else
        {
          node = buf_node_;
          node->load(f_, value_pool_->get_child(value));
        }

        break;
      }
      
      if (value_pool_->get_loaded(value))
      {
        node = (TermHashTable* )value_pool_->get_child(value);
        continue;
      }
      node = buf_node_;
      node->load(f_,value_pool_->get_child(value));
    }
    
    for (TermHashTable::const_iterator i=node->begin(); i!=node->end(); ++i)
    {
      uint64_t va = (*i).value();
      suffixs.push_back(std::string((*i).term()));
      counts.push_back(value_pool_->get_freq(va));
    }

    return true;
  }

  inline void save()
  {
    fseek(f_, 0, SEEK_SET);
    IASSERT(fwrite(&node_num_, sizeof(uint32_t), 1, f_)==1);
    save_(root_);
    fflush(f_);
    root_ = NULL;
    node_num_ = 0;
  }
  
}
;
NS_IZENELIB_AM_END
#endif

/**
   TgPhraseExtractor(const std::string& path);
		~TgPhraseExtractor();
		void addSentence(const std::vector<termid_t>& termList, const std::vector<loc_t>& positionList, docid_t docId);
		bool analysis();
		labelid_t getPhraseId(const std::vector<termid_t>& termList);
		count_t freq(labelid_t labelId);
		bool getDocIdList(labelid_t labelId, std::vector<docid_t>& docIdList);
		private:
			SuffixNodeList begin();
			bool getAllPrefix(const std::vector<termid_t>& termList, std::vector<termid_t>& prefixList, std::vector<count_t>& countList);
			bool getAllSuffix(const std::vector<termid_t>& termList, std::vector<termid_t>& suffixList, std::vector<count_t>& countList);
 **/
