#include <ir/index_manager/index/SingleIndexBarrelReader.h>
#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/Indexer.h>

#define MAX_MEM_POOL_SIZE_FOR_ONE_POSTING 10*1024*1024

using namespace izenelib::ir::indexmanager;

SingleIndexBarrelReader::SingleIndexBarrelReader(Indexer* pIndex, BarrelInfo* pBarrel,DiskIndexOpenMode mode)
        : IndexBarrelReader(pIndex)
        , pBarrelInfo_(pBarrel)
        , pMemCache_(NULL)
{
    pCollectionsInfo_ = new CollectionsInfo();
    open(pBarrelInfo_->getName().c_str(),mode);
}

SingleIndexBarrelReader::~SingleIndexBarrelReader(void)
{
    close();
    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
        delete iter->second;
    termReaderMap_.clear();
    delete pCollectionsInfo_;
    if(pMemCache_)
        delete pMemCache_;
}

void SingleIndexBarrelReader::open(const char* name,DiskIndexOpenMode mode)
{
    this->name_ = name;

    Directory* pDirectory = pIndexer->getDirectory();
    string s = name;
    s+= ".fdi";
    IndexInput* pIndexInput = pDirectory->openInput(s.c_str());
    pCollectionsInfo_->read(pIndexInput);
    pIndexInput->close();
    delete pIndexInput;

    for (map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
        delete iter->second;

    termReaderMap_.clear();

    pCollectionsInfo_->startIterator();
    CollectionInfo* pColInfo = NULL;
    FieldsInfo* pFieldsInfo = NULL;
    TermReader* pTermReader = NULL;
    while (pCollectionsInfo_->hasNext())
    {
        pColInfo = pCollectionsInfo_->next();
        pFieldsInfo = pColInfo->getFieldsInfo();
        if (pFieldsInfo->numIndexFields() > 1)
        {
            pTermReader = new MultiFieldTermReader(pDirectory,name,pFieldsInfo,mode);
        }
        else if (pFieldsInfo->numIndexFields() == 1)
        {
            pFieldsInfo->startIterator();
            FieldInfo* pFieldInfo;
            while (pFieldsInfo->hasNext())
            {
                pFieldInfo = pFieldsInfo->next();
                if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
                {
                    pTermReader = new DiskTermReader(mode);
                    pTermReader->open(pDirectory,name,pFieldInfo);
                    break;
                }
            }
        }
        else
        {
            SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the field number is 0.");
        }
        termReaderMap_.insert(pair<collectionid_t, TermReader*>(pColInfo->getId(),pTermReader));
    }

}

TermReader* SingleIndexBarrelReader::termReader(collectionid_t colID, const char* field)
{
    TermReader* pTermReader = termReaderMap_[colID];
    if (pTermReader == NULL)
        return NULL;
    FieldsInfo* pFieldsInfo = pCollectionsInfo_->getCollectionInfo(colID)->getFieldsInfo();
    if (pFieldsInfo->numIndexFields() > 1)
        return ((MultiFieldTermReader*)pTermReader)->termReader(field);
    else
    {
        pFieldsInfo->startIterator();
        FieldInfo* pFieldInfo;
        while (pFieldsInfo->hasNext())
        {
            pFieldInfo = pFieldsInfo->next();
            if (pFieldInfo->isIndexed()&&pFieldInfo->isForward())
            {
                if (!strcasecmp(field,pFieldInfo->getName()))
                    return pTermReader;//->clone();
            }
        }
    }
    return NULL;
}

TermReader* SingleIndexBarrelReader::termReader(collectionid_t colID)
{
    return termReaderMap_[colID];//->clone();
}

void SingleIndexBarrelReader::deleteDocumentPhysically(IndexerDocument* pDoc)
{
    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);
    DocId uniqueID;
    pDoc->getDocId(uniqueID);
    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if(!iter->first.isIndex())
            continue;
        if (iter->first.isForward())
        {
            if(! iter->first.isLAInput())
            {
                boost::shared_ptr<ForwardIndex> forwardIndex = boost::get<boost::shared_ptr<ForwardIndex> >(iter->second);
                delDocField(uniqueID.colId, uniqueID.docId, iter->first.getName().c_str(), forwardIndex);
            }
        }
    }
}

