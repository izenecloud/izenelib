#ifndef ALPHABET_H
#define ALPHABET_H

#include <wiselib/ustring/UString.h>
#include<string>
#include <ostream>

using namespace wiselib;
using namespace std;

/**
 *@class AlphabetGenerator
 *@brief It is to generate alphabet in an unsigned short array.
 *
 * Take english charactors for example. The input string is 'abcdefghijklmnopqrstuvwxyz'.
 *Then, it will print an sorted unsigned short array for the input in console.
 */
template
<
  UString::EncodingType ENCODE_TYPE
>
class AlphabetGenerator
{
public:

  AlphabetGenerator(const string& alp)
  {
    // UString s(alp, UString::UTF_8);
//     string str;
//     s.convertString(str, ENCODE_TYPE);

    pUstr_ = new UString(alp, ENCODE_TYPE);
    sort();
  }

  /**
   *It prints the sorted alphabet in console
   **/
  friend ostream& operator << ( ostream& os, const AlphabetGenerator& alp)
  {
    //alp.pUstr_->displayStringValue(ENCODE_TYPE, os);

    os<<"[";
    for (size_t i=0; i<alp.pUstr_->length()-1; i++)
      os<<(*alp.pUstr_)[i]<<",";

    os<<(*alp.pUstr_)[alp.pUstr_->length()-1]<<"]\n";

    os<<"TOTAL:"<<alp.pUstr_->length()<<endl;

    return os;
  }

protected:
  UString* pUstr_;

  /**
   *It sorts the alphabet in increasing order.
   **/
  void sort()
  {
    for (size_t i=0; i<pUstr_->length()-1; i++)
    {
      UCS2Char min = (*pUstr_)[i];
      size_t idx = i;

      for (size_t j=i+1; j<pUstr_->length(); j++)
      {
        if (min>(*pUstr_)[j])
        {
          idx = j;
          min = (*pUstr_)[j];
        }
      }

      if (idx == i)
        continue;

      (*pUstr_)[idx] = (*pUstr_)[i];
      (*pUstr_)[i] = min;
    }

  }
}
;

static const unsigned int a2z_size = 26;
//
//static unsigned short a2z[a2z_size] =
//{97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122
//};

static char a2z[a2z_size] =
  {97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122
  };

// static const unsigned int a2z_size = 63;//!< The alphabet size.
// static unsigned short a2z[a2z_size] =
//   {48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70,71,72,73,74,75,
// 		  76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,95,97,98,99,100,101,
// 		  102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122
//   };//!< The alphabet.

//   {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
//   };



#endif
