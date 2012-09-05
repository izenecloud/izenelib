#include <ir/index_manager/index/IndexBinlog.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/SortHelper.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>
#include <ir/index_manager/index/LAInput.h>

#include <vector>
#include <map>
#include <list>
#include <fstream>
#include <am/external_sort/izene_sort.hpp>
#include <string>
#include <deque>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace izenelib::ir::indexmanager;
namespace bfs = boost::filesystem;

Binlog::Binlog(Indexer* pIndex)
    :pIndexer_(pIndex)
{
}

Binlog::~Binlog()
{
}

bool Binlog::openForRead(std::string binlogPath)
{
    if(boost::filesystem::exists(binlogPath))
    {
        return true;
    }
    else
        return false;
}

void Binlog::load_Binlog(vector<boost::shared_ptr<LAInput> >&laInputArray, vector<uint32_t> &docidList, string file)//
{
    load_BinlogFile(laInputArray, docidList, file);
}

void Binlog::load_BinlogFile(vector<boost::shared_ptr<LAInput> >&laInputArray, vector<uint32_t> &docidList, string file)
{
    ifstream iBinFile;
    iBinFile.open(file.c_str(),ios::binary);
    unsigned int docid = -1;
    TermId term;
    boost::shared_ptr<LAInput> tmpLainput;
    iBinFile.read(reinterpret_cast<char*>(&term),sizeof(TermId));
    while(!iBinFile.eof())
    {
	if(term.docId_ != docid)
        {
            docidList.push_back(term.docId_);
            boost::shared_ptr<LAInput> laInput(new LAInput);
            laInputArray.push_back(laInput);
            tmpLainput = laInput;
            tmpLainput->push_back(term);
            docid = term.docId_;
        }
	else
        {
            tmpLainput->push_back(term);
     	}
	iBinFile.read(reinterpret_cast<char*>(&term),sizeof(TermId));	
    }
}

