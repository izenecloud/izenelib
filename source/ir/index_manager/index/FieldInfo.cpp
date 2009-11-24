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
        fdInfosById_.insert(make_pair(ppFieldsInfo_[i]->getID(),ppFieldsInfo_[i]));		
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
                                                           it->isForward(),
                                                           it->isIndex());
           ppFieldsInfo_[n]->setColID(colId_);
           fdInfosByName_.insert(make_pair(ppFieldsInfo_[n]->getName(),ppFieldsInfo_[n]));
           fdInfosById_.insert(make_pair(it->getPropertyId(),ppFieldsInfo_[n]));
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
        fdInfosById_.insert(make_pair(pInfo->getID(),pInfo));
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
            DLOG(INFO) << "FieldsInfo::read():field count <=0." << endl;
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
            if (pInfo->isIndexed()&&pInfo->isForward())
            {
                pInfo->setDistinctNumTerms(pIndexInput->readLong());
                pInfo->setIndexOffset(pIndexInput->readLong());
                pInfo->vocLength_ = pIndexInput->readLong();
                pInfo->dfiLength_ = pIndexInput->readLong();
                pInfo->ptiLength_ = pIndexInput->readLong();

                DLOG(INFO)<<"FieldInfo:"<<"indexoffset "<<pInfo->getIndexOffset()<<" distinctnumterms "<<pInfo->distinctNumTerms()<<" voclength "<<pInfo->vocLength_<<" dfilength "<<pInfo->dfiLength_<<" ptilength "<<pInfo->ptiLength_<<endl;
            }

            ppFieldsInfo_[i] = pInfo;
            fdInfosByName_.insert(pair<string,FieldInfo*>(pInfo->getName(),pInfo));
            fdInfosById_.insert(make_pair(pInfo->getID(),pInfo));
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
            if (pInfo->isIndexed()&&pInfo->isForward())
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
    fdInfosById_.clear();
}

void FieldsInfo::reset()
{
    for (int32_t i = 0;i<nNumFieldInfo_;i++)
    {
        ppFieldsInfo_[i]->reset();
    }
}

void FieldsInfo::setFieldOffset(fieldid_t fid,fileoffset_t offset)
{
    //ppFieldsInfo_[fid]->setIndexOffset(offset);
    fdInfosById_[fid]->setIndexOffset(offset);
}
fileoffset_t FieldsInfo::getFieldOffset(fieldid_t fid)
{
    return fdInfosById_[fid]->getIndexOffset();//ppFieldsInfo_[fid]->getIndexOffset();
}

void FieldsInfo::setDistinctNumTerms(fieldid_t fid,uint64_t distterms)
{
    fdInfosById_[fid]->setDistinctNumTerms(distterms);//ppFieldsInfo_[fid]->setDistinctNumTerms(distterms);
}

uint64_t FieldsInfo::distinctNumTerms(fieldid_t fid)
{
    return fdInfosById_[fid]->distinctNumTerms();//ppFieldsInfo_[fid]->distinctNumTerms();
}

fieldid_t FieldsInfo::getFieldID(const char* fname)
{
    map<string,FieldInfo*>::iterator iter = fdInfosByName_.find(fname);
    if (iter != fdInfosByName_.end())
    {
        return iter->second->getID();
    }
    return -1;
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

