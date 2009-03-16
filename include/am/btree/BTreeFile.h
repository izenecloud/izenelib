/**
 * @file BTREEFILE.h
 * @brief The header file of BTreeFile.
 *
 *
 * This file defines class BTreeFile.
 */
#ifndef BTREEFILE_H_
#define BTREEFILE_H_

#include "BTreeNode.h"
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>

#include <am/concept/DataType.h>
using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 * 	\brief file version of B tree.
 * 
 *   
 *     @FileHeader
 * 
 *     |----------------|
 *     |   magic        |  
 *     |----------------| 
 * 	   |   minDegree    | /then the order of the btree is 2*minDegree-1.
 *     |----------------|
 *     |   pageSize     |
 *	   |----------------| 
 *     |   cacheSize    |
 *     |----------------|
 *     |    .....       |
 *     |----------------|
 *     |   numItem      |   
 *     |----------------|      
 *     |   rootPos      |  
 *     |----------------|
 * 
 */

template<typename KeyType, typename ValueType=NullType, typename LockType=NullLock, typename Alloc=std::allocator<DataType<KeyType,ValueType> > >class BTreeFile
: public AccessMethod<KeyType, ValueType, LockType, Alloc> {
public:
	typedef DataType<KeyType,ValueType> DataType;
	typedef intrusive_ptr<BTreeNode<KeyType, DataType, LockType, Alloc> >
	BTreeNodePtr;
	typedef std::vector<BTreeNodePtr> BTreeNodeVECTOR;
	typedef typename std::vector<BTreeNodePtr>::iterator BnPtrIter;
	typedef std::pair<BTreeNodePtr, size_t> NodeKeyLocn;
	typedef intrusive_ptr<PtrObj<DataType, LockType, Alloc> > DataTypePtr;
	typedef std::vector<DataTypePtr> DataTypeVECTOR;

	typedef typename std::vector<BTreeNodePtr>::iterator BIT;
	typedef typename std::vector<DataTypePtr>::iterator DIT;
public:
	/**
	 * \brief constructor
	 * \param minDegree, default is 2, i.e the order is 3.
	 */
	BTreeFile(const std::string& fileName = "sequentialdb.dat");
	virtual ~BTreeFile();

	/**
	 *   set the degree
	 */
	void setDegree(int degree)
	{
		_minDegree = degree;
	}

	/**
	 * 	\brief set tha pageSize.
	 *  \param maxDataSize if the size of DataType(binary) exceeds maxDataSize, overflowing accur.
	 *  if we can predict it first, we can reduce the file space.
	 */
	void setPageSize(size_t maxDataSize) {
		_pageSize = sizeof(byte); //leafFlag
		_pageSize += sizeof(size_t); // object count

		_pageSize += (_minDegree * 2 - 1) * (sizeof(size_t) + maxDataSize
				+ sizeof(long) + sizeof(byte) ); // records + overflow address
		_pageSize += _minDegree * 2 * sizeof(long); // child locations		
		_maxDataSize = maxDataSize;

		BTreeNode<KeyType, DataType, LockType, Alloc>::setDataSize(maxDataSize,
				_pageSize, _overFlowSize);

		//_pageSize += BOOST_SERIAZLIZATION_HEAD_LENGTH;
	}

	/**
	 *  \brief set size for overflow page.
	 * 
	 */
	void setOverFlowPageSize(size_t overFlowSize) {
		_overFlowSize = overFlowSize;
	}

	/**
	 * 	We would peroidically flush the memory items, according to the cache Size. 
	 */

	void setCacheSize(size_t sz) {
		_cacheSize = 2*sz/_minDegree;
	}

	/**
	 * 	\brief return the file name of the SequentialDB
	 */
	std::string getFileName() const {
		return _fileName;
	}

	/**
	 * 	 \brief open the database. 
	 * 
	 *   Everytime  we use the database, we mush open it first.  
	 */
	bool open();

	/**
	 * 	 \brief close the database. 
	 * 
	 *    if we don't call it, it will be automately called in deconstructor 	 
	 */
	bool close() {
		//note that _root can be  NULL, if no items.
		flush();
		if (_root) {
			_root->unload();
			_root.reset(0);
		}

		if (_dataFile != 0) {
			fclose(_dataFile);
			_dataFile = 0;
		}
		return true;
	}

	/**
	 * 	 \brief del an item from the database
	 * 
	 */
	bool del(const KeyType& key);
	/**
	 * 	\brief insert an item.
	 */
	bool insert(const DataType& rec);

	/**
	 *  \brief insert an item.
	 */
	bool insert(const KeyType& key, const ValueType& value) {
		//DataType data(key, value);
		return insert( DataType(key, value) );
	}

	/**
	 *  \brief find an item given a key.
	 */
	ValueType* find(const KeyType& key) {
		DataType temp;
		ValueType* pv = NULL;
		if( get(key, temp) ) {
			pv = new ValueType( temp.get_value() );
		}
		return pv;
	}

	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const KeyType& key, const ValueType& val)
	{
		return update( DataType(key, val) );
	}
	/**
	 *  \brief updata an item with given key, if it not exist, insert it directly. 
	 */
	bool update(const DataType& rec);

	/**
	 * 	
	 * \brief get the number of the items.
	 */
	int num_items() {
		return _numItem;
	}

	/**
	 * 	\brief get an item by its key.
	 */
	bool get(const KeyType& key, DataType& rec);

	/**
	 *  \brief get an item from given Locn.	 * 
	 */
	bool get(const NodeKeyLocn& locn, DataType& rec);

	/**
	 *  \brief get an item from given Locn.	 * 
	 */
	bool get(const NodeKeyLocn& locn, KeyType& key, ValueType& value)
	{
		DataType dat;
		bool ret =get(locn, dat);
		key = dat.get_key();
		value = dat.get_value();
		return ret;
	}

	/**
	 *  \brief get the cursor of the first item.
	 * 
	 */

	NodeKeyLocn get_first_Locn()
	{
		NodeKeyLocn locn;
		search(KeyType(), locn);
		return locn;
	}
	/**
	 * 	\brief get the next or prev item.
	 */
	bool
	seq(NodeKeyLocn& locn, DataType& rec,
			ESeqDirection sdir = ESD_FORWARD);

	/**
	 *  \brief given a  key, get next key
	 */
	KeyType getNext(const KeyType& key) {
		//NodeKeyLocn locn =search(key);
		NodeKeyLocn locn;
		search(key, locn);
		if (locn.second == size_t(-1) ) {
			return KeyType();
		} else {
			DataType dat;
			seq(locn, dat, ESD_FORWARD);
			return dat.get_key();
		}
	}

	/**
	 *  \brief given a  key, get next key
	 */
	KeyType getPrev(const KeyType& key) {
		//NodeKeyLocn locn = search(key);
		NodeKeyLocn locn;
		search(key, locn);
		if (locn.second == size_t(-1) ) {
			return KeyType();
		} else {
			DataType dat;
			seq(locn, dat, ESD_BACKWARD);
			return dat.get_key();
		}
	}
	/**
	 * 	\brief write all the items in memory to file.
	 */

	void flush();
	/**
	 * 	\brief write back the dirypages
	 */
	void commit() {
		//write back the fileHead
		if(_root)
		sfh.rootPos = _root->fpos;
		sfh.numItem = _numItem;
		sfh.pageSize = _pageSize;

		if( !_dataFile )return;

		if ( 0 != fseek(_dataFile, 0, SEEK_SET)) {
			abort();
		}
		if (1 != fwrite(&sfh, sizeof(sfh), 1, _dataFile)) {
			abort();
		}

		_flush(_root, _dataFile);

		/*while( !_dirtyPages.empty() )
		 {
		 BTreeNodePtr ptr = _dirtyPages.back();
		 _dirtyPages.pop_back();
		 ptr->write(_dataFile);
		 }*/
		fflush(_dataFile);

	}

	/**
	 *   for debug.  print the shape of the B tree.
	 */

	void display(std::ostream& os = std::cout) {
		if(_root)_root->display(os);
	}

	/**
	 *  if the input key exists, just return itself, otherwise return 
	 * 	the smallest existing key that bigger than it.   
	 */
	KeyType getNearest(const KeyType& key) {

		BTreeNodePtr node = _root;
		OBJECTPOS op;
		NodeKeyLocn locn;
		DataType dat;

		while (true) {
			op = node->findPos1(key);

			switch (op.second) {
				case ECP_NONE:
				return KeyType();
				break;
				case ECP_INTHIS:
				return node->elements[op.first]->pdat->get_key();
				break;

				case ECP_INLEFT:
				node = node->loadChild(op.first, _dataFile);
				break;

				case ECP_INRIGHT:
				node = node->loadChild(op.first + 1, _dataFile);
				break;

				case ECP_NEARLEFT:
				return node->elements[op.first]->pdat->get_key();
				break;

				case ECP_NEARRIGHT:
				locn.first = node;
				locn.second = op.first;
				if (seq(locn, dat) ) {
					return dat.get_key();
				} else {
					return node->elements[op.first]->pdat->get_key();
				}
				break;

				default:
				break;
			}

		}
	}

	/**
	 * 
	 *  \brief External method that searches for elements that match the	 
	 *  given key. 
	 *
	 */
	NodeKeyLocn search(const KeyType& key)
	{
		NodeKeyLocn locn;
		search(key, locn);
		return locn;
	}

	bool search(const KeyType& key, NodeKeyLocn& locn) {

		//do Flush, when there are too many active nodes.
		_flushCache();

		locn.first = BTreeNodePtr();
		locn.second = (size_t)-1;

		BTreeNodePtr temp = _root;
		while (1) {
			int ctr = temp->objCount;
			int low = 0;
			int high = ctr-1;
			int compVal;
			while (low <= high) {
				int mid = (low+high)/2;
				//cout<<"mid "<<mid<<endl;
				compVal = comp(key,temp->elements[mid]->pdat->get_key() );
				if (compVal == 0) {
					locn.first = temp;
					locn.second = mid;
					return true;
				}
				else if (compVal < 0)
				high = mid-1;
				else {
					low = mid+1;
				}
			}

			if (!temp->isLeaf) {
				temp = temp->loadChild(low, _dataFile);
			} else {
				break;
			}
		}
		return false;
	}

