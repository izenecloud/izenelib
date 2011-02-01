/**
 * @file    Document.h
 * @brief   Defines the dummy Document class
 * @author  MyungHyun Lee  (Kent)
 * @date    2009-01-30
 */

#ifndef _DUMMY_DOCUMENT_H_
#define _DUMMY_DOCUMENT_H_

//#include <message-framework/Serializable.h>
#include <boost/serialization/serialization.hpp>

#include <util/izene_serialization.h>
#include <string>

namespace sf1v5_dummy
{
    class Document 
    {
        public:
            Document();

            //
            void clear();


            //SET METHODS

            void setId( const unsigned int& id )
            {
                docId_ = id;
            }

            void setTitle( const std::string & title )
            {
                title_ = title;
            }

            void setContent( const std::string & content )
            {
                content_ = content;
            }


            //GET METHODS

            unsigned int getId() const
            {
                return docId_;
            }

            const std::string & getTitle() const
            {
                return title_;
            }

            const std::string & getContent() const
            {
                return content_;
            }


            bool operator() ( const Document & i, const Document & j );

        public:
            friend class boost::serialization::access;

            /**
             * @brief   Using boost::serialization for serializing this class
             */
            template <class Archive>
            void serialize( Archive & ar, const unsigned int version )
            {
                ar & docId_ & title_ & content_;
            }

            DATA_IO_LOAD_SAVE(Document, &docId_&title_&content_)
              

        private:
            unsigned int    docId_;
            std::string     title_;
            std::string     content_;
    };
}

MAKE_FEBIRD_SERIALIZATION(sf1v5_dummy::Document);


#endif  //_DUMMY_DOCUMENT_H_
