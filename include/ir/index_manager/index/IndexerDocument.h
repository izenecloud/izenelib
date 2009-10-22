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

typedef boost::variant<int64_t,uint64_t, float, double, String> PropertyType;
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
