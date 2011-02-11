#include <ir/index_manager/index/FieldInfo.h>

#include <util/izene_log.h>

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

FieldsInfo::FieldsInfo()
        :ppFieldsInfo_(NULL)
        ,nNumFieldInfo_(0)
        ,fdInfosIterator_(0)
{
}
FieldsInfo::FieldsInfo(const FieldsInfo& src)
        :fdInfosIterator_(0)
{
    colId_ = src.colId_;
    nNumFieldInfo_ = src.nNumFieldInfo_;
    ppFieldsInfo_ = new FieldInfo*[nNumFieldInfo_];
    for (int32_t i = 0;i<nNumFieldInfo_;i++)
    {
        ppFieldsInfo_[i] = new FieldInfo(*(src.ppFieldsInfo_[i]));
        fdInfosByName_.insert(make_pair(ppFieldsInfo_[i]->name_.c_str(),ppFieldsInfo_[i]));
    }
}

FieldsInfo::~FieldsInfo()
{
    clear();
}

void FieldsInfo::setSchema(const IndexerCollectionMeta& collectionMeta)
{
    clear();

    nNumFieldInfo_ = 0;
    std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> schema = collectionMeta.getDocumentSchema();

    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator it = schema.begin(); it != schema.end(); it++ )
	if (it->getPropertyId() != BAD_PROPERTY_ID)
           nNumFieldInfo_++;

    ppFieldsInfo_ = new FieldInfo*[nNumFieldInfo_];
    memset(ppFieldsInfo_,0,nNumFieldInfo_*sizeof(FieldInfo*));

    int32_t n = 0;
    for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::const_iterator it = schema.begin(); it != schema.end(); it++ )
    {
	if (it->getPropertyId() != BAD_PROPERTY_ID)
	{
	    ppFieldsInfo_[n] = new FieldInfo(it->getPropertyId(),
                                                           it->getName().c_str(),
                                                           it->isAnalyzed(),
                                                           it->isIndex());
           ppFieldsInfo_[n]->setColID(colId_);
           fdInfosByName_.insert(make_pair(ppFieldsInfo_[n]->getName(),ppFieldsInfo_[n]));
           n++;
	}
    }
}

void FieldsInfo::addField(FieldInfo* pFieldInfo)
{
    FieldInfo* pInfo = getField(pFieldInfo->getName());
    if (!pInfo)
    {
        pInfo = new FieldInfo(*pFieldInfo);
        FieldInfo** ppFieldInfos = new FieldInfo*[nNumFieldInfo_+1];
        for (int32_t i = 0;i<nNumFieldInfo_;i++)
        {
            ppFieldInfos[i] = ppFieldsInfo_[i];
        }
        ppFieldInfos[nNumFieldInfo_] = pInfo;

        delete[] ppFieldsInfo_;
        ppFieldsInfo_ = ppFieldInfos;
        nNumFieldInfo_++;

        fdInfosByName_.insert(pair<string,FieldInfo*>(pInfo->getName(),pInfo));
    }
}

