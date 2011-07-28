#include <am/vsynonym/VSynonym.h>

#include <types.h>

NS_IZENELIB_AM_BEGIN

VSynonym* VSynonym::createObject()
{
    return new VSynonym;
}


VSynonym::VSynonym( )
    : start_( 0 ), moreLong_( false )
{
}


VSynonym::VSynonym( uint8_t* start, bool moreLong = false )
    : start_(start), moreLong_(moreLong)
{
}


void VSynonym::setData(uint8_t* start)
{
    start_ = start;
}


void VSynonym::setMoreLong( bool moreLong )
{
    moreLong_ = moreLong;
}


bool VSynonym::hasMoreLong()
{
    return moreLong_;
}


vtnum_t VSynonym::getOverlapCount()
{
    if(!start_) return 0;
    return *reinterpret_cast<vtnum_t*>(start_);
}


vtnum_t VSynonym::getSynonymCount( int index )
{
    if(!start_) return 0;

    // check for first offset
    if( index >= *reinterpret_cast<vtnum_t*>(start_) )
        return 0;

    uint32_t off = index * VT_OFFSET_LEN + VT_NUM_LEN;
    vtoffset_t offset = *reinterpret_cast<vtoffset_t*>(start_ + off);
    return *reinterpret_cast<vtnum_t*>(start_ + offset);
}


char* VSynonym::getWord( int index, int offset )
{
    if(!start_) return 0;

    // check for first offset
    if( index >= *reinterpret_cast<vtnum_t*>(start_) )
        return 0;

    //first offset
    uint32_t off = VT_NUM_LEN + index * VT_OFFSET_LEN;
    vtoffset_t woffset = *reinterpret_cast<vtoffset_t*>(start_ + off);

    // check for second offset
    if( offset >= *reinterpret_cast<vtnum_t*>(start_ + woffset) )
        return 0;

    //second offset
    off = VT_NUM_LEN + offset * VT_OFFSET_LEN;
    woffset = *reinterpret_cast<vtoffset_t*>(start_ + woffset + off);
    return (char*)(start_ + woffset);
}


char* VSynonym::getHeadWord( int index )
{
    return getWord(index, 0);
}


