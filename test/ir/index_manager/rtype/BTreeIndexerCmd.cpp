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
    UString key(argv[2], UString::UTF_8);
    indexer.getValue(key, docid_list);
    for(uint32_t i=0;i<docid_list.size();i++)
    {
        std::cout<<docid_list[i]<<std::endl;
    }
    std::cout<<"count:"<<docid_list.size()<<std::endl;
}

