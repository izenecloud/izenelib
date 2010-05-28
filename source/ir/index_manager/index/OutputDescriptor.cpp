#include <ir/index_manager/index/OutputDescriptor.h>

using namespace izenelib::ir::indexmanager;

OutputDescriptor::OutputDescriptor()
        :pVocOutput_(NULL)
        ,pDPostingOutput_(NULL)
        ,pPPostingOutput_(NULL)
        ,bDestroy_(false)
        ,pDirectory_(0)
{
}
OutputDescriptor::OutputDescriptor(IndexOutput* pVocOutput,IndexOutput* pDPostingOutput,IndexOutput* pPPostingOutput,bool bDestroy)
        :pVocOutput_(pVocOutput)
        ,pDPostingOutput_(pDPostingOutput)
        ,pPPostingOutput_(pPPostingOutput)
        ,bDestroy_(bDestroy)
        ,pDirectory_(0)
{
}

OutputDescriptor::~OutputDescriptor()
{
    pVocOutput_->flush();
    pDPostingOutput_->flush();
    pPPostingOutput_->flush();

    if (bDestroy_)
    {
        if (pVocOutput_)
            delete pVocOutput_;
        if (pDPostingOutput_)
            delete pDPostingOutput_;
        if (pPPostingOutput_)
            delete pPPostingOutput_;
    }
    pVocOutput_ = NULL;
    pDPostingOutput_ = NULL;
    pPPostingOutput_ = NULL;
}

