/**
* @file        IndexerPropertyConfig.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   The main configuration interface exposed to user of IndexManager
* it is used to describe the attibutes of properties in IndexerDocument
*/
#ifndef INDEXER_PROPERTYCONFIG_H
#define INDEXER_PROPERTYCONFIG_H

#include <types.h>
#include <string>
#include <sstream>
#include <vector>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class IndexerPropertyConfig
{
public:
    IndexerPropertyConfig()
            :propertyId_(0),
            index_(false),
            analyzed_(false),
            filter_(false)
    { }

    IndexerPropertyConfig(const IndexerPropertyConfig& other)
            :propertyId_(other.propertyId_),
             propertyName_(other.propertyName_),
             index_(other.index_),
             analyzed_(other.analyzed_),
             filter_(other.filter_)
    {}

    IndexerPropertyConfig(unsigned int propertyid, std::string propertyname, bool index, bool analyzed, bool filter = false)
		:propertyId_(propertyid)
		,propertyName_(propertyname)
		,index_(index)
		,analyzed_(analyzed)
		,filter_(filter)
    { }

public:
    void setPropertyId( uint32_t id )
    {
        propertyId_ = id;
    }

    uint32_t getPropertyId() const
    {
        return propertyId_;
    }

    void setName( const std::string & name )
    {
        propertyName_ = name;
    }

    std::string getName() const
    {
        return propertyName_;
    }

    void setIsIndex( const bool isIndex )
    {
        index_ = isIndex;
    }

    bool isIndex() const
    {
        return index_;
    }

    void setIsAnalyzed( const bool isAnalyzed)
    {
        analyzed_ = isAnalyzed;
    }

    bool isAnalyzed() const
    {
        return analyzed_;
    }

    void setIsFilter( const bool isFilter)
    {
        filter_ = isFilter;
    }

    bool isFilter() const
    {
        return filter_;
    }
		
    std::string toString() const
    {
        std::stringstream sStream;
        sStream << "[IndexerPropertyConfig] @id=" << propertyId_
        << " @name=" << propertyName_
        << " @type=";


        sStream << " @index=" << ( index_ ? "yes" : "no" )
        << " @analyzed=" << ( analyzed_ ? "yes" : "no" );

        return sStream.str();
    }
    bool operator==(const IndexerPropertyConfig & rhs) const
    {
        return ( propertyName_ == rhs.propertyName_);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & propertyId_;
        ar & propertyName_;
        ar & index_;
        ar & analyzed_;
    }

    bool operator<(const IndexerPropertyConfig & other) const
    {
        return ( propertyName_ < other.propertyName_);
    }

protected:
    friend class boost::serialization::access;

    uint32_t propertyId_;

    std::string propertyName_;
    /// if only true, then inverted or BTree index will be built
    bool index_;
    ///whether this property is analyzed by LA,
    ///inverted index will only be built when both index_ and analyzed_ are true
    bool analyzed_;
    ///whether filter index is going to be built on this property,
    ///BTree index will only be built when both index_ and filter_ are true
    bool filter_;
};

struct IndexerPropertyConfigComp
{
    bool operator()(const IndexerPropertyConfig & lhs, const IndexerPropertyConfig & rhs) const
    {
        return ( lhs.getName() < rhs.getName() );
    }
};

}

NS_IZENELIB_IR_END

#endif