private:
	/**
	 *  \brief it provides the basical information of the btree.
	 * 
	 */
	struct SFileHeader {
		int magic; //set it as 0x061561, check consistence.
		size_t minDegree;
		size_t maxDataSize;
		size_t pageSize;
		size_t overFlowSize;
		size_t cacheSize;
		size_t numItem;
		long rootPos; //streamoff of the root of btree.

		void display() {
			cout<<"fileHeader display...\n";
			cout<<"minDegree: "<<minDegree<<endl;
			cout<<"numItem: "<<numItem<<endl;
			cout<<"maxDataSize: "<<maxDataSize<<endl;
			cout<<"pageSize: "<<pageSize<<endl;
			cout<<"overFlowSize: "<<overFlowSize<<endl;
			cout<<"cacheSize: "<<cacheSize<<endl;
			cout<<"rootPos: "<<rootPos<<endl;
		}
	};

private:

	std::string _fileName; // name of the database file	
	size_t _minDegree;
	BTreeNodePtr _root;
	FILE* _dataFile;
	size_t _maxDataSize;
	size_t _pageSize; //pagesize
	size_t _overFlowSize;
	size_t _cacheSize;
	size_t _numItem;
	//long _recOff;


	SFileHeader sfh;

	bool _isUnload;

private:
	//BTreeNodeVECTOR _dirtyPages;
	CompareFunctor<KeyType> comp;

