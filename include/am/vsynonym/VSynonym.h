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
    VSynonym() : start_(0){
    }

    VSynonym(uint8_t* start) : start_(start){
    }

    void setData(uint8_t* start){
        start_ = start;
    }

    /**
     * Return the count of lines including query used in searching.
     * \return number of the synonym entries matched
     */
    vtnum_t getMatchedCount(){
        if(!start_) return 0;
        return *reinterpret_cast<vtnum_t*>(start_);
    }

    /**
     * After calling getMatchedCount( ), it is called for iterating whole
     * synonyms within a line.
     * This method won't take bound check.
     *
     * \param index in range of 0 to getMatchedCount()-1
     * \return the num of synonyms in the indexth synonym entry
     */
    vtnum_t getSynonymCount(int index){
        if(!start_) return 0;

        uint32_t off = index * VT_OFFSET_LEN + VT_NUM_LEN;
        vtoffset_t offset = *reinterpret_cast<vtoffset_t*>(start_ + off);
        return *reinterpret_cast<vtnum_t*>(start_ + offset);
    }

    /**
     * Return the string which is placed on index and offset.
     * This method won't take bound check
     */
    char* getWord(int index, int offset){
        //first offset
        uint32_t off = VT_NUM_LEN + index * VT_OFFSET_LEN;
        vtoffset_t woffset = *reinterpret_cast<vtoffset_t*>(start_ + off);

        //second offset
        off = VT_NUM_LEN + offset * VT_OFFSET_LEN;
        woffset = *reinterpret_cast<vtoffset_t*>(start_ + woffset + off);
        return (char*)(start_ + woffset);
    }

    /**
     * Return the head word(entry word or first word in a line) which is placed on index.
     * In fact, It is exactly same with getWord( index, 0 ).
     *
     * \param index in range of 0 to getMatchedCount()-1
     * \return first word in the ith lap.
     */
    char* getHeadWord( int index){
        return getWord(index, 0);
    }

    /**
     * Get the size of the VSynonym
     * \return the size of the VSynonym
     */
    vtnum_t size(){
        if(!start_)
            return 0;
        size_t ret = 0;
        vtnum_t size1 = getMatchedCount();
        ret += VT_NUM_LEN + size1 * VT_OFFSET_LEN;
        for(vtnum_t i=0; i<size1; ++i){
            vtnum_t size2 = getSynonymCount(i);
            ret += VT_NUM_LEN + size2 * VT_OFFSET_LEN;
            for(vtnum_t j=0; j<size2; ++j){
                ret += strlen(getWord(i, j)) + 1;
            }
        }

        return ret;
    }

    friend ostream& operator << ( ostream& sout, VSynonym& retUnit ){
        if(!retUnit.start_)
            return sout;
        vtnum_t max1 = retUnit.getMatchedCount() - 1;
        for(vtnum_t i=0; i<=max1; ++i){
            vtnum_t max2 = retUnit.getSynonymCount(i) - 1;
            for(vtnum_t j=0; j<=max2; ++j){
                //cout<<"print "<<i<<","<<j<<endl;
                if(j != max2 || i != max1){
                    sout<<retUnit.getWord(i, j)<<",";
                }else{
                    sout<<retUnit.getWord(i, j);
                }
            }
        }
        return sout;
    }

    /**
     * For testing the speed of iterating all the words
     */
    void testLoop(){
        if(!start_)
            return;
        vtnum_t max1 = getMatchedCount() - 1;
        for(vtnum_t i=0; i<=max1; ++i){
            vtnum_t max2 = getSynonymCount(i) - 1;
            for(vtnum_t j=0; j<=max2; ++j){
                getWord(i, j);
            }
        }
    }

private:
    /** uint8_t array to hold synonym entry list */
    uint8_t* start_;
};

/**
 * \brief The Unit of the VExpandedQuery
 *
 * VSynRetUnit contains two attributes, and at most and at least one can be set
 * value. The fragment is the segments between the matched synonyms.
 */
class VSynRetUnit{
public:

    VSynRetUnit(string* fragment): fragment(fragment), synonyms(NULL){
    }

    VSynRetUnit(VSynonym *synonyms): fragment(NULL), synonyms(synonyms){
    }

    virtual ~VSynRetUnit(){
        if(fragment != NULL){
            delete fragment;
            fragment = NULL;
        }else{
            delete synonyms;
            synonyms = NULL;
        }
    }

