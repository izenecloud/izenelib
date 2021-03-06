/**
   @file alphabet.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef ALPHABET_H
#define ALPHABET_H

#include<string>
#include <ostream>
#include <cctype>
#include <algorithm>
#include <util/ustring/UString.h>

using namespace izenelib::util;
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

    AlphabetGenerator(const string& name)
    {
        name_ = name;
    }

    void addCharsFromString(const string& str, UString::EncodingType encoding)
    {
        UString ustr(str, encoding);
        for (size_t i=0; i<ustr.length(); i++)
        {
            UCS2Char ch = ustr[i];
            std::vector<unsigned short>::iterator it = chars.begin();
            for(; it != chars.end(); it ++)
            {
                if( ch > *it )
                    continue;
                break;
            }
            chars.insert(it, ch);
        }
    }

    void addCharsFromRange(unsigned short start, unsigned short stop)
    {
        std::vector<unsigned short>::iterator it = chars.begin();
        for(; it != chars.end(); it ++)
        {
            if( start > *it )
                continue;
            break;
        }
        while(start <= stop)
        {
            it = chars.insert(it, start);
            ++it;
            ++start;
        }
    }

    /**
     *It prints the sorted alphabet in console
     **/
    friend ostream& operator << ( ostream& os, const AlphabetGenerator& alp)
    {
        std::string uname;
        uname.resize(alp.name_.size());
        std::transform(alp.name_.begin(), alp.name_.end(), uname.begin(), ::toupper);
        std::string include_macro = "_" + uname + "_ALPHABET_H_";

        os<<"#ifndef " << include_macro << std::endl;
        os<<"#define " << include_macro << std::endl<<std::endl;

        os<<"/* Alphabet table for " << alp.name_ << " language, generated by izenelib::am::trie::AlphabetGenerator */"
          << std::endl << std::endl;
        os<<"unsigned int " << alp.name_  << "_size = " << alp.chars.size() << ";"
          << std::endl << std::endl;
        os<<"unsigned short " << alp.name_  << "[" << alp.chars.size() <<"] = " << std::endl;
        os<< "{" << std::endl;
        for (size_t i=0; i<alp.chars.size(); i++)
        {
            if(i%8 == 7)
                os << std::endl;
            else
                os << "\t";
            os << alp.chars[i];
            if(i!=alp.chars.size()-1)
                os << ",";
        }
        os<< "};" << std::endl << std::endl;

        os<<"#endif" << std::endl;
        return os;
    }

protected:

    std::string name_;

    std::vector<unsigned short> chars;

}
;

#endif
