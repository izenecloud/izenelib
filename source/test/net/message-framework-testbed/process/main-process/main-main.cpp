/**
 * @author  MyungHyun Lee (Kent)
 * @date    2009-01-30
 */

#include <document-manager/Document.h>

#include <net/message_framework.h>

#include <IndexClient.h>

#include "ProcessOptions.h"

#include <boost/smart_ptr.hpp>

#include <vector>

#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace sf1v5_dummy;
using namespace messageframework;

namespace MainProcess {

//IndexManager        indexMgr_;
//DocumentManager     docMgr_;
//LAmanager           lamMgr_;

MessageClient * client_;
IndexClient indexClient_;

//Document t;

}

MF_AUTO_MAIN(mainProcess) {
	using namespace MainProcess;

	sf1v5_dummy::ProcessOptions po;

	if ( !po.setMainProcessOptions(argc, argv)) {
		return 0;
	}

	MessageFrameworkNode controllerInfo(po.getControllerIp(),
			po.getControllerPort() );

	client_ = new MessageClient( MF_CLIENT_ARG("MainProcess", controllerInfo) );

	MessageClientPtr tempClient(new MessageClient( MF_CLIENT_ARG("MainProcess", controllerInfo) ));
	indexClient_.setMessageClient(tempClient);

	string queryStr;

	//shared_ptr<VariantType> queryString;
	shared_ptr<vector<string> > termList(new vector<string>);

	//shared_ptr<VariantType > term;
	shared_ptr<vector<unsigned int> > docIdList(new vector<unsigned int>);

	shared_ptr< vector<Document> > resultDocList(new vector<Document>);

	vector<vector<unsigned int> > docIdLists;

	bool ret = true;
	ifstream inf("test2.txt");
	long total_time = 0;
	long total_stime = 0;
	long total_rtime = 0;

	while (inf >> queryStr) {
		//--- containers ---
		cout << "Enter Search Query: " << endl;
		cin >> queryStr;
		cout<<queryStr;

		//        client_->getMessageDispatcher().serialization_time = 0;
		//        client_->getMessageDispatcher().deserialization_time = 0;

		boost::posix_time::ptime begin =
				boost::posix_time::microsec_clock::local_time();

		messageframework::permission_time = 0;
		messageframework::send_time = 0;
		messageframework::recv_time = 0;

		ServiceRequestInfoPtr requestToLA(new ServiceRequestInfo);
		ServiceResultPtr resultFromLA;

		requestToLA->setServiceName("parseString");
		//queryString.reset( new VariantType(shared_ptr<string>( new string(queryStr)) ) );
		// requestToLA.appendParameter(queryString);
		// 1. parse query string into terms(words)
		mf_serialize(queryStr, requestToLA);
		ret
				= requestService("parseString", requestToLA, resultFromLA,
						*client_);
		if (ret == false) {
			cerr
					<< "[Main-Process]: ERROR when calling service, \"parseString\" "
					<< endl;
			return -1;
		} else {
			//resultFromLA.getServiceResult().at(0)->getData( termList );
			mf_deserialize(*termList, resultFromLA);
		}

		// for the number of terms in a query string
		for (size_t i = 0; i < termList->size(); i++) {
			///requst to two collection
			{
				std::vector<std::string> agentInfos;
				agentInfos.push_back("index1");
				agentInfos.push_back("index2");
				if (!indexClient_.findDocListByTerm(agentInfos, (*termList)[i], docIdLists) )
					return -1;
			}

			/*ServiceRequestInfoPtr requestToIndex(new ServiceRequestInfo);
			 ServiceResultPtr resultFromIndex;

			 requestToIndex->setServiceName("findDocListByTerm");
			 //term.reset( new VariantType(
			 //            shared_ptr<string>( new string( (*termList)[i]) )
			 //) );
			 //requestToIndex.appendParameter(term);
			 mf_serialize((*termList)[i], requestToIndex);

			 //requestToIndex->display();
			 
			 // 2. Get the list of documents that are searched with the term
			 ret = requestService( "findDocListByTerm", requestToIndex, resultFromIndex, *client_ );
			 if( ret == false )
			 {
			 cout << "[Main-Process]: ERROR when calling service, \"findDocListByTerm\" " << endl;
			 return -1;
			 }
			 else
			 {
			 //resultFromIndex.getServiceResult().at(0)->getData( docIdList );
			 mf_deserialize(*docIdList, resultFromIndex);
			 if( docIdList->empty() )
			 {
			 cout << "\tDEBUG [Main-Process]: no result from \"findDocListByTerm\"" << endl;
			 continue;
			 }
			 }
			 */

			{
				cout
						<< "\tFrom index1:DEBUG [Main-Process]: docIdList.size(): "
						<< docIdLists[0].size() << endl;
				cout
						<< "\tFrom index2:DEBUG [Main-Process]: docIdList.size(): "
						<< docIdLists[1].size() << endl;
			}

			//resultDocList.reserve(docIdList->size());

			// 3. get all documents' text by a batch processing call
			// vector<ServiceRequestInfoPtr> requestsToDoc;
			// vector<ServiceResultPtr> resultsFromDoc;


			*docIdList = docIdLists[0];

			if (docIdList->empty() )
				continue;
			ServiceRequestInfoPtr requestsToDoc(new ServiceMessage);
			ServiceResultPtr resultsFromDoc;
			requestsToDoc->setServiceName("getDocument");

			// requestsToDoc.resize(docIdList->size());
			for (size_t j =0; j< docIdList->size(); j++) {
				//ServiceMessagePtr ptr(new ServiceMessage);
				//requestsToDoc[j] = ptr;
				//requestsToDoc[j]->setServiceName("getDocument");
				mf_serialize((*docIdList)[j], requestsToDoc, j);
				//boost::shared_ptr<mf::VariantType> parameter( new mf::VariantType(
				//	 boost::shared_ptr<unsigned int>( new unsigned int( (*docIdList)[i]) )
				//) );
				//requestsToDoc[j].appendParameter(parameter);
			}

			if ( false == requestService("getDocument", requestsToDoc,
					resultsFromDoc, *client_) ) {
				cout
						<< "[Main-Process]: ERROR when calling service \"getDocument\""
						<< endl;
				return -1;
			} else {
				for (int i=0; i<resultsFromDoc->getBufferNum(); i++) {
					//boost::shared_ptr<mf::VariantType> result = (it->getServiceResult())[0];

					//if( result == NULL )
					//{
					//	cout << "\tDEBUG [Main-Process]: miss a result from \"getDocument\"" << endl;
					//	continue;
					//}
					boost::shared_ptr<Document> docPtr(new Document);
					//result->getCustomData( docPtr );
					mf_deserialize(*docPtr, resultsFromDoc, i);
					resultDocList->push_back(*docPtr);
				}
				//resultDocList->clear();
			}

		} // for the number of terms in a query string


		boost::posix_time::ptime end =
				boost::posix_time::microsec_clock::local_time();
		long used_time = (end-begin).total_microseconds();
		total_time += (end-begin).total_microseconds();
		total_stime += messageframework::send_time;
		total_rtime += messageframework::recv_time;

		cout << "[Main-Process]: Done searching!!: " << resultDocList->size()
				<< "results, cost " << used_time << " micro seconds" << endl;

		cout << "[PROFILE] get permission cost "
				<< messageframework::permission_time << " microseconds" << endl;
		cout << "[PROFILE] send message cost " << messageframework::send_time
				<< " microseconds" << endl;
		cout << "[PROFILE] recv result cost " << messageframework::recv_time
				<< " microseconds" << endl;

		//        cout << "[PROFILE] serialization cost " << client_->getMessageDispatcher().serialization_time << endl;
		//        cout << "[PROFILE] deserialization cost " << client_->getMessageDispatcher().deserialization_time << endl;

		//        if( resultDocList.size() > 0 )
		//        {
		//            sort( resultDocList.begin(), resultDocList.end(), resultDocList[0] );
		//
		//
		//            for( size_t k = 0; k <  resultDocList.size(); k++ )
		//            {
		//                cout << resultDocList[k].getTitle() << endl;
		//                cout << resultDocList[k].getContent() << endl;
		//            }
		//        }

		resultDocList->clear();

	}//while loop. RECEIVING QUERIES

	cout
			<< "[Main-Process Total]: Done batch searching!!  (in micro seconds) \n";
	cout<< " send time: "<<total_stime<<endl;
	cout<< " recv time: "<<total_rtime<<endl;
	cout<< " MF time "<<total_stime+total_rtime<<endl;
	cout<< " no MFtime "<<total_time-total_stime-total_rtime<<endl;
	cout<< " total time "<<total_time << endl;

#ifndef USE_MF_LIGHT
	cout<<" total data sended size "<<AsyncStream::sended_data_size<<endl;
#endif

	return 1;
}
