#ifndef ALGO_HPP
#define ALGO_HPP


#include <hlmalloc.h>
#include <types.h>
#include <string>
#include <vector>
#include <ostream>
//#include "vector_string.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <sys/types.h>
#include <math.h>

class is_alphabet
{
public:
  static bool value(char c)
  {
    return c>='a' && c<='z'||c>='A' && c<= 'Z';
  }

  static bool is_upper(char c)
  {
    return c>='A' && c<= 'Z';
  }

  static bool is_lower(char c)
  {
    return c>='a' && c<='z';
  }

  static char to_lower(char c)
  {
    if (is_upper(c))
      return c - ('A'-'a');

    return c;    
  }

  static char to_upper(char c)
  {
    if (is_lower(c))
      return c + ('A'-'a');
    return c;
  }

  static bool equal_ignore_case(char c1, char c2)
  {
    return c1==c2 || c1-c2=='A'-'a' || c1 - c2=='a'-'A';
  }

  static bool is_space(char c)
  {
    return c == ' ';
  }

  static bool is_new_line(char c)
  {
    return (c == '\n');
  }
  

}
  ;


class is_numeric
{
public:
  static bool value(char c)
  {
    return c>='0' && c<='9';
  }

  static bool is_positive(char c)
  {
    return c=='+';
  }

  static bool is_negative(char c)
  {
    return c=='-';
  }
  
  static unsigned char numeric_value(char c)
  {
    if (!value(c))
      return 0;
    
    return c- '0';
  }

  static bool is_dot(char c)
  {
    return c == '.';
  }

  static char dot()
  {
    return '.';
  }
  
}
  ;



NS_IZENELIB_AM_BEGIN

template <
  class StringT = std::string,
  class IS_NUMERIC = is_numeric,
  class IS_ALPHABET = is_alphabet
  >
class Algorithm
{
  typedef typename StringT::value_type CharT;
  typedef typename StringT::size_t size_t;
  
public:
  enum StrCompMode { SM_SENSITIVE, SM_IGNORE };
  
  //*******************KMP**********************
  static void get_nextval(const StringT& s, uint64_t next[], StrCompMode caseChk=SM_SENSITIVE)
  {
    // 求模式串T的next函数值并存入数组 next。
    uint64_t j = 0, k = -1;

    next[0] = -1;

    while ( j < s.length()-1 )
    {
      if (k == (uint64_t)-1 
          || caseChk==SM_SENSITIVE && s[j]== s[k]
         || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(s[j], s[k]) )
      {
        ++j; ++k;

        if (caseChk==SM_SENSITIVE && s[j]!= s[k]
            ||caseChk==SM_IGNORE && !IS_ALPHABET::equal_ignore_case(s[j], s[k]) )
          next[j] = k;
        else
          next[j] = next[k];
      }// if
      else
        k = next[k];
    }// while
  }// get_nextval　

  static uint64_t KMP(const StringT& text,const StringT& pattern, StrCompMode caseChk=SM_SENSITIVE) //const 表示函数内部不会改变这个参数的值。
  {

    if( text.length()==0 || pattern.length()==0 )//
      return -1;//空指针或空串，返回-1。

    uint64_t len= pattern.length();

    uint64_t *next= (uint64_t*)hlmalloc((len+1)*sizeof (uint64_t));

    get_nextval(pattern,next, caseChk);//求Pattern的next函数值

    uint64_t index=0,i=0,j=0;

    while(i < text.length() && j < pattern.length())
    {
      if(caseChk==SM_SENSITIVE && text[i]== pattern[j]
         || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(text[i], pattern[j]) )
      {
        ++i;// 继续比较后继字符
        ++j;

      }
      else
      {
        index += j-next[j];

        if(next[j]!=(uint64_t)-1)

          j=next[j];// 模式串向右移动
        else
        {
          j=0;
          ++i;
        }
      }
    }//while

    hlfree(next);

    if(j == pattern.length())
      return index;// 匹配成功
    else
      return -1;      

  }

