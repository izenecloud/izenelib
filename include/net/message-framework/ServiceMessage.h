///
///  @file : ServiceMessage.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#ifndef _SERVICE_MESSAGE_H_
#define _SERVICE_MESSAGE_H_

/************* Include boost header files *****************/
#include <boost/shared_ptr.hpp>
#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/MFBuffer.h>

/************* Include std header files *****************/
#include <string>
#include <vector>
#include <cstdlib>

#include <boost/serialization/array.hpp>
#include <boost/serialization/split_member.hpp>

using namespace std;

namespace messageframework {

struct MessageHeader {
	unsigned int requestId;
	unsigned int minorId;
	unsigned int nbuffer;
	unsigned int serviceNameSize;
	//unsigned int bufferSize;

	template <typename Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & requestId & minorId & nbuffer & serviceNameSize;
	}
	MessageHeader() :
		requestId(0), minorId(0), nbuffer(0), serviceNameSize(0) {
	}
};

/**
 * @brief This class defines a message transfered between MessageClient, MessageController,
 * and MessageServer then MessgeClient requests a result of service. The message is generic
 * message, it can be a request or a result.
 */
class ServiceMessage {
	
	friend class ServiceResultCompare;
public:
	/**
	 * @brief The constructor, initialize variables with default values.
	 */
	ServiceMessage() {
	}

	/**
	 * @brief Destroy variables if it is neccessary.
	 */
	~ServiceMessage() {
		//std::cout<<"~ServiceMessage"<<std::endl;	
		//display();
	}

	//	int getStatus();

	//	void setStatus(int status);

	unsigned int getRequestId() const {
		return mh.requestId;
	}

	void setRequestId(unsigned int requestId) {
		mh.requestId = requestId;
	}

	unsigned int getMinorId() const {
		return mh.minorId;
	}

	void setMinorId(unsigned int minorId) {
		mh.minorId = minorId;
	}

	const MessageFrameworkNode getRequester() const {
		return requester_;
	}

	void setRequester(const MessageFrameworkNode &requester) {
		requester_ = requester;
	}

	/**
	 * @brief Set the service name
	 * @param
	 * serviceName - the service name
	 */
	void setServiceName(const std::string& servicename) {
		serviceName = servicename;
		mh.serviceNameSize = serviceName.size();
	}

	/**
	 * @brief get the service name
	 * @return
	 * The service name
	 */
	const std::string& getServiceName() const {
		return serviceName;
	}
	
	/**
	 * @brief Set the agentInfo
	 * @param
	 * serviceName - the service name
	 */
	void setAgentInfo(const std::string& agentInfo) {
		agentInfo_ = agentInfo;		
	}

	/**
	 * @brief get the agent Info
	 * @return
	 * The service name
	 */
	const std::string& getAgentInfo() const {
		return agentInfo_;
	}
	

	unsigned int getBufferNum() const {
		assert(mh.nbuffer == bufferPtrVec.size());
		return mh.nbuffer;
	}
	
	void setBufferNum(unsigned int sz)  {
		//assert(mh.nbuffer == bufferPtrVec.size());
		bufferPtrVec.resize(sz);
		mh.nbuffer = bufferPtrVec.size();
	}

	MFBufferPtr getBuffer(unsigned int idx) const {
		assert(idx < mh.nbuffer);
		return bufferPtrVec[idx];
	}

	void setBuffer(unsigned int idx, const MFBufferPtr &ptr) {
		assert(idx >=0 && idx <= mh.nbuffer);
		/* if(bufferPtrVec.size() < mh.nbuffer)
		 {
		 bufferPtrVec.resize(mh.nbuffer);  	      
		 } 
		 mh.nbuffer = bufferPtrVec.size();*/
		if ( (unsigned int)idx == mh.nbuffer) {
			pushBuffer(ptr);
		} else {
			bufferPtrVec[idx] = ptr;
		}
	}
	
	void setBuffer(const std::vector<MFBufferPtr>& buffers) {
		bufferPtrVec = buffers;	
		mh.nbuffer = buffers.size();
	}
	

	/*void pushBuffer(char* ptr, unsigned int sz) {
		MFBufferPtr dptr(new MFBuffer(ptr, sz));
		bufferPtrVec.push_back(dptr);
		//mh.nbuffer = bufferPtrVec.size();
		//++mh.nbuffer;
		if (mh.nbuffer < bufferPtrVec.size() ) {
			mh.nbuffer = bufferPtrVec.size();
		}
	}*/

	void pushBuffer(const MFBufferPtr& ptr) {
		bufferPtrVec.push_back(ptr);
		//mh.nbuffer = bufferPtrVec.size();
		//++mh.nbuffer;
		if (mh.nbuffer < bufferPtrVec.size() ) {
			mh.nbuffer = bufferPtrVec.size();
		}
	}

#ifndef USE_MF_LIGHT
	
