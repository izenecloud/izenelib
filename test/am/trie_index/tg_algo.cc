#include <am/trie_index/term_hash_table.hpp>
#include <am/trie_index/hash_trie.hpp>
#include <am/trie_index/doc_list.hpp>
#include <am/trie_index/trie_indexer.hpp>
#include <util/hashFunction.h>
#include <string>
#include <time.h>
#include <math.h>

#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

using namespace izenelib::am;
using namespace izenelib::util;
using namespace std;

#define THRESHOLD 20;                           \
  
typedef TrieIndexer<46000, 26> trie_t;
typedef uint32_t count_t;
typedef uint64_t termid_t;
typedef Term Node;
typedef trie_t::node_t::const_iterator NodeIterator;
typedef trie_t::node_t NT;

trie_t* sfxtidxer;
trie_t* pfxtidxer;

std::vector<termid_t> reverse(const std::vector<termid_t>& v)
{
  std::vector<termid_t> r(v.size());
  for (std::size_t i=0; i<v.size(); ++i)
    r[i] = v[v.size()-1-i];
  return r;
}

void search(Node parent_node, Node child_node, const std::vector<termid_t>& parentValue)
{
  if (!child_node.has_child())
      return;
  
  //for root
  if (parent_node.is_null())
  {  
    std::vector<termid_t> id(1);
    for (NodeIterator i = child_node.begin(NT()); i!=child_node.end(NT()); ++i)
    {
      id[0] = child_node.get_id();
      search(child_node, Node(*i), id);
    }
    
    return;
  }
  
  count_t f = 0;/*abcd*/
  count_t f1 = 0;/*abc*/
  count_t f2 = 0;/*bcd*/
  count_t parentFreq = parent_node.get_freq();

  f1 = parentFreq;
  f = child_node.get_freq();

  std::vector<termid_t> currentValue(parentValue);
  currentValue.push_back(child_node.get_id());

  std::vector<termid_t> f2Value(currentValue.begin()+1,currentValue.end());
  
  f2 = sfxtidxer->get_freq(f2Value);

  double score1 = 0.;//compute(f,f1,f2);
  
  //compute LCD
  std::vector<termid_t> leftTermList/*?A*/;
  std::vector<count_t> leftTermCount/*Freq*/;
  pfxtidxer->get_suffix(reverse(currentValue), leftTermList, leftTermCount);
  double scoreLCD = 0.;//computeLD(leftTermCount);
  
  //compute RCD
  std::vector<count_t> rightTermCount/*D?*/;

  NodeIterator it2 = child_node.begin(NT());
  while(it2 != child_node.end(NT()))
  {
    rightTermCount.push_back(Node(*it2).get_freq());
    ++it2;
  }
  
  double scoreRCD = 0.;//computeLD(rightTermCount);
  
  //final score for childNode
  double SCORE = 0.;//combine(score1,scoreLCD,scoreRCD);
  if(SCORE >= 2)//THRESHOLD)
  {
    ;//ALL_IMPORTANT_PHRASE.add(currentValue, childNode.get_doc_list());
  }

  it2 = child_node.begin(NT());
  while(it2 != child_node.end(NT()))
  {
    search(child_node, Node(*it2), currentValue);
    ++it2;
  }
  
}

void input()
{
  struct timeval tvafter,tvpre;
  struct timezone tz;
  
  const uint32_t SIZE = 1000000;
  const uint32_t snip_len = 10;
  vector<uint64_t> vs;

  gettimeofday (&tvpre , &tz);
  for (uint32_t p = 0; p<10; ++p)
  {
    vs.resize(SIZE);
    for (uint64_t i=0; i<SIZE; ++i)
      vs[i] = (rand()%80000);

    uint32_t docid = 0;
    for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
    {
      vector<uint64_t> terms;
      for (size_t j=i; j<i+snip_len; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(10*snip_len)==0)
        ++docid;

      sfxtidxer->insert(terms, docid);
    }
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nInsert into suffix trie ("<<sfxtidxer->doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  gettimeofday (&tvpre , &tz);
  sfxtidxer->merge();
  gettimeofday (&tvafter , &tz);
  cout<<"\nMerge sufiix trie : "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  for (uint32_t p = 0; p<10; ++p)
  {
    vs.resize(SIZE);
    for (uint64_t i=0; i<SIZE; ++i)
      vs[i] = (rand()%80000);

    uint32_t docid = 0;
    for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
    {
      vector<uint64_t> terms;
      for (size_t j=i; j<i+snip_len; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(10*snip_len)==0)
        ++docid;

      pfxtidxer->insert(terms, docid);
    }
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nInsert into prefix trie ("<<sfxtidxer->doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);  
  pfxtidxer->merge();
  gettimeofday (&tvafter , &tz);
  cout<<"\nMerge prefix trie : "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

//   sfxtidxer->insert(termlist, id);
//   sfxtidxer->merge();

  
//   pfxtidxer->insert(reverse(termlist), 0);
//   pfxtidxer->merge();
}

int main()
{
  system("./rm.sh");
  
  struct timeval tvafter,tvpre;
  struct timezone tz;
  
  sfxtidxer = new trie_t("./tt_sfx");
  pfxtidxer = new trie_t("./tt_pfx");

  cout<<"Start to input ...\n";
  gettimeofday (&tvpre , &tz);
  input();
  gettimeofday (&tvafter , &tz);
  cout<<"Input is done: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" Min\n";

  gettimeofday (&tvpre , &tz);

  trie_t* sfxtidxer4branch = new trie_t("./tt_sfx");
  pfxtidxer->ratio_load();
  sfxtidxer->ratio_load();
  gettimeofday (&tvafter , &tz);
  cout<<"Ratio load is done: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" Min\n";

  gettimeofday (&tvpre , &tz);
  std::vector<termid_t> parentValue(0);
  trie_t::node_t* rootNode = sfxtidxer4branch->load_root();
  
  trie_t::node_t::const_iterator it = rootNode->begin();
  while(it != rootNode->end())
  {
    if (!(*it).has_child())
    {
      ++it;
      continue;
    }
    
    //recursive call
    trie_t::trie_t* tree = NULL;

    uint64_t addr = sfxtidxer4branch->load_sub_tree(Node(*it), &tree);

    Node null(0);
    search(null, Node(*it), parentValue);
    //add doc_list
    sfxtidxer4branch->release_sub_tree(Node(*it), &tree, addr);

    ++it;
  }

  
  pfxtidxer->ratio_release();
  //sfxtidxer->ratio_release();

  gettimeofday (&tvafter , &tz);
  cout<<"TG is done: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" Min\n";
}
