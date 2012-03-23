/**
* @file       r_IndexStats.cpp
* @author     Jun
* @version    SF1 v5.0
* @brief Print statistics for index data, such as:
* - doc number
* - total term count
* - average doc length
* - unique term count (this value is invalid when multiple barrels exist)
* - df and ctf of each term
*/

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/AbsTermIterator.h>
#include <ir/id_manager/IDManager.h>
#include <util/ustring/UString.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm> // sort
#include <utility> // pair

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <boost/scoped_ptr.hpp>

using namespace std;
using namespace izenelib::ir::indexmanager;

#define HAS_UNIGRAM_INDEX
//#define PRINT_CONFIG

namespace
{

const unsigned int COLLECTION_ID = 1;
const char* CONFIG_OPTION = "-c";
const char* COLLECTION_NAME = "collection-name";
const char* INDEX_PATH = "index-path";
const char* ID_PATH = "id-path";
const char* DOCLEN_PROPS = "doclen-properties";
const char* STATS_PROPS = "stats-properties";

struct StatsConfig
{
    string collectionName_;
    string indexPath_;
    string idPath_;
    vector<string> docLenProps_;
    vector<string> statsProps_;

    bool load(const char* fileName);
};

bool StatsConfig::load(const char* fileName)
{
    ifstream ifs(fileName);
    if(! ifs)
    {
        cerr << "fail to open " << fileName << endl;
        return false;
    }

    map<string, vector<string> > configMap;
    string line;
    while(getline(ifs, line))
    {
        if(line.empty() || line[0] == '#')
            continue;

        istringstream iss(line);
        string left, mid, right;
        iss >> left >> mid;
        vector<string> rightVec;
        while(iss >> right)
            rightVec.push_back(right);

        if(mid != "=" || rightVec.empty())
        {
            cerr << "in loading " << fileName << ", invalid format in line: " << line << endl;
            return false;
        }

        configMap[left] = rightVec;
    }

    map<string, vector<string> >::const_iterator it = configMap.find(COLLECTION_NAME);
    if(it == configMap.end() || it->second.size() != 1)
    {
        cerr << "invalid config value for " << COLLECTION_NAME << endl;
        return false;
    }
    else
        collectionName_ = it->second.front();

    it = configMap.find(INDEX_PATH);
    if(it == configMap.end() || it->second.size() != 1)
    {
        cerr << "invalid config value for " << INDEX_PATH << endl;
        return false;
    }
    else
        indexPath_ = it->second.front();

    it = configMap.find(ID_PATH);
    if(it == configMap.end())
        idPath_.clear();
    else if (it->second.size() != 1)
    {
        cerr << "invalid config value for " << ID_PATH << endl;
        return false;
    }
    else
        idPath_ = it->second.front();

    it = configMap.find(DOCLEN_PROPS);
    if(it == configMap.end() || it->second.empty())
    {
        cerr << "no config value for " << DOCLEN_PROPS << endl;
        return false;
    }
    else
    {
#ifdef HAS_UNIGRAM_INDEX
        docLenProps_.clear();
        for(vector<string>::const_iterator sit=it->second.begin();
            sit!= it->second.end(); ++sit)
        {
            docLenProps_.push_back(*sit);
            docLenProps_.push_back(*sit + "_unigram");
        }
#else
        docLenProps_ = it->second;
#endif
    }

    it = configMap.find(STATS_PROPS);
    if(it == configMap.end())
    {
        cerr << "no config value for " << STATS_PROPS << endl;
        return false;
    }
    else
        statsProps_ = it->second;

#ifdef PRINT_CONFIG
    cout << "loaded config file " << fileName << endl;
    cout << COLLECTION_NAME << ": " << collectionName_ << endl;
    cout << INDEX_PATH << ": " << indexPath_ << endl;
    if(! idPath_.empty())
        cout << ID_PATH << ": " << idPath_ << endl;
    cout << DOCLEN_PROPS << ": ";
    for(vector<string>::const_iterator it=docLenProps_.begin(); it!=docLenProps_.end(); ++it)
        cout << *it << " ";
    cout << endl;
    cout << STATS_PROPS << ": ";
    for(vector<string>::const_iterator it=statsProps_.begin(); it!=statsProps_.end(); ++it)
        cout << *it << " ";
    cout << endl << endl;
#endif
    return true;
}

Indexer* createIndexer(const StatsConfig& statsConfig)
{
    Indexer* pIndexer = new Indexer;

    IndexManagerConfig indexManagerConfig;

    indexManagerConfig.indexStrategy_.indexLocation_ = statsConfig.indexPath_;
    indexManagerConfig.indexStrategy_.indexMode_ = "default";
    indexManagerConfig.indexStrategy_.memory_ = 30000000;
    indexManagerConfig.indexStrategy_.indexDocLength_ = true;
    indexManagerConfig.indexStrategy_.skipInterval_ = 3;
    indexManagerConfig.indexStrategy_.maxSkipLevel_ = 128;
    indexManagerConfig.mergeStrategy_.param_ = "default";
    indexManagerConfig.storeStrategy_.param_ = "file";

    vector<string> propertyList(statsConfig.docLenProps_);
    sort(propertyList.begin(),propertyList.end());

    IndexerCollectionMeta indexCollectionMeta;
    indexCollectionMeta.setName(statsConfig.collectionName_);
    for (std::size_t i=0;i<propertyList.size();i++)
    {
        IndexerPropertyConfig c(1+i, propertyList[i], true, true);
        c.setIsStoreDocLen(true);
        indexCollectionMeta.addPropertyConfig(c);
    }
    indexManagerConfig.addCollectionMeta(indexCollectionMeta);

    map<string, unsigned int> collectionIdMapping;
    collectionIdMapping.insert(pair<string, unsigned int>(statsConfig.collectionName_, COLLECTION_ID));

    pIndexer->setIndexManagerConfig(indexManagerConfig, collectionIdMapping);

    return pIndexer;
}

void printIndexStats(Indexer* pIndexer, const StatsConfig& statsConfig)
{
    cout << "collection name: " << statsConfig.collectionName_ << endl;

    IndexReader* pIndexReader = pIndexer->getIndexReader();
    cout << "doc number: " << pIndexReader->numDocs() << endl;

    const docid_t maxDocId = pIndexReader->maxDoc();

    for(vector<string>::const_iterator it = statsConfig.statsProps_.begin();
        it != statsConfig.statsProps_.end(); ++it)
    {
        cout << endl << "field name: " << *it << endl;

        int64_t docLenSum = 0;
        const fieldid_t fieldId = pIndexer->getPropertyIDByName(COLLECTION_ID, *it);
        for(docid_t id = 1; id <= maxDocId; ++id)
        {
            docLenSum += pIndexReader->docLength(id, fieldId);
        }
        cout << "total term count: " << docLenSum << endl;

        double avgDocLen = pIndexReader->getAveragePropertyLength(fieldId);
        cout << "average doc length: " << avgDocLen << endl;

        size_t termCount = pIndexReader->getDistinctNumTerms(COLLECTION_ID, *it);
        cout << "unique term count: " << termCount << endl;
    }
}

void printTerms(Indexer* pIndexer, const StatsConfig& statsConfig)
{
    cout << endl;

    IndexReader* pIndexReader = pIndexer->getIndexReader();
    boost::scoped_ptr<TermReader> pTermReader(pIndexReader->getTermReader(COLLECTION_ID));

    izenelib::ir::idmanager::IDManager* pIDManager = NULL;
    if(! statsConfig.idPath_.empty())
        pIDManager = new izenelib::ir::idmanager::IDManager(statsConfig.idPath_);

    for(vector<string>::const_iterator it = statsConfig.statsProps_.begin();
        it != statsConfig.statsProps_.end(); ++it)
    {
        string fileName = statsConfig.collectionName_ + "." + *it + ".terms";
        ofstream ofs(fileName.c_str());
        if(! ofs)
        {
            cerr << "fail to write " << fileName << endl;
            return;
        }

        boost::scoped_ptr<TermIterator> pTermIterator(pTermReader->termIterator(it->c_str()));
        Term term(it->c_str());
        while(pTermIterator->next())
        {
            const Term* pTerm = pTermIterator->term();
            const TermInfo* pTermInfo = pTermIterator->termInfo();

            ofs << "term id: " << pTerm->value
                << ", df: " << pTermInfo->docFreq_
                << ", ctf: " << pTermInfo->ctf_
                << ", mtf: " << pTermInfo->maxTF_
                << ", dfpLen: " << pTermInfo->docPostingLen_
                << ", popLen: " << pTermInfo->positionPostingLen_;

            if(pIDManager)
            {
                izenelib::util::UString termStr("abc", UString::UTF_8);
                if(pIDManager->getTermStringByTermId(pTerm->value, termStr))
                {
                    string utf8Str;
                    termStr.convertString(utf8Str, UString::UTF_8);
                    ofs << ", string: " << utf8Str;
                }
                else
                    ofs << ", no term string";
            }

            ofs << endl;
        }

        cout << "term statistics are written into " << fileName << endl;
    }

    delete pIDManager;
}

}

int main(int argc, char* argv[])
{
    if(argc < 3 || strcmp(argv[1], CONFIG_OPTION))
    {
        cerr << "Usage: " << argv[0] << " -c r_IndexStats.config" << endl;
        exit(1);
    }

    StatsConfig statsConfig;
    const char* configName = argv[2];
    if(! statsConfig.load(configName))
    {
        cerr << "load " << configName << " error!" << endl;
        exit(1);
    }

    Indexer* pIndexer = createIndexer(statsConfig);
    assert(pIndexer);

    printIndexStats(pIndexer, statsConfig);
    printTerms(pIndexer, statsConfig);

    delete pIndexer;

    return 0;
}
