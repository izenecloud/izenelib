#ifndef CACHE_STRATEGY_H
#define CACHE_STRATEGY_H

class CachePolicyLRU//latest rare used
{
  clock_t time_;
public:
  int compare(const CachePolicyLRU& t)const
  {
    return time_ - t.time_;
  }

  void visit()
  {
    time_ = clock();
  }
  
  
friend ostream& operator << ( ostream& os, const CachePolicyLRU& inf)
  {
    os<<"time: "<<inf.time_<<endl;
    return os;
  }

}
  ;

class CachePolicyLU//least used
{
  uint64_t visit_count_;
  
public:
  CachePolicyLU()
  {
    visit_count_ = 0;
  }
  
  int compare(const CachePolicyLU& t)const
  {
    return visit_count_ - t.visit_count_;
  }

  void visit()
  {
    visit_count_++;
  }
  
friend ostream& operator << ( ostream& os, const CachePolicyLU& inf)
  {
    os<<"visited: "<<inf.visit_count_<<endl;
    return os;
  }
  
  
}
  ;

class CachePolicyLARU//least and rarest used
{
  uint64_t visit_count_;
  clock_t time_;
  
public:
  CachePolicyLARU()
  {
    visit_count_ = 0;
  }
  
  int compare(const CachePolicyLARU& t)const
  {
    return (visit_count_*time_ - t.visit_count_*t.time_)/CLOCKS_PER_SEC;
  }

  void visit()
  {
    time_ = clock();
    visit_count_++;
  }

friend ostream& operator << ( ostream& os, const CachePolicyLARU& inf)
  {
    os<<"time: "<<inf.time_<<"  visited: "<<inf.visit_count_<<endl;
    return os;
  }
  
  
}
  ;
#endif
