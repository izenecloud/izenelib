/*
  * fujimap.hpp
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


#ifndef FUJIMAP_HPP__
#define FUJIMAP_HPP__

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include <fstream>
#include "fujimap_block.hpp"
#include "keyedge.hpp"
#include "keyfile.hpp"

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/**
  * Succinct Associative Array
  * Support basic key/value store operations (set/get)
  */
class Fujimap
{
public:
    /**
     * Constructor
     */
    Fujimap();

    /**
     * Destructor
     */
    ~Fujimap();

    /**
     * Initialize a seed for hash function
     * @param seed A seed value
     */
    void initSeed(const uint64_t seed);

    /**
     * Initialize a false positive rate
     * @param fpLen A negative of false positive rate power (prob. of false positive rate is 2^{-fpLen_})
     */
    void initFP(const uint64_t fpLen);

    /**
     * Initialize a size of temporary map size. A succinct and static map will be constructured after every tmpN_ key/values are added.
     * @param tmpN A size of temporary map.
     */
    void initTmpN(const uint64_t tmpN);

    /**
     * Initialize a number of blocks in hash. This would be log(number of key/values).
     * @param keyBlockN A number of blocks.
     */
    void initKeyBlockN(const uint64_t keyBlockN);

    /**
     * Initialize a working file.
     * @param fn A name of working file which stores temporary data.
     */
    int initWorkingFile(const char* fn);

    /**
     * Initialize an encode type
     * @param et A type of encoding
     */
    void initEncodeType(const EncodeType et);

    /**
     * Set a record of key/value.
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param vbuf the pointer to the value region.
     * @param vlen the length of the value.
     * @param searchable true if this record will be searchable immediately after this operation false
     * if this record will be searchable after build() is called (default: false).
     */
    void setString(const char* kbuf, const size_t klen,
                   const char* vbuf, const size_t vlen,
                   const bool searchable = false);

    /**
     * Set a record of key/value. This record will be searchable immediately after this operation.
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param value the interger value.
     * @param searchable true if this record will be searchable immediately after this operation or false
     * if this record will be searchable after build() is called (defalut: false).
     */
    void setInteger(const char* kbuf, const size_t klen, const uint64_t value,
                    const bool searchable = false);

    /**
     * Build an index for registered key/value pairs which are not indexed.
     * @return true on success, or false on failure.
     : @note when build() failed, a user specify new seed funciton by initSeed(), and retry build().
     */
    int build();

    /**
     * Retrieve the string value for a key
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @param vlen the length of the value.
     * @return the pointer to the value region of the corresponding record, or NULL  on failure.
     * @note Because the pointer of the returned value is a member of fm,
     * a user should copy the returned value if using the returned value.
    */
    const char* getString(const char* kbuf, const size_t klen, size_t& vlen) const;

    /**
     * Retrieve the integer value for a key
     * @param kbuf the pointer to the key region.
     * @param klen the length of the key.
     * @return the interge value for a key, or fujimap::NOTFOUND on failure.
    */
    uint64_t getInteger(const char* kbuf, const size_t klen) const;


    /**
     * Load the previous status from a file
     * @param index the file name where the index is stored.
     * @return 0 on success, -1 on failure
     */
    int load(const char* index);

    /**
     * Save the current status in a file
     * @param index the file name where the index is stored.
     * @return 0 on success, -1 on failure
     */
    int save(const char* index); ///< Load a map from index

    /**
     * Report the current status when some error occured.
     * @return the current status message
     */
    std::string what() const;

    /**
     * Get the registered number of key/values.
     * @return the number of registered keys.
     */
    size_t getKeyNum() const;

    /**
     * Get the size of working space (estimated)
     * @return the size of working space in bits
     */
    size_t getWorkingSize() const;

    /**
     * Get the fpLen
     * @return fpLen
     */
    uint64_t getFpLen() const;

    /**
     * Get the current EncodeType
     * @return Current Encoding Type
     */
    EncodeType getEncodeType() const;

    /**
     * Get the current EncodeType in string
     * @return Current Encoding Type
     */
    std::string getEncodeTypeStr() const;

private:
    int build_(std::vector<std::pair<std::string, uint64_t> >& kvs,
               FujimapBlock& fb);


    void saveString(const std::string& s, std::ofstream& ofs) const; ///< Util for save
    void loadString(std::string& s, std::ifstream& ifs) const; ///< Util for load
    uint64_t getBlockID(const char* kbuf, const size_t len) const;
    uint64_t getCode(const std::string& value); ///< Return corresponding code of a given value

    std::ostringstream what_; ///< Store a message

    std::map<std::string, uint64_t> val2code_; ///< Map from value to code
    std::vector<std::string> code2val_; ///< Map from code to value

    KeyFile kf_; ///< A set of non-searchable key/values
    std::map<std::string, uint64_t> tmpEdges_; ///< A set of searchable key/values to be indexed

    std::vector< std::vector<FujimapBlock> > fbs_; ///< BitArrays

    uint64_t seed_; ///< A seed for hash
    uint64_t fpLen_; ///< A false positive rate (prob. of false positive is 2^{-fpLen})
    uint64_t tmpN_; ///< A size of tempolary map
    uint64_t keyBlockN_; ///< A number of blocks
    EncodeType et_; ///< An encode type of values
};

}}

NS_IZENELIB_AM_END

#endif // FUJIMAP_HPP__
