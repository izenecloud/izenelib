/*
 * =====================================================================================
 *
 *       Filename:  kstring.hpp
 *
 *    Description:  unicode string with java::string interface and copy-on-write mem management.
 *
 *        Version:  1.0
 *        Created:  2013年05月20日 09时37分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */
#ifndef IZENELIB_STRING_KSTRING__HPP
#define IZENELIB_STRING_KSTRING__HPP

#include "types.h"
#include "/usr/include/iconv.h"
#include <cerrno>
#include <string>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
# include <ext/atomicity.h>
#else
# include <bits/atomicity.h>
#endif

namespace izenelib {namespace util{
/**
 * @brief Unicode string.
 * Structrue is like this: 
 * { 
 *      uint32_t reference_count;
 *      uint32_t the capacity of buffer in sizeof(uint16_t);
 *      uint32_t # of chars of string;
 *      uint16_t* unicode array of string;
 * }
 */
class KString
{
    uint32_t* mem_;

#define CHECK_NULL(mem) {if(!mem)return;}

    uint32_t& reference_count_()
    {
        IASSERT(mem_ != NULL);
        return (*(uint32_t*)mem_);
    }

    uint32_t& char_num_()
    {
        IASSERT(mem_ != NULL);
        return (*(uint32_t*)(mem_+2));
    }

    uint32_t& capacity_()
    {
        IASSERT(mem_ != NULL);
        return (*(uint32_t*)(mem_+1));
    }

    uint32_t capacity_()const
    {
        if (!mem_)return 0;
        return (*(uint32_t*)(mem_+1));
    }

    uint32_t total_bytes_(uint32_t char_num = 0)const
    {
        if (char_num == 0 && mem_)
          return sizeof(uint32_t)*3  +sizeof(uint16_t)*capacity_();
        return sizeof(uint32_t)*3  +sizeof(uint16_t)*char_num;
    }

    uint16_t* unicodes_()
    {
        IASSERT(mem_ != NULL);
        return (uint16_t*)(mem_+3);
    }

    uint16_t* unicodes_()const
    {
        IASSERT(mem_ != NULL);
        return (uint16_t*)(mem_+3);
    }

    void refer_()
    {
        CHECK_NULL(mem_);
         __gnu_cxx::__atomic_add(( _Atomic_word*)(uint32_t*)mem_, 1);
    }

    void defer_()
    {
        CHECK_NULL(mem_);
        if (__sync_add_and_fetch(( _Atomic_word*)(uint32_t*)mem_, -1) <= 0)
        {
            free(mem_);
            mem_ = NULL;
        }
    }

    uint32_t string_len_()const
    {
        if (!mem_)return 0;
        return (*(uint32_t*)(mem_+2));
    }

    uint32_t expansion_(uint32_t t)
    {
        if (t + string_len_() < 100)return 2*(t + string_len_());
        if (t + string_len_() < 1000)return 1.5*(t + string_len_());
        return 1.1*(t + string_len_());
    }

    void copy_on_write_()
    {
        CHECK_NULL(mem_);
        IASSERT(reference_count_() > 0);
        if (reference_count_() == 1)
          return;
        uint32_t s = total_bytes_();
        uint32_t* m =  (uint32_t*)malloc(s);
        memcpy(m, mem_, s);
        defer_();
        mem_ = m;
        reference_count_() = 1;
    }

