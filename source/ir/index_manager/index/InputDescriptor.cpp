#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <util/izene_log.h>

#include <memory> // unique_ptr

using namespace izenelib::ir::indexmanager;

InputDescriptor::InputDescriptor(bool bDestroy)
        :bDestroy_(bDestroy)
        ,pVocInput_(NULL)
        ,pDPostingInput_(NULL)
        ,pPPostingInput_(NULL)
        ,pBarrelInfo_(NULL)
{

}

InputDescriptor::InputDescriptor(IndexInput* pVocInput,IndexInput* pDPostingInput,IndexInput* pPPostingInput,bool bDestroy)
        :bDestroy_(bDestroy)
        ,pVocInput_(pVocInput)
        ,pDPostingInput_(pDPostingInput)
        ,pPPostingInput_(pPPostingInput)
        ,pBarrelInfo_(NULL)
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

InputDescriptor* InputDescriptor::clone(IndexLevel type)
{
    DVLOG(4) << "=> InputDescriptor::clone(), type: " << type << ", pBarrelInfo_: " << pBarrelInfo_;

    // use unique_ptr in case of memory leak when exception is thrown in IndexInput::clone()
    std::unique_ptr<IndexInput> vocInputPtr, dPostingInputPtr, pPostingInputPtr;
    if(pVocInput_)
        vocInputPtr.reset(pVocInput_->clone());
    if(pDPostingInput_)
        dPostingInputPtr.reset(pDPostingInput_->clone());
    if(type == WORDLEVEL && pPPostingInput_)
        pPostingInputPtr.reset(pPPostingInput_->clone());

    InputDescriptor* pInputDes = new InputDescriptor(vocInputPtr.get(), dPostingInputPtr.get(), pPostingInputPtr.get(), true);

    if(pBarrelInfo_)
        pInputDes->setBarrelInfo(pBarrelInfo_);

    vocInputPtr.release();
    dPostingInputPtr.release();
    pPostingInputPtr.release();

    DVLOG(4) << "<= InputDescriptor::clone()";
    return pInputDes;
}

void InputDescriptor::setBarrelInfo(BarrelInfo* pBarrelInfo)
{
    pBarrelInfo_ = pBarrelInfo;

    if(pVocInput_)
        pVocInput_->setBarrelInfo(pBarrelInfo_);
    if(pDPostingInput_)
        pDPostingInput_->setBarrelInfo(pBarrelInfo_);
    if(pPPostingInput_)
        pPPostingInput_->setBarrelInfo(pBarrelInfo_);
}
