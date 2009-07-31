/**
 * @file    DocumentManager.h
 * @brief   Defines the dummy DocumentManager class
 * @author  MyungHyun Lee  (Kent)
 * @date    2009-01-30
 */

#ifndef _DUMMY_DOCUMENT_MANAGER_H_
#define _DUMMY_DOCUMENT_MANAGER_H_

#include <document-manager/ForwardIndex.h>
#include <document-manager/Document.h>

#include <map>

namespace sf1v5_dummy
{
    /**
     * @brief   This class stores and manages a list of documents 
     *          as well as creating forward indexes to provide IndexManager
     * @details
     * This is a dummy code. No incremental db building. Once the databases are built it is final.
     */
    class DocumentManager
    {
        public:
            DocumentManager();

            /**
             * @brief   Stores documents parsed from a scd file
             */
            unsigned int insertDocument( const Document & doc );
            unsigned int insertDocument( Document & doc );

            /**
             * @brief   Stores the forward index
             * TODO: NOT USED
             */
            //void insertForwardIndex( const unsigned int docId, const ForwardIndex & forwardIndex );


            /**
             * @brief   Retrieves documents from the database, based on their id
             */
            bool getDocument( unsigned int id, Document & doc ) const;

            /**
             * @brief   Retrives the forward index of a document.
             */
            //bool getForwardIndexByDoc( const unsigned int docId, ForwardIndex & forwardIndex ) const;




        private:

            /// @brief  Keeps track of the next id to create. starts from 1
            unsigned int                        docIdIndex_;

            /// @brief  Stores the documents by their id
            std::map<unsigned int, Document>        documentDb_; 

            /// @brief  Stores the list of forward indexes
            std::map<unsigned int, ForwardIndex>    forwardIndexDb_;    
    };
}


#endif  //_DUMMY_DOCUMENT_MANAGER_H_