  //*******************rKMP**********************  
  static void rget_nextval(const StringT& s, uint64_t next[], StrCompMode caseChk=SM_SENSITIVE)
  {
    // 求模式串T的next函数值并存入数组 next。

    uint64_t j = s.length()-1, k = s.length();

    next[s.length()] = s.length();

    while ( j >0 )
    {
      if (k == s.length()
          || caseChk==SM_SENSITIVE && s[j]== s[k]
          || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(s[j], s[k]) )
      {
        --j; --k;

        if (caseChk==SM_SENSITIVE && s[j]!= s[k]
            ||caseChk==SM_IGNORE && !IS_ALPHABET::equal_ignore_case(s[j], s[k]) )
          next[j+1] = k;
        else
          next[j+1] = next[k+1];

      }// if
      else
        k = next[k+1];

    }// while

  }// get_nextval　

  static uint64_t rKMP(const StringT& text,const StringT& pattern, StrCompMode caseChk=SM_SENSITIVE) //const 表示函数内部不会改变这个参数的值。
  {

    if( text.length()==0 || pattern.length()==0 )//
      return -1;//空指针或空串，返回-1。

    uint64_t len= pattern.length();

    uint64_t *next= (uint64_t*)hlmalloc((len+1)*sizeof (uint64_t));

    rget_nextval(pattern,next, caseChk);//求Pattern的next函数值
    
    uint64_t index=text.length()-1,i=text.length()-1,j=pattern.length()-1;

    while(i !=(uint64_t)-1  && j !=(uint64_t)-1 )
    {
      if(caseChk==SM_SENSITIVE && text[i]== pattern[j]
         || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(text[i], pattern[j]) )
      {
        --i;// 继续比较后继字符
        --j;
      }
      else
      {
        index -= next[j+1] - j;
        
        if(next[j+1]!= pattern.length())
          j=next[j+1];// 模式串向右移动
        else
        {
          j=pattern.length()-1;
          --i;
        }
      }

    }//while

    hlfree(next);

    if(j == (uint64_t)-1)
      return index;// 匹配成功
    else
      return -1;
  }

    //*******************Data type**********************
  static int to_integer(const StringT& str) 
  {
    int r = 0;
    int k=1;
    
    for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend();i++, k *= 10)
    {
      if (!IS_NUMERIC::value(*i))
      {
        assert(IS_NUMERIC::is_positive( *i) || IS_NUMERIC::is_negative( *i));
        if (IS_NUMERIC::is_negative( *i))
          return -1*r;
        return r;
      }
      
      r += IS_NUMERIC::numeric_value(*i)*k;
    }
    
