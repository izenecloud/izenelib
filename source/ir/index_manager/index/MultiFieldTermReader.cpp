#include <ir/index_manager/index/MultiFieldTermReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/FieldInfo.h>


using namespace izenelib::ir::indexmanager;

MultiFieldTermReader::MultiFieldTermReader(Directory* pDirectory,const char* barrelname,FieldsInfo* pFieldsInfo)
        : TermReader()
        , pCurReader(NULL)
{
    open(pDirectory,barrelname,pFieldsInfo);
}

MultiFieldTermReader::MultiFieldTermReader()
        : TermReader()
        , pCurReader(NULL)
{
}

MultiFieldTermReader::~MultiFieldTermReader()
{
    close();
}

void MultiFieldTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{}

void MultiFieldTermReader::reopen()
{
    for(reader_map::iterator iter = fieldsTermReaders.begin(); 
            iter != fieldsTermReaders.end(); ++iter)
        iter->second->reopen();
}

TermReader* MultiFieldTermReader::termReader(const char* field)
{
    reader_map::iterator iter = fieldsTermReaders.find(field);
    if (iter != fieldsTermReaders.end())
    {
        return iter->second;//->clone();
    }
    return NULL;
}


TermIterator* MultiFieldTermReader::termIterator(const char* field)
{
    map<string,TermReader*>::iterator iter = fieldsTermReaders.find(field);
    if (iter != fieldsTermReaders.end())
        return iter->second->termIterator(field);
    return NULL;
}

bool MultiFieldTermReader::seek( Term* term)
{
    string field = term->getField();
    map<string,TermReader*>::iterator iter = fieldsTermReaders.find(field);
    if (iter != fieldsTermReaders.end())
    {
        pCurReader = iter->second;
        return pCurReader->seek(term);
    }
    return false;
}

TermDocFreqs* MultiFieldTermReader::termDocFreqs()
{
    return pCurReader->termDocFreqs();
}

TermPositions* MultiFieldTermReader::termPositions()
{
    return pCurReader->termPositions();
}


freq_t MultiFieldTermReader::docFreq(Term* term)
{
    return pCurReader->docFreq(term);
}

TermInfo* MultiFieldTermReader::termInfo(Term* term)
{
    return pCurReader->termInfo(term);
}

void MultiFieldTermReader::close()
{
    pCurReader = NULL;;

    reader_map::iterator iter = fieldsTermReaders.begin();
    while (iter != fieldsTermReaders.end())
    {
        delete iter->second;
        iter++;
    }
    fieldsTermReaders.clear();
}


TermReader* MultiFieldTermReader::clone()
{
    MultiFieldTermReader* pReader = new MultiFieldTermReader();

    reader_map::iterator iter = fieldsTermReaders.begin();
    while (iter != fieldsTermReaders.end())
    {
        pReader->fieldsTermReaders.insert(make_pair(iter->first,iter->second->clone()));
        iter++;
    }
    return pReader;
}

void MultiFieldTermReader::open(Directory* pDirectory,const char* barrelname,FieldsInfo* pFieldsInfo)
{
    FieldInfo* pInfo = NULL;
    TermReader* pTermReader = NULL;
    pFieldsInfo->startIterator();
    while (pFieldsInfo->hasNext())
    {
        pInfo = pFieldsInfo->next();

        if (pInfo->isIndexed()&&pInfo->isForward())
        {
            pTermReader = new DiskTermReader(pDirectory,barrelname,pInfo);
            fieldsTermReaders.insert(pair<string,TermReader*>(pFieldsInfo->getFieldName(pInfo->getID()),pTermReader));
        }
    }
}

void MultiFieldTermReader::addTermReader(const char* field,TermReader* pTermReader)
{
    string sf = field;
    fieldsTermReaders.insert(make_pair(sf,pTermReader));
}

