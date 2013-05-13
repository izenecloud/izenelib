/**
   @file algo.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef ALGO_HPP
#define ALGO_HPP


//#ifdef __cplusplus
//extern "C"
//{
//#endif
#include "hlmalloc.h"
//#ifdef __cplusplus
//}
//#endif

#include "types.h"
#include <string>
#include <vector>
#include <ostream>
//#include "vector_string.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <sys/types.h>
#include <math.h>
//#include <iconv.h>

enum StrCompMode { SM_SENSITIVE, SM_IGNORE };

/**
   @class is_alphabet
   @brief it's used to judge if it's english charactor
 */
template<
class CHAR_T = char
>
class is_alphabet
{
public:
    static bool value(CHAR_T c)
    {
        return (c>='a' && c<='z') ||(c>='A' && c<= 'Z');
    }

    static bool is_upper(CHAR_T c)
    {
        return c>='A' && c<= 'Z';
    }

    static bool is_lower(CHAR_T c)
    {
        return c>='a' && c<='z';
    }

    static CHAR_T to_lower(CHAR_T c)
    {
        if (is_upper(c))
            return c - ('A'-'a');

        return c;
    }

    static CHAR_T to_upper(CHAR_T c)
    {
        if (is_lower(c))
            return c + ('A'-'a');
        return c;
    }

