#include <ir/index_manager/index/IndexMerger.h>
#include <ir/index_manager/index/FieldMerger.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/IndexWriter.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/utility/ParamParser.h>
#include <ir/index_manager/utility/StringUtils.h>

#include <sstream>
#include <memory>
#include <algorithm>

#define  MAX_BARREL_LEVEL 32


using namespace izenelib::ir::indexmanager;

//////////////////////////////////////////////////////////////////////////
///MergeBarrelEntry
MergeBarrelEntry::MergeBarrelEntry(Directory* pDirectory_,BarrelInfo* pBarrelInfo_)
        :pDirectory(pDirectory_)
        ,pBarrelInfo(pBarrelInfo_)
        ,pCollectionsInfo(NULL)
{
}
MergeBarrelEntry::~MergeBarrelEntry()
{
    if (pCollectionsInfo)
    {
        delete pCollectionsInfo;
        pCollectionsInfo = NULL;
    }

    pBarrelInfo = NULL;
}
void MergeBarrelEntry::load()
{
    IndexBarrelWriter* pWriter = pBarrelInfo->getWriter();
    if (!pCollectionsInfo)
    {
        if (pWriter)
        {
            pCollectionsInfo = new CollectionsInfo(*(pWriter->getCollectionsInfo()));
        }

        else
            pCollectionsInfo = new CollectionsInfo();
    }

    if (!pWriter)
    {
        IndexInput* fdiStream = pDirectory->openInput(pBarrelInfo->getName() + ".fdi");

        pCollectionsInfo->read(fdiStream);///read collection info

        delete fdiStream;
    }
}
//////////////////////////////////////////////////////////////////////////
///IndexMerger
IndexMerger::IndexMerger()
        :pDirectory(NULL)
        ,buffer(NULL)
        ,bufsize(0)
        ,bBorrowedBuffer(false)
        ,pMergeBarrels(NULL)
        ,pDocFilter(NULL)
{
}
IndexMerger::IndexMerger(Directory* pDirectory_)
        :pDirectory(pDirectory_)
        ,buffer(NULL)
        ,bufsize(0)
        ,bBorrowedBuffer(false)
        ,pMergeBarrels(NULL)
	,pDocFilter(NULL)
{
}
IndexMerger::IndexMerger(Directory* pDirectory_,char* buffer_,size_t bufsize_)
        :pDirectory(pDirectory_)
        ,buffer(buffer_)
        ,bufsize(bufsize_)
        ,bBorrowedBuffer(false)
        ,pMergeBarrels(NULL)
	,pDocFilter(NULL)
{
}
IndexMerger::~IndexMerger()
{
    if (pMergeBarrels)
    {
        pMergeBarrels->clear();
        delete pMergeBarrels;
        pMergeBarrels = NULL;
    }
    pDocFilter = 0;
}

void IndexMerger::setBuffer(char* buffer,size_t length)
{
    this->buffer = buffer;
    bufsize = length;
}

void IndexMerger::merge(BarrelsInfo* pBarrels)
{
    if (!pBarrels || ((pBarrels->getBarrelCount() <= 1)&&!pDocFilter))
    {
        pendingUpdate(pBarrels);
        return ;
    }

    pBarrelsInfo = pBarrels;

    MergeBarrel mb(pBarrels->getBarrelCount());
    ///put all index barrel into mb
    pBarrels->startIterator();
    BarrelInfo* pBaInfo;
    while (pBarrels->hasNext())
    {
        pBaInfo = pBarrels->next();

        mb.put(new MergeBarrelEntry(pDirectory,pBaInfo));
    }

    while (mb.size() > 0)
    {
        addBarrel(mb.pop());
    }

    endMerge();
    pendingUpdate(pBarrels); ///update barrel name and base doc id
    pBarrelsInfo = NULL;

    if (bBorrowedBuffer)
    {
        ///the buffer is borrowed from indexer, give it back to indexer
        setBuffer(NULL,0);
        bBorrowedBuffer = false;
    }
    pBarrels->write(pDirectory);
}

