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
    :docFreq_(0)
    ,ctf_(0)
    ,maxTF_(0)
    ,lastDocID_(BAD_DOCID)
    ,skipLevel_(0)
    ,skipPointer_(-1)
    ,docPointer_(-1)
    ,docPostingLen_(0)
    ,positionPointer_(-1)
    ,positionPostingLen_(0)
    ,currTF_(0)    {}

    TermInfo(const TermInfo& ti)
    :docFreq_(ti.docFreq_)
    ,ctf_(ti.ctf_)
    ,maxTF_(ti.maxTF_)
    ,lastDocID_(ti.lastDocID_)
    ,skipLevel_(ti.skipLevel_)
    ,skipPointer_(ti.skipPointer_)
    ,docPointer_(ti.docPointer_)
    ,docPostingLen_(ti.docPostingLen_)
    ,positionPointer_(ti.positionPointer_)
    ,positionPostingLen_(ti.positionPostingLen_)
    ,currTF_(ti.currTF_)    {}

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
        set(ti.docFreq_, ti.ctf_,ti.maxTF_,  ti.lastDocID_, ti.skipLevel_, ti.skipPointer_, ti.docPointer_,
              ti.docPostingLen_, ti.positionPointer_, ti.positionPostingLen_);
    }

    void set(freq_t df, freq_t ctf,freq_t maxTF, docid_t lastDocID,
                 freq_t skipLevel, fileoffset_t skipPointer, fileoffset_t docPointer,
                 freq_t docPostingLen, fileoffset_t positionPointer, freq_t positionPostingLen)
    {
        docFreq_ = df;
        ctf_ = ctf;
        maxTF_ = maxTF;
        lastDocID_ = lastDocID;
        skipLevel_ = skipLevel;
        skipPointer_ = skipPointer;
        docPointer_ = docPointer;
        docPostingLen_ = docPostingLen;
        positionPointer_ = positionPointer;
        positionPostingLen_ = positionPostingLen;
    }

    void reset()
    {
        docFreq_ = 0;
        ctf_ = 0;
        maxTF_ = 0;
        lastDocID_ = BAD_DOCID;
        skipLevel_ = 0;
        skipPointer_ = -1;
        docPointer_ = -1;
        docPostingLen_ = 0;
        positionPointer_ = -1;
        positionPostingLen_ = 0;
    }
public:
    const static int32_t version = 1;

    freq_t docFreq_;

    freq_t ctf_;

    freq_t maxTF_;

    docid_t lastDocID_;

    freq_t skipLevel_;

    fileoffset_t skipPointer_;

    fileoffset_t docPointer_;

    freq_t docPostingLen_;

    fileoffset_t positionPointer_;

    freq_t positionPostingLen_;

    //Do not store,just for real time search
    freq_t currTF_;
};

}

NS_IZENELIB_IR_END

#endif
