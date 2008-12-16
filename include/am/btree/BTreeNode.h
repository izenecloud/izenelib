/**
 * @file BTREENODE.h
 * @brief The header file of BTreeNODE.
 *
 * This file defines class BTreeNODE.
 */

#ifndef BTREENODE_H_
#define BTREENODE_H_

#include "sdb_types.h"
#include "SDBException.h"

#include <vector>
using namespace std;
using namespace boost;

NS_IZENELIB_AM_BEGIN

// the key position in the BTreeNode
enum EChildPos
{
	ECP_INTHIS,
	ECP_INLEFT,
	ECP_INRIGHT,
	ECP_NONE,

	ECP_NEARLEFT, //wps add it.
	ECP_NEARRIGHT
};

typedef std::pair<size_t, EChildPos> OBJECTPOS;

template <typename DataType, typename LockType, typename Alloc =std::allocator<DataType> > class PtrObj :
	public RefCount<LockType> {
public:
	PtrObj() {
		pdat = NULL;
	}
	PtrObj(const DataType &other) {
		pdat = new DataType(other);
	}

	void display() {
		//pdat->display();
		//cout<<*pdat;
	}

	virtual ~PtrObj() {
		delete pdat;
	}

	/*	 
	 *   Assignment operator. Make a copy of the other object.
	 */
	PtrObj& operator=(PtrObj& other) {
		pdat = new DataType(*other.pdat);
		return *this;
	}
	bool operator <=(PtrObj& other) {
		return *pdat.compare(*other.pdat);

	}
public:
	DataType* pdat;
};

/**
 * 
 * \brief  BTreeNode represents a node(internal node or leaf node)
 * in the B tree structure.
 *  
 * 
 * A BTreeDB is made up of a collection of n
 odes. A node
 * contains information about which of its parent's children
 * is, how many objects it has, whether or not it's a leaf,
 * where it lives on the disk, whether or not it's actually
 * been loaded from the disk, a collection of records,
 * and (if it's not a leaf) a collection of children.
 * It also contains a ptr to its parent.
 * 
 * A node page in file is as follows:
 * 
 * Page format :
 * @code
 *  +------------------------------+
 *  | leafFlag     | objCount      | 
 *  +--------+---------------------+
 *  |childAddress|--->|ChildAddress| 
 *  +--------+-----+----+----------+  
 *  | DbObj |   -->   | DbObj      |  
 *  +------------+--------+--------+
 * 
 *  where DbObj wraps the content of  pair [key | data], 
 * 
 * 
 *  @DbObj
 *  +-------------+
 *  | size | data |  
 *  +-------------+
 *   
 */

