#ifndef GRAPH_HPP
#define GRAPH_HPP

#include<types.h>
#include "sorter.hpp"
#include "dyn_array.hpp"
#include "id_transfer.hpp"
#include "addr_bucket.hpp"
#include<string>
#include<vector>
#include <time.h>

NS_IZENELIB_AM_BEGIN

template<typename VALUE_TYPE = uint32_t>
struct FREQ_STRUCT
{
  char freq[4];
  char nid[sizeof(VALUE_TYPE)];

  uint32_t& FREQ_()
  {
    return *(uint32_t*)freq;
  }

  VALUE_TYPE& NID_()
  {
    return *(VALUE_TYPE*)nid;
  }

  uint32_t FREQ()const
  {
    return *(uint32_t*)freq;
  }

  VALUE_TYPE NID()const
  {
    return *(VALUE_TYPE*)nid;
  }
  
  inline FREQ_STRUCT(uint32_t i, VALUE_TYPE j)
  {
    FREQ_() = i;
    NID_() = j;
  }
  
  inline FREQ_STRUCT(uint32_t i)
  {
    FREQ_() = i;
    NID_() = 0;
  }
  
  inline FREQ_STRUCT()
  {
    FREQ_() = 0;
    NID_() = 0;
  }
  
  inline FREQ_STRUCT(const FREQ_STRUCT& other)
  {
    FREQ_() = other.FREQ();
    NID_() = other.NID();
  }

  inline FREQ_STRUCT& operator = (const FREQ_STRUCT& other)
  {
    FREQ_() = other.FREQ();
    NID_() = other.NID();
    return *this;
  }

  inline bool operator == (const FREQ_STRUCT& other)const
  {
    return (FREQ() == other.FREQ());
  }

  inline bool operator != (const FREQ_STRUCT& other)const
  {
    return (FREQ() != other.FREQ());
  }

  inline bool operator < (const FREQ_STRUCT& other)const 
  {
    return (FREQ() > other.FREQ());
  }

  inline bool operator > (const FREQ_STRUCT& other)const
  {
    return (FREQ() < other.FREQ());
  }

  inline bool operator <= (const FREQ_STRUCT& other)const 
  {
    return (FREQ() >= other.FREQ());
  }

  inline bool operator >= (const FREQ_STRUCT& other)const
  {
    return (FREQ() <= other.FREQ());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (FREQ() % e);
  }

  
friend std::ostream& operator << (std::ostream& os, const FREQ_STRUCT& v)
  {
    os<<"<"<<v.FREQ()<<","<<v.NID()<<">";
    return os;
  }

  
  
}
  ;

template<typename VALUE_TYPE = uint32_t>
struct EDGE_STRUCT
{
  char edge[4];
  char nid[sizeof(VALUE_TYPE)];

  uint32_t& EDGE_()
  {
    return *(uint32_t*)edge;
  }

  VALUE_TYPE& NID_()
  {
    return *(VALUE_TYPE*)nid;
  }

  uint32_t EDGE()const
  {
    return *(uint32_t*)edge;
  }

  VALUE_TYPE NID()const
  {
    return *(VALUE_TYPE*)nid;
  }
  
  inline EDGE_STRUCT(uint32_t i, VALUE_TYPE j)
  {
    EDGE_() = i;
    NID_() = j;
  }
  
  inline EDGE_STRUCT(uint32_t i)
  {
    EDGE_() = i;
    NID_() = 0;
  }
  
  inline EDGE_STRUCT()
  {
    EDGE_() = 0;
    NID_() = 0;
  }
  
  inline EDGE_STRUCT(const EDGE_STRUCT& other)
  {
    EDGE_() = other.EDGE();
    NID_() = other.NID();
  }

  inline EDGE_STRUCT& operator = (const EDGE_STRUCT& other)
  {
    EDGE_() = other.EDGE();
    NID_() = other.NID();
    return *this;
  }

  inline bool operator == (const EDGE_STRUCT& other)const
  {
    return (EDGE() == other.EDGE());
  }

