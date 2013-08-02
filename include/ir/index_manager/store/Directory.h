/**
* @file        Directory.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <ir/index_manager/utility/system.h>
#include <vector>

#include <util/ThreadModel.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class IndexInput;
class IndexOutput;
typedef vector<string>FileList;
/**
* virtual base class of FSDirectory and RAMDirectory, the former is an envelop of file system
* , the latter is an envelop of memory file system
*/
class Directory
{
public:
    Directory(){}

    virtual ~Directory(){}

public:
    virtual bool fileExists(const string& name) const = 0;

    virtual IndexInput* openInput(const string& name) = 0;

    virtual IndexInput* openInput(const string& name, size_t bufsize) = 0;

    virtual void deleteFile(const string& filename,bool throwError = true) = 0;

    virtual void renameFile(const string& from, const string& to) = 0;

    virtual void deleteFiles(const string& filename,bool throwError = true) = 0;

    virtual void renameFiles(const string& from, const string& to) = 0;

    virtual IndexOutput* createOutput(const string& name, const string& mode = "w+b") = 0;

    virtual IndexOutput* createOutput(const string& name, size_t buffersize, const string& mode = "w+b") = 0;

    virtual void close() = 0;

    virtual izenelib::util::ReadWriteLock* getLock() = 0;

    virtual std::string directory() {return "";}
};


}

NS_IZENELIB_IR_END

#endif
