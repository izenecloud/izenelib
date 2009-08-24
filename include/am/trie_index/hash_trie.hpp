#ifndef HASH_TRIE
#define HASH_TRIE

#include "term_hash_table.hpp"
#include "doc_list.hpp"
#include <vector>
#include <string>
#include <math.h>
#include <iostream>

NS_IZENELIB_AM_BEGIN

template<
  uint32_t ENTRY_SIZE = 1000000,
  uint32_t NODE_SIZE_RATE = 10 // it indicate the reducing rate of hash table entries number for different levels in trie.
  >
class HashTrie
{
#define NODE_ENTRY_SIZE_TABLE_SIZE 4

  typedef DocList<> doc_list_t;

  TermHashTable *root_;
  TermHashTable* buf_node_;
  FILE* f_;
  FILE* doc_f_;
  uint32_t node_entry_size_[NODE_ENTRY_SIZE_TABLE_SIZE];
  uint32_t node_num_;
  
  void insert_(const std::vector<uint64_t>& terms, std::size_t start,  uint32_t docid)
  {
    TermHashTable* node = root_;
    for (std::size_t i=start; i<terms.size(); ++i)
    {
      bool is_new = false;
      char* p = node->insert(terms[i], is_new);
      Term t(p);
      if (is_new)//new term
      {
        if (i == terms.size()-1)//the last one
        {
          //add doc id
          if (t.get_doc_list() == (uint64_t)-1)
            t.set_doc_list((uint64_t)(new doc_list_t(docid)));
          else
            ((doc_list_t*)t.get_doc_list())->append(docid);

          continue;
        }
        
        node = new TermHashTable(node_entry_size_[i-start+1>=NODE_ENTRY_SIZE_TABLE_SIZE? NODE_ENTRY_SIZE_TABLE_SIZE-1: i-start+1]);
        ++node_num_;
        t.set_child((uint64_t)node);
        t.set_loaded(1);
        continue;
      }

      if (i == terms.size()-1)//the last one
      {
        //add doc id
        if (t.get_doc_list() == (uint64_t)-1)
          t.set_doc_list((uint64_t)(new doc_list_t(docid)));
        else
          ((doc_list_t*)t.get_doc_list())->append(docid);

        continue;
      }
      
      node = (TermHashTable*)t.get_child();
      if ((uint64_t)node == (uint64_t)-1)
      {
        node = new TermHashTable(node_entry_size_[i-start+1>=NODE_ENTRY_SIZE_TABLE_SIZE? NODE_ENTRY_SIZE_TABLE_SIZE-1: i-start+1]);
        ++node_num_;
        t.set_child((uint64_t)node);
        t.set_loaded(1);
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
      //std::cout<<(*i).term()<<std::endl;
      
      if (!(*i).get_loaded() || (*i).get_child()==(uint64_t)-1)
        continue;
      r += size_((TermHashTable*)((*i).get_child()));
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
      if (!(*i).get_loaded() || (*i).get_child()==(uint64_t)-1)
        continue;
      release_((TermHashTable*)((*i).get_child()));
    }

    delete tb;
    ++node_num_;
  }

  void save_(TermHashTable* tb)
  {
    uint64_t pos1 = ftell(f_);
    tb->save(f_);

    bool f = false;
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      if (!(*i).get_loaded() || (*i).get_child()==(uint64_t)-1)
        continue;

      f = true;
      uint64_t pos = ftell(f_);
      
      save_((TermHashTable*)((*i).get_child()));
      (*i).set_child(pos);
      (*i).set_loaded(0);

      uint64_t list = (*i).get_doc_list();
      if (list !=(uint64_t)-1)
      {
        (*i).set_doc_list(ftell(doc_f_));
        //std::cout<<ftell(doc_f_)<<std::endl;
        ((doc_list_t*)list)->save(doc_f_);
      }
    }

    if (f)
    {
      uint64_t pos2 = ftell(f_);
      fseek(f_, pos1, SEEK_SET);
      tb->save(f_);
      fseek(f_, pos2, SEEK_SET);
    }
    
    delete tb;
  }
  
  void load_(TermHashTable* tb, uint64_t addr)
  {
    tb->load(f_, addr);
    
    for (TermHashTable::const_iterator i=tb->begin();i!=tb->end();++i)
    {
      uint64_t child = (*i).get_child();

      if ((*i).get_loaded() || (*i).get_child()==(uint64_t)-1)
        continue;

      TermHashTable* tht = new TermHashTable();
      ++node_num_;
      load_(tht, child);
      (*i).set_child((uint64_t)tht);
      (*i).set_loaded(1);

      if ((*i).get_doc_list() == (uint64_t)-1)
        continue;
      
      doc_list_t* list = new doc_list_t(doc_f_,(*i).get_doc_list());
      (*i).set_doc_list((uint64_t)list);
    }
  }
  
  inline void load()
  {
    uint32_t nn = 0;
    node_num_ = 1;
    
    fseek(f_, 0, SEEK_SET);
    
    assert(fread(&nn, sizeof(uint32_t), 1, f_)==1);
    
    root_ = new TermHashTable(ENTRY_SIZE);
    load_(root_, ftell(f_));
    assert(node_num_ == nn);
    //std::cout<<"node num loaded: "<<node_num_<<std::endl;
  }