template<typename KeyType, typename DataType, typename LockType, typename Alloc> class BTreeNode :
	public RefCount<LockType> {

	typedef intrusive_ptr<BTreeNode<KeyType, DataType, LockType, Alloc> >
			BTreeNodePtr;
	typedef std::vector<BTreeNodePtr> BTreeNodeVECTOR;
	typedef std::pair<BTreeNodePtr, size_t> NodeKeyLocn;
	typedef intrusive_ptr<PtrObj<DataType, LockType, Alloc> > DataTypePtr;
	typedef std::vector<DataTypePtr> DataTypeVECTOR;

	typedef typename std::vector<BTreeNodePtr>::iterator BIT;
	typedef typename std::vector<DataTypePtr>::iterator DIT;

public:
	BTreeNode();
	~BTreeNode() {
		BTreeNode::activeNodeNum--;
		//cout<<"\n~ "<<activeNodeNum<<endl;
		//display();
	}

	/**
	 * 	\brief when we want to access to node, we should load it to memory from disk. 
	 */
	inline BTreeNodePtr loadChild(size_t childNum, FILE* f,
			const string& fileName);

	/**
	 * Unload a child, which means that we get rid of all
	 * children in the children vector.
	 */
	void unload();
	/**
	 * 	\brief read the node from disk.
	 */
	bool read(FILE* f);
	/**
	 * 	\brief write the node to  disk.
	 */
	bool write(FILE* f);
	/**
	 *
	 *  \brief delete a child from a given node.
	 */
	bool delFromLeaf(size_t objNo);

	/**
	 * 	\brief  Find the position of the object in a node.
	 * 
	 *  If the key is at current node,
	 * the function returns (pos, ECP_INTHIS). If the key is in a child to
	 * the left of pos, the function returns (pos, ECP_INLEFT). If the node
	 * is an internal node, the function returns (objCount, ECP_INRIGHT).
	 * Otherwise, the function returns ((size_t)-1, false).
	 * 
	 * The main assumption here is that we won't be searching for a key
	 *  in this node unless it (a) is not in the tree, or (b) it is in the
	 * subtree rooted at this node.
	 */
	OBJECTPOS findPos(const KeyType& key);

	/**
	 * 
	 * 	 \brief Find the position of the nearest object in a node,  given the key.
	 */
	OBJECTPOS findPos1(const KeyType& key);
	//OBJECTPOS findPos2(const KeyType& key);
	/**
	 *  We need to change the number of Objects and children, when we split a 
	 * 	node or merge two nodes.  
	 */
	inline void setCount(size_t newSize) {
		objCount = newSize;
		//objects. resize(newSize);
		elements.resize(newSize);
		children.resize(newSize + 1);
	}

	/**
	 * 	When we modify page, we can set it dirty, so that we need to write it back to disk. or 
	 * 	we don't need to write back.
	 */
	void setDirty(bool is) {
		_isDirty = is;
	}

	static void setDataSize(size_t maxDataSize, size_t pageSize,
			size_t overFlowSize) {
		_maxDataSize = maxDataSize;
		_pageSize = pageSize;
		_overFlowSize = overFlowSize;
	}

public:
	size_t childNo; //from 0 to objCount. Default is size_t(-1).

	size_t objCount; //the number of the objects, and if the node is not leafnode,
	//it has objCount+1 childrens.
	bool isLeaf; //leafNode of internal node, node that int disk we put the data beside the key. 
	bool loaded; //if it is loaded from disk.
	long fpos; //its streamoff in file.


	DataTypeVECTOR elements;
	BTreeNodeVECTOR children;
	BTreeNodePtr parent;

	/**
	 * 	\brief print the shape of  tree.
	 
	 *   eg. below is an example of display result of a btree with 14 nodes,with
	 *   the root node is "continued".
	 *  
	 *	
	 *	accelerating
	 *	alleviating
	 *	----|bahia
	 *	cocoa
	 *	----|----|continued
	 *	drought
	 *	early
	 *	----|howers
	 *	in
	 *	since
	 *	----|the
	 *	throughout
	 *	week
	 * 	 
	 */
	void display() {

		size_t i;
		for (i=0; i<objCount; i++) {
			if (!isLeaf) {
				if (children[i])
					children[i]->display();
				cout<<"----|";
			}
			if (elements[i]) {
				elements[i]->display();
				if (parent) {
					//cout<<"( "<<childNo<<" "<<parent->objCount<<") ";
					assert(childNo <= parent->objCount);
				}
			}
			cout<<endl;
		}
		if (!isLeaf) {
			if (children[i])
				children[i]->display();
			cout<<"----|";
		}
	}

public:
	static size_t activeNodeNum;
private:
	DBOBJVECTOR objects;
	bool _isDirty;
	static size_t _pageSize;
	static size_t _maxDataSize;
	static size_t _overFlowSize;
	//LockType _lock;
	static LockType _lock;
private:
//typedef pair<long, size_t> OverflowInfo;
//map<size_t, OverflowInfo> _preOverflowMap;
};

template<typename KeyType, typename DataType, typename LockType, typename Alloc> size_t BTreeNode<
		KeyType, DataType, LockType, Alloc>::activeNodeNum;