  inline bool operator != (const EDGE_STRUCT& other)const
  {
    return (EDGE() != other.EDGE());
  }

  inline bool operator < (const EDGE_STRUCT& other)const 
  {
    return (EDGE() < other.EDGE());
  }

  inline bool operator > (const EDGE_STRUCT& other)const
  {
    return (EDGE() > other.EDGE());
  }

  inline bool operator <= (const EDGE_STRUCT& other)const 
  {
    return (EDGE() <= other.EDGE());
  }

  inline bool operator >= (const EDGE_STRUCT& other)const
  {
    return (EDGE() >= other.EDGE());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (EDGE() % e);
  }

friend std::ostream& operator << (std::ostream& os, const EDGE_STRUCT& v)
  {
    os<<"<"<<v.EDGE()<<","<<v.NID()<<">";
    return os;
  }
  
  
  
}
  ;


template<
  uint32_t BUCKET_NUM = 800,//used for alpha sort, number of inputfiles
  uint32_t BATCH_SAVE_SIZE = 100, //number of branch of root for one time saving.
  class TERM_TYPE = uint32_t,
  //uint32_t SAVE_RATIO = 500,//number of branch for saving
  class NID_LEN_TYPE = uint32_t,//the length type of nodes table
  uint32_t ADDING_BUF_SIZE = 100000000//used for adding and fetch terms
  >
class Graph
{
  typedef Graph<BUCKET_NUM, BATCH_SAVE_SIZE, TERM_TYPE, NID_LEN_TYPE, ADDING_BUF_SIZE> self_t;
  
  typedef DynArray<uint64_t> array64_t;
  typedef DynArray<uint32_t> array32_t;
  
  typedef Sorter<BUCKET_NUM, ADDING_BUF_SIZE, TERM_TYPE> sorter_t;
  
  typedef EDGE_STRUCT<NID_LEN_TYPE> edge_t;
  typedef FREQ_STRUCT<NID_LEN_TYPE> sort_freq_t;
  
  typedef DynArray<edge_t> edges_t;
  typedef DynArray<edge_t, true> sorted_edges_t;
  typedef DynArray<sorted_edges_t*, false, 1, false, NID_LEN_TYPE> nids_t;

  typedef DynArray<uint64_t, false, 1, false, NID_LEN_TYPE> docs_t;
  typedef DynArray<uint32_t, false, 1, false, NID_LEN_TYPE> freqs_t;
  typedef DynArray<uint8_t, false, 1, false, NID_LEN_TYPE> loads_t;

  typedef DynArray<sort_freq_t, false, 1, true, NID_LEN_TYPE> sort_freqs_t;

  nids_t nodes_;
  freqs_t freqs_;
  loads_t  loads_;
  docs_t docs_;
  //IdTransfer<60000> id_mgr_;
  uint32_t doc_num_;
  sorter_t* sorter_;
  std::string filenm_;
  FILE*  nid_f_;
  FILE*  doc_f_;

  NID_LEN_TYPE insert_(TERM_TYPE term, NID_LEN_TYPE nid)
  {
    assert (nodes_.length() == freqs_.length());
    assert (nodes_.length() == loads_.length());
    assert (nodes_.length() == docs_.length());
    
    if (nid >= nodes_.length())
    {
      nodes_.push_back(new sorted_edges_t());
      nid = nodes_.length()-1;
      
      nodes_.at(nid)->push_back(edge_t(term, nodes_.length()));
      
      freqs_.push_back(1);
      
      loads_.push_back(1);
            
      docs_.push_back(0);

      return nodes_.length();
    }
    
    sorted_edges_t* edges = nodes_.at(nid);
    if (edges == (sorted_edges_t*)-1)
      edges = nodes_[nid] = new sorted_edges_t();
    
    ++freqs_[nid];

    //exist
    if (nid == 0)
    {
      if (edges->length()>0 && edges->back() == edge_t(term))
      {
        return edges->back().NID();
      }
      
      edges->push_back(edge_t(term, nodes_.length()));
      return nodes_.length();
    }
    
    typename sorted_edges_t::size_t i  = edges->find(edge_t(term));
    if (sorted_edges_t::NOT_FOUND != i)
    {
      return edges->at(i).NID();
    }
    
    edges->push_back(edge_t(term, nodes_.length()));
    return nodes_.length();
  }