    /**
     * Output the VSynRetUnit
     */
    friend ostream& operator << ( ostream& sout, VSynRetUnit& retUnit )
    {
        if(retUnit.isFragment()){
            sout<<(*retUnit.fragment);
        }else{
            sout<<(*retUnit.synonyms);
        }
        return sout;
    }

    /** store the fragment of the sentence */
    string* fragment;

    /** Synonym Objects */
    VSynonym *synonyms;

    /**
     * Whether this Unit contains unit
     * \return true if fragment is not null
     */
    bool isFragment(){
        return fragment != NULL;
    }

    bool isSynonyms(){
        return synonyms != NULL;
    }
};

/**
 * \brief The type the of Expanded Query
 *
 * VExpandedQuery is a combination of the VSynRetUnit
 */
class VExpandedQuery{
public:
    VExpandedQuery(){
        data_ = new vector<VSynRetUnit*>();
    }

    ~VExpandedQuery(){
        for(vector<VSynRetUnit*>::iterator itr = data_->begin();
              itr != data_->end(); ++itr){
            delete *itr;
        }

        delete data_;
        data_ = NULL;
    }

    void addUnit(VSynRetUnit* unit){
        data_->push_back(unit);
    }

    friend ostream& operator << ( ostream& sout, VExpandedQuery& query )
    {
        vector<VSynRetUnit*> *data = query.data_;
        int size = data->size();
        for(int i=0; i<size; ++i){
            cout<<*((*data)[i]);
            if(i != size - 1)
                sout<<" | ";
        }
        return sout;
    }

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

    /**
     * Create the PHSynonymCntainer with default synonym delimiter (',')
     * and word delimiter ('_')
     */
    VSynonymContainer(){
        init();
        setSynonymDelimiter(",");
        setWordDelimiter("_");
    }

    /**
     * Create the PHSynonymCntainer with specific synonym and word delimiter,
     * and the resource file
     * \param nameFile specific synonym resource file, each line is a synonym,
     * each synonym is separated by sep_synonyms in a synonym, and each
     * individal word is separated by sep_synonyms in a synonym entry.
     * \param sep_synonyms synonym entry delimiter in a synonym
     * \param sep_words word delimiter in a synonym entry
     */
    VSynonymContainer(string nameFile, const char* sep_synonyms = ",",
            const char* sep_words = "_"){
        init();
        setSynonymDelimiter(sep_synonyms);
        setWordDelimiter(sep_words);
        loadSynonyms(nameFile.c_str());
    }

    ~VSynonymContainer(){
        if(lengthMap_){
            delete lengthMap_;
        }

        if(value_){
            free(value_);
        }

        if(trie_){
            delete trie_;
        }
    }

    /**
     * Set the synonym delimiter
     * \param delim new synonym delimiter
     */
    void setSynonymDelimiter(const char* delim){
        synonymDelim_ = delim;
    }

    /**
     * Set the word delimiter
     * \param delim new word delimiter
     */
    void setWordDelimiter( const char* delim){
        wordDelim_ = delim;
    }

    /**
     * Get the synonyms with specific key, if found nothing, data in the
     * PHSynonym is set NULL.
     * \param key specific key to find the synonyms
     * \param synonym PHSynonym object to store the data
     */
    void get_synonyms( const string& key, VSynonym& synonym ){
        VTrieNode node;
        if(trie_->search(key.data(), &node)){
            synonym.setData(value_ + node.data);
        }else{
            synonym.setData(0);
        }
    }

    /**
     * Search the query from VSynonymContainer.
     * if searching fail, assign 0 or NULL to synonym parameter. otherwise,
     * synonym parameter has a proper value.
     *
     * \param query word to be queried
     * \param synonym to store the returned information
     */
    void searchNgetSynonym( char* query, VSynonym* synonym ){
        VTrieNode node;
        if(trie_->search(query, &node)){
            synonym->setData(value_ + node.data);
        }else{
            synonym->setData(0);
        }
    }

    /**
     * Get the synonyms with specific key, if found nothing, return NULL.
     * \param key specific key to find the synonyms
     * \return synonym VSynonym object to store the data; return NULL if the
     * key is not exists.
     */
    VSynonym* get_synonyms(const string& key){
        VTrieNode node;
        if(trie_->search(key.data(), &node)){
            return new VSynonym(value_ + node.data);
        }else{
            return NULL;
        }
    }


