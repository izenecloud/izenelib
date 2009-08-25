#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/CollectionIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>

#include <ir/index_manager/utility/StringUtils.h>

#include <ir/index_manager/store/IndexOutput.h>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <deque>

using namespace std;
using namespace wiselib;

using namespace izenelib::ir::indexmanager;

CollectionIndexer::CollectionIndexer(collectionid_t id,MemCache* pCache,Indexer* pIndexer)
        :colID_(id)
        ,pMemCache_(pCache)
        ,pIndexer_(pIndexer)
        ,pForwardIndexWriter_(NULL)
{
    pFieldsInfo_ = new FieldsInfo();
}

CollectionIndexer::~CollectionIndexer()
{
    delete pFieldsInfo_;
    pMemCache_ = NULL;
    pIndexer_ = NULL;
    pForwardIndexWriter_ = NULL;
}

void CollectionIndexer::setSchema(const IndexerCollectionMeta& schema)
{
    pFieldsInfo_->setSchema(schema);
}

void CollectionIndexer::setFieldIndexers()
{
    pFieldsInfo_->startIterator();
    FieldInfo* pFieldInfo = NULL;
    while (pFieldsInfo_->hasNext())
    {
        pFieldInfo = pFieldsInfo_->next();
        if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
        {
            FieldIndexer* pFieldIndexer = new FieldIndexer(pMemCache_);
            pFieldIndexer->setField(pFieldInfo->getName());
            fieldIndexerMap_.insert(make_pair(pFieldInfo->getName(),pFieldIndexer));
        }
    }
}

void CollectionIndexer::addDocument(IndexerDocument* pDoc)
{
    DocId uniqueID;
    pDoc->getDocId(uniqueID);

    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);
    pForwardIndexWriter_->addDocument(uniqueID.docId);

    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (! iter->first.isForward())
        {
            pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), boost::get<PropertyType>(iter->second), uniqueID.docId);
        }
        else
        {
            map<string, boost::shared_ptr<FieldIndexer> >::iterator it = fieldIndexerMap_.find(iter->first.getName());
            if (it == fieldIndexerMap_.end())
                // This field is not indexed.
                continue;
            if(! iter->first.isLAInput())
            {
                boost::shared_ptr<ForwardIndex> forwardIndex = boost::get<boost::shared_ptr<ForwardIndex> >(iter->second);
                it->second->addField(uniqueID.docId, forwardIndex);
                pForwardIndexWriter_->addProperty(iter->first.getPropertyId(), forwardIndex);
            }
            else
            {
                boost::shared_ptr<LAInput> laInput = boost::get<boost::shared_ptr<LAInput> >(iter->second);
                it->second->addField(uniqueID.docId, laInput);
                pForwardIndexWriter_->addProperty(iter->first.getPropertyId(), laInput);
            }
            continue;
        }

    }
}

bool CollectionIndexer::deleteDocument(IndexerDocument* pDoc,string barrelName, Directory* pDirectory, OutputDescriptor* desc, bool inMemory)
{
    CollectionsInfo* pCollectionsInfo_of_Barrel = NULL;
    FieldsInfo* pFieldsInfo_of_Barrel = NULL;

    DocId uniqueID;
    pDoc->getDocId(uniqueID);

    bool ret = false;

    if (!inMemory)
    {
        pCollectionsInfo_of_Barrel = new CollectionsInfo();
        string s = barrelName + ".fdi";
        IndexInput* pFdiInput = pDirectory->openInput(s.c_str());
        pCollectionsInfo_of_Barrel->read(pFdiInput);
        pFdiInput->close();
        delete pFdiInput;
        pFieldsInfo_of_Barrel = pCollectionsInfo_of_Barrel->getCollectionInfo(uniqueID.colId)->getFieldsInfo();
    }


    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);

    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (iter->first.isForward())
        {
            boost::shared_ptr<LAInput> laInput = boost::get<boost::shared_ptr<LAInput> >(iter->second);

            if (inMemory)
            {
                map<string, boost::shared_ptr<FieldIndexer> >::iterator it = fieldIndexerMap_.find(iter->first.getName());
                it->second->removeField(uniqueID.docId, laInput);
            }
            else
                ret |=removeDocumentInField(uniqueID.docId, pFieldsInfo_of_Barrel->getField(iter->first.getName().c_str()), laInput, barrelName, pDirectory,desc);
        }
    }

    if (pCollectionsInfo_of_Barrel)
        delete pCollectionsInfo_of_Barrel;
    return ret;
}

