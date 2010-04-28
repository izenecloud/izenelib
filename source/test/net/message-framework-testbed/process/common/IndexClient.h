#ifndef INDEXCLIENT_H_
#define INDEXCLIENT_H_

#include <net/MFClient.h>
#include <IndexManager.h>

using namespace sf1v5_dummy;
using namespace messageframework;


class IndexClient:public MFClient{
	
public:
	bool addDocument(const std::vector<std::string>& agentInfos, const ForwardIndex& forwardIndex){			
		MF_CLIENT_IMPL_N_1_0(agentInfos, "addDocument", forwardIndex)		
	}
	
	
	//one agentInfo -> one docIdList(vector<unsigned int>
	bool findDocListByTerm(const std::vector<std::string>& agentInfos,
			std::string term, std::vector<std::vector<unsigned int> >& docIdLists){	
		MF_CLIENT_IMPL_N_1_1(agentInfos, "findDocListByTerm", term, docIdLists)				
		
	}
	
};

#endif /*INDEXCLIENT_H_*/
