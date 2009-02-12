#ifndef ALPHABET_HPP
#define ALPHABET_HPP

#include<string>
#include <ostream>
using namespace std;

/**
 *@class AlphabetGenerator
 *@brief It is to generate alphabet in an unsigned short array.
 *
 * Take english charactors for example. The input string is 'abcdefghijklmnopqrstuvwxyz'.
 *Then, it will print an sorted unsigned short array for the input in console.
 */
class AlphabetGenerator
{
public:
  
  AlphabetGenerator(const string& alp)
  {
    // UString s(alp, UString::UTF_8);
//     string str;
//     s.convertString(str, ENCODE_TYPE);
    
    pStr_ = new string(alp);
    sort();
  }

  /**
   *It prints the sorted alphabet in console
   **/
  friend ostream& operator << ( ostream& os, const AlphabetGenerator& alp)
  {
    //alp.pUstr_->displayStringValue(ENCODE_TYPE, os);
    
    os<<"[";
    for (size_t i=0; i<alp.pStr_->length()-1; i++)
      os<<"'"<<(*alp.pStr_)[i]<<"',";

    os<<"'"<<(*alp.pStr_)[alp.pStr_->length()-1]<<"']\n";

    os<<"TOTAL:"<<alp.pStr_->length()<<endl;
    
    return os;
  }
  
protected:
  string* pStr_;

  /**
   *It sorts the alphabet in increasing order.
   **/
  void sort()
  {
    for (size_t i=0; i<pStr_->length()-1; i++)
    {
      char min = (*pStr_)[i];
      size_t idx = i;
      
      for (size_t j=i+1; j<pStr_->length(); j++)
      {
        if (min>(*pStr_)[j])
        {
          idx = j;
          min = (*pStr_)[j];
        }
      }

      if (idx == i)
        continue;

      (*pStr_)[idx] = (*pStr_)[i];
      (*pStr_)[i] = min;
    }
    
  }
}
;

// static const unsigned int a2z_size = 26;
// static unsigned short a2z[a2z_size] = 
//   {97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122
//   };

static const unsigned char a2z_size = 63;//!< The alphabet size.
static char a2z[a2z_size] = 
  {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','_','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
  };//!< The alphabet.

//   {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
//   };
#endif

