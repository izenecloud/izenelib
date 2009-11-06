/**
* @file        FieldInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FIELDINFO_H
#define FIELDINFO_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Logger.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/index/Posting.h>
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
/// FieldFlag information include isForward and isIndexed
/// FieldFlag = |4bit isIndexed|4Bit isForward|
///----------------------------------------------------------------------//
#define FIELD_INDEXED(flag) (bool)((flag)>>4)
#define FIELD_FORWARD(flag) (bool)((flag)&0x0F)

class FieldInfo
{
public:

    FieldInfo(void)
            :id(-1)
            ,fieldFlag(0)
    {
        reset();
    }
    FieldInfo(fieldid_t fid,const char* name,bool isforward,bool indexed)
            :id(fid)
            ,name(name)
            ,fieldFlag(0)
    {
        reset();
        fieldFlag |= isforward;
        fieldFlag |= indexed<<4;
    }
    FieldInfo(const FieldInfo& src)
    {
        colid = src.colid;
        fieldFlag = src.fieldFlag;
        id = src.id;
        name = src.name;
        distTerms = src.distTerms;
        indexOffset = src.indexOffset;
        vocLength = src.vocLength;
        dfiLength = src.dfiLength;
        ptiLength = src.ptiLength;
    }
    ~FieldInfo(void)
    {
    }
public:
    /**
    * @brief get field id information.    Field id is formed according to the Field sequence in the document schema definition file
    */
    fieldid_t getID()
    {
        return id;
    }
    /**
    * @brief set field id information.    Field id is formed according to the Field sequence in the document schema definition file
    */
    void setID(fieldid_t fid)
    {
        id = fid;
    }
    /**
    * @brief get collection id information.
    */
    collectionid_t getColID()
    {
        return colid;
    }
    /**
    * @brief set collection id information.
    */
    void setColID(collectionid_t cid)
    {
        colid = cid;
    }
    /**
    * @brief get field name
    */
    const char* getName()
    {
        return name.c_str();
    }
    /**
    * @brief set field name
    */
    void setName(const char* name)
    {
        this->name = name;
    }
    /**
    * @brief set indexed flag
    */
    // TODO: "byte" is ambiguous.
    void setFieldFlag(byte flag)
    {
        fieldFlag = flag;
    }
    /**
    * @brief get indexed flag
    */
    byte getFieldFlag()
    {
        return fieldFlag;
    }

    /**
    * @brief get index offset value, indexOffset information indicates the offset of this field in  the vocabulary index file, named *.voc, * refers to the barrel name
    */
    fileoffset_t getIndexOffset()
    {
        return indexOffset;
    }
    /**
    * @brief set index offset value, indexOffset information indicates the offset of this field in  the vocabulary index file, named *.voc, * refers to the barrel name
    */
    void setIndexOffset(fileoffset_t offset)
    {
        indexOffset = offset;
    }
    /**
    * @brief get total distinct term number of this indexed field in a certain barrel
    */
    uint64_t distinctNumTerms()
    {
        return distTerms;
    }
    /**
    * @brief set total distinct term number of this indexed field in a certain barrel
    */
    void setDistinctNumTerms(uint64_t n)
    {
        distTerms = n;
    }
    /**
    * @brief set length
    * @param nVocLen the length of this field that has occupied in the vocabulary *.voc file
    * @param dfiLen the length of this field's document-frequency posting in the *.dfp index file
    * @param ptiLen the length of this field's position posting in the *.pop index file
    */
    void setLength(int64_t nVocLen,int64_t dfiLen,int64_t ptiLen)
    {
        vocLength = nVocLen;
        dfiLength=dfiLen;
        ptiLength=ptiLen;
    }
    /**
    * @brief get length
    */
    void getLength(int64_t* pNVocLen,int64_t* pNDLen,int64_t* pNPLen);
    /**
    * @brief Whether this field is indexed,only for PropertyType of ForwardIndex
    */
    bool isIndexed()
    {
        return FIELD_INDEXED(fieldFlag);
    }

    bool isForward()
    {
        return FIELD_FORWARD(fieldFlag);
    }

    void reset()
    {
        distTerms = 0;
        indexOffset = -1;
        vocLength = dfiLength = ptiLength = 0;
    }
private:
    fieldid_t id;

    collectionid_t colid;

    std::string name;

    byte fieldFlag;

    uint64_t distTerms;

    fileoffset_t indexOffset;

    int64_t vocLength;

    int64_t dfiLength;

    int64_t ptiLength;

    friend class FieldsInfo;
};
//////////////////////////////////////////////////////////////////////////
//
inline void FieldInfo::getLength(int64_t* pNVocLen,int64_t* pNDLen,int64_t* pNPLen)
{
    if (pNVocLen)
        *pNVocLen = vocLength;
    if (pNDLen)
        *pNDLen = dfiLength;
    if (pNPLen)
        *pNPLen = ptiLength;
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
    collectionid_t getColID()
    {
        return colId;
    }

    void setColID(collectionid_t id)
    {
        colId = id;
    }

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

    fieldid_t getFieldID(const char* fname);

    inline const char* getFieldName(fieldid_t fid);
    /**
    * @brief get FieldInfo by fieldid
    */
    inline FieldInfo* getField(fieldid_t fid);
    /**
    * @brief get FieldInfo by field name
    */
    FieldInfo* getField(const char* field);

    void setFieldOffset(fieldid_t fid,fileoffset_t offset);
    /**
    * @brief offset of vocabulary in .voc file of the certain field
    */
    fileoffset_t getFieldOffset(fieldid_t fid);

    void setDistinctNumTerms(fieldid_t fid,uint64_t distterms);
    /**
    * @brief number of all distinct terms that this field has contained
    */
    uint64_t distinctNumTerms(fieldid_t fid);
    /**
    * @brief number of all fields
    */
    int32_t numFields()
    {
        return (int)nNumFieldInfo;
    }
    /**
    * @brief number of all fields to be indexed
    */
    int32_t numIndexFields();

    void startIterator();

    bool hasNext();

    FieldInfo* next();
private:
    collectionid_t colId;

    std::map<std::string,FieldInfo*> fdInfosByName;

    std::map<fieldid_t,FieldInfo*> fdInfosById;

    FieldInfo** ppFieldsInfo;

    int32_t nNumFieldInfo;

    int32_t fdInfosIterator;
};


//////////////////////////////////////////////////////////////////////////
//Inline functions
inline void FieldsInfo::startIterator()
{
    fdInfosIterator = 0;
}
inline bool FieldsInfo::hasNext()
{
    return (fdInfosIterator != nNumFieldInfo);
}
inline FieldInfo* FieldsInfo::next()
{
    return ppFieldsInfo[fdInfosIterator++];
}
inline int32_t FieldsInfo::numIndexFields()
{
    int32_t indFields = 0;
    for (int32_t i = 0;i<nNumFieldInfo;i++)
    {
        if (ppFieldsInfo[i]->isIndexed())
            indFields++;
    }
    return indFields;
}
inline const char* FieldsInfo::getFieldName(fieldid_t fid)
{
    return fdInfosById[fid]->getName();//ppFieldsInfo[fid]->getName();
}
inline FieldInfo* FieldsInfo::getField(fieldid_t fid)
{
    return fdInfosById[fid];//ppFieldsInfo[fid];
}

}

NS_IZENELIB_IR_END

#endif
