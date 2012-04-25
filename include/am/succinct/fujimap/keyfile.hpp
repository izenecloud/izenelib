/*
 * keyFile.hpp
 * Copyright (c) 2010 Daisuke Okanohara All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef KEYFILE_HPP__
#define KEYFILE_HPP__

#include <fstream>
#include <string>
#include <vector>
#include <types.h>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

class KeyFile
{
    enum
    {
        BLOCKSIZE = 4096
    };

public:
    KeyFile();
    ~KeyFile();

    int initWorkingFile(const char* fn);
    void initMaxID(const uint64_t maxID);
    int clear();
    int write(const uint64_t id, const char* key, const size_t klen,
              const uint64_t value);
    int read(const uint64_t id, std::vector<std::pair<std::string, uint64_t> >& kvs);
    size_t getNum() const;

private:
    static void writeUint64(std::vector<char>& v, const uint64_t x);
    static uint64_t readUint64(std::vector<char>::const_iterator& it);

    std::string fns_;
    std::vector< std::vector<std::pair<std::string, uint64_t> > > buffers_;
    std::vector<uint64_t> nextPointers_;
    std::vector<uint64_t> firstPointers_;
    size_t num_;
    uint64_t maxID_;

};

}}

NS_IZENELIB_AM_END

#endif // KEY_FILE_HPP__

