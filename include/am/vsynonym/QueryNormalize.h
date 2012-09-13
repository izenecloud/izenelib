/**
 * \file QueryNormalize.h
 * \author hongliang.zhao
 * \brief query Normalize
 * \date 2012-08-08
*/
#ifndef _QUERYNORMALIZE_H
#define _QUERYNORMALIZE_H

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <types.h>

using namespace std;

/*the max number of words that can be normalized*/
#define MAX_NORMALIZE_SIZE 4096
/*the max length of the type need to normalized*/
#define MAX_TYPE_LENGTH 8

/**
@brief example of dic for normalize:
#format [brand#type#type...#]
#rules: brand.lenth() >= 3
iphone#4s#4#3gs#3#
三星#galaxy#i9100#i9200#
...
*/

NS_IZENELIB_AM_BEGIN

class QueryNormalize
{
public:
    QueryNormalize();

    ~QueryNormalize();
    /**
    @brief for query like"iphone4s外壳"，we change it to "iphone 4s 外壳"
    */
    bool query_Normalize(string &str);
    /**
    @brief use load() to build Normalize Table;
    */
    bool load(string nameFile);

private:
    bool build(string& str);
    void buildkeystring();
    bool isTypeString(const string& valuestring, const string& type);
    /**
    @brief check if Query dic is load ok
    */
    bool LoadOK_;
    /**
    @brief process the space and upper char in string
    */
    void stanrd_raw(string& str);
    /**
    @brief the path of dictionary for normalize;
    */
    string filePath_;
    /**
    @brief the number of words can be normalize
    */
    uint32_t count_;
    /**
    @brief keyStr_ store the brand and valueStr_ store the type
    */
    string* keyStr_;
    string* valueStr_;
    /**
    @brief keySting_ keeps first three charactors for keep fast check if a query need to normalize, like"iph#ipa#nok#"
    */
    string keyString_;
};

NS_IZENELIB_AM_END
#endif
