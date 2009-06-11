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
            forward_(false)
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

    void setIsForward( const bool isForward)
    {
        forward_ = isForward;
    }

    bool isForward() const
    {
        return forward_;
    }

    std::string toString() const
    {
        std::stringstream sStream;
        sStream << "[IndexerPropertyConfig] @id=" << propertyId_
        << " @name=" << propertyName_
        << " @type=";


        sStream << " @index=" << ( index_ ? "yes" : "no" )
        << " @forward=" << ( forward_ ? "yes" : "no" );

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
        ar & forward_;
    }

    bool operator<(const IndexerPropertyConfig & other) const
    {
        return ( propertyName_ < other.propertyName_);
    }

protected:
    friend class boost::serialization::access;

    uint32_t propertyId_;

    std::string propertyName_;

    bool index_;

    bool forward_;
};



}

NS_IZENELIB_IR_END

#endif