bool CollectionIndexer::removeDocumentInField(docid_t docid, FieldInfo* pFieldInfo, boost::shared_ptr<LAInput> laInput, string barrelName, Directory* pDirectory, OutputDescriptor* desc)
{
    bool ret = false;

    docid_t decompressed_docid;

    freq_t doclength;

    DiskTermReader* pTermReader = new DiskTermReader();

    ///open on-disk index barrel
    pTermReader->open(pDirectory,barrelName.c_str(),pFieldInfo);

    string bn = barrelName;

    IndexInput* pVocInput = pDirectory->openInput(bn + ".voc");
    IndexInput* pDPInput = pDirectory->openInput(bn + ".dfp");
    IndexInput* pPPInput = pDirectory->openInput(bn + ".pop");

    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        termid_t termId = (termid_t)iter->termId_;

        Term term(pFieldInfo->getName(), termId);
        if (pTermReader->seek(&term))
        {
            TermInfo* pTermInfo = pTermReader->termInfo(&term);
            InMemoryPosting* newPosting = new InMemoryPosting(pMemCache_);

            TermPositions* pTermPositions = pTermReader->termPositions();
            while (pTermPositions->next())
            {
                decompressed_docid = pTermPositions->doc();
                if (decompressed_docid == docid)
                {
                    ret = true;
                    continue;
                }
                doclength = pTermPositions->docLength();
                loc_t pos = pTermPositions->nextPosition();
                loc_t subpos = pTermPositions->nextPosition();
                while (pos != BAD_POSITION)
                {
                    newPosting->addLocation(decompressed_docid, doclength, pos, subpos);
                    pos = pTermPositions->nextPosition();
                    subpos = pTermPositions->nextPosition();
                }
                newPosting->updateDF(decompressed_docid);
            }

            if (ret)
            {
                //only required if this document lied in this barrel
                ///seek to the offset of the old posting, because after deleting a document, the new posting must be shorter than the original
                ///therefore we choose to override the posting to save space.
                pDPInput->seekInternal(pTermInfo->docPointer());///not seek(), because seek() may trigger a large data read event.
                ///read descriptor of posting list <PostingDescriptor>
                uint8_t buf[32];
                uint8_t* u = buf;
                pDPInput->readInternal((char*)buf,32,false);
                PostingDescriptor postingDesc;
                postingDesc.length = CompressedPostingList::decodePosting64(u); ///<PostingLength(VInt64)>
                postingDesc.df = CompressedPostingList::decodePosting32(u); 	///<DF(VInt32)>
                postingDesc.tdf = CompressedPostingList::decodePosting32(u);	///<TDF(VInt32)>
                postingDesc.ctf = CompressedPostingList::decodePosting64(u);		///<CTF(VInt64)>
                postingDesc.poffset = CompressedPostingList::decodePosting64(u);	///PositionPointer(VInt64)
                pPPInput->seekInternal(postingDesc.poffset);///not seek(), because seek() may trigger a large data read event.
                pPPInput->readInternal((char*)buf,8,false);
                u = buf;
                int64_t nPPostingLength = CompressedPostingList::decodePosting64(u); ///<ChunkLength(VInt64)>
                desc->getPPostingOutput()->seek(postingDesc.poffset - nPPostingLength);///seek to the begin of position posting data
                desc->getDPostingOutput()->seek(pTermInfo->docPointer() - postingDesc.length); ///seek to the begin of posting data
                fileoffset_t poffset = newPosting->write(desc);///write posting data
                desc->getDPostingOutput()->flush();
                desc->getPPostingOutput()->flush();
                pTermReader->updateTermInfo(&term, newPosting->docFreq(), poffset);
            }
            //newPosting->reset();
            delete newPosting;
            delete pTermPositions;
        }
    }

    if (ret)
    {
        //only required if this document lied in this barrel
        pVocInput->seek(pFieldInfo->getIndexOffset());
        fileoffset_t voffset = pVocInput->getFilePointer();

        int64_t nVocLength = pVocInput->readLong();
        pVocInput->readLong(); ///get total term count

        IndexOutput* pVocWriter = desc->getVocOutput();
        pVocWriter->seek(voffset - nVocLength);///seek to begin of vocabulary data

        TERM_TABLE* pTermTable = pTermReader->getTermTable();

        fileoffset_t lastPOffset = 0;
        for(TERM_TABLE::iterator termTableIter = pTermTable->begin(); termTableIter!= pTermTable->end(); ++termTableIter)
        //for (int i = 0; i < nTermCount; i++)
        {
         /*
            pVocWriter->writeInt(pTermTable[i].tid); 				///write term id
            pVocWriter->writeInt(pTermTable[i].ti.docFreq()); 					///write df
            pVocWriter->writeLong(pTermTable[i].ti.docPointer() - lastPOffset);	///write postingpointer
            lastPOffset = pTermTable[i].ti.docPointer();
            */
            pVocWriter->writeInt(termTableIter->first); 				///write term id
            pVocWriter->writeInt(termTableIter->second.docFreq()); 					///write df
            pVocWriter->writeLong(termTableIter->second.docPointer());	///write postingpointer
            lastPOffset = termTableIter->second.docPointer();
        }
        pVocWriter->flush();
    }
    delete pTermReader;
    delete pVocInput;
    delete pDPInput;
    delete pPPInput;

    return ret;
}

