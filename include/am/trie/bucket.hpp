/**
   @file bucket.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <stdio.h>
#include <string>
#include <types.h>
#include <algorithm>
#include "alphabet.hpp"
#include "automation.hpp"
#include <vector>
#include <util/log.h>
#include <util/ustring/UString.h>

using namespace izenelib::util;
using namespace std;

template <
class STRING_TYPE
>
struct item_pair
{
    STRING_TYPE str_;
    uint64_t addr_;

    inline item_pair(const STRING_TYPE& str, uint64_t addr)
    {
        str_ = str;
        addr_ = addr;
    }

};

/**
 *@class Bucket
 * B-trie has two typess of nodes, alphabet node and bucket. Bucket stores string and value pair in increasing sequencial order.
 **/
template<
class STRING_TYPE,
      uint32_t BUCKET_SIZE,//byte
      uint8_t SPLIT_RATIO,
      typename STRING_TYPE::value_type* ALPHABET,
      uint32_t ALPHABET_SIZE
      >
class Bucket
{

public:
    typedef typename STRING_TYPE::value_type charT;
    typedef Bucket<STRING_TYPE, BUCKET_SIZE,SPLIT_RATIO,ALPHABET, ALPHABET_SIZE > SelfType;
protected:
    /**
    * @def INITIAL_BUCKET_SIZE The initial bucket size for disk space
    * */
#define INITIAL_BUCKET_SIZE (sizeof(uint32_t)*2 +2*sizeof(charT))
    /**
    * @def DISK_STR_BUF_SIZE The bucket string array size.
    * */
#define DISK_STR_BUF_SIZE (BUCKET_SIZE - INITIAL_BUCKET_SIZE)
    /**
    * @def ONE_STRING_SIZE(STR) One string space it takes.
    * */
#define ONE_STRING_SIZE(STR) ((STR).size()+sizeof(charT)+sizeof(uint32_t)+sizeof(uint64_t))

    static string substr(const string& str, size_t pos)
    {
        return str.substr(pos);
    }

    static izenelib::util::UString substr(const izenelib::util::UString& str, size_t pos)
    {
        izenelib::util::UString tmp;
        str.substr(tmp, pos);

        return tmp;
    }

    struct _disk_bucket_
    {
        charT from_;
        charT to_;
        uint32_t count_;
        uint32_t size_;
        char strBuf[DISK_STR_BUF_SIZE];
    }
    ;

    /**
     * @class _string_ptr_
     * A wrapper class which contains the pointer to a STRING_TYPE object.
     */
    class _string_ptr_
    {
    public:
        STRING_TYPE*  p_;
        uint64_t contentAddr_;

        _string_ptr_(STRING_TYPE* p, uint64_t addr)
            :p_(p),contentAddr_(addr)
        {
        }


        bool operator == (const _string_ptr_& p) const
        {
            return p_->compare(*p.p_)==0;
        }

        bool operator >  (const _string_ptr_& p)const
        {
            return p_->compare(*p.p_)>0;
        }

        bool operator <  (const  _string_ptr_& p)const
        {
            return p_->compare(*p.p_)<0;
        }

        bool operator == (const STRING_TYPE* p)const
        {
            return p_->compare(*p)==0;
        }

        bool operator >  (const STRING_TYPE* p)const
        {
            return p_->compare(*p)>0;
        }

        bool operator <  (const STRING_TYPE* p)const
        {
            return p_->compare(*p)<0;
        }

    }
    ;
    typedef typename vector<_string_ptr_>::iterator str_ptr_it;
    typedef typename vector<_string_ptr_>::const_iterator const_str_ptr_it;

    /**
     * @class _string_group_
     * A group of strings which all share the same first character.
     */
    class _string_group_
    {
    public:
        ~ _string_group_()
        {
        }

        charT firstChar_;
        vector<_string_ptr_> strPtrs_;

        bool operator == (const _string_group_& sg)const
        {
            return firstChar_==sg.firstChar_;
        }

        bool operator > (const _string_group_& sg)const
        {
            return firstChar_ > sg.firstChar_;
        }

