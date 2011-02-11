
#include <iostream>
#include <am/blockmanager/main.h>

using namespace std;
using namespace izenelib::am;

#define BLOCK_SIZE (1024 * 512)

struct MyType
{
    int integer;
    char chars[4];
    ~MyType() { }
};

struct my_handler
{
    void operator () (request * req)
    {
        cout<<req << " done, type=" <<endl;
    }
};

////////////////////////////////////////////////////////////////////////////

template <class T>
class new_alloc;

template <typename T, typename U>
struct new_alloc_rebind;

template <typename T>
struct new_alloc_rebind<T, T>
{
    typedef new_alloc<T> other;
};

template <typename T, typename U>
struct new_alloc_rebind
{
    typedef std::allocator<U> other;
};


// designed for typed_block (to use with std::vector )
template <class T>
class new_alloc
{
public:
    // type definitions
    typedef T value_type;
    typedef T * pointer;
    typedef const T * const_pointer;
    typedef T & reference;
    typedef const T & const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U, use new_alloc only if U == T
    template <class U>
    struct rebind
    {
        typedef typename new_alloc_rebind<T, U>::other other;
    };

    // return address of values
    pointer address(reference value) const
    {
        return &value;
    }
    const_pointer address(const_reference value) const
    {
        return &value;
    }

    new_alloc() throw () { }
    new_alloc(const new_alloc &) throw () { }
    template <class U>
    new_alloc(const new_alloc<U> &) throw () { }
    ~new_alloc() throw () { }

    template <class U>
    operator std::allocator<U>()
    {
        static std::allocator<U> helper_allocator;
        return helper_allocator;
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw ()
    {
        return (std::numeric_limits<std::size_t>::max) () / sizeof(T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate(size_type num, const void * = 0)
    {
        pointer ret = (pointer)(T::operator new (num * sizeof(T)));
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct(pointer p, const T & value)
    {
        // initialize memory with placement new
        new ((void *)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy(pointer p)
    {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate(pointer p, size_type /*num*/)
    {
        T::operator delete ((void *)p);
    }
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
inline bool operator == (const new_alloc<T1> &,
                         const new_alloc<T2> &) throw ()
{
    return true;
}

template <class T1, class T2>
inline bool operator != (const new_alloc<T1> &,
                         const new_alloc<T2> &) throw ()
{
    return false;
}

int main()
{
#if 0
    typedef typed_block<BLOCK_SIZE, MyType> block_type;

    const unsigned nblocks = 2;
    BIDArray<BLOCK_SIZE> bids(nblocks);
    std::vector<int> disks(nblocks, 2);
    request_ptr * reqs = new request_ptr[nblocks];
    block_manager * bm = block_manager::get_instance();
    bm->init("stxxl");
    bm->new_blocks(bids.begin(), bids.end());

    block_type * block = new block_type[2];
    unsigned i = 0;
    for (i = 0; i < block_type::size; ++i)
    {
        block->elem[i].integer = i;
        memcpy (block->elem[i].chars, "STXXL", 4);
    }
    for (i = 0; i < nblocks; ++i)
        reqs[i] = block->write(bids[i], my_handler());


    std::cout << "Waiting " << std::endl;
    wait_all(reqs, nblocks);

    for (i = 0; i < nblocks; ++i)
    {
        reqs[i] = block->read(bids[i], my_handler());
        reqs[i]->wait();
        for (int j = 0; j < block_type::size; ++j)
        {
            if (j != block->elem[j].integer)
            {
                cout<<"Error in block " << std::hex << i << " pos: " << j
                    << " value read: " << block->elem[j].integer<<endl;
            }
        }
    }


    bm->delete_blocks(bids.begin(), bids.end());

    delete[] reqs;
    delete[] block;
#else
    typedef typed_block<128 * 1024, double> block_type;
    std::vector<block_type::bid_type> bids;
    std::vector<request_ptr> requests;
    block_manager * bm = block_manager::get_instance();
    bm->init("stxxl");
    bm->new_blocks<block_type>(32, std::back_inserter(bids));
    std::vector<block_type/*, new_alloc<block_type>*/ > blocks(32);
    int vIndex;
    for (vIndex = 0; vIndex < 32; ++vIndex)
    {
        for (int vIndex2 = 0; vIndex2 < block_type::size; ++vIndex2)
        {
            blocks[vIndex][vIndex2] = vIndex2;
        }
    }
    for (vIndex = 0; vIndex < 32; ++vIndex)
    {
        requests.push_back(blocks[vIndex].write(bids[vIndex]));
    }
    wait_all(requests.begin(), requests.end());
    bm->delete_blocks(bids.begin(), bids.end());
    return 0;

#endif
}

