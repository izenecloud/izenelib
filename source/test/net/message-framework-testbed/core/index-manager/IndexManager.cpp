#include  <IndexManager.h>

#include <set>

#include <iostream>

using namespace std;
using namespace boost;

namespace sf1v5_dummy
{
    IndexManager::IndexManager()
    { }

    /**
     * @brief   Build a inverse index based on the forward index given by DocumentManager
     */
    void IndexManager::addDocument( const ForwardIndex & forwardIndex )
    {
        const set<string> & termList = forwardIndex.getTermList();
        set<string>::const_iterator it;

        unsigned int docId = forwardIndex.getDocId();

        for( it = termList.begin(); it != termList.end(); it++ )
        {
            inverseIndex_.insert( pair<string, unsigned int>(*it, docId ) );
        }
    }


    /**
     * @brief   Returns the document IDs of which the term belongs
     */
    bool IndexManager::findDocListByTerm( const string & term, vector<unsigned int> & docIdList ) const
    {
        unordered_multimap<string, unsigned int>::const_iterator it;
        pair<unordered_multimap<string, unsigned int>::const_iterator, unordered_multimap<string, unsigned int>::const_iterator> ret;

        ret = inverseIndex_.equal_range( term );

        docIdList.clear();

        for( it = ret.first; it != ret.second; it++ )
        {
            docIdList.push_back( it->second );
        }

        if( docIdList.empty() )
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}