        bool operator < (const _string_group_& sg)const
        {
            return firstChar_ < sg.firstChar_;
        }

        bool operator >= (const _string_group_& sg)const
        {
            return firstChar_ >= sg.firstChar_;
        }

        bool operator <= (const _string_group_& sg)const
        {
            return firstChar_ <= sg.firstChar_;
        }
        //////////////////////////
        bool operator == (const STRING_TYPE& str)const
        {
            return firstChar_==str[0];
        }

        bool operator > (const STRING_TYPE& str)const
        {
            return firstChar_ > str[0];
        }

        bool operator < (const STRING_TYPE& str)const
        {
            return firstChar_ < str[0];
        }

        bool operator >= (const STRING_TYPE& str)const
        {
            return firstChar_ >= str[0];
        }

        bool operator <= (const STRING_TYPE& str)const
        {
            return firstChar_ <= str[0];
        }

        size_t addString(STRING_TYPE* pStr, uint64_t addr)
        {
            //STRING_TYPE tmp;
            //pStr->subString(tmp,1);
            *pStr = SelfType::substr(*pStr , 1);

            _string_ptr_ p(pStr, addr);
            str_ptr_it it = lower_bound(strPtrs_.begin(), strPtrs_.end(), p);
            if (it!=strPtrs_.end() && (*it)==p)
            {
                delete pStr;
                return strPtrs_.size();
            }

            strPtrs_.insert(it, p);
            return strPtrs_.size();
        }

    }
    ;

    typedef typename vector<struct _string_group_>::iterator str_group_it;
    typedef typename vector<struct _string_group_>::const_iterator const_str_group_it;
    struct _bucket_
    {
        charT from_;
        charT to_;
        uint32_t count_;
        uint32_t size_;
        bool dirty_;
        vector<_string_group_> strGroup_;
        uint64_t diskPos_;

        inline _bucket_(charT from, charT to)
        {
            /*
              if(from>to)
              throw exception;
            */
            from_ = from;
            to_ = to;
            count_ = 0;
            size_ = INITIAL_BUCKET_SIZE;//byte
            dirty_ = true;
            diskPos_ = -1;
        }

        void setUstring(STRING_TYPE& ustr, const charT* p, size_t len)
        {
            for (size_t i=0; i<len; i++)
                ustr += p[i];
        }

        inline _bucket_(struct _disk_bucket_* disk, uint64_t addr )
        {
            from_ = disk->from_;
            to_ = disk->to_;
            count_ = disk->count_;
            diskPos_ = addr;
            dirty_ = false;
            size_ = disk->size_;

            //cout<<count_<<" "<<size_<<" "<<from_<<" "<<to_<<"  "<<addr<<endl;

            uint32_t t=0;
            charT first = -1;
            for (uint32_t i =0; i<count_; i++)
            {
                uint32_t strSize = *(uint32_t*)(disk->strBuf+t);//ustring's length
                t += sizeof (uint32_t);
                if (t > DISK_STR_BUF_SIZE || strSize > DISK_STR_BUF_SIZE)
                {
                    //THROW exception
                    //LDBG_<<"\nError:  _bucket_(struct _disk_bucket_* disk, uint64_t addr ) 1:"<<debug_count;
                    LDBG_<<"\nError:  _bucket_(struct _disk_bucket_* disk, uint64_t addr ) 1:";
                    return;
                }

                charT firstCh = *(charT*)(disk->strBuf+t);
                t += sizeof (charT);//strSize;

                STRING_TYPE* s = new  STRING_TYPE();
                setUstring(*s, (const charT*)(disk->strBuf+t), strSize);
//         s->displayStringValue(ENCODE_TYPE, cout);
//         cout<<endl;

                t += s->size();

                if (t > DISK_STR_BUF_SIZE)
                {
                    //THROW exception
                    LDBG_<<"\nError:  _bucket_(struct _disk_bucket_* disk, uint64_t addr ) 2:"<<strSize<<"\n";
                    return;
                }

                uint64_t cont = *(uint64_t*)(disk->strBuf+t);
                t += sizeof(uint64_t);
                if (t > DISK_STR_BUF_SIZE)
                {
                    //THROW exception
                    LDBG_<<"\nError:  _bucket_(struct _disk_bucket_* disk, uint64_t addr ) 3:\n";
                    return;
                }


                if (first!=firstCh)
                {
                    _string_group_ sg;
                    sg.firstChar_ = firstCh;
                    sg.strPtrs_.push_back(_string_ptr_(s, cont));
                    first = firstCh;
                    strGroup_.push_back(sg);
                }
                else
                {
                    strGroup_.back().strPtrs_.push_back(_string_ptr_(s, cont));
                }


            }

        }

    }
    ;

public:
    enum slef_size { SIZE_= BUCKET_SIZE};