  void save_edge_(FILE* nid_f, FILE* doc_f, NID_LEN_TYPE nid, bool root = false)
  {
    if (!loads_.at(nid))
      return;
    
    uint64_t addr = -1;
    if (nid!=0 || root)
    {
      loads_[nid] = 0;
      if (docs_.at(nid) != 0)
      {
        addr = ftell(doc_f);
        ((array32_t*)docs_.at(nid))->save(doc_f);
        delete (array32_t*)docs_.at(nid);
      }
      
      docs_[nid] = addr;
    }
    
    
    if (nodes_.at(nid) == (sorted_edges_t*)-1)
      return;

    sorted_edges_t* e = nodes_.at(nid);
    for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
    {
      save_edge_(nid_f, doc_f, e->at(i).NID(), root);
    }

    if (nid==0 && !root)
      return;

    addr = ftell(nid_f);
    e->save(nid_f);
    delete e;
    nodes_[nid] = (sorted_edges_t*)addr;

    //     for (NID_LEN_TYPE i=start; i<loads_.length(); ++i)
//     {
//       if (!loads_.at(i))
//         continue;
      
//       loads_[i] = 0;
//       sorted_edges_t* e = nodes_.at(i);
//       if (e != (sorted_edges_t*)-1)
//       {
//         uint64_t addr = ftell(nid_f);
//         e->save(nid_f);
//         delete e;
//         nodes_[i] = (sorted_edges_t*)addr;
//       }

//       uint64_t addr = -1;
//       if (docs_.at(i) != 0)
//       {
//         addr = ftell(doc_f);
//         ((array32_t*)docs_.at(i))->save(doc_f);
//         delete (array32_t*)docs_.at(i);
//       }
      
//       docs_[i] = addr;
//     }
    
  }

  void load_edge_(NID_LEN_TYPE nid, double ratio)
  {
    if (loads_.at(nid))
      return;

    sorted_edges_t* e = new sorted_edges_t();
    e->load(nid_f_, (uint64_t)nodes_.at(nid));
    nodes_[nid] = e;
    loads_[nid] = 1;

    sort_freqs_t sf;
    for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
      sf.push_back(sort_freq_t(freqs_.at(e->at(i).NID()), e->at(i).NID()));

    sf.sort();
    typename sort_freqs_t::size_t len = (typename sort_freqs_t::size_t)(sf.length()*ratio+0.5);
    for (typename sort_freqs_t::size_t i=0; i<len; ++i)
      load_edge_(sf.at(i).NID(), ratio);
  }

  NID_LEN_TYPE get_next_(NID_LEN_TYPE nid, TERM_TYPE term)const
  {
    if (nid>=nodes_.length())
      return -1;

    if (loads_.at(nid))
    {
      edge_t e(term);
      typename sorted_edges_t::size_t i = ((sorted_edges_t*)nodes_.at(nid))->find(e);
      
      //std::cout<<e<<"+"<<nid<<"+"<<i<<std::endl;

      if (i == sorted_edges_t::NOT_FOUND)
        return -1;
      return nodes_.at(nid)->at(i).NID();
    }

    if (nodes_.at(nid) == (sorted_edges_t*)-1)
      return -1;
    
    sorted_edges_t edges;
    edges.load(nid_f_, (uint64_t)nodes_.at(nid));

    edge_t e(term);
    typename sorted_edges_t::size_t i = edges.find(e);
    if (i == sorted_edges_t::NOT_FOUND)
      return -1;
    return edges.at(i).NID();
  }