	unsigned int getSerializedSize() const {
		unsigned int len = 0;
		len += sizeof(MessageHeader);
		len += mh.serviceNameSize;
		for (unsigned int i=0; i<mh.nbuffer; i++) {
			len += sizeof(size_t);
			assert(bufferPtrVec[i]);
			len += bufferPtrVec[i]->getSize();
		}
		return len;

	}	

	/*	template <typename Archive> void serialize(Archive & ar,
	 const unsigned int version) {
	 assert(false);
	 }*/

	/**
	 * @brief Append a value to the data list of a service
	 * @param
	 * value - the parameter value
	 */
	// void appendData(VariantType* value);
	//void appendData(const boost::shared_ptr<VariantType>& value);

	/**
	 * @brief Get values of the data of a service
	 * @param
	 * dataList - list of data values
	 */
	// Removed by TuanQuang Nguyen, Dec 15th 2009, avoid copy operation
	// void getDataList(std::vector<VariantType>& dataList) const;
	//const std::vector<boost::shared_ptr<VariantType> >& getDataList() const{ return dataList_;}

	friend class boost::serialization::access;

	template <typename Archive> void load(Archive & ar,
			const unsigned int version) {

		ar & mh;
		ar & serviceName;
		if (mh.nbuffer >0) {
			bufferPtrVec.resize(mh.nbuffer);
			for (int i=0; i<mh.nbuffer; i++) {
				size_t sz;
				ar & sz;
				if (sz >0) {
					char* buffer = new char[sz];
					ar & boost::serialization::make_array(buffer, sz);					
					setBuffer(i, MFBufferPtr(new MFBuffer(buffer, sz)) );
					delete buffer;
					buffer = 0;
				}
				else{
					setBuffer(i, MFBufferPtr(new MFBuffer) );					
				}
			}
		}
	}

	template <typename Archive> void save(Archive & ar,
			const unsigned int version) const {
		ar & mh;
		ar & serviceName;
		if (mh.nbuffer >0) {
			for (int i=0; i<mh.nbuffer; i++) {
				size_t sz = bufferPtrVec[i]->getSize();
				ar & sz;
				if (sz >0){
					ar & boost::serialization::make_array(
							(char*)bufferPtrVec[i]->getData(),
							bufferPtrVec[i]->getSize());
				}
			}
		}
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
	
#endif

	void clear()
	{
		bufferPtrVec.resize(0);
	}

	void display(std::ostream& os = std::cout) const
	{
		os<<"ServiceMessage: "<<std::endl;
		os<<serviceName<<std::endl;
		os<<mh.requestId<<std::endl;
		os<<mh.minorId<<std::endl;
		os<<mh.serviceNameSize<<std::endl;
		os<<mh.nbuffer<<std::endl;

		//os<<getSerializedSize()<<std::endl;
		for(unsigned int i=0; i<mh.nbuffer; i++)
		{
			//os<<(char*)bufferPtrVec[i]->getData()<<endl;			
			if(bufferPtrVec.size() == mh.nbuffer) {
#ifndef USE_MF_LIGHT
				if(bufferPtrVec[i])
					os<<"buffer "<<i<<" : "<<bufferPtrVec[i]->getSize()<<std::endl;
#endif
				
			}
		}
		os<<"====requester ip======"<<std::endl;
		os<<requester_.nodeIP_<<std::endl;
		os<<requester_.nodePort_<<std::endl;
	}

	bool operator ==(const ServiceMessage& other)const
	{
		return (serviceName == other.serviceName)
		&&(mh.requestId == other.mh.requestId);
	}

	//protected:
public:
   std::vector<MFBufferPtr> bufferPtrVec;
	/**
	 * @brief The service name
	 */
	MessageHeader mh;

	std::string serviceName;

	/**
	 * @brief status of the message
	 */
	//int status_;	

private:
     
	std::string agentInfo_;
	MessageFrameworkNode requester_;

	/**
	 * @brief The list of parameter values of the service request
	 */
	//std::vector<boost::shared_ptr<VariantType> > dataList_;	
};

typedef ServiceMessage ServiceRequestInfo;
typedef ServiceMessage ServiceResult;
typedef boost::shared_ptr<ServiceMessage> ServiceRequestInfoPtr;
typedef boost::shared_ptr<ServiceMessage> ServiceResultPtr;
typedef boost::shared_ptr<ServiceMessage> ServiceMessagePtr;

class ServiceResultCompare
{
public:
	bool operator()(const ServiceResultPtr& s1, const ServiceResultPtr& s2) const
	{
		int o = s1->mh.requestId - s2->mh.requestId;
		if ( o < 0 )
			return true;
		if( o == 0 )
		{
			if(s1->mh.minorId < s2->mh.minorId)
				return true;
		}
		return false;
	}
};


} // end of messageframework

#endif  //#ifndef _SERVICE_MESSAGE_H_
