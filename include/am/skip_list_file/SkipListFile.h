/**
 * @file SkipListFile.h
 * @brief The header file of SkipListFile class.
 */
#ifndef _SkipListFile_H_
#define _SkipListFile_H_

#include "SlfHeader.h"
#include "SkipNode.h"
#include "MemMap.h"

#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <set>
#include <am/concept/DataType.h>

using namespace std;

NS_IZENELIB_AM_BEGIN

#define KEY element.get_key()

/**
 * @brief The definition and implementation of the
 *        SkipList(Actually, it is a deterministic skip list)
 */
template<typename KeyType, typename ValueType=NullType, typename LockType=NullLock, typename Alloc=std::allocator<DataType<KeyType,ValueType> > >class SkipListFile
: public AccessMethod<KeyType, ValueType, LockType, Alloc>
{
public:
	typedef DataType<KeyType, ValueType> DataType;
	typedef SkipNode<DataType, LockType, Alloc> SkipNode;

	SkipListFile(const std::string& fileName, const size_t& minDegree = 2);
	virtual ~SkipListFile();

	void setPageSize(size_t maxDataSize, size_t overFlowSize) {
		maxDataSize_ = maxDataSize;
		size_t pageSize = sizeof(int) + MAX_LEVEL*sizeof(long) + maxDataSize;

		//	size_t pageSize = sizeof(byte) + 4*sizeof(long) + maxDataSize;

		sfh_.pageSize = pageSize;
		sfh_.overFlowSize = overFlowSize;

		SkipNode::setDataSize(maxDataSize,
				pageSize, overFlowSize);
	}

	/**
	 * 	\brief return the file name of the SequentialDB
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/**
	 * 	We would peroidically flush the memory items, according to the cache Size. 
	 */

	void setCacheSize(size_t sz) {
		sfh_.cacheSize = sz;
	}

	const KeyType* find_min() const;

	const KeyType* find_max() const;

	ValueType* find(const KeyType& x);
	/*ValueType* find(const KeyType& x) {
	 return (ValueType*)((const SkipListFile*)this)->find(x);
	 }*/

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const KeyType& key, const ValueType& val) {
		return update(DataType(key, val) );
	}

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const DataType& rec) {
		if( find (rec.get_key() ) ) {
			//to be implemented
		}
		else {
			return insert(rec);
		}
		return true;
	}

	int num_items() const {
		return sfh_.numItem;
	}
	void release();
	void release(SkipNode * ph);
	bool insert(const KeyType& key, const ValueType& value)
	{
		return insert( DataType(key, value) );
	}
	bool insert(const DataType& x);
	bool insert1(const DataType& x);

	bool del(const KeyType& x);

	SkipNode* searchPre(const KeyType& key)
	{
		SkipNode* x = header_;
		int h = x->height-1;
		if (sfh_.numItem == 0)
		return NULL;
		while (h>=0) {
			x->loadRight(h, dataFile_);
			while (x->right[h] && comp_(key, x->right[h]->KEY)> 0 )
			{	x = x->loadRight(h, dataFile_);
				if(x->right[h])x->loadRight(h, dataFile_);
			}
			if( x->right[h] && comp_( key, x->right[h]->KEY) == 0 ) {
				return x;
			}
			--h;
		}
		return NULL;
	}

	/**
	 *  \brief given a  key, get next key
	 */
	KeyType getNext(const KeyType& key) {
		SkipNode *p;
		p = serachPre(key);
		if(p && p->right[0] )return p->right[0]->right[0]->KEY;
		else
		return KeyType();
	}

	/**
	 *  \brief given a  key, get next key
	 */
	KeyType getPrev(const KeyType& key) {
		SkipNode *p;
		p = serachPre(key);
		if(p)return p->KEY;
		else
		return KeyType();
	}

	SkipNode* firstNode() {
		return header_->right[0];
	}

	void commit() {
		//cout<<"commiting... "<<endl;
		//sfh_.headerPos = header_->fpos;
		sfh_.toFile(dataFile_);

		//sfh_.display();

		/*	for(int i=0; i<(int)sfh_.nNode; i++) {
		 cout<<"commit node: "<<i<<endl;
		 nodeMap_[i]->write(dataFile_);
		 }*/

		while ( !dirtyPages_.empty() ) {
			SkipNode* p = dirtyPages_.back();
			//	p->display();
			dirtyPages_.pop_back();
			if(p)p->write(dataFile_);
		}
		for(vector<MemBlock*>::iterator it=mbList.begin(); it !=mbList.end(); it++ ) {
			if(*it)(*it)->flush(dataFile_);
		}

	}

	void flush() {
		commit();
		//cout<<"commit finised\n"<<endl;
		//	SkipNode *ph = header_;
		//	vector<SkipNode*> headers;

		/*	while( ph != SkipNode::bottom ) {
		 headers.push_back( ph );
		 ph = ph->down;
		 }*/

		vector<MemBlock*>::iterator it, it1;
		for(it=mbList.begin(); it != mbList.end(); it++ ) {
			it1 = it;
			delete (*it1);
			*it1 = 0;
		}
		//cout<<"sdfsf "<<headers.size()<<endl;
		/*	while ( !headers.empty() ) {
		 if (SkipNode::activeNodeNum
		 < sfh_.cacheSize/2 && isUnload_) {
		 #ifdef DEBUG
		 cout<<"AcitveNodeNum= "
		 <<SkipNode::activeNodeNum
		 <<"\n stop unload\n";
		 #endif

		 break;
		 }
		 //SkipNode* ph = headers.back();
		 //headers.pop_back();
		 //release( ph );
		 }*/

	}

	void display() {
		print_list();
	}

	bool open() {
		// We're creating if the file doesn't exist.
		struct stat statbuf;
		bool creating = stat(fileName_.c_str(), &statbuf);

		dataFile_ = fopen(fileName_.c_str(), creating ? "w+b" : "r+b");
		if ( 0 == dataFile_) {
			cout<<"Error in open(): open file failed"<<endl;
			return false;
			//throw( SkipListFileException(OPEN_FILE_ERROR);
		}
		// Create a new node
		bool ret = false;
		if (creating) {

#ifdef DEBUG
			cout<<"creating...\n"<<endl;
			sfh_.display();
#endif

			// when creating, write the node to the disk.
			// remember that the first four bytes contain
			// the address of the root node.			

			// If creating, allocate a node instead of
			// reading one.
			header_ = allocateNode_();
			header_->height = 1;
			//sfh_.headerPos = header_->fpos;
			sfh_.toFile(dataFile_);

			dirtyPages_.push_back(header_);
			ret = true;
		} else {
			if ( !sfh_.fromFile(dataFile_) ) {
				return false;
			} else {
				if (sfh_.magic != 0x061561) {
					cout<<"Error, read wrong file header_\n"<<endl;
					return false;
				}
#ifdef DEBUG
				cout<<"open exist...\n"<<endl;
				sfh_.display();
#endif

				mbList.resize( sfh_.nNode/BLOCK_SIZE+1 );
				header_ = new SkipNode;
				header_ -> fpos = sfh_.headerPos;
				header_ -> read(dataFile_);

				//header_->display();
				ret = true;
			}
			// If note creating, just create and read
			// rather than allocating.
		}

		SkipNode::setDataSize(maxDataSize_,
				sfh_.pageSize, sfh_.overFlowSize);
		return ret;
	}

