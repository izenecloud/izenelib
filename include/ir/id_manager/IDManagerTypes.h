/**
 * Using SDB/hash
 *
 * @Peisheng Wang
 * @date 2009-04-16
 *
 */
#ifndef IDMANAGERTYPES_H_
#define IDMANAGERTYPES_H_

//#include <vector>
//#include <list>
//#include <cstddef>

//#include <types.h>
//#include <am/util/Wrapper.h>
//#include <sdb/SequentialDB.h>
//#include <util/ustring/UString.h>
//#include <am/tokyo_cabinet/tc_hash.h>

//#include <am/trie/b_trie.hpp>
//using namespace std;
//
//NS_IZENELIB_IR_BEGIN
//
//namespace idmanager {
//
//template<typename NameString> struct NameHook {
//	unsigned int collId;
//	NameString docName;
//
//	template<class Archive> void serialize(Archive & ar,
//			const unsigned int version) {
//		ar & collId;
//		ar & docName;
//	}
//
//	int compare(const NameHook& other) const {
//		if (collId != other.collId)
//			return collId-other.collId;
//		else {
//			return docName.compare(other.docName);
//		}
//	}
//
//};
//
//template<typename NameID> struct IDHook {
//	unsigned int collId;
//	NameID docId;
//
//	template<class Archive> void serialize(Archive & ar,
//			const unsigned int version) {
//		ar & collId;
//		ar & docId;
//	}
//
//	int compare(const IDHook& other) const {
//		if (collId != other.collId)
//			return collId-other.collId;
//		else {
//			return docId-other.docId;
//		}
//	}
//};
//
//}
//
//NS_IZENELIB_IR_END
//
//using namespace izenelib::ir::idmanager;
//
//MAKE_MEMCPY_SERIALIZATION(NameHook<izenelib::util::UString>);
//
////for pod types
//MAKE_MEMCPY_TYPE(IDHook<unsigned int> );
//
//NS_IZENELIB_UTIL_BEGIN
//
//
//template<> inline void read_image_memcpy<NameHook<izenelib::util::UString> >(NameHook<izenelib::util::UString>& dat, const char* str, const size_t size) {
//	char* p = (char*)str;
//	memcpy(&dat.collId, p, sizeof(unsigned int));
//	p += sizeof(unsigned int);
//	read_image_memcpy(dat.docName, p, size-sizeof(unsigned int));
//}
//
//template<> inline void write_image_memcpy<NameHook<izenelib::util::UString> >(const NameHook<izenelib::util::UString>& dat, char* &str,
//		size_t &size){
//	char* uptr;
//	size_t usz;
//
//	write_image_memcpy(dat.docName, uptr, usz);
//
//	size = sizeof(unsigned int) + usz;
//	str = new char[size];
//
//	char *p = str;
//	memcpy(p, &dat.collId, sizeof(unsigned int) );
//	p += sizeof(unsigned int);
//	memcpy(p, uptr, usz);
//
//	delete uptr;
//	uptr = 0;
//
//	//test read_image, write_image.
//	/*NameHook<izenelib::util::UString> dat1;
//	read_image_memcpy(dat1, str, size);
//	assert(dat.compare(dat1) == 0);
//	cout<<dat1.collId<<endl;*/
//
//}
//
//
//NS_IZENELIB_UTIL_END

/*
BEGIN_SERIALIZATION

using namespace idmanager;


template<> inline void read_image<NameHook<izenelib::util::UString> >(NameHook<izenelib::util::UString>& dat, const DbObjPtr& ptr) {
	char* p = (char*)ptr->getData();
	memcpy(&dat.collId, p, sizeof(unsigned int));
	p += sizeof(unsigned int);
	DbObjPtr ptr1;
	ptr1.reset(new DbObj(p, ptr->getSize()-sizeof(unsigned int)));
	read_image(dat.docName, ptr1);
}

template<> inline void write_image<NameHook<izenelib::util::UString> >(const NameHook<izenelib::util::UString>& dat, DbObjPtr& ptr) {
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
	//NameHook<izenelib::util::UString> dat1;
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
*/

#endif /*IDMANAGERTYPES_H_*/
