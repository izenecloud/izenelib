#include  <DocumentManager.h>

#include <iostream>

using namespace std;

namespace sf1v5_dummy
{
    DocumentManager::DocumentManager()
        : docIdIndex_(1)
    {
    }

    /**
     * @brief   Stores documents parsed from a scd file
     */
    unsigned int DocumentManager::insertDocument( Document & doc )
    {
// Document doc_copy = doc;

// doc_copy.setId( docIdIndex_ );
//doc.setId( docIdIndex_ ); // TODO: decide if this is right. Convenience vs. Not Sloppy

//documentDb_.insert( pair<unsigned int, Document>(doc_copy.getId(), doc_copy) );
        documentDb_.insert( make_pair(doc.getId(), doc) );

        return docIdIndex_++;
    }

    unsigned int DocumentManager::insertDocument( const Document & doc )
    {
//Document doc_copy = doc;

//       doc_copy.setId( docIdIndex_ );

//       documentDb_.insert( pair<unsigned int, Document>(doc_copy.getId(), doc_copy) );

        documentDb_.insert( make_pair(doc.getId(), doc) );

        return docIdIndex_++;
    }

    /**
     * @brief   Stores the forward index
     */
    /*
    void DocumentManager::insertForwardIndex( const unsigned int docId, const ForwardIndex & forwardIndex )
    {
        forwardIndexDb_.insert( pair<unsigned int, ForwardIndex>(docId, forwardIndex) );
    }
    */


    /*
    bool DocumentManager::getForwardIndexByDoc( const unsigned int docId, ForwardIndex & forwardIndex ) const
    {
        map<unsigned int, ForwardIndex>::const_iterator it;

        if( (it = forwardIndexDb_.find(docId)) == forwardIndexDb_.end() )
        {
            return false;
        }
        else
        {
            forwardIndex = it->second;
            return true;
        }
    }
    */

    /**
     * @brief   Retrieves documents from the database, based on their id
     */
    bool DocumentManager::getDocument( unsigned int id, Document & doc ) const
    {
        map<unsigned int, Document>::const_iterator it;
        
        if( (it = documentDb_.find( id )) == documentDb_.end() )
        {
            return false;
        }
        else
        {
            doc = it->second;
            return true;
        }
    }

}