private:
	bool is_empty() const;
	void print_list() const;
private:
	string fileName_;
	size_t minDegree_;
	FILE* dataFile_;

	SkipNode *header_; // The list
	SlfHeader sfh_;
	size_t maxDataSize_;
private:
	bool isUnload_;
	vector<SkipNode*> dirtyPages_;
	CompareFunctor<KeyType> comp_;
private:
	//cache effectness?
	map<int, SkipNode*> nodeMap_;
private:

	/**
	 * 	 \brief open the database. 
	 *   \return if open successfully, return true, otherwise return false.
	 * 
	 *   Everytime  we use the database, we mush open it first.  
	 */

	void flushCache_() {
		if (SkipNode::activeNodeNum
>				sfh_.cacheSize) {
#ifdef DEBUG
			cout<<"flush ... "<<sfh_.cacheSize << endl;
			cout<<"activeNode: " <<SkipNode::activeNodeNum
			<<endl;
#endif
			isUnload_ = true;
			flush();
		}

	}

	SkipNode* allocateNode_() {

		SkipNode* newNode;
		newNode = new SkipNode;

		//fseek(dataFile_, 0L, SEEK_END);
		//long len = ftell(dataFile_);
		//newNode->fpos = len;

		newNode->isLoaded = true;
		newNode->height = 0;
		newNode->nid = sfh_.nNode;

		//	nodeMap_.insert( make_pair(newNode->nid, newNode) );
		newNode->fpos = sizeof(SlfHeader) + sfh_.pageSize*sfh_.nNode;

		/*if( sfh_.nNode % BLOCK_SIZE == 0){
		 MemBlock* newBlock = new MemBlock( sizeof(SlfHeader)+ sfh_.pageSize*sfh_.nNode, sfh_.pageSize);
		 mbList.push_back( newBlock );
		 }*/
		if( sfh_.nNode % BLOCK_SIZE == 0) {
			mbList.resize( sfh_.nNode/BLOCK_SIZE+1 );
		}
		sfh_.nNode++;

		return newNode;
	}

	/*SkipNode* search_(const KeyType& key, int height, vector<SkipNode*>& update) {
	 SkipNode* x = header_;
	 int h = x->height-1;
	 if (sfh_.numItem == 0)
	 return false;
	 while ( h>=0 ) {
	 x->loadRight(h, dataFile_);
	 while (x->right[h] && x->right[h]->height> height && comp_(key, x->right[h]->KEY)> 0 )
	 {
	 x = x->loadRight(h, dataFile_);
	 if(x->right[h])x->loadRight(h, dataFile_);
	 }
	 update.push_back(x);
	 if( x->right[h] && comp_(key, x->right[h]->KEY) == 0 ) {
	 return x->right[h];
	 }
	 --h;
	 }
	 cout<<"\nsearch_ pre: \n"<<endl;
	 for(int i=0; i<(int)update.size(); i++) {
	 cout<<update[i]->KEY<<endl;
	 }
	 return 0;
	 }*/

	void promote_(SkipNode* x, vector<SkipNode*>& update, int from, int to) {
		//	int tb = totalBand(sfh_.numItem);


		if(from == to)return;
		//protome to the top
		//int h = x->height;
		x->height = to;

		SkipNode* temp;
		int h= from;
		int i = update.size()-1;
		for(; i>=0; i--, h++)
		{
			temp = update[i];
			//	cout<<h<<"update "<<update[i]->KEY<<endl;
			if( h<=to && h>from ) {
				x->right[h-1] = temp->loadRight(h-1, dataFile_);
				if( temp->right[h-1] )
				temp->right[h-1]->left[h-1] = x;

				temp->right[h-1] = x;
				x->left[h-1] = temp;
				dirtyPages_.push_back(temp);
			}
		}
		//cout<<" !!!!!after promote "<<x->KEY<<endl;
		dirtyPages_.push_back(x);
		//display();

	}

	void demote_(vector<SkipNode*>& update, int from, int to)
	{
		if(from == to)return;
		//cout<<"\nsearch_ pre: \n"<<endl;
		//	for(int i=0; i<(int)update.size(); i++) {
		//		cout<<update[i]->KEY<<endl;
		//	}

		int h = update[1]->height;
		//cout<<"height: "<<update[1]->KEY;
		//cout<<"====="<<endl;
		//		cout<<h<<endl;
		//		cout<<from<<endl;
		//		cout<<to<<endl;
		for(int i=1; i<(int)update.size() && h>0; i++,h--) {
			//cout<<"height 0: "<<update[i]->height<<endl;
			//cout<<"demote 0 "<<update[i]->KEY<<endl;
			if(update[i] == header_) {continue;}
			//cout<<h<<" "<<update[i]->KEY<<endl;
			//cout<<update[i]->left[h-1];
			//if( h<=from && h>to && update[i]->left[h-1])
			if( update[i]->height> to )
			{
				//cout<<h-1<<" demoting "<<update[i]->KEY<<endl;
				//	if( update[i]->left[h-1] )cout<<update[i]->left[h-1]->KEY<<endl;
				//	if(update[i]->right[h-1])cout<<update[i]->right[h-1]->KEY<<endl;
				if(update[i]->left[h-1])
				update[i]->left[h-1]->right[h-1] = update[i]->loadRight(h-1, dataFile_);
				if( update[i]->right[h-1] )
				update[i]->right[h-1]->left[h-1] = update[i]->left[h-1];

				update[i]->height = update[i]->height - 1;
			}
			//cout<<"height: "<<update[i]->height<<endl;
			//cout<<"!!!after demote "<<update[i]->KEY<<endl;
			//display();
			dirtyPages_.push_back(update[i]);
			if( update[i]->left[h-1] )dirtyPages_.push_back(update[i]->left[h-1]);
		}

	}

	/*void demote1_(vector<SkipNode*>& update, int fromBand, int toBand)
	 {
	 cout<<"demote,...\n";
	 cout<<"from "<<fromBand<<endl;
	 cout<<"to "<<toBand<<endl;
	 cout<<"\nsearch_ pre: \n"<<endl;
	 for(int i=0; i<(int)update.size(); i++) {
	 cout<<update[i]->KEY<<endl;
	 }
	 assert(fromBand <= toBand);
	 int b = fromBand;
	 int h =0;
	 int newh = 0;
	 vector<SkipNode*> vs;
	 typename vector<SkipNode*>::iterator it;
	 for(int i=1; i<(int)update.size() && b<toBand; i++) {

	 //if(update[i] == update[i-1])continue;
	 if( whichBand( sfh_.height-update[i]->height+1 ) == b && update[i] != header_) {
	 cout<< "demoting...\n" <<update[i]->KEY<<endl;
	 cout<<vBandHeight[b]<<endl;
	 cout<<rand()%vBandHeight[b]<<endl;

	 h = update[i]->height;
	 //newh = h - rand()%vBandHeight[b]-1;
	 newh = h - vBandHeight[b];
	 //newh = update[i]->height - rand()%(vBandHeight[b]+1);
	 cout<<"to newh="<<newh<<endl;
	 vs.clear();
	 search_(update[i]->KEY, newh, vs);
	 it = vs.begin();
	 for(; it != vs.end(); it++) {
	 if((*it)->right[h-1] == update[i] && h> newh) {
	 (*it)->right[h-1] = (*it)->right[h-1]->right[h-1];
	 }
	 h--;
	 }
	 update[i]->height = newh;
	 b++;
	 }

	 }
	 display();
	 }*/

	/*void demote_(vector<SkipNode*>& update, int fromBand, int toBand) {
	 if(fromBand >= toBand)return;
	 for(int b=fromBand; b<toBand; b++)
	 {
	 SkipNode* p = randomSelect_(b);
	 vector<SkipNode*> vs_before, vs_after;
	 typename vector<SkipNode*>::iterator it;
	 search_(p->KEY, vs_before);
	 for(it=vs_before.begin();it != vs_before.end(); it++)
	 {
	 (*it)->counter[fromBand]--;
	 }

	 p->height -= vBandHeightAcc[toBand]-vBandHeightAcc[fromBand];
	 search_(p->KEY, vs_after);

	 it = vs_after.begin();
	 (*it)->counter[toBand]++;
	 int h = (*it)->height;
	 for(++it; it != vs_after.end(); it++)
	 {
	 (*it)->counter[toBand]++;
	 if( (*it)->right[h] == p ) {
	 (*it)->right[h] = p->right[h-1];
	 p->right[h] = 0;
	 }
	 h--;
	 }

	 display();
	 }
	 }*/

	/*SkipNode * randomSelect_(int band) {
	 SkipNode* x = header_;
	 SkipNode* s;
	 SkipNode* s1;
	 int c, c1;
	 int h = x->height-1;
	 int acc = 0;
	 while( h>=0 ) {
	 s = x;
	 if( s->right[h] ) {
	 s1 = s->loadRight(h, dataFile_);
	 c = s->counter[h];
	 c1 = s1->counter[h];
	 if( rand()% c < c1 )s = s->loadRight(h,dataFile_);
	 else {
	 h--;acc++;
	 }
	 }
	 else {
	 h--;acc++;
	 }
	 cout<<acc<<" >= "<<vBandHeightAcc[band]<<endl;
	 if( acc >= vBandHeightAcc[band] )
	 cout<<" randomSelect = "<< s->KEY<<" with h="<<h<<endl;
	 return s;
	 }
	 }*/

};