    return r;
  }
  
  static long to_long(const StringT& str) 
  {
    long r=0;
    long k =1;

        
    for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend();i++, k *= 10)
    {
      if (!IS_NUMERIC::value(*i))
      {
        assert(IS_NUMERIC::is_positive( *i) || IS_NUMERIC::is_negative( *i));
        if (IS_NUMERIC::is_negative( *i))
          return -1*r;
        return r;
      }
      
      r += IS_NUMERIC::numeric_value(*i)*k;
    }

    return r;
  }
  
  static unsigned long to_ulong(const StringT& str) 
  {
    unsigned  long r=0;
    unsigned long k =1;

        
    for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend();i++, k *= 10)
    {
      r += IS_NUMERIC::numeric_value(*i)*k;
    }

    return r;
  }
  
  static float to_float(const StringT& str) 
  {
    float r = 0;
    float k = 1;
    
    size_t dot = str.find(IS_NUMERIC::dot());
    if (dot == StringT::npos)
      dot = str.length();

    size_t i=dot-1;
    for (; i!=(size_t)-1 && IS_NUMERIC::value(str[i]); i--, k *=10)
      r += IS_NUMERIC::numeric_value(str[i])*k;

    
    float o = 1;
    if (i!=(size_t)-1 && IS_NUMERIC::is_negative(str[i]))
      o = -1;

    k = 10.;
    for (i = dot+1; i<str.length() && str[i]>='0'&& str[i]<='9'; i++, k *=10)
      r += IS_NUMERIC::numeric_value(str[i])/k;
    
    r *= o;

    return r;
  }
  
  static void to_lower(StringT& str)
  {
    for (typename StringT::iterator i=str.begin(); i!=str.end();i++)
    {
      *i = IS_ALPHABET::to_lower(*i);
    }
  }

  static void to_upper(StringT& str)
  {
    for (typename StringT::iterator i=str.begin(); i!=str.end();i++)
    {
      *i = IS_ALPHABET::to_upper(*i);
    }
  }

  static StringT to_lower(const StringT& str)
  {
    StringT s = str;
    for (typename StringT::iterator i=s.begin(); i!=s.end();i++)
    {
      *i = IS_ALPHABET::to_lower(*i);
    }
    return s;
  }

  static StringT to_upper(const StringT& str)
  {
    StringT s = str;
    for (typename StringT::iterator i=s.begin(); i!=s.end();i++)
    {
      *i = IS_ALPHABET::to_upper(*i);
    }
    return s;
  }

  static bool equal_ignore_case (const StringT& str1, const StringT& str2)
  {
    if (str1.length()!=str2.length())
      return false;

    typename StringT::const_iterator j=str2.begin();

    for (typename StringT::const_iterator i=str1.begin(); i!=str1.end()&&j!=str2.end();i++, j++)
    {
      if (!IS_ALPHABET::equal_ignore_case(*i, *j))
        return false;
    }

    return true;
  }

  static bool equal_ignore_case_space(const StringT& str1, const StringT& str2)
  {
    typename StringT::const_iterator j=str2.begin();
    typename StringT::const_iterator i=str1.begin();

    for (; i!=str1.end()&&j!=str2.end();i++,j++)
    {
      while (i!=str1.end() && IS_ALPHABET::is_space(*i))
        i++;

      while (j!=str2.end() && IS_ALPHABET::is_space(*j))
        j++;

      if (i==str1.end() || j==str2.end())\
        break;
      
      if (!IS_ALPHABET::equal_ignore_case(*i, *j))
        return false;
    }

    while (i!=str1.end() && IS_ALPHABET::is_space(*i))
      i++;

    if (i!= str1.end())
      return false;
    
    while (j!=str2.end() && IS_ALPHABET::is_space(*j))
        j++;
    if (j!= str2.end())
      return false;

    return true;
  }

  static size_t find(const StringT& str1, const StringT& str2, size_t pos1 = 0, StrCompMode caseChk=SM_IGNORE)
  {
    if (str2.length()>str1.length()-pos1)
        return -1;

    if (SM_SENSITIVE == caseChk)
      return str1.find(str2, pos1);
    else
    {
      size_t i = KMP(str1.substr(pos1), str2, caseChk);
      if (i == (size_t)-1)
        return -1;
      return pos1 + i;
    }
  }

  static size_t rfind(const StringT& str1, const StringT& str2, size_t pos1 = StringT::npos, StrCompMode caseChk=SM_IGNORE)
  {
    if (pos1 == StringT::npos &&  str2.length()>str1.length())
        return -1;
    if (pos1 != StringT::npos && str2.length()>pos1+1)
        return -1;

    if (SM_SENSITIVE == caseChk)
      return str1.rfind(str2, pos1);
    else
    {
      return rKMP(str1.substr(0, (pos1 == StringT::npos? pos1: pos1+1)), str2, caseChk);
    }
  }

  static size_t multi_find(const StringT& str1, const StringT& str2, std::vector<size_t>& indices,
                 size_t pos1 = 0, StrCompMode caseChk=SM_IGNORE) 
  {
    indices.clear();

    size_t i = pos1;

    while (i!= (size_t)-1)
    {
      i = find(str1, str2, i, caseChk);
      if (i!= (size_t)-1)
      {
        indices.push_back(i);
        i += str2.length();
      }
      
    }

    return indices.size();
  }
  
  static bool start_with(const StringT& str1, const StringT& str2, size_t pos1 = 0, StrCompMode caseChk=SM_IGNORE)
  {
    if (str2.length()>str1.length()-pos1)
        return false;
    
    size_t i = pos1;
    size_t j = 0;
      
    if (SM_SENSITIVE == caseChk)
    {
      for (; j<str2.length();i++, j++)
      {
        if (str1[i] != str2[j])
          return false;
      }
    }
    else
    {
      for (; j<str2.length();i++, j++)
      {
        if (!IS_ALPHABET::equal_ignore_case(str1[i], str2[j]))
          return false;
      }
    }

    return true;
  }

  
  static bool end_with(const StringT& str1, const StringT& str2, size_t pos1 = StringT::npos, StrCompMode caseChk=SM_IGNORE)
  {
    if (pos1 == StringT::npos)
      pos1 = str1.length()-1;
    
    if (str2.length()>pos1+1)
        return false;
    
    size_t i = pos1;
    size_t j = str2.length()-1;
      
    if (SM_SENSITIVE == caseChk)
    {
      for (; j!=(size_t)-1; i--, j--)
      {
        if (str1[i] != str2[j])
          return false;
      }
    }
    else
    {
      for (; j !=(size_t) -1;i--, j--)
      {
        if (!IS_ALPHABET::equal_ignore_case(str1[i], str2[j]))
          return false;
      }
    }

    return true;
  }

  static size_t num_occurrence(const StringT& str1, const StringT& str2,
                                size_t pos1 = 0, StrCompMode caseChk=SM_IGNORE) 
  {
    size_t r =0;
    size_t i = pos1;
    
    while (i!= (size_t)-1)
    {
      i = find(str1, str2, i, caseChk);
      if (i!= (size_t)-1)
      {
        i += str2.length();
        r ++;
      }
      
    }

    return r;
  }

  static void frequency_counter(const StringT& str, const std::vector<StringT>& patterns, std::vector<size_t>& freq,
                                StrCompMode caseChk=SM_IGNORE) 
  {
    freq.clear();
    freq.reserve(patterns.size());
    
    for (typename std::vector<StringT>::const_iterator i=patterns.begin(); i!= patterns.end();i++)
    {
      freq.push_back(num_occurrence(str, *i, 0, caseChk));
    }
  }

  static size_t  substitute_char(CharT from, CharT to, StringT& str, size_t pos = 0, StrCompMode caseChk=SM_IGNORE)
  {
    size_t r = 0;
    for (size_t i=pos; i<str.length(); i++)
      if (caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i], from)
          ||caseChk==SM_SENSITIVE && str[i]==from)
      {
        r++;
        str[i] = to;
      }
    return r;
  }

  
  static StringT  substitute_char(CharT from, CharT to, const StringT& str, size_t pos = 0, StrCompMode caseChk=SM_IGNORE)
  {
    StringT s = str;
    size_t r = 0;
    for (size_t i=pos; i<str.length(); i++)
      if (caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i], from)
          ||caseChk==SM_SENSITIVE && str[i]==from)
      {
        r++;
        s[i] = to;
      }
    return r;
  }

  static size_t  multi_substitute_string(const StringT& from, const StringT& to,
                                   StringT& str, size_t pos = 0,
                                   StrCompMode caseChk=SM_IGNORE)
  {
    std::vector<size_t> v;
    multi_find(str, from, v, pos, caseChk);
    
    StringT s;

    size_t last = 0;
    for (typename std::vector<size_t>::iterator i=v.begin(); i!=v.end(); i++)
    {
      assert(last<str.length());
      StringT ssss = str.substr(last, *i-last);
      s += ssss;
      s += to;
      last = *i+from.length();
    }
    
    s += str.substr(last);
    str = s;
    
    return v.size();
  }

  static StringT  multi_substitute_string(const StringT& from, const StringT& to,
                                   const StringT& str, size_t pos = 0,
                                   StrCompMode caseChk=SM_IGNORE)
  {
    std::vector<size_t> v;
    multi_find(str, from, v, pos, caseChk);
    StringT s;

    size_t last = 0;
    for (typename std::vector<size_t>::iterator i=v.begin(); i!=v.end(); i++)
    {
      assert(last<str.length());
      s += str.substr(last, *i-last);
      s += to;
      last = *i+from.length();
    }

    s += str.substr(last);
    return s;
  }

  static StringT  substitute_string(const StringT& from, const StringT& to,
                                   const StringT& str, size_t pos = 0,
                                   StrCompMode caseChk=SM_IGNORE)
  {
    StringT s = str;
    substitute_string(from, to, s, pos, caseChk);
    return s;
    
  }

  static void substitute_string(const StringT& from, const StringT& to,
                                   StringT& str, size_t pos = 0,
                                   StrCompMode caseChk=SM_IGNORE)
  {
    size_t i = find(str, from, pos, caseChk);
    StringT s;
    if (i == (size_t)-1)
      return;

    s += str.substr(0, i);
    s += to;
    i += from.length();
    if (i<str.length())
      s += str.substr(i);

    str = s;
  }

  static size_t remove_newlines(StringT& str)
  {
    size_t r = 0;
    for (size_t i=0; i<str.length(); i++)
      if (IS_ALPHABET::is_new_line(str[i]))
      {
        r++;
        str[i] = ' ';
      }
    
    return r;
  }

  
  static StringT remove_newlines(const StringT& str)
  {
    StringT r = str;
    remove_newlines(r);
    return r;
  }

  static void remove_duplicate_chars(StringT& str, CharT c)
  {
    StringT s;
    s.reserve(str.length());
    
    size_t i;
    int count = 0;
    
    for (i = 0; i < str.length(); i++)
    {
      if (str[i] == c)
      {
        if (count > 0);
        else s += c;
        count++;
      }
      else {
        s += str[i];
        count = 0;
      }
    }

    //std::cout<<s.getReferCnt()<<std::endl;
    str = s;
    //std::cout<<s.getReferCnt()<<std::endl;
  }

  static StringT remove_duplicate_chars(const StringT& str, CharT c)
  {
    StringT s = str;
    remove_duplicate_chars(s, c);
    return s;
    
  }

  static void remove_multi_whitespace(StringT& str)
  {
    remove_duplicate_chars(str, ' ');
  }

  static StringT remove_multi_whitespace(const StringT& str)
  {
    return remove_duplicate_chars(str, ' ');
  }

  static void make_tokens_with_delimiter(const StringT& str, const StringT& delimiter, std::vector<StringT>& tokens)
  {
    tokens.clear();
    
    if (str.length()<=delimiter.length())
      return;

    std::vector<size_t> indices;
    multi_find(str, delimiter, indices, 0, SM_SENSITIVE);
    tokens.reserve(indices.size()+1);
    indices.push_back(-1);
    
    size_t i = indices[0];
    if (i!=0)
      tokens.push_back(str.substr(0, i));

    for (; i<indices.size()-2;i++)
    {
      if (indices[i]+delimiter.length()>=indices[i+1])
        continue;
      tokens.push_back(str.substr(indices[i]+delimiter.length(),
                                  indices[i+1]==(size_t)-1? indices[i+1]: indices[i+1]-(indices[i]+delimiter.length())));
    }    
  }

  static StringT cut_range(const StringT& str, size_t start, size_t end) 
  {
    return str.substr(start, end - start + 1);
  }

  static void cut_range(StringT& str, size_t start, size_t end) 
  {
    str = str.substr(start, end - start + 1);
  }

  static StringT cut_delimiter(const StringT& str, size_t start, CharT delimiter) 
  {
    StringT s =str;
    cut_delimiter(s, start, delimiter);
    return s;
  }

  static void cut_delimiter(StringT& str, size_t start, CharT delimiter) 
  {
    assert(start < str.length());
    
    size_t i=start;
    
    while(str[i]!= delimiter)
    {
      i++;
      if (start + i >= str.length())
      {
        str = "";
        return;
      }
    }

    if (i==start){
      str = "";
      return ;
    }
    
    
    return cut_range(str, start, i-1);
  }

  static StringT cut_between_words(const StringT& str, const StringT& p1,
                                   const StringT& p2, StrCompMode caseChk=SM_SENSITIVE) 
  {
    StringT s = str;
    cut_between_words(s, p1, p2, caseChk);
    return s;
  }

  static void cut_between_words(StringT& str, const StringT& p1,
                                const StringT& p2, StrCompMode caseChk=SM_SENSITIVE ) 
  {
    size_t i = find(str, p1, 0, caseChk);
    if (i==(size_t)-1)
    {
      str = "";
      return;
    }
    
    size_t j = rfind(str, p2, StringT::npos, caseChk);
    if (j==(size_t)-1)
    {
      str = "";
      return ;
    }
    
    if (i+p1.length()>=j)
    {
      str = "";
      return ;
    }

    str = str.substr(i+p1.length(), j-i-p1.length()-1);
  }

  static StringT cut_word_after(const StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE) 
  {
    StringT s = str;
    cut_word_after(s, tag, caseChk);
    return s;
  }

  static void cut_word_after(StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE) 
  {
    size_t index = find(str, tag,0, caseChk);
    if (index!=(size_t)-1)
    {
      size_t i = index + tag.length();
      for (; i < str.length() && IS_ALPHABET::is_space(str[i]); i++);
      
      if (i == str.length())
      {
        str = "";
        return ;
      }
      
      
      size_t start = i;
      for (; i < str.length() && !IS_ALPHABET::is_space(str[i]); i++);
      
      cut_range(str, start, i - 1);
      return;
    }
    
    str = "";
  }

  static StringT cut_line_after(const StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE) 
  {
    StringT s = str;
    cut_line_after(s, tag, caseChk);
    return s;
  }

  static void cut_line_after(StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE) 
  {
    size_t index = find(str, tag, 0, caseChk);
    if (index!=(size_t)-1)
    {
      size_t i = index + tag.length();
      if (i == str.length())
      {
        str = "";
        return;
      }

      size_t start = i;
      for (; i < str.length() && !IS_ALPHABET::is_new_line(str[i]); i++);
      
      cut_range(str, start, i - 1);
      return;
    }
    
    str = "";
  }

    /**
   *
     @brief : Returns n DynamicArray of StringTs of which elements contain
                 the words between the marks.
	       It doesn't contain the marks itself.
   *
   */
  static void take_between_marks(const StringT& str, CharT beg, CharT end,
                          std::vector<StringT>& arrayOfstrings) 
  {
    arrayOfstrings.clear();
    
    if (beg == end)
      return;

    size_t i;
    size_t index = -1;
    size_t level = 0;
    size_t step = 0;
    
    for (i = 0; i < str.length(); i++) {
      if (str[i] == beg) {
        index++;
        level++;
      }
      else if (str[i] == end) {
        if (level > 0) {
          level--;
          if (level == 0)
            step = 0;
          else
            step++;
        }
      }
      else if (level > 0) {
        CharT temp = str[i];
        size_t finalIndex = index - step;
        while (finalIndex >=arrayOfstrings.size())
          arrayOfstrings.push_back("");
        arrayOfstrings[finalIndex] += temp;
      }
      else;
    }
  }

  static void take_between_mark(const StringT& str, CharT mark,
				  std::vector<StringT>& arrayOfstrings) 
  {
    arrayOfstrings.clear();
    
    size_t index = 0;
    size_t prev = 0;

    index = str.find(mark);
    prev = index;

    index = str.find(mark, index+1);
    while (index != (size_t)-1) {
      arrayOfstrings.push_back(cut_range(str, prev+1, index - 1));
      prev = index;
      index = str.find(mark, index+1);
    }
  }

    /**
      @brief Returns the rear part that begins with the tag, including
                      the tag depending on the option.
   */
  static StringT reverse_cut_rear_with(const StringT& str, CharT tag,
                                       bool includeTag = false, StrCompMode caseChk = SM_IGNORE) 
  {
    StringT s = str;
    reverse_cut_rear_with(s, tag, includeTag, caseChk);
    return s;
  }

  static void reverse_cut_rear_with(StringT& str, CharT tag,
                                       bool includeTag = false, StrCompMode caseChk = SM_IGNORE) 
  {
    size_t i;
    for (i = str.length()-1; i!=(size_t)-1; i--) {
      if (caseChk == SM_SENSITIVE && str[i]==tag
          || caseChk == SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i],tag))
        break;
    }

    if (i!=(size_t)-1) {
      if (includeTag)
        cut_range(str, i, str.length()-1);
      else if (i < str.length()-1)
        cut_range(str, i+1, str.length()-1);

      return;
      
    }

    str = "";
  }

  static StringT reverse_cut_front_with(const StringT& str, CharT tag,
                                       bool includeTag = false, StrCompMode caseChk = SM_IGNORE) 
  {
    StringT s = str;
    reverse_cut_front_with(s, tag, includeTag, caseChk);
    return s;
  }

  static void reverse_cut_front_with(StringT& str, CharT tag,
                                        bool includeTag = false, StrCompMode caseChk = SM_IGNORE) 
  {
    size_t i;
    for (i = str.length()-1; i!=(size_t)-1; i--)
      if (caseChk == SM_SENSITIVE && str[i]==tag
          || caseChk == SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i],tag))
        break;

    if (i!=(size_t)-1) {
      if (includeTag)
        cut_range(str, 0, i);
      else if (i!=0)
        cut_range(str, 0, i-1);
      return;
    }

    str = "";
  }

  static StringT compact_head(const StringT& str)
  {
    StringT s = str;
    
    compact_head(s);
    return s;

  }

  static void compact_head(StringT& str)
  {
    size_t i=0;
    for (; i<str.length(); i++)
      if (!IS_ALPHABET::is_space(str[i]))
        break;

    str = str.substr(i);
  }

  static StringT compact_tail(const StringT& str)
  {
    StringT s = str;
    compact_tail(s);
    return s;
  }

  static void compact_tail(StringT& str)
  {
    size_t i=str.length()-1;
    for (; i!=(size_t)-1; i--)
      if (!IS_ALPHABET::is_space(str[i]))
        break;

    str = str.substr(0, i+1);
  }

  static StringT compact(const StringT& str)
  {
    StringT s;
    s = compact_head(str);
    compact_tail(s);
    return s;
  }
  
  static void compact(StringT& str)
  {
    compact_head(str);
    compact_tail(str);
  }

  static StringT trim(const StringT& str)
  {
    StringT s;
    s.reserve(str.length()*3/4);
    
    size_t i=0;
    size_t last = i;
    for (; i<str.length(); i++)
    {
      if (!IS_ALPHABET::is_space(str[i]))
        continue;
      
      if (last<i )
      {
        s += str.substr(last, i-last);
      }
      
      last = i+1;
    }
    
    if (last < str.length())
      s += str.substr(last);

    return s;
  }

  static void trim(StringT& str)
  {
    const StringT s =str;
    str = trim(s);
  }

  static void read_from_file(const StringT& filename, std::vector<StringT>& lines, CharT line='\n')
  {
    std::ifstream file (filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    boost::archive::binary_iarchive ia(file);
    StringT s;
    ia >> s;

    make_tokens_with_delimiter(s, line, lines);
  }

  static void write_to_file(const StringT& filename, const std::vector<StringT>& lines)
  {
    std::ofstream file(filename.as_const_char(), std::ios::out|std::ios::binary);
    boost::archive::binary_oarchive oa(file);

    for (size_t i=0; i<lines.size(); i++)
      oa<<lines[i]<<"\n";

    file.close();
  }

    /**
   * @brief Read all the names of the files in the specified directory.
   *
       Read all the names of the files in the specified directory.
                    If there are any errors such as no such directory, it
              just returns leaving the reference argument unchanged.
            It discards '.' and '..' files.

   *
   */
  static void read_filenames(const StringT& dir, std::vector<StringT>& files)
  {
//     DIR *dirp;
//     struct dirent *direntp;
//     dirp = opendir(dir.c_str());
//     if (dirp == NULL) {
//       std::cout << "Warning: Algorithm::read_filenames : invalid directory: "
//            << dir << std::endl;
//       return;
//     }
//     while ( (direntp = readdir( dirp )) != NULL )
//       if (!((strcmp(direntp->d_name, ".") == 0
// 	     || strcmp(direntp->d_name, "..") == 0)))
// 	files.push_back(direntp->d_name);
//     (void)closedir( dirp );
    
  }

  static StringT generate_random_string(size_t size)
  {
    StringT s;
    s.reserve(size);
    CharT span = 'z'-'a';

    for (size_t i=0; i<size; i++)
      s += (CharT)(random()%span);

    return s;
  }
  
  

}
  ;


NS_IZENELIB_AM_END
#endif
