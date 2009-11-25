/**
* @file        IndexerDocument.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   The main interface exposed to user of IndexManager
* it encapsulates the Document entity to serve for the usage of IndexManager
*/
#ifndef INDEXERDOCUMENT_H
#define INDEXERDOCUMENT_H

#include <ir/index_manager/utility/system.h>

#include <ir/index_manager/index/IndexerPropertyConfig.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/ForwardIndex.h>

#include <util/BoostVariantUtil.h>

#include <boost/variant.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/map.hpp> 
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///PropertyType is the data supported to set up BTree index
typedef boost::variant<int64_t,uint64_t, float, double, String> PropertyType;
///Besides the type for building BTree index, we have two other kinds of type dedicated for analyzed properties,
///which means inverted indices would be built
typedef boost::variant<PropertyType, boost::shared_ptr<LAInput>, boost::shared_ptr<ForwardIndex> > IndexerDocumentPropertyType;

struct DocId
{
    unsigned int docId;
    unsigned int colId;
};

class deque_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(T&v, LAInputUnit& u)
    {
    }

    void operator()(boost::shared_ptr<LAInput>& v, LAInputUnit& u)
    {
        v->push_back(u);
    }
};

class IndexerDocument
{
public:
    IndexerDocument():docId_(0),colId_(0){}

    IndexerDocument(unsigned int docId, unsigned int colId):docId_(docId),colId_(colId){}
	
    ~IndexerDocument(){}
public:
    void setDocId(unsigned int docId, unsigned int colId){docId_ = docId;colId_ = colId;}

    void getDocId(DocId& docId){ docId.docId = docId_; docId.colId = colId_;}

    ///insert data to IndexerDocument
    template<typename T>
    bool insertProperty(const IndexerPropertyConfig& config, T& property)
    {
        IndexerDocumentPropertyType p(property);
        std::pair<std::map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator, bool> ret 
			= propertyList_.insert(std::make_pair(config, p));
        if(ret.second)
        {
            termIterator_ = ret.first;
            return true;
        }
        else
            return false;
    }

    ///This interface is dedicated to the data with type of LAInput, because it can reduce
    ///lots of data replication. We should use this interface in this sequence:
    ///1. insertProperty
    ///2. add_to_property
    void add_to_property(LAInputUnit& unit)
    {
        izenelib::util::boost_variant_visit(boost::bind(deque_visitor(), _1, unit), termIterator_->second);
    }

    void getPropertyList(map<IndexerPropertyConfig, IndexerDocumentPropertyType>& propertyList)
    {
        propertyList = propertyList_;
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & docId_;
        ar & colId_;
        ar & propertyList_;
    }

private:
	friend class boost::serialization::access;
	
	unsigned int docId_;
	
	unsigned int colId_;
	
	map<IndexerPropertyConfig, IndexerDocumentPropertyType> propertyList_;

	map<IndexerPropertyConfig, IndexerDocumentPropertyType>::iterator termIterator_;
};	
}

NS_IZENELIB_IR_END

#endif
