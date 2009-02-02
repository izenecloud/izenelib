#ifndef AUTOMATION_HPP
#define AUTOMATION_HPP

#include <string>
#include <vector>

using namespace std;


template<
  char MULTI_WORDS_WILDCARD = '*',
  char SINGLE_WORD_WILDCARD = '?'
  >
class Automation
{
  class _state_ ;
  

  class _edge_
  {
  public:
    _edge_(char c, _state_* p)
      :p_(p),ch_(c)
    {
    }

    _state_* leadTo(char c)
    {
      if (c == ch_)
        return p_;

      if (ch_ == MULTI_WORDS_WILDCARD)
        return p_;

      if (ch_ == SINGLE_WORD_WILDCARD)
        return p_;

      return NULL;
    }

  friend ostream& operator << ( ostream& os, const _edge_& eg)
    {
      os<<"<"<<eg.ch_<<">==>"<<eg.p_<<endl;
      return os;
    }
    
    
  protected:
    _state_* p_;
    char ch_;
  }
    ;

  class _state_
  {
    vector<_edge_> edges_;

  public:
    _state_* nextState(char ch)
    {
      for (typename vector<_edge_>::iterator i=edges_.begin(); i!=edges_.end(); i++)
      {
        _state_* ret = (*i).leadTo(ch);
        if (ret != NULL)
          return ret;
      }

      return NULL;
    }

    void insertEdge(char ch, _state_* p)
    {
      edges_.push_back(_edge_(ch, p));
    }

    void insertEdge(const _edge_& eg)
    {
      edges_.push_back(eg);
    }
    

  friend ostream& operator << ( ostream& os, const _state_& st)
    {
      os<<"state: "<<&st<<endl;
      
      for (typename vector<_edge_>::const_iterator i=st.edges_.begin(); i!=st.edges_.end(); i++)
      {
        os<<(*i);
      }

      os<<endl;

      return os;
    }
    
  }
    ;
  
  
public:
  Automation(const string& regex)
  {
    _state_* back = NULL;
    _edge_ back_edge(0,0);
    _state_* pThisState = new _state_();
    pV_.push_back(pThisState);
    
    pStart_ = pThisState;
    pEnd_ = NULL;
    
    for (size_t i=0; i<regex.length(); i++)
    {
      _state_* pNextState = new _state_();
      pV_.push_back(pNextState);
              
      if (i == regex.length()-1)
        pEnd_ = pNextState;
      
      if (regex[i]==MULTI_WORDS_WILDCARD)
      {
        if (i == regex.length()-1)
        {
          pThisState->insertEdge(MULTI_WORDS_WILDCARD, pThisState);
          pEnd_ = pThisState;
          back = pThisState;
          break;
        }
        
        pThisState->insertEdge(regex[i+1], pNextState);
        back_edge = _edge_(regex[i+1], pNextState);
          
        pThisState->insertEdge(MULTI_WORDS_WILDCARD, pThisState);
        back = pThisState;
        pThisState = pNextState;
        i++;
          
        continue;
      }

      pThisState->insertEdge(regex[i], pNextState);
      if (back != NULL && regex[i]!=SINGLE_WORD_WILDCARD )
      {
        pThisState->insertEdge(back_edge);
        pThisState->insertEdge(MULTI_WORDS_WILDCARD, back);
      }
      

      pThisState = pNextState;
    }

    if (pEnd_ == NULL)
      pEnd_ = pThisState;
    if (back != NULL)
    {
      pEnd_->insertEdge(back_edge);
      pEnd_->insertEdge(MULTI_WORDS_WILDCARD, back);
    }
  }

  ~Automation()
  {
    for (typename vector<_state_*>::iterator i=pV_.begin(); i!=pV_.end(); i++)
    {
      delete (*i);
    }
  }

friend ostream& operator << ( ostream& os, const Automation<MULTI_WORDS_WILDCARD, SINGLE_WORD_WILDCARD>& aut)
  {
    for (typename vector<_state_*>::const_iterator i=aut.pV_.begin(); i!=aut.pV_.end(); i++)
      os<< *(*i);
    
    return os;
  }
  
  bool match(const string& str)
  {
    _state_* next = pStart_;
    
    for (size_t i=0; i<str.length(); i++)
    {
      //cout<<endl<<str[i]<<endl;
      next = next->nextState(str[i]);
      
      if (next == NULL)
        return false;
      
      //cout<<next;
    }

    if (next == pEnd_)
      return true;

    return false;
  }
  
  

protected:
  _state_* pStart_;
  _state_* pEnd_;
  vector<_state_*> pV_;
  
  
}
  ;





#endif
