/**
 * \file VSynonym.h
 * \author vernkin
 * \brief contains associated files for the VSynonym
 */

#ifndef _VSYNONYM_H
#define	_VSYNONYM_H

#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <string.h>
#include <math.h>
#include <fstream>

#include <am/vsynonym/strutil.h>
#include <am/vsynonym/VTrie.h>

#include <types.h>

NS_IZENELIB_AM_BEGIN

#ifndef VSYN_INCRE_SIZE
    /** 50k */
    #define VSYN_INCRE_SIZE 51200
#endif

/** length of the number in the syns array */
#define VT_NUM_LEN 2

typedef uint16_t vtnum_t;

/** length of the offset in the syns array */
#define VT_OFFSET_LEN 2

typedef uint16_t vtoffset_t;

/**
 * \brief VSynonym the Param Type of the VSynonymDictionary.
 *
 * It can hold the Synonym Entry list after query synonym from VSynonymContainer.
 *
 * \author vernkin
 */
class VSynonym{
public:
    static VSynonym* createObject();

    VSynonym();

    VSynonym( uint8_t* start, bool moreLong );

    void setData(uint8_t* start);

    void setMoreLong( bool moreLong );

    bool hasMoreLong();

    /**
     * Return the count of lines including query used in searching.
     * \return number of the synonym entries matched
     */
    vtnum_t getOverlapCount();

    /**
     * After calling getMatchedCount( ), it is called for iterating whole
     * synonyms within a line.
     * This method won't take bound check.
     *
     * \param index in range of 0 to getMatchedCount()-1
     * \return the num of synonyms in the indexth synonym entry
     */
    vtnum_t getSynonymCount(int index);

    /**
     * Return the string which is placed on index and offset.
     * This method won't take bound check
     */
    char* getWord(int index, int offset);

    /**
     * Return the head word(entry word or first word in a line) which is placed on index.
     * In fact, It is exactly same with getWord( index, 0 ).
     *
     * \param index in range of 0 to getMatchedCount()-1
     * \return first word in the ith lap.
     */
    char* getHeadWord( int index);

    /**
     * Get the size of the VSynonym
     * \return the size of the VSynonym
     */
    vtnum_t size();

    friend ostream& operator << ( ostream& sout, VSynonym& retUnit );

    /**
     * For testing the speed of iterating all the words
     */
    void testLoop();

private:
    /** uint8_t array to hold synonym entry list */
    uint8_t* start_;

    /** Whether have longer word with the same prefix */
    bool moreLong_;
};

/**
 * \brief The Unit of the VExpandedQuery
 *
 * VSynRetUnit contains two attributes, and at most and at least one can be set
 * value. The fragment is the segments between the matched synonyms.
 */
class VSynRetUnit{
public:

    VSynRetUnit(string* fragment);

    VSynRetUnit(VSynonym *synonyms);

    virtual ~VSynRetUnit();

    /**
     * Output the VSynRetUnit
     */
    friend ostream& operator << ( ostream& sout, VSynRetUnit& retUnit );

    /**
     * Whether this Unit contains unit
     * \return true if fragment is not null
     */
    bool isFragment();

    bool isSynonyms();

public:
    /** store the fragment of the sentence */
    string* fragment;

    /** Synonym Objects */
    VSynonym *synonyms;
};

/**
 * \brief The type the of Expanded Query
 *
 * VExpandedQuery is a combination of the VSynRetUnit
 */
class VExpandedQuery{
public:

    VExpandedQuery();

    ~VExpandedQuery();

    void addUnit(VSynRetUnit* unit);

    friend ostream& operator << ( ostream& sout, VExpandedQuery& query );

private:
    vector<VSynRetUnit*> *data_;
};

/**
 * \brief the VTrie version of the Synonym Dictionary
 *
 * The class keep the relation between the synonym and macthed synonym lists.
 * VTrie is used to hold the all the synonyms, and a extra array to hold all the
 * synonym entry lists.
 *
 * \author Vernkin
 */
class VSynonymContainer {
public:

    static VSynonymContainer* createObject();

    /**
     * Create the PHSynonymCntainer with default synonym delimiter (',')
     * and word delimiter ('_')
     */
    VSynonymContainer();

    /**
     * Create the PHSynonymCntainer with specific synonym and word delimiter,
     * and the resource file
     * \param nameFile specific synonym resource file, each line is a synonym,
     * each synonym is separated by sep_synonyms in a synonym, and each
     * individal word is separated by sep_synonyms in a synonym entry.
     * \param sep_synonyms synonym entry delimiter in a synonym
     * \param sep_words word delimiter in a synonym entry
     */
    VSynonymContainer(string nameFile, const char* sep_synonyms,
            const char* sep_words);

    ~VSynonymContainer();

    /**
     * Set the synonym delimiter
     * \param delim new synonym delimiter
     */
    void setSynonymDelimiter(const char* delim);

    /**
     * Set the word delimiter
     * \param delim new word delimiter
     */
    void setWordDelimiter( const char* delim);

    /**
     * Get the synonyms with specific key, if found nothing, data in the
     * PHSynonym is set NULL.
     * \param key specific key to find the synonyms
     * \param synonym PHSynonym object to store the data
     */
    void get_synonyms( const string& key, VSynonym& synonym );

    /**
     * Search the query from VSynonymContainer.
     * if searching fail, assign 0 or NULL to synonym parameter. otherwise,
     * synonym parameter has a proper value.
     *
     * \param query word to be queried
     * \param synonym to store the returned information
     */
    bool searchNgetSynonym( const char* query, VSynonym* synonym );

    /**
     * Set synonym data, after searching reached node.
     * This interface can be called after a outside search on trie was performed.
     *
     * @param synonym
     * @param node
     */
    void setSynonym(VSynonym* synonym, const VTrieNode* node);

    /**
     * Get the synonyms with specific key, if found nothing, return NULL.
     * \param key specific key to find the synonyms
     * \return synonym VSynonym object to store the data; return NULL if the
     * key is not exists.
     */
    VSynonym* get_synonyms(const string& key);


    /**
     * Expand the query with all the possible synonyms
     */
    VExpandedQuery* expandQuery( const string& query );

    VTrie* getData();

    /**
     * load the data from the file
     * \param pathDic the path of the synonym dictionary file
     * \return 0 if occur error and 1 if works fine
     */
    int loadSynonym( const char* pathDic );

    /**
     * Get the size of the whole VSynonymContainer used
     */
    size_t size();


    /**
     * Clear the VTrie
     * \param releaseData whether release allocated data
     */
    void clear( bool releaseData = false );

private:

    inline size_t getFromLengthMap(const string& key, size_t maxValue);

    inline void removeDuplicate(vector<string> & vec);

    void init();

    /**
     * append the synonym entry value
     */
    int appendValue(vector<string>& vec, int origData);

public:
    /** The Minimum Length starts to record for the length map */
    static const size_t MinLen = 5;

private:
    /** synonym delimiter */
    string synonymDelim_;

    /** word delimiter */
    string wordDelim_;

    /** Length Map, record the key maps to the length of the key */
    map<string, size_t> *lengthMap_;

    /** Perfect Hash Pointer */
    VTrie* trie_;

    /** The array to store the value */
    uint8_t* value_;

    /** ending pointer(exclude) of the value_ */
    uint8_t* endValPtr_;


    size_t valueSize_;
};

NS_IZENELIB_AM_END

#endif	/* _VSYNONYM_H */

