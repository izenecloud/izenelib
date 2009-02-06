#ifndef SKIPNODE_H_
#define SKIPNODE_H_

#include "slf_types.h"
#include "slf_util.h"
#include "SlfHeader.h"

NS_IZENELIB_AM_BEGIN
/**
 * @brief SkipNode reprents a node in a SkipList.
 */

template <typename DataType, typename LockType, typename Alloc> class SkipNode {

public:
	SkipNode(const DataType& theElement = DataType(), int h = 0) :
	element(theElement), height(h) {
		for(int i=0; i<MAX_LEVEL; i++) {
			right[i] = 0;
			rfpos[i] = 0;
		}
		//for(int i=0; i<MAX_BAND; i++)
		//	counter[i] = 0;
		isLoaded = false;
		isDirty = true;
		//down = bottom;
		activeNodeNum++;
	}
	virtual ~SkipNode() {
		/*if (right)
		 right = 0;
		 if (down && down != bottom )
		 down = 0;*/
		isLoaded = false;
		isDirty = false;
		activeNodeNum--;
	}

	void unload() {
		//to do
		isLoaded = false;
	}

	bool read(FILE* f);
	bool write(FILE* f);

	inline bool read(char* pBuf);
	inline bool write(char* pBuf);

	inline SkipNode<DataType, LockType, Alloc>* loadRight(int h, FILE* f) {
		//if( !isHasRight ) return NULL;
		/*	if ( !right ){
		 //cout<<"????"<<endl;
		 right = new SkipNode<DataType, LockType, Alloc>;
		 }*/
	//	cout<<element.key<<endl;
		//cout<< h<<" vs "<<height<<endl;
    	//assert(h < height);

		if ( right[h] && !right[h]->isLoaded ) {
			right[h]->read(f);
			//cout<<"load right..."<<right[h]->element.get_key()<<endl;
			//cout<<"fpos: "<<fpos<<
			//cout<<"->"<<rfpos<<endl;
		}
		//if( right[h] )cout<<"load right..."<<right[h]->element.get_key()<<endl;
		if(right[h])right[h]->left[h] = this;
		return right[h];
	}

	/*inline SkipNode<DataType, LockType, Alloc>* loadDown(FILE* f) {
	 if(height >0);
	 height --;
	 return this;
	 }*/

	void display() {
		cout<<"diplaying skipnode...\n\n";
		cout<<element.get_key()<<endl;
		cout<<"isLoaded: "<<isLoaded<<endl;
		cout<<"isDirty: "<<isDirty<<endl;
		cout<<"fpos: "<<fpos<<endl;
		cout<<"height: "<<height<<endl;
		for (int i=0; i<height; i++) {
			cout<<"rfpos: "<<rfpos[i]<<endl;
		}

	}

	static void setDataSize(size_t maxDataSize, size_t pageSize) {
		maxDataSize_ = maxDataSize;
		pageSize_ = pageSize;		
	}
public:
	DataType element;
	SkipNode* right[MAX_LEVEL];
	SkipNode* left[MAX_LEVEL];
	long rfpos[MAX_LEVEL];
	NodeID nid;

	int height;

	//int d_part;
	//int r_part;
	//int band;

	//int counter[MAX_BAND];
	//for reserving 	
public:
	//	bool isLeaf;
	//	bool isHasRight;
	bool isLoaded;
	bool isDirty;
	long fpos;
	/*long rfpos;
	 long dfpos;	*/

	static size_t activeNodeNum;
	//static SkipNode<DataType, LockType, Alloc>* bottom;
private:
	DbObjPtr object_;
	static size_t maxDataSize_;
	//static size_t overFlowSize_;
	static size_t pageSize_;
	static LockType lock_;
	static map<long, SkipNode<DataType, LockType, Alloc>*> pos2node_;
};

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
		DataType, LockType, Alloc>::activeNodeNum;

//template<typename DataType, typename LockType, typename Alloc> SkipNode<DataType, LockType, Alloc>
//		* SkipNode< DataType, LockType, Alloc>::bottom;
//new SkipNode<DataType, LockType, Alloc>(DataType(), bottom, bottom);

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
		DataType, LockType, Alloc>::maxDataSize_;

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
		DataType, LockType, Alloc>::pageSize_;

//template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
//		DataType, LockType, Alloc>::overFlowSize_;

template<typename DataType, typename LockType, typename Alloc> LockType
		SkipNode<DataType, LockType, Alloc>::lock_;