    /**
     *@param f File handler of bucket data file.
     **/
    Bucket(FILE* f)
        :f_(f),pBucket_(NULL)
    {
        pBucket_ = new _bucket_(ALPHABET[0], ALPHABET[ALPHABET_SIZE-1]);

    }

    ~Bucket()
    {
        for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end(); i++)
        {
            for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
                if ((*j).p_!=NULL)
                {
                    delete (*j).p_;
                    (*j).p_ = NULL;
                }

        }
        delete pBucket_;
        pBucket_ = NULL;
    }

    /**
     *Is the bucket empty.
     **/
    bool isEmpty()
    {
        if (pBucket_==NULL)
            return true;

        if (pBucket_->strGroup_.size()==0)
            return true;

        return false;
    }

    /**
     *Load bucket data from disk starting from the position 'addr'.
     **/
    bool load(uint64_t addr)
    {
        //cout<<"bucket loading!\n";
        struct _disk_bucket_ b;

        fseek(f_, addr, SEEK_SET);
        if(fread(&b, sizeof(struct _disk_bucket_),1,f_)!=1)
            return false;

        if (!isEmpty())
        {
            update2disk();
            for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end(); i++)
            {
                for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
                    if ((*j).p_!=NULL)
                    {
                        delete (*j).p_;
                        (*j).p_ = NULL;
                    }

            }

            delete pBucket_;
            pBucket_ = NULL;
        }

        if (pBucket_!=NULL)
            delete pBucket_;

        pBucket_ = new struct _bucket_(&b, addr);

//     if (getStrCount() != pBucket_->count_)
//       LDBG_<<"Bucket load()**********\n**********\n*************\n************\n*******************Bucket()\n";

