/**
* @file        RemoteDirectory.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef REMOTEDIRECTORY_H
#define REMOTEDIRECTORY_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/UDTFSIndexOutput.h>

#include <boost/thread.hpp>
#include <string>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class RemoteDirectory : public Directory
{
public:
    RemoteDirectory(){}

    virtual ~RemoteDirectory(void){}

public:
    bool fileExists(const string& name) const { assert(false); return false; }

    void deleteFile(const string& filename,bool throwError = true) { assert(false); }

    void renameFile(const string& from, const string& to) { assert(false); }

    void deleteFiles(const string& filename,bool throwError = true) { assert(false); }

    void renameFiles(const string& from, const string& to) { assert(false); }

    IndexInput*	openInput(const string& name) { assert(false);return NULL;}

    IndexInput*	openInput(const string& name, size_t bufsize) { assert(false); return NULL;}

    IndexOutput* createOutput(const string& name,const string& mode="w+b")
    {
         return new UDTFSIndexOutput(name.c_str(), 1024*1024, mode);
    }

    IndexOutput* createOutput(const string& name, size_t buffersize, const string& mode = "w+b")
    {
        return new UDTFSIndexOutput(name.c_str(), buffersize, mode);
    }

    void close(){ }

    izenelib::util::ReadWriteLock* getLock(){ return NULL; }
};

}

NS_IZENELIB_IR_END

#endif
