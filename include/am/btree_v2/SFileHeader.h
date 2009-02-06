#ifndef SFILEHEADER_H_
#define SFILEHEADER_H_

NS_IZENELIB_AM_BEGIN

/**
 *  \brief it provides the basical information of the btree.
 * 
 */
struct SFileHeader{	
	int magic; //set it as 0x061561, check consistence.
	size_t minDegree;
	size_t pageSize;
	//size_t overFlowSize;
	size_t cacheSize;
	size_t numItem;
	int nNode;
	long rootPos; //streamoff of the root of btree.

	void display() {
		cout<<"fileHeader display...\n";
		cout<<"minDegree: "<<minDegree<<endl;
		cout<<"numItem: "<<numItem<<endl;
		cout<<"pageSize: "<<pageSize<<endl;
		//cout<<"overFlowSize: "<<overFlowSize<<endl;
		cout<<"cacheSize: "<<cacheSize<<endl;
		cout<<"rootPos: "<<rootPos<<endl;
		cout<<"nNode: "<<nNode<<endl;
	}
};

SFileHeader sfh;

NS_IZENELIB_AM_END

#endif /*SFILEHEADER_H_*/
