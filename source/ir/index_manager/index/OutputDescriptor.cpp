#include <ir/index_manager/index/OutputDescriptor.h>

using namespace izenelib::ir::indexmanager;

OutputDescriptor::OutputDescriptor(void)
        :pVocOutput(NULL)
        ,pDPostingOutput(NULL)
        ,pPPostingOutput(NULL)
        ,bDestroy(false)
{
}
OutputDescriptor::OutputDescriptor(IndexOutput* pVocOutput,IndexOutput* pDPostingOutput,IndexOutput* pPPostingOutput,bool bDestroy)
        :pVocOutput(pVocOutput)
        ,pDPostingOutput(pDPostingOutput)
        ,pPPostingOutput(pPPostingOutput)
        ,bDestroy(bDestroy)
{
}

OutputDescriptor::~OutputDescriptor(void)
{
    pVocOutput->flush();
    pDPostingOutput->flush();
    pPPostingOutput->flush();

    if (bDestroy)
    {
        if (pVocOutput)
            delete pVocOutput;
        if (pDPostingOutput)
            delete pDPostingOutput;
        if (pPPostingOutput)
            delete pPPostingOutput;
    }
    pVocOutput = NULL;
    pDPostingOutput = NULL;
    pPPostingOutput = NULL;
}