private:

	void _flushCache() {

		//cout<<bucket_chain::activeNum<<" vs "<<sfh_.cacheSize <<endl;
		if( BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum> sfh.cacheSize )
		{
			//commit();
			//_dirtyPages.clear();
#ifdef  DEBUG
			cout<<"cache is full..."<<endl;
			cout<<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum<<" vs "<<sfh.cacheSize <<endl;
#endif

			queue<BTreeNodePtr> qnode;
			qnode.push(_root);
			//_root->unload();

			size_t popNum = 0;
			while ( !qnode.empty() ) {
				BTreeNodePtr popNode = qnode.front();
				qnode.pop();
				popNum++;
				if(popNum >= sfh.cacheSize/_minDegree/2)
				{
					if(popNode->_isDirty)
					_flush(popNode, _dataFile);
					popNode->unload();

					//cout<<"unload....";
					//cout<<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum<<" vs "<<sfh.cacheSize <<endl;

					if(BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum < sfh.cacheSize/2)
					{
						break;
					}
				}
				if (!popNode->isLeaf) {
					for (BIT tnvit = popNode->children.begin(); tnvit
							!= popNode->children.end(); tnvit++) {
						if (*tnvit)
						qnode.push(*tnvit);
					}

				}
				popNode.reset(0);
			}
			//display();
#ifdef DEBUG
			cout<<"stop unload..."<<endl;
			cout<<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum<<" vs "<<sfh.cacheSize <<endl;
#endif		
		}
		fflush(_dataFile);

	}

	void _flushCache1() {
		commit();
		//cout<<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum <<" > "<<_cacheSize<<endl;
		if (BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum> _cacheSize) {
#ifdef DEBUG
			// display();	
			cout<<"flush ... "<<_cacheSize << endl;
			cout<<"activeNode: " <<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum
			<<endl;
#endif
			_isUnload = true;

			if ( _root && !_root->isLeaf) {
				for (size_t ctr = 0; ctr <= _root->objCount; ctr++) {
					// keep engough node in memory to impove the efficency.
					if (BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum
							< _cacheSize/2 ) {
#ifdef DEBUG
						cout<<"AcitveNodeNum= "
						<<BTreeNode<KeyType, DataType, LockType, Alloc>::activeNodeNum
						<<"\n stop unload\n";
#endif

						break;
					}

					BTreeNodePtr pChild = _root->children[ctr];
					if ((BTreeNodePtr)pChild != 0) {
						_root->children[ctr]->unload();
					}
					_isUnload = false;

				}
			}
			//display();
		}

	}

	// internal data manipulation functions (see Cormen, Leiserson, Rivest).
	//	static inline int _defaultCompare(const DbObjPtr& obj1, const DbObjPtr& obj2);
	//	static int _searchCompare(const DbObjPtr& obj1, const DbObjPtr& obj2);

	BTreeNodePtr _allocateNode() {

		BTreeNodePtr newNode;
		newNode.reset(new BTreeNode<KeyType, DataType, LockType, Alloc>);

		fseek(_dataFile, 0L, SEEK_END);
		long len = ftell(_dataFile);
		newNode->fpos = len;

		newNode->loaded = true;
		newNode->childNo = size_t(-1);

		char *pBuf = new char[_pageSize];
		memset(pBuf, 0, _pageSize);

		fwrite(pBuf, sizeof(char), _pageSize, _dataFile);

		delete [] pBuf;

		return newNode;

		/*	static int num;
		 BTreeNodePtr newNode;
		 newNode.reset(new BTreeNode<KeyType, DataType>);

		 newNode->fpos = _recOff + _pageSize*num++;

		 newNode->loaded = true;

		 newNode->childNo = size_t(-1);

		 return newNode;		*/
	}

	void _split(BTreeNodePtr& parent, size_t childNum, BTreeNodePtr& child);
	BTreeNodePtr _merge(BTreeNodePtr& parent, size_t objNo) {

		size_t ctr = 0;

		BTreeNodePtr c1 = parent->loadChild(objNo, _dataFile);
		BTreeNodePtr c2 = parent->loadChild(objNo+1, _dataFile);

		c1->elements.resize(2 * _minDegree - 1);
		for (ctr = 0; ctr < _minDegree - 1; ctr++) {
			c1->elements[_minDegree + ctr] = c2->elements[ctr];
		}
		if (!c2->isLeaf) {
			c1->children.resize(2 * _minDegree);
			for (ctr = 0; ctr < _minDegree; ctr++) {
				size_t newPos = _minDegree + ctr;

				c2->loadChild(ctr, _dataFile);
				c1->children[newPos] = c2->children[ctr];
				c1->children[newPos]->childNo = newPos;
				c1->children[newPos] ->parent = c1;//wps add it!				
			}
		}

		// Put the parent into the middle
		c1->elements[_minDegree - 1] = parent->elements[objNo];
		c1->objCount = 2 * _minDegree - 1;

		// Reshuffle the parent (it has one less object/child)
		for (ctr = objNo + 1; ctr < parent->objCount; ctr++) {
			parent->elements[ctr - 1] = parent->elements[ctr];
			parent->loadChild(ctr+1, _dataFile);
			parent->children[ctr] = parent->children[ctr + 1];
			parent->children[ctr]->childNo = ctr;
		}
		--parent->objCount;
		parent->elements.resize(parent->objCount);
		parent->children.resize(parent->objCount + 1);

		if (parent->objCount == 0)//wps add it! 
		{
			parent = c1;
		}

		// Write the two affected nodes to the disk. Note that
		// c2 just goes away. The node will be deallocated because
		// of the smart pointers, and the node's location on
		// disk will become inaccessible. This will have to be
		// fixed by the judicious use of the compact() method.

		//c1->write(_dataFile);
		//parent->write(_dataFile);

		c1->setDirty(1);
		parent->setDirty(_dataFile);

		//_dirtyPages.push_back(c1);
		//_dirtyPages.push_back(parent);

		// Return a pointer to the new child.
		return c1;
	}

	bool _insert(const DataType& data);
	bool _seqNext(NodeKeyLocn& locn, DataType& rec);
	bool _seqPrev(NodeKeyLocn& locn, DataType& rec);
	void _flush(BTreeNodePtr& node, FILE* f);
	bool _delete(BTreeNodePtr& node, const KeyType& key);

	// Finds the location of the predecessor of this key, given
	// the root of the subtree to search. The predecessor is going
	// to be the right-most object in the right-most leaf node.
	NodeKeyLocn _findPred(BTreeNodePtr& node) {
		NodeKeyLocn ret(BTreeNodePtr(), (size_t)-1);
		BTreeNodePtr child = node;
		while (!child->isLeaf) {
			child = child->loadChild(child->objCount, _dataFile);
		}
		ret.first = child;
		ret.second = child->objCount - 1;
		return ret;
	}

	// Finds the location of the successor of this key, given
	// the root of the subtree to search. The successor is the
	// left-most object in the left-most leaf node.
	NodeKeyLocn _findSucc(BTreeNodePtr& node) {
		NodeKeyLocn ret(BTreeNodePtr(), (size_t)-1);
		BTreeNodePtr child = node;
		while (!child->isLeaf) {
			child = child->loadChild(0, _dataFile);
		}
		ret.first = child;
		ret.second = 0;
		return ret;
	}

};