    static bool equal_ignore_case(CHAR_T c1, CHAR_T c2)
    {
        // normalize to lowercase first
        if(c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';

        if(c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';

        return c1 == c2;
    }

    static bool is_space(CHAR_T c)
    {
        return c == ' ';
    }

    static bool is_new_line(CHAR_T c)
    {
        return (c == '\n');
    }


}
;


/**
   @class is_numeric
   @brief it's used for numeric operation
 */
template<
class CHAR_T = char
>
class is_numeric
{
public:
    static bool value(CHAR_T c)
    {
        return c>='0' && c<='9';
    }

    static bool is_positive(CHAR_T c)
    {
        return c=='+';
    }

    static bool is_negative(CHAR_T c)
    {
        return c=='-';
    }

    static CHAR_T  numeric_value(CHAR_T c)
    {
        if (!value(c))
            return 0;

        return c- '0';
    }

    static bool is_dot(CHAR_T c)
    {
        return c == '.';
    }

    static CHAR_T dot()
    {
        return '.';
    }

}
;



NS_IZENELIB_UTIL_BEGIN

/**
 *@class Algorithm
 * It is an algorithm set about string. Temlate parameter StringT indicates the string type.
 *IS_NUMERIC indicates a class about deciding if it is a numeric. IS_ALPHABET is a class to
 *judge about charactors in alphabet.
 *@brief An string algorithm set.
 **/
template <
    class StringT = std::string,
    class IS_NUMERIC = is_numeric<typename StringT::value_type>,
    class IS_ALPHABET = is_alphabet<typename StringT::value_type>
    >
class Algorithm
{
    typedef typename StringT::value_type CharT;
    typedef typename StringT::size_t size_t;

public:
    //*******************KMP**********************
    static void get_nextval(const StringT& s, uint64_t* next, StrCompMode caseChk=SM_SENSITIVE)
    {
        uint64_t j = 0, k = -1;

        next[0] = -1;

        while ( j < s.length()-1 )
        {
            if (k == (uint64_t)-1
                    || (caseChk==SM_SENSITIVE && s[j]== s[k])
                    || (caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(s[j], s[k])) )
            {
                if (k != (uint64_t)-1 )
                    IASSERT(k<s.length());

                ++j;
                ++k;

                IASSERT(k<s.length());
                if ((caseChk==SM_SENSITIVE && s[j]!= s[k])
                        || (caseChk==SM_IGNORE && !IS_ALPHABET::equal_ignore_case(s[j], s[k])) )
                    next[j] = k;
                else
                {
                    IASSERT(k<=s.length());
                    next[j] = next[k];
                }
            }// if
            else
            {
                IASSERT(k<=s.length());
                k = next[k];
            }
        }// while
    }// get_nextval　

    /*!
     *KMP, an string maching algorithm. caseChk could be SM_SENSITIVE or SM_IGNORE.
     *Return the index of the first instance of pattern started from 0.
     **/
    static uint64_t KMP(const StringT& text,const StringT& pattern, StrCompMode caseChk=SM_SENSITIVE) //const 表示函数内部不会改变这个参数的值。
    {

        if( text.length()==0 || pattern.length()==0 )//
            return -1;//空指针或空串，返回-1。

        uint64_t len= pattern.length();

        uint64_t *next= (uint64_t*)HLmemory::hlmalloc((len+1)*sizeof (uint64_t));

        get_nextval(pattern,next, caseChk);//求Pattern的next函数值

        uint64_t index=0,i=0,j=0;

        while(i < text.length() && j < pattern.length())
        {
            if((caseChk==SM_SENSITIVE && text[i]== pattern[j])
                    || (caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(text[i], pattern[j])) )
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

        HLmemory::hlfree(next);

        if(j == pattern.length())
        {
            IASSERT(index<text.length());
            return index;// 匹配成功
        }
        else
            return -1;

    }

    //*******************rKMP**********************
    static void rget_nextval(const StringT& s, uint64_t next[], StrCompMode caseChk=SM_SENSITIVE)
    {
        uint64_t j = s.length()-1, k = s.length();

        next[s.length()] = s.length();

        while ( j >0 )
        {
            if (k == s.length()
                    || caseChk==SM_SENSITIVE && s[j]== s[k]
                    || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(s[j], s[k]) )
            {
                --j;
                --k;

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

    /**
     *KMP, an string maching algorithm. caseChk could be SM_SENSITIVE or SM_IGNORE.
     * It searches from back.
     *Return the index of the first instance of pattern started from 0.
     **/
    static uint64_t rKMP(const StringT& text,const StringT& pattern, StrCompMode caseChk=SM_SENSITIVE)
    {

        if( text.length()==0 || pattern.length()==0 )//
            return -1;//

        uint64_t len= pattern.length();

        uint64_t *next= (uint64_t*)HLmemory::hlmalloc((len+1)*sizeof (uint64_t));

        rget_nextval(pattern,next, caseChk);//

        uint64_t index=text.length()-1,i=text.length()-1,j=pattern.length()-1;

        while(i !=(uint64_t)-1  && j !=(uint64_t)-1 )
        {
            if(caseChk==SM_SENSITIVE && text[i]== pattern[j]
                    || caseChk==SM_IGNORE && IS_ALPHABET::equal_ignore_case(text[i], pattern[j]) )
            {
                --i;
                --j;
            }
            else
            {
                index -= next[j+1] - j;

                if(next[j+1]!= pattern.length())
                    j=next[j+1];
                else
                {
                    j=pattern.length()-1;
                    --i;
                }
            }

        }//while

        HLmemory::hlfree(next);

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

        for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend(); i++, k *= 10)
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


        for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend(); i++, k *= 10)
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


        for (typename StringT::const_reverse_iterator i= str.rbegin(); i!=str.rend(); i++, k *= 10)
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
        for (typename StringT::iterator i=str.begin(); i!=str.end(); i++)
        {
            *i = IS_ALPHABET::to_lower(*i);
        }
    }

    /**
     * @brief Change all the lower case to upper caes in the _text.
     *
      Change all the lower case to upper caes in the _text.
      It forces const char* to char*. Do more research on std::string
     *
     */
    static void to_upper(StringT& str)
    {
        for (typename StringT::iterator i=str.begin(); i!=str.end(); i++)
        {
            *i = IS_ALPHABET::to_upper(*i);
        }
    }

    /**
     * @brief Change all the upper case to lower case in the _text.
     *
      Change all the upper case to lower case in the _text.
      It forces const char* to char*. Do more research on std::string
     *
     */
    static StringT to_lower(const StringT& str)
    {
        StringT s = str;
        for (typename StringT::iterator i=s.begin(); i!=s.end(); i++)
        {
            *i = IS_ALPHABET::to_lower(*i);
        }
        return s;
    }

    static StringT to_upper(const StringT& str)
    {
        StringT s = str;
        for (typename StringT::iterator i=s.begin(); i!=s.end(); i++)
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

        for (typename StringT::const_iterator i=str1.begin(); i!=str1.end()&&j!=str2.end(); i++, j++)
        {
            if (!IS_ALPHABET::equal_ignore_case(*i, *j))
                return false;
        }

        return true;
    }

    /**
     *@brief Compare two string after cutting head and tail spaces out.
     **/
    static bool equal_ignore_case_space(const StringT& str1, const StringT& str2)
    {
        typename StringT::const_iterator j=str2.begin();
        typename StringT::const_iterator i=str1.begin();

        for (; i!=str1.end()&&j!=str2.end(); i++,j++)
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

    /**
        @brief : Returns the number of keywords found.
                      returnIndices contains the offsets for the keywords.
              If there is no such keyword, it sets the offset to -1.
     *
     */
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

    /**
     * @brief Returns true if the string starts with the given key in the
                        position pos.
     */
    static bool start_with(const StringT& str1, const StringT& str2, size_t pos1 = 0, StrCompMode caseChk=SM_IGNORE)
    {
        if (str2.length()>str1.length()-pos1)
            return false;

        size_t i = pos1;
        size_t j = 0;

        if (SM_SENSITIVE == caseChk)
        {
            for (; j<str2.length(); i++, j++)
            {
                if (str1[i] != str2[j])
                    return false;
            }
        }
        else
        {
            for (; j<str2.length(); i++, j++)
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
            for (; j !=(size_t) -1; i--, j--)
            {
                if (!IS_ALPHABET::equal_ignore_case(str1[i], str2[j]))
                    return false;
            }
        }

        return true;
    }

    /**
    *
       @brief Returns the number of occurrences for a given pattern in the
                      string.
    *
    */
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

    /**
     * @brief  Returns the counter array (reference argument) each array
                   element representing the number of occurrences of the
    	 keyword in the patterns array.
     */
    static void frequency_counter(const StringT& str, const std::vector<StringT>& patterns, std::vector<size_t>& freq,
                                  StrCompMode caseChk=SM_IGNORE)
    {
        freq.clear();
        freq.reserve(patterns.size());

        for (typename std::vector<StringT>::const_iterator i=patterns.begin(); i!= patterns.end(); i++)
        {
            freq.push_back(num_occurrence(str, *i, 0, caseChk));
        }
    }

    /**
    * @brief Substitute to character for from.
    * @return the times of ocurrence
    */
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

    /**
       @return substituted string.
     */
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

    /**
       * @brief Substitute the to string for from.
       *
         Substitute the to string for from. Beware that it will
                      substitute as many times as necessary.
    		Returns the resulting string.

       *
       */
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
            //std::cout<<str.substr(last, *i-last)<<last<<" "<<*i-last<<"\n";
            StringT ssss = str.substr(last, *i-last);
            //std::cout<<"jjjjjjjjjjjjj\n";
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

    /**
    * @brief Substitute the to string for from.
    *
    Substitute the to string for from. Beware that it will
                   substitute only one time!
    	Returns the resulting string. Aborts if there is no
    	  'from' string.

    *
    */
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


//   static StringT remove_newlines(const StringT& str)
//   {
//     StringT r = str;
//     remove_newlines(r);
//     return r;
//   }

    /**
     * @brief Replace all the occurrences of the given characters with
                     spaces.
     *
      Replace all the occurrences of the given characters with
                     spaces.
     *
     */
    static void remove_duplicate_chars(StringT& str, CharT c)
    {
        StringT s;
        s.reserve(str.length());

        typename StringT::iterator i;
        int count = 0;

        for (i = str.begin(); i < str.end(); i++)
        {
            if (*i == c)
            {
                if (count > 0);
                else s += c;
                count++;
            }
            else
            {
                s += *i;
                count = 0;
            }
        }

        //std::cout<<s.getReferCnt()<<std::endl;
        str = s;
        //std::cout<<s.getReferCnt()<<std::endl;
    }

//   static StringT remove_duplicate_chars(const StringT& str, CharT c)
//   {
//     StringT s = str;
//     remove_duplicate_chars(s, c);
//     return s;

//   }

    static void remove_multi_whitespace(StringT& str)
    {
        remove_duplicate_chars(str, ' ');
    }

//   static StringT remove_multi_whitespace(const StringT& str)
//   {
//     return remove_duplicate_chars(str, ' ');
//   }

    /**
     *   @brief : Returns an array of strings, including delimiters, each
     *                   containing tokens separated by the delimiter argument.
     *
     */
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

        for (i=0; i<indices.size()-1; i++)
        {
            if (indices[i]+delimiter.length()>=indices[i+1])
                continue;
            tokens.push_back(str.substr(indices[i]+delimiter.length(),
                                        indices[i+1]==(size_t)-1? indices[i+1]: indices[i+1]-(indices[i]+delimiter.length())));
        }
    }

    /**
       @brief cut the content of string with the range
     */
    static void cut_range(StringT& str, size_t start, size_t end)
    {
        str = str.substr(start, end - start + 1);
    }

    //   static StringT cut_range(const StringT& str, size_t start, size_t end)
//   {
//     return str.substr(start, end - start + 1);
//   }


    /**
     * @brief string retrieval method.
     *
     string retrieval method. Returns the string from
                      start to right before the delimiter character.
      Does not affect the receiver itself.
     *
     */
    static void cut_delimiter(StringT& str, size_t start, CharT delimiter)
    {
        assert(start < str.length());

        size_t i=start;

        while(str[i]!= delimiter)
        {
            i++;
            if (start + i >= str.length())
            {
                str.clear();
                return;
            }
        }

        if (i==start)
        {
            str.clear();
            return ;
        }


        return cut_range(str, start, i-1);
    }

    //   static StringT cut_delimiter(const StringT& str, size_t start, CharT delimiter)
//   {
//     StringT s =str;
//     cut_delimiter(s, start, delimiter);
//     return s;
//   }


    /**
     * @brief Cut between the specified two patterns.
     *
       Cut between the specified two patterns.
                     The return values starts with the first pattern and ends
                with the second one.

     */
    static void cut_between_words(StringT& str, const StringT& p1,
                                  const StringT& p2, StrCompMode caseChk=SM_SENSITIVE )
    {
        size_t i = find(str, p1, 0, caseChk);
        if (i==(size_t)-1)
        {
            str.clear();
            return;
        }

        size_t j = rfind(str, p2, StringT::npos, caseChk);
        if (j==(size_t)-1)
        {
            str.clear();
            return ;
        }

        if (i+p1.length()>=j)
        {
            str.clear();
            return ;
        }

        str = str.substr(i+p1.length(), j-i-p1.length()-1);
    }
    //   static StringT cut_between_words(const StringT& str, const StringT& p1,
//                                    const StringT& p2, StrCompMode caseChk=SM_SENSITIVE)
//   {
//     StringT s = str;
//     cut_between_words(s, p1, p2, caseChk);
//     return s;
//   }


    /**
     *@brief cut the word after a tag.
     */
    static void cut_word_after(StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE)
    {
        size_t index = find(str, tag,0, caseChk);
        if (index!=(size_t)-1)
        {
            size_t i = index + tag.length();
            for (; i < str.length() && IS_ALPHABET::is_space(str[i]); i++);

            if (i == str.length())
            {
                str.clear();
                return ;
            }


            size_t start = i;
            for (; i < str.length() && !IS_ALPHABET::is_space(str[i]); i++);

            cut_range(str, start, i - 1);
            return;
        }

        str.clear();
    }
//   static StringT cut_word_after(const StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE)
//   {
//     StringT s = str;
//     cut_word_after(s, tag, caseChk);
//     return s;
//   }


    /**
     *@brief cut a line after a tag.
     */
    static void cut_line_after(StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE)
    {
        size_t index = find(str, tag, 0, caseChk);
        if (index!=(size_t)-1)
        {
            size_t i = index + tag.length();
            if (i == str.length())
            {
                str.clear();
                return;
            }

            size_t start = i;
            for (; i < str.length() && !IS_ALPHABET::is_new_line(str[i]); i++);

            cut_range(str, start, i - 1);
            return;
        }

        str.clear();
    }
    // static StringT cut_line_after(const StringT& str, const StringT& tag, StrCompMode caseChk=SM_IGNORE)
//   {
//     StringT s = str;
//     cut_line_after(s, tag, caseChk);
//     return s;
//   }


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

        size_t index = -1;
        size_t level = 0;
        size_t step = 0;

        for (typename StringT::const_iterator i = str.begin();
                i != str.end(); i++)
        {
            if (*i == beg)
            {
                index++;
                level++;
            }
            else if (*i == end)
            {
                if (level > 0)
                {
                    level--;
                    if (level == 0)
                        step = 0;
                    else
                        step++;
                }
            }
            else if (level > 0)
            {
                CharT temp = *i;
                size_t finalIndex = index - step;
                while (finalIndex >=arrayOfstrings.size())
                {
                    arrayOfstrings.push_back(StringT());
                }

                arrayOfstrings[finalIndex] += temp;
            }
            else;
        }
    }

    /**
       *
         @brief : Returns n DynamicArray of strings of which elements contain
                     the words between the marks.
    	       It doesn't contain the marks itself.
       *
       */
    static void take_between_mark(const StringT& str, CharT mark,
                                  std::vector<StringT>& arrayOfstrings)
    {
        arrayOfstrings.clear();
        if (str.length()==0)
            return;

        size_t index = 0;
        size_t prev = 0;

        index = str.find(mark, index);
        while(index == prev)
        {
            prev = index+1;
            index = str.find(mark, prev);
        }

        while (index != (size_t)-1)
        {
            StringT s = str;
            cut_range(s, prev, index - 1);
            arrayOfstrings.push_back(s);
            prev = index + 1;
            index = str.find(mark, index+1);
        }

        StringT s = str;
        cut_range(s, prev, str.length()-1);
        arrayOfstrings.push_back(s);
    }

    /**
      @brief Returns the rear part that begins with the tag, including
                      the tag depending on the option.
    */
    static void reverse_cut_rear_with(StringT& str, CharT tag,
                                      bool includeTag = false, StrCompMode caseChk = SM_IGNORE)
    {
        size_t i;
        for (i = str.length()-1; i!=(size_t)-1; i--)
        {
            if (caseChk == SM_SENSITIVE && str[i]==tag
                    || caseChk == SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i],tag))
                break;
        }

        if (i!=(size_t)-1)
        {
            if (includeTag)
                cut_range(str, i, str.length()-1);
            else if (i < str.length()-1)
                cut_range(str, i+1, str.length()-1);

            return;

        }

        str.clear();
    }
    // static StringT reverse_cut_rear_with(const StringT& str, CharT tag,
//                                        bool includeTag = false, StrCompMode caseChk = SM_IGNORE)
//   {
//     StringT s = str;
//     reverse_cut_rear_with(s, tag, includeTag, caseChk);
//     return s;
//   }



    /**
       @brief cut the content that starts with the tag.
     */
    static void reverse_cut_front_with(StringT& str, CharT tag,
                                       bool includeTag = false, StrCompMode caseChk = SM_IGNORE)
    {
        size_t i;
        for (i = str.length()-1; i!=(size_t)-1; i--)
            if (caseChk == SM_SENSITIVE && str[i]==tag
                    || caseChk == SM_IGNORE && IS_ALPHABET::equal_ignore_case(str[i],tag))
                break;

        if (i!=(size_t)-1)
        {
            if (includeTag)
                cut_range(str, 0, i);
            else if (i!=0)
                cut_range(str, 0, i-1);
            return;
        }

        str.clear();
    }
    //   static StringT reverse_cut_front_with(const StringT& str, CharT tag,
//                                        bool includeTag = false, StrCompMode caseChk = SM_IGNORE)
//   {
//     StringT s = str;
//     reverse_cut_front_with(s, tag, includeTag, caseChk);
//     return s;
//   }

    /**
     @brief Cut trailing spaces in the head.
     */
    static void compact_head(StringT& str)
    {
        size_t i=0;
        for (; i<str.length(); i++)
            if (!IS_ALPHABET::is_space(str[i]))
                break;

        str = str.substr(i);
    }
    // static StringT compact_head(const StringT& str)
//   {
//     StringT s = str;

//     compact_head(s);
//     return s;
//   }


    /**
     @brief Cut trailing spaces in the tail.
     */
    static void compact_tail(StringT& str)
    {
        size_t i=str.length()-1;
        for (; i!=(size_t)-1; i--)
            if (!IS_ALPHABET::is_space(str[i]))
                break;

        str = str.substr(0, i+1);
    }
    // static StringT compact_tail(const StringT& str)
//   {
//     StringT s = str;
//     compact_tail(s);
//     return s;
//   }


    /**
       @brief remove all the spaces at head or tail
     */
    static void compact(StringT& str)
    {
        compact_head(str);
        compact_tail(str);
    }
    // static StringT compact(const StringT& str)
//   {
//     StringT s;
//     s = compact_head(str);
//     compact_tail(s);
//     return s;
//   }


    /**
       @brief remove all the spaces
     */
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

    static StringT trimToOneSpace(const StringT& str)
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
                s += str.substr(last, i-last+1);
            }