  void get_docs_(NID_LEN_TYPE nid, array32_t& doclist)
  {
    array32_t docs;
    if (docs_.at(nid)!= (uint64_t)-1)
      docs.load(doc_f_, docs_.at(nid));
    
    if (loads_.at(nid))
    {
      edges_t* edges = nodes_.at(nid);
      
      for (typename edges_t::size_t i =0; i<edges->length(); ++i)
        get_docs_(edges->at(i).NID(), docs);

      doclist += docs;
      return;
    }

    if (nodes_.at(nid) == (edges_t*)-1)
    {
      doclist += docs;
      return;
    }
    
    edges_t edges;
    edges.load(nid_f_, (uint64_t)nodes_.at(nid));
    for (typename edges_t::size_t i = 0; i<edges.length(); ++i)
      get_docs_(edges.at(i).NID(), docs);
    doclist += docs;
  }

  uint32_t min_(sort_freq_t* array)
  {
    uint32_t i  = 0;
    uint32_t min = 0;

    for (uint32_t k=0;k<BUCKET_NUM;++k)
    {
      if (array[k].FREQ()>min)
      {
        i = k;
        min = array[k].FREQ();
      }
      
    }

    return i;
  }
  
//   void freq_sort_(sort_freqs_t& sf, float ratio)
//   {
//     typedef FileDataBucket<sort_freq_t, 10000> bucket_t;

//     sort_freqs_t freq1;
//     bucket_t*  buckets[BUCKET_NUM];
//     sf.clear();

//     for (uint32_t i=0; i<BUCKET_NUM; ++i)
//     {
//       std::stringstream ss;
//       ss<< (filenm_+".freq.buc.")<<i<<std::endl;
//       std::string tmp;
//       ss >> tmp;

//       buckets[i] = new bucket_t(tmp.c_str());
//       buckets[i]->ready4add();
//     }
        
//     for (NID_LEN_TYPE i=0; i<freqs_.length(); ++i)
//     {
//       if (freqs_.at(i)==1)
//       {
//         freq1.push_back(sort_freq_t(1, i));
//         continue;
//       }
      
//       buckets[freqs_.at(i)%BUCKET_NUM]->push_back(sort_freq_t(freqs_.at(i), i));
//     }

//     for (uint32_t i=0; i<BUCKET_NUM; ++i)
//     {
//       buckets[i]->flush();
//       buckets[i]->sort();
//     }

//     for (uint32_t i=0; i<BUCKET_NUM; ++i)
//       buckets[i]->ready4fetch();
    
//     sort_freq_t firsts[BUCKET_NUM];
//     uint64_t    index[BUCKET_NUM];

//     //uint32_t finished = 0;
//     for (uint32_t i=0; i<BUCKET_NUM; ++i)
//     {
//       if (buckets[i]->num()>0)
//         firsts[i] = buckets[i]->next();
//       else
//         firsts[i] = 0;
      
//       index[i] = 1;
//     }


//     typename  sort_freqs_t::size_t len = (typename sort_freqs_t::size_t)(freqs_.length()*ratio+.5);
//     sf.reserve(len);


//     uint32_t last = 0;
//     while (1)
//     {
//       last = min_(firsts);
//       assert(last<BUCKET_NUM);
      
//       if (firsts[last].FREQ() == 0||len<=sf.length())
//         break;
      
//       sf.push_back(firsts[last]);

//       if (index[last]>=buckets[last]->num())
//       {
//         firsts[last] = 0;
//         continue;
//       }

//       firsts[last] = buckets[last]->next();
//       ++index[last];
//     }

//     for (uint32_t i=0; i<BUCKET_NUM; ++i)
//     {
//       //assert(firsts[i].FREQ()==0);
//       buckets[i]->dump();
//       delete buckets[i];
//     }

//     sf += freq1;

//   }
  
public:
  inline Graph(const char* nm)
  {
    filenm_ = nm;
    doc_num_ = 0;
  }

  ~Graph()
  {
    for (NID_LEN_TYPE i=0; i<loads_.length(); ++i)
      if (loads_.at(i))
        delete nodes_.at(i);
  }
  
  void ready4add()
  {
    sorter_ = new sorter_t(filenm_.c_str());
    sorter_->ready4add();
  }

  uint32_t doc_num()const
  {
    return doc_num_;
  }
  
