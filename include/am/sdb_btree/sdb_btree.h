/**
 * @file sdb_btree.h
 * @brief The header file of sdb_btree.
 *
 *
 * This file defines class sdb_btree.
 */
#ifndef sdb_btree_H_
#define sdb_btree_H_

#include "sdb_node.h"
#include "sdb_btree_types.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <map>

#include <am/concept/DataType.h>
#include <util/ClockTimer.h>
using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 * 	\brief file version of cc-b*-btree
 *
 *   A B*-tree is a tree data structure used in the HFS and Reiser4 file systems,
 *  which requires non-root nodes to be at least 2/3 full instead of 1/2. To maintain
 *  this, instead of immediately splitting up a node when it gets full, its keys are
 *  shared with the node next to it. When both are full, then the two of them
 *  are split into three. Now SDBv1.0’s disk space is large than BerkeleyDB when
 *  storing the same data set. There is a perspective that, B*-tree can save more
 *  disk space than normal btree.
 *
 *  For implementation convience and maintainess, we only apply this delay splitting
 *  at leaves nodes.
 *
 *  Merging will occur when two sibling nodes' objCount both less than maxKeys/3/
 *
 *
 *
 */

template<typename KeyType,typename ValueType=NullType,typename LockType=NullLock, bool fixed = false,
typename Alloc=std::allocator<DataType<KeyType,ValueType> >
>class sdb_btree
: public AccessMethod<KeyType, ValueType, LockType, Alloc>
{
    enum {unloadbyRss = false};
    enum {unloadAll = true};
    enum {orderedCommit =true};
    enum {quickFlush = false};
    public:
    typedef sdb_node_<KeyType, ValueType, LockType, fixed, Alloc> sdb_node;
    typedef std::pair<sdb_node*, size_t> SDBCursor;
    public:
    /**
     * \brief constructor
     *
     * \param fileName is the name for data file if fileName ends with '#', we set b-tree mode to
     *  not delay split.
     */
    sdb_btree(const std::string& fileName = "sdb_btree.dat#");
    virtual ~sdb_btree();

    /**
     *
     *  \brief set mod
     *
     *  \param delaySplit, if true, the btree is cc-b*-btee, otherwise is normal cc-b-tree.
     *
     *  For ascending insertion, cc-b-tree is much faster than cc-b*-btree, while cc-b*-btree
     *  uses less disk space and find faster than cc-b-btree.
     *
     *
     */
    void setBtreeMode(bool autoTuning=true, unsigned int optimizeNum = 65536, bool delaySplit=true)
{
    _isDelaySplit = delaySplit;
    _autoTunning = autoTuning;
    _optimizeNum = optimizeNum;
}
/**
 *  \brief set the MaxKeys
 *
 *  Note that it must be at least 6.
 *  It can only be called before open. And it doesn't work when open an existing dat file,
 *  _sfh.maxKeys will be read from file.
 *
 */
void setMaxKeys(size_t maxkeys)
{
    assert( _isOpen == false );
    _sfh.maxKeys = maxkeys;
    if (_sfh.maxKeys < 6)
    {
        cout<<"Note: maxKeys at least 6.\nSet to 6 automatically.\n";
        _sfh.maxKeys = 6;
    }
}

/**
 *   \brief set maxKeys for fileHeader
 *
 *   maxKeys is 2*degree
 *   Note that it must be at least 6
 *   It can only be called  before opened.And it doesn't work when open an existing dat file,
 *  _sfh.maxKeys will be read from file.
 */
void setDegree(size_t degree)
{
    assert( _isOpen == false );
    _sfh.maxKeys = 2*degree;
    if (_sfh.maxKeys < 6)
    {
        cout<<"Note: maxKeys at leaset 6.\nSet to 6 automatically.\n";
        _sfh.maxKeys = 6;
    }
}

/**
 * 	\brief set tha pageSize of fileHeader.
 *
 *   It can only be called before open.And it doesn't work when open an existing dat file,
 *  _sfh.pageSize will be read from file.
 *
 *   It should set the pageSize according the maxKeys and max inserting data size.
 *   When pageSize is too small, overflowing will occur and cause efficiency to decline.
 *   When pageSize is too large, it will waste disk space.
 *
 */
void setPageSize(size_t pageSize)
{
    assert( _isOpen == false );
    _sfh.pageSize = pageSize;
}

/**
 *  \brief set Cache Size.
 *
 *  Cache Size is the active node number in memory.When cache is full,
 *  some nodes will be released.
 *
 * 	We would peroidically flush the memory items, according to the cache Size.
 */

void setCacheSize(size_t sz)
{
    _sfh.cacheSize = sz;
    _cacheSize = sz;
}


/**
 *  \brief set file name.
 *
 */
void setFileName(const std::string& fileName )
{
    _fileName = fileName;
}

/**
 * 	\brief return the file name of the SequentialDB
 */
std::string getFileName() const
{
    return _fileName;
}

/**
 * 	 \brief open the database.
 *
 *   Everytime  we use the database, we mush open it first.
 */
bool open();

bool is_open()
{
    return _isOpen;
}

void clear()
{
    close();
    std::remove(_fileName.c_str() );
    _sfh.initialize();
    open();
}

/**
 * 	 \brief close the database.
 *
 *    if we don't call it, it will be automately called in deconstructor
 */
bool close()
{
    if ( !_isOpen)
        return true;
    _isOpen = false;
    flush();
    //note that _root can be  NULL, if there is no items.
    if (_root)
    {
        _root->unload();
        delete _root;
        _root = 0;
    }
    if (_dataFile != 0)
    {
        fflush(_dataFile);
        fclose(_dataFile);
        _dataFile = 0;
    }
    return true;
}

template<typename AM>
bool dump(AM& other)
{
    if (!is_open() )
        open();
    if ( !other.is_open() )
    {
        if ( !other.open() )
            return false;
    }
    SDBCursor locn = get_first_locn();
    KeyType key;
    ValueType value;
    while (get(locn, key, value))
    {
        other.insert(key, value);
        if ( !seq(locn) )
            break;
    }
    return true;
}

bool dump2f(const string& fileName)
{
    sdb_btree other(fileName);
    if ( !other.open() )
        return false;
    return dump( other );
}

/**
 * 	 \brief del an item from the database
 *
 */
bool del(const KeyType& key);
/**
 * 	\brief insert an item.
 */
bool insert(const DataType<KeyType,ValueType>& rec)
{
    return insert(rec.key, rec.value);
}

/**
 *  \brief insert an item.
 */
bool insert(const KeyType& key, const ValueType& value);

/**
 *  \brief find an item given a key.
 */
ValueType* find(const KeyType& key)
{
    SDBCursor locn;
    if ( search(key, locn) )
    {
        ScopedReadLock<LockType> lock(_flushLock);
        return new ValueType(locn.first->values[locn.second]);
    }
    else return NULL;
}

bool get(const KeyType& key, ValueType& value)
{
    _safeFlushCache();
    ScopedReadLock<LockType> lock(_flushLock);
    SDBCursor locn;
    if ( search_(key, locn) )
    {
        //cout<<"get "<<endl;
        value = locn.first->values[locn.second];
        return true;
    }
    return false;
}

/**
 *  \brief find an item given a key.
 */
const ValueType* find(const KeyType& key)const
{
    return (const ValueType*) (this->find(key));
}

/**
 *  \brief updata an item with given key, if it not exist, insert it directly.
 */
bool update(const KeyType& key, const ValueType& val);

/**
 *  \brief updata an item with given key, if it not exist, insert it directly.
 */
bool update(const DataType<KeyType,ValueType>& rec)
{
    return update(rec.key, rec.value);
}

/**
 *
 * \brief get the number of the items.
 */
int num_items()
{
    return _sfh.numItems;
}

/**
 *  \brief get an item from given Locn.	 *
 */
bool get(const SDBCursor& locn, DataType<KeyType,ValueType>& rec)
{
    return get(locn, rec.key, rec.value);
}

/**
 *  \brief get an item from given Locn.	 *
 */
bool get(const SDBCursor& locn, KeyType& key, ValueType& value);

/**
 *  \brief get the cursor of the first item.
 *
 */
SDBCursor get_first_locn()
{
    ScopedReadLock<LockType> lock(_flushLock);
    sdb_node* node = getRoot();
    while ( node && !node->isLeaf )
        node = node->loadChild(0, _dataFile);
    return SDBCursor(node, 0);
}

SDBCursor get_last_locn()
{
    ScopedReadLock<LockType> lock(_flushLock);
    sdb_node* node = getRoot();
    while (node && !node->isLeaf )
    {
        node = node->loadChild(node->objCount, _dataFile);
    }
    return SDBCursor(node, node->objCount-1);
}

/**
 * 	\brief get the next or prev item.
 *
 *  \locn when locn is default value, it will start with firt element when sdri=ESD_FORWARD
 *   and start with last element when sdir = ESD_BACKWARD
 */
bool seq(SDBCursor& locn, KeyType& key, ValueType& value, ESeqDirection sdir=ESD_FORWARD)
{
    bool ret = seq(locn);
    get(locn, key, value);
    return ret;
}
bool seq(SDBCursor& locn, DataType<KeyType, ValueType>& dat, ESeqDirection sdir=ESD_FORWARD)
{
    return seq(locn, dat.key, dat.value, sdir);
}

bool seq(SDBCursor& locn, ESeqDirection sdir=ESD_FORWARD)
{
    _flushCache(locn);
    ScopedReadLock<LockType> lock(_flushLock);
    return seq_(locn, sdir);
}

bool seq_(SDBCursor& locn, ESeqDirection sdir=ESD_FORWARD);

/**
 * 	\brief write all the items in memory to file.
 */

void flush();
/**
 * 	\brief write back the dirypages
 */
void commit()
{
    if (_root)
    {
        _flush(_root, _dataFile);
        _sfh.rootPos = _root->fpos;
    }

    if ( !_dataFile )return;

    if ( 0 != fseek(_dataFile, 0, SEEK_SET))
    {
        abort();
    }

    //write back the fileHead later, for overflow may occur when
    //flushing.
    _sfh.toFile(_dataFile);
    fflush(_dataFile);
}

/**
 *   for debug.  print the shape of the B tree.
 */
void display(std::ostream& os = std::cout, bool onlyheader = false)
{
    _sfh.display(os);
    os<<"activeNum: "<<_activeNodeNum<<endl;
    os<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
    if (!onlyheader && _root)_root->display(os);
}

/**
 *
 *  \brief Get the DB cursor of given key
 *
 */
SDBCursor search(const KeyType& key)
{
    SDBCursor locn;
    search(key, locn);
    return locn;
}
/**
 *   \brief get the cursor for given key
 *
 *   @param locn is cursor of key.
 *   @return true if key exists otherwise false.
 *
 */
bool search(const KeyType& key, SDBCursor& locn)
{
    _safeFlushCache();
    ScopedReadLock<LockType> lock(_flushLock);
    return search_(key, locn);
}

bool search_(const KeyType& key, SDBCursor& locn);

sdb_node* getRoot()
{
    if ( _root == NULL )
    {
        _root = new sdb_node(_sfh, _fileLock, _activeNodeNum);
        _root->fpos = _sfh.rootPos;
        _root->read(_dataFile);
    }
    return _root;
}

void optimize()
{
    optimize_();
    //		string tempfile = _fileName+ ".swap";
    //		dump2f(tempfile);
    //		close();
    //		std::remove(_fileName.c_str() );
    //		std::rename(tempfile.c_str(), _fileName.c_str() );
    //		std::remove(tempfile.c_str());
    //		open();
}

void fillCache()
{
    queue<sdb_node*> qnode;
    qnode.push( getRoot() );

    while ( !qnode.empty() )
    {
        sdb_node* popNode = qnode.front();
        qnode.pop();
        if (popNode && !popNode->isLeaf && popNode->isLoaded )
        {
            for (size_t i=0; i<=popNode->objCount; i++)
            {
                if ( _activeNodeNum> _sfh.cacheSize )
                    goto LABEL;
                sdb_node* node = popNode->loadChild(i, _dataFile);
                if ( node )
                    qnode.push( node );

            }
        }
    }
LABEL:
    return;
}

private:
sdb_node* _root;
FILE* _dataFile;
CbFileHeader _sfh;
size_t _cacheSize;

bool _isDelaySplit;
bool _isOpen;
bool _autoTunning;
unsigned int _optimizeNum;

izenelib::am::CompareFunctor<KeyType> _comp;
std::string _fileName; // name of the database file
private:
LockType _fileLock;
LockType _flushLock;
size_t _activeNodeNum;
size_t _dirtyPageNum;

private:
unsigned long _initRss;
unsigned int _flushCount;

inline void _setDirty(sdb_node* node)
{
    if ( !node->isDirty )
    {
        ++_dirtyPageNum;
        node->setDirty(true);
    }
}

void _flushCache()
{
    getRoot();
    ++_flushCount;
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        _flushCacheImpl();
    }
}

