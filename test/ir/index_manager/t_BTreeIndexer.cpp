#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/rtype/BTreeIndexer.h>

using namespace izenelib::ir::indexmanager;


class DirController
{
    public:
        DirController(const std::string& dir)
        :dir_(dir)
        {
            boost::filesystem::remove_all(dir_);
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

void PrintBitVector(const BitVector& docs)
{
    std::cout<<"final "<<docs<<std::endl;
}

BOOST_AUTO_TEST_SUITE( t_BTreeIndexer )

// BOOST_AUTO_TEST_CASE(io_iterator)
// {
//     typedef izenelib::am::EWAHBoolArray<uint32_t> ValueType;
//     typedef izenelib::am::luxio::BTree<int, int> DbType;
//     typedef izenelib::am::AMIterator<DbType> ForwardIterator;
//     
// }

BOOST_AUTO_TEST_CASE(simple)
{
    DirController dir("./tbtreeindexer");
    
    CBTreeIndexer<uint32_t> bt(dir.path()+"/test", "int");
    BOOST_CHECK( bt.open() );
    bt.add(1, 1);
    bt.add(1, 2);
    bt.add(2, 2);
    bt.add(2, 3);
//     {
//         
//         BitVector docs;
//         bt.getValueLess(2, docs);
//         PrintBitVector(docs);
//     }
//     
//     {
//         std::cout<<"test 2"<<std::endl;
//         BitVector docs;
//         bt.getValueGreatEqual(1, docs);
//         PrintBitVector(docs);
//     }
    
    bt.flush();
    {
        std::cout<<"test 11"<<std::endl;
        BitVector docs;
        bt.getValueLess(2, docs);
        PrintBitVector(docs);
    }
    
//     {
//         std::cout<<"test 21"<<std::endl;
//         BitVector docs;
//         bt.getValueGreatEqual(1, docs);
//         PrintBitVector(docs);
//     }
    bt.close();
}

BOOST_AUTO_TEST_SUITE_END()
