/**
* @file        FieldInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FIELDINFO_H
#define FIELDINFO_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/index/PostingReader.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/IndexerCollectionMeta.h>


#include <map>
#include <string>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* @brief Field information about the index in a certain barrel. It will be stored in the index file of "*.fdi", * refers to the barrel name.
* @note FieldInfo is composed of several information
*
*/

///----------------------------------------------------------------------//
/// FieldFlag information include isIndexed and isAnalyzed
/// FieldFlag = |4bit isIndexed|4Bit isAnalyzed|
///----------------------------------------------------------------------//
#define FIELD_INDEXED(flag) (bool)((flag)>>4)
#define FIELD_ANALYZED(flag) (bool)((flag)&0x0F)

class FieldInfo
{
public:

    FieldInfo(void)
            :id_(-1)
            ,fieldFlag_(0)
    {
        reset();
    }
    FieldInfo(fieldid_t fid,const char* name,bool analyzed,bool indexed)
            :id_(fid)
            ,name_(name)
            ,fieldFlag_(0)
    {
        reset();
        fieldFlag_ |= analyzed;
        fieldFlag_ |= indexed<<4;
    }
    FieldInfo(const FieldInfo& src)
        :id_(src.id_)
        ,colid_(src.colid_)
        ,name_(src.name_)
        ,fieldFlag_(src.fieldFlag_)
        ,distTerms_(src.distTerms_)
        ,indexOffset_(src.indexOffset_)
        ,vocLength_(src.vocLength_)
        ,dfiLength_(src.dfiLength_)
        ,ptiLength_(src.ptiLength_)
    {}
    ~FieldInfo(){}
public:
    /**
    * @brief get field id_ information.    Field id_ is formed according to the Field sequence in the document schema definition file
    */
    fieldid_t getID() { return id_; }
    /**
    * @brief set field id_ information.    Field id_ is formed according to the Field sequence in the document schema definition file
    */
    void setID(fieldid_t fid) { id_ = fid; }
    /**
    * @brief get collection id_ information.
    */
    collectionid_t getColID() { return colid_; }
    /**
    * @brief set collection id_ information.
    */
    void setColID(collectionid_t cid) { colid_ = cid; }
    /**
    * @brief get field name
    */
    const char* getName() { return name_.c_str(); }
    /**
    * @brief set field name
    */
    void setName(const char* name) { name_ = name; }
    /**
    * @brief set indexed flag
    */
    void setFieldFlag(byte flag) { fieldFlag_ = flag; }
    /**
    * @brief get indexed flag
    */
    byte getFieldFlag() { return fieldFlag_; }

    /**
    * @brief get index offset value, indexOffset_ information indicates the offset of this field in  
    *  the vocabulary index file, named *.voc, * refers to the barrel name
    */
    fileoffset_t getIndexOffset() { return indexOffset_; }
    /**
    * @brief set index offset value, indexOffset_ information indicates the offset of this field in  
    * the vocabulary index file, named *.voc, * refers to the barrel name
    */
    void setIndexOffset(fileoffset_t offset) { indexOffset_ = offset; }
    /**
    * @brief get total distinct term number of this indexed field in a certain barrel
    */
     uint64_t distinctNumTerms() { return distTerms_; }
    /**
    * @brief set total distinct term number of this indexed field in a certain barrel
    */
    void setDistinctNumTerms(uint64_t n) { distTerms_ = n; }
    /**
    * @brief set length
    * @param nVocLen the length of this field that has occupied in the vocabulary *.voc file
    * @param dfiLen the length of this field's document-frequency posting in the *.dfp index file
    * @param ptiLen the length of this field's position posting in the *.pop index file
    */
    void setLength(int64_t nVocLen,int64_t dfiLen,int64_t ptiLen)
    {
        vocLength_ = nVocLen;
        dfiLength_=dfiLen;
        ptiLength_=ptiLen;
    }
    /**
    * @brief get length
    */
    void getLength(int64_t* pNVocLen,int64_t* pNDLen,int64_t* pNPLen);
    /**
    * @brief Whether this field is indexed
    */
    bool isIndexed() { return FIELD_INDEXED(fieldFlag_);}