  void add_terms(std::vector<uint32_t> terms, uint32_t docid)
  {
    static uint32_t last = -1;
    if (docid != last)
    {
      last = docid;
      ++doc_num_;
    }

    //id transfer
    array32_t ids;
    ids.reserve(terms.size());
    for (uint32_t i=0; i<terms.size(); ++i)
      ids.push_back(terms[i]/*id_mgr_.insert(terms[i])*/);

    assert(ids.length() == terms.size());

    sorter_->add_terms(ids, docid);
  }

  void indexing()
  {
    struct timeval tvafter,tvpre;
    struct timezone tz;

    //sorter_ = new sorter_t(filenm_.c_str());
    gettimeofday (&tvpre , &tz);
    
    //FILE* id_f = fopen((filenm_+".id").c_str(), "w+");
    //id_mgr_.save(id_f);
    //fclose(id_f);

    sorter_->flush();

    sorter_->sort();

    gettimeofday (&tvafter , &tz);
    std::cout<<"\nSorting is done. ("<<doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

    std::cout<<"Indexing is going on...\n";

    FILE* nid_f = fopen((filenm_+".nid").c_str(), "w+");
    FILE* doc_f = fopen((filenm_+".doc").c_str(), "w+");

    sorter_->ready4fetch();
    

    TERM_TYPE last_term = 0;
    uint32_t  batch_size = 0;
    
    array32_t t;
    uint32_t docid;

    nodes_.reserve(13000000);
    docs_.reserve(13000000);
    loads_.reserve(13000000);
    freqs_.reserve(13000000);
    for (uint32_t i=0; i<sorter_->num(); ++i)
    {
      sorter_->next(t, docid);
      assert(last_term<=t.at(0));

      if (last_term != t.at(0))
      {
        last_term = t.at(0);
        ++batch_size;

        if (batch_size > BATCH_SAVE_SIZE)
        {
          save_edge_(nid_f, doc_f, 0);
          batch_size = 0;
        }
      }
      
          
      NID_LEN_TYPE next = 0;
      for (array32_t::size_t j=0; j<t.length(); ++j)
      {
        next = insert_(t.at(j), next);

        //docid should be added at the last term
        if (j == t.length()-1)
        {
          assert(next <= nodes_.length());
          if (next == nodes_.length())
          {
            freqs_.push_back(0);
            loads_.push_back(1);
            docs_.push_back((uint64_t)(new array32_t()));
            nodes_.push_back((sorted_edges_t*)-1);
          }

          ++freqs_[next];
          
          if (docs_.at(next) == 0)
            docs_[next] = (uint64_t)(new array32_t());
          
          ((array32_t*)docs_.at(next))->push_back(docid);
        }
      }
    }

    //std::cout<<*this;
    
    save_edge_(nid_f, doc_f, 0, true);
    fclose(nid_f);
    fclose(doc_f);

    FILE* v_f = fopen((filenm_+".v").c_str(), "w+");
    nodes_.save(v_f);
    freqs_.save(v_f);
    docs_.save(v_f);
    fclose(v_f);

    nodes_.clear();
    freqs_.clear();
    docs_.clear();
    
    delete sorter_;
    sorter_ = NULL;
  }

