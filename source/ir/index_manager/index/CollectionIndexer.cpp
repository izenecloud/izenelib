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
#include <cstring>

using namespace std;
using namespace izenelib::util;

using namespace izenelib::ir::indexmanager;

CollectionIndexer::CollectionIndexer(collectionid_t id, MemCache* pCache, Indexer* pIndexer)
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
        pDocLengthWriter_ = new DocLengthWriter(schema.getDocumentSchema(), pIndexer_->getDirectory());
        docLengthWidth_ = pDocLengthWriter_->get_num_properties();
    }
}

void CollectionIndexer::setFieldIndexers()
{
    size_t memCacheSize = (size_t)pIndexer_->getIndexManagerConfig()->indexStrategy_.memory_;

    pFieldsInfo_->startIterator();
    FieldInfo* pFieldInfo = NULL;
    size_t indexedProperties = 0;
    while (pFieldsInfo_->hasNext())
    {
        pFieldInfo = pFieldsInfo_->next();
        if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())
            indexedProperties++;
    }

    memCacheSize = (memCacheSize/indexedProperties) < 10*1024*1024 ? 
                              10*1024*1024:(memCacheSize/indexedProperties) ;

    pFieldsInfo_->startIterator();

    while (pFieldsInfo_->hasNext())
    {
        pFieldInfo = pFieldsInfo_->next();
        if (pFieldInfo->isIndexed()&&pFieldInfo->isAnalyzed())
        {
            FieldIndexer* pFieldIndexer = new FieldIndexer(pFieldInfo->getName(), pMemCache_,pIndexer_);
            fieldIndexerMap_.insert(make_pair(pFieldInfo->getName(),pFieldIndexer));
            pFieldIndexer->setHitBuffer(memCacheSize);
        }
    }
}

void CollectionIndexer::addDocument(IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    doc.getPropertyList(propertyValueList);

    doclen_t docLength[docLengthWidth_];
    memset(docLength, 0, docLengthWidth_*sizeof(doclen_t));

    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter 
                            = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (!iter->first.isIndex())
            continue;

        if (iter->first.isFilter())
        {
            if(iter->first.isMultiValue())
            {
                MultiValuePropertyType prop;
                if (iter->first.isAnalyzed())
                    prop = boost::get<MultiValueIndexPropertyType >(iter->second).second;
                else
                    prop = boost::get<MultiValuePropertyType>(iter->second);

                for(MultiValuePropertyType::iterator it = prop.begin(); it != prop.end(); ++it)
                    pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), *it, uniqueID.docId);
            }
            else
            {
                PropertyType prop;
                if (iter->first.isAnalyzed())
                    prop = boost::get<IndexPropertyType >(iter->second).second;
                else
                    prop = boost::get<PropertyType>(iter->second);

                pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), prop, uniqueID.docId);
            }
        }

        if (iter->first.isAnalyzed())
        {
            map<string, boost::shared_ptr<FieldIndexer> >::iterator it = fieldIndexerMap_.find(iter->first.getName());
            if (it == fieldIndexerMap_.end())
                // This field is not indexed.
                continue;

            boost::shared_ptr<LAInput> laInput;
            if (iter->first.isFilter())
                if(iter->first.isMultiValue())
                    laInput = boost::get<MultiValueIndexPropertyType >(iter->second).first;
                else
                    laInput = boost::get<IndexPropertyType >(iter->second).first;
            else
                laInput = boost::get<boost::shared_ptr<LAInput> >(iter->second);

            it->second->addField(uniqueID.docId, laInput);

            if (pDocLengthWriter_ && iter->first.isStoreDocLen())
                pDocLengthWriter_->fill(iter->first.getPropertyId(), laInput->size(), docLength);
        }
    }

    if (pDocLengthWriter_)
        pDocLengthWriter_->add(uniqueID.docId, docLength);
}

void CollectionIndexer::updateDocument(IndexerDocument& oldDoc, IndexerDocument& doc)
{
    DocId uniqueID;
    doc.getDocId(uniqueID);

    map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyValueList;
    doc.getPropertyList(propertyValueList);
    map<IndexerPropertyConfig, IndexerDocumentPropertyType> oldPropertyValueList;
    oldDoc.getPropertyList(oldPropertyValueList);
    for (map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator iter
                = propertyValueList.begin(); iter != propertyValueList.end(); ++iter)
    {
        if (iter->first.isIndex() && iter->first.isFilter())
        {
            map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator it;
            if( (it = oldPropertyValueList.find(iter->first)) != oldPropertyValueList.end() )
            {
                if(it->first.isMultiValue())
                {
                    MultiValuePropertyType oldProp;
                    oldProp = boost::get<MultiValuePropertyType>(it->second);
                    for(MultiValuePropertyType::iterator multiIt = oldProp.begin(); multiIt != oldProp.end(); ++multiIt)
                       pIndexer_->getBTreeIndexer()->remove(uniqueID.colId, iter->first.getPropertyId(), *multiIt, uniqueID.docId);
                }
                else
                {
                    PropertyType oldProp;
                    oldProp = boost::get<PropertyType>(it->second);
                    pIndexer_->getBTreeIndexer()->remove(uniqueID.colId, iter->first.getPropertyId(), oldProp, uniqueID.docId);
                }

                if(iter->first.isMultiValue())
                {
                    MultiValuePropertyType prop;
                    prop = boost::get<MultiValuePropertyType>(iter->second);
                    for(MultiValuePropertyType::iterator multiIt = prop.begin(); multiIt != prop.end(); ++multiIt)
                        pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), *multiIt, uniqueID.docId);
                }
                else
                {
                    PropertyType prop;
                    prop = boost::get<PropertyType>(iter->second);
                    pIndexer_->getBTreeIndexer()->add(uniqueID.colId, iter->first.getPropertyId(), prop, uniqueID.docId);
                }
            }
        }
    }
}

void CollectionIndexer::write(OutputDescriptor* desc)
{
    IndexOutput* pVocOutput = desc->getVocOutput();
    IndexOutput* pDOutput = desc->getDPostingOutput();
    IndexOutput* pPOutput = desc->getPPostingOutput();

    fileoffset_t vocOff1,vocOff2,dfiOff1,dfiOff2,ptiOff1,ptiOff2;
    fileoffset_t vocOffset;

    FieldIndexer* pFieldIndexer;

    bool emptyBarrel = true;
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = 
                    fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
    {
        pFieldIndexer = iter->second.get();
        if(pFieldIndexer && !pFieldIndexer->isEmpty())
        {
            emptyBarrel = false;
        }
    }
    if(emptyBarrel) SF1V5_THROW(ERROR_FILEIO,"CollectionIndexer::write empty barrels");
    for (map<string, boost::shared_ptr<FieldIndexer> >::iterator iter = 
                    fieldIndexerMap_.begin(); iter != fieldIndexerMap_.end(); ++iter)
    {
        pFieldIndexer = iter->second.get();
        if (pFieldIndexer == NULL)
            continue;
        vocOff1 = pVocOutput->getFilePointer();
        dfiOff1 = pDOutput->getFilePointer();
        ptiOff1 = pPOutput->getFilePointer();
        vocOffset = pFieldIndexer->write(desc);///write field index data

        pFieldsInfo_->setDistinctNumTerms(iter->first,pFieldIndexer->distinctNumTerms());
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