//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<pBucket_->diskPos_<<getBucketSize()<<pBucket_->size_<<pBucket_->count_<<"Bucket load(): bucket size error!"<<debug_count;

        return true;

    }

    /**
     *Get the bucket disk size it takes.
     **/
    uint32_t getBucketSize()
    {
        uint32_t s = INITIAL_BUCKET_SIZE;

        for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end(); i++)
        {
            for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
            {
                s += ONE_STRING_SIZE(*((*j).p_));
            }
        }

        return s;

    }

    /**
     *Turn the bucket into disk format.
     **/
    uint32_t bucket2disk(struct _bucket_* b, struct _disk_bucket_* d)
    {
        d->from_ = b->from_;
        d->to_ = b->to_;
        d->count_ = b->count_;
        d->size_ = b->size_;

        uint32_t t=0;

        for (str_group_it i=b->strGroup_.begin(); i!=b->strGroup_.end(); i++)
        {
            for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
            {
//         string tmp;
//         (*j).p_->convertString(tmp, ENCODE_TYPE);

                *(uint32_t*)(d->strBuf+t) = (*j).p_->length();// + sizeof(UCS2Char);

                t += sizeof(uint32_t);
                if (t > DISK_STR_BUF_SIZE)
                {
                    LDBG_<<"bucket2disk(struct _bucket_* b, struct _disk_bucket_* d): 1";
                    return (uint32_t)-1;
                }

                *(charT*)(d->strBuf+t) = (*i).firstChar_;
                t += sizeof(charT);
                //(*j).p_->copy(d->strBuf+t,  (*j).p_->length());

                memcpy(d->strBuf+t, (*j).p_->c_str(),  (*j).p_->size());

                t += (*j).p_->size();
                if (t > DISK_STR_BUF_SIZE)
                {
                    LDBG_<<"bucket2disk(struct _bucket_* b, struct _disk_bucket_* d): 2->"<< (*j).p_->size();
                    return (uint32_t)-1;
                }
                *(uint64_t*)(d->strBuf+t) = (*j).contentAddr_;
                t += sizeof(uint64_t);
            }
        }

        return t;
    }

    /**
     *Update the bucket data onto disk.
     **/
    uint64_t update2disk()
    {
        //cout<<"update2disk()\n";

        if (pBucket_==NULL)
            return false;


        if (pBucket_->dirty_)
        {
            //cout<<"update2disk\n";
            pBucket_->dirty_ = false;
            if (pBucket_->diskPos_ == (uint64_t)-1)
            {
                //cout<<"add2disk\n";
                return add2disk();
            }

            struct _disk_bucket_ b;
            fseek(f_, pBucket_->diskPos_, SEEK_SET);
            bucket2disk(pBucket_, &b);
            if(fwrite(&b, sizeof(struct _disk_bucket_), 1, f_)!=1)
            {
                return (uint64_t)-1;
            }
            return pBucket_->diskPos_;

        }

//     if (getStrCount() != pBucket_->count_)
//       LDBG_<<"Bucket update()**********\n**********\n*************\n************\n*******************Bucket()\n";


//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<pBucket_->diskPos_<<getBucketSize()<<pBucket_->size_<<pBucket_->count_<<"Bucket update(): bucket size error!"<<debug_count;

        return true;
    }

    /**
     *Add the new bucket data into the tail of bucket data file.
     **/
    uint64_t add2disk()
    {
        if (pBucket_==NULL)
            return (uint64_t)-1;

        fseek(f_, 0, SEEK_END);
        uint64_t end = ftell(f_);

        if (end%2==1)
        {
            //bucket only be stored at even address, which distinguish from alphabet node
            fseek(f_, 1, SEEK_END);
            end++;
        }

        pBucket_->diskPos_ = end;
        struct _disk_bucket_ b;
        bucket2disk(pBucket_, &b);
        if (fwrite( &b, sizeof(struct _disk_bucket_), 1, f_)!=1)
            return (uint64_t) -1;

//     if (getStrCount() != pBucket_->count_)
//       LDBG_<<"Bucket add2disk()**********\n**********\n*************\n************\n*******************Bucket()\n";


//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<pBucket_->diskPos_<<getBucketSize()<<pBucket_->size_<<pBucket_->count_<<"Bucket add2disk(): bucket size error!"<<debug_count;

        pBucket_->dirty_ = false;

        return end;
    }

    /**
     *Update the content related to a specific string.
     **/
    bool updateContent(const STRING_TYPE& str, uint64_t contentAddr)
    {
        str_group_it g = lower_bound(pBucket_->strGroup_.begin(), pBucket_->strGroup_.end(), str);
        if (g != pBucket_->strGroup_.end() && *g==str)
        {
            STRING_TYPE s = substr(str ,1);
            //str.subString(s, 1);
            str_ptr_it p = lower_bound((*g).strPtrs_.begin(), (*g).strPtrs_.end(), &s);
            if (p !=  (*g).strPtrs_.end() && (*p)==&s )
            {
                (*p).contentAddr_ = contentAddr;
                return true;
            }

        }
        return false;
    }

    /**
     *Get content of a string.
     **/
    uint64_t getContentBy(const STRING_TYPE& str)const
    {
        str_group_it g = lower_bound(pBucket_->strGroup_.begin(), pBucket_->strGroup_.end(), str);
        if (g != pBucket_->strGroup_.end() && *g==str)
        {
            STRING_TYPE s = substr(str ,1);
            //UString s;
            //str.subString(s, 1);
            str_ptr_it p = lower_bound((*g).strPtrs_.begin(), (*g).strPtrs_.end(), &s);
            if (p !=  (*g).strPtrs_.end() && (*p)==&s )
                return (*p).contentAddr_;
        }

        return (uint64_t)-1;
    }

    /**
     *Find regular expression in trie tree.
     *@param regexp Regular expression.
     *@param sofar Record the string ahead sofar.
     *@param ret The found results.
     **/
    void findRegExp(const STRING_TYPE& regexp, const STRING_TYPE& sofar, vector<item_pair<STRING_TYPE> >& ret)
    {
        if (regexp.length()==0)
            return ;

        if (regexp[0]!='*' && regexp[0]!='?')
        {
            str_group_it g = lower_bound(pBucket_->strGroup_.begin(), pBucket_->strGroup_.end(), regexp);
            if (g != pBucket_->strGroup_.end() && *g==regexp)
            {
                STRING_TYPE s = substr(regexp ,1);
                //regexp.subString(s, 1);
                Automation<STRING_TYPE> aut(s);
                if (!aut.hasWildcard())
                {
                    str_ptr_it p = lower_bound((*g).strPtrs_.begin(), (*g).strPtrs_.end(), &s);
                    if (p !=  (*g).strPtrs_.end() && (*p)==&s )
                    {
                        s = sofar;
                        s += regexp;
                        ret.push_back(item_pair<STRING_TYPE>(s, (*p).contentAddr_));
                        //ret.push_back((*p).contentAddr_);
                        return;
                    }
                    return;
                }

                for (str_ptr_it j=(*g).strPtrs_.begin(); j!=(*g).strPtrs_.end(); j++)
                {
                    if (aut.match(*(*j).p_))
                    {
                        STRING_TYPE tmp = sofar;
                        tmp +=  (*(*j).p_);
                        ret.push_back(item_pair<STRING_TYPE>(tmp, (*j).contentAddr_));
                        //ret.push_back((*j).contentAddr_);
                    }
                }

                return;
            }
        }

        STRING_TYPE s = regexp;
        if (regexp[0]=='?')
        {
            s = substr(regexp,1);
            //regexp.subString(s, 1);
        }


        Automation<STRING_TYPE> aut(s);
        for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end(); i++)
        {
            for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
            {
                if (aut.match(*(*j).p_))
                {
                    STRING_TYPE tmp = sofar;
                    tmp +=  (*(*j).p_);
                    ret.push_back(item_pair<STRING_TYPE>(tmp, (*j).contentAddr_));
                    //ret.push_back((*j).contentAddr_);
                }
            }
        }

    }


    friend ostream& operator << ( ostream& os, const SelfType& node)
    {
        cout<<endl;
        cout<<"-----------------------------\n";
        cout<<"Range: "<<node.pBucket_->from_<<"--"<<node.pBucket_->to_<<endl;
        cout<<"Count: "<<node.pBucket_->count_<<"  Size: "<<node.pBucket_->size_<<endl;
        cout<<"Dirty: "<<node.pBucket_->dirty_<<"  Disk Pos: "<<node.pBucket_->diskPos_<<endl;

        for (str_group_it i=node.pBucket_->strGroup_.begin(); i!=node.pBucket_->strGroup_.end(); i++)
        {
            cout<<"@"<<(*i).firstChar_<<" ........."<<endl;
            for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
            {
                //(*(*j).p_).displayStringValue(ENCODE_TYPE, os);
                os<<" =>"<<(*j).contentAddr_<<endl;
            }
        }

        return os;
    }

    void display(ostream& os)
    {
        os<<*this;
    }

    /**
     *Add string and content pair into bucket
     **/
    uint32_t addString(STRING_TYPE* pStr, uint64_t addr)
    {
        //if (!canAddString(str))
        //return pBucket_->size_;

        if (pStr->length()<=1)
        {
            delete pStr;
            return pBucket_->size_;
        }

        if ((*pStr)[0]<pBucket_->from_ || (*pStr)[0]>pBucket_->to_)
        {
            delete pStr;
            return pBucket_->size_;
        }

//     if (getStrCount() != pBucket_->count_)
//     LDBG_<<"before addstring(): string count in bucket error!";

//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<"before addstring(): bucket size error!"<<debug_count;

        str_group_it t = lower_bound(pBucket_->strGroup_.begin(), pBucket_->strGroup_.end(), (*pStr));

        if (t!=pBucket_->strGroup_.end() && (*t)==(*pStr))
        {
            //has group
            //LDBG_<<"group exist!";

            size_t s = (*t).strPtrs_.size();
            if(s<(*t).addString(pStr, addr))
            {
                pBucket_->count_++;
                pBucket_->size_ += ONE_STRING_SIZE(*pStr);
            }

        }
        else
        {
            // no group at all
            //LDBG_<<"group inexist!";
            _string_group_ g;
            g.firstChar_ = (*pStr)[0];

            g.addString(pStr, addr);

            pBucket_->count_++;
            pBucket_->size_ += ONE_STRING_SIZE(*pStr);
            pBucket_->strGroup_.insert(t,g);
        }

        pBucket_->dirty_ = true;


//     if (getStrCount() != pBucket_->count_)
//      LDBG_<<"after addstring(): string count in bucket error!";

//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<"after addstring(): bucket size error!";

        return pBucket_->size_;
    }

    /**
     *Is it full in bucket.
     **/
    bool isFull()const
    {
        if (pBucket_->size_ >= BUCKET_SIZE)
            return true;

        return false;

    }

    /**
     *Can string be added into bucket.
     **/
    bool canAddString(const STRING_TYPE& str)
    {
        return length()+ONE_STRING_SIZE(str)<=BUCKET_SIZE;
    }

    /**
     *Bucket size.
     **/
    uint32_t length()const
    {
        return pBucket_->size_;
    }

    /**
     *Get count of strings.
     **/
    uint32_t getStrCount()
    {
        uint32_t c = 0;

        for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end(); i++)
            c += (*i).strPtrs_.size();

        return c;
    }

    /**
     *Split bucket into 'newBucket'. Some short string is consumed whitch is stored into leftStr.
     **/
    charT split(Bucket* newBucket, vector<STRING_TYPE>& leftStr, vector<uint64_t>& leftAddr)
    {
//     leftStr.clear();
//     if (getStrCount() != pBucket_->count_)
//       LDBG_<<"before split()**********\n**********\n*************\n************\n*******************before split()\n";

//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<"before split(): bucket size error!";

        if (isPure())
        {
//      std::cout << "pure" << std::endl;
//
//      for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end();i++)
//      {
//        for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end();j++)
//        {
//            std::cout << (*i).firstChar_ << (*(*j).p_) << std::endl;
//        }
//      }

            struct _bucket_* t = pBucket_;
            pBucket_ = new struct _bucket_(ALPHABET[0], ALPHABET[ALPHABET_SIZE-1]);

            for (str_group_it i=t->strGroup_.begin(); i!=t->strGroup_.end(); i++)
            {
                for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end(); j++)
                {
                    if (((*j).p_)->length()<=1)
                    {
                        STRING_TYPE s;
                        s += (*i).firstChar_;
                        s += (*(*j).p_);
                        leftStr.push_back(s);
                        leftAddr.push_back((*j).contentAddr_);
                        delete (*j).p_;
                        continue;
                    }


                    addString(((*j).p_),(*j).contentAddr_ );
                }
            }

            pBucket_->diskPos_ = t->diskPos_;
            pBucket_->dirty_ = true;

            delete t;

        }

        if (pBucket_->strGroup_.size()==1)
        {
            //throw exception
//      cout<<"\n-------------------Split error!--------------------";
            return (charT)-1;
        }

        uint32_t left = (uint32_t)(pBucket_->count_/100.00*SPLIT_RATIO);
        //
        uint32_t c = 0;

        str_group_it i=pBucket_->strGroup_.begin();

        for (size_t j=1; i!=pBucket_->strGroup_.end(); i++,j++)
        {
            //cout<<(*i).strPtrs_.size()<<endl;

            if (c>=left || j==pBucket_->strGroup_.size())
            {
                //found the spliting point
                pBucket_->count_ -= (*i).strPtrs_.size();
                pBucket_->size_ -= newBucket->addStrGroup(*i);

                i = pBucket_->strGroup_.erase(i);
                i--;
                j--;
            }
            c += (*i).strPtrs_.size();
        }

        newBucket->setUpBound(ALPHABET[getIndexOf(pBucket_->strGroup_.back().firstChar_)+1]);
        newBucket->setLowBound(pBucket_->to_);
        pBucket_->to_ = (pBucket_->strGroup_.back()).firstChar_;

        pBucket_->dirty_ = true;

