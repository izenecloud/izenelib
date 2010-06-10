#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/utility/XML.h>

#include <sstream>
#include <fstream>
#include <iostream>


using namespace izenelib::ir::indexmanager;

void BarrelInfo::remove(Directory* pDirectory)
{
    try
    {
        pDirectory->deleteFiles(barrelName);
    }
    catch (IndexManagerException& fe)
    {
        SF1V5_RETHROW(fe);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_FILEIO,"BarrelInfo::remove():remove failed.");
    }
}

void BarrelInfo::rename(Directory* pDirectory,const string& newName)
{
    try
    {
        if (barrelName == newName)
            return ;
        if (pBarrelWriter)///in-memory index barrel
        {
            pBarrelWriter->rename(newName.c_str());
        }
        else
        {
            pDirectory->renameFiles(barrelName,newName);
        }
        barrelName = newName;
        modified = true;
    }
    catch (IndexManagerException& fe)
    {
        SF1V5_RETHROW(fe);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_FILEIO,"BarrelInfo::rename():rename failed.");
    }
}

//////////////////////////////////////////////////////////////////////////
//
BarrelsInfo::BarrelsInfo()
        :lock(false) 
        ,version(SF1_VERSION)
        ,nBarrelCounter(0)
        ,maxDoc(0)
{
}

BarrelsInfo::~BarrelsInfo(void)
{
    clear();
}

int32_t BarrelsInfo::getBarrelCount()
{
    return (int32_t)barrelInfos.size();
}

void BarrelsInfo::setVersion(const char* ver)
{
    version = ver;
}

void BarrelsInfo::clear()
{
    boost::mutex::scoped_lock lock(mutex_);

    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        delete *iter;
        iter++;
    }
    barrelInfos.clear();
    nBarrelCounter = 0;
}

void BarrelsInfo::updateMaxDoc(docid_t docId)
{
    boost::mutex::scoped_lock lock(mutex_);
    maxDoc = (docId>maxDoc)?docId:maxDoc;
}

