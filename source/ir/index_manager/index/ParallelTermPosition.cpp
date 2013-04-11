#include <ir/index_manager/index/ParallelTermPosition.h>

//#define SF1_TIME_CHECK

#include <util/profiler/ProfilerGroup.h>


#include <algorithm>

using namespace izenelib::ir::indexmanager;

ParallelTermPosition::ParallelTermPosition(collectionid_t colID, IndexReader* pIndexReader, const std::vector<std::string>& properties)
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
            while (pos != BAD_POSITION)
            {
                item.positions->push_back(pos);
                pos = pPositions->nextPosition();
            }
            item.tf = pPositions->freq();
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

