#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <util/mem_pool.h>

#ifndef MEM_POOL_DEBUG
#define mprotect(a, b, c)
#endif

namespace izenelib
{
namespace util
{
inline size_t detail::get_aligned_seg_size(size_t suggested_min_seg_size)
{
    long syspagesize=sysconf(_SC_PAGESIZE);
    return (suggested_min_seg_size+syspagesize-1)/syspagesize*syspagesize;
}

void *mem_pool::segment::init(int fd, size_t offset, size_t size)
{
    void *base=MAP_FAILED;
    if(fd<0)
    {
        // Create anonymous mapping, fd set to -1 as it may have side-effects
        // For anonymous mapping, offset is ignored
        base=mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    }
    else
    {
        // Create named/shared file mapping
        base=mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
    }
    if (base==MAP_FAILED)
    {
        // TODO: Error log
        perror("XXX");
        base=0;
    }
    posix_madvise(base, size, POSIX_MADV_RANDOM | POSIX_MADV_WILLNEED);
    return base;
}

mem_pool::mem_pool(size_t seg_size, const char *name)
{
    if(name && name[0])
    {
        strcpy(name_, name);
    }
    else
    {
        name_[0]=0;
    }
    if(!init(seg_size))
    {
        throw std::runtime_error("failed to init mem_pool path:" + std::string(name_));
    }
}

mem_pool::mem_pool(const char *name)
{
    name_[0]=0;
    if (name && name[0])
    {
        strcpy(name_, name);
    }
    if(!name_[0] || !load())
    {
        if(!init(0))
            throw std::runtime_error("failed to init mem_pool path:" + std::string(name_));
    }
}

mem_pool::mem_pool(const char *base_name, const char *suffix)
{
    name_[0]=0;
    add_suffix(name_, base_name, suffix);
    if(name_[0])
    {
        if(!load())
        {
            if(!init(0)) std::runtime_error("failed to init mem_pool path:" + std::string(name_));
        }
    }
    else
    {
        // Anonymous mapping
        if(!init(0)) std::runtime_error("failed to init mem_pool path:" + std::string(name_));
    }
}

mem_pool::mem_pool(size_t seg_size, const char *base_name, const char *suffix)
{
    name_[0]=0;
    add_suffix(name_, base_name, suffix);
    if (!init(seg_size))
        throw std::runtime_error("failed to init mem_pool path:" + std::string(name_));
}

mem_pool::~mem_pool()
{
    save();
    mprotect(get_index(), 8192, PROT_READ|PROT_WRITE);
    for (int i=0; i<get_index()->used_seg; i++)
    {
        //munmap(get_seg(i).base_, get_index()->seg_size);
        munmap(segment_bases_[i], actual_seg_size_);
    }
    munmap(idx_, SEG_INDEX_SIZE);
}

bool mem_pool::init(size_t seg_size)
{
    if (seg_size==0)
    {
        if (name_[0])
        {
            seg_size=DEFAULT_SEG_SIZE;
        }
        else
        {
            seg_size=DEFAULT_ANON_SEG_SIZE;
        }
    }
    actual_seg_size_=detail::get_aligned_seg_size(seg_size);
    if(open_idx(true))
    {
        get_index()->seg_size=seg_size;
        // Create at least one segment
        expand();
        return true;
    }
    return load();
}

void mem_pool::clear()
{
    mprotect(get_index(), 8192, PROT_READ|PROT_WRITE);
    for (int i=0; i<get_index()->used_seg; i++)
    {
        //munmap(get_seg(i).base_, get_index()->seg_size);
        munmap(segment_bases_[i], actual_seg_size_);
        get_seg(i).clear();
    }
    munmap(idx_, SEG_INDEX_SIZE);
    mprotect(get_index(), 8192, PROT_READ);
}

void mem_pool::reset()
{
    size_t seg_size=get_index()->seg_size;
    clear();
    init(seg_size);
}

bool mem_pool::load()
{
    // Open existing index
    if(!open_idx(false))
    {
        return false;
    }

    actual_seg_size_=detail::get_aligned_seg_size(get_index()->seg_size);

    // Open existing segments
    for (int i=0; i<get_index()->used_seg; i++)
    {
        if(!open_segment(i, &(get_seg(i)), get_index()->seg_size))
            return false;
    }
    return true;
}

bool mem_pool::save()
{
    for (int i=0; i<get_index()->used_seg; i++)
    {
        if(msync(segment_bases_[i], get_index()->seg_size, MS_SYNC)<0)
            return false;
    }
    return msync(idx_, SEG_INDEX_SIZE, MS_SYNC)>=0;
}

bool mem_pool::expand(size_t size)
{
    if (size==0)
    {
        // Just add a new segment
        return new_segment();
    }
    // Expand pool to at least spec size
    if (size<=capacity())
    {
        return true;
    }
    int more_segs=int((size-capacity()+get_index()->seg_size-1)/get_index()->seg_size);
    for (int n=0; n<more_segs; n++)
    {
        if(!new_segment())
        {
            return false;
        }
    }
    return true;
}

OFFSET mem_pool::allocate(size_t length)
{
    int index=find_avail_seg(length);
    if (index<0)
    {
        if(!expand())
            return INVALID_OFFSET;
        index=int(get_index()->used_seg)-1;
    }
    OFFSET off=map_offset_seg_to_pool(index, get_seg(index).allocate(length));
    mprotect(get_index(), 8192, PROT_READ|PROT_WRITE);
    get_seg(index).used_size_+=length;
    mprotect(get_index(), 8192, PROT_READ);
    memset(get_addr(off), 0, length);
    return off;
}

OFFSET mem_pool::add_chunk(const void *p, size_t length)
{
    int index=find_avail_seg(length);
    if (index<0)
    {
        if(!expand())
            return INVALID_OFFSET;
        index=int(get_index()->used_seg)-1;
    }
    OFFSET off=map_offset_seg_to_pool(index, get_seg(index).add_chunk(p, length, segment_bases_[index]));
    mprotect(get_index(), 8192, PROT_READ|PROT_WRITE);
    get_seg(index).used_size_+=length;
    mprotect(get_index(), 8192, PROT_READ);
    return off;
}

OFFSET mem_pool::add_chunk_with_length(const void *p, size_t length)
{
    OFFSET off=allocate(length+4);
    if (off==INVALID_OFFSET)
    {
        return off;
    }
    memcpy(get_addr(off+4), p, length);
    *offptr<uint32_t>(off)=uint32_t(length);
    return off;
}

int mem_pool::find_avail_seg(size_t size) const
{
    for (int i=0; i<get_index()->used_seg; i++)
    {
        if (avail_size(i)>=size)
        {
            return i;
        }
    }
    return -1;
}

bool mem_pool::open_idx(bool initialize)
{
    if(!open_segment(-1, &idx_seg_, SEG_INDEX_SIZE, initialize))
        return false;
    idx_=get_index();
    if (initialize)
    {
        // Reinitialize segment index
        memset(idx_, 0, SEG_INDEX_SIZE);
    }
    return true;
}

bool mem_pool::new_segment()
{
    if (name_[0])
    {
        //printf("mem_pool(%s) create new segment...\t", name_);
    }
    mprotect(get_index(), 8192, PROT_READ|PROT_WRITE);
    int new_seg=int(get_index()->used_seg);
    if(open_segment(new_seg, &get_seg(new_seg), get_index()->seg_size))
    {
        get_index()->used_seg++;
        // This is a new unused seg
        get_seg(new_seg).used_size_=0;
        if (name_[0])
        {
            //printf("OK\n");
        }
        mprotect(get_index(), 8192, PROT_READ);
        return true;
    }
    if (name_[0])
    {
        //printf("FAILED\n");
    }
    mprotect(get_index(), 8192, PROT_READ);
    return false;
}

bool mem_pool::open_segment(int index, segment *seg, size_t size, bool initialize)
{
    int fd=-1;
    if (name_[0]!=0)
    {
        // There is a file name, create named mapping
        if (initialize)
        {
            // Create file, fail if it exists
            fd=open(name_, O_RDWR|O_CREAT|O_EXCL, 0644);
        }
        else
        {
            fd=open(name_, O_RDWR, 0644);
        }
        if (fd<0)
        {
            // TODO: Error log
            //perror("XX1XX");
            return false;
        }
        // Set file size, index segmeng+actual
        size_t filesize=SEG_INDEX_SIZE+(index+1)*actual_seg_size_;
        if (!set_file_size(fd, filesize, initialize))
        {
            close(fd);
            // TODO: Error log
            //perror("XX2XX");
            return false;
        }
    }
    // TODO: Error log
    void *base=seg->init(fd, get_seg_offset(index), size);
    close(fd);
    // Set segment base
    if (index<0)
    {
        // Opening index segment
        idx_=(col_idx *)base;
    }
    else
    {
        segment_bases_[index]=base;
    }
    return seg->check(base);
}

bool mem_pool::set_file_size(int fd, size_t sz, bool may_shrink)
{
    if (fd<0)
    {
        // Anonymous file for mapping
        return true;
    }
    struct stat st;
    if(fstat(fd, &st)<0)
        // TODO: Error log
        return false;
    if (st.st_size<sz || may_shrink)
    {
        return ftruncate(fd, sz)>=0;
    }
    // No need to expand
    return true;
}

bool mem_pool::rename(const char *new_name)
{
    if (!name_[0])
    {
        // No need to rename anonymous mapping
        return true;
    }
    if (::rename(name_, new_name)<0)
    {
        return false;
    }
    strcpy(name_, new_name);
    return true;
}

const char *add_suffix(char *buf, const char *name, const char *suffix)
{
    if (!name || !suffix || !name[0] || !suffix[0])
    {
        // No name of suffix
        return NULL;
    }
    strcpy(buf, name);
    strcat(buf, suffix);
    return buf;
}

void destroy_tl_mem_pool(void *p)
{
    mem_pool *mp=(mem_pool *)p;
    if (mp)
    {
        delete mp;
    }
}

__thread mem_pool *the_tl_pool=0;

mem_pool *get_tl_mem_pool()
{
    if (!the_tl_pool)
    {
        // There is no thread local mem_pool created
        the_tl_pool=new mem_pool();
    }
    return the_tl_pool;
}
}
}

