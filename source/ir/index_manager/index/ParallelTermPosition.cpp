#include <ir/index_manager/index/ParallelTermPosition.h>

#include <algorithm>

using namespace izenelib::ir::indexmanager;

ParallelTermPosition::ParallelTermPosition(collectionid_t colID, IndexReader* pIndexReader, vector<string>& properties)
        :colID_(colID)
        ,pIndexReader_(pIndexReader)
        ,properties_(properties)
{
}

ParallelTermPosition::~ParallelTermPosition()
{
    for (map<string, TermPositions*>::iterator iter = termPositionMap_.begin(); iter != termPositionMap_.end(); ++iter)
        delete iter->second;
    termPositionMap_.clear();
    for (map<string, TermReader*>::iterator iter = termReaderMap_.begin(); iter != termReaderMap_.end(); ++iter)
        delete iter->second;
    termReaderMap_.clear();
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
            termReaderMap_.insert(make_pair((*iter), pTermReader));
            termPositionMap_.insert(make_pair((*iter), pPositions));
            flagMap_.insert(make_pair((*iter), true));
        }
        else
            delete pTermReader;
    }
    return find;
}

bool ParallelTermPosition::next(vector<string>& properties, docid_t& docid)
{
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
	cout<<"docid!!! "<<docid<<endl;

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