template<typename KeyType, typename DataType, typename LockType, typename Alloc> size_t BTreeNode<
		KeyType, DataType, LockType, Alloc>::_pageSize;

template<typename KeyType, typename DataType, typename LockType, typename Alloc> size_t BTreeNode<
		KeyType, DataType, LockType, Alloc>::_maxDataSize;

template<typename KeyType, typename DataType, typename LockType, typename Alloc> size_t BTreeNode<
		KeyType, DataType, LockType, Alloc>::_overFlowSize;

template<typename KeyType, typename DataType, typename LockType, typename Alloc> LockType
		BTreeNode< KeyType, DataType, LockType, Alloc>::_lock;

// Constructor initialises everything to its default value.
// Not that we assume that the node is a leaf,
// and it is not loaded from the disk.
template<typename KeyType, typename DataType, typename LockType, typename Alloc> BTreeNode<
		KeyType, DataType, LockType, Alloc>::BTreeNode() :
	childNo((size_t)-1), objCount(0), isLeaf(true), loaded(false), fpos(-1) {
	_isDirty = 1;
	//_overFlowAddress = 0;

	activeNodeNum++;
	//cout<<"activeNodeNum: "<<activeNodeNum<<endl;

}

// Read a node page to initialize this node from the disk
template<typename KeyType, typename DataType, typename LockType, typename Alloc> bool BTreeNode<
		KeyType, DataType, LockType, Alloc>::read(FILE* f) {

	long _overFlowAddress;
	//static int _rcount;

	if (!f) {
		return false;
	}

	//cout<<"read from fpos "<<fpos<<endl;
	// get to the right location
	if (0 != fseek(f, fpos, SEEK_SET)) {
		return false;
	}
	// read the leaf flag and the object count
	byte leafFlag = 0;
	if (1 != fread(&leafFlag, sizeof(byte), 1, f)) {
		return false;
	}
	if (1 != fread(&objCount, sizeof(size_t), 1, f)) {
		return false;
	}
	isLeaf = (leafFlag == 1);

	// read the addresses of the child pages
	if (objCount> 0 && !isLeaf) {
		long* childAddresses = new long[objCount + 1];
		long* thisChild = childAddresses;
		memset(childAddresses, 0xff, sizeof(long) * (objCount + 1));
		if (objCount + 1
				!= fread(childAddresses, sizeof(long), objCount + 1, f)) {
			delete[] childAddresses;
			return false;
		}
		children.resize(objCount + 1);

		//Only allocate childnode when the node is no a leaf node.
		if ( !isLeaf ) {
			for (size_t ctr = 0; ctr <= objCount; ctr++) {
				if (children[ctr] == 0) {
					children[ctr].reset(new BTreeNode);
				}
				children[ctr]->fpos = *thisChild++;
				children[ctr]->childNo = ctr;
				//cout<<children[ctr]->fpos<<endl;
			}
			delete[] childAddresses;
		}
	}

	// read the contents
	objects.resize(objCount);
	elements.resize(objCount);

	long nextReadPos = ftell(f);

	bool fposChanged = false;

	for (size_t ctr = 0; ctr < objCount; ctr++) {
		if (fposChanged) {
			if (0 != fseek(f, nextReadPos, SEEK_SET)) {
				return false;
			}
		}
		//cout<<" read from data"<< nextReadPos<<endl;
		size_t recSize;

		// read the size of the DbObj.
		if (1 != fread(&recSize, sizeof(size_t), 1, f)) {
			return false;
		}
		//cout<<recSize<< " vs "<< _maxDataSize<<endl;

		byte* pBuf = new byte[recSize];
		if (recSize <= _maxDataSize) {
			//read the data of the DbObj to pBuf.
			if (1 != fread(pBuf, recSize, 1, f)) {
				return false;
			}
			nextReadPos = ftell(f);
			fposChanged = false;
		} else {
			if (1 != fread(pBuf, _maxDataSize, 1, f)) {
				return false;
			}

			if (1 != fread(&_overFlowAddress, sizeof(long), 1, f) ) {
				return false;
			}
			nextReadPos = ftell(f);

			//cout<<" read from overflow"<< _overFlowAddress<<endl;
			//cout<<" read left size "<<recSize - _maxDataSize<<endl;

			// get to the overflow location
			fposChanged = true;
			if (0 != fseek(f, _overFlowAddress, SEEK_SET)) {
				return false;
			}
			if (1 != fread(pBuf + _maxDataSize, recSize - _maxDataSize, 1, f)) {
				return false;
			}
			//int nopage = (recSize -_maxDataSize) / _overFlowSize;
			//if (recSize - _maxDataSize > nopage * _overFlowSize) {
			//nopage++;
			//}
			//_preOverflowMap[ctr] = make_pair(_overFlowAddress, nopage);

		}
		assert(recSize != 0);
		//for dbg		
		objects[ctr]. reset(new DbObj(pBuf, recSize));

		//sync
		DataType dat;
		read_image(dat, objects[ctr]);
		elements[ctr].reset(new PtrObj<DataType, LockType, Alloc>(dat));

		//cout<<"reading ";
		//cout<<dat.size()<<" "<<dat<<endl;

		delete[] pBuf;
	}

	loaded = true;
	_isDirty = false;
	return true;
}

