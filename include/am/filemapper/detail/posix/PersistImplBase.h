#ifndef PERSISTIMPLBASE_H_POSIX
#define PERSISTIMPLBASE_H_POSIX

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

NS_IZENELIB_AM_BEGIN

PersistImplBase::PersistImplBase()
{
    fd = -1;	// Invalid file
    setPageSize(getpagesize());

    pthread_mutex_init(&memMutex, 0);
    pthread_mutex_init(&userMutex, 0);
}


PersistImplBase::~PersistImplBase()
{
    pthread_mutex_destroy(&memMutex);
    pthread_mutex_destroy(&userMutex);
}

void PersistImplBase::setPageSize(size_t ps)
{
    pageSize = ps;
}

void PersistImpl::openFile(const char *filename)
{
    close();

    size_t length = sizeof(MappedData);

    if (flags & private_map) mapFlags = MAP_PRIVATE;
    else mapFlags =  MAP_SHARED;

    if (flags & create_new)
        fd = -1;
    else
        fd = ::open(filename, O_RDWR, S_IRWXU|S_IRGRP|S_IROTH);

    if (fd == -1)
    {
        // File did not exist, so we create it instead

        fd = ::open(filename, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRGRP|S_IROTH);

        // TODO: A better exception
        if (fd == -1) throw std::runtime_error("Could not create mapping from file");

        // Fill up the file with zeros

        char c=0;
        lseek(fd, length-1, SEEK_SET);
        write(fd, &c, 1);
    }
}


void PersistImpl::closeFile()
{
    if (fd != -1)
    {
        ::close(fd);
        fd = -1;
    }
}


void *PersistImpl::map(size_t offset, size_t length, void *base)
{
    void *base2;

    if (base)
        base2 = mmap(base, length, PROT_READ|PROT_WRITE, MAP_FIXED|mapFlags, fd, offset);
    else
        base2 = mmap(base, length, PROT_READ|PROT_WRITE, mapFlags, fd, offset);

    if (base2 == MAP_FAILED)
    {
        throw std::bad_alloc();
    }

    return base2;
}


void PersistImpl::remapFile(size_t length)
{
    // Must grow the file in order that it is mapped correctly (some platforms)
    char c=0;
    lseek(fd, length-1, SEEK_SET);
    write(fd, &c, 1);
}


void PersistImpl::unmap(void *addr, size_t length)
{
    munmap((char*)addr, length);
}


bool PersistImpl::lock(int ms)
{
    if (flags & lock_cs)
    {
        pthread_mutex_lock(&userMutex);
    }
    else if (flags & lock_ipc)
    {
        pthread_mutex_lock(&mappedData->userMutex);
        updateMappedBlocks();
    }

    return true;
}


void PersistImpl::unlock()
{
    if (flags & lock_cs)
        pthread_mutex_unlock(&userMutex);
    else if (flags & lock_ipc)
        pthread_mutex_unlock(&mappedData->userMutex);
}


void PersistImpl::lockMem()
{
    if (flags & lock_cs)
    {
        pthread_mutex_lock(&memMutex);
    }
    else if (flags & lock_ipc)
    {
        pthread_mutex_lock(&mappedData->memMutex);
        updateMappedBlocks();
    }
}


void PersistImpl::unlockMem()
{
    if (flags & lock_cs)
        pthread_mutex_unlock(&memMutex);
    else if (flags & lock_ipc)
        pthread_mutex_unlock(&mappedData->memMutex);

}


NS_IZENELIB_AM_END

#endif

