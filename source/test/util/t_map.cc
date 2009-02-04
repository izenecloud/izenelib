#include <iostream>

#include <util/Map.h>

using namespace std;

void doSTLMap()
{
    clock_t t1 = clock();

    typedef std::map<int, int> MapT;
    cout<<"===== std::map =====\n";
    MapT coll;
    for (int i = 0; i < 2000000; ++i)
        coll.insert(MapT::value_type(i, i));
    printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void doNewMap()
{
    clock_t t1 = clock();

    typedef izenelib::util::Map<int, int> MapT;
    cout<<"===== izenelib::util::Map =====\n";
    NS_BOOST_MEMORY::block_pool recycle;           //block_pool could be put globally for recycle usage.
    boost::scoped_alloc alloc(recycle);	
    MapT coll(alloc);
    for (int i = 0; i < 2000000; ++i)
        coll.insert(MapT::value_type(i, i));
    printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

int main()
{
    doSTLMap();
    doNewMap();
    return 0;
}
