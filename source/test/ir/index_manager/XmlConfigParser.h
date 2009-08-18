/**
 * @file XmlConfigParser.h
 * @brief Defines XmlConfigParser class, which is a XML configuration file parser for SF-1 v5.0
 * @author MyungHyun (Kent)
 * @date 2008-09-05
 */

#ifndef _XML_CONFIG_PARSER_H_
#define _XML_CONFIG_PARSER_H_

#include <wiselib/ticpp/ticpp.h>

#include <ir/index_manager/utility/IndexManagerConfig.h>

#include <string>
#include <sstream>
#include <map>

using namespace ticpp;

namespace izenelib { namespace ir { namespace indexmanager {

enum PropertyDataType
{
	STRING_PROPERTY_TYPE = 0,
	INT_PROPERTY_TYPE,
	FLOAT_PROPERTY_TYPE,
	NOMINAL_PROPERTY_TYPE,
	UNKNOWN_DATA_PROPERTY_TYPE,
	// document-manager's  internal data types
	//FLOAT_TYPE = 0,
	//INT_TYPE,
	UNSIGNED_INT_PROPERTY_TYPE,
	DOUBLE_PROPERTY_TYPE,
	FORWARD_INDEX_PROPERTY_TYPE,
	CBIT_ARRAY_PROPERTY_TYPE,
	//STRING_TYPE,
	USTRING_PROPERTY_TYPE,
	USTRING_ARRAY_PROPERTY_TYPE,
	VECTOR_UNSIGNED_INT_TYPE
	//UNKNOWN_DATA_TYPE
};

/**
 * @brief   This class parses a SF-1 v5.0 configuration file, in the form of a xml file
 */
class XmlConfigParser
{

public:
    //----------------------------  PUBLIC FUNCTIONS  ----------------------------

    XmlConfigParser();

    ~XmlConfigParser() {}


    void parseConfigFile( const std::string & fileName ) throw(ticpp::Exception);


    const IndexManagerConfig & getIndexManagerConfig( )
    {
        return indexManagerConfig_;
    }
    void getIndexManagerConfig( IndexManagerConfig & indexManagerConfig )
    {
        indexManagerConfig = indexManagerConfig_;
    }



private:
    void parseSystemSettings( const ticpp::Element & system );

    void parseIndexProcess( const ticpp::Element & indexProcess );

    void parseCollectionSettings( const ticpp::Element & collection, IndexerCollectionMeta & collectionMeta );

    void parseProperty( const ticpp::Element & property, IndexerCollectionMeta & collectionMeta,std::set<std::string> & propertyNameList );

    void parsePropertyIndexingConfig(const ticpp::Element & indexing, IndexerPropertyConfig & propertyData);

private:
    IndexManagerConfig indexManagerConfig_;

    std::vector<IndexerCollectionMeta> collectionMetaList_;
};

void parseByComma( const std::string & str, std::vector<std::string> & subStrList );


}}}

#endif //_XML_CONFIG_PARSER_H_