  void get_docs_(uint64_t tb, bool loaded, doc_list_t** list)
  {
    if (tb == (uint64_t)-1)
        return;

    TermHashTable* node = NULL;
    if (loaded)
      node = (TermHashTable* )tb;
    else
    {
      node = buf_node_;
      node->load(f_, tb);
    }
    

    std::vector<uint64_t> children;
    std::vector<bool> loads;
    
    for (TermHashTable::const_iterator i=node->begin(); i!=node->end(); ++i)
    {
      children.push_back((*i).get_child());
      loads.push_back((*i).get_loaded());
      
      if ((*i).get_doc_list() == (uint64_t)-1)
        continue;
      
      if (*list == NULL)
        *list = new doc_list_t(*(doc_list_t*)(*i).get_doc_list());
      else
        (*list)->append(*(doc_list_t*)(*i).get_doc_list());
    }

    std::vector<bool>::const_iterator j=loads.begin();
    for (std::vector<uint64_t>::const_iterator i=children.begin(); i!=children.end(); ++i, ++j)
      get_docs_(*i,*j, list);
  }
  
  
public:
  HashTrie(const char* nm)
  {
    node_entry_size_[0] = ENTRY_SIZE;
    for (uint32_t i=1; i<NODE_ENTRY_SIZE_TABLE_SIZE; ++i)
    {
      node_entry_size_[i] = (uint32_t)(sqrt((double)node_entry_size_[i-1])*i/(NODE_ENTRY_SIZE_TABLE_SIZE*NODE_SIZE_RATE)+0.5);
      if(node_entry_size_[i]==0)
        node_entry_size_[i] = 1;
      //std::cout<<node_entry_size_[i]<<std::endl;
    }
    
    buf_node_ = new TermHashTable();
    
    node_num_ = 1;

    f_ = fopen(nm, "r+");
    if (f_ == NULL)
    {
      f_ = fopen(nm, "w+");
      assert(fwrite(&node_num_, sizeof(uint32_t), 1, f_)==1);
      root_ = new TermHashTable(ENTRY_SIZE);
    }
    else
      load();
    
    if (f_ == NULL)
    {
      std::cout<<"Can't create file: "<<nm<<std::endl;
      return;
    }

    std::string str = nm;
    str += ".doc";
    doc_f_ = fopen(str.c_str(), "r+");
    if (doc_f_ == NULL)
    {
      doc_f_ = fopen(str.c_str(), "w+");
    }
    
    if (doc_f_ == NULL)
    {
      std::cout<<"Can't create file: "<<str<<std::endl;
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
    assert(nn == node_num_);
    //std::cout<<"deleting buf_node_!\n";
    delete buf_node_;
  }

  inline uint32_t node_num()const
  {
    return node_num_;
  }
  
  void insert(const std::vector<uint64_t>& terms, uint32_t docid)
  {
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      insert_(terms, i, docid);
    }

    //std::cout<<*root_<<std::endl;
  }

  uint64_t size()
  {
    uint32_t nn = node_num_;
    node_num_ = 0;
    uint64_t s = size_(root_);
    assert(nn==node_num_);

    return s;
  }
  
  uint32_t get_freq(const std::vector<uint64_t>& terms)
  {
    TermHashTable* node = root_;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      Term t(node->find(terms[i]));
      if (t.is_null())
        return 0;

      if (i == terms.size()-1)//last one
      {
        return t.get_freq();
      }
      
      if (t.get_loaded())
      {
        node = (TermHashTable* )t.get_child();
        continue;
      }
      
      node = buf_node_;
      node->load(f_, t.get_child());
    }

    return 0;
  }

  bool get_suffix(const std::vector<uint64_t>& terms, std::vector<uint64_t>& suffixs, std::vector<uint32_t>& counts)
  {
    suffixs.clear();
    counts.clear();
    
    TermHashTable* node = root_;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      Term t( node->find(terms[i]));
      if (t.is_null())
        return false;

      if (i == terms.size()-1)//last one
      {
        if (t.get_loaded())
          node = (TermHashTable* )t.get_child();
        else
        {
          node = buf_node_;
          node->load(f_, t.get_child());
        }

        break;
      }
      
      if (t.get_loaded())
      {
        node = (TermHashTable* )t.get_child();
        continue;
      }
      node = buf_node_;
      node->load(f_, t.get_child());
    }
    
    for (TermHashTable::const_iterator i=node->begin(); i!=node->end(); ++i)
    {
      suffixs.push_back((*i).get_id());
      counts.push_back((*i).get_freq());
    }

    return true;
  }

  bool get_docs(const std::vector<uint64_t>& terms, std::vector<uint32_t>& docs)
  {
    docs.clear();
    doc_list_t* list = NULL;
    
    TermHashTable* node = root_;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      Term t( node->find(terms[i]));
      if (t.is_null())
        return false;

      if (i == terms.size()-1)//last one
      {
        if (t.get_doc_list()!= (uint64_t)-1)
        {
          list = new doc_list_t(*(doc_list_t*)t.get_doc_list());
        }
        
        get_docs_(t.get_child(), t.get_loaded(), &list);
        
        if (list == NULL)
          docs.clear();
        else
          list->assign(docs);
        
        return true;
      }
      
      if (t.get_loaded())
      {
        node = (TermHashTable* )t.get_child();
        continue;
      }
      node = buf_node_;
      node->load(f_, t.get_child());
    }
    
    return false;
  }

  inline void save()
  {
    fseek(f_, 0, SEEK_SET);
    assert(fwrite(&node_num_, sizeof(uint32_t), 1, f_)==1);
    save_(root_);
    fflush(f_);
    root_ = NULL;
    node_num_ = 0;
  }

  const TermHashTable* get_root()const
  {
    return root_;
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