void BarrelsInfo::read(Directory* pDirectory, const char* name)
{
    clear();
    boost::mutex::scoped_lock lock(mutex_);

    if (pDirectory->fileExists(name))
    {
        XMLElement* pDatabase = NULL;
        try
        {
            IndexInput* pIndexInput = pDirectory->openInput(name);
            string str;
            str.resize((size_t)pIndexInput->length());
            pIndexInput->read((char*)str.c_str(),str.length());
            delete pIndexInput;

            pDatabase = XMLElement::fromString(str.c_str(),true);
            if (!pDatabase) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///get <version></version> element
            XMLElement* pItem = pDatabase->getElementByName("version");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            version = pItem->getValue();

            ///get <barrel_counter></barrel_counter> element
            pItem = pDatabase->getElementByName("barrel_counter");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            nBarrelCounter = atoi(pItem->getValue().c_str());

            ///get <barrel_count></barrel_count> element
            pItem = pDatabase->getElementByName("barrel_count");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///get <maxdoc></maxdoc> element
            pItem = pDatabase->getElementByName("maxdoc");
            if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
            maxDoc = atoi(pItem->getValue().c_str());

            XMLElement* pBarrelsItem = pDatabase->getElementByName("barrels");
            if (!pBarrelsItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

            ///read barrel information
            BarrelInfo* pBarrelInfo = NULL;
            ElementIterator  iter = pBarrelsItem->getElementIterator();
            XMLElement*	pBarrelItem;
            while (pBarrelsItem->hasNextElement(iter))
            {
                pBarrelItem = pBarrelsItem->getNextElement(iter);
                pBarrelInfo = new BarrelInfo();

                ///get <name></name> element
                pItem = pBarrelItem->getElementByName("name");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->barrelName = pItem->getValue();

                ///get <doc_begin></doc_begin> element
                pItem = pBarrelItem->getElementByName("doc_begin");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");

                vector<string>ps = izenelib::ir::indexmanager::split(pItem->getValue(),",");
                for (vector<string>::iterator it = ps.begin(); it != ps.end(); it++)
                {
                    if ((*it).empty())
                        continue;
                    vector<string>pair = izenelib::ir::indexmanager::split((*it),":");
                    assert(pair.size() == 2);
                    pBarrelInfo->baseDocIDMap.insert(make_pair(atoi(pair[0].c_str()),atoi(pair[1].c_str())));
                }

                ///get <doc_count></doc_count> element
                pItem = pBarrelItem->getElementByName("doc_count");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->nNumDocs = atoi(pItem->getValue().c_str());

                ///get <max_doc></max_doc> element
                pItem = pBarrelItem->getElementByName("max_doc");
                if (!pItem) SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
                pBarrelInfo->maxDocId = atoi(pItem->getValue().c_str());

                ///get <is_update></is_update> element
                pItem = pBarrelItem->getElementByName("is_update");
                if (!pItem) pBarrelInfo->isUpdate = false;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->isUpdate = true;
                    else
                        pBarrelInfo->isUpdate = false;
                }

                barrelInfos.push_back(pBarrelInfo);
            }
            delete pDatabase;
            pDatabase = NULL;
        }
        catch (IndexManagerException& e)
        {
            if (pDatabase)
                delete pDatabase;
            SF1V5_RETHROW(e);
        }
        catch (...)
        {
            SF1V5_THROW(ERROR_INDEX_COLLAPSE,"index collapsed.");
        }
    }
}
void BarrelsInfo::write(Directory* pDirectory)
{
    boost::mutex::scoped_lock lock(mutex_);

    string str;
    XMLElement* pDatabase = new XMLElement(NULL,"database");

    ///add version element
    XMLElement* pItem = pDatabase->addElement("version");
    pItem->setValue(version.c_str());

    ///add barrel counter element
    str = izenelib::ir::indexmanager::append(str,nBarrelCounter);
    pItem = pDatabase->addElement("barrel_counter");
    pItem->setValue(str.c_str());

    ///add barrel count element
    str = "";
    str = izenelib::ir::indexmanager::append(str,(int32_t)barrelInfos.size());
    pItem = pDatabase->addElement("barrel_count");
    pItem->setValue(str.c_str());

    char buffer[10];
    pItem = pDatabase->addElement("maxdoc");
    sprintf(buffer,"%d",maxDoc);
    pItem->setValue(buffer);

    ///add barrels elements
    XMLElement* pBarrelsItem = pDatabase->addElement("barrels");
    XMLElement* pBarrelItem = NULL;
    BarrelInfo* pBarrelInfo = NULL;
    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        pBarrelInfo = *iter;
        pBarrelItem = pBarrelsItem->addElement("barrel");
        ///add <name></name> element
        pItem = pBarrelItem->addElement("name");
        pItem->setValue(pBarrelInfo->barrelName.c_str());
        ///add <doc_begin></doc_begin>
        pItem = pBarrelItem->addElement("doc_begin");
        str = "";
        //colid:baseDocid,colid:baseDocid,...
        for (map<collectionid_t,docid_t>::iterator it = pBarrelInfo->baseDocIDMap.begin(); it != pBarrelInfo->baseDocIDMap.end(); it ++)
        {
            //itoa(it->first, idstr, 10);
            ostringstream buffer1,buffer2;
            buffer1<<it->first;
            buffer2<<it->second;
            str.append(buffer1.str()).append(":").append(buffer2.str()).append(",");
        }
        pItem->setValue(str.c_str());
        ///add <doc_count></doc_count>
        pItem = pBarrelItem->addElement("doc_count");
        str = "";
        str = izenelib::ir::indexmanager::append(str,pBarrelInfo->nNumDocs);
        pItem->setValue(str.c_str());
        ///add <max_doc></max_doc>
        pItem = pBarrelItem->addElement("max_doc");
        str = "";
        str = izenelib::ir::indexmanager::append(str,pBarrelInfo->maxDocId);
        pItem->setValue(str.c_str()); 
        ///add <is_update></is_update>
        pItem = pBarrelItem->addElement("is_update");
        str = pBarrelInfo->isUpdate ? "yes":"no";
        pItem->setValue(str.c_str()); 

        iter ++;
    }

    IndexOutput* pIndexOutput = pDirectory->createOutput(BARRELS_INFONAME);
    str = pDatabase->toString(true,true);
    pIndexOutput->write((const char*)str.c_str(),str.length());
    delete pIndexOutput;
    delete pDatabase;
}