// The constructor simply sets up the different data members, and if
// the caller doesn't provide a compare function of their own, specifies
// the default comparison function.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> BTreeFile< KeyType, ValueType, LockType, Alloc>::BTreeFile(
		const std::string& fileName) :
	_fileName(fileName), _dataFile(0), _numItem(0) {
	_minDegree = 16;

	_maxDataSize = 64;
	setPageSize(_maxDataSize);

	_overFlowSize = 1024;
	_cacheSize = 1000000; //default set 1000000	

	_isUnload = false;
	_root = 0;
}

// The destructor of a BTreeFile object unloads the root
// node and then assigns null to the _root smart pointer.
// This deletes the item that was already there.
// Unloading the root node ensures that there are no
// children floating around referring to the root node, so
// that when we come to assign null to the smart pointer,
// the number of references will drop to zero, and the
// actual node will be deleted.
// We also have to close the file.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> BTreeFile< KeyType, ValueType, LockType, Alloc>::~BTreeFile() {
	close();
}

// Splits a child node, creating a new node. The median value from the
// full child is moved into the *non-full* parent. The keys above the
// median are moved from the full child to the new child.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> void BTreeFile< KeyType, ValueType, LockType, Alloc>::_split(
		BTreeNodePtr& parent, size_t childNum, BTreeNodePtr& child) {

	//cout<<"splitting"<<endl;
	size_t ctr = 0;
	DataTypePtr saveObj;
	BTreeNodePtr newChild = _allocateNode();
	newChild->isLeaf = child->isLeaf;
	newChild->setCount(_minDegree - 1);

	// Put the high values in the new child, then shrink the existing child.
	for (ctr = 0; ctr < _minDegree - 1; ctr++) {
		newChild->elements[ctr] = child->elements[_minDegree + ctr];
	}
	if (!child->isLeaf) {
		for (ctr = 0; ctr < _minDegree; ctr++) {
			BTreeNodePtr mover = child->children[_minDegree + ctr];
			newChild->children[ctr] = mover;
			mover->childNo = ctr;
			mover->parent = newChild;
		}
	}
	saveObj = child->elements[_minDegree - 1];
	child->setCount(_minDegree - 1);

	// Move the child pointers above childNum up in the parent
	parent->setCount(parent->objCount + 1);
	for (ctr = parent->objCount; ctr> childNum + 1; ctr--) {
		parent->children[ctr] = parent->children[ctr - 1];
		parent->children[ctr]->childNo = ctr;
	}
	parent->children[childNum + 1] = newChild;
	newChild->childNo = childNum + 1;
	newChild->parent = parent;
	for (ctr = parent->objCount - 1; ctr> childNum; ctr--) {
		parent->elements[ctr] = parent->elements[ctr - 1];
	}
	parent->elements[childNum] = saveObj;

	child->setDirty(1);
	newChild->setDirty(1);
	parent->setDirty(1);

	//_dirtyPages.push_back(child);
	//_dirtyPages.push_back(newChild);
	//_dirtyPages.push_back(parent);
	//child->write(_dataFile);	
	//newChild->write(_dataFile);	
	//parent->write(_dataFile);
}