  void ratio_load(double ratio = 0.9)
  {
    assert(ratio <= 1.);
    FILE* v_f = fopen((filenm_+".v").c_str(), "r");
    nodes_.load(v_f);
    freqs_.load(v_f);
    docs_.load(v_f);
    fclose(v_f);

    //FILE* id_f = fopen((filenm_+".id").c_str(), "r");
    //id_mgr_.load(id_f);
    //fclose(id_f);


    loads_.clear();
    loads_.reserve(nodes_.length());
    for (NID_LEN_TYPE i=0; i<nodes_.length(); ++i)
      loads_.push_back(0);
      
    
    nid_f_ = fopen((filenm_+".nid").c_str(), "r");
    doc_f_ = fopen((filenm_+".doc").c_str(), "r");

    load_edge_(0, ratio);
//     typename  sort_freqs_t::size_t len = (typename sort_freqs_t::size_t)(freqs_.length()*ratio+.5);

//     NID_LEN_TYPE loaded = 0;
//     for (NID_LEN_TYPE i=0; i<len && i<freqs_.length(); ++i)
//     {
//       if (freqs_.at(i)==1)
//         continue;
//       if (nodes_.at(i)==(sorted_edges_t*)-1)
//       {
//         len++;
//         continue;
//       }
      
//       sorted_edges_t* e = new sorted_edges_t();
//       e->load(nid_f_, (uint64_t)nodes_.at(i));
//       nodes_[i] = e;
//       loads_[i] = 1;
//       ++loaded;
//     }

//     for (NID_LEN_TYPE i=0; i<len && i<freqs_.length() && loaded<len; ++i)
//     {
//       if (loads_.at(i))
//         continue;

//       if (nodes_.at(i)==(sorted_edges_t*)-1)
//       {
//         len++;
//         continue;
//       }
      
//       sorted_edges_t* e = new sorted_edges_t();
//       e->load(nid_f_, (uint64_t)nodes_.at(i));
//       nodes_[i] = e;
//       loads_[i] = 1;
//       ++loaded;
//     }
  }

  uint32_t get_freq(const std::vector<uint32_t>& terms)const
  {
    NID_LEN_TYPE next = 0;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      //std::cout<<terms[i]<<"-"<<id_mgr_.get32(terms[i])<<std::endl;
      
      next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
      //std::cout<<next<<std::endl;
      
      if (next == (NID_LEN_TYPE)-1)
        return 0;
    }

    return freqs_.at(next);
  }

  void get_suffix(const std::vector<uint32_t>& terms, std::vector<uint32_t>& suffix, std::vector<uint32_t>& freqs)const
  {
    freqs.clear();
    suffix.clear();
    
    NID_LEN_TYPE next = 0;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
      if (next == (NID_LEN_TYPE)-1)
        return ;
    }
    
    
    if (loads_.at(next))
    {
      sorted_edges_t* edges = nodes_.at(next);

      suffix.reserve(edges->length());
      freqs.reserve(edges->length());
      for (typename edges_t::size_t i =0; i<edges->length(); ++i)
      {
        suffix.push_back(edges->at(i).EDGE());//(id_mgr_.get64(edges->at(i).EDGE()));
        freqs.push_back(freqs_.at(edges->at(i).NID()));
      }

      return;
    }

    if (nodes_.at(next) == (sorted_edges_t*)-1)
      return ;
    
    edges_t edges;
    edges.load(nid_f_, (uint64_t)nodes_.at(next));

    suffix.reserve(edges.length());
    freqs.reserve(edges.length());
    for (typename edges_t::size_t i =0; i<edges.length(); ++i)
    {
      suffix.push_back(edges.at(i).EDGE());//(id_mgr_.get64(edges.at(i).EDGE()));
      freqs.push_back(freqs_.at(edges.at(i).NID()));
    }
  }

  void get_doc_list(const std::vector<uint32_t>& terms, std::vector<uint32_t>& docids)const
  {
    docids.clear();

    NID_LEN_TYPE next = 0;
    for (std::size_t i=0; i<terms.size(); ++i)
    {
      next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
      if (next == (NID_LEN_TYPE)-1)
        return ;
    }

    array32_t docs;
    if (docs_.at(next)!= (uint64_t)-1)
      docs.load(doc_f_, docs_.at(next));
    
    if (loads_.at(next))
    {
      edges_t* edges = nodes_.at(next);
      
      for (typename edges_t::size_t i =0; i<edges->length(); ++i)
        get_docs_(edges->at(i).NID(), docs);

      docids.reserve(docs.length());
      for (array32_t::size_t i=0; i<docs.length(); ++i)
        docids.push_back(docs.at(i));
      
      return;
    }

    if (nodes_.at(next) == (edges_t*)-1)
    {
      docids.reserve(docs.length());
      for (typename array32_t::size_t i=0; i<docs.length(); ++i)
        docids.push_back(docs.at(i));
      return;
    }
    
    edges_t edges;
    edges.load(nid_f_, (uint64_t)nodes_.at(next));
    for (typename edges_t::size_t i =0;i<edges.length(); ++i)
      get_docs_(edges.at(i).NID(), docs);

    docids.reserve(docs.length());
    for (typename array32_t::size_t i=0; i<docs.length(); ++i)
      docids.push_back(docs.at(i));
      
  }

