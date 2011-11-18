/**
* @file        PostingWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief posting builder
*/

#ifndef POSTING_WRITER_H
#define POSTING_WRITER_H


NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class OutputDescriptor;
class TermInfo;
struct PostingWriter
{
    virtual ~PostingWriter(){}

    virtual void add(uint32_t docId, uint32_t pos, bool realTimeFlag=false) = 0;

    virtual bool isEmpty() = 0;

    virtual void write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo) = 0;

    virtual void reset() = 0;
};

}

NS_IZENELIB_IR_END


#endif
