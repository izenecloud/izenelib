/**
* @file        OutputDescriptor.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef OUTPUTDESCRIPTOR_H
#define OUTPUTDESCRIPTOR_H

#include <ir/index_manager/store/IndexOutput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/// Helper class of combining different IndexOutput, including *.voc, *.dfp, *.pop
class OutputDescriptor
{
public:
    OutputDescriptor(void);
    OutputDescriptor(IndexOutput* pVocOutput,IndexOutput* pDPostingOutput,IndexOutput* pPPostingOutput,bool bDestroy);
    ~OutputDescriptor(void);

public:
    IndexOutput* getVocOutput()
    {
        return pVocOutput;
    }
    IndexOutput* getDPostingOutput()
    {
        return pDPostingOutput;
    };
    IndexOutput* getPPostingOutput()
    {
        return pPPostingOutput;
    };

    void setVocOutput(IndexOutput* pVocOutput)
    {
        pVocOutput = pVocOutput;
    }
    void setDPostingOutput(IndexOutput* pDPostingOutput)
    {
        pDPostingOutput = pDPostingOutput;
    };
    void setPPostingOutput(IndexOutput* pPPostingOutput)
    {
        pPPostingOutput = pPPostingOutput;
    };
protected:
    IndexOutput* pVocOutput;
    IndexOutput* pDPostingOutput;
    IndexOutput* pPPostingOutput;
    bool bDestroy;
};


}


NS_IZENELIB_IR_END

#endif
