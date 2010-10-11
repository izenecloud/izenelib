/**
* @file        RAMDirectory.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief A memory based file system
*/
#ifndef RAMDIRECTORY_H
#define RAMDIRECTORY_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/utility/Utilities.h>

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <map>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#define RAMFILE_STREAMSIZE	4096
#define RAMFILE_MAXSEGS		10000

class RAMFile
{
public:
    RAMFile()
    {
        length = 0;
        lastModified = Utilities::currentTimeMillis();
        streamBufSize = RAMFILE_STREAMSIZE;
    }
    ~RAMFile()
    {
        vector<uint8_t*>::iterator iter = buffers.begin();
        while (iter != buffers.end())
        {
            delete[] (*iter);
            iter++;
        }
        buffers.clear();
    }
public:
    RAMFile* clone()
    {
        RAMFile* pRAM = new RAMFile();
        pRAM->length = length;
        pRAM->lastModified = lastModified;
        pRAM->streamBufSize = streamBufSize;
        uint8_t* tmp;
        vector<uint8_t*>::iterator iter = buffers.begin();
        while (iter != buffers.end())
        {
            tmp = new uint8_t[streamBufSize];
            memcpy(tmp,(*iter),streamBufSize);
            pRAM->buffers.push_back(tmp);
            iter++;
        }
        return pRAM;
    }
protected:
    int64_t length;
    int64_t	 lastModified;

    vector<uint8_t*> buffers;
    int32_t streamBufSize;

    friend class RAMDirectory;
    friend class RAMIndexInput;
    friend class RAMIndexOutput;
};
///IndexInput does not read data really
///RAMIndexInput provides the practical data reading through memory.
class RAMIndexInput : public IndexInput
{
public:
    RAMIndexInput(RAMFile* file);
    RAMIndexInput(RAMFile* file,char* buffer,size_t bufSize);
    RAMIndexInput(const RAMIndexInput& clone);
    virtual ~RAMIndexInput();
public:
    void readInternal(char* b,size_t length,bool bCheck = true);
    IndexInput*	clone();
    void close();
    void seekInternal(int64_t position);
protected:
    RAMFile* file;
    int64_t	 pointer;

    friend class RAMDirectory;
};
///IndexOutput does not write data really
///RAMIndexOutput provides the practical data writing to memory files.
class RAMIndexOutput : public IndexOutput
{
public:
    RAMIndexOutput();
    RAMIndexOutput(RAMFile* ramFile);
    virtual ~RAMIndexOutput();
public:
    int64_t length()
    {
        return file->length;
    }
    void close();
    void writeTo(IndexOutput* pOutput);
protected:
    void flushBuffer(char* b, size_t len);
    void  seekInternal(int64_t pos);

protected:
    RAMFile* file;
    bool bDeleteFile;
    int64_t pointer;
    friend class RAMDirectory;
};

///RAMDirectory, an encapsulation of memory based file system
class RAMDirectory :	public Directory
{
public:
    RAMDirectory(void);
    virtual ~RAMDirectory(void);
public:
    bool fileExists(const string& name) const;

    IndexInput*	openInput(const string& name);

    IndexInput*	openInput(const string& name, size_t bufsize);

    void deleteFile(const string& filename,bool throwError = true);

    void renameFile(const string& from, const string& to);

    void deleteFiles(const string& filename,bool throwError = true);

    void renameFiles(const string& from, const string& to);

    IndexOutput* createOutput(const string& name, const string& mode);

    IndexOutput* createOutput(const string& name, size_t buffersize, const string& mode = "w+b");	

    void close();

    izenelib::util::ReadWriteLock* getLock() { return rwLock_; }

private:
    map<string,RAMFile*> files;
    izenelib::util::ReadWriteLock* rwLock_;
};



}

NS_IZENELIB_IR_END

#endif