    bool isAnalyzed() { return FIELD_ANALYZED(fieldFlag_); }

    bool isEmpty() { return indexOffset_ == -1;}

    void reset()
    {
        distTerms_ = 0;
        indexOffset_ = -1;
        vocLength_ = dfiLength_ = ptiLength_ = 0;
    }
private:
    fieldid_t id_;

    collectionid_t colid_;

    std::string name_;

    byte fieldFlag_;

    uint64_t distTerms_;

    fileoffset_t indexOffset_;

    int64_t vocLength_;

    int64_t dfiLength_;

    int64_t ptiLength_;

    friend class FieldsInfo;
};
//////////////////////////////////////////////////////////////////////////
//
inline void FieldInfo::getLength(int64_t* pNVocLen,int64_t* pNDLen,int64_t* pNPLen)
{
    if (pNVocLen)
        *pNVocLen = vocLength_;
    if (pNDLen)
        *pNDLen = dfiLength_;
    if (pNPLen)
        *pNPLen = ptiLength_;
}

/**
* @brief FieldsInfo, Each Field of a Document would have different FieldInfo
* @note FieldsInfo is a manager class of FieldInfo
*/
class FieldsInfo
{
public:
    FieldsInfo();

    FieldsInfo(const FieldsInfo& src);

    virtual ~FieldsInfo();
public:
    collectionid_t getColID() { return colId_; }

    void setColID(collectionid_t id) { colId_ = id; }

    void setSchema(const IndexerCollectionMeta& schema);

    void addField(FieldInfo* pFieldInfo);
    /**
    * @brief read the fieldsinfo from *.fdi file
    */
    void read(IndexInput* pIndexInput);
    /**
    * @brief flush the fieldsinfo into *.fdi file
    */
    void write(IndexOutput* pIndexOutput);

    void clear();

    void reset();

    /**
    * @brief get FieldInfo by field name
    */
    inline FieldInfo* getField(const string& field){ return getField(field.c_str()); }
    /**
    * @brief get FieldInfo by field name
    */
    FieldInfo* getField(const char* field);

    void setFieldOffset(const string& field,fileoffset_t offset);
    /**
    * @brief offset of vocabulary in .voc file of the certain field
    */
    fileoffset_t getFieldOffset(const string& field);
    /**
    * @brief set distinct num terms to a field
    */ 
    void setDistinctNumTerms(const string& field,uint64_t distterms);
    /**
    * @brief number of all distinct terms that this field has contained
    */
    uint64_t distinctNumTerms(const string& field);
    /**
    * @brief number of all fields
    */
    int32_t numFields() { return nNumFieldInfo_; }
    /**
    * @brief number of all fields to be indexed
    */
    int32_t numIndexFields();

    void startIterator();

    bool hasNext();

    FieldInfo* next();
private:
    collectionid_t colId_;

    std::map<std::string,FieldInfo*> fdInfosByName_;

    FieldInfo** ppFieldsInfo_;

    int32_t nNumFieldInfo_;

    int32_t fdInfosIterator_;
};


//////////////////////////////////////////////////////////////////////////
//Inline functions
inline void FieldsInfo::startIterator()
{
    fdInfosIterator_ = 0;
}
inline bool FieldsInfo::hasNext()
{
    return (fdInfosIterator_ != nNumFieldInfo_);
}
inline FieldInfo* FieldsInfo::next()
{
    return ppFieldsInfo_[fdInfosIterator_++];
}
inline int32_t FieldsInfo::numIndexFields()
{
    int32_t indFields = 0;
    for (int32_t i = 0;i<nNumFieldInfo_;i++)
    {
        if (ppFieldsInfo_[i]->isIndexed())
            indFields++;
    }
    return indFields;
}

}

NS_IZENELIB_IR_END

#endif