    std::string encoding_name_(const std::string encode)
    {
        if (strcmp(encode.c_str(), "utf8") == 0)
          return "utf-8";
        return encode;
    }
    public:
    explicit KString(const std::string& str, const std::string& encode="utf-8//IGNORE")
        :mem_(NULL)
    {
        if (str.length() == 0)
          return;
        reserve(str.length());
        std::size_t inlen = str.length();
        std::size_t outlen = capacity_()*2;
        char* out = (char*)unicodes_();
        char* in = const_cast <char *> (str.c_str());

        iconv_t hdl = iconv_open("ucs-2", encode.c_str());//encoding_name_(encode).c_str()) ;
        if (hdl == (iconv_t)-1)
          throw std::runtime_error("Can't initiate iconv handler");
        std::size_t ret;
        while(1)
        {
            ret = iconv(hdl, &in, &inlen, &out, &outlen);
            if (inlen == 0)
              break;
            if (ret == (std::size_t)-1)
            {
                iconv_close(hdl);
                if(errno == E2BIG)
                    throw std::runtime_error("Not enough output buffer for conversion.");
                if (errno == EINVAL)
                    throw std::runtime_error("Incomplete multibyte sequence.");
                if (errno == EILSEQ)//std::cerr<<"Invalid multibyte sequence.\n";
                    throw std::runtime_error("Invalid multibyte sequence.");
                throw std::runtime_error("iconv error");
            }
        }
        iconv_close(hdl);
        char_num_() = (capacity_()*2 - outlen)/2;
        reference_count_() = 1;
    }
     ~KString()
     {
         defer_();
     }

    explicit KString()
        :mem_(NULL)
    {}

    /**
     * @brief 
     *
     * @param s
     * @param e exclusivly
     */
    explicit KString(uint16_t* s, uint16_t* e)
    {
        uint32_t len = (e - s);
        uint32_t b = total_bytes_(len);
        mem_ = (uint32_t*)malloc(b);
        memset(mem_, 0, b);
        memcpy(unicodes_(), s, len*sizeof(uint16_t));
        capacity_() = len;
        char_num_() = len;
        reference_count_() = 1;
    }

    KString(const KString& o)
    {
        mem_ = o.mem_;
        refer_();
    }

    KString& operator = (const KString& o)
    {
        defer_();
        mem_ = o.mem_;
        refer_();
        return *this;
    }

    uint32_t length()const
    {
        return string_len_();
    }

    uint16_t char_at(uint32_t i)const
    {
        IASSERT(i < string_len_());
        return unicodes_()[i];
    }

    uint16_t& operator [] (uint32_t i)
    {
        IASSERT(i < string_len_());
        copy_on_write_();
        return unicodes_()[i];
    }

    uint16_t operator [] (uint32_t i)const
    {
         return char_at(i);
    }

    int32_t compare_to(const KString& o)const
    {
        uint32_t i=0;
        for ( ; i<length() && i<o.length(); ++i)
          if (char_at(i) < o.char_at(i))
            return -1;
          else if (char_at(i) > o.char_at(i))
            return 1;

        if (length() > i)
          return 1;
        else if (o.length() > i)
          return -1;
        return 0;
    }

    void reserve(uint32_t len)
    {
        if (mem_ && len < capacity_())
          return;

        uint32_t s = total_bytes_(len);
        IASSERT(s > sizeof(uint32_t)*3);
        uint32_t* m = (uint32_t*)malloc(s);
        memset(m, 0, s);
        if (mem_)
        {
            IASSERT(total_bytes_() > sizeof(uint32_t)*3);
            IASSERT(total_bytes_() < s);
            memcpy(m, mem_, total_bytes_());
            defer_();
        }
        mem_ = m;
        capacity_() = len;
        reference_count_() = 1;
        IASSERT(capacity_() >= char_num_());
    }

    void concat(const KString& o)
    {
        if (o.length() == 0)return;
        if (length() == 0)
        {
            *this = o;
            return;
        }
        if (mem_ && length() + o.length() < capacity_())
        {
            copy_on_write_();
            uint32_t l = char_num_();
            char_num_() += o.length();
            for ( uint32_t i=0; i<o.length(); ++i)
              unicodes_()[l+i] = o[i];
            return;
        }
        reserve(expansion_(o.length()));
        copy_on_write_();
        uint32_t l = char_num_();
        char_num_() += o.length();
        for ( uint32_t i=0; i<o.length(); ++i)
           unicodes_()[l+i] = o[i];
    }

