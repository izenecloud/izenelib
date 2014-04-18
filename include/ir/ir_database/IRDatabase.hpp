///
/// @file IRDatabase.h
/// @brief A wrapper of IndexManager, easy to use.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2009
/// @date Updated 2009-11-21 00:54:26
/// @date Refined 2009-11-24 Jinglei
///
/// @todo separate implementation of indexer and raw data db
/// @todo move implementation of indexer to cpp
/// @todo reuse indexer and document manager?
///

#ifndef __IRDATABASE_H
#define __IRDATABASE_H

#include "IRConstant.hpp"
#include "IRDocument.hpp"

#include <am/3rdparty/rde_hash.h>
#include <am/sdb_hash/sdb_hash.h>
#include <am/tokyo_cabinet/tc_hash.h>
#include <am/tokyo_cabinet/tc_btree.h>
#include <ir/id_manager/IDManager.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/TermIterator.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Term.h>

#include <boost/filesystem.hpp>


#include <deque>
#include <string>
#include <vector>

NS_IZENELIB_IR_BEGIN
namespace irdb
{
    template <typename K, typename V>
    class NULLContainerType
    {
        public:
            NULLContainerType(const std::string& path)
            {
            }
            void open(){}
            void flush(){}
            void update(const K& key, const V& value){}
            void setCacheSize(uint32_t num){}
            void close(){}
            bool get(const K& key, V& value)
            {
                return false;
            }
    };
    
    namespace iii = izenelib::ir::indexmanager;
    
    
    template <typename DATA_VECTOR_TYPE, typename DB_VECTOR_TYPE>
    class IRDatabase : public boost::noncopyable
    {
        BOOST_MPL_ASSERT_RELATION( boost::mpl::size<DATA_VECTOR_TYPE>::value, ==, boost::mpl::size<DB_VECTOR_TYPE>::value );
        BOOST_MPL_ASSERT_RELATION( boost::mpl::size<DATA_VECTOR_TYPE>::value, >, 0 );
        typedef typename DATA_VECTOR_TYPE::type DATA_VECTOR_TYPE_TYPE;
        typedef typename DB_VECTOR_TYPE::type DB_VECTOR_TYPE_TYPE;
        typedef std::vector<std::string> fields_type;

        typedef IRConstant C;
        typedef boost::tuple<uint32_t> INFO_TYPE;
        
