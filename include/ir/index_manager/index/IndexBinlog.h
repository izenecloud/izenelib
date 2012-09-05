/**
* @file        IndexBinlog.h
* @author     Hongliang Zhao
* @brief   add binlog to index
*/
#ifndef INDEXBINLOG_H
#define INDEXBINLOG_H

#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/index/Indexer.h>
#include <util/cronexpression.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace izenelib::ir::indexmanager;
NS_IZENELIB_IR_BEGIN
namespace indexmanager{

class Indexer;
class Binlog
{
public:
    Binlog(Indexer* pIndex);

    ~Binlog();

    bool openForRead(std::string binlogPath);

    void load_Binlog(vector<boost::shared_ptr<LAInput> >&laInputArray, vector<uint32_t> &docidList, string file);

    void load_BinlogFile(vector<boost::shared_ptr<LAInput> >&laInputArray, vector<uint32_t> &docidList, string file);

private:

    Indexer* pIndexer_;
};

}
NS_IZENELIB_IR_END
#endif
