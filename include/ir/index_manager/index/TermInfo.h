/**
* @file        TermInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/

#ifndef TERMINFO_H
#define TERMINFO_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///TermInfo is a helper class that indicates a term's posting location and document frequency information
class TermInfo
{
public:
    TermInfo()
        :docFreq_(0),
         ctf_(0),
         lastDocID_(BAD_DOCID),
         skipLevel_(0),
         docPointer_(0),
         docPostingLen_(0),
         positionPointer_(0),
         positionPostingLen_(0)
    {}
		
    TermInfo(const TermInfo& ti)
        :docFreq_(ti.docFreq_),
         ctf_(ti.ctf_),
         lastDocID_(ti.lastDocID_),
         skipLevel_(ti.skipLevel_),
         docPointer_(ti.docPointer_),
         docPostingLen_(ti.docPostingLen_),
         positionPointer_(ti.positionPointer_),
         positionPostingLen_(ti.positionPostingLen_)
    {}

    ~TermInfo() {}
public:
    freq_t docFreq()const
    {
        return docFreq_;
    }

    void setDocPointer(fileoffset_t pointer)
    {
        docPointer_ = pointer;
    }

    fileoffset_t docPointer()
    {
        return docPointer_;
    }

    void set(const TermInfo& ti)
    {
        set(ti.docFreq_, ti.ctf_, ti.lastDocID_, ti.skipLevel_, ti.docPointer_, 
              ti.docPostingLen_, ti.positionPointer_, ti.positionPostingLen_);
    }

    void set(
                   freq_t df,
                   freq_t ctf,
                   docid_t lastDocID,
                   freq_t skipLevel,
                   fileoffset_t docPointer,
                   freq_t docPostingLen,
                   fileoffset_t positionPointer,
                   freq_t positionPostingLen
                   )
    {
        docFreq_ = df;
        ctf_ = ctf;
        lastDocID_ = lastDocID;
        skipLevel_ = skipLevel;
        docPointer_ = docPointer;
        docPostingLen_ = docPostingLen;
        positionPointer_ = positionPointer;
        positionPostingLen_ = positionPostingLen;
    }

    void reset()
    {
        docFreq_ = 0;
        ctf_ = 0;
        lastDocID_ = BAD_DOCID;
        skipLevel_ = 0;
        docPointer_ = -1;
        docPostingLen_ = 0;
        positionPointer_ = -1;
        positionPostingLen_ = 0;
    }
public:
    freq_t docFreq_;

    freq_t ctf_;

    docid_t lastDocID_;

    freq_t skipLevel_;

    fileoffset_t docPointer_;

    freq_t docPostingLen_;

    fileoffset_t positionPointer_;

    freq_t positionPostingLen_;
};

}

NS_IZENELIB_IR_END

#endif
