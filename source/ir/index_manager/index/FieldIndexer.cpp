#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/TermPositions.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

FieldIndexer::FieldIndexer(MemCache* pCache):pMemCache_(pCache),vocFilePointer_(0)
{
}

FieldIndexer::~FieldIndexer()
{
    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        delete iter->second;
    }

    pMemCache_ = NULL;
}

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<LAInput> laInput)
{
    InMemoryPosting* curPosting;

    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->termId_);
        if(postingIter == postingMap_.end())
        {
            curPosting = new InMemoryPosting(pMemCache_);
            postingMap_[iter->termId_] = curPosting;
        }
        else
            curPosting = postingIter->second;

        curPosting->addLocation(docid, iter->offset_);
    }
}

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<ForwardIndex> forwardindex)
{
    InMemoryPosting* curPosting;

    for(ForwardIndex::iterator iter = forwardindex->begin(); iter != forwardindex->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->first);
        if(postingIter == postingMap_.end())
        {
            curPosting = new InMemoryPosting(pMemCache_);
            postingMap_[iter->first] = curPosting;
        }
        else
            curPosting = postingIter->second;

        ForwardIndexOffset::iterator	endit = iter->second->end();
        for(ForwardIndexOffset::iterator it = iter->second->begin(); it != endit; ++it)
            curPosting->addLocation(docid, *it);
    }
}

void FieldIndexer::removeField(docid_t docid, boost::shared_ptr<LAInput> laInput)
{
    docid_t decompressed_docid;

    InMemoryPosting* newPosting;

    InMemoryTermReader* pTermReader = new InMemoryTermReader(getField(),this);

    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        termid_t termId = (termid_t)iter->termId_;

        Term term(getField(), termId);
        if (pTermReader->seek(&term))
        {
            newPosting = new InMemoryPosting(pMemCache_);
            TermPositions* pTermPositions = pTermReader->termPositions();
            while (pTermPositions->next())
            {
                decompressed_docid = pTermPositions->doc();
                loc_t pos = pTermPositions->nextPosition();
                while (pos != BAD_POSITION)
                {
                    if (decompressed_docid != docid)
                        newPosting->addLocation(decompressed_docid, pos);
                    pos = pTermPositions->nextPosition();
                }
            }

            InMemoryPosting* curPosting =  (InMemoryPosting*)postingMap_[termId];
            delete curPosting;
            postingMap_[termId] = newPosting;
        }
    }

    delete pTermReader;

}

void FieldIndexer::removeField(docid_t docid, boost::shared_ptr<ForwardIndex> forwardindex)
{
    docid_t decompressed_docid;

    InMemoryPosting* newPosting;

    InMemoryTermReader* pTermReader = new InMemoryTermReader(getField(),this);

    for(ForwardIndex::iterator iter = forwardindex->begin(); iter != forwardindex->end(); ++iter)
    {
        termid_t termId = (termid_t)iter->first;

        Term term(getField(), termId);
        if (pTermReader->seek(&term))
        {
            newPosting = new InMemoryPosting(pMemCache_);
            TermPositions* pTermPositions = pTermReader->termPositions();
            while (pTermPositions->next())
            {
                decompressed_docid = pTermPositions->doc();
                loc_t pos = pTermPositions->nextPosition();
                while (pos != BAD_POSITION)
                {
                    if (decompressed_docid != docid)
                        newPosting->addLocation(decompressed_docid, pos);
                    pos = pTermPositions->nextPosition();
                }
            }

            InMemoryPosting* curPosting =  (InMemoryPosting*)postingMap_[termId];
            delete curPosting;
            postingMap_[termId] = newPosting;
        }
    }

    delete pTermReader;

}


void FieldIndexer::reset()
{
    InMemoryPosting* pPosting;
    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        pPosting = iter->second;
        if (!pPosting->hasNoChunk())
        {
            pPosting->reset();		///clear posting data
        }
    }
    postingMap_.clear();
}

fileoffset_t FieldIndexer::write(OutputDescriptor* pWriterDesc)
{
    vocFilePointer_ = pWriterDesc->getVocOutput()->getFilePointer();

    IndexOutput* pVocWriter = pWriterDesc->getVocOutput();

    fileoffset_t poffset;
    termid_t tid;
    termid_t lastTermId = 0;
    int32_t termCount = 0;
    InMemoryPosting* pPosting;
    fileoffset_t vocOffset = pVocWriter->getFilePointer();

    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        pPosting = iter->second;
        if (!pPosting->hasNoChunk())
        {
            tid = iter->first;
            pVocWriter->writeInt(tid);			///write term id

            pVocWriter->writeInt(pPosting->docFreq());		///write df

            poffset = pPosting->write(pWriterDesc);		///write posting data

            //pVocWriter->writeLong(poffset - lastPOffset);	///write offset of posting descriptor
            pVocWriter->writeLong(poffset);	///write offset of posting descriptor
            pPosting->reset();								///clear posting data

            lastTermId = tid;

            termCount++;
        }
        delete pPosting;
        pPosting = NULL;
    }
    postingMap_.clear();

    fileoffset_t vocDescOffset = pVocWriter->getFilePointer();
    int64_t vocLength = vocDescOffset - vocOffset;
    ///begin write vocabulary descriptor
    pVocWriter->writeLong(vocLength);	///<VocLength(Int64)>
    pVocWriter->writeLong(termCount);	///<TermCount(Int64)>
    ///end write vocabulary descriptor

    return vocDescOffset;
}

TermReader* FieldIndexer::termReader()
{
    return new InMemoryTermReader(getField(),this);
}

