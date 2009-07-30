/**
 * @file    IndexManager.h
 * @brief   Defines the dummy IndexManager class
 * @author  MyungHyun Lee  (Kent)
 * @date    2009-01-30
 */

#ifndef _DUMMY_INDEX_MANAGER_H_
#define _DUMMY_INDEX_MANAGER_H_

#include <document-manager/ForwardIndex.h>

#include <boost/unordered_map.hpp>

#include <vector>
#include <string>


namespace sf1v5_dummy
{
    /**
     * @brief   Stores the inverse index and provides the information
     * @detail
     * The inverse index maps a term to list of documents that it appears in.
     */
    class IndexManager
    {
        public:
            IndexManager();

            /**
             * @brief   The documents are requested to be indexed. 
             * @details
             * Forward index information of the documents need to be retrieved from the document-manager
             */
            //void addDocuments( const std::vector<unsigned int> & docList );


            /**
             * @brief   Adds the document's forward index data to the inverse index 
             */
            void addDocument( const ForwardIndex & forwardIndex );


            /**
             * @brief   Returns the document IDs of which the term belongs
             */
            bool findDocListByTerm( const std::string & term, std::vector<unsigned int> & docIdList ) const;



        private:
            /// @brief  Maps a term to the documents (ID) that appears
            boost::unordered_multimap<std::string, unsigned int>    inverseIndex_;
    };
}


#endif  //_DUMMY_INDEX_MANAGER_H_