            last = i+1;
        }

        if (last < str.length())
            s += str.substr(last);

        return s;
    }

    static StringT padForAlphaNum(const StringT& str)
    {
        StringT s;
        if (str.empty()) return s;
        s.reserve(str.length());

        if (IS_ALPHABET::value(str[0]) || IS_NUMERIC::value(str[0]))
            s.push_back(' ');
        s.push_back(str[0]);

        //size_t last = 0;
        for (size_t current = 1; current < str.length(); ++current)
        {
            if (str[current] == ' ')
            {
                if (str[current - 1] != ' ')
                    s.push_back(' ');
            }
            else if (IS_ALPHABET::value(str[current]))
            {
                if (str[current - 1] != ' ' && (!IS_ALPHABET::value(str[current - 1]) || IS_NUMERIC::value(str[current - 1]) ))
                    s.push_back(' ');
                s.push_back(str[current]);
            }
            else if (IS_NUMERIC::value(str[current]))
            {
                if (str[current - 1] != ' ' && (!IS_NUMERIC::value(str[current - 1]) || IS_ALPHABET::value(str[current - 1]) ))
                    s.push_back(' ');
                s.push_back(str[current]);
            }
            else
            {
                if (IS_ALPHABET::value(str[current - 1]) || IS_NUMERIC::value(str[current - 1]))
                    s.push_back(' ');
                s.push_back(str[current]);
            }
        }

        if (IS_ALPHABET::value(str[str.length() - 1]) || IS_NUMERIC::value(str[str.length() - 1]))
            s.push_back(' ');

        return s;
    }

    static StringT trimHead(const StringT& str)
    {
        StringT s;
        s.reserve(str.length()*3/4);

        size_t i=0;
        // size_t last = i;
        for (; i<str.length(); i++)
        {
            if (!IS_ALPHABET::is_space(str[i]))
                break;
        }
        s += str.substr(i);


        return s;
    }

    static StringT trimTail(const StringT& str)
    {
        StringT s;
        s.reserve(str.length()*3/4);

        size_t i=str.length();
        // size_t last = i;
        for (; i>0; i--)
        {
            if (!IS_ALPHABET::is_space(str[i]))
                break;
        }
        s += str.substr(0,i);


        return s;
    }


    static StringT trim1(const StringT& str)
    {
        return  trimHead(trimToOneSpace(str));
    }

    static StringT trim2(const StringT& str)
    {
        return    trimTail(trim1(str));
    }
    // static void trim(StringT& str)
