#ifndef __SCD__PARSER__H__
#define __SCD__PARSER__H__


#include <wiselib/ustring/UString.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>


typedef std::pair<wiselib::UString, wiselib::UString> FieldPair;
typedef vector< FieldPair > SCDDoc;
typedef boost::shared_ptr< SCDDoc > SCDDocPtr;

class ScdParser
{
    public:
        ScdParser();
        ScdParser(wiselib::UString::EncodingType & encodingType);
        virtual ~ScdParser();

        /// @brief Release allocated memory and make it as initial state.
        void clear();

        /// @brief Read a SCD file and load data into memory.
        bool load(const std::string& path);
        
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
            
            long offset_;
            
            string line_;
            
            SCDDocPtr doc_;

            wiselib::UString::EncodingType codingType_;
            
        };

        iterator begin(unsigned int start_doc = 0);
        
        iterator end();
        
    private:
        ifstream fs_; 
        
        long size_;

        wiselib::UString::EncodingType encodingType_;
};


#endif