    /**
     * Expand the query with all the possible synonyms
     */
    VExpandedQuery* expandQuery(const string& query){
        VExpandedQuery *ret = new VExpandedQuery();
        string lower = toLower(query);
        vector<size_t>* tokens1 = findTokens(lower,true);
        vector<size_t>& tokens = *tokens1;
        size_t size = tokens.size();
        if(size <= 1)
            return ret;

        size_t start = 1;
        size_t lastEndIndex = 0; //exclude

        while(start < size){
            size_t subStartIndex = tokens[start-1]+1;
            string head = lower.substr(subStartIndex,tokens[start]-tokens[start-1]);
            //cout<<"Check head "<<head<<endl;
            int offset = getFromLengthMap(head, size-start) - 1;
            while(offset >= 0){
                string key = lower.substr(subStartIndex,tokens[start+offset]-tokens[start-1]);
                //cout<<"Check Key "<<key<<endl;
                VSynonym* syn = get_synonyms(key);
                if(syn == NULL){
                    --offset;
                    continue;
                }

                if(lastEndIndex < subStartIndex){
                    ret->addUnit(new VSynRetUnit(
                        new string(query.substr(lastEndIndex,subStartIndex-lastEndIndex))));
                }
                lastEndIndex = subStartIndex + tokens[start+offset] - tokens[start-1];
                ret->addUnit(new VSynRetUnit(syn));
                //cout<<"Find Key "<<key<<endl;
                break;
            }

            if(offset < 0)
                ++start;
            else
                start += offset + 1;
        }

        if(lastEndIndex < query.length()){
            ret->addUnit(new VSynRetUnit(
                        new string(query.substr(lastEndIndex,query.length()-lastEndIndex))));
        }


        return ret;
    }

    VTrie* getData(){
        return trie_;
    }

    /**
     * load the data from the file
     * \param pathDic the path of the synonym dictionary file
     * \return 0 if occur error and 1 if works fine
     */
    int loadSynonyms(const char* pathDic){
        if( synonymDelim_ == wordDelim_){
            cerr << "Because delimeter among synonyms and delimiter among "<<
                "words are same, Can't not load synonym dictionary.\n"<<
                "Please set delimiters differentlty.\n";
            return 0;
        }

        ifstream fin(pathDic);
        if( fin.fail())
        {
            cerr << "Can't open " << pathDic << " file." << endl;
            return 0;
        }

        string line;
        lengthMap_ = new map<string, size_t>();

        const char* synoDelim = synonymDelim_.c_str();

        VTrieNode node; //for intValue
        while( !fin.eof() )
        {
            getline(fin, line);
            trimSelf(line);
            if(line.size() <= 0)
                continue;

            replaceAll(line, wordDelim_, " ");
            vector<string> synvec;
            tokenizeAndLowerCase(synvec, line, synoDelim);

            size_t vecLen = synvec.size();
            //one synonym make no sense
            if(vecLen <= 1)
                continue;

            //the starting address for those only has one synonym entry
            int value = 0;
            set<string> keySet;
            for(size_t i=0; i<vecLen; ++i){
                string& key = synvec[i];
                //avoid duplicate keys
                if(keySet.find(key) == keySet.end()){
                    keySet.insert(key);
                }else{
                    continue;
                }
                size_t keyLoginLen = computeLength(key, true);
                //update the length map
                if(keyLoginLen > MinLen && (*lengthMap_)[key] < keyLoginLen){
                    (*lengthMap_)[key] = static_cast<int>(keyLoginLen);
                }
                node.init();
                //more than one synonym
                if(trie_->search(key.data(), &node)){
                    node.data = appendValue(synvec, node.data);
                }
                //only one synonym entry value not set
                else if(value <= 0){
                    node.data = appendValue(synvec, 0);
                    value = node.data;
                }
                else{
                    node.data = value;
                }

                assert(node.data);
                trie_->insert(key.data(), &node);
            }
        }

        return 1;
    }

    /**
     * Get the size of the whole VSynonymContainer used
     */
    size_t size(){
        return sizeof(this) + valueSize_ + trie_->size();
    }

private:


    inline size_t getFromLengthMap(const string& key, size_t maxValue){
        size_t ret = static_cast<size_t>((*lengthMap_)[key]);
        ret = ret > MinLen ? ret : MinLen;
        return ret >= maxValue ? maxValue : ret;
    }