void CollectionIndexer::write(OutputDescriptor* desc)
{
    IndexOutput* pVocOutput = desc->getVocOutput();
    IndexOutput* pDOutput = desc->getDPostingOutput();
    IndexOutput* pPOutput = desc->getPPostingOutput();

    fileoffset_t vocOff1,vocOff2,dfiOff1,dfiOff2,ptiOff1,ptiOff2;
    fileoffset_t vocOffset;

    FieldIndexer* pFieldIndexer;
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
    {
        pFieldIndexer = iter->second.get();
        if (pFieldIndexer == NULL)
            continue;
        vocOff1 = pVocOutput->getFilePointer();
        dfiOff1 = pDOutput->getFilePointer();
        ptiOff1 = pPOutput->getFilePointer();

        fieldid_t fid = pIndexer_->getPropertyIDByName(colID_, iter->first);

        pFieldsInfo_->setDistinctNumTerms(fid,pFieldIndexer->distinctNumTerms());///set distinct term numbers

        vocOffset = pFieldIndexer->write(desc);///write field index data

        pFieldsInfo_->setFieldOffset(fid,vocOffset);///set offset of vocabulary descriptor

        vocOff2 = pVocOutput->getFilePointer();
        dfiOff2 = pDOutput->getFilePointer();
        ptiOff2 = pPOutput->getFilePointer();

        pFieldsInfo_->getField(fid)->setLength(vocOff2-vocOff1,dfiOff2-dfiOff1,ptiOff2-ptiOff1);
    }
}


void CollectionIndexer::reset()
{
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
    {
        iter->second->reset();
    }
}

FieldIndexer* CollectionIndexer::getFieldIndexer(const char* field)
{
    string fieldstr(field);
    FieldIndexer* fieldIndexer = fieldIndexerMap_[fieldstr].get();
    return fieldIndexer;
}