    void concat(uint16_t c)
    {
        if (mem_ && length() + 1 < capacity_())
        {
            copy_on_write_();
            char_num_() ++;
            unicodes_()[char_num_()-1] = c;
            //std::cout<<(char)c<<":::"<<char_num_()<<std::endl;
            //std::cout<<(*this)<<std::endl;
            return;
        }
        reserve(expansion_(1));
        copy_on_write_();
        char_num_()++;
        unicodes_()[char_num_()-1] = c;
        //std::cout<<(char)c<<"::"<<char_num_()<<std::endl;
        //  std::cout<<(*this)<<std::endl;
    }   

    KString& operator += (const KString& o)
    {
        concat(o);
        return *this;
    }

    KString& operator += (uint16_t c)
    {
        concat(c);
        return *this;
    }

    KString& operator + (const std::string& utf8str)
    {
        concat(KString(utf8str));
        return *this;
    }

    KString& operator + (uint16_t c)
    {
        concat(c);
        return *this;
    }

    bool end_with(const KString& o)const
    {
        if (length() == 0 || o.length() == 0 || length() < o.length())
          return false;
        for ( int32_t i = o.length() -1, j=length()-1; i >=0 && j>=0; --i,--j)
          if (o[i] != char_at(j))
            return false;
        return true;
    }

    bool end_with(const std::string& utf8str)const
    {
        return end_with(KString(utf8str));
    }

    bool equals(const KString& o)const
    {
        return compare_to(o) == 0;
    }

    bool equals(const std::string& utf8str)const
    {
        return compare_to(KString(utf8str)) == 0;
    }

    bool operator == (const KString& o)const
    {
        return equals(o);
    }

    bool operator == (const std::string& utf8str)const
    {
        return equals(KString(utf8str));
    }

    uint16_t* get_bytes()const
    {
        return unicodes_();
    }

    std::string get_bytes(const std::string& encode)const
    {
        if (length() == 0)return "";

        char* out = new char[length()*3];
        char* outbuf = out;
        std::size_t inlen = length() *2 ;
        std::size_t outlen = length() *3;
        char* inbuf = (char*)unicodes_();
        std::size_t ret = 0;
        iconv_t hdl = iconv_open(encode.c_str(), "ucs-2") ;
        if (hdl == (iconv_t)-1)
        {
            delete[] out;
            throw std::runtime_error("Can't initiate iconv handler");
        }

        while(1)
        {
            ret = iconv(hdl, &inbuf, &inlen, &outbuf, &outlen);
            if (ret == 0)
              break;
            if (ret == (std::size_t)-1 && errno == E2BIG)
            {
                iconv_close(hdl);
                delete[] out;
                throw std::runtime_error("encoding convert error");
            }
            inbuf++;
            inlen--;
        }
        
        iconv_close(hdl);
        if (outlen == (std::size_t)-1){
            delete out;
            throw std::runtime_error("Not malloc enough memory.");
        }

        std::string re(out, length()*3-outlen);
        delete[] out;
        return re;
    }

    friend std::ostream& operator << (std::ostream& os, const KString& o)
    {
        os << o.get_bytes("utf-8");
        return os;
    }
    
    uint32_t index_of(uint16_t c, uint32_t start_from=0)const
    {
        for ( uint32_t i=start_from; i<length(); ++i)
          if (char_at(i) == c)
            return i;
        return -1;
    }

    uint32_t find(const std::string& o)const
    {
        return find(KString(o));
    }

    uint32_t find(const KString& o)const
    {
        if (length() == 0 || o.length() == 0 || o.length() > length())
          return -1;
        for ( uint32_t i=0; i<=length()-o.length(); ++i)
          if (char_at(i) == o[0])
          {
              uint32_t j=0;
              for ( ; j<o.length(); ++j)
                if (char_at(i+j) != o[j])
                  break;
              if (j == o.length())
                return i;
          }
        return -1;
    }

    uint32_t find(int o)const
    {
        if (length() == 0)
          return -1;
        for ( uint32_t i=0; i<length();i++)
            if (char_at(i) == o)
                return i;
        return -1;
    }