template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::_insert(
		const DataType& dat) {

	KeyType key = dat.get_key();

	if (_root->objCount == _minDegree *2 - 1) {
		// Growing the tree happens by creating a new
		// node as the new root, and splitting the
		// old root into a pair of children.
		BTreeNodePtr oldRoot = _root;

		_root = _allocateNode();
		_root->setCount(0);
		_root->isLeaf = false;
		_root->children[0] = oldRoot;
		oldRoot->childNo = 0;
		oldRoot->parent = _root;
		_split(_root, 0, oldRoot);
		goto L0;

	} else {
		L0: BTreeNodePtr node = _root;
		L1: size_t ctr = node->objCount;

		/*if (ctr == 0) {
		 node->setCount(1);
		 node->elements[0].reset(new PtrObj<DataType>(dat));
		 node->setDirty(1);
		 return true;
		 }*/

		// If the node is a leaf, we just find the location to insert
		// the new item, and shuffle everything else up.
		if (node->isLeaf) {
			int low = 0;
			int high = ctr-1;
			int compVal;
			while (low<=high) {
				int mid = (low+high)/2;//cout<<"mid "<<mid<<endl;
				compVal = comp(key, node->elements[mid]->pdat->get_key());
				if (compVal == 0)
					return false;
				else if (compVal < 0)
					high = mid-1;
				else {
					low = mid+1;
				}
			}

			node->setCount(node->objCount + 1);
			for (; (int)ctr> low; ctr--) {
				node->elements[ctr] = node->elements[ctr - 1];
			}

			node->elements[low].reset(new PtrObj<DataType, LockType, Alloc>(dat));
			node->setDirty(1);
			//_dirtyPages.push_back(node);
			return true;
		}

		// If the node is an internal node, we need to find
		// the location to insert the value ...
		else {
			int low = 0;
			int high = ctr-1;
			int compVal;
			while (low<=high) {
				int mid = (low+high)/2;
				compVal = comp(key, node->elements[mid]->pdat->get_key());
				if (compVal == 0)
					return false;
				else if (compVal < 0)
					high = mid-1;
				else {
					low = mid+1;
				}
			}

			ctr = low;
			// Load the child into which the value will be inserted.
			BTreeNodePtr child = node->loadChild(ctr, _dataFile);

			// If the child node is full (2t - 1 elements), then we need
			// to split the node.
			if (child->objCount == _minDegree *2 - 1) {
				_split(node, ctr, child);
				int compVal =comp(key, node->elements[ctr]->pdat->get_key() );
				//comp(key, node->elements[ctr]->pdat->get_key() );
				if (compVal == 0)
					return false;
				if (compVal> 0) {
					++ctr;
				}
				child = node->children[ctr];
			}

			// Insert the key (recursively) into the non-full child
			// node.
			node = child;
			goto L1;
			//_insertNonFull(child, dat);
		}
	}
}

// Write all nodes in the tree to the file given.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> void BTreeFile< KeyType, ValueType, LockType, Alloc>::_flush(
		BTreeNodePtr& node, FILE* f) {

	// Bug out if the file is not valid
	if (!f) {
		return;
	}
	queue<BTreeNodePtr> qnode;
	qnode.push(node);
	while (!qnode.empty()) {
		BTreeNodePtr popNode = qnode.front();
		if (popNode->loaded) {
			popNode->write(f);
		}
		qnode.pop();
		if (!popNode->isLeaf) {
			for (BIT tnvit = popNode->children.begin(); tnvit
					!= popNode->children.end(); tnvit++) {
				if (*tnvit)
					qnode.push(*tnvit);
			}

		}
		popNode.reset(0);
	}

}