template<typename KeyType, typename DataType, typename LockType, typename Alloc> bool BTreeNode<
		KeyType, DataType, LockType, Alloc>::write(FILE* f) {

	//static int _wcount;

	typedef pair<long, size_t> OverflowInfo;
	map<size_t, OverflowInfo> _overflowMap;

	long _overFlowAddress = 0;

	// If we're not loaded, we haven't been changed it ,
	// so we can say that the flush was successful.
	if (!_isDirty) {
		return true;
	}

	if (!loaded) {
		return true;
	}

	if (!f) {
		return false;
	}

	if (0 != fseek(f, 0, SEEK_END)) {
		return false;
	}
	_overFlowAddress = ftell(f);

	//cout<<"write "<<_wcount++ <<endl;;

	// get to the right location
	if (0 != fseek(f, fpos, SEEK_SET)) {
		return false;
	}
	//cout<<"write data "<<fpos<<endl;

	// write the leaf flag and the object count
	byte leafFlag = isLeaf ? 1 : 0;
	if (1 != fwrite(&leafFlag, sizeof(byte), 1, f)) {
		return false;
	}
	if (1 != fwrite(&objCount, sizeof(size_t), 1, f)) {
		return false;
	}

	//cout<<"write count= "<<objCount<<endl;

	// write the addresses of the child pages
	if (objCount> 0 && !isLeaf) {
		long* childAddresses = new long[objCount + 1];
		long* thisChild = childAddresses;
		memset(childAddresses, 0xff, sizeof(long) * (objCount + 1));

		BIT tnvit = children.begin();
		while (tnvit != children.end()) {
			if ((BTreeNodePtr)(*tnvit) != 0) {
				*thisChild = (*tnvit)->fpos;
			}
			++thisChild;
			++tnvit;
		}
		size_t longsWritten = fwrite(childAddresses, sizeof(long),
				objCount + 1, f);
		delete[] childAddresses;
		if (objCount + 1 != longsWritten) {
			return false;
		}
	}
	//sync
	objects.resize(elements.size() );
	for (size_t ctr=0; ctr<objCount; ctr++) {
		objects[ctr].reset(new DbObj);
		write_image(*elements[ctr]->pdat, objects[ctr]);
		//cout<<"writing "<< *elements[ctr]->pdat <<endl;
	}

	// write the contents
	DBOBJVECTOR::iterator dovit = objects.begin();
	for (size_t ctr=0; ctr<objCount; ctr++) {

		DbObjPtr pObj = objects[ctr];
		size_t sz = pObj->getSize();
		//write the size of the data of DbObj.	

		if ( 1 != fwrite(&sz, sizeof(size_t), 1, f)) {
			return false;
		}

		//write the data of DbObj.	
		byte *pd;
		pd = (byte* )pObj->getData();
		if (sz <= _maxDataSize) {
			if ( 1 != fwrite(pd, sz, 1, f) ) {
				return false;
			}
		} else {
			int nopage = (sz -_maxDataSize) / _overFlowSize;
			if (sz - _maxDataSize > nopage * _overFlowSize) {
				nopage++;
			}
			if (1 != fwrite(pd, _maxDataSize, 1, f)) {
				return false;
			}
			if (1 != fwrite(&_overFlowAddress, sizeof(long), 1, f)) {
				return false;
			}
			_overflowMap[ctr] = make_pair(_overFlowAddress, nopage);

			//overFlowAddress is always at the end of file.
			_overFlowAddress += _overFlowSize * nopage;
		}
	}

	//write overflowpage
	for (map<size_t, OverflowInfo>::iterator it = _overflowMap.begin(); it
			!= _overflowMap.end(); it++) {

		byte *pd;
		size_t sz;
		pd = (byte* )objects[it->first]->getData();
		sz = objects[it->first]->getSize();

		//cout<<"size "<<sz<<endl;	
		//cout<<"overflow size "<< sz- _maxDataSize<<endl;

		byte *pbuf;
		pbuf = new byte[it->second.second * _overFlowSize];
		memcpy(pbuf, pd + _maxDataSize, sz- _maxDataSize);

		long overFlowAddress = it->second.first;

		/*if ( (_preOverflowMap.find(it->first) != _preOverflowMap.end())
		 && _preOverflowMap[it->first].second >= it->second.second) {
		 cout<<"not change overFlwoAddress";
		 overFlowAddress = _preOverflowMap[it->first].first;
		 }*/

		if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
			return false;
		}
		if (1 != fwrite(pbuf, it->second.second*_overFlowSize, 1, f)) {
			return false;
		}
		delete [] pbuf;
	}
	_isDirty = false;
	fflush(f);
	return true;
}

