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
#include <util/BoostVariantUtil.h>

#include <boost/variant.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/vector.hpp> 
#include <boost/serialization/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <map>
#include <list>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{


///MultiValuePropertyType is splitted by ","
typedef std::list<PropertyType > MultiValuePropertyType;

///A property that both inverted index and BTree index will be built
typedef std::pair<boost::shared_ptr<LAInput>, PropertyType >  IndexPropertyType;

///A multivalueproperty that both inverted index and BTree index will be built
typedef std::pair<boost::shared_ptr<LAInput>, MultiValuePropertyType >  MultiValueIndexPropertyType;

///A ptoperty type that support inverted index, BTree index, and both index (inverted and BTree)
typedef boost::variant<boost::shared_ptr<LAInput>, PropertyType, MultiValuePropertyType, IndexPropertyType, MultiValueIndexPropertyType > IndexerDocumentPropertyType;

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
    IndexerDocument():id_(0),docId_(0),colId_(0){}

    IndexerDocument(unsigned int docId, unsigned int colId):id_(0),docId_(docId),colId_(colId){}
	
    ~IndexerDocument(){}
public:
    void setId(docid_t id) {id_ = id;}

    docid_t getId() {return id_;}

    void setDocId(unsigned int docId, unsigned int colId){docId_ = docId;colId_ = colId;}

    void getDocId(DocId& docId){ docId.docId = docId_; docId.colId = colId_;}

    ///insert data to IndexerDocument
    template<typename T>
    bool insertProperty(const IndexerPropertyConfig& config, T& property)
    {
        IndexerDocumentPropertyType p(property);
        propertyList_.push_back(std::make_pair(config, p));
        termIterator_ = propertyList_.rbegin();
        return true;
    }

    ///This interface is dedicated to the data with type of LAInput, because it can reduce
    ///lots of data replication. We should use this interface in this sequence:
    ///1. insertProperty
    ///2. add_to_property
    void add_to_property(LAInputUnit& unit)
    {
        izenelib::util::boost_variant_visit(boost::bind(deque_visitor(), _1, unit), termIterator_->second);
    }

    std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >& getPropertyList()
    {
        return propertyList_;
    }

    void to_map(std::map<IndexerPropertyConfig, IndexerDocumentPropertyType>& docMap)
    {
        std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >::iterator
            it = propertyList_.begin();
        for(; it!= propertyList_.end(); ++it)
        {
            docMap[it->first] = it->second;
        }
    }

    void swap(IndexerDocument& rhs)
    {
        using std::swap;
        swap(id_, rhs.id_);
        swap(docId_, rhs.docId_);
        swap(colId_, rhs.colId_);
        swap(propertyList_, rhs.propertyList_);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & docId_;
        ar & colId_;
        ar & propertyList_;
    }

    bool empty() { return propertyList_.empty();}

    void clear() 
    {
        id_ = 0;
        docId_ = 0;
        colId_ = 0;
        propertyList_.clear();
    }
private:
	friend class boost::serialization::access;
	docid_t id_;
	
	docid_t docId_;
	
	docid_t colId_;
	
	std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> > propertyList_;

	std::list<std::pair<IndexerPropertyConfig, IndexerDocumentPropertyType> >::reverse_iterator termIterator_;
};	
}

NS_IZENELIB_IR_END

#endif
