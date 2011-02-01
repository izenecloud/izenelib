
#include <am/graph_index/graph.hpp>
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
  
typedef Graph<800, 1000> trie_t;
typedef uint32_t count_t;
typedef uint32_t termid_t;
typedef trie_t::Node Node;
typedef trie_t::NodeIterator NodeIterator;

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
    for (NodeIterator i = child_node.children_begin(); i!=child_node.children_end(); ++i)
    {
      id[0] = child_node.get_term();
      search(child_node, *i, id);
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
  currentValue.push_back(child_node.get_term());

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

  NodeIterator it2 = child_node.children_begin();
  while(it2 != child_node.children_end())
  {
    rightTermCount.push_back((*it2).get_freq());
    ++it2;
  }
  
  double scoreRCD = 0.;//computeLD(rightTermCount);
  
  //final score for childNode
  double SCORE = 0.;//combine(score1,scoreLCD,scoreRCD);
  if(SCORE >= 2)//THRESHOLD)
  {
    ;//ALL_IMPORTANT_PHRASE.add(currentValue, childNode.get_doc_list());
  }

  it2 = child_node.children_begin();
  while(it2 != child_node.children_end())
  {
    search(child_node, *it2, currentValue);
    ++it2;
  }
  
}

void input()
{
  std::system("rm -f ./tt*");
  
  struct timeval tvafter,tvpre;
  struct timezone tz;
  
  const uint32_t SIZE = 1000000;
  const uint32_t snip_len = 4;
  vector<uint32_t> vs;

  gettimeofday (&tvpre , &tz);
  sfxtidxer->ready4add();
  for (uint32_t p = 0; p<10; ++p)
  {
    vs.resize(SIZE);
    for (uint64_t i=0; i<SIZE; ++i)
      vs[i] = (rand()%80000);

    uint32_t docid = 0;
    for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
    {
      vector<uint32_t> terms;
      for (size_t j=i; j<i+snip_len; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(10*snip_len)==0)
        ++docid;

      sfxtidxer->add_terms(terms, docid);
    }
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nInsert into suffix trie ("<<sfxtidxer->doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  gettimeofday (&tvpre , &tz);
  sfxtidxer->indexing();
  gettimeofday (&tvafter , &tz);
  cout<<"\nIndexing sufiix trie : "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);
  pfxtidxer->ready4add();
  for (uint32_t p = 0; p<10; ++p)
  {
    vs.resize(SIZE);
    for (uint64_t i=0; i<SIZE; ++i)
      vs[i] = (rand()%80000);

    uint32_t docid = 0;
    for (size_t i=0; i<vs.size()-snip_len; i+=snip_len)
    {
      vector<uint32_t> terms;
      for (size_t j=i; j<i+snip_len; ++j)
      {
        //cout<<vs[j]<<" ";
        terms.push_back(vs[j]);
      }
      //cout<<endl;

      if (i%(10*snip_len)==0)
        ++docid;

      pfxtidxer->add_terms(reverse(terms), docid);
    }
  }
  gettimeofday (&tvafter , &tz);
  cout<<"\nInsert into prefix trie ("<<sfxtidxer->doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

  gettimeofday (&tvpre , &tz);  
  pfxtidxer->indexing();
  gettimeofday (&tvafter , &tz);
  cout<<"\nindexing prefix trie : "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;

//   sfxtidxer->insert(termlist, id);
//   sfxtidxer->merge();

  
//   pfxtidxer->insert(reverse(termlist), 0);
//   pfxtidxer->merge();
}

int main()
{
  std::system("rm -f ./tt*");
  
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
  
  pfxtidxer->ratio_load(0.6);
  cout<<"prefix loaded\n";
  
  sfxtidxer->ratio_load(.08);
  gettimeofday (&tvafter , &tz);
  cout<<"Ratio load is done: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

  gettimeofday (&tvpre , &tz);

  std::vector<termid_t> parentValue(0);
  Node rootNode = sfxtidxer->get_root();
  
  NodeIterator it = rootNode.children_begin();
  uint64_t i= 0;
  while(it != rootNode.children_end())
  {
    if (i%200==0)
      cout<<"Branch:"<<(double)i/rootNode.children_num()<<endl;
    
    if (!(*it).has_child())
    {
      ++it;
      ++i;
      continue;
    }
    
    search(rootNode, (*it), parentValue);
    //add doc_list
    ++it;
    ++i;
  }

  gettimeofday (&tvafter , &tz);
  cout<<"TG is done: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

  delete sfxtidxer;
  delete pfxtidxer;
}
