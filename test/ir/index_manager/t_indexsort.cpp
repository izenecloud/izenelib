#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <algorithm>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/store/FSIndexInput.h>

using namespace std;
using namespace boost;

using namespace izenelib::ir::indexmanager;

namespace bfs = boost::filesystem;

std::map<std::string, unsigned int> propertyMap;
boost::uniform_int<> termDistribution(0xFFFF, 0x7FFFFFFF) ;
boost::mt19937 engine ;
boost::variate_generator<boost::mt19937, boost::uniform_int<> > termrandom (engine, termDistribution);
boost::uniform_int<> offsetDistribution(0, 2000);
boost::variate_generator<boost::mt19937, boost::uniform_int<> > offsetrandom (engine, offsetDistribution);



std::string file;

izenelib::am::IzeneSort<uint32_t, uint8_t, true,SortIO<FieldIndexIO> >* sorter = NULL;

uint8_t convert(char* data, uint32_t termId, uint32_t docId, uint32_t offset)
{
    char* pData = data;
    memcpy(pData, &(termId), sizeof(termId));
    pData += sizeof(uint32_t);
    FieldIndexIO::addVInt(pData, docId);
    FieldIndexIO::addVInt(pData, offset);
    return pData - data;
}

void buildup()
{
    bfs::path path(bfs::path(".") /"Content.tmp");
    file = path.string();
    sorter = new izenelib::am::IzeneSort<uint32_t, uint8_t, true,SortIO<FieldIndexIO> >
      (file.c_str(), 10000000);


        char data[12];
        uint8_t len = 12;
    for(size_t i = 1; i <= 100; ++i)
    {
        for (size_t j = 0; j < 10; ++j)
        {
            len = convert(data,termrandom(),i,offsetrandom());
            sorter->add_data(len,data);
        }
    }
    sorter->sort();
}


void cleanup()
{
    sorter->clear_files();
    bfs::path indexPath(bfs::path(".") /"Content.tmp");
    boost::filesystem::remove_all(indexPath);
    if(sorter) delete sorter;
}

BOOST_AUTO_TEST_SUITE( t_index_sort )

BOOST_AUTO_TEST_CASE(sort)
{
    buildup();

    IndexInput* pSortedInput = new FSIndexInput(file.c_str());
    std::deque<uint32_t>	 terms;
    std::deque<std::deque<uint32_t> > termDocs;
    uint64_t c = pSortedInput->readLongBySmallEndian();
    uint32_t tid = 0;
    uint32_t lastTid = 0;
    uint32_t lastDoc = 0;
    uint32_t lastOffset = 0;
    //std::deque<uint32_t> termDoc;
    cout<<"count "<<c<<endl;
    for (uint64_t i = 0; i < c; ++i)
    {
        uint8_t len = pSortedInput->readByte();
        BOOST_CHECK(len == 12);
        tid = pSortedInput->readIntBySmallEndian();
        uint32_t docId = pSortedInput->readVInt();
        uint32_t offset = pSortedInput->readVInt();
        //termDoc.push_back(docId);
        if(tid != lastTid && lastTid != 0)
        {
            terms.push_back(tid);
            //termDocs.push_back(termDoc);
            //termDoc.clear();
        }
        //BOOST_CHECK(tid >= lastTid);
        lastTid = tid;
        lastDoc = docId;
        lastOffset = offset;
    }
    delete pSortedInput;
    cout<<"overall "<<terms.size()<<" terms"<<endl;
    std::deque<uint32_t> sortedTerms = terms;
    std::sort(sortedTerms.begin(), sortedTerms.end());
    BOOST_CHECK(sortedTerms.size() == terms.size());
    for(size_t i = 0; i < terms.size(); ++i)
        BOOST_CHECK(sortedTerms[i] == terms[i]);

    cleanup();
}


BOOST_AUTO_TEST_SUITE_END()

