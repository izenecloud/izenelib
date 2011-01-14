#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/utility/Exception.h>
#include <util/izene_log.h>

#include <memory> // auto_ptr

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

InputDescriptor* InputDescriptor::clone(IndexType type)
{
    DVLOG(4) << "=> InputDescriptor::clone(), type: " << type << ", pBarrelInfo_: " << pBarrelInfo_;

    // to avoid opening new merged barrel file,
    // throw exception if old barrel instance is removed
    checkBarrelExist();

    // use auto_ptr in case of memory leak when exception
    // is thrown in checkBarrelExist()
    std::auto_ptr<IndexInput> vocInputPtr, dPostingInputPtr, pPostingInputPtr;
    if(pVocInput_)
        vocInputPtr.reset(pVocInput_->clone());
    if(pDPostingInput_)
        dPostingInputPtr.reset(pDPostingInput_->clone());
    if(type == WORD_LEVEL && pPPostingInput_)
        pPostingInputPtr.reset(pPPostingInput_->clone());

    // to avoid the files opened in previous IndexInput::clone() are new merged barrel files,
    // check again whether old barrel instance is removed
    checkBarrelExist();

    if(pBarrelInfo_)
    {
        if(vocInputPtr.get())
            vocInputPtr->setBarrelInfo(pBarrelInfo_);
        if(dPostingInputPtr.get())
            dPostingInputPtr->setBarrelInfo(pBarrelInfo_);
        if(pPostingInputPtr.get())
            pPostingInputPtr->setBarrelInfo(pBarrelInfo_);
    }

    InputDescriptor* pInputDes = new InputDescriptor(vocInputPtr.get(), dPostingInputPtr.get(), pPostingInputPtr.get(), true);

    vocInputPtr.release();
    dPostingInputPtr.release();
    pPostingInputPtr.release();

    DVLOG(4) << "<= InputDescriptor::clone()";
    return pInputDes;
}

void InputDescriptor::checkBarrelExist()
{
    if(pBarrelInfo_ && pBarrelInfo_->isRemoved())
        SF1V5_THROW(ERROR_FILEIO, "Index dirty.");
}
