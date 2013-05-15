#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/index/EPostingWriter.h>
#include <ir/index_manager/store/IndexInput.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <util/izene_log.h>

#include <cassert>

using namespace std;

namespace bfs = boost::filesystem;

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

void writeTermInfo(
    IndexOutput* pVocWriter,
    termid_t tid,
    const TermInfo& termInfo)
{
    pVocWriter->writeInt(tid);					///write term id
    pVocWriter->writeInt(termInfo.docFreq_);		///write df
    pVocWriter->writeInt(termInfo.ctf_);			///write ctf
    pVocWriter->writeInt(termInfo.maxTF_);     ///write maxtf
    pVocWriter->writeInt(termInfo.lastDocID_);		///write last doc id
    pVocWriter->writeInt(termInfo.skipLevel_);		///write skip level
    pVocWriter->writeLong(termInfo.skipPointer_);	///write skip list offset offset
    pVocWriter->writeLong(termInfo.docPointer_);	///write document posting offset
    pVocWriter->writeInt(termInfo.docPostingLen_);	///write document posting length (without skiplist)
    pVocWriter->writeLong(termInfo.positionPointer_);	///write position posting offset
    pVocWriter->writeInt(termInfo.positionPostingLen_);///write position posting length
}

#pragma pack(push,1)
struct Record
{
    uint8_t len;
    uint32_t tid;
    uint32_t docId;
    uint32_t offset;
};
#pragma pack(pop)

FieldIndexer::FieldIndexer(
    const char* field,
    Indexer* pIndexer
)
    :pBinlog_(NULL)
    ,field_(field)
    ,pIndexer_(pIndexer)
    ,vocFilePointer_(0)
    //,alloc_(0)
    ,f_(0)
    ,termCount_(0)
    ,iHitsMax_(0)
    ,recordCount_(0)
    ,run_num_(0)
    ,pHits_(0)
    ,pHitsMax_(0)
    ,flush_(false)
{
    skipInterval_ = pIndexer_->getSkipInterval();
    maxSkipLevel_ = pIndexer_->getMaxSkipLevel();
    indexLevel_ = pIndexer_->pConfigurationManager_->indexStrategy_.indexLevel_;

    sorterFileName_ = field_+".tmp";
    bfs::path path(bfs::path(pIndexer_->pConfigurationManager_->indexStrategy_.indexLocation_)
                   /bfs::path(sorterFileName_));
    sorterFullPath_ = path.string();
}

FieldIndexer::~FieldIndexer()
{
    if (f_)
    {
        fclose(f_);
        if(! boost::filesystem::remove(sorterFullPath_))
            LOG(WARNING) << "FieldIndexer::~FieldIndexer(): failed to remove file " << sorterFullPath_;
    }
    if (pBinlog_ != NULL)
    {
        delete pBinlog_;
    }
}

void FieldIndexer::deletebinlog()
{
    boost::filesystem::remove(BinlogPath_);
}

void FieldIndexer::checkBinlog()
{
    BarrelInfo* pCurBarrelInfo = pIndexer_->getIndexWriter()->getBarrelInfo();

    if(pBinlog_ == NULL)
    {
        pBinlog_ = new Binlog(pIndexer_);
        std::string BinlogName = field_ + ".binlog";
        bfs::path path(bfs::path(pIndexer_->pConfigurationManager_->indexStrategy_.indexLocation_)
                       /bfs::path(BinlogName));
        BinlogPath_ = path.string();
        if(pBinlog_->openForRead(BinlogPath_))
        {
            if(!pIndexer_->isRealTime())
                pIndexer_->setIndexMode("realtime");
            if(!pCurBarrelInfo)
                pIndexer_->getIndexWriter()->createBarrelInfo();
            if (pIndexer_->isRealTime())
            {
                if (!pIndexer_->getIndexReader()->hasMemBarrelReader())
                {
                    pIndexer_->setDirty();
                }
            }

            vector<boost::shared_ptr<LAInput> > laInputArray;
            vector<uint32_t> docidList;
            pBinlog_->load_Binlog(laInputArray, docidList, BinlogPath_);
            vector<boost::shared_ptr<LAInput> >::iterator iter;
            uint32_t i = 0;
            for(iter = laInputArray.begin(); iter != laInputArray.end(); iter++)
            {
                if (pIndexer_->getIndexWriter()->getBarrelInfo()->getBaseDocID() == BAD_DOCID)
                    pIndexer_->getIndexWriter()->getBarrelInfo()->addBaseDocID(1,docidList[i]);
                pIndexer_->getIndexWriter()->getBarrelInfo()->updateMaxDoc(docidList[i]);
                pIndexer_->getIndexWriter()->getBarrelsInfo()->updateMaxDoc(docidList[i]);
                /* A Document could have several fields and the field's number could vary. */
                //++(pIndexer_->getIndexWriter()->getBarrelInfo()->nNumDocs);
                addBinlog(docidList[i], (*iter));
                i++;
            }
            pIndexer_->setDirty();
            pIndexer_->getIndexReader();
        }
    }
}

