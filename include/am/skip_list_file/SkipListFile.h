/**
 * @file SkipListFile.h
 * @brief The header file of SkipListFile class.
 */
#ifndef _SkipListFile_H_
#define _SkipListFile_H_

#include "SlfHeader.h"
#include "SkipNode.h"

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
	typedef SkipNode* NodeKeyLocn;
public:
	SkipListFile(const std::string& fileName);
	virtual ~SkipListFile();

	void setPageSize(size_t maxDataSize) {
		maxDataSize_ = maxDataSize;
		size_t pageSize = sizeof(int) + MAX_LEVEL*sizeof(long) + maxDataSize;		
		sfh_.pageSize = pageSize;
	}

	void setDegree(int degree) {
		minDegree_ = degree;
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

	ValueType* find(const KeyType& x);

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
		SkipNode* p = NULL;
		search(rec.get_key(), p);
		if( p ) {
			p->element = rec;
			p->isDirty = true;			
		}
		else {
			return insert(rec);
		}
		return true;
	}

	int num_items() const {
		return sfh_.numItem;
	}
	
	bool insert(const KeyType& key, const ValueType& value)
	{
		return insert( DataType(key, value) );
	}
	
	bool insert(const DataType& x);
	
	bool insert1(const DataType& x);

	bool del(const KeyType& x);
	
	
	NodeKeyLocn search(const KeyType& key)
	{
		NodeKeyLocn locn = 0;
		search(key, locn);
		return locn;
	}

	bool search(const KeyType& key, NodeKeyLocn& locn)
	{
		SkipNode* x = header_;
		int h = x->height-1;
		if (sfh_.numItem == 0)
		return false;
		while (h>=0) {
			x->loadRight(h, dataFile_);
			while (x->right[h] && comp_(key, x->right[h]->KEY)> 0 )
			{	x = x->loadRight(h, dataFile_);
				if( x->right[h] )x->loadRight(h, dataFile_);
			}
			if( x->right[h] && comp_( key, x->right[h]->KEY) == 0 ) {
				locn =  x->right[h];
				return true;
			}
			--h;
		}
		return false;
	}


	const KeyType* find_min() const;

	const KeyType* find_max() const;

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

	/**
	 *  if the input key exists, just return itself, otherwise return 
	 * 	the smallest existing key that bigger than it.   
	 */
	KeyType getNearest(const KeyType& key) {

		SkipNode* x = header_;
		KeyType rk;

		int h = x->height-1;
		if (sfh_.numItem == 0)
		return rk;
		while (h>=0) {
			x->loadRight(h, dataFile_);
			while (x->right[h] && comp_(key, x->right[h]->KEY)> 0 )
			{	x = x->loadRight(h, dataFile_);
				if(x->right[h])x->loadRight(h, dataFile_);
			}
			if(h == 0 && x->right[h] ) {
				return x->right[h]->KEY;
			}
			if( x->right[h] && comp_( key, x->right[h]->KEY) == 0 ) {
				return key;
			}
			--h;
		}
		return rk;

	}

	
	NodeKeyLocn get_first_locn()
	{
		return header_->right[0];
	}
	/**
	 * 	\brief get the next or prev item.
	 */
	bool
	seq(NodeKeyLocn& locn, DataType& rec, ESeqDirection sdir = ESD_FORWARD) {
		if(sdir == ESD_FORWARD) {
			if(locn) {
				locn = locn->loadRight(0, dataFile_);
				rec = locn->element;
				cout<<"seq: "<<rec.key<<endl;
				return true;
			}
			else {
				return false;
			}
		}
		else
		{
			//to to implemented
			return false;
		}

	}

	SkipNode* firstNode() {
		return header_->right[0];
	}

	void commit() {
		//cout<<"commiting... "<<endl;
		//sfh_.headerPos = header_->fpos;
		sfh_.toFile(dataFile_);
		//sfh_.display();

		while ( !dirtyPages_.empty() ) {
			SkipNode* p = dirtyPages_.back();			
			dirtyPages_.pop_back();
			if (p)
			p->write(dataFile_);
		}
		for (vector<MemBlock*>::iterator it=mbList.begin(); it !=mbList.end(); it++) {
			if (*it)
			(*it)->flush(dataFile_);
		}

	}

	void flush() {
		commit();	
		vector<MemBlock*>::iterator it, it1;
		for (it=mbList.begin(); it != mbList.end(); it++) {
			it1 = it;
			delete (*it1);
			*it1 = 0;
		}	

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

				mbList.resize(sfh_.nNode/BLOCK_SIZE+1);
				if(sfh_.numItem == 0) {
					header_ = allocateNode_();
					header_->height = 1;
					sfh_.toFile(dataFile_);
					dirtyPages_.push_back(header_);
				}
				else {
					header_ = new SkipNode;
					header_ -> fpos = sfh_.headerPos;
					header_ -> read(dataFile_);
				}

				//header_->display();
				ret = true;
			}
			// If note creating, just create and read
			// rather than allocating.
		}

		SkipNode::setDataSize(maxDataSize_, sfh_.pageSize);
		return ret;
	}

	bool close(){
		release();
		return true;
	}
	
