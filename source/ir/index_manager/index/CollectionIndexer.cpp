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
using namespace izenelib::util;

using namespace izenelib::ir::indexmanager;

CollectionIndexer::CollectionIndexer(collectionid_t id,
                                                                        MemCache* pCache,
                                                                        Indexer* pIndexer)
        :colID_(id)
        ,pMemCache_(pCache)
        ,pIndexer_(pIndexer)
        ,pDocLengthWriter_(NULL)
        ,docLengthWidth_(0)
{
    pFieldsInfo_ = new FieldsInfo();
}

CollectionIndexer::~CollectionIndexer()
{
    delete pFieldsInfo_;
    pMemCache_ = NULL;
    pIndexer_ = NULL;
    if (pDocLengthWriter_)
    {
        delete pDocLengthWriter_;
        pDocLengthWriter_ = NULL;
    }
}

void CollectionIndexer::setSchema(const IndexerCollectionMeta& schema)
{
    pFieldsInfo_->setSchema(schema);
    if (pIndexer_->getIndexManagerConfig()->indexStrategy_.indexDocLength_)
    {
        pDocLengthWriter_ = new DocLengthWriter(schema.getDocumentSchema(), 
                                                                          pIndexer_->getDirectory());
        docLengthWidth_ = pDocLengthWriter_->get_num_properties();
    }
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
            FieldIndexer* pFieldIndexer = new FieldIndexer(pMemCache_,pIndexer_);
            pFieldIndexer->setField(pFieldInfo->getName());
            fieldIndexerMap_.insert(make_pair(pFieldInfo->getName(),pFieldIndexer));
        }
    }
}

void CollectionIndexer::addDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    doc.getPropertyList(propertyValueList);

    uint16_t docLength[docLengthWidth_];
    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter 
                            = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (!iter->first.isIndex())
            continue;
        if (iter->first.isForward())
        {
            if (iter->first.isFilter())
            {
                IndexPropertyType indexData = boost::get<IndexPropertyType >(iter->second);

                pIndexer_->getBTreeIndexer()->
                        add(uniqueID.colId, iter->first.getPropertyId(), indexData.second, uniqueID.docId);

                map<string, boost::shared_ptr<FieldIndexer> >::iterator it = 
                        fieldIndexerMap_.find(iter->first.getName());

                if (it == fieldIndexerMap_.end())
                    // This field is not indexed.
                    continue;

                boost::shared_ptr<LAInput> laInput = indexData.first;
                it->second->addField(uniqueID.docId, laInput);

                if (pDocLengthWriter_)
                    pDocLengthWriter_->fill(iter->first.getPropertyId(), laInput->size(), docLength);
            }
            else
            {
                map<string, boost::shared_ptr<FieldIndexer> >::iterator it = 
                        fieldIndexerMap_.find(iter->first.getName());

                if (it == fieldIndexerMap_.end())
                    // This field is not indexed.
                    continue;
                if (! iter->first.isLAInput())
                {
                    boost::shared_ptr<ForwardIndex> forwardIndex = 
                        boost::get<boost::shared_ptr<ForwardIndex> >(iter->second);

                    it->second->addField(uniqueID.docId, forwardIndex);

                    if (pDocLengthWriter_)
                        pDocLengthWriter_->fill(iter->first.getPropertyId(), 
                                                            forwardIndex->docLength_, docLength);
                }
                else
                {
                    boost::shared_ptr<LAInput> laInput = 
                                            boost::get<boost::shared_ptr<LAInput> >(iter->second);
                    it->second->addField(uniqueID.docId, laInput);

                    if (pDocLengthWriter_)
                        pDocLengthWriter_->fill(iter->first.getPropertyId(), 
                                                            laInput->size(), docLength);

                }
            }
        }
        else
        {
            pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), 
                                                    boost::get<PropertyType>(iter->second), uniqueID.docId);
        }

    }
    if (pDocLengthWriter_)
        pDocLengthWriter_->add(uniqueID.docId,docLength);
}

void CollectionIndexer::write(OutputDescriptor* desc)
{
    IndexOutput* pVocOutput = desc->getVocOutput();
    IndexOutput* pDOutput = desc->getDPostingOutput();
    IndexOutput* pPOutput = desc->getPPostingOutput();

    fileoffset_t vocOff1,vocOff2,dfiOff1,dfiOff2,ptiOff1,ptiOff2;
    fileoffset_t vocOffset;

    FieldIndexer* pFieldIndexer;
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = 
                    fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
    {
        pFieldIndexer = iter->second.get();
        if (pFieldIndexer == NULL)
            continue;
        vocOff1 = pVocOutput->getFilePointer();
        dfiOff1 = pDOutput->getFilePointer();
        ptiOff1 = pPOutput->getFilePointer();

        pFieldsInfo_->setDistinctNumTerms(iter->first,pFieldIndexer->distinctNumTerms());

        vocOffset = pFieldIndexer->write(desc);///write field index data

        pFieldsInfo_->setFieldOffset(iter->first,vocOffset);

        vocOff2 = pVocOutput->getFilePointer();
        dfiOff2 = pDOutput->getFilePointer();
        ptiOff2 = pPOutput->getFilePointer();

        pFieldsInfo_->getField(iter->first)->
                setLength(vocOff2-vocOff1,dfiOff2-dfiOff1,ptiOff2-ptiOff1);
    }
    if (pDocLengthWriter_)
        pDocLengthWriter_->flush();
}

void CollectionIndexer::reset()
{
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = 
                fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
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

