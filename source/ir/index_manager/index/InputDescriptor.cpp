#include <ir/index_manager/index/InputDescriptor.h>


using namespace izenelib::ir::indexmanager;

InputDescriptor::InputDescriptor(bool bDestroy)
        :bDestroy_(bDestroy)
        ,pVocInput_(NULL)
        ,pDPostingInput_(NULL)
        ,pPPostingInput_(NULL)
{

}

InputDescriptor::InputDescriptor(IndexInput* pVocInput,IndexInput* pDPostingInput,IndexInput* pPPostingInput,bool bDestroy)
        :bDestroy_(bDestroy)
        ,pVocInput_(pVocInput)
        ,pDPostingInput_(pDPostingInput)
        ,pPPostingInput_(pPPostingInput)
{
}

InputDescriptor::~InputDescriptor()
{
    if (bDestroy_)
    {
        if (pVocInput_)
            delete pVocInput_;
        if (pDPostingInput_)
            delete pDPostingInput_;
        if (pPPostingInput_)
            delete pPPostingInput_;
    }
    pVocInput_ = NULL;
    pDPostingInput_ = NULL;
    pPPostingInput_ = NULL;
}

InputDescriptor* InputDescriptor::clone(IndexType type)
{
    IndexInput* pVocInput = pVocInput_?pVocInput_->clone():NULL;
    IndexInput* pDPostingInput = pDPostingInput_?pDPostingInput_->clone():NULL;
    IndexInput* pPPostingInput = NULL;
    if(type == WORD_LEVEL)
        pPPostingInput = pPPostingInput_?pPPostingInput_->clone():NULL;
    return new InputDescriptor(pVocInput, pDPostingInput, pPPostingInput, true);
}

