#ifndef TRIE_INDEXER
#define TRIE_INDEXER

#include <types.h>
#include <vector>
#include <string>
#include "hash_trie.hpp"

NS_IZENELIB_AM_BEGIN

template <
  uint32_t DOC_NUM_PER_TRIE = 50000,//150000
  //hash trie
  uint32_t NODE_SIZE_RATE = 25, // it indicate the reducing rate of hash table entries number for different levels in trie.
  uint32_t LOAD_DEGREE = 1,
  uint32_t ENTRY_SIZE = 1000000
  >
class TrieIndexer
{
public:
  typedef HashTrie<NODE_SIZE_RATE, LOAD_DEGREE, ENTRY_SIZE> trie_t;
  typedef typename trie_t::node_t node_t;
  typedef typename trie_t::doc_list_t doc_list_t;
protected:
  struct CONFLICT_NODE
  {
    uint64_t addr1;
    uint64_t addr2;
    uint64_t obj_addr;

    inline CONFLICT_NODE(uint64_t a, uint64_t b, uint64_t c)
    {
      addr1 = a;
      addr2 = b;
      obj_addr = c;
    }
  }
    ;
  
  typedef std::vector<struct CONFLICT_NODE> conflict_q_t;

  std::string fname_;
  uint32_t doc_num_;
  uint32_t trie_num_;
  trie_t* cur_trie_;
  conflict_q_t cq_;
  uint32_t last_docid_;
  //  std::vector<trie_t*> tries_;

  void merge_nodes(node_t* a, node_t* b, node_t& c)
  {
    c = *b;
    bool is_new = false;
    for (TermHashTable::const_iterator i=a->begin(); i!=a->end(); ++i)
    {
      Term t(*i);
      Term added(c.insert(t.get_id(), is_new));
      if (!is_new)
      {
        added.set_freq(t.get_freq()+added.get_freq()-1);
      }
      else
      {
        added.set_freq(t.get_freq());
        //merge doc list
      }

//       //merge doc list
//       if(t.get_doc_list() == (uint64_t)-1)
//           continue;
      
//       if(added.get_doc_list() == (uint64_t)-1)
//       {
//         added.set_doc_list((uint64_t)(new doc_list_t(*(doc_list_t*)t.get_doc_list())));
//         continue;
//       }

//       ((doc_list_t*)added.get_doc_list())->append(*(doc_list_t*)t.get_doc_list());
      
    }
  }

  uint32_t save_node(FILE* f, FILE* doc_f, uint64_t& addr, node_t* nodeA, node_t* nodeB,
                     node_t* obj, trie_t& a, trie_t& b)
  {
    uint32_t node_num = 1;
    uint32_t cqs = cq_.size();
    
    for (TermHashTable::const_iterator i=obj->begin(); i!=obj->end(); ++i)
    {
      Term ta(nodeA->find((*i).get_id()));
      Term tb(nodeB->find((*i).get_id()));

      // conflicting ones existing in both tree and all have sub tree
      if ((!ta.is_null()) && (!tb.is_null()) &&
          ta.get_child()!= (uint64_t)-1 && tb.get_child()!= (uint64_t)-1)
      {
        //std::cout<<"LLLLLL "<<ta.get_child()<<" "<<tb.get_child()<<" "<<i.file_pos()+addr<<std::endl;
        //add to conflict queue
        cq_.push_back(CONFLICT_NODE(ta.get_child(), tb.get_child(), i.file_pos()+13));
        continue;
      }

      //save children
      if (!ta.is_null() && ta.get_child() !=(uint64_t)-1)
      {
        //std::cout<<"tt "<<ta.get_child()<<std::endl;
        //load its sub tree for resaving
        a.partial_load(ta.get_child());
        //std::cout<<"ttttttt\n";
        
        node_num += a.node_num();
        
          //save its sub tree
        (*i).set_child(a.partial_save(f, doc_f));
      }

      if (!tb.is_null() && tb.get_child() !=(uint64_t)-1)
      {        
        //std::cout<<"oo"<<tb.get_child()<<std::endl;

        //load its sub tree for resaving
        b.partial_load(tb.get_child());
        //std::cout<<"uuuuuuuuu\n";
        
        node_num += b.node_num();
        
          //save its sub tree
        (*i).set_child(b.partial_save(f, doc_f));
      }
    }

    //save the mergee
    addr = ftell(f);
    obj->save(f);
    for (uint32_t i=cqs; i<cq_.size(); ++i)
      cq_[i].obj_addr += addr;

    return node_num;
  }
  
