#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/utility/XML.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

#include <boost/scoped_ptr.hpp>

using namespace izenelib::ir::indexmanager;

void BarrelInfo::remove(Directory* pDirectory)
{
    try
    {
        boost::mutex::scoped_lock lock(mutex_);
        isRemoved_ = true;
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
    if (barrelName == newName)
        return;

    try
    {
        boost::mutex::scoped_lock lock(mutex_);
        DVLOG(2) << "=> BarrelInfo::rename(), " << barrelName << " => " << newName << " ...";

        if (pBarrelWriter == NULL)/// not in-memory index barrel
            pDirectory->renameFiles(barrelName,newName);

        barrelName = newName;
        DVLOG(2) << "<= BarrelInfo::rename(), => " << barrelName;
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

void BarrelInfo::write(Directory* pDirectory)
{
    if(pBarrelWriter == NULL)
    {
        LOG(WARNING) << "barrel " << barrelName << " could not be written without a IndexBarrelWriter";
        return;
    }

    if(baseDocIDMap.empty())
    {
        LOG(WARNING) << "barrel " << barrelName << " could not be written without base doc id";
        return;
    }

    collectionid_t colID = baseDocIDMap.begin()->first;
    CollectionIndexer* pColIndexer = pBarrelWriter->getCollectionIndexer(colID);
    CollectionsInfo* pColInfo = pBarrelWriter->getCollectionsInfo();
    if(pColIndexer == NULL || pColInfo == NULL)
    {
        LOG(WARNING) << "barrel " << barrelName << " could not be written without CollectionIndexer or CollectionsInfo";
        return;
    }

    try
    {
        boost::mutex::scoped_lock lock(mutex_);
        LOG(INFO) << "=> BarrelInfo::write(), barrel name: " << barrelName
                 << ", doc range: [" << baseDocIDMap.begin()->second << ", " << maxDocId << "]"
                 << ", nNumDocs: " << nNumDocs << " ..."
                 << ", index level: " << indexLevel_;

        string s = barrelName +".voc";
        IndexOutput* pVocOutput = pDirectory->createOutput(s.c_str());

        s = barrelName + ".dfp";
        IndexOutput* pDOutput = pDirectory->createOutput(s.c_str());

        IndexOutput* pPOutput = NULL;
        if (indexLevel_ == WORDLEVEL)
        {
            s = barrelName + ".pop";
            pPOutput = pDirectory->createOutput(s.c_str());
        }

        OutputDescriptor desc(pVocOutput,pDOutput,pPOutput,true);
        // write ".bti" and "doclen.map"
        pColIndexer->write(&desc);
        // flush ".voc", ".dfp" and ".pop"
        desc.flush();

        s = barrelName + ".fdi";
        boost::scoped_ptr<IndexOutput> fdiOutputPtr(pDirectory->createOutput(s.c_str()));
        pColInfo->write(fdiOutputPtr.get());
        fdiOutputPtr->flush();

        // after written into disk, it would not be in-memory barrel
        setWriter(NULL);
        DVLOG(2) << "<= BarrelInfo::write(), barrel name: " << barrelName;
    }
    catch (const FileIOException& e)
    {
        SF1V5_RETHROW(e);
    }
    catch (std::bad_alloc& be)
    {
        string serror = be.what();
        SF1V5_THROW(ERROR_OUTOFMEM,"BarrelInfo::write():alloc memory failed.-" + serror);
    }
}

void BarrelInfo::registerIndexInput(IndexInput* pIndexInput)
{
    boost::mutex::scoped_lock lock(mutex_);
    indexInputs.insert(pIndexInput);
}

void BarrelInfo::unRegisterIndexInput(IndexInput* pIndexInput)
{
    boost::mutex::scoped_lock lock(mutex_);
    indexInputs.erase(pIndexInput);
}

void BarrelInfo::setDirty()
{
    boost::mutex::scoped_lock lock(mutex_);
    for(std::set<IndexInput*>::iterator iter = indexInputs.begin(); 
        iter != indexInputs.end(); ++iter)
        (*iter)->setDirty(true);
}

//////////////////////////////////////////////////////////////////////////
//
BarrelsInfo::BarrelsInfo(IndexLevel indexLevel)
        :version(SF1_VERSION)
        ,nBarrelCounter(0)
        ,maxDoc(0)
        ,indexLevel_(indexLevel)
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

    for(vector<BarrelInfo*>::iterator iter = rubbishBarrelInfos.begin(); 
                     iter != rubbishBarrelInfos.end(); ++iter)
        delete *iter;
    rubbishBarrelInfos.clear();

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
                pBarrelInfo = new BarrelInfo(indexLevel_);

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

                ///get <searchable></searchable> element
                pItem = pBarrelItem->getElementByName("searchable");
                if (!pItem) pBarrelInfo->searchable = true;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->searchable = true;
                    else
                        pBarrelInfo->searchable = false;
                }
                
                ///get <is_in_memory_barrel></is_in_memory_barrel> element
                pItem = pBarrelItem->getElementByName("is_in_memory_barrel");
                if (!pItem) pBarrelInfo->inMemoryBarrel = false;
                else
                {
                    if(pItem->getValue().compare("yes") == 0)
                        pBarrelInfo->inMemoryBarrel = true;
                    else
                        pBarrelInfo->inMemoryBarrel = false;
                }


                ///get <compress></compress> element
                pItem = pBarrelItem->getElementByName("compress");
                if (!pItem) pBarrelInfo->compressType = BYTEALIGN;
                else
                {
                    if(pItem->getValue().compare("byte") == 0)
                        pBarrelInfo->compressType = BYTEALIGN;
                    else if(pItem->getValue().compare("block") == 0)
                        pBarrelInfo->compressType = BLOCK;
                    else if(pItem->getValue().compare("chunk") == 0)
                        pBarrelInfo->compressType = CHUNK;
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
    DVLOG(2) << "=> BarrelsInfo::write(), barrel count: " << barrelInfos.size() << ", max doc: " << maxDoc << " ...";

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
        /* Change for barrel consistency.*/
        /*if(pBarrelInfo->getWriter())
        {
            DVLOG(2) << "ignore in-memory barrel " << pBarrelInfo->getName() << " in writing file \"barrels\"";
            iter ++;
            continue;
        }*/
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
        ///add <searchable></searchable>
        pItem = pBarrelItem->addElement("searchable");
        str = pBarrelInfo->searchable ? "yes":"no";
        pItem->setValue(str.c_str()); 
        ///add <is_inmemorybarrel></searchable>
        pItem = pBarrelItem->addElement("is_in_memory_barrel");
        if(pBarrelInfo->isRealTime() )
        {
            str = "yes";
        }
        else
        {
            str = "no";
        }
        pItem->setValue(str.c_str()); 

        ///add <compress></compress>
        pItem = pBarrelItem->addElement("compress");
        switch(pBarrelInfo->compressType)
        {
        case BYTEALIGN:
            str = "byte";
            break;
        case BLOCK:
            str = "block";
            break;
        case CHUNK:
            str = "chunk";
            break;
        default:
            assert(false);
         }
        pItem->setValue(str.c_str()); 

        iter ++;
    }

    IndexOutput* pIndexOutput = pDirectory->createOutput(BARRELS_INFONAME);
    str = pDatabase->toString(true,true);
    pIndexOutput->write((const char*)str.c_str(),str.length());
    delete pIndexOutput;
    delete pDatabase;
    DVLOG(2) << "<= BarrelsInfo::write()";
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
void BarrelsInfo::removeBarrel(Directory* pDirectory,const string& barrelname, bool bLock)
{
    boost::defer_lock_t defer_lock;
    boost::mutex::scoped_lock lock(mutex_, defer_lock);
    if(bLock)
        lock.lock();

    BarrelInfo* pBInfo = NULL;

    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        pBInfo = *iter;
        if (pBInfo->getName() == barrelname)
        {
            pBInfo->setDirty();
            pBInfo->remove(pDirectory);
            rubbishBarrelInfos.push_back(pBInfo);
            //delete pBInfo;
            barrelInfos.erase(iter);
            DVLOG(2) << "BarrelsInfo::removeBarrel(), barrel name: " << barrelname;
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

void BarrelsInfo::addBarrel(BarrelInfo* pBarrelInfo, bool bLock)
{
    boost::defer_lock_t defer_lock;
    boost::mutex::scoped_lock lock(mutex_, defer_lock);
    if(bLock)
        lock.lock();

    barrelInfos.push_back(pBarrelInfo);

    DVLOG(2) << "BarrelsInfo::addBarrel(), barrel name: " << pBarrelInfo->getName() << ", doc count: " << pBarrelInfo->getDocCount();
}

int32_t BarrelsInfo::getDocCount()
{
    boost::mutex::scoped_lock lock(mutex_);
    int32_t count = 0;
    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    while (iter != barrelInfos.end())
    {
        count += (*iter)->getDocCount();
        iter++;
    }
    return count;
}

void BarrelsInfo::sort(Directory* pDirectory)
{
    boost::mutex::scoped_lock lock(mutex_);

    DVLOG(2) << "=> BarrelsInfo::sort(), barrel count: " << barrelInfos.size() << ", max doc: " << maxDoc << " ...";

    BarrelInfo* pBaInfo;
    BarrelsInfo newBarrelsInfo(indexLevel_);

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
            pBaInfo->setDirty();
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
        pBaInfo->setDirty();
        pBaInfo->rename(pDirectory,str);///update barrel name
        iter++;
    }

    DVLOG(2) << "<= BarrelsInfo::sort(), barrel count: " << barrelInfos.size() << ", max doc: " << maxDoc;
}

void BarrelsInfo::setSearchable()
{
    boost::mutex::scoped_lock lock(mutex_);
    for(vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
            iter != barrelInfos.end();
            ++iter)
        (*iter)->setSearchable(true);
}

bool BarrelsInfo::deleteDocument(collectionid_t colId, docid_t docId)
{
    boost::mutex::scoped_lock lock(mutex_);

    for(vector<BarrelInfo*>::reverse_iterator rit = barrelInfos.rbegin();
            rit != barrelInfos.rend();
            ++rit)
    {
        BarrelInfo* pBarrelInfo = *rit;
        map<collectionid_t,docid_t>::const_iterator baseDocIt = pBarrelInfo->baseDocIDMap.find(colId);
        if(baseDocIt != pBarrelInfo->baseDocIDMap.end())
        {
            if(baseDocIt->second <= docId && pBarrelInfo->getMaxDocID() >= docId)
                return pBarrelInfo->deleteDocument(docId);
        }
    }

    LOG(INFO) << "BarrelsInfo::deleteDocument(), no BarrelInfo found for collection id: " << colId << ", doc id: " << docId;
    return false;
}

void BarrelsInfo::printBarrelsInfo(Directory* pDirectory)
{
    boost::mutex::scoped_lock lock(mutex_);

    vector<BarrelInfo*>::iterator iter = barrelInfos.begin();
    LOG(INFO)<<"Print barrels info begin";
    while (iter != barrelInfos.end())
    {
        BarrelInfo* pBarrelInfo = *iter;
        std::string directory = pDirectory->directory();
        std::string fileName = directory + pBarrelInfo->barrelName;
        if (boost::filesystem::exists(fileName + ".dfp"))
        {
            LOG(INFO)<<"Barrel name: "<<pBarrelInfo->barrelName << std::endl
                    <<" Document count: "<<pBarrelInfo->nNumDocs << std::endl
                    <<" Max document number: "<<pBarrelInfo->nNumDocs << std::endl
                    << " Base doc : " << pBarrelInfo->getBaseDocID() << std::endl
                    <<" dfp size: "<<boost::filesystem::file_size(fileName + ".dfp")
                    <<" voc size: "<<boost::filesystem::file_size(fileName + ".voc")
                    <<" fdi size: "<<boost::filesystem::file_size(fileName + ".fdi")
                    <<" pop size: "<<boost::filesystem::file_size(fileName + ".pop");
        }
        iter ++;
    }
    LOG(INFO)<<"Print barrels info end";
}
