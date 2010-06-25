/**
 * \file VTrie.h
 * \author vernkin
 * \brief hold the VTrie and the VTrieNode
 */

#include <string.h>
#include <vector>
#include <iostream>

#include <types.h>

NS_IZENELIB_AM_BEGIN

using namespace std;

#ifndef _VTRIE_H
#define	_VTRIE_H

#define VTPTR_L 4

#define VALUE_L 4
/** Length of the VTrie Entry, INT_LEN+1 */
#define VTENTRY_L 5

/** Type of the VTrie pointer */
typedef uint32_t vtptr_t;

/** max length of the same path */
#define MAX_SAMEP_LEN 256

/** number of the slots in the VTrie keys */
#define VTKEY_NUM 256

#define MIN_REMAIN 3072

#ifndef INCRE_SIZE
#define INCRE_SIZE 204800
#endif

/** length of VTrie's child status byte */
#define VTCHILDS_L 1

/** length of VTrie's same path status byte*/
#define VTSAMEPS_L 1

/* Conversion table for VTKEY_NUM numerical code */
extern uint8_t VTRIE_CODE[];


/**
 * \brief The node of the VTrie
 *
 * VTrieNode hold a int value and a bool attribute which indicates whether has
 * the synonyms with the same prefix.
 *
 * \author vernkin
 */
class VTrieNode{
public:
    /**
     * Default Constructor
     */
    VTrieNode();

    /**
     * Assign value to the node of VTrie, And value is over 0.
     * Almost, the value(data) may be a index of other data structure.
     * \param pData new data
     */
    void setData(int pData);

    /**
     * Get the value of the node
     * \return value of the node
     */
    int getData();

    /**
     * Whether more long string whose prefix is the query exists in trie
     * structure.
     * \return true if more string with same prefix
     */
    bool hasMoreLong();

    /**
     * see hasMoreLong()
     * \param pMoreLong new more long
     */
    void setMoreLong(bool pMoreLong);

    /**
     * Reset all the value to initial state.
     */
    void init();

    friend std::ostream& operator << ( std::ostream& sout, VTrieNode& node );

public:
    /** the integer value */
    int data;

    /** the state of the node */
    uint16_t state;

    /** the offset in the data array of the VTrie */
    vtptr_t offset;

    /** whethe has synonyms with the same prefix */
    bool moreLong;
};

/**
 * \brief VTrie is a variant of the Radix Tree.
 *
 * While the Radix Tree is a space-optimized standard trie, the VTrie is a
 * space-optimized of the Radix Tree.
 *
 * \author vernkin
 */
class VTrie{
public:
    VTrie();

    ~VTrie();

    /**
     * If insert failed, return 0, otherwise any value bigger than 0
     * And the information of a current trie node is set to node parameter.
     *
     * \param key the key
     * \param node the node to stote the information
     * \return return 0 if insert fails
     */
    int insert( const char* key, VTrieNode* node );

    /**
     * Optimize the data structure of the VTrie, it makes querying
     * faster and use less memory.
     */
    void optimize();

    /**
     * If string is not found, return 0, otherwise any value bigger than 0.
     * And the information of a searched trie node is set to node parameter.
     * \param key the key
     * \param node the node to stote the information
     * \return return 0 if search fails
     */
    int search( const char* key, VTrieNode* node);

    /**
     * The function of this method is to iterate trie node by each character.
     * Parameter (VTrieNode*)node is including current node state.
     * \param ch specific character
     * \param node the last VTrieNode
     * \return If following node does not exist, return value is 0,
     * other wise bigger than 0.
     */
    int find( char ch, VTrieNode* node );

    /**
     * Get the size of the structure
     */
    size_t size();

    /**
     * Clear the VTrie
     * \param releaseData whether release allocated data
     */
    void clear( bool releaseData = false );

private:
    /** initialize the VTrie and add assert statement*/
    void init();

    /**
     * ensure the length of the data
     */
    void ensureDataLength(size_t keyLen);

    /**
     * return the ending pointer (exclude that location)
     */
    uint8_t* optimize(uint8_t* nData, uint8_t* oData, uint8_t* nRoot, uint8_t* oRoot);

    /**
     * append the leaf node to the end of the data_
     */
    inline void appendLeafNode(const char* key, VTrieNode* node);

    /**
     * Exclude same path status byte
     * this method would update endPtr_
     */
    inline void copyLeafNodeValue(uint8_t* dataPtr, const char* key, VTrieNode* node);

    /**
     * Get the minimum mod value of the children
     */
    inline uint8_t getModMinSize(const vector<int>& vec);

    inline vtptr_t resolveConfilct(uint8_t* nodeStart, const char* key, VTrieNode* node);

    void printBytes(uint8_t* start, size_t num);

private:
    /** uint8_t array to store the trie */
    uint8_t* data_;

    /** ending used pointer in data_ */
    uint8_t* endPtr_;

    /** Bytes have been wasted */
    size_t wastedBytes_;

    /** current data size */
    size_t curDataSize_;
};

NS_IZENELIB_AM_END

#endif	/* _VTRIE_H */

