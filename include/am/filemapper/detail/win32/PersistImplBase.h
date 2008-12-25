#ifndef PERSISTIMPLBASE_H_WIN32
#define PERSISTIMPLBASE_H_WIN32

#include <windows.h>
#include <cassert>
#include <iostream>

using namespace std;

NS_IZENELIB_AM_BEGIN

PersistImplBase::PersistImplBase()
{
    hFile = INVALID_HANDLE_VALUE;
    hMapFile = INVALID_HANDLE_VALUE;

    InitializeCriticalSection(&userCriticalSection);
    InitializeCriticalSection(&memoryCriticalSection);

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    setPageSize(info.dwAllocationGranularity);

    hUserMutex = CreateMutex(0, 0, "My shared mutex");      // TODO - a sensible name
    hMemoryMutex = CreateMutex(0, 0, "My memory mutex");    // TODO - a sensible name
    hEvent = CreateEvent(0, 1, 0, "My shared event");
}


PersistImplBase::~PersistImplBase()
{
    CloseHandle(hEvent);
    CloseHandle(hMemoryMutex);
    CloseHandle(hUserMutex);

    DeleteCriticalSection(&memoryCriticalSection);
    DeleteCriticalSection(&userCriticalSection);
}

void PersistImplBase::setPageSize(size_t ps)
{
    pageSize = ps;
}

void *PersistImpl::map(size_t offset, size_t length, void *base)
{
    void *base2 = MapViewOfFileEx(hMapFile, FILE_MAP_ALL_ACCESS, 0, offset, length, base);

    if (!base2) throw failed_to_map();

    return base2;
}


void PersistImpl::unmap(void *base, size_t)
{
    UnmapViewOfFile(base);
}


// Called the first time the file is created.

void PersistImpl::openFile(const char *filename)
{
    if (flags & read_only) mapFlags = PAGE_READONLY;
    else if (flags & private_map) mapFlags = PAGE_WRITECOPY;
    else mapFlags = PAGE_READWRITE;

    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, filename);
    sharename = filename;

    if (!hMapFile)
    {
        int openFlags;
        if (flags & create_new) openFlags = CREATE_ALWAYS;
        else openFlags = OPEN_ALWAYS;

        hFile = CreateFile(filename,
                           FILE_ALL_ACCESS, FILE_SHARE_WRITE|FILE_SHARE_READ, 0,
                           openFlags,
                           FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS,
                           0);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Could not open file");
            // Can't proceed
            return;
        }

        unsigned size = GetFileSize(hFile, 0);
        if (!size) size = sizeof(MappedData);

        // The file does not exist
        hMapFile = CreateFileMapping(hFile, 0, mapFlags, 0, size, sharename);

    }

    if (!hMapFile)
    {
        throw std::runtime_error("Could not create mapping from file");
    }
}


void PersistImpl::closeFile()
{
    CloseHandle(hMapFile);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    hMapFile = INVALID_HANDLE_VALUE;
}


void PersistImpl::remapFile(size_t new_length)
{
    unmapAll();
    CloseHandle(hMapFile);
    hMapFile = CreateFileMapping(hFile, 0, mapFlags, 0, new_length, sharename);
    mapBase();
    mapBlocks();
}



// map_file::lock
//
// Locks the user mutex.  Returns true if mutex obtained

bool PersistImpl::lock(int ms)
{
    if (flags & lock_cs)
    {
        EnterCriticalSection(&userCriticalSection);
    }
    else if (flags & lock_ipc)
    {
        if (ms == 0) ms=INFINITE;

        DWORD waitresult = WaitForSingleObject(hUserMutex, ms);

        updateMappedBlocks();

        return waitresult == WAIT_OBJECT_0;  // Got ownership
    }

    return true;
}


// map_file::unlock
//
// Unlocks the user mutex.

void PersistImpl::unlock()
{
    if (flags & lock_ipc)
    {
        ReleaseMutex(hUserMutex);
    }
    else if (flags & lock_cs)
    {
        LeaveCriticalSection(&userCriticalSection);
    }
}


// map_file::lockMem
//
// Locks the internal mutex used by the memory allocator

void PersistImpl::lockMem()
{
    if (flags & lock_ipc)
    {
        WaitForSingleObject(hMemoryMutex, INFINITE);
        updateMappedBlocks();
    }
    else if (flags & lock_cs)
    {
        EnterCriticalSection(&memoryCriticalSection);
    }
}


// map_file::unlockMem
//
// Unlocks the internal mutex used by the memory allocator

void PersistImpl::unlockMem()
{
    if (flags & lock_ipc)
    {
        ReleaseMutex(hMemoryMutex);
    }
    else if (flags & lock_cs)
    {
        LeaveCriticalSection(&memoryCriticalSection);
    }
}


// map_file::signal
//
// Experimental: communicate between two processes

void PersistImpl::signal()
{
    SetEvent(hEvent);
}


// map_file::wait
//
// Wait for the event.  Experimental, unfinished, don't use.

bool PersistImpl::wait(int ms)
{
    if (ms == 0) ms=INFINITE;

    DWORD waitresult = WaitForSingleObject(hEvent, ms);

    return waitresult == WAIT_OBJECT_0;
}


NS_IZENELIB_AM_END

#endif