/**
 * @brief Find item x in the tree.
 *
 * Return the matching item or m_infinity if not found.
 */

template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> ValueType * SkipListFile<KeyType, ValueType, LockType,
		Alloc>::find(const KeyType & key) {

	SkipNode *p =searchPre(key);
	ValueType* pv= NULL;
	if (p) {
	   pv = new ValueType( p->right[0]->element.get_value() );
	}
	return pv;

	/*SkipNode* x = header_;
	 int h = x->height-1;
	 if (sfh_.numItem == 0)
	 return false;
	 vector<SkipNode*> update;
	 while (h>=0) {
	 x->loadRight(h, dataFile_);
	 while (x->right[h] && comp_(key, x->right[h]->KEY)> 0 )
	 {	x = x->loadRight(h, dataFile_);
	 if(x->right[h])x->loadRight(h, dataFile_);
	 }
	 //update.push_back(x);
	 //x->right[h]->left[h]= x;
	 if( x->right[h] && comp_( key, x->right[h]->KEY) == 0 ) {
	 //cout<<"find height="<<h<<endl;
	 //promote_(x->right[h], update, h+1, sfh_.height);
	 int fromBand = 1;
	 int toBand = whichBand(sfh_.height - h +1);
	 demote_(update, fromBand, toBand);
	 //demote_(update, sfh_.height, h+1);
	 //display();
	 ValueType* pv = new ValueType( x->right[h]->element.get_value() );
	 return pv;
	 }
	 --h;
	 //x = x->loadDown(dataFile_);
	 }
	 return 0;*/
}