//   {
//     const StringT s =str;
//     str = trim(s);
//   }

    static void read_from_file(const char* filename, std::vector<StringT>& lines, CharT line='\n')
    {
        std::ifstream file (filename, std::ios::in|std::ios::binary|std::ios::ate);
        // get length of file:
        file.seekg (0, std::ios::end);
        std::size_t length = file.tellg();
        file.seekg (0, std::ios::beg);

        // allocate memory:
        char* buffer = (char*)HLmemory::hlmalloc(length);

        // read data as a block:
        file.read (buffer,length);
        file.close();

        StringT s((CharT* )buffer, length/sizeof(CharT));
        HLmemory::hlfree(buffer);

        make_tokens_with_delimiter(s, line, lines);
    }

    static void read_from_file(const char* encode_t, const char* filename, std::vector<StringT>& lines, CharT line='\n')
    {
        std::ifstream file (filename, std::ios::in|std::ios::binary|std::ios::ate);
        // get length of file:
        file.seekg (0, std::ios::end);
        std::size_t length = file.tellg();
        file.seekg (0, std::ios::beg);

        // allocate memory:
        char* buffer = (char*)HLmemory::hlmalloc(length);

        // read data as a block:
        file.read (buffer,length);
        file.close();

        StringT s;
        read_from_encode(encode_t, buffer, length, s);

        HLmemory::hlfree(buffer);

        make_tokens_with_delimiter(s, line, lines);
    }

    static void write_to_file(const char* filename, const std::vector<StringT>& lines)
    {
        std::ofstream of(filename, std::ios::out|std::ios::binary);

        for (size_t i=0; i<lines.size(); i++)
        {
            for (typename StringT::const_iterator j = lines[i].begin(); j!=lines[i].end(); j++)
            {
                CharT ch = *j;
                of.write((char*)&ch, sizeof(CharT));
            }
            CharT ch = '\n';
            of.write((char*)&ch, sizeof(CharT));
        }

        of.close();
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

    /**
       @brief it generates random strings
       @param size the length of string
     */
    static StringT generate_random_string(size_t size)
    {
        StringT s;
        s.reserve(size);
        CharT span = 'z'-'a';

        for (size_t i=0; i<size; i++)
            s += (CharT)(::rand()%span);

        return s;
    }

    /**
     * Text in input is transformed from encode_type into UCS-2 and stored in output.
     *Generally, this step is ahead of text processing for various encode type text.
     **/
//   static bool read_from_encode(const char* encode_type, const char* input, size_t len, StringT& output)
//   {
//     output.clear();

//     iconv_t ic = iconv_open("ucs-2", encode_type);
//     if (ic == (iconv_t)(-1))
//       return false;

//     size_t ilen = 0;
//     size_t olen = 0;
//     ilen = len;
//     olen = ilen * 4;

//     char *ibuf = const_cast<char *>(input);

//     char *obuf_org = (char*)HLmemory::hlmalloc(olen);
//     char *obuf = obuf_org;
//     std::fill(obuf, obuf + olen, 0);

//     size_t olen_org = olen;
//     iconv(ic, 0, &ilen, 0, &olen);  // reset iconv state

//     while (ilen != 0) {
//       if (iconv(ic, &ibuf, &ilen, &obuf, &olen)
//           == (size_t) -1) {
//         ibuf++;
//         ilen++;
//       }
//       //std::cout<<"uu"<<ilen<<std::endl;
//     }

//     output.assign((CharT*)obuf_org, (olen_org - olen)/sizeof(CharT));

//     HLmemory::hlfree(obuf_org);

//     iconv_close(ic);

//     return true;
//   }

    /**
     *Text in input of UCS-2 can be transformed into encode_type and restored into output.
     *There a new memory block will be applyed for outputing. And the size of the block will be of len.
     *So, the output should be released by calling release at the later section.
     *This step is mainly called after text have been processed.
     **/
//   static bool write_to_encode(const char* encode_type, char** output, size_t& len, const StringT& input)
//   {
//     iconv_t ic = iconv_open(encode_type, "ucs-2");

//     if (ic == (iconv_t)(-1))
//       return false;

//     size_t ilen = 0;
//     size_t olen = 0;
//     ilen = input.size();
//     olen = ilen * 4+1;
//     char *ibuf = (char*)HLmemory::hlmalloc(ilen);
//     char *ibuf_org = ibuf;

//     input.copy((CharT*)ibuf, input.length());

//     *output = (char*)HLmemory::hlmalloc(olen);
//     char *obuf_org = *output;
//     char *obuf = obuf_org;
//     std::fill(obuf, obuf + olen, 0);

//     size_t olen_org = olen;
//     iconv(ic, 0, &ilen, 0, &olen);  // reset iconv state
//     while (ilen != 0) {
//       if (iconv(ic, &ibuf, &ilen, &obuf, &olen)
//           == (size_t) -1) {
//         //std::cout<<"jj"<<ilen<<std::endl;
//         ibuf++;
//         ilen++;
//         //len =  olen_org - olen;
//         //HLmemory::hlfree(ibuf_org);
//         //iconv_close(ic);

//         //return false;
//       }
//       //std::cout<<"yy"<<ilen<<std::endl;
//     }

//     len =  olen_org - olen;

//     HLmemory::hlfree(ibuf_org);

//     iconv_close(ic);
//     return true;
//   }

//   static void release(char* p)
//   {
//     HLmemory::hlfree(p);
//   }

//   /**
//    *Output str in encode_t into os.
//    **/
//   static void display(const StringT& str, const char* encode_t="UTF-8", std::ostream& os=std::cout)
//   {
//     char* buf=NULL;
//     size_t len = 0;

//     if (!write_to_encode(encode_t, &buf, len, str))
//       return;
//     //buf[len-1] = '\0';

//     os<<buf<<std::endl;
//     release(buf);
//   }


}
;

NS_IZENELIB_UTIL_END
#endif