void _safeFlushCache()
{
    getRoot();
    ++_flushCount;
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        //    cout<<"_safeFlushCache"<<endl;
        ScopedWriteLock<LockType> lock(_flushLock);
        _flushCacheImpl();
    }
}

//for seq, reset SDBCursor
void _flushCache(SDBCursor& locn)
{
    getRoot();
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        KeyType key;
        ValueType value;
        get(locn, key, value);
        {
            //cout<<"_flushCache(locn)"<<endl;
            ScopedWriteLock<LockType> lock(_flushLock);
            _flushCacheImpl();
        }
        search(key, locn);
    }
}

void _flushCacheImpl(bool quickFlush=false)
{
    if ( _activeNodeNum < _sfh.cacheSize )
        return;
#ifdef  DEBUG
    cout<<"\n\ncache is full..."<<endl;
    cout<<"activeNum: "<<_activeNodeNum<<endl;
    cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
    //display();
#endif
    izenelib::util::ClockTimer timer;
    if ( !quickFlush )
        commit();

#ifdef DEBUG
    printf("commit elapsed 1 ( actually ): %lf seconds\n",
           timer.elapsed() );
#endif
    if (unloadAll)
    {
        for (size_t i=0; i<_root->objCount+1; i++)
        {
            _root->children[i]->unload();
        }
        if (_activeNodeNum !=1 )
        {
            cout<<"Warning! multi-thread enviroment"<<endl;
            _activeNodeNum = 1;
        }
#ifdef DEBUG
        cout<<"\n\nstop unload..."<<endl;
        cout<<_activeNodeNum<<" vs "<<_sfh.cacheSize <<endl;
        cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
#endif
        return;
    }
    else
    {

        queue<sdb_node*> qnode;
        qnode.push(_root);

        size_t popNum = 0;
        size_t escapeNum = _activeNodeNum>>1;
        sdb_node* interval = NULL;
        while ( !qnode.empty() )
        {
            sdb_node* popNode = qnode.front();
            qnode.pop();
            popNum++;

            if ( popNum >= escapeNum )
            {
                if ( popNode == interval )
                    break;

                if ( interval == NULL && !popNode->isLeaf )
                {
                    interval = popNode->children[0];
                }

                if ( popNode->isDirty && quickFlush)
                    _flush(popNode, _dataFile);

                popNode->unload();
            }

            if (popNode && popNode->isLoaded && !popNode->isLeaf)
            {
                for (size_t i=0; i<popNode->objCount+1; i++)
                {
                    if ( popNode->children[i] )
                    {
                        qnode.push( popNode->children[i] );
                    }
                    else
                    {
                        //cout<<"corrupted nodes!!!"<<endl;
                    }

                }
            }

        }

#ifdef DEBUG
        cout<<"stop unload..."<<endl;
        cout<<_activeNodeNum<<" vs "<<_sfh.cacheSize <<endl;
        //display();
#endif
        fflush(_dataFile);
    }

}