friend std::ostream& operator <<(std::ostream& os, const self_t& g)
    {
      //os<<g.nodes_.length()<<std::endl;
      for (NID_LEN_TYPE i=0; i<g.nodes_.length(); ++i)
      {
        os<<"----"<<i<<"-----\n";
        
        if ((uint64_t)g.nodes_.at(i) == (uint64_t)-1)
          continue;

        if (g.loads_.at(i))
        {
          os <<"+"<<g.freqs_.at(i)<<"|"<<*g.nodes_.at(i)<<std::endl;
          continue;
        }
        
        sorted_edges_t edges;
        edges.load(g.nid_f_, (uint64_t)g.nodes_.at(i));
        os<<g.freqs_.at(i)<<"|"<<edges<<std::endl;
      }
      
      return os;
    }
  

  class NodeIterator;
  
  class Node
  {
    const self_t* graph_;
    typename self_t::edge_t edge_;
    sorted_edges_t edges_;

  public:
    inline Node(const self_t* graph, typename self_t::edge_t edge)
      :graph_(graph),edge_(edge)
    {
      if (graph_->nodes_.at(edge_.NID())== (sorted_edges_t*)-1)
        return ;
      
      if (graph_->loads_.at(edge_.NID()))
      {
        edges_ = *graph_->nodes_.at(edge_.NID());
        return;
      }

      edges_.load(graph_->nid_f_, (uint64_t)graph_->nodes_.at(edge_.NID()));

    }

    Node& operator  = (const Node& node)
    {
      graph_ = node.graph_;
      edge_ = node.edge_;
      edges_ = node.edges_;
    }
    
    uint64_t get_term()const
    {
      //return graph_->id_mgr_.get64(edge_.EDGE());
      return edge_.EDGE();
    }

    uint32_t get_freq()const
    {
      return graph_->freqs_.at(edge_.NID());
    }

    typename sorted_edges_t::size_t children_num()const
    {
      return edges_.length();
    }
    
    NodeIterator children_begin()
    {
      if (graph_->nodes_.at(edge_.NID())== (sorted_edges_t*)-1)
        return children_end();
      
      return NodeIterator(edges_, graph_);
    }
    
    NodeIterator children_end()
    {
      return NodeIterator(edges_, graph_, edges_.length());
    }

    bool has_child()
    {
      return  (graph_->nodes_.at(edge_.NID())!= (sorted_edges_t*)-1);
    }

    bool is_null()
    {
      return edge_.EDGE()==(uint32_t)-1;
    }
    
  };

  Node get_root()
  {
    return Node(this, edge_t(-1, 0));
  }
  
  class NodeIterator
  {
    sorted_edges_t edges_;
    const self_t* graph_;
    typename edges_t::size_t p_;

  public:
    NodeIterator(const sorted_edges_t edges, const self_t* graph, typename edges_t::size_t p = 0)
      :edges_(edges),graph_(graph),p_(p)
    {
    }

    
    NodeIterator(const NodeIterator& other)
      : edges_(other.edges_), graph_(other.graph_),p_(other.p_)
    {
    }

    NodeIterator& operator = (const NodeIterator& other)
    {
      edges_ = other.edges_;
      graph_ = other.graph_;
      p_ = other.p_;

      return *this;
    }
    
    Node operator *()
    {
      return Node(graph_, edges_.at(p_));
    }

    NodeIterator& operator++()
    {
      ++p_;
      return(*this);
    }
    
    NodeIterator operator++(int)
    {
      NodeIterator tmp(*this);

      ++p_;
      
      return(tmp);
    }

    bool operator != (const NodeIterator& other)
    {
      return p_ != other.p_;
    }
    
  }
    ;
  
}
  ;


NS_IZENELIB_AM_END
#endif
