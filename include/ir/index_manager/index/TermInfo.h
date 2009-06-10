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
    TermInfo(void)
    {
        docFreq_ = 0;
        postingPointer_ = 0;
    }
    TermInfo(const TermInfo& ti)
    {
        docFreq_ = ti.docFreq_;
        postingPointer_ = ti.postingPointer_;
    }
    TermInfo(count_t df,fileoffset_t dp)
    {
        docFreq_ = df;
        postingPointer_ = dp;
    }

    ~TermInfo(void)
    {
    }
public:
    count_t docFreq()const
    {
        return docFreq_;
    }

    void setDocPointer(fileoffset_t pointer)
    {
        postingPointer_ = pointer;
    }

    fileoffset_t docPointer()
    {
        return postingPointer_;
    }

    void set(count_t df,fileoffset_t dp)
    {
        docFreq_ = df;
        postingPointer_ = dp;
    }
private:
    count_t docFreq_;

    fileoffset_t	postingPointer_;
};

}

NS_IZENELIB_IR_END

#endif
