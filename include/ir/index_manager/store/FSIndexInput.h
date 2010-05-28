/**
* @file        FSIndexInput.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/
#ifndef FSINDEXINPUT_H
#define FSINDEXINPUT_H

#include <ir/index_manager/store/IndexInput.h>

#include <boost/thread.hpp>
#include <fstream>
#include <string>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class FSIndexInput : public IndexInput
{

public:

    FSIndexInput(const char* filename);

    FSIndexInput(const char* filename,size_t buffsize);

    virtual ~FSIndexInput();

public:
    void readInternal(char* b,size_t length,bool bCheck = true);

    IndexInput* clone();

    void close();

    void seekInternal(int64_t position);

    void reopen();
private:
    FILE* fileHandle_;

    string filename_;

    mutable boost::mutex mutex_;

};

}

NS_IZENELIB_IR_END

#endif