void BarrelsInfo::remove(Directory* pDirectory)
{
    boost::mutex::scoped_lock lock(mutex_);

    try
    {
        BarrelInfo* pBInfo = NULL;

        vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
        while (iter != barrelInfos.end())
        {
            pBInfo = *iter;
            pBInfo->remove(pDirectory);
            delete pBInfo;
            iter++;
        }
        barrelInfos.clear();
        nBarrelCounter = 0;
        if (pDirectory->fileExists(BARRELS_INFONAME))
        {
            pDirectory->deleteFile(BARRELS_INFONAME,false);
        }

    }
    catch (IndexManagerException& fe)
    {
        SF1V5_RETHROW(fe);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_FILEIO,"BarrelsInfo::remove():remove failed.");
    }
}
void BarrelsInfo::removeBarrel(Directory* pDirectory,const string& barrelname)
{
    boost::mutex::scoped_lock lock(mutex_);

    BarrelInfo* pBInfo = NULL;

    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        pBInfo = *iter;
        if (pBInfo->getName() == barrelname)
        {
            pBInfo->remove(pDirectory);
            delete pBInfo;
            barrelInfos.erase(iter);
            break;
        }
        iter++;
    }
}
const string BarrelsInfo::newBarrel()
{
    boost::mutex::scoped_lock lock(mutex_);
    string sName = "_";
    sName = izenelib::ir::indexmanager::append(sName,nBarrelCounter);
    nBarrelCounter ++;
    return sName;
}
void BarrelsInfo::addBarrel(const char* name,count_t docCount)
{
    boost::mutex::scoped_lock lock(mutex_);
    BarrelInfo* barrelInfo = new BarrelInfo(name,docCount);
    barrelInfos.push_back(barrelInfo);
}
void BarrelsInfo::addBarrel(BarrelInfo* pBarrelInfo,bool bCopy)
{
    boost::mutex::scoped_lock lock(mutex_);
    BarrelInfo* barrelInfo ;
    if (bCopy)
        barrelInfo = new BarrelInfo(*pBarrelInfo);
    else barrelInfo = pBarrelInfo;
    barrelInfos.push_back(barrelInfo);
}

int32_t BarrelsInfo::getDocCount()
{
    int32_t count = 0;
    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        count += (*iter)->getDocCount();
        iter++;
    }
    return count;
}

BarrelInfo* BarrelsInfo::getBarrelInfo(const char* barrel)
{
    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        if (!strcasecmp((*iter)->getName().c_str(),barrel))
            return (*iter);
        iter++;
    }
    return NULL;
}
BarrelInfo* BarrelsInfo::getLastBarrel()
{
    if (barrelInfos.size() <= 0)
        return NULL;
    return barrelInfos[barrelInfos.size() - 1];
}
void BarrelsInfo::deleteLastBarrel()
{
    boost::mutex::scoped_lock lock(mutex_);

    if (barrelInfos.size() <= 0)
        return ;
    delete barrelInfos[barrelInfos.size() - 1];
    barrelInfos.pop_back();
}

void BarrelsInfo::sort(Directory* pDirectory)
{
    boost::mutex::scoped_lock lock(mutex_);

    BarrelInfo* pBaInfo;
    BarrelsInfo newBarrelsInfo;

    vector<BarrelInfo*>::iterator iter;
    if (barrelInfos.size() > 1)
    {
        ///sort barrels by base doc id and number of documents
        stable_sort(barrelInfos.begin(),barrelInfos.end(),BarrelInfo::greater);

        ///rename to a temp name
        string strPrefix = "tmp";
        iter = barrelInfos.begin();
        while (iter != barrelInfos.end())
        {
            pBaInfo = *iter;
            pBaInfo->rename(pDirectory,strPrefix + pBaInfo->getName());///update barrel name
            iter++;
        }
    }

    ///rename to final name
    iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        pBaInfo = *iter;
        string str = newBarrelsInfo.newBarrel();
        pBaInfo->rename(pDirectory,str);///update barrel name
        pBaInfo->setSearchable(true);
        iter++;
    }
}

void BarrelsInfo::wait_for_barrels_ready()
{
    boost::unique_lock<boost::mutex> waitlock(mutex_);
    while(lock)
    {
        modifyBarrelsEvent_.wait(waitlock);
    }
}

