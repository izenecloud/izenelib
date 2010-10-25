#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <util/izene_log.h>

#include <util/ThreadModel.h>

using namespace izenelib::ir::indexmanager;

IndexReader::IndexReader(Indexer* pIndex)
        :pIndexer_(pIndex)
        ,pBarrelsInfo_(NULL)
        ,pBarrelReader_(NULL)
        ,pDocFilter_(NULL)
        ,pDocLengthReader_(NULL)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();

    Directory* pDirectory = pIndexer_->getDirectory();
    if(pDirectory->fileExists(DELETED_DOCS))
    {
        pDocFilter_ = new BitVector;
        pDocFilter_->read(pDirectory, DELETED_DOCS);
    }
    ///todo since only one collection is used within indexmanager, so we only get collectionmeta for one collection to build
    ///up the DocLengthReader
    if(pIndexer_->getIndexManagerConfig()->indexStrategy_.indexDocLength_)
        pDocLengthReader_ = new DocLengthReader(
                                pIndexer_->getCollectionsMeta().begin()->second.getDocumentSchema(), 
                                pIndexer_->getDirectory());
}

IndexReader::~IndexReader()
{
    if (pBarrelReader_)
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
    }
    if(pDocFilter_&& pDocFilter_->any())
    {
        if(pIndexer_->getDirectory()->fileExists(DELETED_DOCS))
            pIndexer_->getDirectory()->deleteFile(DELETED_DOCS);
        pDocFilter_->write(pIndexer_->getDirectory(), DELETED_DOCS);
        delete pDocFilter_;
    }
    if(pDocLengthReader_) {delete pDocLengthReader_; pDocLengthReader_ = NULL;}
}

void IndexReader::delDocFilter()
{
    if(pDocFilter_)
    {
        if(pIndexer_->getDirectory()->fileExists(DELETED_DOCS))
            pIndexer_->getDirectory()->deleteFile(DELETED_DOCS);
        pDocFilter_->clear();
    }
}

docid_t IndexReader::maxDoc()
{
    return pBarrelsInfo_->maxDocId();
}

size_t IndexReader::docLength(docid_t docId, fieldid_t fid)
{
    if (pBarrelReader_ == NULL)
        createBarrelReader();
    return pDocLengthReader_->docLength(docId, fid);
}

double IndexReader::getAveragePropertyLength(fieldid_t fid)
{
    if (pBarrelReader_ == NULL)
        createBarrelReader();
    boost::mutex::scoped_lock indexReaderLock(this->mutex_);
    return pDocLengthReader_->averagePropertyLength(fid);
}

void IndexReader::createBarrelReader()
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if (pBarrelReader_)
        return;

    int32_t bc = pBarrelsInfo_->getBarrelCount();
    DVLOG(2) << "=> IndexReader::createBarrelReader(), " << bc << " barrels...";

    if (bc == 1)
    {
        BarrelInfo* pLastBarrel = (*pBarrelsInfo_)[0];

        if(pLastBarrel && pLastBarrel->getDocCount() > 0) ///skip empty barrel
        {
            if (pLastBarrel->getWriter() && pLastBarrel->isSearchable())
                pBarrelReader_ = pLastBarrel->getWriter()->inMemoryReader();
            else
                pBarrelReader_ = new SingleIndexBarrelReader(this,pLastBarrel);
        }
    }
    else if (bc > 1)
    {
        pBarrelReader_ = new MultiIndexBarrelReader(this,pBarrelsInfo_);
    }
    else
        return;

    if(pDocLengthReader_)
        pDocLengthReader_->load(pBarrelsInfo_->maxDocId());

    //pIndexer_->setDirty(false);///clear dirty_ flag

    DVLOG(2) << "<= IndexReader::createBarrelReader()";
}

TermReader* IndexReader::getTermReader(collectionid_t colID)
{
    //boost::try_mutex::scoped_try_lock lock(pIndexer_->mutex_);
    //if(!lock.owns_lock())
        //return NULL;
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->mutex_);
    if (pBarrelReader_ == NULL)
        createBarrelReader();
    if (pBarrelReader_ == NULL)
        return NULL;
    boost::mutex::scoped_lock indexReaderLock(this->mutex_);
	
    TermReader* pTermReader = pBarrelReader_->termReader(colID);
    if (pTermReader)
    {
        pTermReader->setSkipInterval(pIndexer_->getSkipInterval());
        pTermReader->setMaxSkipLevel(pIndexer_->getMaxSkipLevel());
        if(pDocFilter_) pTermReader->setDocFilter(pDocFilter_);
        return pTermReader->clone();
    }
    else
        return NULL;
}

void IndexReader::reopen()
{
    {
    boost::mutex::scoped_lock lock(this->mutex_);
    if(pBarrelReader_)
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
    }
    }
    createBarrelReader();
}

count_t IndexReader::numDocs()
{
    return pBarrelsInfo_->getDocCount();
}

BarrelInfo* IndexReader::findDocumentInBarrels(collectionid_t colID, docid_t docID)
{
    for (int i = pBarrelsInfo_->getBarrelCount() - 1; i >= 0; i--)
    {
        BarrelInfo* pBarrelInfo = (*pBarrelsInfo_)[i];
        if ((pBarrelInfo->baseDocIDMap.find(colID) != pBarrelInfo->baseDocIDMap.end())&&
                (pBarrelInfo->baseDocIDMap[colID] <= docID))
            return pBarrelInfo;
    }
    return NULL;
}

void IndexReader::delDocument(collectionid_t colID,docid_t docId)
{
    BarrelInfo* pBarrelInfo = findDocumentInBarrels(colID, docId);
    if(NULL == pBarrelInfo)
        return;
    pBarrelInfo->deleteDocument(docId);
    if(!pDocFilter_)
    {
        pDocFilter_ = new BitVector(pBarrelsInfo_->getDocCount() + 1);
    }
    pDocFilter_->set(docId);
}

freq_t IndexReader::docFreq(collectionid_t colID, Term* term)
{
    //boost::try_mutex::scoped_try_lock lock(pIndexer_->mutex_);
    //if(!lock.owns_lock())
        //return 0;
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->mutex_);

    if (pBarrelReader_ == NULL)
        createBarrelReader();
    if (pBarrelReader_ == NULL)
        return 0;

    boost::mutex::scoped_lock indexReaderLock(this->mutex_);
	
    TermReader* pTermReader = pBarrelReader_->termReader(colID);
    if (pTermReader)
    {
        return pTermReader->docFreq(term);
    }

    return 0;
}

TermInfo* IndexReader::termInfo(collectionid_t colID,Term* term)
{
    //boost::try_mutex::scoped_try_lock lock(pIndexer_->mutex_);
    //if(!lock.owns_lock())
        //return NULL;
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->mutex_);

    if (pBarrelReader_ == NULL)
        createBarrelReader();
    if (pBarrelReader_ == NULL)
        return NULL;

    boost::mutex::scoped_lock indexReaderLock(this->mutex_);

    TermReader* pTermReader = pBarrelReader_->termReader(colID);
    if (pTermReader)
    {
        return pTermReader->termInfo(term);
    }

    return 0;
}

size_t IndexReader::getDistinctNumTerms(collectionid_t colID, const std::string& property)
{
    //collection has been removed, need to rebuild the barrel reader
    if (pBarrelReader_ == NULL)
        createBarrelReader();
    if (pBarrelReader_ == NULL)
        return 0;

    boost::mutex::scoped_lock indexReaderLock(this->mutex_);

    return pBarrelReader_->getDistinctNumTerms(colID, property);
}