private:
	bool is_empty() const;
	void print_list() const;
	void release();
	void release(SkipNode * ph);
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
		
		if (sfh_.nNode % BLOCK_SIZE == 0) {
			mbList.resize(sfh_.nNode/BLOCK_SIZE+1);
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

	/*void promote_(SkipNode* x, vector<SkipNode*>& update, int from, int to) {

		if (from == to)
		return;
	
		x->height = to;

		SkipNode* temp;
		int h= from;
		int i = update.size()-1;
		for (; i>=0; i--, h++) {
			temp = update[i];		
			if (h<=to && h>from) {
				x->right[h-1] = temp->loadRight(h-1, dataFile_);
				if (temp->right[h-1])
				temp->right[h-1]->left[h-1] = x;

				temp->right[h-1] = x;
				x->left[h-1] = temp;
				dirtyPages_.push_back(temp);
			}
		}		
		dirtyPages_.push_back(x);	

	}*/

	
	/*
	void demote_(vector<SkipNode*>& update, int from, int to) {
		if (from == to)
		return;	

		int h = update[1]->height;

		for (int i=1; i<(int)update.size() && h>0; i++, h--) {
			
			if (update[i] == header_) {
				continue;
			}
			
			if (update[i]->height> to) {
		
				if (update[i]->left[h-1])
				update[i]->left[h-1]->right[h-1] = update[i]->loadRight(h-1,
						dataFile_);
				if (update[i]->right[h-1])
				update[i]->right[h-1]->left[h-1] = update[i]->left[h-1];

				update[i]->height = update[i]->height - 1;
			}
			
			dirtyPages_.push_back(update[i]);
			if (update[i]->left[h-1])
			dirtyPages_.push_back(update[i]->left[h-1]);
		}
	}
	*/	
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
}

/**
 * @brief Construct the tree.
 *
 * inf is the largest DataType
 * and is used to signal failed finds.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> SkipListFile<KeyType, ValueType, LockType, Alloc>::SkipListFile(
		const string& fileName) :
	fileName_(fileName), minDegree_(2) {

	isUnload_ = false;
	sfh_.degree = minDegree_;
}

/**
 * @brief Destructor.
 */
template <typename KeyType, typename ValueType, typename LockType,
		typename Alloc> SkipListFile<KeyType, ValueType, LockType, Alloc>::~SkipListFile() {
	if(dataFile_)
		release();
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
		while (x->right[h] && comp_(v.get_key(), x->right[h]->KEY)> 0) {
			x = x->loadRight(h, dataFile_);			
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

	typename
	vector<SkipNode*>::iterator it = update.end();
	--it;
	for(int i=0; i<height; i++)
	{
		SkipNode* p = *it;
		it--;	
		t->right[i] = p->right[i];
		if( p->right[i] )p->right[i]->left[i] = t->right[i];
		
		p->right[i] = t;		
		dirtyPages_.push_back(p);		
	}
	dirtyPages_.push_back(t);
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
	x = header_;
	int h = x->height - 1;	

	while (h>=0 && x) {	
		while (x->right[h] && comp_(v.get_key(), x->right[h]->KEY)> 0) {	
			x = x->loadRight(h, dataFile_);			
		}
		
		if( x->right[h] && comp_( v.get_key(), x->right[h]->KEY) == 0 ) {			
			return false;
		}		
		SkipNode* temp = x;

		bool isSplit = false;
		if( h-1 >=0 ) {
			if( temp->right[h-1] ) temp = temp->loadRight(h-1,dataFile_);
			if( temp->right[h-1] ) temp = temp->loadRight(h-1, dataFile_);
			if( temp->right[h-1] ) {
				temp = temp->loadRight(h-1, dataFile_);
				isSplit = true;				
			}
		}

		if ( (h == 0 )
		|| ( isSplit
				&& (x->right[h] == temp->right[h-1]
						|| ( (x->right[h] && temp->right[h-1])
								&& (temp->right[h-1]->isLoaded)
								&& (x->right[h]->KEY> temp->right[h-1]->KEY) ) ) ) )		
		{
			if( h == 0 ) {				
				t = allocateNode_();
				t->element = v;				
			}
			else {
				t = x->right[h-1] ->right[h-1];				
			}
			
			t->height++;
			t->right[h] = x->right[h];
			
			t->isDirty = true;
			dirtyPages_.push_back(t);		

			x->right[h] = t;			
			x->isDirty = true;
			dirtyPages_.push_back(x);			
		}		
		--h;
		
	}

	if (header_->right[header_->height-1] != NULL) {	
		header_->height++;
		dirtyPages_.push_back(t);		
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
	
	vector<MemBlock*>::iterator it, it1;
	for (it = mbList.begin(); it != mbList.end(); it++) {
		it1 = it;
		delete *it1;
	}
}

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
			current = current->right[h];
		}
		cout<<" tail "<<endl;
		if (counted)
			b++;
		h--;
		a++;
	}	
}

NS_IZENELIB_AM_END

#endif

