/*
 * \file StrBasedVTrie.h
 * \brief VTrie related classes and program
 * \date May 4, 2010
 * \author Vernkin Chen
 */

#ifndef STRBASEDVTRIE_H_
#define STRBASEDVTRIE_H_

#include <vector>
#include <string>

using std::vector;
using std::string;

#include <types.h>

namespace izenelib{ namespace am{

class VTrie;
class VTrieNode;

bool createVTrie( const vector<string>& words, VTrie& trie );

/**
 * \brief The utility class to search the string one by one
 *
 * The utility class to search the string one by one
 */
class StrBasedVTrie
{
public:
    /**
     * Create an instance
     *
     * \param pTrie the VTrie
     */
    StrBasedVTrie( VTrie* pTrie );

    ~StrBasedVTrie();

    /**
     * Reset all the status
     */
    void reset();

    /**
     * Get node current info
     */
    void getCurrentNode(VTrieNode& rnode) const;

    /**
     * search the specific string
     *
     * \param p point to the specific string
     * \return true if have extending string or just the right string, here
     * string is the combination of the previous strings and p
     */
    bool search( const char *p );

    /**
     * First reset then search
     *
     * \param p point to the specific string
     * \param len the length to compare
     */
    bool firstSearch( const char* p );

    /**
     * Whether exists in the dictionary
     */
    bool exists();

public:
    /**
     * The VTrie
     */
    VTrie* trie;

    /**
     * The VTrieNode
     */
    VTrieNode* node;

    /**
     * true if the search over all the characters in the input string
     */
    bool completeSearch;
};

}}


#endif /* STRBASEDVTRIE_H_ */