        private:
          
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerNew
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                        irDb.dataDb_[POS] = new DB_TYPE(irDb.path_+"/data-db-"+boost::lexical_cast<std::string>(POS));
                        DBHandlerNew<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerNew<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                        irDb.dataDb_[0] = new DB_TYPE(irDb.path_+"/data-db-0");
                    }
            };
            
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerOpen
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[POS]);
                        pDb->open();
                        DBHandlerOpen<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerOpen<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[0]);
                        pDb->open();
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerClose
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[POS]);
                        pDb->close();
                        DBHandlerClose<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerClose<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[0]);
                        pDb->close();
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerFlush
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[POS]);
                        pDb->flush();
                        DBHandlerFlush<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerFlush<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[0]);
                        pDb->flush();
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerDelete
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[POS]);
                        delete pDb;
                        
                        DBHandlerDelete<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerDelete<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb)
                    {
                        typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                        DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[0]);
                        delete pDb;
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I, std::size_t POS>
            class DBHandlerUpdate
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb, docid_t docId, IRDocument<DATA_VECTOR_TYPE_I>& doc)
                    {
                        if( doc.hasData(POS) )
                        {
                            typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, POS>::type DB_TYPE;
                            typedef typename boost::mpl::at_c<DATA_VECTOR_TYPE_TYPE_I, POS>::type DATA_TYPE;
                            std::vector<boost::any>& allData = doc.getAllData();
                            
                            DATA_TYPE data = boost::any_cast<DATA_TYPE>(allData[POS]);
                            
                            DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[POS]);
                            pDb->update(docId, data);
                            
                        }
                        
                        DBHandlerUpdate<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, POS-1>::act(irDb, docId, doc);
                    }
            };
            
            template <typename DATA_VECTOR_TYPE_I, typename DB_VECTOR_TYPE_I>
            class DBHandlerUpdate<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I, 0>
            {
                typedef typename DATA_VECTOR_TYPE_I::type DATA_VECTOR_TYPE_TYPE_I;
                typedef typename DB_VECTOR_TYPE_I::type DB_VECTOR_TYPE_TYPE_I;
                public:
                    static void act(IRDatabase<DATA_VECTOR_TYPE_I, DB_VECTOR_TYPE_I>& irDb, docid_t docId, IRDocument<DATA_VECTOR_TYPE_I>& doc)
                    {
                        if( doc.hasData(0) )
                        {
                            typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE_I, 0>::type DB_TYPE;
                            typedef typename boost::mpl::at_c<DATA_VECTOR_TYPE_TYPE_I, 0>::type DATA_TYPE;
                            std::vector<boost::any>& allData = doc.getAllData();
                            
                            DATA_TYPE data = boost::any_cast<DATA_TYPE>(allData[0]);
                            
                            DB_TYPE* pDb = (DB_TYPE*)(irDb.dataDb_[0]);
                            pDb->update(docId, data);
                            
                        }
                    }
            };
            
                    
        
        public:
            IRDatabase(const std::string& dir,const fields_type& propertyList = C::kDefaultFields(),bool inMemory = false)
            : path_(dir)
            , isOpen_(false)
            , inMemory_(inMemory)
            , indexer_()
            , propertyList_(propertyList)
            , propertyMap_()
            , dataSize_(boost::mpl::size<DATA_VECTOR_TYPE>::value)
            , dataDb_(dataSize_, NULL)
            , info_(1)
            , serInfo_(NULL)
            
            {
                DBHandlerNew<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this);
            }
            
            ~IRDatabase()
            {
                close();
                DBHandlerDelete<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this);
                
                if(serInfo_!= NULL)
                {
                    delete serInfo_;
                    serInfo_ = NULL;
                }
            }
            
        public:
            
            template <std::size_t POS>
            void setCacheSize(uint32_t num)
            {
                typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE, POS>::type DB_TYPE;
                DB_TYPE* pDb = (DB_TYPE*)dataDb_[POS];
                pDb->setCacheSize(num);
            }

            
            void initIndexer(boost::shared_ptr<iii::Indexer>& indexer)
            {
                iii::IndexManagerConfig indexManagerConfig;
                indexManagerConfig.indexStrategy_.indexLocation_ = path_;
                indexManagerConfig.indexStrategy_.indexMode_ = "default";
                indexManagerConfig.indexStrategy_.memory_ = 30000000;
                indexManagerConfig.indexStrategy_.indexDocLength_ = true;
                indexManagerConfig.indexStrategy_.skipInterval_ = 0;
                indexManagerConfig.indexStrategy_.maxSkipLevel_ = 0;
                indexManagerConfig.indexStrategy_.isIndexBTree_ = false;
                indexManagerConfig.mergeStrategy_.param_ = "default";
                indexManagerConfig.mergeStrategy_.isAsync_ = false;
                if(inMemory_)
                {
                    indexManagerConfig.storeStrategy_.param_ = "memory";
                }
                else
                {
                    indexManagerConfig.storeStrategy_.param_ = "file";
                }
                //now collection id always be 1 in IRDatabase.
                collectionid_t collectionId = C::kCollectionId();
                std::map<std::string, collectionid_t> collectionIdMapping;
                collectionIdMapping.insert(std::pair<std::string, collectionid_t>("coll1", collectionId));
                std::sort(propertyList_.begin(),propertyList_.end());
                iii::IndexerCollectionMeta indexCollectionMeta;
                indexCollectionMeta.setName("coll1");
                for(std::size_t i=0;i<propertyList_.size();i++)
                {
                    iii::IndexerPropertyConfig indexerPropertyConfig(C::kStartFieldId()+i, propertyList_[i], true, true);
                    indexCollectionMeta.addPropertyConfig(indexerPropertyConfig);
                    propertyMap_.insert(propertyList_[i], C::kStartFieldId()+i);
                    
                    
                }
                indexManagerConfig.addCollectionMeta(indexCollectionMeta);
                
                try {
                    indexer.reset(new iii::Indexer);
                    indexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
                }
                catch(std::exception& ex)
                {
                    indexer.reset();
                    std::cout<<"index at "<<path_<<" broken, clear"<<std::endl;
                    boost::filesystem::remove_all(path_);
                    boost::filesystem::create_directories(path_);
                    indexer.reset(new iii::Indexer);
                    indexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
                }
            }
            
            
            /// @brief Open all neccessary resource, automatically called by constructor.
            void open()
            {
                if(isOpen()) return;
                initIndexer(indexer_);
                //deserialize max doc id
                serInfo_ = new izenelib::am::tc_hash<bool, INFO_TYPE>(path_+"/ser_info");
                serInfo_->setCacheSize(1);
                serInfo_->open();
                bool b = serInfo_->get(0, info_);
                if(!b)
                {
                    serInfo_->update(0, info_);
                }
                serInfo_->flush();
                
                
                indexReader_ = NULL;
                
                DBHandlerOpen<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this);
                //open the db which storage the data to each document