void SingleIndexBarrelReader::delDocField(unsigned int colID, docid_t docId, const char* fieldName, boost::shared_ptr<ForwardIndex>& forwardIndex)
{
    DiskTermReader* pTermReader = reinterpret_cast<DiskTermReader*>(termReader(colID,fieldName));
    if (pTermReader == NULL)
        return;

    if(!pMemCache_)
    {
        pMemCache_ = new MemCache(MAX_MEM_POOL_SIZE_FOR_ONE_POSTING);
    }

    Directory* pDirectory = pIndexer->getDirectory();

    IndexOutput* pVocOutput = pDirectory->createOutput(pBarrelInfo_->getName() + ".voc","r+");
    IndexOutput* pDOutput = pDirectory->createOutput(pBarrelInfo_->getName() + ".dfp","r+");
    IndexOutput* pPOutput = pDirectory->createOutput(pBarrelInfo_->getName() + ".pop","r+");
    OutputDescriptor desc(pVocOutput,pDOutput,pPOutput,true);

    IndexInput* pVocInput = pDirectory->openInput(pBarrelInfo_->getName() + ".voc");
    IndexInput* pDPInput = pDirectory->openInput(pBarrelInfo_->getName() + ".dfp");
    IndexInput* pPPInput = pDirectory->openInput(pBarrelInfo_->getName() + ".pop");

    FieldInfo* pFieldInfo = pCollectionsInfo_->getCollectionInfo(colID)->getFieldsInfo()->getField(fieldName);

    bool ret = false;
    docid_t decompressed_docid;
    freq_t doclength;

    for(ForwardIndex::iterator iter = forwardIndex->begin(); iter != forwardIndex->end(); ++iter)
    {
        termid_t termId = (termid_t)iter->first;

        Term term(pFieldInfo->getName(), termId);
        if (pTermReader->seek(&term))
	{
            TermInfo* pTermInfo = pTermReader->termInfo(&term);
			
            InMemoryPosting* newPosting = new InMemoryPosting(pMemCache_);

            TermPositions* pTermPositions = pTermReader->termPositions();
            while (pTermPositions->next())
            {
                decompressed_docid = pTermPositions->doc();
                if (decompressed_docid == docId)
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
                desc.getPPostingOutput()->seek(postingDesc.poffset - nPPostingLength);///seek to the begin of position posting data
                desc.getDPostingOutput()->seek(pTermInfo->docPointer() - postingDesc.length); ///seek to the begin of posting data
                fileoffset_t poffset = newPosting->write(&desc);///write posting data
                desc.getDPostingOutput()->flush();
                desc.getPPostingOutput()->flush();
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

        IndexOutput* pVocWriter = desc.getVocOutput();
        pVocWriter->seek(voffset - nVocLength);///seek to begin of vocabulary data

        TERM_TABLE* pTermTable = pTermReader->getTermTable();

        fileoffset_t lastPOffset = 0;
        for(TERM_TABLE::iterator termTableIter = pTermTable->begin(); termTableIter!= pTermTable->end(); ++termTableIter)
        //for (int i = 0; i < nTermCount; i++)
        {
	 /*
		pVocWriter->writeInt(pTermTable[i].tid);				///write term id
		pVocWriter->writeInt(pTermTable[i].ti.docFreq());					///write df
		pVocWriter->writeLong(pTermTable[i].ti.docPointer() - lastPOffset); ///write postingpointer
		lastPOffset = pTermTable[i].ti.docPointer();
		*/
            pVocWriter->writeInt(termTableIter->first); 				///write term id
            pVocWriter->writeInt(termTableIter->second.docFreq());					///write df
            pVocWriter->writeLong(termTableIter->second.docPointer());	///write postingpointer
            lastPOffset = termTableIter->second.docPointer();
        }
        pVocWriter->flush();
    }
    delete pVocInput;
    delete pDPInput;
    delete pPPInput;
    pMemCache_->flushMem();
}

void SingleIndexBarrelReader::close()
{
    map<collectionid_t, TermReader*>::iterator iter = termReaderMap_.begin();
    while (iter!=termReaderMap_.end())
    {
        if (iter->second)
            iter->second->close();
        iter++;
    }

}