  void merge_tree(trie_t& a, trie_t& b)
  {
    //open mergee files
    FILE* f = fopen((fname_+"_o").c_str(), "w+");
    FILE* doc_f = fopen((fname_+"_o.doc").c_str(), "w+");

    assert(f!=NULL);
    assert(doc_f!=NULL);
    fseek(f, sizeof(uint32_t)+sizeof(uint64_t), SEEK_SET);

    uint32_t node_num = 0;
    uint64_t root_addr = -1;
    for (uint32_t i=0; i<cq_.size();++i )
    {
      node_t obj;
      
      //std::cout<<cq_[i].addr1<<" ----------\n";
      node_t* nodeA = a.read_node(cq_[i].addr1);
      //std::cout<<cq_[i].addr2<<" ===========\n";
      node_t* nodeB = b.read_node(cq_[i].addr2);
      //std::cout<<"kkkkkkkkkkkkk\n";
      
      //merge frequency to mergee
      merge_nodes(nodeA, nodeB, obj);

      uint64_t node_pos = -1;//ftell(f);
      
      //save unconflict sub tree and add conflicting ones to queue
      node_num +=save_node(f, doc_f, node_pos, nodeA, nodeB, &obj, a, b);
      
      if (cq_[i].obj_addr == (uint64_t)-1)
      {
        root_addr = node_pos;
        continue;
      }
      //set parent
      uint64_t pos = ftell(f);
      //std::cout<<node_pos<<" hhhhhhh\n";
      fseek(f, cq_[i].obj_addr, SEEK_SET);
      assert(fwrite(&node_pos, sizeof(uint64_t), 1, f)==1);
      fseek(f, pos, SEEK_SET);
    }

    //std::cout<<"root_addr: "<<root_addr<<std::endl;
    fseek(f, 0, SEEK_SET);
    assert(fwrite(&node_num, sizeof(uint32_t), 1, f) == 1);
    assert(fwrite(&root_addr, sizeof(uint64_t), 1, f) == 1);
    
    fclose(f);
    fclose(doc_f);
  }

public:
  
  inline TrieIndexer(const char* nm)
    :fname_(nm), doc_num_(0), trie_num_(0), cur_trie_(NULL), last_docid_(-1)
  {
  }

  inline ~TrieIndexer()
  {
    if (cur_trie_!=NULL)
    {
      cur_trie_->save();
      delete cur_trie_;
      cur_trie_ = NULL;
    }
  }

  void insert(const std::vector<uint64_t>& terms, uint32_t docid)
  {
    if (cur_trie_ == NULL)
    {
      char buf[6];
      sprintf(buf, "_%d", trie_num_);
      std::string nm = fname_ + buf;
      cur_trie_ = new trie_t(nm.c_str());
      ++trie_num_;

      ++doc_num_;
      last_docid_ = docid;
    }
    
    if (last_docid_ != docid)
    {
      if (doc_num_%DOC_NUM_PER_TRIE == 0)
      {
        //std::cout<<doc_num_<<" "<<docid<<std::endl;
        cur_trie_->save();
        delete cur_trie_;

        char buf[6];
        sprintf(buf, "_%d", trie_num_);
        std::string nm = fname_ + buf;
        cur_trie_ = new trie_t(nm.c_str());
        ++trie_num_;
        //tries_.push_back(cur_trie_);
      }
      
      ++doc_num_;
      last_docid_ = docid;

    }
        
    cur_trie_->insert(terms, docid);
  }

  inline uint32_t doc_num()const
  {
    return doc_num_;
  }
  