/**
 * @brief Construct the tree.
 *
 * inf is the largest DataType
 * and is used to signal failed finds.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> SkipListFile<KeyType, ValueType, LockType, Alloc>::SkipListFile(
		const string& fileName, const size_t& degree) :
	fileName_(fileName), minDegree_(degree) {

	isUnload_ = false;
	sfh_.degree = degree;

	//SkipNode::bottom = new SkipNode;
	//SkipNode::bottom->right = SkipNode::bottom->down = SkipNode::bottom;
	//open_();

}

/**
 * @brief Destructor.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> SkipListFile<KeyType, ValueType, LockType, Alloc>::~SkipListFile() {
	release();
	//header_->unload();
	//header_ = 0;
	//delete SkipNode::bottom;
	//SkipNode::releaseBottom();
}

/**
 * @brief Insert item x into the SkipList.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool SkipListFile<KeyType, ValueType, LockType, Alloc>::insert1(
		const DataType& v) {

	SkipNode *t, *x;

	x = header_;
	register int h = header_->height-1;

	vector<SkipNode*> update;

	while (h>=0) {
		//x->loadRight(h, dataFile_);
		while (x->right[h] && comp_(v.get_key(), x->right[h]->KEY)> 0) {
			x = x->loadRight(h, dataFile_);
			//x->loadRight(h, dataFile_);
		}
		update.push_back(x);
		if( x->right[h] && comp_( v.get_key(), x->right[h]->KEY) == 0 ) {
			/*promote_(x->right[h],update);
			 int fromBand = 1;
			 int toBand = whichBand(h);
			 demote_(fromBand, toBand);*/
			return false;
		}
		--h;
	}

	int height = coin_flip(sfh_.degree);
	if( height >= header_->height ) {
		height = header_->height;
		++sfh_.height;
		++header_->height;
		dirtyPages_.push_back(header_);
		assert(header_->height <= MAX_LEVEL);
	}
	t = allocateNode_();
	t->element = v;
	t->height = height;

	//promote_(t, update, 0, height);

	//get band
	// int b = whichBand(height);

	//SkipNode* y = SkipNode::bottom;
	typename
	vector<SkipNode*>::iterator it = update.end();
	--it;
	for(int i=0; i<height; i++)
	{
		SkipNode* p = *it;

		//also update counter.
		// p->counter[b]++;
		it--;
		//update.back();	
		//update.pop_back();
		//if(right[i])
		t->right[i] = p->right[i];
		if( p->right[i] )p->right[i]->left[i] = t->right[i];
		//t->rfpos[i] = p->rfpos[i];

		//t->down = y;	
		p->right[i] = t;
		//t->left[i] = p ;
		//p->rfpos[i] = t->fpos;
		//cout<< p->rfpos[i]<<endl;

		dirtyPages_.push_back(p);
		//y = t;
	}
	dirtyPages_.push_back(t);
	/*if (header_->right[header_->height - 1] != NULL) {
	 header_->height++;
	 sfh_.height++;
	 dirtyPages_.push_back(t);
	 //cout<<"reach here 2"<<endl;
	 }*/

	sfh_.numItem++;
	return true;
}