    inline void removeDuplicate(vector<string> & vec){
        std::sort(vec.begin(), vec.end());
        vector<string>::iterator new_end = std::unique(vec.begin(),vec.end());

        vec.erase(new_end, vec.end());
    }

    void init(){
        valueSize_ = 1;
        value_ = (uint8_t*)malloc(valueSize_);
        //reserve the first TRIE
        endValPtr_ = value_ + 1;
        trie_ = new VTrie();
    }

    /**
     * append the synonym entry value
     */
    int appendValue(vector<string>& vec, int origData){
        VSynonym origSyn;
        if(origData > 0){
            origSyn.setData(value_ + origData);
        }

        // 1st:  compute the size have to append
        size_t usedSize = origSyn.size();
        //origData not exists, add the lapCount status
        if(usedSize == 0){
            usedSize += VT_NUM_LEN;
        }

        size_t vecSize = vec.size();
        //first level offset for vector
        usedSize += VT_OFFSET_LEN;
        usedSize += VT_NUM_LEN + VT_OFFSET_LEN * vecSize;
        for(size_t i=0; i<vecSize; ++i){
            usedSize += vec[i].size() + 1;
        }

        // 2nd: ensure the size to copy
        size_t remainSize = valueSize_ - (endValPtr_ - value_);
        if(remainSize < usedSize){
            size_t copyLen = (size_t)(endValPtr_ - value_);
            size_t increSize = (size_t)(ceil((usedSize - remainSize) * 1.0 /
                    VSYN_INCRE_SIZE)) * VSYN_INCRE_SIZE;
            assert(increSize);
            valueSize_ += increSize;
            uint8_t* nValue = (uint8_t*)malloc(valueSize_);
            memcpy(nValue, value_, copyLen);
            free(value_);
            value_ = nValue;
            endValPtr_ = value_ + copyLen;
            memset(endValPtr_, 0x0, valueSize_ - copyLen);

            //update the origSyn
            if(origData > 0){
                origSyn.setData(value_ + origData);
            }
        }

        // 3rd: copy the value
        int ret = endValPtr_ - value_;

        uint8_t* startPtr = endValPtr_;

        vtnum_t lapCount = origSyn.getMatchedCount();
        //set the lap count
        *reinterpret_cast<vtnum_t*>(endValPtr_) = (vtnum_t)(1 + lapCount);
        endValPtr_ += VT_NUM_LEN;
        vtoffset_t* firstOffset = (vtoffset_t*)endValPtr_;
        endValPtr_ += VT_OFFSET_LEN * (1 + lapCount);


        for(vtnum_t i=0; i<lapCount; ++i){
            *firstOffset++ = (vtoffset_t)(endValPtr_ - startPtr);

            vtnum_t wordCount = origSyn.getSynonymCount(i);
            *reinterpret_cast<vtnum_t*>(endValPtr_) = wordCount;
            endValPtr_ += VT_NUM_LEN;
            vtoffset_t* secondOffset = (vtoffset_t*)endValPtr_;
            endValPtr_ += VT_OFFSET_LEN * wordCount;

            //copy the words
            for(vtnum_t j=0; j<wordCount; ++j){
                *secondOffset++ = (vtoffset_t)(endValPtr_ - startPtr);
                char* word = origSyn.getWord(i, j);
                do{
                    *endValPtr_++ = *word++;
                }while(*word);
                *endValPtr_++ = 0; // '\0', terminal character
            }
        }

        //copy the new append vector
        *firstOffset++ = (vtoffset_t)(endValPtr_ - startPtr);
        *reinterpret_cast<vtnum_t*>(endValPtr_) = (vtnum_t)(vecSize);
        endValPtr_ += VT_NUM_LEN;
        vtoffset_t* secondOffset = (vtoffset_t*)endValPtr_;
        endValPtr_ += VT_OFFSET_LEN * vecSize;
        for(size_t j=0; j<vecSize; ++j){
            *secondOffset++ = (vtoffset_t)(endValPtr_ - startPtr);
            const char* word = vec[j].data();
            do{
                *endValPtr_++ = (uint8_t)*word++;
            }while(*word);
            *endValPtr_++ = 0; // '\0', terminal character
        }

        return ret;
    }

public:
    /** The Mininum Length starts to record for the length map */
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