  void merge()
  {
    if (cur_trie_!=NULL)
    {
      cur_trie_->save();
      delete cur_trie_;
      cur_trie_ = NULL;
    }
    if (trie_num_ == 1)
      return;
    
    trie_t* big = new trie_t((fname_+"_0").c_str(), false);
    static char buf[6];
    
    //for each small trie tree
    for (uint32_t i=1; i<trie_num_; ++i)
    {
      cq_.clear();
      sprintf(buf, "_%d", i);
      std::string nm = fname_ + buf;
      trie_t* small = new trie_t(nm.c_str(), false);

      //std::cout<<"HHHH "<<big->get_root_pos()<<"=="<<small->get_root_pos()<<std::endl;
      //initiate conflict queue with root nodes of both tree
      cq_.push_back(CONFLICT_NODE(big->get_root_pos(), small->get_root_pos(), -1));

      merge_tree(*big, *small);
      
      delete big;
      delete small;
      
      //remove ith files
      remove(nm.c_str());
      remove((nm+".doc").c_str());
      
      // remane merged files to i th
      rename((fname_+"_o").c_str(), nm.c_str());
      rename((fname_+"_o.doc").c_str(), (nm+".doc").c_str());

      //reopen the mergee
      big = new trie_t(nm.c_str(), false);

      //std::cout<<"pppppppppppppppppppppp\n";
      //remove i-1 th files
      sprintf(buf, "_%d", i-1);
      nm = fname_ + buf;
      remove(nm.c_str());
      remove((nm+".doc").c_str());
    }

    delete big;

    sprintf(buf, "_%d", trie_num_-1);
    std::string nm = fname_ + buf;
    rename(nm.c_str(), (fname_+"_0").c_str());
    rename((nm+".doc").c_str(), (fname_+"_0.doc").c_str());
    trie_num_ = 1;
  }

  void load()
  {
    if (cur_trie_ != NULL)
    {
      cur_trie_->save();
      delete cur_trie_;
    }

    cur_trie_ = new trie_t((fname_+"_0").c_str());
  }

  void ratio_load()
  {
    if (cur_trie_ != NULL)
    {
      cur_trie_->save();
      delete cur_trie_;
    }

    cur_trie_ = new trie_t((fname_+"_0").c_str(), false);
    cur_trie_->ratio_load(80);
  }

  void ratio_release()
  {
    if (cur_trie_ != NULL)
    {
      cur_trie_->ratio_release();
      delete cur_trie_;
      cur_trie_  = NULL;
    }
  }
  
  
  uint64_t load_sub_tree(Term t, trie_t** tree)
  {
    *tree = NULL;
    
    if (t.get_loaded() || t.get_child() == (uint64_t)-1)
      return -1;

    *tree = new trie_t((fname_+"_0").c_str(), false);

    t.set_loaded(1);
    uint64_t r = t.get_child();
    t.set_child((uint64_t)(*tree)->partial_load(t.get_child()));

    return r;
  }

  void release_sub_tree(Term t, trie_t** tree, uint64_t child)
  {
    if (!t.get_loaded() || t.get_child() == (uint64_t)-1 || *tree == NULL)
      return;

    delete *tree;
    t.set_loaded(0);
    t.set_child(child);
  }
  
  
  inline uint32_t node_num()const
  {
    return cur_trie_->node_num();
  }
  
  inline uint64_t size() 
  {
    return cur_trie_->size();
  }
  
  inline uint32_t get_freq(const std::vector<uint64_t>& terms)
  {
    return cur_trie_->get_freq(terms);
  }

  inline bool get_suffix(const std::vector<uint64_t>& terms, std::vector<uint64_t>& suffixs, std::vector<uint32_t>& counts)
  {
    return cur_trie_->get_suffix(terms, suffixs, counts);
  }

  inline bool get_docs(const std::vector<uint64_t>& terms, std::vector<uint32_t>& docs)
  {
    return cur_trie_->get_docs(terms, docs);
  }

  inline TermHashTable* get_root()
  {
    return cur_trie_->get_root();
  }
  
  inline TermHashTable* load_root()
  {
    if (cur_trie_ != NULL)
    {
      delete cur_trie_;
    }

    cur_trie_ = new trie_t((fname_+"_0").c_str(), false);
    return cur_trie_->get_root();
  }
  
};

NS_IZENELIB_AM_END

#endif
