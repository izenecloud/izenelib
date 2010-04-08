/**
 * @author  MyungHyun Lee (Kent)
 * @date    2009-01-30
 */

#include "ScdParser.h"


#include <ProcessOptions.h>
#include <net/message_framework.h>
#include <IndexClient.h>

#include <DocumentManager.h>

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <string>

#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace boost;
using namespace sf1v5_dummy;
using namespace messageframework;

namespace DocumentProcess {

DocumentManager docMgr_;
MessageClientPtr client_;
MessageServer * server_;
sf1v5_dummy::ProcessOptions po;

IndexClient indexClient_;
//indexClient.setMessageClient()

void getDocument(const DocumentManager& docMgr, ServiceMessagePtr& sm) {
	for (int i=0; i<sm->getBufferNum(); i++) {
		unsigned int docId;
		Document doc;
		mf_deserialize(docId, sm, i);
		docMgr.getDocument(docId, doc);
		mf_serialize(doc, sm, i);
	}
}

void initMFClient();
void initMFServer();
void initServiceList();
int registerServiceList();
void buildDataBase(const string & scdFilePath, vector<Document> & docList);
bool buildForwardIndex(const vector<Document> & docList,
		vector<ForwardIndex> & forwardIndexList);

void initMFClient() {
	//-H -I -P -C -S -N
	MessageFrameworkNode controllerNode(po.getControllerIp(),
			po.getControllerPort() );

	client_.reset(new MessageClient( MF_CLIENT_ARG( "DocumentProcess_Server", controllerNode ) ));
	indexClient_.setMessageClient(client_);
	cout<<"\n!!!!! main-document "<<typeid(client_).name()<<endl;
	cout << "[DocumentProcess]: MF-Client up and ready to go" << endl;
}

void initMFServer() {
	MessageFrameworkNode controllerNode(po.getControllerIp(),
			po.getControllerPort() );
	server_ = new MessageServer( MF_SERVER_ARG( "DocumentProcess_Server", po.getHostPort(), controllerNode ) );

	cout << "[DocumentProcess]: MF-Server up and ready to go" << endl;
}

void buildDataBase(const string & scdFilePath, vector<Document> & docList) {
	unsigned int i=0;

	parseScdFile(scdFilePath.c_str(), docList);

	for (unsigned i = 0; i < docList.size(); i++) {
		docMgr_.insertDocument(docList[i]);
	}
}

long laTime = 0;
long indexTime = 0;

bool buildForwardIndex(const vector<Document> & docList,
		vector<ForwardIndex> & forwardIndexList) {
	/*
	 * Parse document titles and contents
	 */
	vector<ServiceRequestInfoPtr> requestsToLA;
	vector<ServiceResultPtr> resultsFromLA;

	requestsToLA.resize( 2*docList.size() );
	resultsFromLA.resize( 2*docList.size() );
	for (int i=0; i<docList.size(); i++) {
		//cout<<"idx: "<<i<<endl;
		// make parsing title request
		requestsToLA[2*i].reset(new ServiceRequestInfo);
		requestsToLA[2*i]->setServiceName("parseString");
		//shared_ptr<VariantType> temp( new VariantType( shared_ptr<string>(
		//	new string(docList[i].getTitle()) )));
		//requestsToLA[2*i].appendParameter(temp);

		mf_serialize(docList[i].getTitle(), requestsToLA[2*i]);
		//cout<<"GetTitle:: "<<docList[i].getTitle()<<endl;

		//requestsToLA[2*i]->display();


		// make parsing content request
		requestsToLA[2*i+1].reset(new ServiceRequestInfo);
		requestsToLA[2*i+1]->setServiceName("parseString");
		//temp.reset( new VariantType( shared_ptr<string>(
		//	new string(docList[i].getContent()) )));
		//requestsToLA[2*i+1].appendParameter(temp);
		mf_serialize(docList[i].getContent(), requestsToLA[2*i+1]);

		//requestsToLA[2*i+1]->display();
		//cout<<"GetContent:: "<<docList[i].getContent()<<endl;			
	}
	// batch process all parsing requests
	posix_time::ptime before = posix_time::microsec_clock::local_time();
	//	if ( false == requestService("parseString", requestsToLA, resultsFromLA,
	//			*client_) )
	//		return false;
	for (unsigned int i=0; i<2*docList.size(); i++) {
		if ( false == requestService("parseString", requestsToLA[i],
				resultsFromLA[i], *client_) )
			return false;
	}
	posix_time::ptime after = posix_time::microsec_clock::local_time();
	laTime += (after-before).total_microseconds();
	cout << "[PROFILE] on client la cost " << laTime << " macroseconds" << endl;

	/*
	 * Build document index
	 */
	vector<ServiceRequestInfoPtr> requestsToIndex;
	requestsToIndex.resize(docList.size());
	cout<<docList.size()<<endl;
	for (int i=0; i < docList.size(); i++) {

		// a document indexing request
		requestsToIndex[i]
				= shared_ptr<ServiceRequestInfo>(new ServiceRequestInfo);
		requestsToIndex[i]->setServiceName("addDocument");

		// init forward index
		shared_ptr<ForwardIndex> forwardIndex =
				shared_ptr<ForwardIndex>(new ForwardIndex(docList[i].getId()));

		// insert terms in title into forward index
		//shared_ptr<VariantType> titleLaResult =
		//	resultsFromLA[2*i].getServiceResult().at(0);

		shared_ptr<vector<string> > titleTerms(new vector<string>);
		//cout<<"requestsToIndex 0: "<<i<<endl;	

		//resultsFromLA[2*i]->display();
		mf_deserialize(*titleTerms, resultsFromLA[2*i]);
		//cout<<"requestsToIndex 0.5: "<<i<<endl;

		//cout<<titleTerms->size()<<endl;
		//titleLaResult->getData(titleTerms);
		for (int j=0; j< titleTerms->size(); j++) {
			//	cout<<"j="<<j<<endl;
			//	cout<<"ForwardIndex: add term="<< titleTerms->at(j)<<endl; 
			forwardIndex->addTerm(titleTerms->at(j) );
		}

		// insert terms in content into forward index
		//shared_ptr<VariantType> contentLaResult =
		//	resultsFromLA[2*i+1].getServiceResult().at(0);				


		shared_ptr<vector<string> > contentTerms(new vector<string>);
		//contentLaResult->getData(contentTerms);		

		//resultsFromLA[2*i+1]->display();
		mf_deserialize(*contentTerms, resultsFromLA[2*i+1]);

		for (int j=0; j< contentTerms->size(); j++) {
			//cout<<"ForwardIndex: add term1="<< contentTerms->at(j)<<endl; 
			forwardIndex->addTerm(contentTerms->at(j) );
		}

		// set forward index created above as parameter
		//shared_ptr<VariantType> temp( new VariantType() );
		//temp->putCustomData(forwardIndex);
		//requestsToIndex[i].appendParameter(temp);

		//mf_serialize(*forwardIndex, requestsToIndex[i]);
		// requestsToIndex[i]->display();

		before = posix_time::microsec_clock::local_time();

		{

			std::vector<std::string> agentInfos;
			agentInfos.push_back("index1");
			agentInfos.push_back("index2");

			if ( !indexClient_.addDocument(agentInfos, *forwardIndex) )
				return false;

		}
		/*if (false == requestService("index1", "addDocument", requestsToIndex,
		 *client_) )
		 return false;
		 if ( false == requestService("index2", "addDocument", requestsToIndex,
		 *client_) )
		 return false;*/
		after = posix_time::microsec_clock::local_time();
		indexTime += (after - before).total_microseconds();

	}

	// batch processing all indexing requests


	cout << "[PROFILE] on client index cost " << indexTime << " macroseconds"
			<< endl;

	return true;
}

void initServiceList(vector<ServiceInfo> & serviceList) {
	ServiceInfo serviceInfo;
	//vector<ServiceParameterType>    params;

	//serviceInfo.setServer(po.getControllerIp(), po.getControllerPort() );
	//serviceInfo.setPermissionFlag( SERVE_AT_SERVER );

	serviceInfo.setServiceName("getDocument");
	//params.clear();
	//params.push_back( UNSIGNED_INT_TYPE );
	//serviceInfo.setParameterList( params );
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
			cout
					<< "[DocumentProcess] service registration failed, trying again."
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

long docTime = 0;

int docServerMain() {
	int ret = 0;

	vector<ServiceInfo> serviceList;
	vector<ServiceRequestInfoPtr> requestList;

	initServiceList(serviceList);
	if ( (ret = registerServiceList(serviceList)) != 0) {
		cerr << "[DocumentProcess]: number of failed service registrations: "
				<< ret << endl;
		return -1;
	}

	cout << "[DocumentProcess]: Running server... " << endl;

	while (true) {
		{
			cout << "\tDEBUG [Document-Process]: GOoooo" << endl;
		}
		// check if there is a request
		server_->getServiceRequestList(requestList);
		//std::vector<boost::shared_ptr<VariantType> > paramList;
		//ServiceResultPtr serviceResult(new ServiceResult);
		ServiceResultPtr serviceResult;

		// {
		//      cout << "\tDEBUG [Document-Process]: request list size"
		//		<< requestList.size() << endl;
		//  }
		posix_time::ptime begin = posix_time::microsec_clock::local_time();

		for (unsigned int i = 0; i < requestList.size(); i++) {
			//				serviceResult.clear();
			///ServiceResultPtr serviceResult(new ServiceResult);
			serviceResult = requestList[i];
			//    {
			//        cout << "\tDEBUG [Document-Process]: incomming service name:"
			//		<< requestList[i]->getServiceName() << endl;
			//    }

			if (requestList[i]->getServiceName() == "getDocument") {
				getDocument(docMgr_, requestList[i]);
				//shared_ptr<unsigned int> docId;
				//paramList = requestList[i].getParameterList();
				//paramList[0]->getData( docId );

				/*unsigned int docId;
				 Document doc;
				 mf_deserialize(docId, requestList[i]);
				 
				 //shared_ptr<Document> doc(new Document);					
				 docMgr_.getDocument(docId,  doc);
				 {
				 cout << "\tDEBUG [Document-Process]: INPUT: " << *docId << endl;
				 cout << "\tDEBUG [Document-Process]: OUTPUT: " << doc->getTitle() << endl;
				 }
				 //shared_ptr<VariantType> resultData( new VariantType());
				 //resultData->putCustomData(doc);
				 //serviceResult.appendServiceData( resultData );
				 mf_serialize(doc, serviceResult);*/
			}
			server_->putResultOfService(serviceResult);
		}
		posix_time::ptime end = posix_time::microsec_clock::local_time();
		docTime += (end-begin).total_microseconds();

		cout << "[PROFILE] doc process " << "total time" << docTime << "ms"
				<< endl;
	}
}

}

MF_AUTO_MAIN(documentProcess) {
	using namespace DocumentProcess;

	if ( !po.setDocumentProcessOptions(argc, argv)) {
		return 0;
	}

	initMFServer();
	initMFClient();

	vector<Document> docList;
	vector<ForwardIndex> forwardIndexList;

	buildDataBase(po.getScdFileName(), docList);

	if (buildForwardIndex(docList, forwardIndexList) == false) {
		cerr << "[DocumentProcess]: building forwardindex failed" << endl;
		return -1;
	}

	docServerMain();
	//boost::thread serverThread( &docServerMain );
	//serverThread.join();

	return 1;
}