// Internal delete function, used once we've identified the
// location of the node from which a key is to be deleted.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::_delete(
		BTreeNodePtr& nd, const KeyType& k) {
	bool ret = false;

	// Find the object position. op will have the position
	// of the object in op.first, and a flag (op.second)
	// saying whether the object at op.first is an exact
	// match (true) or if the object is in a child of the
	// current node (false). If op.first is -1, the object
	// is neither in this node, or a child node.	

	BTreeNodePtr node = nd;
	KeyType key = k;

	L0: OBJECTPOS op = node->findPos(key);

	if (op.first != (size_t)-1) // it's in there somewhere ...
	{

		if (op.second == ECP_INTHIS) // we've got an exact match
		{
			// Case 1: deletion from leaf node.
			if (node->isLeaf) {
				node->delFromLeaf(op.first);

				//now node is dirty
				node->setDirty(1);
				//_dirtyPages.push_back(node);
				ret = true;
			}
			// Case 2: Exact match on internal leaf.
			else {
				// Case 2a: prior child has enough elements to pull one out.
				node->loadChild(op.first, _dataFile);
				node->loadChild(op.first+1, _dataFile);
				if (node->children[op.first]->objCount >= _minDegree) {
					BTreeNodePtr childNode = node->loadChild(op.first,
							_dataFile);
					NodeKeyLocn locn = _findPred(childNode);

					DataType dat = *(locn.first->elements[locn.second]->pdat);
					DataTypePtr pdat;
					pdat.reset(new PtrObj<DataType, LockType, Alloc>(dat));
					node->elements[op.first] = pdat;

					//now node is dirty
					node->setDirty(1);
					//_dirtyPages.push_back(node);

					node = childNode;
					key = dat.get_key();
					goto L0;
					//ret = _delete(childNode, dat.get_key());
				}

				// Case 2b: successor child has enough elements to pull one out.
				else if (node->children[op.first + 1]->objCount >= _minDegree) {
					BTreeNodePtr childNode = node->loadChild(op.first + 1,
							_dataFile);
					NodeKeyLocn locn = _findSucc(childNode);
					DataType dat = *(locn.first->elements[locn.second]->pdat);
					DataTypePtr pdat;
					pdat.reset(new PtrObj<DataType, LockType, Alloc>(dat));
					node->elements[op.first] = pdat;

					//now node is dirty
					node->setDirty(1);
					//_dirtyPages.push_back(node);

					node = childNode;
					key = dat.get_key();
					goto L0;
					//ret = _delete(childNode, dat.get_key());
				}

				// Case 2c: both children have only t-1 elements.
				// Merge the two children, putting the key into the
				// new child. Then delete from the new child.
				else {
					BTreeNodePtr mergedChild = _merge(node, op.first);
					node = mergedChild;
					goto L0;
					//ret = _delete(mergedChild, key);
				}
			}
		}

		// Case 3: key is not in the internal node being examined,
		// but is in one of the children.
		else if (op.second == ECP_INLEFT || op.second == ECP_INRIGHT) {
			// Find out if the child tree containing the key
			// has enough elements. If so, we just recurse into
			// that child.

			node->loadChild(op.first, _dataFile);
			if (op.first+1<=node->objCount)
				node->loadChild(op.first+1, _dataFile);
			size_t keyChildPos = (op.second == ECP_INLEFT) ? op.first
					: op.first + 1;
			BTreeNodePtr childNode = node->loadChild(keyChildPos, _dataFile);
			if (childNode->objCount >= _minDegree) {
				node = childNode;
				goto L0;
				//ret = _delete(childNode, key);
			} else {
				// Find out if the childNode has an immediate
				// sibling with _minDegree keys.
				BTreeNodePtr leftSib;
				BTreeNodePtr rightSib;
				size_t leftCount = 0;
				size_t rightCount = 0;
				if (keyChildPos> 0) {
					leftSib = node->loadChild(keyChildPos - 1, _dataFile);
					leftCount = leftSib->objCount;
				}
				if (keyChildPos < node->objCount) {
					rightSib = node->loadChild(keyChildPos + 1, _dataFile);
					rightCount = rightSib->objCount;
				}

				// Case 3a: There is a sibling with _minDegree or more keys.
				if (leftCount >= _minDegree || rightCount >= _minDegree) {
					// Part of this process is making sure that the
					// child node has minDegree elements.
					childNode->setCount(_minDegree);

					// Bringing the new key from the left sibling
					if (leftCount >= _minDegree) {
						// Shuffle the keys and elements up
						size_t ctr = _minDegree - 1;
						for (; ctr> 0; ctr--) {
							childNode->elements[ctr]
									= childNode->elements[ctr - 1];
							childNode->children[ctr + 1]
									= childNode->children[ctr];
							if (childNode->children[ctr + 1]) {
								childNode->children[ctr + 1]->childNo = ctr + 1;
							}
						}
						childNode->children[ctr + 1] = childNode->children[ctr];
						if (childNode->children[ctr + 1]) {
							childNode->children[ctr + 1]->childNo = ctr +1;
						}//wps add it!

						// Put the key from the parent into the empty space,
						// pull the replacement key from the sibling, and
						// move the appropriate child from the sibling to
						// the target child.
						childNode->elements[0]
								= node->elements[keyChildPos - 1];
						node->elements[keyChildPos - 1]
								= leftSib->elements[leftSib->objCount - 1];
						leftSib->elements.resize(leftSib->objCount - 1);
						if (!leftSib->isLeaf) {
							childNode->children[0]
									= leftSib->children[leftSib->objCount];
							leftSib->children.resize(leftSib->objCount);

							childNode->children[0]->childNo = 0;//wps add it
							childNode->children[0]->parent = childNode;//wps add it
						}
						--leftSib->objCount;

						//now node is dirty
						leftSib->setDirty(1);
						node->setDirty(1);
						//_dirtyPages.push_back(leftSib);
						//_dirtyPages.push_back(node);
					}

					// Bringing a new key in from the right sibling
					else {
						// Put the key from the parent into the child,
						// put the key from the sibling into the parent,
						// and move the appropriate child from the
						// sibling to the target child node.
						childNode->elements[childNode->objCount - 1]
								= node->elements[op.first];
						node->elements[op.first] = rightSib->elements[0];
						if (!rightSib->isLeaf) {
							childNode->children[childNode->objCount]
									= rightSib->children[0];

							childNode->children[childNode->objCount]->childNo
									= childNode->objCount;//wps add it!
							childNode->children[childNode->objCount]->parent
									= childNode;//wps add it!
						}

						// Now clean up the right node, shuffling keys
						// and elements to the left and resizing.
						size_t ctr = 0;
						for (; ctr < rightSib->objCount - 1; ctr++) {
							rightSib->elements[ctr]
									= rightSib->elements[ctr + 1];
							if (!rightSib->isLeaf) {
								rightSib->children[ctr]
										= rightSib->children[ctr + 1];

								rightSib->children[ctr]->childNo = ctr;
							}
						}
						if (!rightSib->isLeaf) {
							rightSib->children[ctr]
									= rightSib->children[ctr + 1];
							rightSib->children[ctr]->childNo = ctr;
						}
						rightSib->setCount(rightSib->objCount - 1);

						//now node is dirty
						rightSib->setDirty(true);
						node->setDirty(1);
						//_dirtyPages.push_back(rightSib);
						//_dirtyPages.push_back(node);
					}
					node = childNode;

					node->setDirty(true);
					//_dirtyPages.push_back(node);
					goto L0;
					//ret = _delete(childNode, key);
				}

				// Case 3b: All siblings have _minDegree - 1 keys
				else {
					BTreeNodePtr mergedChild = _merge(node, op.first);
					node = mergedChild;
					goto L0;
					//ret = _delete(mergedChild, key);
				}
			}
		}
	}
	return ret;
}

