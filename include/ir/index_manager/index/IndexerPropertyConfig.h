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
            forward_(false),
            laInput_(false)
    { }

    IndexerPropertyConfig(const IndexerPropertyConfig& other)
            :propertyId_(other.propertyId_),
             propertyName_(other.propertyName_),
             index_(other.index_),
             forward_(other.forward_),
             laInput_(other.laInput_)
    {}

    IndexerPropertyConfig(unsigned int propertyid, std::string propertyname, bool index, bool forward)
		:propertyId_(propertyid)
		,propertyName_(propertyname)
		,index_(index)
		,forward_(forward)
		,laInput_(false)
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

    void setIsLAInput( bool isLAInput)
    {
        laInput_ = isLAInput;
    }

    bool isLAInput() const
    {
        return laInput_;
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

    ///This field is for compatabile with SF1
    ///IndexManager permits two kinds of inputs:LAInput, or ForwardIndex
    ///The latter means the forwardindex of a document is generated outside IndexManager.
    bool laInput_;
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