sdb_node* _allocateNode()
{
    sdb_node* newNode;

    newNode = new sdb_node(_sfh, _fileLock, _activeNodeNum);
    newNode->isLoaded = true;
    newNode->isDirty = true;
    assert(sizeof(CbFileHeader) < 1024);
    newNode->fpos = SDB_FILE_HEAD_SIZE + _sfh.pageSize
                    *(_sfh.nPages+_sfh.oPages);

    //cout<<"allocate idx="<<CbFileHeader::nPages<<" "<<newNode->fpos;
    ++_sfh.nPages;
    ++_dirtyPageNum;
    ++_activeNodeNum;

    //pre allocate memory for newNode for efficiency
    //		newNode->keys.resize(_sfh.maxKeys);
    //		newNode->values.resize(_sfh.maxKeys);
    //		newNode->children.resize(_sfh.maxKeys+1);

    return newNode;
}

void _split(sdb_node* parent, size_t childNum, sdb_node* child);
void _split3Leaf(sdb_node* parent, size_t childNum);
sdb_node* _merge(sdb_node* &parent, size_t objNo);

bool _seqNext(SDBCursor& locn);
bool _seqPrev(SDBCursor& locn);
void _flush(sdb_node* node, FILE* f);
bool _delete(sdb_node* node, const KeyType& key);

// Finds the location of the predecessor of this key, given
// the root of the subtree to search. The predecessor is going
// to be the right-most object in the right-most leaf node.
SDBCursor _findPred(sdb_node* node)
{
    SDBCursor ret(NULL, (size_t)-1);
    sdb_node* child = node;
    while (!child->isLeaf)
    {
        child = child->loadChild(child->objCount, _dataFile);
    }
    ret.first = child;
    ret.second = child->objCount - 1;
    return ret;
}

// Finds the location of the successor of this key, given
// the root of the subtree to search. The successor is the
// left-most object in the left-most leaf node.
SDBCursor _findSucc(sdb_node* node)
{
    SDBCursor ret(NULL, (size_t)-1);
    sdb_node* child = node;
    while (!child->isLeaf)
    {
        child = child->loadChild(0, _dataFile);
    }
    ret.first = child;
    ret.second = 0;
    return ret;
}

void optimize_(bool autoTuning = false)
{
    commit();
    double ofactor = double(_sfh.oPages)/double(_sfh.nPages)+1;
    int pfactor = int(ofactor);

    //auto adapt cache size.
    setCacheSize( _sfh.cacheSize/pfactor );

    if ( autoTuning )
    {

        string tempfile = _fileName + ".swap";
        sdb_btree other(tempfile);
        if ( ofactor> 0.1 )
        {

            double dfactor = ofactor/pfactor;

            //cout<<pfactor<<" : "<<dfactor<<endl;

            other.setPageSize( _sfh.pageSize*(pfactor+1) );
            other.setCacheSize( _sfh.cacheSize/pfactor );
            other.setMaxKeys( int(_sfh.maxKeys/dfactor) );
            //other.open();

            dump(other);
            other.close();
            close();
            std::remove(_fileName.c_str() );
            std::rename(tempfile.c_str(), _fileName.c_str() );
            std::remove(tempfile.c_str());
            open();
        }
    }
}

};

