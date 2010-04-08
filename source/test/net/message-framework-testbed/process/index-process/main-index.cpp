/**
 * @author  MyungHyun Lee (Kent)
 * @date    2009-01-30
 */

#include <ProcessOptions.h>

#include <net/message_framework.h>
#include <IndexManager.h>

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

struct IndexProcess {

	IndexManager indexMgr_;
	MessageClient * client_;
	MessageServer * server_;
	

	void initMFServer(sf1v5_dummy::ProcessOptions& po) {
		MessageFrameworkNode controllerNode(po.getControllerIp(),
				po.getControllerPort() );
		server_ = new MessageServer( MF_SERVER_ARG( "IndexProcess_Server", po.getHostPort(), controllerNode ) );
		cout<<"!!!! Index server for agentInfo: "<<po.getAgentInfo()<<endl;
		cout<<"host port="<<po.getHostPort()<<endl;
		server_->setAgentInfo(po.getAgentInfo() );
		cout << "[IndexProcess]: MF up and ready to go" << endl;
	}

	void initServiceList(vector<ServiceInfo> & serviceList) {
		ServiceInfo serviceInfo;
		//vector<ServiceParameterType> params;

		//serviceInfo.setServer(po.getControllerIp(), po.getHostPort() );
		//serviceInfo.setPermissionFlag(SERVE_AT_SERVER);

		serviceInfo.setServiceName("addDocument");
		//params.clear();
		//params.push_back(CUSTOM_TYPE);
		//serviceInfo.setParameterList(params);
		//serviceInfo.setServiceResultFlag(SERVICE_WITHOUT_RESULT); // this service does not have any result
		serviceList.push_back(serviceInfo);

		serviceInfo.setServiceName("findDocListByTerm");
		//params.clear();
		//params.push_back(STRING_TYPE);
		//serviceInfo.setParameterList(params);
		//serviceInfo.setServiceResultFlag(SERVICE_WITH_RESULT); // this service does not have any result
		serviceList.push_back(serviceInfo);
	}

	int registerServiceList(const vector<ServiceInfo> & serviceList) {
		unsigned int i = 0;
		unsigned int nTimeout = 0;
		int failCnt = 0;
		bool ret = false;

		for (i = 0; i < serviceList.size(); i++) {
			while ( !(ret = server_->registerService(serviceList[i]) )
					&& nTimeout < 100) {
				cerr
						<< "[IndexProcess] service registration failed, trying again."
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

	//long index_time = 0;

	void addDocument(IndexManager& indexMgr_, ServiceMessagePtr& sm) {
		for (int i=0; i<sm->getBufferNum(); i++) {
			ForwardIndex forwardIndex;
			mf_deserialize(forwardIndex, sm, i);

			//cout			<< "\tDEBUG [IndexProcess]: INPUT: forward index of doc#:"
			//	   << forwardIndex.getDocId() << endl;

			indexMgr_.addDocument(forwardIndex);
		}
	}

	void findDocListByTerm(IndexManager& indexMgr_, ServiceMessagePtr& sm) {
		for (int i=0; i<sm->getBufferNum(); i++) {
			//shared_ptr<string> term(new string);
			//shared_ptr<vector<unsigned int> >
			//docIdList(new vector<unsigned int>());
			string term;
			vector<unsigned int> docIdList;

			mf_deserialize(term, sm, i);

			cout << "\tDEBUG [IndexProcess]: INPUT: term :" << term << endl;
			indexMgr_.findDocListByTerm(term, docIdList);
			cout << "\tDEBUG [IndexProcess]: OUPUT: docIdList size :"
					<< docIdList.size() << endl;

			mf_serialize(docIdList, sm, i);
		}
	}

	int indexServerMain() {
		int ret = 0;

		vector<ServiceInfo> serviceList;
		vector<ServiceRequestInfoPtr> requestList;

		initServiceList(serviceList);
		if ( (ret = registerServiceList(serviceList)) != 0) {
			cerr << "[IndexProcess]: number of failed service registrations: "
					<< ret << endl;
			return -1;
		}

		cout << "[IndexProcess]: Running server... " << endl;

		while (true) {
			// check if there is a request
			server_->getServiceRequestList(requestList);

			//std::vector<boost::shared_ptr<VariantType> > paramList;
			//ServiceResultPtr serviceResult(new ServiceResult);
			ServiceResultPtr serviceResult;

			for (unsigned int i = 0; i < requestList.size(); i++) {
				//serviceResult->clear();				
				//cout<<" requestList idx= "<<i<<endl;
				if (requestList[i]->getServiceName() == "addDocument") {
					addDocument(indexMgr_, requestList[i]);

					/*posix_time::ptime before =
					 posix_time::microsec_clock::local_time();
					 ForwardIndex forwardIndex;
					 //cout<<"\tDEBUG [IndexProcess]: addDocument"<<endl;
					 //requestList[i]->display();
					 mf_deserialize(forwardIndex, requestList[i]);

					 //shared_ptr<ForwardIndex> forwardIndex;    //the input parameter

					 //paramList = requestList[i].getParameterList();
					 //paramList[0]->getCustomData<ForwardIndex>( forwardIndex );

					 //{
					 //	cout
					 //			<< "\tDEBUG [IndexProcess]: INPUT: forward index of doc#:"
					 //			<< forwardIndex.getDocId() << endl;
					 //}
					 indexMgr_.addDocument(forwardIndex);

					 posix_time::ptime after =
					 posix_time::microsec_clock::local_time();
					 posix_time::time_period period(before, after);
					 index_time += period.length().total_microseconds();
					 //cout << endl << "[PROFILE] index_time total " << index_time
					 //		<< " macroseconds" << endl;

					 //THERE ARE NO RETURN VALUES
					 serviceResult = requestList[i];
					 serviceResult->clear();*/

				} else if (requestList[i]->getServiceName()
						== "findDocListByTerm")
				//bool findDocListByTerm( const std::string & term, std::vector<unsigned int> & docIdList ) const;
				{

					/*cout<<"findDocListByTerm"<<endl;
					 shared_ptr<string> term(new string);
					 shared_ptr<vector<unsigned int> >
					 docIdList(new vector<unsigned int>());

					 mf_deserialize(*term, requestList[i]);
					 //paramList = requestList[i].getParameterList();
					 //paramList[0]->getData( term );

					 //{
					 //	cout << "\tDEBUG [IndexProcess]: INPUT: term :" << *term
					 //			<< endl;
					 //}
					 indexMgr_.findDocListByTerm( *term, *docIdList);

					 //{
					 //	cout << "\tDEBUG [IndexProcess]: OUPUT: docIdList size :"
					 //			<< docIdList->size() << endl;
					 //}
					 //shared_ptr<VariantType> resultData( new VariantType( docIdList) );
					 //serviceResult.appendServiceData( resultData );
					 serviceResult = requestList[i];
					 mf_serialize(*docIdList, serviceResult);*/

					findDocListByTerm(indexMgr_, requestList[i]);
					server_->putResultOfService(requestList[i]);
				}

				//cerr << "[IndexProcess]: done with one service" << endl;
			}
		}
	}

};

MF_AUTO_MAIN(indexProcess) {
	sf1v5_dummy::ProcessOptions po;
	
	IndexProcess ip;
	if ( !po.setIndexProcessOptions(argc, argv)) {
		return 0;

	}

	ip.initMFServer(po);
	ip.indexServerMain();

	return 1;
}
