/**
* @file        LAInput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef LAINPUT_H
#define LAINPUT_H

#include <types.h>

#include <util/ustring/UString.h>

#include <deque>
#include <boost/serialization/access.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

struct TermId
{
    TermId():termid_(0),docId_(0),wordOffset_(0) {}
    unsigned int termid_;
    unsigned int docId_;
    unsigned int wordOffset_;

    friend std::ostream & operator<<( std::ostream & out, const TermId & term );

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& termid_;
        ar& docId_;
        ar& wordOffset_;
    }
};

class TermIdList : public std::vector<TermId>
{
public:
    void setDocId(unsigned docId)
    {
        globalTemporary_.docId_ = docId;
    }

    inline void add( const unsigned int termid, const unsigned int offset )
    {
        push_back(globalTemporary_);
        back().termid_ = termid;
        back().wordOffset_ = offset;
    }
/*
    template<typename IDManagerType>
    inline void add( IDManagerType* idm, const Term & term)
    {
        push_back(globalTemporary_);
        idm->getTermIdByTermString(term.text_, back().termid_);
        back().wordOffset_ = term.wordOffset_;
    }
*/
    template<typename IDManagerType>
    inline void add( IDManagerType* idm, const izenelib::util::UString::CharT* termStr, const size_t termLen, const unsigned int offset )
    {
        push_back(globalTemporary_);
        idm->getTermIdByTermString(izenelib::util::UString(termStr, termLen), back().termid_);
        back().wordOffset_ = offset;
    }

private:

    TermId globalTemporary_;
};

typedef TermId LAInputUnit;
typedef TermIdList LAInput;
}

NS_IZENELIB_IR_END

#endif
