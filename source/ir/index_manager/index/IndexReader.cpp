#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>


using namespace izenelib::ir::indexmanager;

IndexReader::IndexReader(Indexer* pIndex, DiskIndexOpenMode openMode)
        :openMode_(openMode)
        ,pIndexer_(pIndex)
        ,pBarrelsInfo_(NULL)
        ,pBarrelReader_(NULL)
        ,pForwardIndexReader_(NULL)
        ,dirty_(true)
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
        pDocLengthReader_ = new DocLengthReader(pIndexer_->getCollectionsMeta().begin()->second.getDocumentSchema(), 
                                                        pIndexer_->getDirectory());
}

IndexReader::~IndexReader(void)
{
    if (pBarrelReader_)
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
    }
    if(pForwardIndexReader_)
        delete pForwardIndexReader_;
    if(pDocFilter_)
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
        delete pDocFilter_;
        pDocFilter_ = NULL;
    }
}

docid_t IndexReader::maxDoc()
{
    return pBarrelsInfo_->maxDocId();
}

size_t IndexReader::docLength(docid_t docId, fieldid_t fid)
{
    assert(pDocLengthReader_ != NULL);
    if (dirty_)
    {
        pDocLengthReader_->load(pBarrelsInfo_->maxDocId());
    }
    return pDocLengthReader_->docLength(docId, fid);
}

void IndexReader::createBarrelReader()
{
    if (pBarrelReader_)
        delete pBarrelReader_;
    int32_t bc = pBarrelsInfo_->getBarrelCount();
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    if ( (bc > 0) && pLastBarrel)
    {
        if (pLastBarrel->getDocCount() <= 0)///skip empty barrel
            bc--;
        pLastBarrel = (*pBarrelsInfo_)[bc - 1];
    }

    if (bc == 1)
    {
        if (pLastBarrel && pLastBarrel->getWriter())
            pBarrelReader_ = pLastBarrel->getWriter()->inMemoryReader();
        else
            pBarrelReader_ = new SingleIndexBarrelReader(pIndexer_,pLastBarrel,openMode_);
    }
    else if (bc > 1)
    {
        pBarrelReader_ = new MultiIndexBarrelReader(pIndexer_,pBarrelsInfo_,openMode_);
    }
    else
        SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the index barrel number is 0.");

    if(pDocLengthReader_)
        pDocLengthReader_->load(pBarrelsInfo_->maxDocId());

    pIndexer_->setDirty(false);///clear dirty_ flag

}

TermReader* IndexReader::doGetTermReader_(collectionid_t colID)
{
    if (dirty_)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader_ == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader_;
            pBarrelReader_ = NULL;
        }
        dirty_ = false;
    }

    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }

    return pBarrelReader_->termReader(colID);
}

TermReader* IndexReader::getTermReader(collectionid_t colID)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
        return pTermReader->clone();
    else
        return NULL;
}

ForwardIndexReader* IndexReader::getForwardIndexReader()
{
    if(!pForwardIndexReader_)
        pForwardIndexReader_ = new ForwardIndexReader(pIndexer_->getDirectory());

    return pForwardIndexReader_->clone();
}

// count_t IndexReader::maxDoc()
// {
//     return pBarrelsInfo_->getDocCount();
// }
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

void IndexReader::deleteDocumentPhysically(IndexerDocument* pDoc)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }
    pBarrelReader_->deleteDocumentPhysically(pDoc);
    DocId uniqueID;
    pDoc->getDocId(uniqueID);
    BarrelInfo* pBarrelInfo = findDocumentInBarrels(uniqueID.colId, uniqueID.docId);
    pBarrelInfo->deleteDocument();
    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    pDoc->getPropertyList(propertyValueList);
    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if(!iter->first.isIndex())
            continue;    
        if (!iter->first.isForward())
        {
            pIndexer_->getBTreeIndexer()->remove(uniqueID.colId, iter->first.getPropertyId(), boost::get<PropertyType>(iter->second), uniqueID.docId);
        }
    }
    pIndexer_->setDirty(true);//flush barrelsinfo when Indexer quit
}

void IndexReader::delDocument(collectionid_t colID,docid_t docId)
{
    BarrelInfo* pBarrelInfo = findDocumentInBarrels(colID, docId);
    pBarrelInfo->deleteDocument();

    if(!pDocFilter_)
    {
        pDocFilter_ = new BitVector(pBarrelsInfo_->getDocCount() + 1);
    }
    pDocFilter_->set(docId);
}

freq_t IndexReader::docFreq(collectionid_t colID, Term* term)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
    {
        return pTermReader->docFreq(term);
    }

    return 0;
}

TermInfo* IndexReader::termInfo(collectionid_t colID,Term* term)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    TermReader* pTermReader = doGetTermReader_(colID);
    if (pTermReader)
    {
        return pTermReader->termInfo(term);
    }

    return 0;
}

size_t IndexReader::getDistinctNumTerms(collectionid_t colID, fieldid_t fid)
{
    if (dirty_)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader_ == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader_;
            pBarrelReader_ = NULL;
        }
        dirty_ = false;
    }

    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }
    return pBarrelReader_->getDistinctNumTerms(colID, fid);
}

