/**
 * @author  MyungHyun Lee (Kent)
 * @date    2009-01-30
 */

#include <ProcessOptions.h>
#include <net/message_framework.h>
#include <net/MFServer.h>
#include <LAManager.h>

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <string>

#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "LaServiceHandle.h"

using namespace std;
using namespace boost;
using namespace sf1v5_dummy;
using namespace messageframework;

namespace LaProcess {

//void initMFClient();
//void initMFServer();
//void initServiceList();
//int registerServiceList();

/*
 void initMFClient()
 {
 //-H -I -P -C -S -N
 MessageFrameworkNode controllerNode( po.getControllerIp(), po.getControllerPort() );
 client_ = new MessageClient( MF_CLIENT_ARG( "DocumentProcess_Server", controllerNode ) );
 }
 

 void initMFServer() {
 MessageFrameworkNode controllerNode(po.getControllerIp(),
 po.getControllerPort() );
 server_ = new MessageServer( MF_SERVER_ARG( "LAProcess_Server", po.getHostPort(), controllerNode ) );

 cout << "[LAProcess]: MF up and ready to go" << endl;
 }

 /*
 void initServiceList(vector<ServiceInfo> & serviceList) {
 ServiceInfo serviceInfo;
 //vector<ServiceParameterType> params;

 serviceInfo.setServer(po.getControllerIp(), po.getControllerPort() );
 //serviceInfo.setPermissionFlag(SERVE_AT_SERVER);

 serviceInfo.setServiceName("parseString");
 //params.clear();
 //params.push_back(STRING_TYPE);
 //serviceInfo.setParameterList(params);
 serviceList.push_back(serviceInfo);
 }

 int registerServiceList(const vector<ServiceInfo> & serviceList) {
 unsigned int i = 0;
 unsigned int nTimeout = 0;
 int failCnt = 0;
 bool ret = false;

 server_->setAgentInfo("col1");
 for (i = 0; i < serviceList.size(); i++) {
 while ( !(ret = server_->registerService(serviceList[i]) ) && nTimeout
 < 100) {
 cout << "[LAProcess] service registration failed, trying again."
 << endl;
 boost::thread::sleep(boost::get_system_time()
 + boost::posix_time::milliseconds(10) );
 nTimeout++;
 }
 if (ret == false) {
 failCnt++;
 }
 }

 return failCnt;
 }

 long la_time = 0;

 void parseString(LAManager& laMgr, ServiceMessagePtr &sm)
 {  
 // sm->display();
 for(int i=0; i<sm->getBufferNum(); i++){
 string str;
 vector<string> termList;				
 mf_deserialize(str, sm, i);	
 //cout<<"Input: "<<str<<endl;
 laMgr.parseString(str, termList);	
 // cout<<"Output: "<<endl;
 //for(int j=0; j<termList.size();j++){
 //	cout<<termList[j]<<endl;
 //}
 mf_serialize(termList, sm, i);  
 }
 }
 
 int laServerMain() {
 int ret = 0;

 vector<ServiceInfo> serviceList;
 vector<ServiceRequestInfoPtr> requestList;

 initServiceList(serviceList);
 if ( (ret = registerServiceList(serviceList)) != 0) {
 cerr << "[LAProcess]: number of failed service registrations: " << ret
 << endl;
 return -1;
 }

 cout << "[LAProcess]: Running server... " << endl;
 while (true) {
 // check if there is a request
 server_->getServiceRequestList(requestList);

 std::vector<ServiceResultPtr> serviceResults;
 serviceResults.reserve(requestList.size());

 //std::vector<boost::shared_ptr<VariantType> > paramList;
 //ServiceResultPtr serviceResult(new ServiceResult);
 ServiceResultPtr serviceResult;

 posix_time::ptime begin = posix_time::microsec_clock::local_time();

 //cerr << "\t[LAPROCES]: requestList.size= : " << requestList.size()
 //		<< endl;

 for (unsigned int i = 0; i < requestList.size(); i++) {
 //serviceResult->clear();

 //requestList[i]->display();
 if (requestList[i]->getServiceName() == "parseString") {
 //shared_ptr<string> str( new string );
 //shared_ptr<vector<string> > termList( new vector<string>() );
 parseString(laMgr_, requestList[i]);			
 server_->putResultOfService(requestList[i]);		

 }

 //serviceResults.push_back(serviceResult);
 //cerr << "\t[LAPROCES]: serviceResults: size= "
 //		<<serviceResults.size()<<endl;
 
 }

 posix_time::ptime end = posix_time::microsec_clock::local_time();
 la_time += (end-begin).total_microseconds();

 std::cout << "[PROFILE] la_time total " << la_time << std::endl;
 }
 }*/
}

void InitLAServer(MFServer<LAServiceHandle>& laServer) {	
	laServer.setAgentInfo("la");
	
	boost::unordered_map<std::string, ServiceItem<LAServiceHandle> > serviceList;
	ServiceItem<LAServiceHandle> item;
	item.callback_ = &LAServiceHandle::parseString;
	serviceList[ "parseString" ] = item;
	laServer.addService("parseString", item);

	LAManagerPtr laMgr_(new LAManager);
	boost::shared_ptr<LAServiceHandle> lash(new LAServiceHandle);
	lash->setLAManager(laMgr_);
	laServer.setServiceHandle(lash);
}

MF_AUTO_MAIN(laProcess) {
	using namespace LaProcess;

	sf1v5_dummy::ProcessOptions po;

	if ( !po.setLaProcessOptions(argc, argv)) {
		return 0;
	}

	//MessageFrameworkNodePtr  mfnode(new  MessageFrameworkNode(controllerNode( po.getControllerIp(), po.getControllerPort() ) );

	MFServer<LAServiceHandle> laServer(po.getHostPort(), po.getControllerIp(),
			po.getControllerPort(), 4);

	cout << "[LAProcess]: MF up and ready to go" << endl;

	InitLAServer(laServer);

	laServer.run();

	// client_ = new MessageClient( MF_CLIENT_ARG( "DocumentProcess_Server", controllerNode ) );


	//initMFServer();
	//initMFClient();


	//laServerMain();
	//boost::thread serverThread( &laServerMain );
	//serverThread.join();

	return 1;
}

