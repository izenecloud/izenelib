/**
 * @file sdb_btree.h
 * @brief The header file of sdb_btree.
 *
 *
 * This file defines class sdb_btree.
 */
#ifndef sdb_bptree_H_
#define sdb_bptree_H_

#include "sdb_pnode.h"
#include "sdb_btree_types.h"
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
>class sdb_bptree
: public AccessMethod<KeyType, ValueType, LockType, Alloc>
{
    enum {unloadbyRss = false};
    enum {unloadAll = true};
    enum {unloadLeavesFirst = true};
    enum {loadIndexFirst = false};
    enum {orderedCommit =true};
    enum {quickFlush = false};
    public:
    typedef sdb_pnode_<KeyType, ValueType, LockType, fixed, Alloc> sdb_pnode;
    typedef std::pair<sdb_pnode*, size_t> SDBCursor;
    public:
    /**
     * \brief constructor
     *
     * \param fileName is the name for data file if fileName ends with '#', we set b-tree mode to
     *  not delay split.
     */
    sdb_bptree(const std::string& fileName = "sdb_bptree.dat#");
    virtual ~sdb_bptree();

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
    void setBtreeMode(bool delaySplit = true)
{
    _isDelaySplit = delaySplit;
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
    sdb_bptree other(fileName);
    if ( !other.open() )
        return false;
    return dump( other );
}

/**
 * 	 \brief del an item from the database
 *
 */
bool del(const KeyType& key)
{
    SDBCursor locn;
    if ( search(key, locn) )
    {
        if ( locn.first->delFromLeaf(locn.second) )
        {
            --_sfh.numItems;
            return true;
        }
    }
    return false;
}

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
    ScopedReadLock<LockType> lock(_flushLock);
    SDBCursor locn;
    if ( search_(key, locn) )
        return new ValueType(locn.first->values[locn.second]);
    return NULL;
}

bool get(const KeyType& key, ValueType& value)
{
    _safeFlushCache();
    ScopedReadLock<LockType> lock(_flushLock);
    SDBCursor locn;
    if ( search_(key, locn) )
    {
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
    sdb_pnode* node = getRoot();
    while (node && !node->isLeaf )
        node = node->loadChild(0, _dataFile);
    return SDBCursor(node, 0);
}

SDBCursor get_last_locn()
{
    ScopedReadLock<LockType> lock(_flushLock);
    sdb_pnode* node = getRoot();
    while (node && !node->isLeaf )
    {
        node = node->loadChild(node->objCount -1 ,_dataFile);
    }
    return SDBCursor(node, node->objCount-1);
}

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
void display(std::ostream& os = std::cout, bool onlyheader = true)
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

void optimize()
{
    string tempfile = _fileName+ ".swap";
    dump2f(tempfile);
    close();
    std::remove(_fileName.c_str() );
    std::rename(tempfile.c_str(), _fileName.c_str() );
    std::remove(tempfile.c_str());
    open();
}

sdb_pnode* getRoot()
{
    if ( _root == NULL )
    {
        _root = new sdb_pnode(_sfh, _fileLock, _activeNodeNum);
        _root->fpos = _sfh.rootPos;
        _root->read(_dataFile);
    }
    return _root;
}

void fillCache()
{
    queue<sdb_pnode*> qnode;
    qnode.push( getRoot() );

    while ( !qnode.empty() )
    {
        sdb_pnode* popNode = qnode.front();
        qnode.pop();
        if (popNode && !popNode->isLeaf && popNode->isLoaded )
        {
            for (size_t i=0; i< popNode->objCount; i++)
            {
                if ( _activeNodeNum> _sfh.cacheSize )
                    goto LABEL;
                sdb_pnode* node = popNode->loadChild(i, _dataFile);
                if ( node )
                    qnode.push( node );

            }
        }
    }
LABEL:
    return;
}

private:
sdb_pnode* _root;
FILE* _dataFile;
CbFileHeader _sfh;
size_t _cacheSize;

bool _isDelaySplit;
bool _isOpen;

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

inline void _setDirty(sdb_pnode* node)
{
    if ( !node->isDirty )
    {
        ++_dirtyPageNum;
        node->setDirty(true);
    }
}

void _safeFlushCache()
{
    getRoot();
    ++_flushCount;
    if (unloadLeavesFirst)
    {
        if ( _activeNodeNum> _sfh.cacheSize )
        {
            ScopedWriteLock<LockType> lock(_flushLock);
            _flushLeaves();
        }
    }
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        ScopedWriteLock<LockType> lock(_flushLock);
        _flushCacheImpl(quickFlush);
    }
}