vtnum_t VSynonym::size()
{
    if(!start_)
        return 0;
    size_t ret = 0;
    vtnum_t size1 = getOverlapCount();
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


ostream& operator << ( ostream& sout, VSynonym& retUnit )
{
    if(!retUnit.start_)
        return sout;
    vtnum_t max1 = retUnit.getOverlapCount() - 1;
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


void VSynonym::testLoop(){
    if(!start_)
        return;
    vtnum_t max1 = getOverlapCount() - 1;
    for(vtnum_t i=0; i<=max1; ++i){
        vtnum_t max2 = getSynonymCount(i) - 1;
        for(vtnum_t j=0; j<=max2; ++j){
            getWord(i, j);
        }
    }
}


VSynRetUnit::VSynRetUnit( string* fragment )
    : fragment(fragment), synonyms(NULL)
{
}

VSynRetUnit::VSynRetUnit( VSynonym *synonyms )
    : fragment(NULL), synonyms(synonyms)
{
}

VSynRetUnit::~VSynRetUnit()
{
    if(fragment != NULL)
    {
        delete fragment;
        fragment = NULL;
    }else
    {
        delete synonyms;
        synonyms = NULL;
    }
}


ostream& operator << ( ostream& sout, VSynRetUnit& retUnit )
{
    if(retUnit.isFragment()){
        sout<<(*retUnit.fragment);
    }else{
        sout<<(*retUnit.synonyms);
    }
    return sout;
}


bool VSynRetUnit::isFragment(){
    return fragment != NULL;
}


bool VSynRetUnit::isSynonyms(){
    return synonyms != NULL;
}


VExpandedQuery::VExpandedQuery()
{
    data_ = new vector<VSynRetUnit*>();
}


VExpandedQuery::~VExpandedQuery()
{
    for(vector<VSynRetUnit*>::iterator itr = data_->begin();
          itr != data_->end(); ++itr){
        delete *itr;
    }

    delete data_;
    data_ = NULL;
}


void VExpandedQuery::addUnit(VSynRetUnit* unit)
{
    data_->push_back(unit);
}


ostream& operator << ( ostream& sout, VExpandedQuery& query )
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


VSynonymContainer* VSynonymContainer::createObject()
{
    return new VSynonymContainer();
}


VSynonymContainer::VSynonymContainer()
{
    init();
    setSynonymDelimiter(",");
    setWordDelimiter("_");
}


VSynonymContainer::VSynonymContainer( string nameFile,
        const char* sep_synonyms = ",", const char* sep_words = "_")
{
    init();
    setSynonymDelimiter(sep_synonyms);
    setWordDelimiter(sep_words);
    loadSynonym(nameFile.c_str());
}


VSynonymContainer::~VSynonymContainer()
{
    delete lengthMap_;
    if(value_){
        free(value_);
    }
    delete trie_;
}


void VSynonymContainer::setSynonymDelimiter( const char* delim )
{
    synonymDelim_ = delim;
}


void VSynonymContainer::setWordDelimiter( const char* delim )
{
    wordDelim_ = delim;
}


void VSynonymContainer::get_synonyms( const string& key, VSynonym& synonym )
{
    VTrieNode node;
    if( trie_->search( key.data(), &node ) )
    {
        synonym.setData( value_ + node.data );
        synonym.setMoreLong( node.moreLong );
    }
    else
    {
        synonym.setData( 0 );
        synonym.setMoreLong( false );
    }
}


bool VSynonymContainer::searchNgetSynonym( const char* query, VSynonym* synonym )
{
    VTrieNode node;
    if( trie_->search( query, &node ) )
    {
        synonym->setData( value_ + node.data );
        synonym->setMoreLong( node.moreLong );
        return true;
    }
    else
    {
        synonym->setData( 0 );
        synonym->setMoreLong( false );
        return false;
    }
}

void VSynonymContainer::setSynonym(VSynonym* synonym, const VTrieNode* node)
{
    synonym->setData(value_ + node->data);
    synonym->setMoreLong(node->moreLong);
}


VSynonym* VSynonymContainer::get_synonyms(const string& key)
{
    VTrieNode node;
    if( trie_->search(key.data(), &node) )
    {
        return new VSynonym( value_ + node.data, node.moreLong );
    }
    else
    {
        return NULL;
    }
}


VExpandedQuery* VSynonymContainer::expandQuery(const string& query)
{
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


VTrie* VSynonymContainer::getData()
{
    return trie_;
}


int VSynonymContainer::loadSynonym(const char* pathDic)
{
    if( synonymDelim_ == wordDelim_){
        cerr << "Because delimeter among synonyms and delimiter among "<<
            "words are same, Can't not load synonym dictionary.\n"<<
            "Please set delimiters differentlty.\n";
        return 0;
    }

    std::ifstream fin(pathDic);
    if( fin.fail())
    {
        cerr << "Can't open " << pathDic << " file." << endl;
        return 0;
    }

    string line;

    const char* synoDelim = synonymDelim_.c_str();

    VTrieNode node; //for intValue
    while( !fin.eof() )
    {
        getline(fin, line);
		
        trimSelf(line);
        if(line.size() <= 0)
            continue;

        replaceAll(line, wordDelim_, ",");
        replaceAll(line, "，", ",");
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


size_t VSynonymContainer::size()
{
    return sizeof(this) + valueSize_ + trie_->size();
}


void VSynonymContainer::clear( bool releaseData )
{
    assert( trie_ != NULL );
    trie_->clear( releaseData );

    if( lengthMap_ != NULL )
    {
        lengthMap_->clear();
    }

    //clear the value bit
    if( releaseData )
    {
        if( value_ )
            free( value_ );
        valueSize_ = 1;
        value_ = (uint8_t*)malloc(valueSize_);
        //reserve the first TRIE
        endValPtr_ = value_ + 1;
    }
    else
    {
        endValPtr_ = value_ + 1;
    }
}


size_t VSynonymContainer::getFromLengthMap( const string& key, size_t maxValue )
{
    size_t ret = static_cast<size_t>((*lengthMap_)[key]);
    ret = ret > MinLen ? ret : MinLen;
    return ret >= maxValue ? maxValue : ret;
}


void VSynonymContainer::removeDuplicate( vector<string> & vec )
{
    std::sort(vec.begin(), vec.end());
    vector<string>::iterator new_end = std::unique(vec.begin(),vec.end());

    vec.erase(new_end, vec.end());
}

void VSynonymContainer::init()
{
    valueSize_ = 1;
    value_ = (uint8_t*)malloc(valueSize_);
    //reserve the first TRIE
    endValPtr_ = value_ + 1;
    trie_ = new VTrie();
    lengthMap_ = new map<string, size_t>();
}


int VSynonymContainer::appendValue(vector<string>& vec, int origData)
{
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

    vtnum_t lapCount = origSyn.getOverlapCount();
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

NS_IZENELIB_AM_END
