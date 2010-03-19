#include <iostream>
#include <cassert>
#include <ctime>

#include <am/filemapper/persist_stl.h>

using namespace izenelib::am;

namespace izenelib { namespace am{
struct TestObject
{
    mapped_string str1;
    mapped_map<mapped_string,mapped_string> map1;
    mapped_set<int> data;
};

typedef map_data<TestObject> TestRoot;

}}


void testRoot(TestObject &root, int n)
{
    for (int i=0; i<n; ++i)
        root.data.insert(i);

    for (int i=0; i<n; ++i)
        assert(root.data.find(i) != root.data.end());
}


int main(int argc, char* argv[])
{
    try
    {
        // Test 1: create a new heap

        {
            TestRoot root("test.map", 1, create_new|auto_grow);
            assert(root->map1.empty());
        }

        // Test 2: check invalid uid

        try
        {
            TestRoot root("test.map", 2);
            assert(0); // Don't get here
        }
        catch (std::bad_alloc&)
        {
        }

        // Test 3: Put some stuff into it
        {
            TestRoot root("test.map", 1);
            root->map1["hello"] = "goodbye";
        }

        // Test 4: Check it out
        {
            TestRoot root("test.map", 1);
            assert(root->map1["hello"] == "goodbye");
        }

        // Test 5: Fill it up
        {
            const int n = 10000000;	// TODO: one more zero
            TestRoot root("test.map", 1);
            for (int i=0; i<n; ++i)
                root->data.insert(i);

            for (int i=0; i<n; ++i)
                assert(root->data.find(i) != root->data.end());
        }

        // Test 6: Test it out
        {
            const int n = 1000000;
            TestRoot root("test.map", 1, create_new|ipc_heap);
            for (int i=0; i<n; ++i)
                root->data.insert(i);

            for (int i=0; i<n; ++i)
                assert(root->data.find(i) != root->data.end());
        }

        // Test 7: Specify initial size
        {
            TestRoot root("test.map", 1, create_new, 10000000);
            testRoot(*root, 100000);
        }

        // Test 8: Allocate large objects
        {
            TestRoot root("test.map", 1, create_new|auto_grow);

            for (int i=0; i<10; ++i)
            {
                const int n=10000000;
                void *x = root.malloc(n);
                memset(x, 1, n);
            }
        }
    }
    catch (std::bad_alloc &e)
    {
        std::cout << "Tests failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Tests passed in " << (clock() * 1000 / CLOCKS_PER_SEC) << "ms"<<std::endl;

    return 0;
}

