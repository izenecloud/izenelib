/**
* @file        FSDirectory.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FSDIRECTORY_H
#define FSDIRECTORY_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/utility/system.h>

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <string>
#include <map>
#include <dirent.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

///FSDirectory, an encapsulation of local file system

class FSDirectory : public Directory
{
public:
    FSDirectory(const string& path,bool bCreate=false);
    virtual ~FSDirectory(void);

public:
    bool fileExists(const string& name) const;

    IndexInput*	openInput(const string& name);

    IndexInput*	openInput(const string& name,size_t bufsize);

    IndexInput* openMMapInput(const string& name);

    void deleteFile(const string& filename,bool throwError = true);

    void renameFile(const string& from, const string& to);

    void deleteFiles(const string& filename,bool throwError = true);

    void renameFiles(const string& from, const string& to);

    IndexOutput* createOutput(const string& name, const string& mode="w+b");

    IndexOutput* createOutput(const string& name, size_t buffersize, const string& mode = "w+b");

    void close();

    izenelib::util::ReadWriteLock* getLock() { return rwLock_; }

    void setMMapFlag(bool flag) { mmap_ = flag;}

    bool isMMapEnable() { return mmap_;}

    std::string directory()
    {
        return directoryName_;
    }
private:
    void create();
private:
    string directoryName_;

    bool mmap_;

    izenelib::util::ReadWriteLock* rwLock_;
};


}

NS_IZENELIB_IR_END

#endif