void FieldIndexer::setIndexMode(
    boost::shared_ptr<MemCache> pMemCache,
    size_t nBatchMemSize,
    bool realtime)
{
    if(!realtime)
    {
        pMemCache_.reset(new MemCache(
                             pIndexer_->getIndexManagerConfig()->mergeStrategy_.memPoolSizeForPostingMerger_));
        setHitBuffer_(nBatchMemSize);
    }
    else
    {
        hits_.reset();
        iHitsMax_ = 0;
        pHits_ = pHitsMax_ = 0;
        pMemCache_ = pMemCache;
    }
    reset();
}

bool FieldIndexer::isEmpty()
{
    if (pIndexer_->isRealTime())
        return postingMap_.size() == 0;
    else
        return isBatchEmpty_();
}

bool FieldIndexer::isBatchEmpty_()
{
    if(hits_.size() == 0) return true;

    if(recordCount_ == 0)
    {
        int iHits = pHits_ - hits_;
        if(iHits == 0) return true;
    }
    return false;
}

void FieldIndexer::setHitBuffer_(size_t size)
{
    iHitsMax_ = size;
    iHitsMax_ = iHitsMax_/sizeof(TermId) ;
    hits_.reset();
    hits_.assign(iHitsMax_);
    pHits_ = hits_;
    pHitsMax_ = hits_ + iHitsMax_;
}

/***********************************
*  Format within single group for merged sort
*  uint32 size
*  uint32 number
*  uint64 nextstart (file offset to location of next group)
*  sorted records
************************************/
void FieldIndexer::writeHitBuffer_(int iHits)
{
    recordCount_ += iHits;
    bufferSort ( &hits_[0], iHits, CmpTermId_fn() );

    uint32_t output_buf_size = iHits * (sizeof(TermId)+sizeof(uint8_t));
    ///buffer size
    IASSERT(fwrite(&output_buf_size, sizeof(uint32_t), 1, f_) == 1);
    ///number of hits
    IASSERT(fwrite(&iHits, sizeof(uint32_t), 1, f_) == 1);

    uint64_t nextStart = 0;
    uint64_t nextStartPos = ftell(f_);
    ///next start
    IASSERT(fwrite(&nextStart, sizeof(uint64_t), 1, f_) == 1);
    FieldIndexIO ioStream(f_, "w");
    ioStream.writeRecord(&hits_[0], iHits * sizeof(TermId));
    nextStart = ftell(f_);

    ///update next start
    fseek(f_, nextStartPos, SEEK_SET);
    IASSERT(fwrite(&nextStart, sizeof(uint64_t), 1, f_) == 1);
    fseek(f_, nextStart, SEEK_SET);

    ++run_num_;
}