/**
 * @brief Insert item x into the SkipList.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool SkipListFile<KeyType, ValueType, LockType, Alloc>::insert(
		const DataType& v) {
	SkipNode *t, *x;

	//SkipNode::bottom->element = v;

	//cout<<" insert key=" <<v.get_key() <<endl;
	//cout<<" sfh_.numItem "<<sfh_.numItem<<endl;

	x = header_;
	int h = x->height - 1;
	/*while (x->down != SkipNode::bottom && (!x->right || comp_(v.get_key(),
	 x->right->KEY)<0 ) ) {
	 cout<<" header down "<<endl;	
	 x = x->loadDown(dataFile_);
	 }
	 x->display();*/

	while (h>=0 && x) {
		//cout<< " rounding... "<<h<<endl;
		//x->display();
		//cout<<"cmp result: "<<comp_(v.get_key(), x->KEY)<<endl;
		//x->loadRight(h, dataFile_);		
		while (x->right[h] && comp_(v.get_key(), x->right[h]->KEY)> 0) {
			//cout<<h<<") passing "<<x->KEY;
			//cout<< " -> "<< x->right[h]->KEY;
			x = x->loadRight(h, dataFile_);
			//x->loadRight(h, dataFile_);
			//cout<<"no right? "<<(x->right[h] == 0)<<endl;

		}
		//
		if( x->right[h] && comp_( v.get_key(), x->right[h]->KEY) == 0 ) {
			//cout<<"\nfound!!!!!"<<endl;
			return false;
		}
		//cout<<"pass through "<<x->KEY<<endl;
		//if ( (x->down == SkipNode::bottom) &&  x->right && ( comp_( v.get_key(), x->right->KEY) == 0 ) )
		//return false; // already inserted

		SkipNode* temp = x;

		//temp = x->loadDown(dataFile_);
		//assert(temp == SkipNode::bottom);
		bool isSplit = false;
		if( h-1 >=0 ) {
			if( temp->right[h-1] ) temp = temp->loadRight(h-1,dataFile_);
			if( temp->right[h-1] ) temp = temp->loadRight(h-1, dataFile_);
			if( temp->right[h-1] ) {
				temp = temp->loadRight(h-1, dataFile_);
				isSplit = true;
				//if( temp->right ) temp = temp->loadRight(dataFile_);
			}
		}

		//cout<<"reach skyline "<<(x->down == SkipNode::bottom)<<endl;
		//cout<<"temp->right: "<<temp->right<<endl;
		//cout<<"x->right: "<<x->right<<endl;
		//cout<<"isSplit: "<<isSplit<<endl;


		if ( (h == 0 )
		|| ( isSplit
				&& (x->right[h] == temp->right[h-1]
						|| ( (x->right[h] && temp->right[h-1])
								&& (temp->right[h-1]->isLoaded)
								&& (x->right[h]->KEY> temp->right[h-1]->KEY) ) ) ) )
		/*	if ( (x->down == SkipNode::bottom)
		 || ( isSplit
		 &&  (x->right == temp->right)  )  ) */
		{
			//if(x->right && temp->right) cout<<x->right->KEY<<" vs "<<temp->right->KEY<<endl;
			//cout<<"actual inserting ... ";
			//x->display();
			//t = new SkipNode(x->element, x->right, x->down->right->right);

			if( h == 0 ) {
				//cout<<"new node"<<endl;
				t = allocateNode_();
				t->element = v;
				//t->height = 0;
			}
			else {
				t = x->right[h-1] ->right[h-1];
				//t->element = x->right[h-1]->right[h-1]->element;
			}
			//increase height
			t->height++;
			t->right[h] = x->right[h];
			//if(x->right[h]) t->isHasRight = true;
			//t->down = x->down->right[h]->right[h];

			t->isDirty = true;
			dirtyPages_.push_back(t);
			//t->display();

			x->right[h] = t;
			//x->isHasRight = true;
			//x->element = x->down->right->element;
			x->isDirty = true;
			dirtyPages_.push_back(x);
			//t->display();
		}
		//cout<<"load down..."<<endl;
		--h;
		//x = x->loadDown(dataFile_);
		//cout<<" | "<<endl;
		//cout<<x->KEY;
	}

	if (header_->right[header_->height-1] != NULL) {
		/*t = allocateNode_();
		 t->down = header_;
		 t->isLeaf = false;
		 header_ = t;*/
		header_->height++;
		dirtyPages_.push_back(t);
		//cout<<"reach here 2"<<endl;
	}
	sfh_.numItem++;
	return true;
}