    /**
     * @brief Replace all 'a' with 'b'
     *
     * @param a
     * @param b
     */
    void replace(uint16_t a, uint16_t b)
    {
        if (a == b)
          return;
        copy_on_write_();
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) == a)
            unicodes_()[i] = b;
    }

    /**
     * @brief Replace all string "a" with "b"
     *
     * @param a
     * @param b
     */
    void replace(const KString& a, const KString& b)
    {
        uint32_t p = find(a);
        if (p == (uint32_t)-1)
          return;
        if (a.length() < b.length())
          reserve(expansion_(length() +  b.length() - a.length()));
        copy_on_write_();
        for ( uint32_t i = length()-1, j=1; i>=p+a.length(); --i,++j)
          unicodes_()[length() +  b.length() - a.length()-j] =  unicodes_()[i];
        for ( uint32_t i=0; i<b.length(); ++i)
          unicodes_()[p+i] = b[i];

        char_num_() = length() - a.length() + b.length();
    }

    void  replace(const std::string& utf8a, const std::string& utf8b)
    {
        replace(KString(utf8a), KString(utf8b));
    }

    /**
     * @brief Get the substr, start from index 's' with the length 'len'.
     *
     * @param s
     * @param len if it's larger than length(), it will be assigned to length()
     *
     * @return the substring.
     */
    KString substr(uint32_t s, uint32_t len = -1)const
    {
        IASSERT(s < length());
        if (length() == 0 || len == 0)return KString();
        if (len == (uint32_t)-1) len = length() - s;
        IASSERT(len + s <= length());
        return KString(unicodes_()+s, unicodes_()+s+len);
    }
    /**
     * @brief Split the string by char c into a vector of string
     *
     * @param c the delimitor
     *
     * @return the vector of string
     */
    std::vector<KString>
        split(uint16_t c)
        {
            std::vector<KString> v;
            uint32_t s = 0;
            uint32_t f = index_of(c);
            while (f != (uint32_t)-1)
            {
                v.push_back(substr(s, f- s));
                s = f+1;
                f = index_of(c, s);
            }
            if (s < length())
              v.push_back(substr(s));
            return v;
        }

