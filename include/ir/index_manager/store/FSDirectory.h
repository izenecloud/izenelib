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
#include <string>
#include <map>
#include <dirent.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

///FSDirectory, an encapsulation of local file system

class FSDirectory : public Directory
{
    typedef map<string,FSDirectory*> directory_map;
    typedef directory_map::iterator directory_iterator;
private:
    FSDirectory(const string& path,bool bCreate=false);
public:
    virtual ~FSDirectory(void);

public:
    static FSDirectory* getDirectory(const string& path,bool bCreate);

    bool fileExists(const string& name) const;

    IndexInput*	openInput(const string& name);

    IndexInput*	openInput(const string& name,size_t bufsize);

    void deleteFile(const string& filename,bool throwError = true);

    void renameFile(const string& from, const string& to);

    void batDeleteFiles(const string& filename,bool throwError = true);

    void batRenameFiles(const string& from, const string& to);

    IndexOutput* createOutput(const string& name, const string& mode="w+b");

    IndexOutput* createOutput(const string& name, size_t buffersize, const string& mode = "w+b");

    void close();

private:
    static FSDirectory::directory_map& getDirectoryMap();

    void create();
private:
    string directory;

    int nRefCount;

    mutable boost::mutex mutex_;
};


}

NS_IZENELIB_IR_END

#endif