void IndexMerger::addToMerge(BarrelsInfo* pBarrelsInfo,BarrelInfo* pBarrelInfo)
{
    if (!pMergeBarrels)
    {
        pMergeBarrels = new vector<MergeBarrelEntry*>();
    }

    MergeBarrelEntry* pEntry = NULL;
    this->pBarrelsInfo = pBarrelsInfo;
    pEntry = new MergeBarrelEntry(pDirectory,pBarrelInfo);
    pMergeBarrels->push_back(pEntry);
    addBarrel(pEntry);

    pendingUpdate(pBarrelsInfo); ///update barrel name and base doc id
    this->pBarrelsInfo = NULL;

    if (bBorrowedBuffer)
    {
        ///the buffer is borrowed from indexer, give it back to indexer
        setBuffer(NULL,0);
        bBorrowedBuffer = false;
    }
    pBarrelsInfo->write(pDirectory);
}

void IndexMerger::pendingUpdate(BarrelsInfo* pBarrelsInfo)
{
    ///TODO:LOCK
    ///sort barrels
    pBarrelsInfo->sort(pDirectory);
    BarrelInfo* pBaInfo;
    pBarrelsInfo->startIterator();
    while (pBarrelsInfo->hasNext())
    {
        pBaInfo = pBarrelsInfo->next();
        pBaInfo->setWriter(NULL);///clear writer
    }
    ///TODO:UNLOCK
}
void IndexMerger::continueDocIDs(BarrelsInfo* pBarrelsInfo)
{
    docid_t baseDocID = 0;
    BarrelInfo* pBaInfo;
    pBarrelsInfo->startIterator();
    while (pBarrelsInfo->hasNext())
    {
        pBaInfo = pBarrelsInfo->next();

        baseDocID += pBaInfo->getDocCount();
    }
}

