#include <am/filemapper/detail/PersistImpl.h>


NS_IZENELIB_AM_BEGIN


PersistImpl::PersistImpl()
{
    mappedData = 0;
    num_mapped_blocks=0;
    flags=0;
}

PersistImpl::~PersistImpl()
{
    close();
}

void PersistImpl::roundToPageSize(size_t &size)
{
    const size_t mask = pageSize-1;
    if (size&mask)
    {
        size &= ~mask;
        size += pageSize;
    }
}


void PersistImpl::open(const char *filename, int uid, int f, size_t init_size)
{
    flags = f;

    try
    {
        openFile(filename);

        mapBase();

        if (!mappedData->created)
        {
            // First time creation

            initialize_map();
            mappedData->uid = uid;
            if (init_size)
                addBlock(init_size);
        }
        else
        {
            // Already created
            if (uid != mappedData->uid)
                throw std::bad_alloc();

            num_mapped_blocks = 0;
            mapBlocks();
        }
    }
    catch (...)
    {
        // Could not open the file
        close();

        mappedData = 0;  // TODO: Done by close
        throw;
    }
}

void PersistImpl::close()
{
    unmapAll();
    closeFile();
}


void PersistImpl::mapBase()
{
    mappedData = static_cast<MappedData*>(map(0, sizeof(MappedData)));
}

void PersistImpl::mapBlocks()
{
    int n = mappedData->num_mapped_blocks;

    for (int b=num_mapped_blocks; b<n; ++b)
    {
        MappedData::mapped_block &block = mappedData->blocks[b];
        map(block.file_offset, block.size, block.base);
    }

    num_mapped_blocks = n;
}

void PersistImpl::initialize_map()
{
    new(mappedData) MappedData();

    mappedData->auto_grow = (flags&auto_grow)!=0;

    mappedData->file_length = sizeof(MappedData);
    roundToPageSize(mappedData->file_length);
}

void PersistImpl::unmapAll()
{
    if (mappedData)
    {
        for (int b=0; b<mappedData->num_mapped_blocks; ++b)
        {
            MappedData::mapped_block &block = mappedData->blocks[b];
            unmap(block.base, block.size);
        }

        unmap(mappedData, sizeof(MappedData));
        mappedData = 0;
    }

    num_mapped_blocks=0;
}

void *PersistImpl::malloc(size_t size)
{
    if (!mappedData) return 0;

    // if(size==0) return mappedData->top;  // A valid address?  TODO

    lockMem();

    int free_cell = object_cell(size);

#if RECYCLE
    if (mappedData->free_space[free_cell])
    {
        // We have a free cell of the desired size

        void *block = mappedData->free_space[free_cell];
        mappedData->free_space[free_cell] = *(void**)block;

#if CHECK_MEM
        ((int*)block)[-1] = size;
#endif

#if TRACE_ALLOCS
        std::cout << " +" << block << "(" << size << ")";
#endif

        unlockMem();
        return block;
    }
#endif

#if CHECK_MEM
    *(int*)mappedData->top = size;
    mappedData->top += sizeof(int);
#endif

    void *t = mappedData->top;

    if (mappedData->top + size > mappedData->end && mappedData->auto_grow)
    {
        // We have run out of mapped memory
        addBlock(size);
        t = mappedData->top;
    }


    if (mappedData->top + size > mappedData->end)
    {
        // We were unable to extend the mapped memory
        unlockMem();
        return 0;  // Failure
    }

    mappedData->top += size;

#if TRACE_ALLOCS
    std::cout << " +" << t << "(" << size << ")";
#endif

    unlockMem();
    return t;
}

void PersistImpl::free(void *block)
{
    // TODO: Not implemented...
}

void PersistImpl::free(void* block, size_t size)
{
    lockMem();

#if TRACE_ALLOCS
    std::cout << " -" << block << "(" << size << ")";
#endif
    if (size==0) return; // Do nothing

    if (block <mappedData || block >=mappedData->end)
    {
        // We have attempted to "free" data not allocated by this memory manager
        // This is a serious fault, but we carry on

        std::cout << "Block out of range!\n";  // This is a serious error!

        // This happens in basic_string...
        unlockMem();
        return;
    }

    // This assertion no longer applies...
    // assert(block>=mappedData && block<mappedData->end);
    // This means that the address is not managed by this heap!

#if CHECK_MEM
    assert(((int*)block)[-1] == size);
    ((int*)block)[-1] = 0;  // This is now DEAD!
#endif

#if RECYCLE   // Enable this to enable block to be reused
    int free_cell = object_cell(size);
    // free_cell is the cell number for blocks of size "size"

    // Add the free block to the linked list in free_space
    *(void**)block = mappedData->free_space[free_cell];
    mappedData->free_space[free_cell] = block;
#endif

    unlockMem();
}

void *PersistImpl::root() const
{
    if (!mappedData) return 0; // Failed

#if CHECK_MEM
    // return (int*)mappedData->root + 1;
    return (int*)mappedData->blocks[0].base + 1;
#endif

    return mappedData->blocks[0].base;
    // return mappedData->root;
}

bool PersistImpl::empty() const
{
    if (!mappedData) return true;
    return mappedData->top == mappedData->bottom;
}

bool PersistImpl::usable() const
{
    return mappedData != 0;
}

void PersistImpl::updateMappedBlocks()
{
    if (num_mapped_blocks != mappedData->num_mapped_blocks)
    {
        mapBlocks();
    }
}

void PersistImpl::addBlock(size_t size)
{
    if (size < MappedData::block_size) size = MappedData::block_size;

    roundToPageSize(size);

    remapFile(mappedData->file_length + size);

    void *base = map(mappedData->file_length, size, 0);

    MappedData::mapped_block &block = mappedData->blocks[mappedData->num_mapped_blocks];

    block.base = base;
    block.file_offset = mappedData->file_length;
    block.size = size;

    mappedData->file_length += size;
    mappedData->num_mapped_blocks++;
    num_mapped_blocks++;

    mappedData->top = (char*)base;
    mappedData->bottom = base;
    mappedData->end = (char*)base + size;
}

NS_IZENELIB_AM_END


#ifdef WIN32
#include  <am/filemapper/detail/win32/PersistImplBase.h>
#else
#include  <am/filemapper/detail/posix/PersistImplBase.h>
#endif