// The constructor simply sets up the different data members
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_btree< KeyType, ValueType, LockType, fixed, Alloc>::sdb_btree(
    const std::string& fileName)
{
    _root = 0;
    _isDelaySplit = true;
    _autoTunning = false;
    _optimizeNum = 8192*2;

    _fileName = fileName;

    int len = _fileName.size();
    if (_fileName[len-1] == '#')
    {
        _isDelaySplit = false;
        _fileName.erase(len-1);
    }
    _dataFile = 0;
    _cacheSize = 0;
    _isOpen = false;

    _activeNodeNum = 0;
    _dirtyPageNum = 0;

    _flushCount = 0;

}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_btree< KeyType, ValueType, LockType, fixed, Alloc>::~sdb_btree()
{
    close();
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::search_(const KeyType& key, SDBCursor& locn)
{
    //do Flush, when cache is full
    if ( !_isOpen)
        return false;
    //cout<<"before flush"<<endl;
    //_safeFlushCache();

    //cout<<"acquire read lock"<<endl;
    //_flushLock.acquire_read_lock();
    locn.first = 0;
    locn.second = (size_t)-1;

    sdb_node* temp = getRoot();
    while (1)
    {
        if ( !temp)
            return false;
        int i = temp->objCount;
        int low = 0;
        int high = i-1;
        int compVal;
        while (low <= high)
        {
            int mid = (low+high)/2;
            compVal = _comp(key, temp->keys[mid]);
            if (compVal == 0)
            {
                locn.first = temp;
                locn.second = mid;
                //_flushLock.release_read_lock();
                return true;
            }
            else if (compVal < 0)
                high = mid-1;
            else
            {
                low = mid+1;
            }
        }

        if (!temp->isLeaf)
        {
            temp = temp->loadChild(low, _dataFile);
        }
        else
        {
            locn.first = temp;
            locn.second = low;

            if (low >= (int)temp->objCount)
            {
                locn.second = low - 1;
                //_flushLock.release_read_lock();
                seq_(locn);
                return false;
            }
            break;
        }
    }
    //_flushLock.release_read_lock();
    return false;
}

// Splits a child node, creating a new node. The median value from the
// full child is moved into the *non-full* parent. The keys above the
// median are moved from the full child to the new child.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_split(sdb_node* parent, size_t childNum, sdb_node* child)
{

    //display();
    size_t i = 0;
    size_t leftCount = (child->objCount)>>1;
    size_t rightCount = child->objCount-1-leftCount;
    sdb_node* newChild = _allocateNode();
    newChild->childNo = childNum+1;

    newChild->isLeaf = child->isLeaf;
    newChild->setCount(rightCount);

    // Put the high values in the new child, then shrink the existing child.
    for (i = 0; i < rightCount; i++)
    {
        newChild->keys[i] = child->keys[leftCount +1 + i];
        newChild->values[i] = child->values[leftCount +1 + i];
    }

    if (!child->isLeaf)
    {
        for (i = 0; i < rightCount+1; i++)
        {
            sdb_node* mover = child->children[leftCount +1 + i];
            newChild->children[i] = mover;
            mover->childNo = i;
            mover->parent = newChild;
        }
    }

    KeyType savekey = child->keys[leftCount];
    ValueType savevalue = child->values[leftCount];
    child->setCount(leftCount);

    // Move the child pointers above childNum up in the parent
    parent->setCount(parent->objCount + 1);
    for (i = parent->objCount; i> childNum + 1; i--)
    {
        parent->children[i] = parent->children[i - 1];
        //if(parent->children[i])
        parent->children[i]->childNo = i;
    }
    parent->children[childNum + 1] = newChild;
    newChild->childNo = childNum + 1;
    newChild->parent = parent;

    for (i = parent->objCount - 1; i> childNum; i--)
    {
        parent->keys[i] = parent->keys[i - 1];
        parent->values[i] = parent->values[i - 1];
    }
    parent->keys[childNum] = savekey;
    parent->values[childNum] = savevalue;
    _setDirty(child);
    _setDirty(newChild);
    _setDirty(parent);
    //child->setDirty(1);
    //newChild->setDirty(1);
    //parent->setDirty(1);
}
//split two full leaf nodes into tree 2/3 ful nodes.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_split3Leaf(sdb_node* parent, size_t childNum)
{

    size_t i = 0;
    size_t count1 = (_sfh.maxKeys<<1)/3;
    size_t count2 = _sfh.maxKeys - _sfh.maxKeys/3-1;
    size_t count3 = (_sfh.maxKeys<<1) -1 -count1 -count2;

    sdb_node* child1 = parent->loadChild(childNum, _dataFile);
    sdb_node* child2 = parent->loadChild(childNum+1, _dataFile);

    sdb_node* newChild = _allocateNode();

    //swap newchild's fpos with child2
    long tempfpos = child2->fpos;
    child2->fpos = newChild->fpos;
    newChild->fpos = tempfpos;

    newChild->isLeaf =true;
    newChild->setCount(count3);
    newChild->parent = parent;

    KeyType tkey1 = child1->keys[(_sfh.maxKeys<<1)/3];
    ValueType tvalue1 = child1->values[(_sfh.maxKeys<<1)/3];
    KeyType tkey2 = child2->keys[_sfh.maxKeys/3];
    ValueType tvalue2 = child2->values[_sfh.maxKeys/3];

    // Put the high values in the new child, then shrink the existing child.
    for (i = 0; i < _sfh.maxKeys - count1 -1; i++)
    {
        newChild->keys[i] = child1->keys[count1+1+i];
        newChild->values[i] = child1->values[count1+1+i];
    }

    newChild->keys[ _sfh.maxKeys - count1 -1] = parent->keys[childNum];
    newChild->values[ _sfh.maxKeys - count1 -1] = parent->values[childNum];

    for (i = _sfh.maxKeys - count1; i < count3; i++)
    {
        newChild->keys[i] = child2->keys[i-_sfh.maxKeys+count1];
        newChild->values[i] = child2->values[i-_sfh.maxKeys+count1];
    }

    for (i=0; i<count2; i++)
    {
        child2->keys[i] = child2->keys[_sfh.maxKeys/3+1+i];
        child2->values[i] = child2->values[_sfh.maxKeys/3+1+i];
    }

    child1->setCount(count1);
    child2->setCount(count2);

    // Move the child pointers above childNum up in the parent
    parent->setCount(parent->objCount + 1);
    parent->keys[childNum] = tkey1;
    parent->values[childNum] = tvalue1;

    for (i = parent->objCount; i> childNum + 2; i--)
    {
        parent->children[i] = parent->children[i - 1];
        parent->children[i]->childNo = i;
    }
    parent->children[childNum + 1] = newChild;
    newChild->childNo = childNum + 1;

    for (i = parent->objCount-1; i> childNum+1; i--)
    {
        parent->keys[i] = parent->keys[i - 1];
        parent->values[i] = parent->values[i - 1];
    }
    parent->keys[childNum+1] = tkey2;
    parent->values[childNum+1] = tvalue2;
    parent->children[childNum+2] = child2;
    child2->childNo = childNum+2;

    _setDirty(child1);
    _setDirty(child2);
    _setDirty(newChild);
    _setDirty(parent);

    //child1->setDirty(1);
    //child2->setDirty(1);
    //newChild->setDirty(1);
    //parent->setDirty(1);
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_node_<KeyType, ValueType, LockType, fixed, Alloc>* sdb_btree<
KeyType, ValueType, LockType, fixed, Alloc>::_merge(sdb_node* &parent,
        size_t objNo)
{

    size_t i = 0;
    sdb_node* c1 = parent->loadChild(objNo, _dataFile);
    sdb_node* c2 = parent->loadChild(objNo+1, _dataFile);
    size_t _minDegree = _sfh.maxKeys/3;

    for (i = 0; i < _minDegree - 1; i++)
    {
        c1->keys[_minDegree+i] = c2->keys[i];
        c1->values[_minDegree+i] = c2->values[i];

    }
    if (!c2->isLeaf)
    {
        for (i = 0; i < _minDegree; i++)
        {
            size_t newPos = _minDegree + i;
            c2->loadChild(i, _dataFile);
            c1->children[newPos] = c2->children[i];
            c1->children[newPos]->childNo = newPos;
            c1->children[newPos]->parent = c1;//wps add it!
        }
    }

    // Put the parent into the middle
    //c1->elements[_minDegree - 1] = parent->elements[objNo];

    c1->keys[_minDegree-1] = parent->keys[objNo];
    c1->values[_minDegree-1] = parent->values[objNo];
    c1->setCount(2*_minDegree-1);

    // Reshuffle the parent (it has one less object/child)
    for (i = objNo + 1; i < parent->objCount; i++)
    {
        parent->keys[i-1] = parent->keys[i];
        parent->values[i-1] = parent->values[i];
        parent->loadChild(i+1, _dataFile);
        parent->children[i] = parent->children[i + 1];
        parent->children[i]->childNo = i;
    }
    parent->setCount(parent->objCount-1);

    if (parent->objCount == 0)
    {
        parent = c1;
    }

    // Note that c2 will be release. The node will be deallocated
    // and the node's location on
    // disk will become inaccessible.

    c2->unloadself();
    delete c2;
    c2 = 0;

    _setDirty(c1);
    _setDirty(parent);
    //c1->setDirty(1);
    //parent->setDirty(1);

    // Return a pointer to the new child.
    return c1;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::insert(const KeyType& key, const ValueType& value)
{
    if ( !_isOpen)
        return false;
    if (_sfh.numItems == _optimizeNum)
        optimize_(_autoTunning);

    _flushCache();
    getRoot();
    if (_root->objCount >= _sfh.maxKeys)
    {
        // Growing the tree happens by creating a new
        // node as the new root, and splitting the
        // old root into a pair of children.
        sdb_node* oldRoot = _root;

        _root = _allocateNode();
        _root->setCount(0);
        _root->isLeaf = false;
        _root->children[0] = oldRoot;
        oldRoot->childNo = 0;
        oldRoot->parent = _root;
        _split(_root, 0, oldRoot);
        goto L0;
    }
    else
    {
L0:
        register sdb_node* node = _root;

L1:
        register size_t i = node->objCount;
        register int low = 0;
        register int high = i-1;
        register int mid;
        register int compVal;
        while (low<=high)
        {
            mid = (low+high)>>1;
            compVal = _comp(key, node->keys[mid]);
            if (compVal == 0)
                return false;
            else if (compVal < 0)
                high = mid-1;
            else
            {
                low = mid+1;
            }
        }

        // If the node is a leaf, we just find the location to insert
        // the new item, and shuffle everything else up.
        if (node->isLeaf)
        {
            node->setCount(node->objCount + 1);
            for (; (int)i> low; i--)
            {
                node->keys[i] = node->keys[i-1];
                node->values[i] = node->values[i-1];
            }
            node->keys[low] = key;
            node->values[low] = value;
            _setDirty(node);
            //node->setDirty(1);

            ++_sfh.numItems;
            return true;

        }

        // If the node is an internal node, we need to find
        // the location to insert the value ...
        else
        {
            // Load the child into which the value will be inserted.
            sdb_node* child = node->loadChild(low, _dataFile);

            //If the child node is full , we will insert into its adjacent nodes, and if bothe are
            //are full, we will split the two node to three nodes.
            if (child->objCount >= _sfh.maxKeys)
            {
                if ( !child->isLeaf || !_isDelaySplit)
                {
                    _split(node, low, child);
                    compVal = _comp(key, node->keys[low]);
                    if (compVal == 0)
                        return false;
                    if (compVal> 0)
                    {
                        ++low;
                    }
                    child = node->loadChild(low, _dataFile);
                }
                else
                {
                    sdb_node* adjNode;
                    int splitNum = low;
                    if ((size_t)low < node->objCount)
                    {
                        adjNode = node->loadChild(low+1, _dataFile);
                        if (adjNode->objCount < _sfh.maxKeys)
                        {

                            //case: child's last key equal inserting key
                            if (_comp(child->keys[child->objCount-1], key)==0)
                            {
                                return false;
                            }
                            adjNode->setCount(adjNode->objCount+1);
                            for (size_t j=child->objCount-1; j>0; j--)
                            {
                                adjNode->keys[j] = adjNode->keys[j-1];
                                adjNode->values[j] = adjNode->values[j-1];
                            }
                            adjNode->keys[0] = node->keys[low];
                            adjNode->values[0] = node->values[low];
                            _setDirty(adjNode);
                            _setDirty(node);
                            //adjNode->setDirty(true);
                            //node->setDirty(true);

                            //case: all of the keys in child are less than inserting keys.
                            if (_comp(child->keys[child->objCount-1], key)<0)
                            {
                                node->keys[low] = key;
                                node->values[low] = value;
                                ++_sfh.numItems;
                                return true;

                            }

                            //case: insert the item into the new child.
                            node->keys[low] = child->keys[child->objCount-1];
                            node->values[low]
                            = child->values[child->objCount-1];
                            //child->setDirty(true);
                            _setDirty(child);
                            child->setCount(child->objCount-1);
                            node = child;

                            goto L1;
                        }
                    }

                    //case: the right sibling is full or no right sibling
                    if (low>0)
                    {
                        adjNode = node->loadChild(low-1, _dataFile);
                        //case: left sibling is no full
                        if (adjNode->objCount < _sfh.maxKeys)
                        {
                            //cacheL child's first key equal inserting key,do nothing
                            if (_comp(child->keys[0], key) == 0)
                            {
                                return false;
                            }

                            adjNode->setCount(adjNode->objCount+1);
                            adjNode->keys[adjNode->objCount-1]
                            = node->keys[low-1];
                            adjNode->values[adjNode->objCount-1]
                            = node->values[low-1];
                            _setDirty(adjNode);
                            _setDirty(node);
                            //adjNode->setDirty(true);
                            //node->setDirty(true);
                            //case: all of the keys in child are bigger than inserting keys.
                            if (_comp(key, child->keys[0])< 0)
                            {
                                node->keys[low-1] = key;
                                node->values[low-1] = value;
                                ++_sfh.numItems;
                                return true;
                            }
                            node->keys[low-1] = child->keys[0];
                            node->values[low-1] = child->values[0];

                            //insert key into child, child's firt item is already put to its parent
                            size_t j=1;
                            bool ret=true;
                            //child->setDirty(true);
                            _setDirty(child);
                            for (; j<child->objCount; j++)
                            {
                                //inserting key exists, mark it
                                if (_comp(child->keys[j], key) == 0)
                                {
                                    ret = false;
                                }
                                //have found the right place for inserting key.
                                if (ret && _comp(child->keys[j], key)>0)
                                {
                                    child->keys[j-1] = key;
                                    child->values[j-1] = value;
                                    ++_sfh.numItems;
                                    return true;
                                }
                                child->keys[j-1] = child->keys[j];
                                child->values[j-1] = child->values[j];
                            }
                            //inserting key exists
                            if ( !ret)
                            {
                                child->setCount(child->objCount-1);
                                return false;
                            }
                            //insert the key at last positon of child
                            child->keys[j-1] = key;
                            child->values[j-1] = value;
                            ++_sfh.numItems;
                            return true;
                        }
                    }

                    //case: both nodes are full
                    if ( (size_t)splitNum>0)
                    {
                        splitNum = splitNum -1;
                    }
                    _split3Leaf(node, splitNum);

                    if (_comp(node->keys[splitNum], key) == 0)
                        return false;
                    if (_comp(node->keys[splitNum+1], key) == 0)
                        return false;
                    if (_comp(key, node->keys[splitNum]) < 0)
                    {
                        child = node->loadChild(splitNum, _dataFile);
                    }
                    else if (_comp(key, node->keys[splitNum+1]) <0)
                    {
                        child=node->loadChild(splitNum+1, _dataFile);
                    }
                    else
                    {
                        child=node->loadChild(splitNum+2, _dataFile);
                    }
                }
            }
            // Insert the key (recursively) into the non-full child
            // node.
            node = child;
            goto L1;
        }
    }
}

// Write all nodes in the tree to the file given.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_flush(sdb_node* node, FILE* f)
{

    // Bug out if the file is not valid
    if (!f)
    {
        return;
    }

    if (orderedCommit)
    {
        typedef map<long, sdb_node*> COMMIT_MAP;
        typedef typename COMMIT_MAP::iterator CMIT;
        COMMIT_MAP toBeWrited;
        queue<sdb_node*> qnode;
        qnode.push(node);
        while (!qnode.empty() )
        {
            sdb_node* popNode = qnode.front();
            if (popNode->isLoaded && popNode-> isDirty)
                toBeWrited.insert(make_pair(popNode->fpos, popNode) );
            qnode.pop();
            if (popNode && !popNode->isLeaf)
            {
                for (size_t i=0; i<popNode->objCount+1; i++)
                {
                    if (popNode->children && popNode->children[i])
                        qnode.push(popNode->children[i]);
                }
            }
        }

        CMIT it = toBeWrited.begin();
        for (; it != toBeWrited.end(); it++)
        {
            if (it->second->write(f) )
                --_dirtyPageNum;
        }

    }
    else
    {

        queue<sdb_node*> qnode;
        qnode.push(node);
        while (!qnode.empty())
        {
            sdb_node* popNode = qnode.front();
            if (popNode && popNode->isLoaded)
            {
                if (popNode->write(f) )
                    --_dirtyPageNum;
            }
            qnode.pop();
            if (popNode && !popNode->isLeaf)
            {
                for (size_t i=0; i<popNode->objCount+1; i++)
                {
                    if (popNode->children[i])
                        qnode.push(popNode->children[i]);
                }
            }
        }

    }

}

// Internal delete function, used once we've identified the
// location of the node from which a key is to be deleted.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_delete(sdb_node* nd, const KeyType& k)
{
    if ( !_isOpen)
        return false;
    bool ret = false;
    // Find the object position. op will have the position
    // of the object in op.first, and a flag (op.second)
    // saying whether the object at op.first is an exact
    // match (true) or if the object is in a child of the
    // current node (false). If op.first is -1, the object
    // is neither in this node, or a child node.

    sdb_node* node = nd;
    KeyType key = k;
    size_t _minDegree = _sfh.maxKeys/3;

L0:
    KEYPOS op = node->findPos(key);
    if (op.first != (size_t)-1) // it's in there somewhere ...
    {
        if (op.second == CCP_INTHIS) // we've got an exact match
        {
            // Case 1: deletion from leaf node.
            if (node->isLeaf)
            {
                //assert(node->objCount >= _minDegree-1);
                node->delFromLeaf(op.first);

                //now node is dirty
                //node->setDirty(1);
                _setDirty(node);
                //_dirtyPages.push_back(node);
                ret = true;
            }
            // Case 2: Exact match on internal leaf.
            else
            {
                // Case 2a: prior child has enough elements to pull one out.
                node->loadChild(op.first, _dataFile);
                node->loadChild(op.first+1, _dataFile);
                if (node->children[op.first]->objCount >= _minDegree)
                {
                    sdb_node* childNode = node->loadChild(op.first, _dataFile);
                    SDBCursor locn = _findPred(childNode);

                    node->keys[op.first] = locn.first->keys[locn.second];
                    node->values[op.first] = locn.first->values[locn.second];

                    //now node is dirty
                    //node->setDirty(1);
                    _setDirty(node);
                    //_dirtyPages.push_back(node);

                    node = childNode;
                    key = locn.first->keys[locn.second];
                    goto L0;
                    //ret = _delete(childNode, dat.get_key());
                }

                // Case 2b: successor child has enough elements to pull one out.
                else if (node->children[op.first + 1]->objCount >= _minDegree)
                {
                    sdb_node* childNode = node->loadChild(op.first + 1,
                                                          _dataFile);
                    SDBCursor locn = _findSucc(childNode);

                    node->keys[op.first] = locn.first->keys[locn.second];
                    node->values[op.first] = locn.first->values[locn.second];

                    //now node is dirty
                    _setDirty(node);
                    //node->setDirty(1);
                    //_dirtyPages.push_back(node);

                    node = childNode;
                    key = locn.first->keys[locn.second];
                    goto L0;
                    //ret = _delete(childNode, dat.get_key());
                }

                // Case 2c: both children have only t-1 elements.
                // Merge the two children, putting the key into the
                // new child. Then delete from the new child.
                else
                {
                    assert(node->children[op.first]->objCount == _minDegree-1);
                    assert(node->children[op.first+1]->objCount == _minDegree-1);
                    sdb_node* mergedChild = _merge(node, op.first);
                    node = mergedChild;
                    goto L0;
                    //ret = _delete(mergedChild, key);
                }
            }
        }

        // Case 3: key is not in the internal node being examined,
        // but is in one of the children.
        else if (op.second == CCP_INLEFT || op.second == CCP_INRIGHT)
        {
            // Find out if the child tree containing the key
            // has enough elements. If so, we just recurse into
            // that child.

            node->loadChild(op.first, _dataFile);
            if (op.first+1<=node->objCount)
                node->loadChild(op.first+1, _dataFile);
            size_t keyChildPos = (op.second == CCP_INLEFT) ? op.first
                                 : op.first + 1;
            sdb_node* childNode = node->loadChild(keyChildPos, _dataFile);
            if (childNode->objCount >= _minDegree)
            {
                node = childNode;
                goto L0;
                //ret = _delete(childNode, key);
            }
            else
            {
                // Find out if the childNode has an immediate
                // sibling with _minDegree keys.
                sdb_node* leftSib = 0;
                sdb_node* rightSib = 0;
                size_t leftCount = 0;
                size_t rightCount = 0;
                if (keyChildPos> 0)
                {
                    leftSib = node->loadChild(keyChildPos - 1, _dataFile);
                    leftCount = leftSib->objCount;
                }
                if (keyChildPos < node->objCount)
                {
                    rightSib = node->loadChild(keyChildPos + 1, _dataFile);
                    rightCount = rightSib->objCount;
                }

                // Case 3a: There is a sibling with _minDegree or more keys.
                if (leftCount >= _minDegree || rightCount >= _minDegree)
                {
                    // Part of this process is making sure that the
                    // child node has minDegree elements.

                    childNode->setCount(_minDegree);

                    // Bringing the new key from the left sibling
                    if (leftCount >= _minDegree)
                    {
                        // Shuffle the keys and elements up
                        size_t i = _minDegree - 1;
                        for (; i> 0; i--)
                        {
                            childNode->keys[i] = childNode->keys[i - 1];
                            childNode->values[i] = childNode->values[i - 1];
                            childNode->children[i + 1] = childNode->children[i];
                            if (childNode->children[i + 1])
                            {
                                childNode->children[i + 1]->childNo = i + 1;
                            }
                        }
                        childNode->children[i + 1] = childNode->children[i];
                        if (childNode->children[i + 1])
                        {
                            childNode->children[i + 1]->childNo = i +1;
                        }

                        // Put the key from the parent into the empty space,
                        // pull the replacement key from the sibling, and
                        // move the appropriate child from the sibling to
                        // the target child.

                        childNode->keys[0] = node->keys[keyChildPos - 1];
                        childNode->values[0] = node->values[keyChildPos - 1];

                        node->keys[keyChildPos - 1]
                        = leftSib->keys[leftSib->objCount - 1];
                        node->values[keyChildPos - 1]
                        = leftSib->values[leftSib->objCount - 1];
                        if (!leftSib->isLeaf)
                        {
                            childNode->children[0]
                            = leftSib->children[leftSib->objCount];
                            childNode->children[0]->childNo = 0;
                            childNode->children[0]->parent = childNode;
                        }
                        leftSib->setCount(leftSib->objCount-1);
                        //--leftSib->objCount;
                        assert(leftSib->objCount >= _minDegree-1);

                        //now node is dirty
                        _setDirty(leftSib);
                        _setDirty(node);
                        //leftSib->setDirty(1);
                        //node->setDirty(1);
                        //_dirtyPages.push_back(leftSib);
                        //_dirtyPages.push_back(node);
                    }

                    // Bringing a new key in from the right sibling
                    else
                    {
                        // Put the key from the parent into the child,
                        // put the key from the sibling into the parent,
                        // and move the appropriate child from the
                        // sibling to the target child node.
                        childNode->keys[childNode->objCount - 1]
                        = node->keys[op.first];
                        childNode->values[childNode->objCount - 1]
                        = node->values[op.first];
                        node->keys[op.first] = rightSib->keys[0];
                        node->values[op.first] = rightSib->values[0];

                        if (!rightSib->isLeaf)
                        {
                            childNode->children[childNode->objCount]
                            = rightSib->children[0];

                            childNode->children[childNode->objCount]->childNo
                            = childNode->objCount;//wps add it!
                            childNode->children[childNode->objCount]->parent
                            = childNode;//wps add it!
                        }

                        // Now clean up the right node, shuffling keys
                        // and elements to the left and resizing.
                        size_t i = 0;
                        for (; i < rightSib->objCount - 1; i++)
                        {
                            rightSib->keys[i] = rightSib->keys[i + 1];
                            rightSib->values[i] = rightSib->values[i + 1];
                            if (!rightSib->isLeaf)
                            {
                                rightSib->children[i]
                                = rightSib->children[i + 1];

                                rightSib->children[i]->childNo = i;
                            }
                        }
                        if (!rightSib->isLeaf)
                        {
                            rightSib->children[i] = rightSib->children[i + 1];
                            rightSib->children[i]->childNo = i;
                        }
                        rightSib->setCount(rightSib->objCount - 1);
                        assert(rightSib->objCount >= _minDegree-1);

                        //now node is dirty
                        _setDirty(rightSib);
                        _setDirty(node);
                        //rightSib->setDirty(true);
                        //node->setDirty(1);
                    }
                    node = childNode;

                    _setDirty(node);
                    //node->setDirty(true);
                    goto L0;
                }

                // Case 3b: All siblings have _minDegree - 1 keys
                else
                {
                    assert(node->children[op.first]->objCount == _minDegree-1);
                    assert(node->children[op.first+1]->objCount == _minDegree-1);
                    sdb_node* mergedChild = _merge(node, op.first);
                    node = mergedChild;
                    goto L0;
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
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::open()
{

    if (_isOpen)
        return true;

    // We're creating if the file doesn't exist.

    struct stat statbuf;
    bool creating = stat(_fileName.c_str(), &statbuf);

    _dataFile = fopen(_fileName.c_str(), creating ? "w+b" : "r+b");
    if (0 == _dataFile)
    {
#ifdef DEBUG
        cout <<"SDB Error: open file failed, check if dat directory exists"
             <<endl;
#endif
        return false;
    }

    // Create a new node
    bool ret = false;
    if (creating)
    {

#ifdef DEBUG
        cout<<"creating sdb_btree: "<<_fileName<<"...\n"<<endl;
        _sfh.display();
#endif

        _sfh.toFile(_dataFile);
        //sdb_node::initialize(_sfh.pageSize, _sfh.maxKeys);

        // If creating, allocate a node instead of
        // reading one.
        _root = _allocateNode();
        _root->isLeaf = true;
        _root->isLoaded = true;

        commit();
        ret = true;

    }
    else
    {

        // when not creating, read the root node from the disk.
        memset(&_sfh, 0, sizeof(_sfh));
        _sfh.fromFile(_dataFile);
        if (_sfh.magic != 0x061561)
        {
            cout<<"Error, read wrong file header\n"<<endl;
            assert(false);
            return false;
        }
        if (_cacheSize != 0)
        {
            //reset cacheSize that has been set before open.
            //cacheSize is dynamic at runtime
            _sfh.cacheSize = _cacheSize;
        }
#ifdef DEBUG
        cout<<"open sdb_btree: "<<_fileName<<"...\n"<<endl;
        _sfh.display();
#endif

        _root = getRoot();
        //		_root = new sdb_node(_sfh, _fileLock, _activeNodeNum);
        //		_root->fpos = _sfh.rootPos;
        //		_root->read(_dataFile);
        ret = true;

    }
    _isOpen = true;
    return ret;
}

// This is the external delete function.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::del(const KeyType& key)
{

    if ( !_isOpen)
        return false;
    _flushCache();
    getRoot();
    // Determine if the root node is empty.
    bool ret = (_root->objCount != 0);

    // If we successfully deleted the key, and there
    // is nothing left in the root node and the root
    // node is not a leaf, we need to shrink the tree
    // by making the root's child (there should only
    // be one) the new root. Write the location of
    // the new root to the start of the file so we
    // know where to look
    if (_root->objCount == 0 && !_root->isLeaf)
    {
        sdb_node* node = _root;
        _root = _root->children[0];
        _root->parent = 0;
        node->unloadself();
        delete node;
        node = 0;
    }

    if (_root->objCount == (size_t) -1)
    {
        return 0;
    }

    // If our root is not empty, call the internal
    // delete method on it.
    ret = _delete(_root, key);

    if (ret)
        --_sfh.numItems;
    return ret;
}

// This method retrieves a record from the database
// given its location.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::get(const SDBCursor& locn, KeyType& key, ValueType& value)
{
    ScopedReadLock<LockType> lock(_flushLock);
    if ((sdb_node*)locn.first == 0 || locn.second == (size_t)-1 || locn.second
            >= locn.first->objCount)
    {
        return false;
    }
    key = locn.first->keys[locn.second];
    value = locn.first->values[locn.second];
    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::update(const KeyType& key, const ValueType& value)
{
    if ( !_isOpen)
        return false;
    SDBCursor locn(NULL, (size_t)-1);
    if (search(key, locn) )
    {
        locn.first->values[locn.second] = value;
        //locn.first->setDirty(true);
        _setDirty(locn.first);
        return true;
    }
    else
    {
        return insert(key, value);
    }
    return false;
}

// This method finds the record following the one at the
// location given as locn, and copies the record into rec.
// The direction can be either forward or backward.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::seq_(SDBCursor& locn, ESeqDirection sdir)
{
    if ( !_isOpen)
        return false;
    if (_sfh.numItems <=0)
    {
        return false;
    }
    getRoot();
    _root->parent = 0;
    //cout<<"seq _flushCache()"<<endl;
    //	_flushCache(locn);
    //	ScopedReadLock<LockType> lock(_flushLock);
    switch (sdir)
    {
    case ESD_FORWARD:
        return _seqNext(locn);
    case ESD_BACKWARD:
        return _seqPrev(locn);
    }

    return false;
}

// Find the next item in the database given a location. Return
// the subsequent item in rec.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_seqNext(SDBCursor& locn)
{
    // Set up a couple of convenience values

    bool ret = false;
    //if( !get(locn, key, value) )
    //	return false;

    sdb_node* node = locn.first;
    size_t lastPos = locn.second;
    bool goUp = false; // indicates whether or not we've exhausted a node.

    // If we are starting at the beginning, initialise
    // the locn reference and return with the value set.
    // This means we have to plunge into the depths of the
    // tree to find the first leaf node.
    if ((sdb_node*)node == 0)
    {
        node = _root;
        while ((sdb_node*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(0, _dataFile);
        }
        if ((sdb_node*)node == 0)
        {
            return false;
        }

        // pre-fetch all nested keys
        sdb_node* parent=node->parent;
        for (unsigned int i=1; i<=parent->objCount; i++)
            parent->loadChild(i, _dataFile);

        locn.first = node;
        locn.second = 0;
        return true;
    }

    // Advance the locn object to the next item

    // If we have a leaf node, we don't need to worry about
    // traversing into children ... only need to worry about
    // going back up the tree.
    if (node->isLeaf)
    {
        // didn't visit the last node last time.
        if (lastPos < node->objCount - 1)
        {
            locn.second = lastPos + 1;
            return true;
        }
        goUp = (lastPos == node->objCount - 1);
    }

    // Not a leaf, therefore need to worry about traversing
    // into child nodes.
    else
    {
        node = node->loadChild(lastPos + 1, _dataFile);//_cacheInsert(node);
        while ((sdb_node*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(0, _dataFile);
        }
        if ((sdb_node*)node == 0)
        {
            return false;
        }

        // pre-fetch all nested keys
        sdb_node* parent=node->parent;
        if (parent)
            for (unsigned int i=1; i<=parent->objCount; i++)
                parent->loadChild(i, _dataFile);

        locn.first = node;
        locn.second = 0;
        return true;
    }

    // Finished off a leaf, therefore need to go up to
    // a parent.
    if (goUp)
    {
        size_t childNo = node->childNo;
        node = node->parent;
        while ((sdb_node*)node != 0 && childNo >= node->objCount)
        {
            childNo = node->childNo;
            node = node->parent;
        }
        if ((sdb_node*)node != 0)
        {
            locn.first = node;
            locn.second = childNo;
            return true;
        }
    }
    //reach the last locn
    ++locn.second;
    return ret;
}

// Find the previous item in the database given a location. Return
// the item in rec.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::_seqPrev(SDBCursor& locn)
{
    // Set up a couple of convenience values
    bool ret = false;
    sdb_node* node = locn.first;
    size_t lastPos = locn.second;
    bool goUp = false; // indicates whether or not we've exhausted a node.


    // If we are starting at the end, initialise
    // the locn reference and return with the value set.
    // This means we have to plunge into the depths of the
    // tree to find the first leaf node.

    if ((sdb_node*)node == 0)
    {
        node = _root;
        while ((sdb_node*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(node->objCount, _dataFile);
        }
        if ((sdb_node*)node == 0)
        {
            return false;
        }
        locn.first = node;
        locn.second = node->objCount - 1;

        if (locn.second == size_t(-1) )
            return false;
        return true;
    }

    // Advance the locn object to the next item

    // If we have a leaf node, we don't need to worry about
    // traversing into children ... only need to worry about
    // going back up the tree.
    if (node->isLeaf)
    {
        // didn't visit the last node last time.
        if (lastPos> 0)
        {
            locn.second = lastPos - 1;
            return true;
        }
        goUp = (lastPos == 0);
    }

    // Not a leaf, therefore need to worry about traversing
    // into child nodes.
    else
    {
        node = node->loadChild(lastPos, _dataFile);
        while ((sdb_node*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(node->objCount, _dataFile);
        }
        if ((sdb_node*)node == 0)
        {

            return false;
        }
        locn.first = node;
        locn.second = node->objCount - 1;
        return true;
    }

    // Finished off a leaf, therefore need to go up to
    // a parent.
    if (goUp)
    {
        size_t childNo = node->childNo;
        node = node->parent;

        while ((sdb_node*)node != 0 && childNo == 0)
        {
            childNo = node->childNo;
            node = node->parent;
        }
        if ((sdb_node*)node != 0)
        {
            locn.first = node;
            locn.second = childNo - 1;
            return true;
        }
    }
    //reach the fist locn
    --locn.second = -1;
    return ret;
}

// This method flushes all loaded nodes to the file and
// then unloads the root node' children. So not only do we commit
// everything to file, we also free up most memory previously
// allocated.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_btree< KeyType, ValueType, LockType, fixed,
Alloc>::flush()
{

    //write back the fileHead and dirtypage
    commit();
    ScopedWriteLock<LockType> lock(_flushLock);
    if (_root)
    {
        delete _root;
        _root = 0;
    }
    // Unload each of the root's childrent.

    //	if (_root && !_root->isLeaf) {
    //		for (size_t i = 0; i < _root->objCount+1; i++) {
    //			sdb_node* pChild = _root->children[i];
    //			if ((sdb_node*)pChild != 0 && pChild->isLoaded) {
    //				_root->children[i]->unload();
    //			}
    //		}
    //	}
    return;
}

NS_IZENELIB_AM_END
#endif /*sdb_btree_H_*/
