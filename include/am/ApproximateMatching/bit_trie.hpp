#ifndef WAT_ARRAY_BIT_TRIE_HPP_
#define WAT_ARRAY_BIT_TRIE_HPP_

#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
namespace izenelib
{

class BitNode
{
public:
    int grade_;
    BitNode* ZNext_;
    BitNode* ONext_;
public:
    BitNode(int grade);
    ~BitNode();
};

class BitTrie
{
    int height_;
public:
    BitNode* Root_;
    BitTrie(int alphbatNum);
    ~BitTrie();
    void insert(int val);
    void insert(vector<uint32_t> val);
    bool exist(int val) const;

};


};
#endif