void _flushCache()
{
    getRoot();
    if (unloadLeavesFirst)
    {
        if ( _activeNodeNum> _sfh.cacheSize )
        {
            _flushLeaves();
        }
    }
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        _flushCacheImpl(quickFlush);
    }

}

//for seq, reset SDBCursor
void _flushCache(SDBCursor& locn)
{
    getRoot();
    if (unloadLeavesFirst)
    {
        if ( _activeNodeNum> _sfh.cacheSize )
        {
            ScopedWriteLock<LockType> lock(_flushLock);
            _flushLeaves();
        }
    }
    if ( _activeNodeNum> _sfh.cacheSize )
    {
        KeyType key;
        ValueType value;
        get(locn, key, value);
        {
            ScopedWriteLock<LockType> lock(_flushLock);
            _flushCacheImpl();
        }
        search(key, locn);
    }

}

void _flushLeaves()
{
#ifdef  DEBUG
    cout<<"\nbefore _flushLeaves..."<<endl;
    cout<<"activeNum: "<<_activeNodeNum<<endl;
    cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
    izenelib::util::ClockTimer timer;
    //display();
#endif

    commit();
    typedef map<long, sdb_pnode*> COMMIT_MAP;
    typedef typename COMMIT_MAP::iterator CMIT;

    queue<sdb_pnode*> qnode;
    COMMIT_MAP toBeWrited;

    qnode.push(_root);
    while ( !qnode.empty() )
    {
        sdb_pnode* popNode = qnode.front();
        qnode.pop();
        if ( popNode->isLeaf)
        {
            toBeWrited.insert(make_pair(popNode->fpos, popNode) );
        }
        if (popNode && popNode->isLoaded && !popNode->isLeaf)
        {
            for (size_t i=0; i<popNode->objCount; i++)
            {
                if ( popNode->children[i] )
                {
                    qnode.push( popNode->children[i] );
                }

            }
        }
    }

    CMIT it = toBeWrited.begin();
    for (; it != toBeWrited.end(); it++)
    {
        it->second->unload();
    }

#ifdef DEBUG
    printf("unload leaves elapsed 1 ( actually ): %lf seconds\n",
           timer.elapsed() );
    cout<<"\n\nstop unload leaves..."<<endl;
    cout<<_activeNodeNum<<" vs "<<_sfh.cacheSize <<endl;
    cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;

#endif

}

void _loadIndex()
{
    queue<sdb_pnode*> qnode;
    qnode.push(_root);

    while ( !qnode.empty() )
    {
        sdb_pnode* popNode = qnode.front();
        qnode.pop();
        if ( _activeNodeNum> _sfh.cacheSize )
            break;
        if (popNode && popNode->isLoaded && !popNode->isLeaf)
        {
            for (size_t i=0; i<popNode->objCount; i++)
            {
                popNode->loadChild(i, _dataFile);
                if ( !popNode->children[i]->isLeaf )
                {
                    qnode.push( popNode->children[i] );
                }
                else
                {
                    goto LABEL;
                }

            }
        }

    }
LABEL:
    return;

}

