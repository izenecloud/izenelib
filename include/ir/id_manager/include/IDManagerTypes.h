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
#include <am/util/Wrapper.h>
#include <sdb/SequentialDB.h>
#include <wiselib/ustring/UString.h>
#include <am/tokyo_cabinet/tc_hash.h>
#include <am/trie/b_trie.hpp>

namespace idmanager {

template<typename NameString> struct NameHook {
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

template<typename NameID> struct IDHook {
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


BEGIN_SERIALIZATION

using namespace idmanager;


template<> inline void read_image<NameHook<wiselib::UString> >(NameHook<wiselib::UString>& dat, const DbObjPtr& ptr) {
	char* p = (char*)ptr->getData();
	memcpy(&dat.collId, p, sizeof(unsigned int));
	p += sizeof(unsigned int);	
	DbObjPtr ptr1;
	ptr1.reset(new DbObj(p, ptr->getSize()-sizeof(unsigned int)));
	read_image(dat.docName, ptr1);
}

template<> inline void write_image<NameHook<wiselib::UString> >(const NameHook<wiselib::UString>& dat, DbObjPtr& ptr) {
	DbObjPtr ptr1;
	ptr1.reset(new DbObj);
	write_image(dat.docName, ptr1);
	unsigned int size = sizeof(unsigned int) + ptr1->getSize();
	char *buf = new char[size];
	char *p = buf;
	memcpy(p, &dat.collId, sizeof(unsigned int) );
	p += sizeof(unsigned int);
	memcpy(p, ptr1->getData(), ptr1->getSize() );
	ptr->setData(buf, size);
	delete buf;
	buf = 0;
	
	//test read_image, write_image.
	//NameHook<wiselib::UString> dat1;
	//read_image(dat1, ptr);
	//assert(dat.compare(dat1) == 0);
	//cout<<dat1.collId<<endl;
	
}


template<> inline void read_image<IDHook<unsigned int>  >(IDHook<unsigned int>& dat, const DbObjPtr& ptr) {
	memcpy(&dat, ptr->getData(), ptr->getSize());
}

template<> inline void write_image<IDHook<unsigned int> >(const IDHook<unsigned int>& dat, DbObjPtr& ptr) {
	ptr->setData(&dat, sizeof(IDHook<unsigned int> ));
}

END_SERIALIZATION

#endif /*IDMANAGERTYPES_H_*/