// Load a child node from the disk. This requires that we
// have the filepos already in place.
template<typename KeyType, typename DataType, typename LockType, typename Alloc> intrusive_ptr<BTreeNode<KeyType, DataType,LockType, Alloc> > BTreeNode<
		KeyType, DataType, LockType, Alloc>::loadChild(size_t childNum, FILE* f,
		const string& fileName) {
	
	BTreeNodePtr child;
	child = children[childNum];
	if ((BTreeNodePtr)child == 0) {
		child.reset(new BTreeNode);
		children[childNo] = child;
	}
	child->childNo = childNum;
	_lock.acquire_write_lock();
	if ( !child->loaded) {
		child->read(f);
		child->parent.reset(this);
	}
	_lock.release_write_lock();
	return child;

	/*assert(childNum <= objCount);
	BTreeNodePtr child;
	child = children[childNum];

	if (child !=0 && child->loaded) {
		return child;
	}

	if ((BTreeNodePtr)child == 0 && !child->loaded) {
		child.reset(new BTreeNode);
		children[childNo] = child;
	}
	child->childNo = childNum;
	//	boost::mutex::scoped_lock lk(mtx);	
	_lock.acquire_write_lock();
	if ( !child->loaded) {
		//f = fopen(fileName.c_str(), "r");		
		child->read(f);
		child->parent.reset(this);
		//fclose(f);			
	}
	_lock.release_write_lock();
	return child;*/
}

