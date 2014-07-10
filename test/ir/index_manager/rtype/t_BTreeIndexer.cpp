#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/rtype/BTreeIndexer.h>

#include "BTreeTestFramework.h"

using namespace izenelib::ir::indexmanager;


class DirController
{
    public:
        DirController(const std::string& dir)
        :dir_(dir)
        {
            boost::filesystem::remove_all(dir_);
            if( boost::filesystem::exists(dir_) )
            {
                throw std::runtime_error("dir not deleted");
            }
            boost::filesystem::create_directories(dir_);
        }

        ~DirController()
        {
            boost::filesystem::remove_all(dir_);
        }

        const std::string& path() const
        {
            return dir_;
        }

    private:
        std::string dir_;
};

class FileController
{
    public:
        FileController(const std::string& file)
        :file_(file)
        {
            boost::filesystem::remove_all(file_);
        }

        ~FileController()
        {
            boost::filesystem::remove_all(file_);
        }

        const std::string& path() const
        {
            return file_;
        }

    private:
        std::string file_;
};

void PrintBitset(const Bitset& docs)
{
    std::cout<<"final "<<docs<<std::endl;
}

void check_ustring(const std::string& s1, const std::string& s2, bool start, bool end, bool contains)
{
    std::cout<<"check_ustring "<<s1<<","<<s2<<std::endl;
    izenelib::util::UString u1(s1, izenelib::util::UString::UTF_8);
    izenelib::util::UString u2(s2, izenelib::util::UString::UTF_8);
    BOOST_CHECK( Compare<izenelib::util::UString>::start_with(u1, u2) == start );
    BOOST_CHECK( Compare<izenelib::util::UString>::end_with(u1, u2) == end );
    BOOST_CHECK( Compare<izenelib::util::UString>::contains(u1, u2) == contains );
}

BOOST_AUTO_TEST_SUITE( t_BTreeIndexer )

// BOOST_AUTO_TEST_CASE(io_iterator)
// {
//     typedef izenelib::am::EWAHBoolArray<uint32_t> ValueType;
//     typedef izenelib::am::luxio::BTree<int, int> DbType;
//     typedef izenelib::am::AMIterator<DbType> ForwardIterator;
//
// }

BOOST_AUTO_TEST_CASE(bitvector)
{
//     Bitset bv;
//     bv.clear(37);
//     bv.set(31);
//     std::cout<<bv<<std::endl;
    izenelib::am::EWAHBoolArray<uint32_t> compressed;
//     bv.compressed(compressed);
//     for(uint32_t i=0;i<32;i++) compressed.set(i);
//     compressed.add(4);

    compressed.set(31);
    compressed.addWord(0);
    std::vector<uint32_t> out;
    compressed.appendRowIDs(out);
    for(uint32_t i=0;i<out.size();i++)
    {
        std::cout<<out[i]<<",";
    }
    std::cout<<std::endl;
    std::cout<<compressed<<std::endl;
}

BOOST_AUTO_TEST_CASE(compare)
{
    BOOST_CHECK( Compare<uint32_t>::compare(1, 2)==-1 );
    BOOST_CHECK( Compare<uint32_t>::compare(2, 2)==0 );
    BOOST_CHECK( Compare<uint32_t>::compare(3, 2)==1 );
    Compare<uint32_t> comp;
    BOOST_CHECK( comp(3, 2)==false );
    BOOST_CHECK( comp(2, 2)==false );
    BOOST_CHECK( comp(1, 2)==true );

    //compare functor gives opposite result
    izenelib::am::CompareFunctor<uint32_t> compf;
    BOOST_CHECK( compf(3, 2)>0 );
    BOOST_CHECK( compf(2, 2)==0 );
    BOOST_CHECK( compf(1, 2)<0 );

}

BOOST_AUTO_TEST_CASE(ustring)
{
    check_ustring("abcdefg", "abc", true, false, true);
    check_ustring("wqert", "abc", false, false, false);
    check_ustring("abc", "abcd", false, false, false);
    check_ustring("asd123", "123", false, true, true);
    check_ustring("数码>手机通讯>手机配件", ">", false, false, true);
    check_ustring("数码>手机通讯>手机配件", "数码", true, false, true);
}

BOOST_AUTO_TEST_CASE(flush)
{
    DirController dir("./t_bt_flush");
    BTreeIndexer<izenelib::util::UString> indexer(dir.path()+"/test", "flush_test");
    indexer.open();

    for(uint32_t docid = 1; docid<=500000;docid++)
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        std::string uuid_str = boost::uuids::to_string(uuid);
//         std::cout<<"!!"<<uuid_str<<std::endl;
        izenelib::util::UString key(uuid_str, izenelib::util::UString::UTF_8);
        indexer.add(key, docid);
        if(docid%1000==0)
        {
            std::cout<<"add docid "<<docid<<std::endl;
        }
    }
    indexer.flush();

}

BOOST_AUTO_TEST_CASE(simple)
{
    DirController dir("./tbtreeindexer");

    BTreeIndexer<uint32_t> bt(dir.path()+"/test", "int");
    BOOST_CHECK( bt.open() );
    bt.add(1, 1);
    bt.add(1, 2);
    bt.add(2, 2);
    bt.add(2, 3);
//     {
//
//         Bitset docs;
//         bt.getValueLess(2, docs);
//         PrintBitset(docs);
//     }
//
//     {
//         std::cout<<"test 2"<<std::endl;
//         Bitset docs;
//         bt.getValueGreatEqual(1, docs);
//         PrintBitset(docs);
//     }

    bt.flush();
    {
        std::cout<<"test 11"<<std::endl;
        Bitset docs;
        bt.getValueLess(2, docs);
        PrintBitset(docs);
    }

//     {
//         std::cout<<"test 21"<<std::endl;
//         Bitset docs;
//         bt.getValueGreatEqual(1, docs);
//         PrintBitset(docs);
//     }
    bt.close();
}

BOOST_AUTO_TEST_CASE(framework_int_small)
{
    DirController dir("./t_bt_framework_int");

    BTreeTestRunner<uint32_t> runner(dir.path()+"/test", 100);
    runner.start();
}

BOOST_AUTO_TEST_CASE(framework_int)
{
    DirController dir("./t_bt_framework_int");

    BTreeTestRunner<uint32_t> runner(dir.path()+"/test");
    runner.start();

}

BOOST_AUTO_TEST_CASE(framework_double)
{
    DirController dir("./t_bt_framework_double");

    BTreeTestRunner<double> runner(dir.path()+"/test");
    runner.start();

}

BOOST_AUTO_TEST_CASE(framework_str)
{
    DirController dir("./t_bt_framework_str");

    BTreeTestRunner<izenelib::util::UString> runner(dir.path()+"/test");
    runner.start();

}

BOOST_AUTO_TEST_CASE(performance_int)
{
    DirController dir("./t_bt_performance_int");

    BTreePerformanceRunner<int64_t> runner(dir.path()+"/test", 18312347, 1, 10, 10);
    runner.start();

}

BOOST_AUTO_TEST_SUITE_END()