void FieldIndexer::addBinlog(
    docid_t docid,
    boost::shared_ptr<LAInput> laInput)
{
    boost::shared_ptr<RTPostingWriter> curPosting;
    for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->termid_);
        if (postingIter == postingMap_.end())
        {
            curPosting.reset(new RTPostingWriter(pMemCache_, skipInterval_, maxSkipLevel_, indexLevel_));
            boost::unique_lock<boost::shared_mutex> lock(rwLock_);
            postingMap_[iter->termid_] = curPosting;
        }
        else
            curPosting = postingIter->second;
        curPosting->add(docid, iter->wordOffset_, true);
    }
}

void FieldIndexer::addField(
    docid_t docid,
    boost::shared_ptr<LAInput> laInput)
{
    if(laInput->empty()) return;
    if (pIndexer_->isRealTime())
    {
        /**
        @breif add binlog
        */
        ofstream oBinFile;
        oBinFile.open(BinlogPath_.c_str(), ios::binary | ios::app);//xxx
        if(oBinFile.is_open())
        {
            for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
            {
                oBinFile.write(reinterpret_cast<char*>(&*iter),sizeof(TermId));
            }
        }
        else
            cout<<"Binlog Path Wrong"<<endl;

        boost::shared_ptr<RTPostingWriter> curPosting;
        for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
        {
            InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->termid_);
            if (postingIter == postingMap_.end())
            {
                curPosting.reset(new RTPostingWriter(pMemCache_, skipInterval_, maxSkipLevel_, indexLevel_));
                boost::unique_lock<boost::shared_mutex> lock(rwLock_);
                postingMap_[iter->termid_] = curPosting;
            }
            else
                curPosting = postingIter->second;
            curPosting->add(docid, iter->wordOffset_, true);
        }
    }
    else
    {
        if(flush_)
        {
            hits_.assign(iHitsMax_);
            pHits_ = hits_;
            pHitsMax_ = hits_ + iHitsMax_;
            flush_ = false;
        }

        int iDocHits = laInput->size();
        TermId * pDocHits = (TermId*)&(* laInput->begin());

        while( iDocHits > 0)
        {
            int iToCopy = iDocHits < (pHitsMax_ - pHits_) ? iDocHits: (pHitsMax_ - pHits_);
            memcpy(pHits_, pDocHits, iToCopy*sizeof(TermId));
            pHits_ += iToCopy;
            pDocHits += iToCopy;
            iDocHits -= iToCopy;

            if (pHits_ < pHitsMax_)
                continue;

            int iHits = pHits_ - hits_;
            writeHitBuffer_(iHits);
            pHits_ = hits_;
        }
    }
}

void FieldIndexer::reset()
{
    termCount_ = 0;

    if (! pIndexer_->isRealTime())
    {
        f_ = fopen(sorterFullPath_.c_str(),"w");
        uint64_t count = 0;
        IASSERT(fwrite(&count, sizeof(uint64_t), 1, f_) == 1);
    }
}

