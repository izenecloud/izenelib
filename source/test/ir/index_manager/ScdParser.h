#ifndef __SCD__PARSER__H__
#define __SCD__PARSER__H__

#include <sf1v5/common/type_defs.h>

#include <wiselib/ustring/UString.h>

//#include <am/3rdparty/rde_hash.h>
#include <am/cccr_hash/cccr_hash.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>


typedef std::pair<wiselib::UString, wiselib::UString> FieldPair;
typedef vector< FieldPair > SCDDoc;
typedef boost::shared_ptr< SCDDoc > SCDDocPtr;
typedef std::pair<wiselib::UString,sf1v5::docid_t> DocIdPair;

enum SCD_TYPE
{
    NOT_SCD = 0,
    INSERT_SCD,
    UPDATE_SCD,
    DELETE_SCD,
    REPLACE_SCD
};


class ScdParser
{
    public:
        ScdParser();
        ScdParser(const wiselib::UString::EncodingType & encodingType);
        virtual ~ScdParser();
        
        bool checkSCDFormat( const string & file );
        static SCD_TYPE checkSCDType( const string & file );
        static bool compareSCD (const string & file1, const string & file2);


        /// @brief Release allocated memory and make it as initial state.
        void clear();

        /// @brief Read a SCD file and load data into memory.
        bool load(const std::string& path);
        
        long getFileSize(){
        	return size_;        	
        }

        /// @brief  A utility function to get all the DOCID values from an SCD
        bool getDocIdList( std::vector<wiselib::UString> & list );

        bool getDocIdList( std::vector<DocIdPair > & list );
        
        /// @brief  Reads a document from the loaded SCD file, when given a DOCID value. 
        //          prerequisites: SCD file must be loaded by load(), and getDocIdList() must be called.
        bool getDoc( const wiselib::UString & docId, SCDDoc& doc );

        /// @brief gets the encoding type from the config
        inline wiselib::UString::EncodingType& getEncodingType() {
            return encodingType_;
        };


        class iterator
        {
            public:
                iterator(long offset);

                iterator(ScdParser* pScdParser, unsigned int start_doc);

                iterator(const iterator& other);

                ~iterator();

                bool operator==(const iterator& other) const;

                bool operator!=(const iterator& other) const;

                void operator++();

                void operator++(int);

                SCDDocPtr operator*();

                long getOffset();

            private:
                SCDDoc* getDoc();
                /// @brief Read a next line from a SCD file.
                bool getNextLine();

                /// @brief Check whether a given string begins with a field name. (ex. <DOCID>520080922 returns true)
                bool hasField(const std::string& str);

                /// @brief From a given string, extrack field name and remains.
                bool splitFieldNameValue(const std::string& str, std::string& name, std::string& value);

                /// @brief Check whether a given field name is the first field for a document.
                /// DOCID field SHOULD precede any other fields.
                bool isDocumentBeginField(const std::string& name);

            private:
                ifstream* pfs_;

                long prevOffset_;

                long offset_;

                string line_;

                SCDDocPtr doc_;

                wiselib::UString::EncodingType codingType_;

        };  // class iterator

        iterator begin(unsigned int start_doc = 0);
        
        iterator end();
        
    private:

        ifstream fs_; 
        
        long size_;

        wiselib::UString::EncodingType encodingType_;

        //izenelib::am::rde_hash<wiselib::UString, long> docOffsetList_;
        izenelib::am::cccr_hash<wiselib::UString, long> * docOffsetList_;
        //izenelib::am::LinearHashTable<wiselib::UString, long> docOffsetList_;
};

        


#endif

