/**
 * @file XmlConfigParser.cpp
 * @brief Implements XmlConfigParser class, which is a XML configuration file parser for SF-1 v5.0
 * @author MyungHyun (Kent)
 * @date 2008-09-05
 */

#include "XmlConfigParser.h"

#include <sf1v5/common/type_defs.h>

#include <iostream>

using namespace std;
using namespace ticpp;

namespace izenelib{ namespace ir { namespace indexmanager {

void downCase( std::string & str )
{
    for ( string::iterator it = str.begin(); it != str.end(); it++ )
    {
        *it = tolower(*it);
    }
}

XmlConfigParser::XmlConfigParser()
{
}

void XmlConfigParser::parseConfigFile( const string & fileName ) throw( ticpp::Exception )
{
    try
    {
        //declare the document for the configuration
        Document configDocument( fileName.c_str() );

        //actually load the information
        configDocument.LoadFile();

        // make sure the top level element is "SF1Config"; if it isn't, an exception is thrown
        Element *configRoot = configDocument.FirstChildElement( "SF1Config" );

        Element *system = configRoot->FirstChildElement( "System" );
        if ( configRoot->IterateChildren( "System", system ) != NULL )
        {
            throw ticpp::Exception( "Multiple <System> configurations" );
        }


        //#1 System
        parseSystemSettings( *system );



        Iterator<Element> collection( "Collection" );
        for ( collection = collection.begin(configRoot); collection != collection.end(); collection++ )
        {
            IndexerCollectionMeta collectionMeta;
            //#3 Collection
            parseCollectionSettings( *collection, collectionMeta );

            collectionMetaList_.push_back( collectionMeta );
        }


        //---------- SETTING COLLECTION DATA ----------

        //TODO: case where emtpy collectionMetaList causes exception
        if ( indexManagerConfig_.setCollectionMetaList( collectionMetaList_ ) == false )
        {
            throw ticpp::Exception( "Duplicate Collection names" );
        }


    }
    catch ( Exception & e)
    {
        //Removing ticpp tags from internal ticpp exceptions so that we do not expose internal library usages.
        e.m_details = e.m_details.substr( 0, e.m_details.rfind( "<ticpp." ) );
        throw e;
    }
}


//================================  SYSTEM SETTINGS  =====================================

void XmlConfigParser::parseSystemSettings( const ticpp::Element & system )
{
    Element * indexProcess = system.FirstChildElement("Indexer");
    if ( system.IterateChildren( "Indexer", indexProcess ) != NULL )
    {
        throw ticpp::Exception( "Multiple <Indexer> configurations" );
    }
    try
    {
        parseIndexProcess( *indexProcess );
    }
    catch ( ticpp::Exception & e )
    {
        //adding element info & removing ticpp tags
        e.m_details.insert( 0, "<Indexer> " );
        throw e;
    }

}


//TODO: FIX
void XmlConfigParser::parseIndexProcess( const ticpp::Element & indexProcess )
{
    Element * settings;

    //Index strategy
    settings = indexProcess.FirstChildElement("IndexStrategy");
    if ( indexProcess.IterateChildren( "IndexStrategy", settings ) != NULL )
    {
        throw ticpp::Exception( "Multiple <IndexStrategy> configurations" );
    }
    {
        int64_t memory;

        if ( (settings->GetAttribute( "memorypoolsize" )).empty() == false )
        {
            settings->GetAttribute( "memorypoolsize", &memory );
        }
        else
        {
            throw ticpp::Exception( "requires \"memorypoolsize\" setting" );
        }

        indexManagerConfig_.indexStrategy_.memory_ = memory;
    }
    indexManagerConfig_.indexStrategy_.indexDocLength_ = true;
    indexManagerConfig_.storeStrategy_.param_ = "file";
    indexManagerConfig_.mergeStrategy_.param_ = "default";

    Iterator<Element> directory("IndexDirectory");
    for (directory = directory.begin(&indexProcess);
         directory != directory.end(); ++directory)
    {
        indexManagerConfig_.indexStrategy_.indexLocation_ = directory->GetText();
    }
}




//================================  COLLECTION SETTINGS  =====================================

void XmlConfigParser::parseCollectionSettings( const ticpp::Element & collection, IndexerCollectionMeta & collectionMeta )
{

    string name, encoding, ranking;

    if ( (name = collection.GetAttribute( "name" )).empty() )
    {
        throw ticpp::Exception( "<Collection> requires a \"name\" setting" );
    }

    if ( (encoding = collection.GetAttribute( "encoding" )).empty() )
    {
        stringstream message;
        message << "<Collection@" << name << "> requires a \"encoding\" setting";
        throw ticpp::Exception( message.str() );
    }


    collectionMeta.setName( name );
    collectionMeta.setEncoding( encoding );

    //counts the number of document schema defined in one Collection
    unsigned int docSchemaCount = 0;

    Iterator<Element> documentSchema( "DocumentSchema" );


    //for now, only iterates once
    for ( documentSchema = documentSchema.begin( &collection ); documentSchema != documentSchema.end(); documentSchema++ )
    {
        //used to check duplicate property names
        set<string> propertyNameList;

        if ( docSchemaCount > 1 )
        {
            throw ticpp::Exception( "Too many document schema definitions in one Collection") ;
        }

        Iterator<Element> property( "Property" );

        for ( property = property.begin( documentSchema.Get() ); property != property.end(); property++ )
        {
            try
            {
                //PARSING PROPERTY CONFIGURATIOINS
                parseProperty( *property, collectionMeta, propertyNameList );
            }
            catch ( ticpp::Exception & e )
            {
                stringstream message;
                message << "<Collection@" << name << "> ";

                e.m_details.insert( 0, message.str() );

                throw e;
            }

        } //property iteration

        docSchemaCount++;


    } // document schema iteration

    if ( docSchemaCount < 1 )
    {
        throw ticpp::Exception( "Need one document schema definition in one Collection" );
    }
}



void XmlConfigParser::parseProperty(
    const ticpp::Element & property,
    IndexerCollectionMeta & collectionMeta,
    set<string> & propertyNameList
)
{
    //holds all the configuration data of this property
    IndexerPropertyConfig propertyConfig;

    string propertyName, type, index;
    int maxLen= 0;
    bool bIndex = false;
    PropertyDataType dataType = UNKNOWN_DATA_PROPERTY_TYPE;

    //get settings from xml file ----------------
    if ( (propertyName = property.GetAttribute( "name")).empty() )
    {
        throw ticpp::Exception( "Property requires a \"name\" setting" );
    }

    if ( (type = property.GetAttribute( "type" )).empty() )
    {
        stringstream message;
        message << "\"type\" setting is required in \"" << propertyName << "\"";
        throw ticpp::Exception( message.str() );
    }

    if ( (property.GetAttribute( "maxlen")).empty() == false )
    {
        property.GetAttribute( "maxlen", &maxLen );
    }
    else
    {
        stringstream message;
        message << "\"maxlen\" setting is required in \"" << propertyName << "\"";
        throw ticpp::Exception( message.str() );
    }

    if ( (index = property.GetAttribute( "index" )).empty() )
    {
        stringstream message;
        message << "\"index\" setting is required in \"" << propertyName << "\"";
        throw ticpp::Exception( message.str() );
    }

    //find right data types -------------------
    if ( type == "string" )
    {
        dataType = STRING_PROPERTY_TYPE;
    }
    else if ( type == "int" )
    {
        dataType = INT_PROPERTY_TYPE;
    }
    else if ( type == "float" )
    {
        dataType = FLOAT_PROPERTY_TYPE;
    }
    else if ( type == "nominal" )
    {
        dataType = NOMINAL_PROPERTY_TYPE;
    }
    else
    {
        throw ticpp::Exception( "given wrong property type" );
    }


    if ( index.compare("y") == 0 || index.compare("Y") == 0 ||
            index.compare("yes") == 0 || index.compare("YES") == 0 )
    {
        bIndex = true;
    }
    else if ( index.compare("n") == 0 || index.compare("N") == 0 ||
              index.compare("no") == 0 || index.compare("NO") == 0 )
    {
        bIndex = false;
    }



    //set data --------------------------------------
    propertyConfig.setName( propertyName );
    propertyConfig.setIsIndex( bIndex );



    //--- parse optional settings for "Property"

    //-------- Display ---------------
    //TODO: may not have display
    //TODO: only one <Display> setting is allowed

    Element * display = property.FirstChildElement( "Display", false );
    if ( display != NULL )
    {
        try
        {
            //parsePropertyDisplayConfig( *display, propertyConfig );
        }
        catch ( ticpp::Exception & e )
        {
            stringstream ss;
            ss << " in \"" << propertyName << "\"";
            e.m_details += ss.str();
            throw e;
        }

        //check for duplicates
        if ( display->NextSibling( "Display", false ) != NULL )
        {
            stringstream message;
            message << "\"" << propertyName << "\": Multiple <Display> configurations";
            throw ticpp::Exception( message.str() );
        }
    }


    //-------- Indexing ---------------
    //TODO: if the property is set index="no", then no <Indexing> options???
    //there are several cases where it is allowed. Need to identify & narrow the cases


    Element * indexing = property.FirstChildElement( "Indexing", false );

    //if <Indexing> element  exists
    if ( indexing != NULL )
    {
        Iterator<Element> indexing_it( "Indexing" );

        ///Yingfeng 2009-06-02, to be merged in sf1-revolution !!!!!!!!!!!!
        propertyConfig.setIsForward( true );

        // for all the <indexing> config
        for ( indexing_it = indexing_it.begin( &property ); indexing_it != indexing_it.end(); indexing_it++ )
        {
            if ( !bIndex )
            {
                //<Property name="Content" type="string" maxlen="40000" index="yes">
                cout << "[ConfigParser] Warning: <Indexing> configuration will be ignored "
                     << "in Property \"" << propertyName << "\"" << endl;
            }

            try
            {
                parsePropertyIndexingConfig( *indexing_it, propertyConfig );
            }
            catch ( ticpp::Exception & e )
            {
                stringstream ss;
                ss << " in \"" << propertyName << "\"";

                e.m_details = e.m_details.substr( 0, e.m_details.find_last_of( "ticpp" ) - 5 );
                e.m_details += ss.str();
                throw e;
            }

            //if there are duplicate property names(including alias)
            if ( (( propertyNameList.insert(propertyConfig.getName()) ).second) == false )
            {
                throw ticpp::Exception( "Duplicate property names (including alias)" );
            }

            //add this property setting(alias) to DocumentSchema
            collectionMeta.addPropertyConfig( propertyConfig );
        }

    }
    else
    {
        //add this property setting to DocumentSchema
        collectionMeta.addPropertyConfig( propertyConfig );
    }
}




void XmlConfigParser::parsePropertyIndexingConfig(
    const ticpp::Element & indexing,
    IndexerPropertyConfig & propertyConfig
)
{
    string alias, analyzer, regulator;
    float rankWeight;


    //------- read XML
    alias = indexing.GetAttribute( "alias" );

    if ( (analyzer = indexing.GetAttribute( "analyzer" )).empty() )
    {
        stringstream message;
        message << "\"" << propertyConfig.getName() << "\": \"analyzer\" setting is required";
        throw ticpp::Exception( message.str() );
    }

    regulator = indexing.GetAttribute( "regulator" );


    if ( indexing.GetAttribute( "rankweight" ).empty() == false )
    {
        indexing.GetAttribute( "rankweight", &rankWeight, false );
    }
    else
    {
        cout << "[ConfigParser] Warning: \"" << propertyConfig.getName() << "\": \"rankweight\" is given default value" << endl;
    }




    //SETS ALIAS AS NAME. Property name is saved as originalName
    //TODO: 2008-10-23
    if ( !alias.empty() )
    {
        propertyConfig.setName( alias );
    }

}


void parseByComma( const string & str, vector<string> & subStrList )
{
    subStrList.clear();

    std::size_t startIndex=0, endIndex=0;


    while ( (startIndex = str.find_first_not_of( " ,\t", startIndex )) != std::string::npos )
    {
        endIndex = str.find_first_of( " ,\t", startIndex + 1 );

        if ( endIndex == string::npos )
        {
            endIndex = str.length();
        }

        std::string substr = str.substr( startIndex, endIndex - startIndex );
        startIndex = endIndex + 1;

        subStrList.push_back( substr );
    }
}

}}}