    void to_lower_case()
    {
        bool f = true;
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) >= 'A' && char_at(i)<='Z')
          {
              f = false;
              break;
          }
        if (f)return;
        copy_on_write_();
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) >= 'A' && char_at(i)<='Z')
            unicodes_()[i] = 'a' + char_at(i) - 'A';
        return;
    }
    
    void to_upper_case()
    {
        bool f = true;
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) >= 'a' && char_at(i)<='z')
          {
              f = false;
              break;
          }
        if (f)return;
        copy_on_write_();
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) >= 'a' && char_at(i)<='z')
            unicodes_()[i] = 'A' + char_at(i) - 'a';
        return;
    }

    void to_dbc()
    {
        bool f = true;
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) == 1228 || (char_at(i) > 65280 && char_at(i) < 65375))
          {
              f = false;
              break;
          }
        if (f)return;
        copy_on_write_();
        for ( uint32_t i=0; i<length(); ++i)
          if (char_at(i) == 1228)
            unicodes_()[i] = 32;
          else if (char_at(i) > 65280 && char_at(i) < 65375)
            unicodes_()[i] -= 65248;
    }

    void trim(uint16_t space = ' ')
    {
        uint32_t p = index_of(space);
        if (p == (uint32_t)-1)
          return;

        copy_on_write_();
        uint32_t r = p +1;
        uint32_t t = 1;
        for(;r<length();++p,++r)
        {
            while(r<length()&&char_at(r) == space)r++,t++;
            unicodes_()[p] = char_at(r);
        }
        char_num_() -= t;
    }

    void trim_head_tail(uint16_t space = ' ')
    {
        if (length() == 0 || (length()==1&&char_at(0)!=space))
            return;

        uint32_t p = 0;
        while(p<length() && char_at(p) == space)
            p++;
        if (p >= length())
        {
            copy_on_write_(), char_num_() = 0;
            return;
        }
        if (p != 0)
        {
            copy_on_write_();
            for (uint32_t i=0;i+p<length();++i)
                unicodes_()[i] = char_at(i+p);
        }
        uint32_t t = length()-p;
        for (int32_t i=t-1; i>=0 && char_at(i)==space; --i,--t);

        if ( p == 0 && length()!= t)
            copy_on_write_();
        char_num_() = t;
    }

    void trim_into_1(uint16_t space = ' ')
    {
        uint32_t f = 0, t = 0, s = -2;
        bool chng = false;
        while (f < length())
        {
            if (char_at(f) == space)
            {
                if (f - s == 1){f++,s++;continue;}
                s = f;
            }

            if (t != f && !chng)
            {
                copy_on_write_();
                chng = true;
            }

            if (t != f)
                unicodes_()[t] = char_at(f);
            t++, f++;
        }
        if (chng) char_num_() = t;
    }

    static KString value_of(uint32_t v)
    {
        char buf[125];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", v);
        return KString(std::string(buf));
    }
    
    static KString value_of(int v)
    {
        char buf[125];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", v);
        return KString(std::string(buf));
    }
    
    static KString value_of(double v)
    {
        char buf[125];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%f", v);
        return KString(std::string(buf));
    }

    bool operator < (const KString& o)const
    {
        for ( uint32_t i=0; i<length()&&i<o.length(); ++i)
          if (char_at(i) < o[i])
            return true;
          else if (char_at(i) > o[i])
            return false;

        if (length() < o.length())
          return true;
        return false;
    }

    static bool is_korean(uint16_t ucs2char)
    {   
        if ((ucs2char>=0x1100 && ucs2char<=0x11FF)
          ||(ucs2char>=0x3130 && ucs2char<=0x318F)
          ||(ucs2char>=0xAC00 && ucs2char<=0xD7AF)
          )return true;
        return false;
    }   

    static bool is_chinese(uint16_t ucs2char)
	{
		if (((ucs2char>=0x2E80 && ucs2char<=0x2EF3)
					|| (ucs2char>=0x2F00 && ucs2char<=0x2FD5)
					|| (ucs2char>=0x3400 && ucs2char<=0x4DB5)
					|| (ucs2char>=0x4E00 && ucs2char<=0x9FC3)
					|| (ucs2char>=0xF900 && ucs2char<=0xFAD9))
          && ucs2char!=12289 
          && ucs2char!=12298 
          && ucs2char!=12290 
          && ucs2char!=12299 
          && ucs2char!=65292 
          && ucs2char!=65311 
          && ucs2char!=65281 
          && ucs2char!=65306 
          && ucs2char!=65307 
          && ucs2char!=8220 
          && ucs2char!=8221 
          && ucs2char!=12304 
          && ucs2char!=12305 
          && ucs2char!=65509 
          && ucs2char!=8230 
          && ucs2char!=65288 
          && ucs2char!=65289 
          && ucs2char!=8212
          && ucs2char!=20022) 
		  return true;

		return false;
	}

    static bool is_chn_numeric(uint16_t ucs2char)
    {
        if (ucs2char == 38646//零
            || ucs2char == 19968//一
            || ucs2char == 20108//二
            || ucs2char == 19977
            || ucs2char == 22235
            || ucs2char == 20116
            || ucs2char == 20845
            || ucs2char == 19971
            || ucs2char == 20843
            || ucs2char == 20061
            || ucs2char == 21313//十
            )
            return true;
        return false;
    }

	static bool is_numeric(uint16_t ucs2char)
	{
		static const uint16_t zero('0'), nine('9');
		    if ( zero <= ucs2char && ucs2char <= nine )
			          return true;
			    return false;
	}	

	static bool is_english(uint16_t ucs2char)
	{
		static const uint16_t a('a'), z('z'), A('A'), Z('Z');
		    if ( ( a <= ucs2char && ucs2char <= z ) || ( A <= ucs2char && ucs2char <= Z ) )
			          return true;
			    return false;
	}

};
}}//end of namespace
#endif