// Opening the database means that we check the file
// and see if it exists. If it doesn't exist, start a database
// from scratch. If it does exist, load the root node into
// memory.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::open() {
	// We're creating if the file doesn't exist.

	struct stat statbuf;
	bool creating = stat(_fileName.c_str(), &statbuf);

	_dataFile = fopen(_fileName.c_str(), creating ? "w+b" : "r+b");
	if (0 == _dataFile) {
		cout<<"SDB Error: open file failed, check if dat directory exists"
				<<endl;
		return false;
	}

	// Create a new node
	bool ret = false;
	if (creating) {

		if (_minDegree == size_t(-1) ) {
			return false;
		}

		sfh.magic = 0x061561;
		sfh.numItem = _numItem;
		sfh.maxDataSize = _maxDataSize;
		sfh.pageSize = _pageSize;
		sfh.overFlowSize = _overFlowSize;
		sfh.minDegree = _minDegree;
		sfh.cacheSize = _cacheSize;

		sfh.rootPos = sizeof(sfh);

#ifdef DEBUG
		cout<<"creating...\n"<<endl;
		sfh.display();
#endif

		// when creating, write the node to the disk.
		// remember that the first four bytes contain
		// the address of the root node.
		if (1 != fwrite(&sfh, sizeof(sfh), 1, _dataFile)) {
			return false;
		}

		// If creating, allocate a node instead of
		// reading one.
		_root = _allocateNode();
		_root->isLeaf = true;
		_root->loaded = true;

		_root->write(_dataFile);
		ret = true;

		BTreeNode<KeyType, DataType, LockType, Alloc>::setDataSize(
				_maxDataSize, _pageSize, _overFlowSize);

	} else {

		// when not creating, read the root node from the disk.
		memset(&sfh, 0, sizeof(sfh));
		if (1 != fread(&sfh, sizeof(sfh), 1, _dataFile)) {
			return false;
		} else {

			if (sfh.magic != 0x061561) {
				cout<<"Error, read wrong file header\n"<<endl;
				return false;
			}

			_minDegree = sfh.minDegree;
			_maxDataSize = sfh.maxDataSize;
			_pageSize = sfh.pageSize;
			_overFlowSize = sfh.overFlowSize;
			_cacheSize = sfh.cacheSize;
			_numItem = sfh.numItem;

#ifdef DEBUG
			cout<<"open exist...\n"<<endl;
			sfh.display();
#endif

			BTreeNode<KeyType, DataType, LockType, Alloc>::setDataSize(
					_maxDataSize, _pageSize, _overFlowSize);

			_root.reset(new BTreeNode<KeyType, DataType, LockType, Alloc>);
			_root->fpos = sfh.rootPos;
			_root->read(_dataFile);
			//_root->display();
			ret = true;

		}
		// If note creating, just create and read
		// rather than allocating.
	}

	return ret;
}

// This is the external delete function.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::del(
		const KeyType& key) {
	// Determine if the root node is empty.
	bool ret = (_root->objCount != 0);

	if (_root->objCount == (size_t) -1) {
		return 0;
	}

	// If our root is not empty, call the internal
	// delete method on it.
	ret = ret && _delete(_root, key);

	// If we successfully deleted the key, and there
	// is nothing left in the root node and the root
	// node is not a leaf, we need to shrink the tree
	// by making the root's child (there should only
	// be one) the new root. Write the location of
	// the new root to the start of the file so we
	// know where to look.
	if (_root->objCount == 0 && !_root->isLeaf) {
		_root = _root->children[0];
		fseek(_dataFile, 0, SEEK_SET);
		fwrite(&_root->fpos, sizeof(_root->fpos), 1, _dataFile);
	}
	if (ret)
		_numItem--;
	return ret;
}

// External put method. This will overwrite a key
// (allowing no duplicates) or insert a new item.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::insert(
		const DataType& rec) {
	_flushCache();
	if (_insert(rec) ) {
		_numItem++;
		return true;
	} else {
		return false;
	}
}

// This method retrieves a record from the database
// given its location.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::get(
		const NodeKeyLocn& locn, DataType& rec) {
	if ((BTreeNodePtr)locn.first == 0 || locn.second == (size_t)-1) {
		return false;
	}
	rec = *(locn.first->elements[locn.second]->pdat);
	return true;
}

// This method retrieves a record from the database
// given its key.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::get(
		const KeyType& key, DataType& rec) {
	NodeKeyLocn locn;
	search(key, locn);
	return get(locn, rec);
}

template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::update(
		const DataType& rec) {
	NodeKeyLocn locn(BTreeNodePtr(), (size_t)-1);
	//locn = search( rec.get_key() );
	search(rec.get_key(), locn);
	if (locn.second != (size_t) -1) {
		locn.first->elements[locn.second].reset(new PtrObj<DataType, LockType, Alloc>(rec));
		return true;
	} else {
		return insert(rec);
	}

}

// This method finds the record following the one at the
// location given as locn, and copies the record into rec.
// The direction can be either forward or backward.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::seq(
		NodeKeyLocn& locn, DataType& rec, ESeqDirection sdir) {
	switch (sdir) {
	case ESD_FORWARD:
		return _seqNext(locn, rec);

	case ESD_BACKWARD:
		return _seqPrev(locn, rec);
	}

	return false;
}

