/** 
 * Using SDB/hash
 *  
 * @Peisheng Wang
 * @date 2009-04-16
 *
 */
#ifndef IDMANAGERTYPES_H_
#define IDMANAGERTYPES_H_

#include <vector>
#include <list>
#include <sdb/SequentialDB.h>
#include <am/tokyo_cabinet/tc_hash.h>

namespace idmanager {

template<typename NameString>
struct NameHook {
	unsigned int collId;
	NameString docName;

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & collId;
		ar & docName;
	}

	int compare(const NameHook& other) const {
		if (collId != other.collId)
			return collId-other.collId;
		else {
			return docName.compare(other.docName);
		}
	}

};

template<typename NameID>
struct IDHook {
	unsigned int collId;
	NameID docId;	

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & collId;
		ar & docId;
	}

	int compare(const IDHook& other) const {
		if (collId != other.collId)
			return collId-other.collId;
		else {
			return docId-other.docId;
		}
	}
};

}

using namespace idmanager;

namespace izenelib {
namespace am {
namespace util {

/*
 template<> inline void read_image<NameHook>(NameHook& dat, const DbObjPtr& ptr) {
 
 }

 template<> inline void write_image<NameHook>(const NameHook& dat, DbObjPtr& ptr) {
 
 }*/

/*
 template<> inline void read_image<wiselib::UString>(wiselib::UString& dat,
 const DbObjPtr& ptr) {
 dat = (wiselib::UString)(ptr->getSize, (char*)ptr->getData() );
 }

 template<> inline void write_image<wiselib::UString>(
 const wiselib::UString& dat, DbObjPtr& ptr) {
 ptr->setData(dat.c_str(), dat.length()+1);
 }*/


/*
template<> inline void read_image<IDHook>(IDHook& dat, const DbObjPtr& ptr) {
memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<IDHook>(const IDHook& dat, DbObjPtr& ptr) {
ptr->setData(&dat, sizeof(IDHook));
}*/


}
}
}

#endif /*IDMANAGERTYPES_H_*/