// Unload a child, which means that we get rid of all
// children in the children vector.
template<typename KeyType, typename DataType, typename LockType, typename Alloc> void BTreeNode<
		KeyType, DataType, LockType, Alloc>::unload() {
	if (loaded) {
		// Clear out all of the children
		BIT tnvit = children.begin();
		while (!isLeaf && tnvit != children.end()) {
			(*tnvit)->unload();
			//BIT temp = tnvit;
			tnvit->reset(0);
			++tnvit;			
		}

		DBOBJVECTOR::iterator dovit = objects.begin();
		DIT dovit1 = elements.begin();

		while (dovit != objects.end()) {
			dovit->reset(0);
			++dovit;
		}

		while (dovit1 != elements.end()) {
			dovit1->reset(0);
			++dovit1;
		}

		children.resize(0);
		objects.resize(0);
		elements.resize(0);

		// Empty the parent node and indicate that the
		// node is no longer loaded.
		parent.reset(0);
		loaded = false;
	}
}

template<typename KeyType, typename DataType, typename LockType, typename Alloc> bool BTreeNode<
		KeyType, DataType, LockType, Alloc>::delFromLeaf(size_t objNo) {
	bool ret = isLeaf;
	if (ret) {
		//objects[objNo].reset();		
		elements[objNo].reset(0);
		for (size_t ctr = objNo + 1; ctr < objCount; ctr++) {
			//objects[ctr - 1] = objects[ctr];
			elements[ctr-1] = elements[ctr];
		}
		setCount(objCount - 1);
	}
	_isDirty = 1;
	return ret;
}

// Find the position of the object in a node. If the key is at pos
// the function returns (pos, ECP_INTHIS). If the key is in a child to
// the left of pos, the function returns (pos, ECP_INLEFT). If the node
// is an internal node, the function returns (objCount, ECP_INRIGHT).
// Otherwise, the function returns ((size_t)-1, false).
// The main assumption here is that we won't be searching for a key
// in this node unless it (a) is not in the tree, or (b) it is in the
// subtree rooted at this node.


template<typename KeyType, typename DataType, typename LockType, typename Alloc> OBJECTPOS BTreeNode<
		KeyType, DataType, LockType, Alloc>::findPos(const KeyType& key) {

	OBJECTPOS ret((size_t)-1, ECP_NONE);
	DIT dovit = elements.begin();
	size_t ctr = 0;
	while (dovit < elements.end()) {
		//int compVal = CompareFun(key, (*dovit)->dat.get_key() );
		int compVal = key.compare((*dovit)->pdat->get_key() );
		if (compVal == 0) {
			return OBJECTPOS(ctr, ECP_INTHIS);
		} else if (compVal < 0) {
			if (isLeaf) {
				return ret;
			} else {
				return OBJECTPOS(ctr, ECP_INLEFT);
			}
		}
		++dovit, ++ctr;
	}
	if (!isLeaf) {
		return OBJECTPOS(ctr - 1, ECP_INRIGHT);
	}
	return ret;

}

template<typename KeyType, typename DataType, typename LockType, typename Alloc> OBJECTPOS BTreeNode<
		KeyType, DataType, LockType, Alloc>::findPos1(const KeyType& key) {

	OBJECTPOS ret((size_t)-1, ECP_NONE);
	DIT dovit = elements.begin();
	size_t ctr = 0;
	while (dovit < elements.end()) {
		//int compVal = CompareFun(key, (*dovit)->dat.get_key() );
		int compVal = key.compare((*dovit)->pdat->get_key() );
		if (compVal == 0) {
			return OBJECTPOS(ctr, ECP_INTHIS);
		} else if (compVal < 0) {
			if (isLeaf) {
				return OBJECTPOS(ctr, ECP_NEARLEFT);;
			} else {
				return OBJECTPOS(ctr, ECP_INLEFT);
			}
		}
		++dovit, ++ctr;
	}
	if (!isLeaf) {
		return OBJECTPOS(ctr - 1, ECP_INRIGHT);
	} else if (ctr>0) {
		return OBJECTPOS(ctr - 1, ECP_NEARRIGHT);
	}
	return ret;

}

NS_IZENELIB_AM_END
#endif
