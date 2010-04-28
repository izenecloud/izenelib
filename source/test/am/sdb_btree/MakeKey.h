/**
 * @file    MakeKey.h
 * @brief   headerfile for creating Key
 * @author  Deepesh
 * @date    20th April 2009
 * @details
 *
 * -Log
 *
 *
 *
 *
 */

#ifndef _MAKEKEY_
#define _MAKEKEY_

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>

class MakeKey {
public:
	MakeKey() {
		collId_ = 0;
		docId_ = 0;
		propertyId_ = 0;
	}

	MakeKey(unsigned int collId, const unsigned int docId,
			const unsigned int pId) :
		collId_(collId), docId_(docId), propertyId_(pId) {

	}

	~MakeKey() {
	}

	void print() const {
		std::cout << "KeyValue " << collId_ << "  " << docId_ << "  "
				<< propertyId_ <<std::endl;
	}

	//
	//Collection Identifier
	//
	unsigned int collId_;

	//
	//Document Identifier
	//

	unsigned int docId_;

	//
	//PropertyId
	//
	unsigned int propertyId_;

	bool createKey(unsigned int collId, const unsigned int docId,
			const unsigned int pId) {
		collId_ = collId;
		docId_ = docId;
		propertyId_ = pId;

		return true;
	}

	bool clear() {
		collId_ = 0;
		docId_ = 0;
		propertyId_ = 0;
		return true;

	}

	template <class Archive> void serialize(Archive& ar,
			const unsigned int version) {
		ar& collId_;
		ar& docId_;
		ar& propertyId_;

	}
	inline int compare(const MakeKey& other) const {
		//return memcmp(this, &other, sizeof(MakeKey));

		if (collId_ != other.collId_) {
			return (collId_ - other.collId_);
		} else if (docId_ != other.docId_) {
			return (docId_ - other.docId_);
		} else {
			return (propertyId_ - other.propertyId_);
		}

	}
private:
	friend class boost::serialization::access;

}; // end class MakeKey

MAKE_MEMCPY_TYPE(MakeKey)

#endif
