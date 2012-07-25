#if !defined(_MFSERIALIZATION_H)
#define _MFSERIALIZATION_H

#include <net/message-framework/MFBuffer.h>
#include <util/izene_serialization.h>
#include <util/hashFunction.h>

#include <net/message-framework/ServiceMessage.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fstream>

using namespace izenelib::util;

namespace messageframework {

#ifdef USE_MF_LIGHT

template<typename TYPE> inline void mf_deserialize(TYPE& dat,
		const ServiceMessagePtr& sm, int idx=0) {

	MFBufferPtr ptr = sm->getBuffer(idx);
	dat = boost::any_cast<TYPE>(*ptr);

#ifdef SF1_DEBUG
	cout<<"mf_deserialize "<<typeid(TYPE).name()<<endl;
	cout<<"!!!!--- hash code ----: "<<izenelib::util::izene_hashing(dat)<<endl;
	sm->display();
#endif


}

template<typename TYPE> inline void mf_serialize(const TYPE& dat,
		ServiceMessagePtr& sm, int idx=0) {

	MFBufferPtr ptr(new boost::any(dat));
	sm->setBuffer(idx, ptr);

#ifdef SF1_DEBUG
	cout<<"mf_serialize "<<typeid(TYPE).name()<<endl;
	cout<<"!!!!==>> hash code: "<<izenelib::util::izene_hashing(dat)<<endl;
	TYPE dat1;
	mf_deserialize(dat1, sm, idx);
	cout<<"!!!!<<== hash code: "<<izenelib::util::izene_hashing(dat1)<<endl;
#endif
}

#else

template<typename TYPE> inline void mf_deserialize(TYPE& dat,
		const ServiceMessagePtr& sm, int idx=0) {

	MFBufferPtr ptr = sm->getBuffer(idx);
	izene_deserialization<TYPE> izd((char*)ptr->getData(), ptr->getSize() );
	izd.read_image(dat);

#ifdef SF1_DEBUG
	cout<<"mf_deserialize "<<typeid(TYPE).name()<<endl;
	cout<<"!!!!--- hash code ----: "<<izenelib::util::izene_hashing(dat)<<endl;
	sm->display();
#endif

}

template<typename TYPE> inline void mf_serialize(const TYPE& dat,
		ServiceMessagePtr& sm, int idx=0) {

	char* buf = 0;
	size_t len = 0;
	izene_serialization<TYPE> izs(dat);
	izs.write_image(buf, len);
	MFBufferPtr ptr(new MFBuffer(buf, len));
	sm->setBuffer(idx, ptr);

#ifdef SF1_DEBUG

	cout<<"mf_serialize "<<typeid(TYPE).name()<<endl;
	cout<<"!!!!==>> hash code: "<<izenelib::util::izene_hashing(dat)<<endl;
	TYPE dat1;
	mf_deserialize(dat1, sm, idx);
	cout<<"!!!!<<== hash code: "<<izenelib::util::izene_hashing(dat1)<<endl;
	//assert( izenelib::util::izene_hashing(dat) == izenelib::util::izene_hashing(dat1) );
	//assert(sizeof(dat1) == sizeof(dat));
#endif
}

#endif


#ifndef USE_MF_LIGHT

const int archive_flags = boost::archive::no_header | boost::archive::no_codecvt;

template<typename DataType> inline void from_buffer(DataType&sm,
		const boost::shared_ptr<izenelib::util::izene_streambuf>& pbuf) {
	{
		boost::archive::binary_iarchive archive(*pbuf, archive_flags);
		archive >> sm;
	}
#ifdef SF1_DEBUG
	cout<<"!!! from_buffer "<<typeid(DataType).name()<<endl;
	cout<<"!!!  buffsize="<<pbuf->size()<<endl;
	cout<<"!!!! <<== hash code: "<<izenelib::util::izene_hashing(sm )<<endl;
#endif

}

template<typename DataType> inline void to_buffer(const DataType&sm,
		boost::shared_ptr<izenelib::util::izene_streambuf>& pbuf) {
	{
		boost::archive::binary_oarchive archive(*pbuf, archive_flags);
		// read data of DataType
		archive << sm;
	}
#ifdef SF1_DEBUG
	cout<<"!!! to_buffer "<<typeid(DataType).name()<<endl;
	cout<<"!!!  buffsize="<<pbuf->size()<<endl;

	cout<<"!!!!==>> hash code: "<<izenelib::util::izene_hashing(sm )<<endl;
#endif

}

#if 1

template<> inline void from_buffer<ServiceMessage>(ServiceMessage &sm,
		const boost::shared_ptr<izenelib::util::izene_streambuf>& pbuf) {
#ifdef SF1_DEBUG
	cout<<"DBG from Buffer: hash Value:  "<<izenelib::util::sdb_hash_fun(
			(void*)pbuf->data(), pbuf->size() )<<endl;
#endif

	pbuf->sgetn((char*)&sm.mh, sizeof(MessageHeader));
	MessageHeader& mh = sm.mh;
	char* buf = new char[mh.serviceNameSize+1];
	memset(buf, 0, mh.serviceNameSize+1);
	pbuf->sgetn(buf, mh.serviceNameSize);

	sm.setServiceName(string(buf));
	delete buf;
	buf = 0;
	sm.bufferPtrVec.resize(mh.nbuffer);
	//sm.display();

	for (unsigned int i = 0; i<mh.nbuffer; i++) {
		unsigned int sz;
		pbuf->sgetn((char*)&sz, sizeof(size_t));
		MFBufferPtr ptr(new MFBuffer);
		ptr->data = new char[sz];
		ptr->size = sz;
		pbuf->sgetn((char*)ptr->data, sz);
		sm.setBuffer(i, ptr);
	}
}


template<> inline void to_buffer<ServiceMessage>(const ServiceMessage& sm,
		boost::shared_ptr<izenelib::util::izene_streambuf> &pbuf) {
	std::string serviceName;
	serviceName = sm.getServiceName();
	pbuf->sputn((char*)&sm.mh, sizeof(MessageHeader));
	pbuf->sputn(serviceName.c_str(), serviceName.size());
	for (unsigned int i=0; i<sm.getBufferNum(); i++) {
		size_t sz = sm.getBuffer(i)->getSize();
		pbuf->sputn((char*)&sz, sizeof(size_t));
		pbuf->sputn((char*)sm.getBuffer(i)->getData(), sz);
	}

#ifdef SF1_DEBUG
	cout<<"DBG from Buffer: hash Value: "<<izenelib::util::sdb_hash_fun(
			(void*)pbuf->data(), pbuf->size() )<<endl;
#endif
}
#endif

#endif

}

#endif