void IndexMerger::mergeBarrel(MergeBarrel* pBarrel)
{
    cout<<"merge 1"<<endl;

    //////////////////////////////////////////////////////////////////////////
    SF1V5_LOG(level::info) << "Begin merge: " << SF1V5_END;
    BarrelInfo* pBaInfo;
    for (int32_t i = 0;i < (int32_t)pBarrel->size();i++)
    {
        pBaInfo = pBarrel->getAt(i)->pBarrelInfo;
        SF1V5_LOG(level::info) << "\t" << i << ":" << pBaInfo->getDocCount() << SF1V5_END;
    }
    //////////////////////////////////////////////////////////////////////////
    pBarrel->load();
    string newBarrelName = pBarrel->getIdentifier();
    BarrelInfo* pNewBarrelInfo = new BarrelInfo(newBarrelName,0);

    string name = newBarrelName + ".voc";///the file name of new index barrel
    IndexOutput* pVocStream = pDirectory->createOutput(name);
    name = newBarrelName + ".dfp";
    IndexOutput* pDStream = pDirectory->createOutput(name);
    name = newBarrelName + ".pop";
    IndexOutput* pPStream = pDirectory->createOutput(name);

    OutputDescriptor* pOutputDesc = new OutputDescriptor(pVocStream,pDStream,pPStream,true);

    int32_t nEntry;
    MergeBarrelEntry* pEntry = NULL;
    count_t nNumDocs = 0;
    int32_t nEntryCount = (int32_t)pBarrel->size();
    ///update min doc id of index barrels,let doc id continuous
    map<collectionid_t,docid_t> newBaseDocIDMap;
    for (nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);

        nNumDocs += pEntry->pBarrelInfo->getDocCount();

        for (map<collectionid_t,docid_t>::iterator iter = pEntry->pBarrelInfo->baseDocIDMap.begin();
                iter != pEntry->pBarrelInfo->baseDocIDMap.end(); ++iter)
        {
            if (newBaseDocIDMap.find(iter->first) == newBaseDocIDMap.end())
            {
                newBaseDocIDMap.insert(make_pair(iter->first,iter->second));
            }
            else
            {
                if (newBaseDocIDMap[iter->first] > iter->second)
                {
                    newBaseDocIDMap[iter->first] = iter->second;
                }
            }
        }
    }
    pNewBarrelInfo->setDocCount(nNumDocs);
    pNewBarrelInfo->setBaseDocID(newBaseDocIDMap);

    FieldsInfo* pFieldsInfo = NULL;
    CollectionsInfo collectionsInfo;
    CollectionInfo* pColInfo = NULL;
    FieldInfo* pFieldInfo = NULL;
    FieldMerger* pFieldMerger = NULL;
    fieldid_t fieldid = 0;
    bool bFinish = false;
    bool bHasPPosting = false;

    fileoffset_t vocOff1,vocOff2,dfiOff1,dfiOff2,ptiOff1 = 0,ptiOff2 = 0;
    fileoffset_t voffset = 0;

    ///collect all colIDs in barrells
    vector<collectionid_t> colIDSet;
    CollectionsInfo* pColsInfo = NULL;

    for (nEntry = 0; nEntry < nEntryCount; nEntry++)
    {
        pColsInfo = pBarrel->getAt(nEntry)->pCollectionsInfo;
        pColsInfo->startIterator();
        while (pColsInfo->hasNext())
            colIDSet.push_back(pColsInfo->next()->getId());
    }
    sort(colIDSet.begin(), colIDSet.end());
    vector<collectionid_t>::iterator it = unique(colIDSet.begin(),colIDSet.end());
    colIDSet.resize(it - colIDSet.begin());

    for (vector<collectionid_t>::const_iterator p = colIDSet.begin(); p != colIDSet.end(); ++p)
    {
        cout<<"merge  colid= "<<*p<<endl;
        bFinish = false;
        fieldid = 0;
        pFieldsInfo = NULL;

        while (!bFinish)
        {
            for (nEntry = 0;nEntry < nEntryCount;nEntry++)
            {
                pEntry = pBarrel->getAt(nEntry);

                pColInfo = pEntry->pCollectionsInfo->getCollectionInfo(*p);

                if (NULL == pColInfo)
                {
                    bFinish = true;
                    break;
                }

                if (pColInfo->getFieldsInfo()->numFields() > fieldid)
                {
                    pFieldInfo = pColInfo->getFieldsInfo()->getField(fieldid);///get field information
                    if (pFieldInfo)
                    {
                        if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())///it's a index field
                        {
                            if (pFieldMerger == NULL)
                            {
                                pFieldMerger = new FieldMerger();
                                pFieldMerger->setDirectory(pDirectory);
                                if(pDocFilter)
                                    pFieldMerger->setDocFilter(pDocFilter);
                            }
                            pFieldInfo->setColID(*p);

                            pFieldMerger->addField(pEntry->pBarrelInfo,pFieldInfo);///add to field merger
                        }
                    }
                }
            } // for

            if (pFieldInfo)
            {
                if (pFieldMerger && pFieldMerger->numFields() > 0)
                {
                    vocOff1 = pVocStream->getFilePointer();
                    dfiOff1 = pDStream->getFilePointer();
                    if (pOutputDesc->getPPostingOutput())
                        ptiOff1 = pOutputDesc->getPPostingOutput()->getFilePointer();

                    pFieldMerger->setBuffer(buffer,bufsize);		///set buffer for field merge
                    voffset = pFieldMerger->merge(pOutputDesc);
                    pFieldInfo->setIndexOffset(voffset);///set offset of this field's index data

                    vocOff2 = pVocStream->getFilePointer();
                    dfiOff2 = pDStream->getFilePointer();
                    ptiOff2 = pOutputDesc->getPPostingOutput()->getFilePointer();
                    pFieldInfo->setDistinctNumTerms(pFieldMerger->numMergedTerms());

                    pFieldInfo->setLength(vocOff2-vocOff1,dfiOff2-dfiOff1,ptiOff2-ptiOff1);
                    if ( (bHasPPosting == false) && ((ptiOff2 - ptiOff1) > 0))
                        bHasPPosting = true;

                    delete pFieldMerger;
                    pFieldMerger = NULL;
                }
                if (pFieldsInfo == NULL)
                    pFieldsInfo = new FieldsInfo();
                pFieldsInfo->addField(pFieldInfo);

                fieldid++;
                pFieldInfo = NULL;
            }
            else
            {
                bFinish = true;
            }
        } // while
        CollectionInfo* pCollectionInfo = new CollectionInfo(*p, pFieldsInfo);
        pCollectionInfo->setOwn(true);
        collectionsInfo.addCollection(pCollectionInfo);
    } // for


    //deleted all merged barrels
    ///TODO:LOCK
    for (nEntry = 0;nEntry < nEntryCount;nEntry++)
    {
        pEntry = pBarrel->getAt(nEntry);
        IndexBarrelWriter* pWriter = pEntry->pBarrelInfo->getWriter();
        if (pWriter)///clear in-memory index
        {
            pWriter->resetCache(true);
            ///borrow buffer from indexer
            setBuffer((char*)pWriter->getMemCache()->getBegin(),pWriter->getMemCache()->getSize());
            bBorrowedBuffer = true;
        }
        pBarrelsInfo->removeBarrel(pDirectory,pEntry->pBarrelInfo->getName());///delete merged barrels
    }
    pBarrelsInfo->addBarrel(pNewBarrelInfo,false);
    continueDocIDs(pBarrelsInfo);///let doc ids in a continuous form
    ///TODO:UNLOCK
    if (pMergeBarrels)
    {
        removeMergedBarrels(pBarrel);
    }
    pBarrel->clear();

    name = newBarrelName + ".fdi";
    IndexOutput* fieldsStream = pDirectory->createOutput(name);
    //fieldsInfo.write(fieldsStream);//field information
    collectionsInfo.write(fieldsStream);
    fieldsStream->flush();
    delete fieldsStream;

    if (bHasPPosting == false)
    {
        name = newBarrelName + ".pop";
        pDirectory->deleteFile(name);
    }

    delete pOutputDesc;
    pOutputDesc = NULL;

    //////////////////////////////////////////////////////////////////////////
    SF1V5_LOG(level::info) << "End merge: " << pNewBarrelInfo->getDocCount() << SF1V5_END;

    MergeBarrelEntry* pNewEntry = new MergeBarrelEntry(pDirectory,pNewBarrelInfo);
    addBarrel(pNewEntry);
}