//     if (getStrCount() != pBucket_->count_)
//       LDBG_<<"split()**********\n**********\n*************\n************\n*******************split()\n";
//     if (pBucket_->size_ != getBucketSize())
//       LDBG_<<"split(): bucket size error!";

        return pBucket_->to_;
    }

    /**
     *Get index of 'ch' in alphabet.
     **/
    static uint32_t getIndexOf(charT ch)
    {
        if (ch<ALPHABET[0] || ch>ALPHABET[ALPHABET_SIZE-1])
        {
            LDBG_<<"Can't find '"<<ch<<"' in alphabet";
            return -1;
        }

        if(ALPHABET[0] == 'a' && ALPHABET[ALPHABET_SIZE-1]=='z')
        {
            if (ch <= 'Z' && ch >='A')
                return ch-'A';
            return ch-ALPHABET[0];
        }

        uint32_t start = 0;
        uint32_t end  = ALPHABET_SIZE -1;
        uint32_t mid = (start + end)/2;

        while ( mid<=end && mid>=start)
        {
            if (ALPHABET[mid]==ch)
                return mid;

            if (ALPHABET[mid]<ch)
            {
                start = mid+1;
                mid = (start + end)/2;
                continue;
            }

            if (ALPHABET[mid]>ch)
            {
                end = mid-1;
                mid = (start + end)/2;
                continue;
            }
        }

        return -1;
    }

    /**
     *Is bucket pure.
     **/
    bool isPure() const
    {
        return (pBucket_->to_ -pBucket_->from_)==0;
    }

    /**
     *Get up bound of bucket
     **/
    charT getUpBound()const
    {
        return pBucket_->from_;
    }

    /**
     *Get string group count.
     **/
    size_t getStrGroupAmount()const
    {
        return pBucket_->strGroup_.size();
    }

    /**
     *Get disk address of this bucket.
     **/
    uint64_t getDiskAddr()const
    {
        return pBucket_->diskPos_;
    }

    /**
     *Get the 'idx' group first char whitch stands for this group
     **/
    charT getGroupChar(size_t idx)
    {
        if (idx>=getStrGroupAmount())
            return (charT)-1;

        return pBucket_->strGroup_[idx].firstChar_;

    }

    /**
     *Get low bound of this bucket.
     **/
    charT getLowBound() const
    {
        return pBucket_->to_;
    }

    /**
     *Add a string group.
     **/
    uint32_t addStrGroup(const _string_group_& g)
    {
        uint32_t size = 0;
        for (const_str_ptr_it j=g.strPtrs_.begin(); j!=g.strPtrs_.end(); j++)
        {
            //cout<<*((*j).p_)<<endl;
            size += ONE_STRING_SIZE(*((*j).p_));
            pBucket_->count_++;
        }


        pBucket_->strGroup_.push_back(g);
        pBucket_->size_ += size;
        pBucket_->dirty_ = true;

        return size;
    }

    /**
     *Set up bound of this bucket.
     **/
    void setUpBound(charT ch)
    {
        if (pBucket_->from_!=ch)
            pBucket_->dirty_ = true;

        pBucket_->from_ = ch;
    }

    /**
     *Set low bound of this bucket.
     **/
    void setLowBound(charT ch)
    {
        if (pBucket_->to_!=ch)
            pBucket_->dirty_ = true;

        pBucket_->to_ = ch;

    }


protected:
    FILE* f_; //!< File handler.
    struct _bucket_* pBucket_;//!< Storage of bucket data.


}
;






#endif