//                 dataDbOpen_<boost::mpl::size<DATA_VECTOR_TYPE>::value>();

                
                isOpen_ = true;
            }
            
            bool isOpen() const
            {
                return isOpen_;
            }

            
            std::string getPath()
            {
                return path_;
            }
            
            /// @brief Return the number of documents.
            /// 
            /// @return The number.
            std::size_t numDocuments() const
            {
                if(!isOpen()) return 0;
                return getIndexReader()->numDocs();
            }
            
            
            /// @brief Return the number of terms in default collection and specific field.
            /// 
            /// @param field The field.
            /// 
            /// @return 
            std::size_t numTerms( const std::string& field ) const
            {
                if(!isOpen()) return 0;
                return indexer_->getDistinctNumTermsByProperty(C::kCollectionId(),field);
            }
            
            docid_t nextDocId()
            {
                if(!isOpen()) return 0;
                docid_t ret = boost::get<0>(info_);
                boost::get<0>(info_)++;
                serInfo_->update(0, info_);
                return ret;
            }
            
            /// @brief Add document, the doc id is incremented.
            /// 
            /// @param irDocument The document to be added.
            void addDocument(IRDocument<DATA_VECTOR_TYPE>& irDocument)
            {
                if(!isOpen()) return;
                docid_t docId = nextDocId();
                addDocument(docId,irDocument);
                
            }
            
            /// @brief Add document with specific doc id.
            /// 
            /// @param docId The doc id.
            /// @param irDocument The document.
            void addDocument(docid_t docId, IRDocument<DATA_VECTOR_TYPE>& irDocument)
            {
                if(!isOpen()) return;
                if(irDocument.termSize()==0) return;
                irDocument.sortField();
                iii::IndexerDocument document;
                document.setDocId(docId,C::kCollectionId());
                std::string propertyName = "";
                
                IRTermIterator ti = irDocument.termListBegin();
                while(ti!=irDocument.termListEnd())
                {
                    uint32_t* propertyId = propertyMap_.find((*ti).field_);
                    if(propertyId == NULL)
                    {
                        ++ti;
                        continue;
                    }
                    if(propertyName == "" || propertyName != (*ti).field_)
                    {
                        propertyName = (*ti).field_;
                        
                        iii::IndexerPropertyConfig propertyConfig(*propertyId,propertyName,true,true);

                        boost::shared_ptr<iii::LAInput> laInput(new iii::LAInput);
                        laInput->setDocId(docId);
                        laInput->reserve(1024);
                        document.insertProperty(propertyConfig, laInput);
                    }
                    
                    termid_t tid = ti->termId_;
                    loc_t pos = (*ti).position_;
                    iii::LAInputUnit unit;
                    unit.termid_ = tid;
                    unit.docId_ = docId;
                    unit.wordOffset_ = pos;
                    document.add_to_property(unit);
                    ti++;
//                     std::cout<<"ADD TERM "<<tid<<" in DOC "<<docId<<std::endl;
                }
                
                indexer_->insertDocument(document);
                //add document data if neccessary
                updateDocData_(docId, irDocument);

            }
            
            
            /// @brief Delete document.
            /// 
            /// @param docId The doc id to be deleted.
            void deleteDocument(docid_t docId)
            {
                if(!isOpen()) return;
                indexer_->removeDocument(C::kCollectionId(), docId);
                //did not delete document data here, for update later
            }
            
            void updateDocument(docid_t oldDocId, docid_t docId, IRDocument<DATA_VECTOR_TYPE>& irDocument)
            {
                if(!isOpen()) return;
                if(irDocument.termSize()==0) return;
                irDocument.sortField();
                iii::IndexerDocument document;
                document.setOldId(oldDocId);
                document.setDocId(docId,C::kCollectionId());
                std::string propertyName = "";

                IRTermIterator ti = irDocument.termListBegin();
                while(ti!=irDocument.termListEnd())
                {
                    uint32_t* propertyId = propertyMap_.find((*ti).field_);
                    if(propertyId == NULL)
                    {
                        ++ti;
                        continue;
                    }
                    if(propertyName == "" || propertyName != (*ti).field_)
                    {
                        propertyName = (*ti).field_;

                        iii::IndexerPropertyConfig propertyConfig(*propertyId,propertyName,true,true);

                        boost::shared_ptr<iii::LAInput> laInput(new iii::LAInput);
                        laInput->setDocId(docId);
                        laInput->reserve(1024);
                        document.insertProperty(propertyConfig, laInput);
                    }

                    termid_t tid = ti->termId_;
                    loc_t pos = (*ti).position_;
                    iii::LAInputUnit unit;
                    unit.termid_ = tid;
                    unit.docId_ = docId;
                    unit.wordOffset_ = pos;
                    document.add_to_property(unit);
                    ti++;
//                     std::cout<<"ADD TERM "<<tid<<" in DOC "<<docId<<std::endl;
                }

                indexer_->updateDocument(document);
                //add document data if neccessary
                updateDocData_(docId, irDocument);
            }
            
            void optimizeIndex()
            {
                if(!isOpen()) return;
                indexer_->optimizeIndex();
            }
            
            template <uint32_t POS>
            void setDocData(docid_t docId, const typename boost::mpl::at_c<DATA_VECTOR_TYPE_TYPE, POS>::type& data)
            {
                if(!isOpen()) return;
                typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE, POS>::type DB_TYPE;
                DB_TYPE* pDb = (DB_TYPE*)dataDb_[POS];
                pDb->update(docId, data);

            }

            template <uint32_t POS>
            bool getDocData (docid_t docId, typename boost::mpl::at_c<DATA_VECTOR_TYPE_TYPE, POS>::type& data)
            {
                if(!isOpen()) return false;
                typedef typename boost::mpl::at_c<DB_VECTOR_TYPE_TYPE, POS>::type DB_TYPE;
                DB_TYPE* pDb = (DB_TYPE*)dataDb_[POS];
                return pDb->get(docId, data);
            }
            
                    

            
            bool getMatchSet(termid_t termId, std::deque<docid_t>& docIdList)
            {
                if(!isOpen()) return false;
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), propList, docIdList);
            }
            
            bool getMatchSet(termid_t termId, const std::vector<std::string>& fieldList, std::deque<docid_t>& docIdList)
            {
                if(!isOpen()) return false;
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), fieldList, docIdList);
            }
            
            bool getMatchSet(termid_t termId, const std::vector<std::string>& fieldList, std::deque<iii::CommonItem>& commonItemList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), fieldList, commonItemList);
            }
            bool getMatchSet(termid_t termId, std::deque<iii::CommonItem>& commonItemList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), propList, commonItemList);
            }
            
            bool getMatchSet(termid_t termId, boost::shared_ptr<iii::TermDocFreqs> & termDocFreq)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                iii::IndexReader* pIndexReader= getIndexReader();
                boost::shared_ptr<iii::TermReader> pTermReader(pIndexReader->getTermReader(1));
                if ( pTermReader.get() == NULL)
                    return false;
                iii::Term irTerm(C::kDefaultFieldName().c_str(),termId);
                if(!pTermReader->seek(&irTerm))
                    return false;
                termDocFreq.reset(pTermReader->termDocFreqs());
                if ( termDocFreq.get() == NULL)
                    return false;
                return true;
            }
            
            bool getTF(termid_t termId, uint32_t& tf)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<unsigned int> tfList;
                std::vector<termid_t> termList(1);
                termList[0] = termId;
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                bool b = indexer_->getTermFrequencyInCollectionByTermId(termList,C::kCollectionId(),propList,tfList);
                if(!b)  return false;
                tf = tfList[0];
                return true;
            }
            
            bool getTF(termid_t termId, const std::vector<std::string>& fieldList, std::vector<uint32_t>& tfList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<termid_t> termList(1);
                termList[0] = termId;
                bool b = indexer_->getTermFrequencyInCollectionByTermId(termList,C::kCollectionId(),fieldList,tfList);
                if(!b)  return false;
                return true;
            }
            
            uint32_t getDocumentLength(docid_t docId, uint32_t fieldId = 1)
            {
                if(!isOpen()) return 0;
                boost::mutex::scoped_lock lock(getMutex_);
                return getIndexReader()->docLength(docId, fieldId);	
                
            }
            
            
            // client should not delete returned IndexReader
            iii::IndexReader* getIndexReader() const
            {
                if(!isOpen()) return NULL;
                boost::mutex::scoped_lock lock(getMutex_);
                if(indexReader_ == NULL)
                {
                    indexReader_ = indexer_->getIndexReader();
                }
                return indexReader_;
            }

            
            void flush()
            {
                if(isOpen())
                {
                    
                    indexer_->flush();
                    indexer_->optimizeIndex();
                    indexer_->waitForMergeFinish();
                    DBHandlerFlush<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this);

                    if(serInfo_)
                        serInfo_->flush();
                    
                }
            }


            void close()
            {
                if(isOpen())
                {
                    
                    flush();
                    DBHandlerClose<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this);
                    if(serInfo_)
                         serInfo_->close();
                    isOpen_ = false;
                    
                }
            }
            
        private:
            
            
            void updateDocData_(docid_t docId, IRDocument<DATA_VECTOR_TYPE>& doc)
            {
                DBHandlerUpdate<DATA_VECTOR_TYPE, DB_VECTOR_TYPE, boost::mpl::size<DATA_VECTOR_TYPE>::value-1>::act(*this, docId, doc);

                
            }

            
        private:
            std::string path_;
            bool isOpen_;
            bool inMemory_;
            
            boost::shared_ptr<iii::Indexer> indexer_;
            std::vector<std::string> propertyList_;
            izenelib::am::rde_hash<std::string, uint32_t> propertyMap_;
            mutable iii::IndexReader* indexReader_;
            
            const std::size_t dataSize_;
            std::vector<void* > dataDb_;
            
            INFO_TYPE info_;
            izenelib::am::tc_hash<bool, INFO_TYPE >* serInfo_;
            
            mutable boost::mutex getMutex_;
    };
    
    
    
    //partial template version for non-data-db
    class PureIRDatabase
    {

        typedef std::vector<std::string> fields_type;

        typedef IRConstant C;
        typedef boost::tuple<uint32_t> INFO_TYPE;
                    
        
        public:
            PureIRDatabase(const std::string& dir,const fields_type& propertyList = C::kDefaultFields(),bool inMemory = false)
            : path_(dir)
            , isOpen_(false)
            , inMemory_(inMemory)
            , indexer_()
            , propertyList_(propertyList)
            , propertyMap_()
            , info_(0)
            , serInfo_(NULL)
            
            {
                
            }
            
            ~PureIRDatabase()
            {
                close();
                
                if(serInfo_!= NULL)
                {
                    delete serInfo_;
                    serInfo_ = NULL;
                }
            }
            
        private:
            //avoid copy constructor
            PureIRDatabase(const PureIRDatabase& db) {}
            PureIRDatabase& operator=(const PureIRDatabase& rhs)
            {
                return *this;
            }
            
        public:
            
           
            void initIndexer(boost::shared_ptr<iii::Indexer>& indexer)
            {
                iii::IndexManagerConfig indexManagerConfig;
                indexManagerConfig.indexStrategy_.indexLocation_ = path_;
                indexManagerConfig.indexStrategy_.indexMode_ = "default";
                indexManagerConfig.indexStrategy_.memory_ = 30000000;
                indexManagerConfig.indexStrategy_.indexDocLength_ = true;
                indexManagerConfig.indexStrategy_.isIndexBTree_ = false;
                indexManagerConfig.mergeStrategy_.param_ = "default";
                indexManagerConfig.mergeStrategy_.isAsync_ = false;
                if(inMemory_)
                {
                    indexManagerConfig.storeStrategy_.param_ = "memory";
                }
                else
                {
                    indexManagerConfig.storeStrategy_.param_ = "file";
                }
                //now collection id always be 1 in IRDatabase.
                collectionid_t collectionId = C::kCollectionId();
                std::map<std::string, collectionid_t> collectionIdMapping;
                collectionIdMapping.insert(std::pair<std::string, collectionid_t>("coll1", collectionId));
                std::sort(propertyList_.begin(),propertyList_.end());
                iii::IndexerCollectionMeta indexCollectionMeta;
                indexCollectionMeta.setName("coll1");
                for(std::size_t i=0;i<propertyList_.size();i++)
                {
                    iii::IndexerPropertyConfig indexerPropertyConfig(C::kStartFieldId()+i, propertyList_[i], true, true);
                    indexCollectionMeta.addPropertyConfig(indexerPropertyConfig);
                    propertyMap_.insert(propertyList_[i], C::kStartFieldId()+i);
                    
                    
                }
                indexManagerConfig.addCollectionMeta(indexCollectionMeta);
                try {
                    indexer.reset(new iii::Indexer);
                    indexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
                }
                catch(std::exception& ex)
                {
                    indexer.reset();
                    boost::filesystem::remove_all(path_);
                    boost::filesystem::create_directories(path_);
                    indexer.reset(new iii::Indexer);
                    indexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);
                }
            }
            
            
            /// @brief Open all neccessary resource, automatically called by constructor.
            void open()
            {
                if(isOpen()) return;
                initIndexer(indexer_);
                //deserialize max doc id
                serInfo_ = new izenelib::am::tc_hash<bool, INFO_TYPE>(path_+"/ser_info");
                serInfo_->setCacheSize(1);
                serInfo_->open();
                bool b = serInfo_->get(0, info_);
                if(!b)
                {
                    serInfo_->update(0, info_);
                }
                serInfo_->flush();
                
                
                indexReader_ = NULL;
                
                isOpen_ = true;
            }
            
            bool isOpen() const
            {
                return isOpen_;
            }

            
            std::string getPath()
            {
                return path_;
            }
            
            /// @brief Return the number of documents.
            /// 
            /// @return The number.
            std::size_t numDocuments() const
            {
                if(!isOpen()) return 0;
                return getIndexReader()->numDocs();
            }
            
            
            /// @brief Return the number of terms in default collection and specific field.
            /// 
            /// @param field The field.
            /// 
            /// @return 
            std::size_t numTerms( const std::string& field ) const
            {
                if(!isOpen()) return 0;
                return indexer_->getDistinctNumTermsByProperty(C::kCollectionId(),field);
            }
            
            docid_t nextDocId()
            {
                if(!isOpen()) return 0;
                docid_t ret = boost::get<0>(info_);
                boost::get<0>(info_)++;
                serInfo_->update(0, info_);
                return ret;
            }
            
            /// @brief Add document, the doc id is incremented.
            /// 
            /// @param irDocument The document to be added.
            void addDocument(PureIRDocument& irDocument)
            {
                if(!isOpen()) return;
                docid_t docId = nextDocId();
                addDocument(docId,irDocument);
                
            }
            
            /// @brief Add document with specific doc id.
            /// 
            /// @param docId The doc id.
            /// @param irDocument The document.
            void addDocument(docid_t docId, PureIRDocument& irDocument)
            {
                if(!isOpen()) return;
                if(irDocument.termSize()==0) return;
                irDocument.sortField();
                iii::IndexerDocument document;
                document.setDocId(docId,C::kCollectionId());
                std::string propertyName = "";
                
                IRTermIterator ti = irDocument.termListBegin();
                while(ti!=irDocument.termListEnd())
                {
                    uint32_t* propertyId = propertyMap_.find((*ti).field_);
                    if(propertyId == NULL)
                    {
                        ++ti;
                        continue;
                    }
                    if(propertyName == "" || propertyName != (*ti).field_)
                    {
                        propertyName = (*ti).field_;
                        
                        iii::IndexerPropertyConfig propertyConfig(*propertyId,propertyName,true,true);

                        boost::shared_ptr<iii::LAInput> laInput(new iii::LAInput);
                        laInput->setDocId(docId);
                        laInput->reserve(1024);
                        document.insertProperty(propertyConfig, laInput);
                    }
                    
                    termid_t tid = ti->termId_;
                    loc_t pos = (*ti).position_;
                    iii::LAInputUnit unit;
                    unit.termid_ = tid;
                    unit.docId_ = docId;
                    unit.wordOffset_ = pos;
                    document.add_to_property(unit);
                    ti++;
//                     std::cout<<"ADD TERM "<<tid<<" in DOC "<<docId<<std::endl;
                }
                
                indexer_->insertDocument(document);
            }
            
            
            /// @brief Delete document.
            /// 
            /// @param docId The doc id to be deleted.
            void deleteDocument(docid_t docId)
            {
                if(!isOpen()) return;
                indexer_->removeDocument(C::kCollectionId(), docId);
                //did not delete document data here, for update later
            }
            
            void updateDocument(docid_t oldDocId, docid_t docId, PureIRDocument& irDocument)
            {
                if(!isOpen()) return;
                if(irDocument.termSize()==0) return;
                irDocument.sortField();
                iii::IndexerDocument document;
                document.setOldId(oldDocId);
                document.setDocId(docId,C::kCollectionId());
                std::string propertyName = "";

                IRTermIterator ti = irDocument.termListBegin();
                while(ti!=irDocument.termListEnd())
                {
                    uint32_t* propertyId = propertyMap_.find((*ti).field_);
                    if(propertyId == NULL)
                    {
                        ++ti;
                        continue;
                    }
                    if(propertyName == "" || propertyName != (*ti).field_)
                    {
                        propertyName = (*ti).field_;

                        iii::IndexerPropertyConfig propertyConfig(*propertyId,propertyName,true,true);

                        boost::shared_ptr<iii::LAInput> laInput(new iii::LAInput);
                        laInput->setDocId(docId);
                        laInput->reserve(1024);
                        document.insertProperty(propertyConfig, laInput);
                    }

                    termid_t tid = ti->termId_;
                    loc_t pos = (*ti).position_;
                    iii::LAInputUnit unit;
                    unit.termid_ = tid;
                    unit.docId_ = docId;
                    unit.wordOffset_ = pos;
                    document.add_to_property(unit);
                    ti++;
//                     std::cout<<"ADD TERM "<<tid<<" in DOC "<<docId<<std::endl;
                }

                indexer_->updateDocument(document);

            }
            
            void optimizeIndex()
            {
                if(!isOpen()) return;
                indexer_->optimizeIndex();
            }
            
    
            bool getMatchSet(termid_t termId, std::deque<docid_t>& docIdList)
            {
                if(!isOpen()) return false;
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), propList, docIdList);
            }
            
            bool getMatchSet(termid_t termId, const std::vector<std::string>& fieldList, std::deque<docid_t>& docIdList)
            {
                if(!isOpen()) return false;
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), fieldList, docIdList);
            }
            
            bool getMatchSet(termid_t termId, const std::vector<std::string>& fieldList, std::deque<iii::CommonItem>& commonItemList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), fieldList, commonItemList);
            }
            bool getMatchSet(termid_t termId, std::deque<iii::CommonItem>& commonItemList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                return indexer_->getDocsByTermInProperties(termId, C::kCollectionId(), propList, commonItemList);
            }
            
            bool getMatchSet(termid_t termId, boost::shared_ptr<iii::TermDocFreqs> & termDocFreq)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                iii::IndexReader* pIndexReader= getIndexReader();
                boost::shared_ptr<iii::TermReader> pTermReader(pIndexReader->getTermReader(1));
                if ( pTermReader.get() == NULL)
                    return false;
                iii::Term irTerm(C::kDefaultFieldName().c_str(),termId);
                if(!pTermReader->seek(&irTerm))
                    return false;
                termDocFreq.reset(pTermReader->termDocFreqs());
                if ( termDocFreq.get() == NULL)
                    return false;
                return true;
            }
            
            bool getTF(termid_t termId, uint32_t& tf)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<unsigned int> tfList;
                std::vector<termid_t> termList(1);
                termList[0] = termId;
                std::vector<std::string> propList(1);
                propList[0] = C::kDefaultFieldName();
                bool b = indexer_->getTermFrequencyInCollectionByTermId(termList,C::kCollectionId(),propList,tfList);
                if(!b)  return false;
                tf = tfList[0];
                return true;
            }
            
            bool getTF(termid_t termId, const std::vector<std::string>& fieldList, std::vector<uint32_t>& tfList)
            {
                if(!isOpen()) return false;
                boost::mutex::scoped_lock lock(getMutex_);
                std::vector<termid_t> termList(1);
                termList[0] = termId;
                bool b = indexer_->getTermFrequencyInCollectionByTermId(termList,C::kCollectionId(),fieldList,tfList);
                if(!b)  return false;
                return true;
            }
            
            uint32_t getDocumentLength(docid_t docId, uint32_t fieldId = 1)
            {
                if(!isOpen()) return 0;
                boost::mutex::scoped_lock lock(getMutex_);
                return getIndexReader()->docLength(docId, fieldId); 
                
            }
            
            
            // client should not delete returned IndexReader
            iii::IndexReader* getIndexReader() const
            {
                if(!isOpen()) return NULL;
                boost::mutex::scoped_lock lock(getMutex_);
                if(indexReader_ == NULL)
                {
                    indexReader_ = indexer_->getIndexReader();
                }
                return indexReader_;
            }

            
            void flush()
            {
                if(isOpen())
                {
                    
                    indexer_->flush();
                    indexer_->optimizeIndex();
                    indexer_->waitForMergeFinish();

                    if(serInfo_)
                        serInfo_->flush();
                    
                }
            }


            void close()
            {
                if(isOpen())
                {
                    
                    flush();
                    if(serInfo_)
                         serInfo_->close();
                    isOpen_ = false;
                    
                }
            }
            

            
        private:
            std::string path_;
            bool isOpen_;
            bool inMemory_;
            
            boost::shared_ptr<iii::Indexer> indexer_;
            std::vector<std::string> propertyList_;
            izenelib::am::rde_hash<std::string, uint32_t> propertyMap_;
            mutable iii::IndexReader* indexReader_;
            
            INFO_TYPE info_;
            izenelib::am::tc_hash<bool, INFO_TYPE >* serInfo_;
            
            mutable boost::mutex getMutex_;
    };
    
    

    
}
NS_IZENELIB_IR_END
#endif
