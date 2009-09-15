#include <ir/index_manager/index/ParallelTermPosition.h>

//#define SF1_TIME_CHECK

#include <wiselib/profiler/ProfilerGroup.h>


#include <algorithm>

using namespace izenelib::ir::indexmanager;

ParallelTermPosition::ParallelTermPosition(collectionid_t colID, IndexReader* pIndexReader, vector<string>& properties)
        :colID_(colID)
        ,pIndexReader_(pIndexReader)
        ,properties_(properties)
        ,positionsQueue_(NULL)
{
}

ParallelTermPosition::~ParallelTermPosition()
{
    for(std::vector<ParallelTermPosition::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        delete (*iter);
    }

    if(positionsQueue_)
        delete positionsQueue_;
/*
for (map<string, TermPositions*>::iterator iter = termPositionMap_.begin(); iter != termPositionMap_.end(); ++iter)
	delete iter->second;
termPositionMap_.clear();
for (map<string, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
	delete iter->second;
termReaderMap_.clear();
*/
}

bool ParallelTermPosition::seek(termid_t termID)
{
    bool find = false;


    for (vector<string>::iterator iter = properties_.begin(); iter != properties_.end(); ++iter)
    {
        Term term((*iter).c_str(),termID);

        TermReader* pTermReader = pIndexReader_->getTermReader(colID_);

        bool localFind = pTermReader->seek(&term);

        find = find ||localFind;
        if (localFind)
        {
            TermPositions* pPositions = pTermReader->termPositions();
            TermPositionEntry* entry = new TermPositionEntry(*iter);
            entry->pTermReader = pTermReader;
            entry->pPositions = pPositions;
            positions_.push_back(entry);
        }
        else
            delete pTermReader;
    }
    return find;
}

count_t ParallelTermPosition::maxdf()
{
    count_t maxdf = 0;
    for(vector<TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        count_t df = (*iter)->pPositions->docFreq();
        if(df > maxdf)
            maxdf = df;
    }
    return maxdf;
}