template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool SkipListFile<KeyType, ValueType, LockType, Alloc>::del(
		const KeyType& key) {

	SkipNode* x = header_;
	int h = x->height - 1;
	SkipNode* p = 0;
	while (h>=0) {
		while (x->right[h] && comp_(key, x->right[h]->KEY)> 0 ) {
			x = x->loadRight(h,dataFile_);
		}
		if( !x )return 0;
		if( x->right[h] && key == x->right[h]->KEY) {
			x->isDirty = true;
			dirtyPages_.push_back(x);
			p = x->right[h];
			x->right[h] = p->right[h];
		}
		--h;
	}

	if ( header_->height>1 && header_->right[header_->height-2] == NULL) {
		--header_->height;
		dirtyPages_.push_back(header_);
	}
	if( p ) {
		delete p;
		p = 0;
		--sfh_.numItem;
		//dirtyPages_.push_back(p);
		return true;
	}

	return 0;
}

/**
 * @brief Find the smallest item in the tree.
 *
 * Return smallest item or m_infinity if empty.
 */

template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> const KeyType* SkipListFile<KeyType, ValueType,
		LockType, Alloc>::find_min() const {

	SkipNode *current = header_;
	if (header_->right[0])
		return &(header_->right[0]->KEY);
		else
			return NULL;
	}
	/**
	 * @brief Find the largest item in the tree.
	 *
	 * Return the largest item or m_infinity if empty.
	 */