void IndexMerger::removeMergedBarrels(MergeBarrel * pBarrel)
{
    if (!pMergeBarrels)
        return;
    vector<MergeBarrelEntry*>::iterator iter = pMergeBarrels->begin();
    size_t nEntry = 0;
    bool bRemoved = false;
    uint32_t nRemoved = 0;
    while (iter != pMergeBarrels->end())
    {
        bRemoved = false;
        for (nEntry = 0;nEntry < pBarrel->size();nEntry++)
        {
            if ((*iter) == pBarrel->getAt(nEntry))
            {
                iter = pMergeBarrels->erase(iter);
                bRemoved = true;
                nRemoved++;
            }
        }
        if (nRemoved == pBarrel->size())
            break;
        if (!bRemoved)
            iter++;
    }
}

void IndexMerger::transferToDisk(const char* pszBarrelName)
{
    if (!pMergeBarrels)
        return;

    vector<MergeBarrelEntry*>::iterator iter = pMergeBarrels->begin();
    MergeBarrelEntry* pEntry = NULL;

    while (iter != pMergeBarrels->end())
    {
        if (!strcmp((*iter)->pBarrelInfo->getName().c_str(),pszBarrelName))
        {
            pEntry = (*iter);
            pEntry->load();
            break;
        }
        iter++;
    }
}