// Find the next item in the database given a location. Return
// the subsequent item in rec.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::_seqNext(
		NodeKeyLocn& locn, DataType& rec) {
	// Set up a couple of convenience values
	bool ret = false;
	BTreeNodePtr node = locn.first;
	size_t lastPos = locn.second;
	bool goUp = false; // indicates whether or not we've exhausted a node.

	// If we are starting at the beginning, initialise
	// the locn reference and return with the value set.
	// This means we have to plunge into the depths of the
	// tree to find the first leaf node.
	if ((BTreeNodePtr)node == 0) {
		node = _root;
		while ((BTreeNodePtr)node != 0 && !node->isLeaf) {
			node = node->loadChild(0, _dataFile);
			////_cacheInsert(node);
		}
		if ((BTreeNodePtr)node == 0) {
			return false;
		}
		rec = *(node->elements[0]->pdat);
		locn.first = node;
		locn.second = 0;
		return true;
	}

	// Advance the locn object to the next item

	// If we have a leaf node, we don't need to worry about
	// traversing into children ... only need to worry about
	// going back up the tree.
	if (node->isLeaf) {
		// didn't visit the last node last time.
		if (lastPos < node->objCount - 1) {
			rec = *(node->elements[lastPos + 1]->pdat);
			locn.second = lastPos + 1;
			return true;
		}
		goUp = (lastPos == node->objCount - 1);
	}

	// Not a leaf, therefore need to worry about traversing
	// into child nodes.
	else {
		node = node->loadChild(lastPos + 1, _dataFile);//_cacheInsert(node);
		while ((BTreeNodePtr)node != 0 && !node->isLeaf) {
			node = node->loadChild(0, _dataFile);
		}
		if ((BTreeNodePtr)node == 0) {
			return false;
		}
		rec = *(node->elements[0]->pdat);
		locn.first = node;
		locn.second = 0;
		return true;
	}

	// Finished off a leaf, therefore need to go up to
	// a parent.
	if (goUp) {
		size_t childNo = node->childNo;
		node = node->parent;
		while ((BTreeNodePtr)node != 0 && childNo >= node->objCount) {
			childNo = node->childNo;
			node = node->parent;
		}
		if ((BTreeNodePtr)node != 0) {
			locn.first = node;
			locn.second = childNo;
			ret = true;
			rec = *(node->elements[childNo]->pdat);
		}
	}
	return ret;
}

// Find the previous item in the database given a location. Return
// the item in rec.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> bool BTreeFile< KeyType, ValueType, LockType, Alloc>::_seqPrev(
		NodeKeyLocn& locn, DataType& rec) {
	// Set up a couple of convenience values

	bool ret = false;
	BTreeNodePtr node = locn.first;
	size_t lastPos = locn.second;
	bool goUp = false; // indicates whether or not we've exhausted a node.


	// If we are starting at the end, initialise
	// the locn reference and return with the value set.
	// This means we have to plunge into the depths of the
	// tree to find the first leaf node.

	if ((BTreeNodePtr)node == 0) {
		node = _root;
		while ((BTreeNodePtr)node != 0 && !node->isLeaf) {
			node = node->loadChild(node->objCount, _dataFile);
		}
		if ((BTreeNodePtr)node == 0) {

			return false;
		}
		locn.first = node;
		locn.second = node->objCount - 1;

		if (locn.second == size_t(-1) )
			return false;
		rec = *(node->elements[locn.second]->pdat);
		return true;
	}

	// Advance the locn object to the next item

	// If we have a leaf node, we don't need to worry about
	// traversing into children ... only need to worry about
	// going back up the tree.
	if (node->isLeaf) {
		// didn't visit the last node last time.
		if (lastPos> 0) {
			locn.second = lastPos - 1;
			rec = *(node->elements[locn.second]->pdat);
			return true;
		}
		goUp = (lastPos == 0);
	}

	// Not a leaf, therefore need to worry about traversing
	// into child nodes.
	else {
		node = node->loadChild(lastPos, _dataFile);
		while ((BTreeNodePtr)node != 0 && !node->isLeaf) {
			node = node->loadChild(node->objCount, _dataFile);
		}
		if ((BTreeNodePtr)node == 0) {

			return false;
		}
		locn.first = node;
		locn.second = node->objCount - 1;
		rec = *(node->elements[locn.second]->pdat);
		return true;
	}

	// Finished off a leaf, therefore need to go up to
	// a parent.
	if (goUp) {
		size_t childNo = node->childNo;
		node = node->parent;

		while ((BTreeNodePtr)node != 0 && childNo == 0) {
			childNo = node->childNo;
			node = node->parent;
		}
		if ((BTreeNodePtr)node != 0) {
			locn.first = node;
			locn.second = childNo - 1;
			rec = *(node->elements[locn.second]->pdat);
			ret = true;
		}
	}
	return ret;
}

// This method flushes all loaded nodes to the file and
// then unloads the root node' children. So not only do we commit
// everything to file, we also free up most memory previously
// allocated.
template<typename KeyType, typename ValueType, typename LockType,
		typename Alloc> void BTreeFile< KeyType, ValueType, LockType, Alloc>::flush() {

	//write back the fileHead and dirtypage
	commit();
	// Unload each of the root's childrent. 
	if (_root && !_root->isLeaf) {
		for (size_t ctr = 0; ctr <= _root->objCount; ctr++) {

			BTreeNodePtr pChild = _root->children[ctr];
			if ((BTreeNodePtr)pChild != 0) {
				_root->children[ctr]->unload();
			}
			_isUnload = false;

		}
	}
}

NS_IZENELIB_AM_END
#endif /*BTREEFILE_H_*/