void _flushCacheImpl(bool quickFlush=false)
{

#ifdef  DEBUG
    cout<<"\n\ncache is full..."<<endl;
    cout<<"activeNum: "<<_activeNodeNum<<endl;
    cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
    izenelib::util::ClockTimer timer;
    //display();
#endif

    if ( !quickFlush )
        commit();

#ifdef DEBUG
    printf("commit elapsed 1 ( actually ): %lf seconds\n",
           timer.elapsed() );
#endif
    if (unloadAll)
    {
        for (size_t i=0; i<_root->objCount; i++)
        {
            _root->children[i]->unload();
        }
        _activeNodeNum = 1;
#ifdef DEBUG
        cout<<"\n\nstop unload..."<<endl;
        cout<<_activeNodeNum<<" vs "<<_sfh.cacheSize <<endl;
        cout<<"dirtyPageNum: "<<_dirtyPageNum<<endl;
#endif
        return;
    }
    else
    {

        queue<sdb_pnode*> qnode;
        qnode.push(_root);

        size_t popNum = 0;
        size_t escapeNum = _activeNodeNum>>1;
        sdb_pnode* interval = NULL;
        while ( !qnode.empty() )
        {
            sdb_pnode* popNode = qnode.front();
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
                for (size_t i=0; i<popNode->objCount; i++)
                {
                    if ( popNode->children[i] )
                    {
                        qnode.push( popNode->children[i] );
                    }
                    else
                    {
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

sdb_pnode* _allocateNode(bool isLeaf=false)
{

    sdb_pnode* newNode;

    newNode = new sdb_pnode(_sfh, _fileLock, _activeNodeNum);
    newNode->isLoaded = true;
    newNode->isDirty = true;
    newNode->fpos = SDB_FILE_HEAD_SIZE + _sfh.pageSize
                    *(_sfh.nPages+_sfh.oPages);

    //cout<<"allocate idx="<<CbFileHeader::nPages<<" "<<newNode->fpos;
    ++_sfh.nPages;
    ++_dirtyPageNum;
    ++_activeNodeNum;

    //pre allocate memory for newNode for efficiency
    newNode->keys.resize(_sfh.maxKeys);
    if ( isLeaf )
        newNode->values.resize(_sfh.maxKeys);
    else
    {
        newNode->children.resize(_sfh.maxKeys+1);
    }

    return newNode;
}

void _split(sdb_pnode* parent, size_t childNum, sdb_pnode* child);
void _split3Leaf(sdb_pnode* parent, size_t childNum);
sdb_pnode* _merge(sdb_pnode* &parent, size_t objNo);

bool _seqNext(SDBCursor& locn);
bool _seqPrev(SDBCursor& locn);
void _flush(sdb_pnode* node, FILE* f);
bool _insert(sdb_pnode* node, const KeyType& key, const ValueType& val);
bool _delete(sdb_pnode* node, const KeyType& key);
bool _delete1(sdb_pnode* node, const KeyType& key);

// Finds the location of the predecessor of this key, given
// the root of the subtree to search. The predecessor is going
// to be the right-most object in the right-most leaf node.
SDBCursor _findPred(sdb_pnode* node)
{
    //assert(false);
    sdb_pnode* child = node;
    while (!child->isLeaf)
    {
        child = child->loadChild(child->objCount-1, _dataFile);
    }
    assert(child->objCount - 2 >= 0 );
    return SDBCursor(child, child->objCount-2 );
}

// Finds the location of the successor of this key, given
// the root of the subtree to search. The successor is the
// left-most object in the left-most leaf node.
SDBCursor _findSucc(sdb_pnode* node)
{
    sdb_pnode* child = node;
    while (!child->isLeaf)
    {
        child = child->loadChild(0, _dataFile);
    }
    return SDBCursor(child, 0);;
}

void optimize_()
{
    commit();
    double ofactor = double(_sfh.oPages)/double(_sfh.nPages)+1;
    int pfactor = int(ofactor);
    //auto adapt cache size.
    setCacheSize( _sfh.cacheSize/pfactor );
}

};

// The constructor simply sets up the different data members
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_bptree< KeyType, ValueType, LockType, fixed, Alloc>::sdb_bptree(
    const std::string& fileName)
{
    _root = 0;
    _isDelaySplit = true;
    //_isDelaySplit = false;
    _fileName = fileName;
    //_sfh.pageSize = 1024;
    //_sfh.maxKeys = 64;
    //_sfh.cacheSize = 1024*64;

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

    if (unloadbyRss)
    {
        unsigned long vm = 0;
        ProcMemInfo::getProcMemInfo(vm, _initRss);
    }
    _flushCount = 0;

}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_bptree< KeyType, ValueType, LockType, fixed, Alloc>::~sdb_bptree()
{
    close();
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::search_(const KeyType& key, SDBCursor& locn)
{
    if ( !_isOpen)
        return false;
    //do Flush, when cache is full
    //_flushCache();

    locn.first = 0;
    locn.second = (size_t)-1;

    sdb_pnode* temp = _root;
    while (1)
    {
        int bound = temp->objCount;
        int low = 0;
        int high = bound-1;
        int compVal;
        while (low <= high)
        {
            int mid = (low+high)/2;
            compVal = _comp(key, temp->keys[mid]);
            if (compVal == 0)
            {
                if ( !temp->isLeaf)
                    temp = temp->loadChild(mid, _dataFile);
                while ( !temp->isLeaf)
                {
                    temp = temp->loadChild(temp->objCount-1, _dataFile);
                }
                assert(temp->isLeaf);
                size_t pos;
                locn.first = temp;
                if (temp->find(key, pos) )
                {
                    locn.second = pos;
                    return true;
                }
                return false;
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
            if (low >= bound)
            {
                low = bound - 1;
            }
            temp = temp->loadChild(low, _dataFile);
        }
        else
        {
            locn.first = temp;
            locn.second = low;
            if (low >= (int)temp->objCount)
                seq_(locn);
            break;
        }
    }
    return false;
}

// Splits a child node, creating a new node. The median value from the
// full child is moved into the *non-full* parent. The keys above the
// median are moved from the full child to the new child.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_split(sdb_pnode* parent, size_t childNum, sdb_pnode* child)
{

    //display();
    size_t i = 0;
    size_t leftCount = (child->objCount)>>1;
    size_t rightCount = child->objCount-leftCount;
    bool isLeaf = child->isLeaf;

    sdb_pnode* newChild = _allocateNode(isLeaf);

    newChild->childNo = childNum+1;
    newChild->isLeaf = isLeaf;
    newChild->setCount(rightCount);

    // Put the high values in the new child, then shrink the existing child.
    for (i = 0; i < rightCount; i++)
    {
        newChild->keys[i] = child->keys[leftCount+i ];
        if (isLeaf)
            newChild->values[i] = child->values[leftCount+i];
        else
        {
            newChild->children[i] = child->children[leftCount+i];
        }
    }

    KeyType savekey = child->keys[leftCount-1];
    child->setCount(leftCount);

    if (parent ->objCount != 0)
    {
        // Move the child pointers above childNum up in the parent
        parent->setCount(parent->objCount + 1);
        for (i = parent->objCount-1; i> childNum + 1; i--)
        {
            parent->children[i] = parent->children[i - 1];
            parent->children[i]->childNo = i;
        }
        parent->children[childNum + 1] = newChild;
        newChild->childNo = childNum + 1;
        newChild->parent = parent;

        for (i = parent->objCount - 1; i> childNum; i--)
        {
            parent->keys[i] = parent->keys[i - 1];
        }
        parent->keys[childNum] = savekey;
    }
    else
    {
        parent->setCount( 2);
        parent->keys[0] = child->keys[child->objCount - 1];
        parent->keys[1] = newChild->keys[newChild->objCount - 1];
        parent->children[1] = newChild;

    }
    _setDirty(child);
    _setDirty(newChild);
    _setDirty(parent);
}
//split two full leaf nodes into tree 2/3 ful nodes.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_split3Leaf(sdb_pnode* parent, size_t childNum)
{

    size_t i = 0;
    size_t count1 = (_sfh.maxKeys<<1)/3;
    size_t count2 = _sfh.maxKeys - _sfh.maxKeys/3;
    size_t count3 = (_sfh.maxKeys<<1) -count1 -count2;

    sdb_pnode* child1 = parent->loadChild(childNum, _dataFile);
    sdb_pnode* child2 = parent->loadChild(childNum+1, _dataFile);

    sdb_pnode* newChild = _allocateNode(true);
    //swap fpos of newChild and child2
    long tempfpos = child2->fpos;
    child2->fpos = newChild->fpos;
    newChild->fpos = tempfpos;

    newChild->isLeaf =true;
    newChild->setCount(count3);
    newChild->parent = parent;

    KeyType tkey1 = child1->keys[count1-1];

    // Put the high values in the new child, then shrink the existing child.
    for (i = 0; i < _sfh.maxKeys - count1; i++)
    {
        newChild->keys[i] = child1->keys[count1+i];
        newChild->values[i] = child1->values[count1+i];
    }

    for (i = _sfh.maxKeys - count1; i < count3; i++)
    {
        newChild->keys[i] = child2->keys[i-_sfh.maxKeys+count1];
        newChild->values[i] = child2->values[i-_sfh.maxKeys+count1];
    }

    for (i=0; i<count2; i++)
    {
        child2->keys[i] = child2->keys[_sfh.maxKeys/3+i];
        child2->values[i] = child2->values[_sfh.maxKeys/3+i];
    }

    child1->setCount(count1);
    child2->setCount(count2);

    KeyType tkey2 = newChild->keys[count3-1];

    // Move the child pointers above childNum up in the parent
    parent->setCount(parent->objCount+1);
    parent->keys[childNum] = tkey1;

    for (i = parent->objCount-1; i>= childNum + 2; i--)
    {
        parent->children[i] = parent->children[i - 1];
        parent->children[i]->childNo = i;
    }
    parent->children[childNum + 1] = newChild;
    newChild->childNo = childNum + 1;

    for (i = parent->objCount-1; i> childNum+1; i--)
    {
        parent->keys[i] = parent->keys[i - 1];
    }
    parent->keys[childNum+1] = tkey2;

    _setDirty(child1);
    _setDirty(child2);
    _setDirty(newChild);
    _setDirty(parent);

}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::insert(const KeyType& key, const ValueType& value)
{
    if ( !_isOpen)
        return false;
    if (_sfh.numItems == 1<<16)
        optimize_();
    _flushCache();
    if (_root->objCount >= _sfh.maxKeys)
    {
        // Growing the tree happens by creating a new
        // node as the new root, and splitting the
        // old root into a pair of children.
        sdb_pnode* oldRoot = _root;

        _root = _allocateNode(false);
        _root->setCount(0);
        _root->isLeaf = false;
        _root->children[0] = oldRoot;
        oldRoot->childNo = 0;
        oldRoot->parent = _root;
        _split(_root, 0, oldRoot);
    }
    bool ret = _insert(_root, key, value);
    if (ret)
        ++_sfh.numItems;
    return ret;

}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_insert(sdb_pnode* nd, const KeyType& key,
                const ValueType& value)
{
    /*if (_root->objCount >= _sfh.maxKeys) {
     // Growing the tree happens by creating a new
     // node as the new root, and splitting the
     // old root into a pair of children.
     sdb_pnode* oldRoot = _root;

     _root = _allocateNode(false);
     _root->setCount(0);
     _root->isLeaf = false;
     _root->children[0] = oldRoot;
     oldRoot->childNo = 0;
     oldRoot->parent = _root;
     _split(_root, 0, oldRoot);
     goto L0;
     } else */
    {

        sdb_pnode* node = nd;

L1:
        register size_t i = node->objCount;
        //	node->display();
        register int low = 0;
        register int high = i-1;
        register int mid;
        register int compVal;
        while (low<=high)
        {
            mid = (low+high)>>1;
            compVal = _comp(key, node->keys[mid]);
            if (compVal == 0)
            {
                if (node->isLeaf)
                    return false;
                low = mid;
                break;
                //return false;
            }
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
            return true;
        }

        // If the node is an internal node, we need to find
        // the location to insert the value ...
        else
        {
            // Load the child into which the value will be inserted.

            //if the inserted keys is bigger than any one)
            if (low == (int)node->objCount )
            {
                node->keys[low -1 ] = key;
                low = low -1;
            }

            sdb_pnode* child = node->loadChild(low, _dataFile);

            //If the child node is full , we will insert into its adjacent nodes, and if bothe are
            //are full, we will split the two node to three nodes.
            if (child->objCount >= _sfh.maxKeys)
            {
                //_isDelaySplit = false;
                if ( !child->isLeaf || !_isDelaySplit)
                {
                    _split(node, low, child);
                    compVal = _comp(key, node->keys[low]);
                    //if (compVal == 0){
                    //	return false;
                    //}
                    if (compVal> 0)
                    {
                        ++low;
                    }
                    child = node->loadChild(low, _dataFile);
                }
                else
                {
                    sdb_pnode* adjNode;
                    int splitNum = low;
                    if ((size_t)low < node->objCount - 1)
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
                            adjNode->keys[0] = child->keys[child->objCount-1];
                            adjNode->values[0]
                            = child->values[child->objCount-1];
                            _setDirty(adjNode);
                            _setDirty(node);

                            //case: all of the keys in child are less than inserting keys.
                            if (_comp(child->keys[child->objCount-2], key)<0)
                            {
                                node->keys[low] = key;
                            }
                            else   //case: insert the item into the new child.
                            {
                                node->keys[low]
                                = child->keys[child->objCount-2];
                            }
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
                        //adjNode->display();
                        //case: left sibling is no full
                        if (adjNode->objCount < _sfh.maxKeys)
                        {
                            //cacheL child's first key equal inserting key,do nothing
                            if (_comp(child->keys[0], key) == 0)
                            {
                                return false;
                            }
                            _setDirty(adjNode);
                            _setDirty(node);
                            //case: all of the keys in child are bigger than inserting keys.
                            if (_comp(key, child->keys[0])< 0)
                            {
                                node->keys[low-1] = key;
                                size_t objCount = adjNode->objCount;
                                adjNode->setCount(objCount+1);
                                adjNode->keys[objCount] = key;
                                adjNode->values[objCount] = value;
                                _setDirty(adjNode);
                                //++_sfh.numItems;
                                return true;
                            }
                            else
                            {

                                node->keys[low-1] = child->keys[0];
                                size_t objCount = adjNode->objCount;
                                assert(adjNode->isLeaf == true);
                                adjNode->setCount(objCount+1);
                                adjNode->keys[objCount] = child->keys[0];
                                adjNode->values[objCount] = child->values[0];

                                size_t pos;
                                bool found = child->find(key, pos);
                                assert(pos != 0);
                                _setDirty(child);
                                if (found)
                                    pos = child->objCount;
                                for (size_t j=1; j<pos; j++)
                                {
                                    child->keys[j-1] = child->keys[j];
                                    child->values[j-1] = child->values[j];
                                }
                                if (found)
                                {
                                    child->setCount(child->objCount-1);
                                    return false;
                                }
                                child->keys[pos-1] = key;
                                child->values[pos-1] = value;
                                return true;
                            }
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
typename Alloc> void sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_flush(sdb_pnode* node, FILE* f)
{

    // Bug out if the file is not valid
    if (!f)
    {
        return;
    }
    if (orderedCommit)
    {
        typedef map<long, sdb_pnode*> COMMIT_MAP;
        typedef typename COMMIT_MAP::iterator CMIT;
        COMMIT_MAP toBeWrited;
        queue<sdb_pnode*> qnode;
        qnode.push(node);
        while (!qnode.empty() )
        {
            sdb_pnode* popNode = qnode.front();
            if (popNode->isLoaded && popNode->isDirty)
                toBeWrited.insert(make_pair(popNode->fpos, popNode) );
            qnode.pop();
            if (popNode && !popNode->isLeaf)
            {
                for (size_t i=0; i<popNode->objCount; i++)
                {
                    if (popNode->children[i])
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

        queue<sdb_pnode*> qnode;
        qnode.push(node);
        while (!qnode.empty())
        {
            sdb_pnode* popNode = qnode.front();
            if (popNode && popNode->isLoaded)
            {
                if (popNode->write(f) )
                    --_dirtyPageNum;
            }
            qnode.pop();
            if (popNode && !popNode->isLeaf)
            {
                for (size_t i=0; i<popNode->objCount; i++)
                {
                    if (popNode->children[i])
                        qnode.push(popNode->children[i]);
                }
            }
        }

    }

}

// Opening the database means that we check the file
// and see if it exists. If it doesn't exist, start a database
// from scratch. If it does exist, load the root node into
// memory.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
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
        cout<<"creating sdb_bptree: "<<_fileName<<"...\n"<<endl;
        _sfh.display();
#endif
        //sdb_pnode::initialize(_sfh.pageSize, _sfh.maxKeys);

        // If creating, allocate a node instead of
        // reading one.
        _root = _allocateNode(true);
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
        cout<<"open sdb_bptree: "<<_fileName<<"...\n"<<endl;
        _sfh.display();
#endif

        _root = getRoot();
        //		_root = new sdb_pnode(_sfh, _fileLock, _activeNodeNum);
        //		_root->fpos = _sfh.rootPos;
        //		_root->read(_dataFile);
        if (loadIndexFirst)
            _loadIndex();
        ret = true;
        //display();
    }
    _isOpen = true;
    return ret;
}

// This method retrieves a record from the database
// given its location.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::get(const SDBCursor& locn, KeyType& key, ValueType& value)
{
    ScopedReadLock<LockType> lock(_flushLock);
    if ((sdb_pnode*)locn.first == 0 || locn.second == (size_t)-1 || locn.second
            >= locn.first->objCount || !(locn.first->isLeaf))
    {
        return false;
    }
    assert(locn.first->isLeaf);
    key = locn.first->keys[locn.second];
    value = locn.first->values[locn.second];
    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
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
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
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
    //_flushCache(locn);
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
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_seqNext(SDBCursor& locn)
{
    // Set up a couple of convenience values

    bool ret = false;
    //if( !get(locn, key, value) )
    //	return false;

    sdb_pnode* node = locn.first;
    size_t lastPos = locn.second;
    assert(node->isLeaf);
    bool goUp = false; // indicates whether or not we've exhausted a node.

    // If we are starting at the beginning, initialise
    // the locn reference and return with the value set.
    // This means we have to plunge into the depths of the
    // tree to find the first leaf node.
    if ((sdb_pnode*)node == 0)
    {
        node = _root;
        while ((sdb_pnode*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(0, _dataFile);
        }
        if ((sdb_pnode*)node == 0)
        {
            return false;
        }
        locn.first = node;
        locn.second = 0;
        if (locn.second == 0)
        {
            sdb_pnode* parent=node->parent;
            for (unsigned int i=1; i<parent->objCount; i++)
                parent->loadChild(i, _dataFile);
        }
        return true;
    }

    // Advance the locn object to the next item

    // If we have a leaf node, we don't need to worry about
    // traversing into children ... only need to worry about
    // going back up the tree.
    if (node->isLeaf)
    {
        // didn't visit the last node last time.
        if (locn.second == 0)
        {
            sdb_pnode* parent=node->parent;
            if (parent)
                for (unsigned int i=1; i<parent->objCount; i++)
                    parent->loadChild(i, _dataFile);
        }

        if (lastPos < node->objCount - 1)
        {
            locn.second = lastPos + 1;
            return true;
        }
        goUp = (lastPos == node->objCount - 1);
    }

    // Finished off a leaf, therefore need to go up to
    // a parent.
    if (goUp)
    {
        size_t childNo = node->childNo;
        node = node->parent;
        while ((sdb_pnode*)node != 0 && childNo >= node->objCount-1)
        {
            childNo = node->childNo;
            node = node->parent;
        }
        if ((sdb_pnode*)node == 0)
        {
            ++locn.second;
            return false;
        }

        node = node->loadChild(childNo + 1, _dataFile);
        while ((sdb_pnode*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(0, _dataFile);
        }
        locn.first = node;
        locn.second = 0;
        return true;

    }
    //reach the last locn
    ++locn.second;
    return ret;
}

// Find the previous item in the database given a location. Return
// the item in rec.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::_seqPrev(SDBCursor& locn)
{
    // Set up a couple of convenience values
    bool ret = false;
    sdb_pnode* node = locn.first;
    size_t lastPos = locn.second;
    bool goUp = false; // indicates whether or not we've exhausted a node.


    // If we are starting at the end, initialise
    // the locn reference and return with the value set.
    // This means we have to plunge into the depths of the
    // tree to find the first leaf node.

    if ((sdb_pnode*)node == 0)
    {
        node = _root;
        while ((sdb_pnode*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(node->objCount-1, _dataFile);
        }
        if ((sdb_pnode*)node == 0)
        {
            return false;
        }
        locn.first = node;
        locn.second = node->objCount-1;
        if (locn.second == node->objCount-1)
        {
            sdb_pnode* parent=node->parent;
            for (unsigned int i=1; i<parent->objCount; i++)
                parent->loadChild(i, _dataFile);
        }
        return true;
    }

    // Advance the locn object to the next item

    // If we have a leaf node, we don't need to worry about
    // traversing into children ... only need to worry about
    // going back up the tree.
    if (node->isLeaf)
    {
        // didn't visit the last node last time.
        if (locn.second == node->objCount-1)
        {
            sdb_pnode* parent=node->parent;
            for (unsigned int i=1; i<parent->objCount; i++)
                parent->loadChild(i, _dataFile);
        }
        if (lastPos> 0)
        {
            locn.second = lastPos - 1;
            return true;
        }
        goUp = (lastPos == 0);
    }

    // Finished off a leaf, therefore need to go up to
    // a parent.
    if (goUp)
    {
        size_t childNo = node->childNo;
        node = node->parent;

        while ((sdb_pnode*)node != 0 && childNo == 0)
        {
            childNo = node->childNo;
            node = node->parent;
        }
        if ((sdb_pnode*)node == 0)
        {
            locn.second = -1;
            return false;
        }
        assert(childNo >= 1);
        node = node->loadChild(childNo-1, _dataFile);
        while ((sdb_pnode*)node != 0 && !node->isLeaf)
        {
            node = node->loadChild(node->objCount-1, _dataFile);
        }
        locn.first = node;
        locn.second = node->objCount - 1;
        return true;

    }
    //reach the fist locn
    locn.second = -1;
    return ret;
}

// This method flushes all loaded nodes to the file and
// then unloads the root node' children. So not only do we commit
// everything to file, we also free up most memory previously
// allocated.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_bptree< KeyType, ValueType, LockType, fixed,
Alloc>::flush()
{

    //write back the fileHead and dirtypage
    commit();
    //	if (_root) {
    //		delete _root;
    //		_root = 0;
    //	}

    ScopedWriteLock<LockType> lock(_flushLock);

    // Unload each of the root's childrent.
    if (_root && !_root->isLeaf)
    {
        for (size_t i = 0; i < _root->objCount; i++)
        {
            sdb_pnode* pChild = _root->children[i];
            if ((sdb_pnode*)pChild != 0 && pChild->isLoaded)
            {
                _root->children[i]->unload();
            }
        }
    }

    return;
}

NS_IZENELIB_AM_END
#endif /*sdb_bptree_H_*/