void FieldsInfo::read(IndexInput* pIndexInput)
{
    try
    {
        clear();
        int32_t count = pIndexInput->readInt(); ///<FieldsCount(Int32)>

        if (count <= 0)
        {
            DVLOG(3) << "FieldsInfo::read():field count <=0.";
            return ;
        }
        nNumFieldInfo_ = count;
        ppFieldsInfo_ = new FieldInfo*[count];
        memset(ppFieldsInfo_,0,count*sizeof(FieldInfo*));

        string str;
        FieldInfo* pInfo = NULL;
        for (int32_t i = 0;i<count;i++)
        {
            pInfo = new FieldInfo();
            pInfo->setColID(colId_);
            pIndexInput->readString(str);	///<FieldName(String)>
            pInfo->setName(str.c_str());
            pInfo->setID(pIndexInput->readInt());
            pInfo->setFieldFlag(pIndexInput->readByte()); //IsIndexed(Bool) and IsForward(Bool)
            if (pInfo->isIndexed()&&pInfo->isAnalyzed())
            {
                pInfo->setDistinctNumTerms(pIndexInput->readLong());
                pInfo->setIndexOffset(pIndexInput->readLong());
                pInfo->vocLength_ = pIndexInput->readLong();
                pInfo->dfiLength_ = pIndexInput->readLong();
                pInfo->ptiLength_ = pIndexInput->readLong();

                VLOG(3) << "field name: " << pInfo->getName()
                        << ", distinctNumTerms: " << pInfo->distinctNumTerms()
                        << ", indexOffset: " << pInfo->getIndexOffset()
                        << ", vocLength_: " << pInfo->vocLength_
                        << ", dfiLength_: " << pInfo->dfiLength_
                        << ", ptiLength_: " << pInfo->ptiLength_;
            }

            ppFieldsInfo_[i] = pInfo;
            fdInfosByName_.insert(pair<string,FieldInfo*>(pInfo->getName(),pInfo));
        }

    }
    catch (const IndexManagerException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (const bad_alloc& )
    {
        SF1V5_THROW(ERROR_OUTOFMEM,"FieldsInfo.read():alloc memory failed.");
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNKNOWN,"FieldsInfo::read failed.");
    }
}

void FieldsInfo::write(IndexOutput* pIndexOutput)
{
    try
    {
        pIndexOutput->writeInt(nNumFieldInfo_);///<FieldsCount(Int32)>
        FieldInfo* pInfo;
        for (int32_t i = 0;i<nNumFieldInfo_;i++)
        {
            pInfo = ppFieldsInfo_[i];

            pIndexOutput->writeString(pInfo->getName());	///<FieldName(String)>
            pIndexOutput->writeInt(pInfo->getID());	///<Field id(int)>            
            pIndexOutput->writeByte(pInfo->getFieldFlag());		///<IsIndexed(Bool) and IsForward(Bool)>(Byte)
            if (pInfo->isIndexed()&&pInfo->isAnalyzed())
            {
                pIndexOutput->writeLong(pInfo->distinctNumTerms());
                pIndexOutput->writeLong(pInfo->getIndexOffset());
                pIndexOutput->writeLong(pInfo->vocLength_);
                pIndexOutput->writeLong(pInfo->dfiLength_);
                pIndexOutput->writeLong(pInfo->ptiLength_);
            }

        }
    }
    catch (const IndexManagerException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (const bad_alloc& )
    {
        SF1V5_THROW(ERROR_OUTOFMEM,"FieldsInfo.write():alloc memory failed.");
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNKNOWN,"FieldsInfo.write() failed.");
    }
}
void FieldsInfo::clear()
{
    for (int32_t i = 0;i<nNumFieldInfo_;i++)
    {
        if (ppFieldsInfo_[i])
            delete ppFieldsInfo_[i];
        ppFieldsInfo_[i] = NULL;
    }
    if (ppFieldsInfo_)
    {
        delete[] ppFieldsInfo_;
        ppFieldsInfo_ = NULL;
    }
    nNumFieldInfo_ = 0;

    fdInfosByName_.clear();
}

void FieldsInfo::reset()
{
    for (int32_t i = 0;i<nNumFieldInfo_;i++)
    {
        ppFieldsInfo_[i]->reset();
    }
}

void FieldsInfo::setFieldOffset(const string& field, fileoffset_t offset)
{
    fdInfosByName_[field]->setIndexOffset(offset);
}
fileoffset_t FieldsInfo::getFieldOffset(const string& field)
{
    return fdInfosByName_[field]->getIndexOffset();
}

void FieldsInfo::setDistinctNumTerms(const string& field,uint64_t distterms)
{
    fdInfosByName_[field]->setDistinctNumTerms(distterms);
}

uint64_t FieldsInfo::distinctNumTerms(const string& field)
{
    return fdInfosByName_[field]->distinctNumTerms();
}

FieldInfo* FieldsInfo::getField(const char* field)
{
    string tmp(field);
    map<string,FieldInfo*>::iterator iter = fdInfosByName_.find(tmp);
    if (iter != fdInfosByName_.end())
    {
        return iter->second;
    }
    return NULL;
}

