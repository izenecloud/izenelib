/**
 * @file    ForwardIndex.h
 * @brief   Defines the dummy ForwardIndex class
 * @author  MyungHyun Lee (Kent)
 * @date    2009-01-30
 */

#ifndef _DUMMY_FORWARD_INDEX_H_
#define _DUMMY_FORWARD_INDEX_H_

//#include <message-framework/Serializable.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <util/izene_serialization.h>

#include <set>

#include <string>

namespace sf1v5_dummy
{
    /**
     * @brief   Stores a forward index of a single document
     * @details
     * A "forward index" is a mapping from document ID to the terms in that document
     */
    class ForwardIndex
    {
        public:
            ForwardIndex()
                : docId_(0)
            {}

            ForwardIndex( unsigned int id )
                : docId_(id)
            { }

            /**
             * @brief   Adds a term to the list of terms that this  document holds.
             */
            void addTerm( const std::string & term )
            {
                termList_.insert( term );
            }

            /**
             * @brief   Sets the document Id
             */
            void setDocId( unsigned int docId )
            {
                docId_ = docId;
            }

            /**
             * @brief   Returns the document ID of which this forward index belongs to
             */
            unsigned int getDocId() const
            {
                return docId_;
            }

            /**
             * @brief   Returns the list of terms that are in this forward index
             */
            const std::set<std::string> & getTermList() const
            {
                return termList_;
            }


        public:

            friend class boost::serialization::access;
            /**
             * @brief   Using boost::serialization for serializing this class
             */
            template <class Archive>
            void serialize( Archive & ar, const unsigned int version )
            {
                ar & docId_ & termList_;
            }
            
            DATA_IO_LOAD_SAVE(ForwardIndex, &docId_&termList_)  

        private:
            unsigned int            docId_;
            std::set<std::string>   termList_;
    };
}
 MAKE_FEBIRD_SERIALIZATION(sf1v5_dummy::ForwardIndex)
#endif  //_DUMMY_FORWARD_INDEX_H_
