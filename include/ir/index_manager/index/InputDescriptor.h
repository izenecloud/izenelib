/**
* @file        InputDescriptor.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef INPUTDESCRIPTOR_H
#define INPUTDESCRIPTOR_H

#include <ir/index_manager/store/IndexInput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/// Helper class of combining different IndexInput, including *.voc, *.dfp, *.pop
enum IndexType
{
    DOCUMENT_LEVEL,  /// document level posting
    WORD_LEVEL /// word level posting
};

class BarrelInfo;
class InputDescriptor
{
public:
    InputDescriptor(bool bDestroy);
    InputDescriptor(IndexInput* pVocInput,IndexInput* pDPostingInput,IndexInput* pPPostingInput,bool bDestroy);
    ~InputDescriptor(void);
public:
    IndexInput*	getVocInput()
    {
        return pVocInput_;
    }
    IndexInput*	getDPostingInput()
    {
        return pDPostingInput_;
    };
    IndexInput*	getPPostingInput()
    {
        return pPPostingInput_;
    };

    void setVocInput(IndexInput* pVocInput)
    {
        pVocInput_ = pVocInput;
    }
    void setDPostingInput(IndexInput* pDPostingInput)
    {
        pDPostingInput_ = pDPostingInput;
    };
    void setPPostingInput(IndexInput* pPPostingInput)
    {
        pPPostingInput_ = pPPostingInput;
    };

    bool destroy()
    {
        return bDestroy_;
    }
    InputDescriptor* clone(IndexType type = WORD_LEVEL);

    void setBarrelInfo(BarrelInfo* pBarrelInfo)
    {
        pBarrelInfo_ = pBarrelInfo;
    }

private:
    /**
     * if @c pBarrelInfo::isRemoved() is true,
     * throw exception @c IndexManagerException.
     */
    void checkBarrelExist();

private:
    bool bDestroy_;
    IndexInput*	pVocInput_;
    IndexInput*	pDPostingInput_;
    IndexInput*	pPPostingInput_;
    BarrelInfo* pBarrelInfo_;
};

}

NS_IZENELIB_IR_END

#endif