count_t ParallelTermPosition::ctf()
{
    count_t ctf = 0;
    for(vector<TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
        ctf += (*iter)->pPositions->getCTF();
    return ctf;
}

void ParallelTermPosition::initQueue()
{
    if (positionsQueue_ || positions_.size()==0)
        return;
    positionsQueue_ = new TermPositionQueue(properties_.size());
    for(vector<TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        if ((*iter)->next())
            positionsQueue_->insert(*iter);
    }
}

bool ParallelTermPosition::next()
{
    if (positionsQueue_ == NULL)
    {
        initQueue();
        if (positionsQueue_ == NULL)
        {
            return false;
        }
        TermPositionEntry* top = positionsQueue_->top();
        if (top == NULL)
        {
            return false;
        }
		
        currDoc_ = top->pPositions->doc();
        for(vector<TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
            (*iter)->setCurrent(false);

        for(size_t i = 0; i < positionsQueue_->size(); ++i)
        {
            TermPositionEntry* pEntry = positionsQueue_->getAt(i);
            if(currDoc_ == pEntry->pPositions->doc())
                pEntry->setCurrent(true);
        }
        return true;
    }

    TermPositionEntry* top = positionsQueue_->top();

    while(top != NULL && top->isCurrent())
    {
        top->setCurrent(false);
        if (top->next())
            positionsQueue_->adjustTop();
        else positionsQueue_->pop();
        top = positionsQueue_->top();
    }

    if (top == NULL)
    {
        return false;
    }

    currDoc_ = top->pPositions->doc();

    currTf_ = 0;

    for(vector<TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
        if(top->pPositions->doc() == currDoc_)
            (*iter)->setCurrent(true);
/*

    while (top != NULL && currDoc_ == top->pPositions->doc())
    {
        top->setCurrent(true);
        if (top->next())
            positionsQueue_->adjustTop();
        else positionsQueue_->pop();
        top = positionsQueue_->top();
    }
*/	
    return true;
}

void ParallelTermPosition::getPositions(std::map<string, PropertyItem>& result)
{
    ParallelTermPosition::TermPositionEntry* pEntry;
    for(std::vector<ParallelTermPosition::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        pEntry = (*iter);
        if(pEntry->isCurrent())
        {
            PropertyItem item;
            TermPositions* pPositions = pEntry->pPositions;
            loc_t pos = pPositions->nextPosition();
            loc_t charpos = pPositions->nextPosition();
            while (pos != BAD_POSITION)
            {
                item.positions->push_back(pos);
                pos = pPositions->nextPosition();
                charpos = pPositions->nextPosition();
            }
            item.tf = pPositions->freq();
            item.doclen = pPositions->docLength();
            result[pEntry->property] = item;

            currTf_ += item.tf;
        }					
    }
}

void ParallelTermPosition::getPositions(std::map<string, PropertyItem2>& result)
{
    ParallelTermPosition::TermPositionEntry* pEntry;
    for(std::vector<ParallelTermPosition::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        pEntry = (*iter);
        if(pEntry->isCurrent())
        {
            PropertyItem2 item;
            TermPositions* pPositions = pEntry->pPositions;
            loc_t pos = pPositions->nextPosition();
            loc_t charpos = pPositions->nextPosition();
            while (pos != BAD_POSITION)
            {
                item.positions->push_back(make_pair(pos,charpos));
                pos = pPositions->nextPosition();
                charpos = pPositions->nextPosition();
            }
            item.tf = pPositions->freq();
            item.doclen = pPositions->docLength();
            result[pEntry->property] = item;

            currTf_ += item.tf;
        }					
    }
}

void ParallelTermPosition::get_df_and_ctf(termid_t termID, DocumentFrequencyInProperties& dfmap, CollectionTermFrequencyInProperties& ctfmap)
{
    ParallelTermPosition::TermPositionEntry* pEntry;

    for(std::vector<ParallelTermPosition::TermPositionEntry*>::iterator iter = positions_.begin(); iter != positions_.end(); ++iter)
    {
        pEntry = (*iter);
        TermPositions* pPositions = pEntry->pPositions;
        DocumentFrequencyInProperties::iterator df_iter = dfmap.insert(std::make_pair(pEntry->property, ID_FREQ_MAP_T())).first;
        df_iter->second.insert(make_pair(termID, (float)pPositions->docFreq()));
			
        CollectionTermFrequencyInProperties::iterator ctf_iter = ctfmap.insert(std::make_pair(pEntry->property, ID_FREQ_MAP_T())).first;
        ctf_iter->second.insert(make_pair(termID, (float)pPositions->getCTF()));
    }
}

/*

bool ParallelTermPosition::seek(termid_t termID)
{
    bool find = false;

    for (vector<string>::iterator iter = properties_.begin(); iter != properties_.end(); ++iter)
    {
        Term term((*iter).c_str(),termID);

        TermReader* pTermReader = pIndexReader_->getTermReader(colID_);

        bool localFind = pTermReader->seek(&term);

        find = find ||localFind;
        if (localFind)
        {
            TermPositions* pPositions = pTermReader->termPositions();
            termReaderMap_.insert(make_pair((*iter), pTermReader));
            termPositionMap_.insert(make_pair((*iter), pPositions));
            flagMap_.insert(make_pair((*iter), true));
        }
        else
            delete pTermReader;
    }
    return find;
}

void ParallelTermPosition::get_df_and_ctf(termid_t termID, DocumentFrequencyInProperties& dfmap, CollectionTermFrequencyInProperties& ctfmap)
{
    for (map<string, TermPositions*>::iterator iter = termPositionMap_.begin(); iter != termPositionMap_.end(); ++iter)
    {
        DocumentFrequencyInProperties::iterator df_iter = dfmap.insert(std::make_pair(iter->first, ID_FREQ_MAP_T())).first;
        df_iter->second.insert(make_pair(termID, (float)iter->second->docFreq()));
			
        CollectionTermFrequencyInProperties::iterator ctf_iter = ctfmap.insert(std::make_pair(iter->first, ID_FREQ_MAP_T())).first;
        ctf_iter->second.insert(make_pair(termID, (float)iter->second->getCTF()));
    }
}




bool ParallelTermPosition::next(vector<string>& properties, docid_t& docid)
{
CREATE_SCOPED_PROFILER ( nextdoc, "IndexManager", "next : get doc id");

    bool hasNext = false;
    docid_t minDocID = 0xFFFFFFFF;
    //flagMap_ is to indicate whether a certain property should be moved forward.
    for (map<string, bool>::iterator iter = flagMap_.begin(); iter != flagMap_.end(); ++iter)
    {
        if (iter->second)
        {
            TermPositions* pPosition = termPositionMap_[iter->first];
            bool local_hasNext = pPosition->next();
            hasNext = hasNext || local_hasNext;
            if (!local_hasNext)
            {
                flagMap_[iter->first] = false;
                currDocMap_[iter->first]= 0xFFFFFFFF;	///Add by Yingfeng 2009.08.27
                continue;
            }
            docid_t curr_docid = pPosition->doc();
            currDocMap_[iter->first] = curr_docid;
            iter->second = true;
            if (curr_docid < minDocID)
                minDocID = curr_docid;
        }
    }
    for (map<string, bool>::iterator iter = flagMap_.begin(); iter != flagMap_.end(); ++iter)
    {
        if (!iter->second)
        {
            if (currDocMap_[iter->first]<= minDocID)
            {
                flagMap_[iter->first] = true;
                minDocID = currDocMap_[iter->first];
            }
        }
    }
    if (!hasNext)
    {
        return false;
    }

    docid = minDocID;

    for (map<string, docid_t>::iterator iter = currDocMap_.begin(); iter != currDocMap_.end(); ++iter)
    {
        if (iter->second == minDocID)
        {
            properties.push_back(iter->first);
            flagMap_[iter->first] = true;
        }
        else
        {
            flagMap_[iter->first] = false;
        }
    }

    return true;
}

void ParallelTermPosition::getPositions(string& property, boost::shared_ptr<std::deque<unsigned int> >& positions, freq_t& tf, freq_t& doclen)
{
    TermPositions* pPositions =  termPositionMap_[property];
    loc_t pos = pPositions->nextPosition();
    loc_t charpos = pPositions->nextPosition();
    while (pos != BAD_POSITION)
    {
        positions->push_back(pos);
        pos = pPositions->nextPosition();
        charpos = pPositions->nextPosition();
    }
    tf = pPositions->freq();
    doclen = pPositions->docLength();
}

void ParallelTermPosition::getPositions(string& property, boost::shared_ptr<std::deque< std::pair<unsigned int,unsigned int> > >& positions, 
	freq_t& tf, freq_t& doclen)
{
    TermPositions* pPositions =  termPositionMap_[property];
    loc_t pos = pPositions->nextPosition();
    loc_t charpos = pPositions->nextPosition();
	
    while (pos != BAD_POSITION)
    {
        positions->push_back(make_pair(pos,charpos));
        pos = pPositions->nextPosition();
        charpos = pPositions->nextPosition();
    }
    tf = pPositions->freq();
    doclen = pPositions->docLength();
}

*/
