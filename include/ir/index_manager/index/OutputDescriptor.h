/**
* @file        OutputDescriptor.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef OUTPUTDESCRIPTOR_H
#define OUTPUTDESCRIPTOR_H

#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/Directory.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/// Helper class of combining different IndexOutput, including *.voc, *.dfp, *.pop
class OutputDescriptor
{
public:
    OutputDescriptor();
    OutputDescriptor(IndexOutput* pVocOutput,IndexOutput* pDPostingOutput,IndexOutput* pPPostingOutput,bool bDestroy);
    ~OutputDescriptor();

public:
    IndexOutput* getVocOutput()
    {
        return pVocOutput_;
    }
    IndexOutput* getDPostingOutput()
    {
        return pDPostingOutput_;
    }
    IndexOutput* getPPostingOutput()
    {
        return pPPostingOutput_;
    }
    void setVocOutput(IndexOutput* pVocOutput)
    {
        pVocOutput_ = pVocOutput_;
    }
    void setDPostingOutput(IndexOutput* pDPostingOutput)
    {
        pDPostingOutput_ = pDPostingOutput_;
    }
    void setPPostingOutput(IndexOutput* pPPostingOutput)
    {
        pPPostingOutput_ = pPPostingOutput_;
    }
    void setBarrelName(std::string name)
    {
        barrelName_ = name;
    }
    std::string getBarrelName()
    {
        return barrelName_;
    }
    void setDirectory(Directory* pDirectory)
    {
        pDirectory_ = pDirectory;
    }
    Directory* getDirectory()
    {
        return pDirectory_;
    }
    /**
     * flush the output of files ".voc", ".dfp" and ".pop".
     */
    void flush();
protected:
    IndexOutput* pVocOutput_;
    IndexOutput* pDPostingOutput_;
    IndexOutput* pPPostingOutput_;
    bool bDestroy_;
    std::string barrelName_;
    Directory* pDirectory_;
};


}


NS_IZENELIB_IR_END

#endif
