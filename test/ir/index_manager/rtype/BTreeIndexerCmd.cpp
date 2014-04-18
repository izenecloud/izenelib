#include <ir/index_manager/index/rtype/BTreeIndexer.h>

using namespace izenelib::ir::indexmanager;

int main(int argc, char** argv)
{
    std::string dir = argv[1];
    BTreeIndexer<UString> indexer(dir, "uuid");
    if(!indexer.open())
    {
        std::cerr<<"open error"<<std::endl;
        return EXIT_FAILURE;
    }
    std::vector<uint32_t> docid_list;
    BTreeIndexer<UString>::ValueType data;
    UString key(argv[2], UString::UTF_8);
    indexer.getValue(key, data);
    if (data.which() == 0)
    {
        docid_list = boost::get<BTreeIndexer<UString>::DocListType>(data);
    }
    else
    {
        const Bitset& bits = boost::get<Bitset>(data);
        for(size_t i = 0; i < bits.size(); ++i)
        {
            if (bits.test(i))
                docid_list.push_back(i);
        }
    }
    for(uint32_t i=0;i<docid_list.size();i++)
    {
        std::cout<<docid_list[i]<<std::endl;
    }
    std::cout<<"count:"<<docid_list.size()<<std::endl;
}