fileoffset_t FieldIndexer::write(OutputDescriptor* pWriterDesc)
{
    vocFilePointer_ = pWriterDesc->getVocOutput()->getFilePointer();

    IndexOutput* pVocWriter = pWriterDesc->getVocOutput();

    termid_t tid;
    fileoffset_t vocOffset = pVocWriter->getFilePointer();
    TermInfo termInfo;

    if (pIndexer_->isRealTime())
    {
        InMemoryPostingMap::iterator iter = postingMap_.begin();
        for (; iter !=postingMap_.end(); ++iter)
        {
            if (!iter->second->isEmpty())
            {
                tid = iter->first;
                iter->second->write(pWriterDesc, termInfo);		///write posting data
                writeTermInfo(pVocWriter,tid,termInfo);
                //Don't need reset anymore
                //iter->second->reset();						///clear posting data
                termInfo.reset();
                termCount_++;
            }
        }
        boost::unique_lock<boost::shared_mutex> lock(rwLock_);
        postingMap_.clear();
    }
    else
    {
        if( !boost::filesystem::exists(sorterFullPath_) || isBatchEmpty_())
        {
            fileoffset_t vocDescOffset = pVocWriter->getFilePointer();
            int64_t vocLength = vocDescOffset - vocOffset;

            pVocWriter->writeInt(TermInfo::version);///write terminfo version
            pVocWriter->writeInt(vocLength);	///<VocLength(Int64)>
            pVocWriter->writeLong(termCount_);	///<TermCount(Int64)>
            ///end write vocabulary descriptor

            return vocDescOffset;
        }
        int iHits = pHits_ - hits_;
        if(iHits > 0)
        {
            writeHitBuffer_(iHits);
        }
        fseek(f_, 0, SEEK_SET);
        IASSERT(fwrite(&recordCount_, sizeof(uint64_t), 1, f_) == 1);
        fclose(f_);
        f_ = NULL;
        hits_.reset();

        typedef izenelib::am::SortMerger<uint32_t, uint8_t, true,SortIO<FieldIndexIO> > merge_t;

        struct timeval tvafter, tvpre;
        struct timezone tz;
        gettimeofday (&tvpre , &tz);

        merge_t* merger = new merge_t(sorterFullPath_.c_str(), run_num_, 100000000, 2);
        merger->run();

        gettimeofday (&tvafter , &tz);
        LOG(INFO) << "It takes " << ((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000
                  << " minutes to sort(" << recordCount_ << ")";
        delete merger;
        recordCount_ = 0;
        run_num_ = 0;
        flush_ = true;

        FILE* f = fopen(sorterFullPath_.c_str(),"r");
        uint64_t count;

        FieldIndexIO ioStream(f);
        Record r;
        ioStream._readBytes((char*)(&count),sizeof(uint64_t));

        PostingWriter* pPosting = NULL;
        switch(pIndexer_->getIndexCompressType())
        {
        case BYTEALIGN:
            pPosting = new RTPostingWriter(pMemCache_, skipInterval_, maxSkipLevel_, indexLevel_);
            break;
        case BLOCK:
            pPosting = new BlockPostingWriter(pMemCache_, indexLevel_);
            break;
        case CHUNK:
            pPosting = new ChunkPostingWriter(pMemCache_, skipInterval_, maxSkipLevel_, indexLevel_);
            break;
        default:
            assert(false);
        }

        termid_t lastTerm = BAD_DOCID;
        try
        {
            for (uint64_t i = 0; i < count; ++i)
            {
                ioStream._read((char *)(&r), 13);
                if(r.tid != lastTerm && lastTerm != BAD_DOCID)
                {
                    if (!pPosting->isEmpty())
                    {
                        pPosting->write(pWriterDesc, termInfo); 	///write posting data
                        writeTermInfo(pVocWriter, lastTerm, termInfo);
                        pPosting->reset();								///clear posting data
                        pMemCache_->flushMem();
                        termInfo.reset();
                        termCount_++;
                    }
                }
                pPosting->add(r.docId, r.offset);
                lastTerm = r.tid;
            }
        }
        catch(std::exception& e)
        {
            LOG(WARNING) << e.what();
        }

        if (!pPosting->isEmpty())
        {
            pPosting->write(pWriterDesc, termInfo);	///write posting data
            writeTermInfo(pVocWriter, lastTerm, termInfo);
            pPosting->reset(); 							///clear posting data
            pMemCache_->flushMem();
            termInfo.reset();
            termCount_++;
        }
        fclose(f);
        boost::filesystem::remove(sorterFullPath_);
        delete pPosting;
    }

    fileoffset_t vocDescOffset = pVocWriter->getFilePointer();
    int64_t vocLength = vocDescOffset - vocOffset;
    cout<<"Write voclength:"<<vocLength<<endl;
    ///begin write vocabulary descriptor
    pVocWriter->writeInt(TermInfo::version);///write terminfo version
    pVocWriter->writeInt((int32_t)vocLength);	///<VocLength(Int64)>
    pVocWriter->writeLong(termCount_);	///<TermCount(Int64)>
    ///end write vocabulary descriptor

    return vocDescOffset;
}

TermReader* FieldIndexer::termReader()
{
    return new MemTermReader(getField(),this);
}

}

NS_IZENELIB_IR_END