template<typename DataType, typename LockType, typename Alloc> map<long, SkipNode<DataType, LockType, Alloc>*>
		SkipNode<DataType, LockType, Alloc>::pos2node_;

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
		DataType, LockType, Alloc>::read(FILE *f) {
	//lock_.acquire_write_lock();

	//if( isLoaded )return true;
	int n = (fpos - sizeof(SlfHeader) )/pageSize_;
	//cout<<"reading page="<<n<<endl;
	//cout<<"nid: "<<nid<<endl;
	n /= BLOCK_SIZE;

	if ( !mbList[n]) {
		mbList[n] = new MemBlock(sizeof(SlfHeader) + n*pageSize_*BLOCK_SIZE, pageSize_);
		mbList[n]-> load(f);
	}
	//cout<<"!!!!"<<endl;
	//cout<<mbList[n]<<endl;
	//cout << mbList[n]->getP(fpos)<<endl;
	return read( mbList[n]->getP(fpos) );

	/*long overFlowAddress;
	 size_t recSize;
	 byte leafFlag = 0;
	 byte rightFlag = 0;
	 byte* pBuf;

	 if (!f) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (0 != fseek(f, fpos, SEEK_SET)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 // read the leafflag 
	 if (1 != fread(&leafFlag, sizeof(byte), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 isLeaf = (leafFlag == 1);

	 // read the leafflag 
	 if (1 != fread(&rightFlag, sizeof(byte), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 isHasRight = (rightFlag == 1);

	 if (isHasRight) {
	 if (1 != fread(&rfpos, sizeof(long), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (right == 0) {
	 right = new SkipNode<DataType, LockType, Alloc>;
	 }
	 right->fpos = rfpos;
	 } else {
	 right = 0;
	 }

	 // read the addresses of the child pages
	 if ( !isLeaf) {
	 if (1 != fread(&dfpos, sizeof(long), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 //cout<<"dfpos: "<<dfpos<<endl;
	 if (down == 0 || down == bottom) {
	 down = new SkipNode<DataType, LockType, Alloc>;
	 }
	 down->fpos = dfpos;
	 } else {
	 down = bottom;
	 }
	 // read the size of the DbObj.
	 if (1 != fread(&recSize, sizeof(size_t), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 pBuf = new byte[recSize];
	 if (recSize <= maxDataSize_) {
	 if (1 != fread(pBuf, recSize, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 } else {
	 if (1 != fread(pBuf, maxDataSize_, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (1 != fread(&overFlowAddress, sizeof(long), 1, f) ) {
	 lock_.release_write_lock();
	 return false;
	 }

	 if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (1 != fread(pBuf + maxDataSize_, recSize - maxDataSize_, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 }
	 assert(recSize != 0);
	 object_.reset(new DbObj(pBuf, recSize));
	 read_image(element, object_);

	 delete[] pBuf;

	 isLoaded = true;
	 isDirty = false;
	 return true;*/

}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
		DataType, LockType, Alloc>::read(char *pBuf) {
	//lock_.acquire_write_lock();
	//cout<<"reading...."<<endl;

	char *p = pBuf;

	//	long overFlowAddress;
	size_t recSize;
	
	//size_t overFlowSize_ = pageSize_;
	
	//byte leafFlag = 0;
	//byte rightFlag = 0;
	char* temp;

	if (!p) {
		//lock_.release_write_lock();
		return false;
	}
	//cout<<p<<endl;

	//	byte *test = new byte[10];
	//memcpy(test, p, sizeof(byte)*10);
	//memcpy(&leafFlag, p, sizeof(byte));
	//p += sizeof(byte);

	//isLeaf = (leafFlag == 1);

	//memcpy(&rightFlag, p, sizeof(byte));
	//p += sizeof(byte);

	//isHasRight = (rightFlag == 1);

	//if (isHasRight)
	memcpy(&height, p, sizeof(int));
	p += sizeof(int);
	memcpy(&rfpos[0], p, sizeof(long)*MAX_LEVEL);
	p += sizeof(long)*MAX_LEVEL;
	//cout<<"height "<<height<<endl;	
	for (int i=0; i<height; i++) {
		///cout<<"height "<<height<<endl;	
		//cout<<"rfpos: "<<rfpos[i]<<endl;
		if (rfpos[i] != 0) 
		{
			if ( right[i] == 0 ) {
				if ( !pos2node_[rfpos[i]] ) {
					right[i] = new SkipNode<DataType, LockType, Alloc>;
					right[i]->fpos = rfpos[i];
					pos2node_[rfpos[i]] = right[i];
				} else {
					right[i] = pos2node_[rfpos[i]];
				}
			}
		}
		//if( right[i] ) cout<<"right fpos "<<i<<" : "<<right[i]->fpos<<endl;
	}
	//cout<<isHasRight<<" vs "<<isLeaf<<endl;
	//cout<<rfpos<<" vs "<<dfpos<<endl;

	memcpy(&recSize, p, sizeof(size_t));
	p += sizeof(size_t);

	temp = new char[recSize];
	if (recSize <= maxDataSize_) {
		memcpy(temp, p, recSize);
		p += recSize;
	} else {
		assert(false);
		/*
		 if (1 != fread(pBuf, maxDataSize_, 1, f)) {
		 lock_.release_write_lock();
		 return false;
		 }
		 if (1 != fread(&overFlowAddress, sizeof(long), 1, f) ) {
		 lock_.release_write_lock();
		 return false;
		 }

		 if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
		 lock_.release_write_lock();
		 return false;
		 }
		 if (1 != fread(pBuf + maxDataSize_, recSize - maxDataSize_, 1, f)) {
		 lock_.release_write_lock();
		 return false;
		 }*/

	}

	assert(recSize != 0);
	object_.reset(new DbObj(temp, recSize));
	read_image(element, object_);

	//cout<<"reading.."<<element.get_key()<<endl;

	delete[] temp;

	isLoaded = true;
	isDirty = false;
	return true;

}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
		DataType, LockType, Alloc>::write(FILE *f) {

	if (!isDirty) {
		return true;
	}

	int n = (fpos - sizeof(SlfHeader) )/(pageSize_*BLOCK_SIZE);

	//cout<<"writing.."<<endl;
	//cout<<n;
	//cout<<fpos<<endl;
	if ( !mbList[n]) {
		mbList[n] = new MemBlock(sizeof(SlfHeader) + n*pageSize_*BLOCK_SIZE, pageSize_);
		//mbList[n]-> load(f);
	}
	mbList[n]->isDirty = true;
	return write(mbList[n]->getP(fpos) );

	/*lock_.acquire_write_lock();

	 long overFlowAddress = ftell(f);

	 size_t recSize;
	 byte leafFlag;
	 byte rightFlag;

	 if (!f) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (0 != fseek(f, fpos, SEEK_SET)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 isLeaf = (down == bottom) ? 1 : 0;

	 isHasRight = (right != NULL) ? 1 : 0;

	 leafFlag = isLeaf ? 1 : 0;
	 rightFlag = isHasRight ? 1 : 0;

	 // write the leafflag 
	 if (1 != fwrite(&leafFlag, sizeof(byte), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 // write the rightflag 
	 if (1 != fwrite(&rightFlag, sizeof(byte), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 if (isHasRight) {
	 rfpos = right->fpos;
	 if (1 != fwrite(&rfpos, sizeof(long), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 }

	 // write the addresses of the child pages
	 if ( !isLeaf) {
	 dfpos = down->fpos;
	 if (1 != fwrite(&dfpos, sizeof(long), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 }

	 object_.reset(new DbObj);
	 write_image(element, object_);

	 // write the size of the DbObj.
	 recSize = object_->getSize();
	 if (1 != fwrite(&recSize, sizeof(size_t), 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }

	 byte *pd;
	 pd = ( byte* )object_->getData();
	 if (recSize <= maxDataSize_) {
	 if (1 != fwrite(pd, recSize, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 } else {
	 if (1 != fwrite(pd, maxDataSize_, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (1 != fwrite(&overFlowAddress, sizeof(long), 1, f) ) {
	 lock_.release_write_lock();
	 return false;
	 }
	 if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 //update would waste the disk, need to improve here.
	 if (1 != fwrite(pd + maxDataSize_, recSize - maxDataSize_, 1, f)) {
	 lock_.release_write_lock();
	 return false;
	 }
	 }
	 isLoaded = true;
	 isDirty = false;
	 return true;*/

}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
		DataType, LockType, Alloc>::write(char* pBuf) {

	//static int gw;
	//cout<<"writing times = "<<++gw<<endl;
	//lock_.acquire_write_lock();
	if (!isDirty) {
		return true;
	}

	if (!isLoaded) {
		return true;
	}

	char* p = pBuf;
	
	//size_t overFlowSize_ = pageSize_;

	//long overFlowAddress = ftell(f);

	if ( !pBuf) {
		//lock_.release_write_lock();
		return false;
	}

	size_t recSize;

	//cout<<"writing..."<<element.get_key()<<endl;
	//cout<<"height: "<<height<<endl;
	memcpy(p, &height, sizeof(int));
	p += sizeof(int);
	for (int i=0; i<height; i++) {
		if (right[i]) {
			rfpos[i] = right[i]->fpos;

		} else {
			rfpos[i] = 0;
		}
		//cout<<"right fpos "<<i<<" : "<<rfpos[i]<<endl;

	}

	memcpy(p, &rfpos[0], sizeof(long)*MAX_LEVEL);
	p += sizeof(long)*MAX_LEVEL;

	object_.reset(new DbObj);
	write_image(element, object_);

	// write the size of the DbObj.
	recSize = object_->getSize();
	memcpy(p, &recSize, sizeof(size_t));
	p += sizeof(size_t);

	char *pd;
	pd = ( char* )object_->getData();
	if (recSize <= maxDataSize_) {
		memcpy(p, pd, recSize);
		p += recSize;
	} else {
		assert(false);
		/*if (1 != fwrite(pd, maxDataSize_, 1, f)) {
		 lock_.release_write_lock();
		 return false;
		 }
		 if (1 != fwrite(&overFlowAddress, sizeof(long), 1, f) ) {
		 lock_.release_write_lock();
		 return false;
		 }
		 if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
		 lock_.release_write_lock();
		 return false;
		 }
		 //update would waste the disk, need to improve here.
		 if (1 != fwrite(pd + maxDataSize_, recSize - maxDataSize_, 1, f)) {
		 lock_.release_write_lock();
		 return false;
		 }*/
	}

	isLoaded = true;
	isDirty = false;
	return true;

}

NS_IZENELIB_AM_END
#endif /*SKIPNODE_H_*/