template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> const KeyType* SkipListFile<KeyType, ValueType,
		LockType, Alloc>::find_max() const {
	if (num_items() == 0)
		return NULL;

	SkipNode *current = header_;
	int h = current->height-2;
	while (h>=0) {
		if (current->right[h] != NULL)
			current = current->right[h];
		else {
			if (h == 0)
				return &(current->KEY);
				else
				h--;
			}
		}
		return NULL;
	}

	/**
	 * @brief Make the tree logically empty.
	 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> void SkipListFile<KeyType, ValueType, LockType, Alloc>::release() {
	//SkipNode* p = header_;
	//while( p )
	{
		//to do
		//should keep a list for caching
	}
	/*while (header_ != SkipNode::bottom) {
	 SkipNode *headDown = header_->down;
	 release(header_);
	 header_ = headDown;
	 }*/

	vector<MemBlock*>::iterator it, it1;
	for (it = mbList.begin(); it != mbList.end(); it++) {
		it1 = it;
		delete *it1;
	}
}

/**
 * @brief Make the tree logically empty.
 */
/*template <typename KeyType, typename ValueType, typename LockType,
 typename Alloc> void SkipListFile<KeyType, ValueType, LockType, Alloc>::release(
 SkipNode *ph) {
 //cout<<"release..."<<endl;
 SkipNode *right;
 while (ph) {
 if (ph)
 right = ph->right;
 else {
 break;
 }
 ph = right;
 }
 }*/

/**
 * @brief Test if the tree is logically empty.
 *
 * Return true if empty, false otherwise.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> inline bool SkipListFile<KeyType, ValueType, LockType,
		Alloc>::is_empty() const {
	return (sfh_.numItem == 0);
}

/**
 * @brief Print the SkipList.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> void SkipListFile<KeyType, ValueType, LockType, Alloc>::print_list() const {
	SkipNode *current = header_;
	//header_->display();
	int h = header_->height-1;
	int a = 0;
	int b = 1;
	bool counted = false;
	while (h>=0) {
		current = header_;
		while (current) {
			cout<<current->element.get_key()<<" -> ";
			/*if (current->left[h])
			 cout<<" <- ";
			 cout<<current->element.get_key()<<"( "; //h: "<<current->height ;
			 //cout<<vBandHeightAcc[b]<<" vs "<<a;
			 if (vBandHeightAcc[b] == a || h==0) {
			 cout<<"b"<<b;
			 counted = true;
			 } else {
			 counted = false;
			 }
			 cout<<" ) ";

			 cout<<" -> ";*/
			current = current->right[h];
		}
		cout<<" tail "<<endl;
		if (counted)
			b++;
		h--;
		a++;

	}
	//cout<<"Down"<<endl;
}

NS_IZENELIB_AM_END

#endif

