/**
 * @file sdb_node.h
 * @brief The header file of sdb_node.
 *
 * This file defines class sdb_node.
 */

#ifndef sdb_pnode_H_
#define sdb_pnode_H_

#include "sdb_btree_types.h"
#include "sdb_btree_header.h"
#include <vector>

NS_IZENELIB_AM_BEGIN

/**
 *
 * \brief  sdb_node represents a node(internal node or leaf node)
 * in the B tree structure.
 *
 *
 * SDB/btree is made up of a collection of n
 odes. A node
 * contains information about which of its parent's children
 * is, how many objects it has, whether or not it's a leaf,
 * where it lives on the disk, whether or not it's actually
 * been loaded from the disk, a collection of records(key/value pair),
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
 *  |DbObj/key | ----- DbObj/value |
 *  +------------+--------+--------+
 *
 *  where DbObj wraps the content of key or value in form of
 *
 *  @DbObj
 *  +-------------+
 *  | size | char*|
 *  +-------------+
 *
 */

template<typename KeyType, typename ValueType, typename LockType,
bool fixed = false,
typename Alloc=std::allocator<DataType<KeyType,ValueType> > > class sdb_pnode_
{
    typedef std::pair<sdb_pnode_*, size_t> NodeKeyLocn;
    typedef typename std::vector<KeyType>::iterator KIT;
public:
    /**
     * \brief constructor
     *
     *  Constructor initialises everything to its default value.
     * Not that we assume that the node is a leaf,
     * and it is not loaded from the disk.
     */
    sdb_pnode_(CbFileHeader& fileHeader, LockType& fileLock, size_t& activeNum);

    ~sdb_pnode_()
    {
        unload();
        //	cout<<"~~~sdb_pnode_ isLeaf:"<<isLeaf<<endl;
    }

    /**
     * 	\brief when we want to access to node, we should load it to memory from disk.
     *
     *  Load a child node from the disk. This requires that we
     *  have the filepos already in place.
     */
    inline sdb_pnode_* loadChild(size_t childNum, FILE* f);

    /**
     * \brief delete  all its children and release self memory
     */
    void unload();

    /**
     *  \brief release most self memory without unloading is child, used for sdb_btree/_merge method.
     */
    void unloadself();

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
    inline bool delFromLeaf(size_t objNo);

    bool find(const KeyType& key, size_t & pos)
    {
        if (objCount == 0)
        {
            pos = size_t(-1);
            return false;
        }
        size_t low = 0;
        size_t high = objCount-1;
        int compVal;
        register size_t mid = 0;
        while (low <= high)
        {
            mid = (low+high)/2;
            compVal = _comp(key, keys[mid]);
            if (compVal == 0 )
            {
                pos = mid;
                return true;
            }
            else if (compVal < 0)
                high = mid-1;
            else
            {
                low = mid+1;
            }
        }
        pos = low;
        return false;

    }

    /**
     * \brief
     *
     * We need to change the number of Objects and children, when we split a
     * 	node or merge two nodes.
     */
    inline void setCount(size_t newSize)
    {
        objCount = newSize;

        //keys.resize(newSize);
        //values.resize(newSize);
        //children.resize(newSize+1);
    }
    /**
     *  \brief it marks this node is dirty
     *
     *  only dirty page will be write back to disk.
     */
    void setDirty(bool is)
    {
        isDirty = is;
    }

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
    void display(std::ostream& os = std::cout)
    {

        size_t i;
        for (i=0; i<objCount; i++)
        {
            if ( !isLeaf)
            {
                if (children[i])
                    children[i]->display(os);
                os<<"----|";
            }
            //keys[i]->display();
            //if(isLeaf)
            //os<<keys[i]<<"->"<<values[i];
            //else
            //os<<keys[i]<<"->"<<"NULL";
            //os<<keys[i];
            size_t pfos=0;
            //if (parent)
            //	pfos = parent->fpos;
            os<<"("<<fpos<<" parent="<<pfos<<" isLeaf="<<isLeaf<<" childNo="
            <<childNo<<" objCount="<<objCount<<" isLoaded="<<isLoaded
            <<")"<<endl;
            //os<<"("<<isDirty<<" "<<parent<<" "<<this<<")";
            os<<endl;
        }
        if (!isLeaf)
        {
            if (i<objCount && children[i])
                children[i]->display(os);
            os<<"----|";
        }
    }

public:
    size_t objCount; //the number of the objects, and if the node is not leafnode,
    long fpos; //its streamoff in file.
    bool isLeaf; //leafNode of internal node, node that int disk we put the data beside the key.
    bool isLoaded; //if it is loaded from disk.
    int isDirty;
    size_t childNo; //from 0 to objCount. Default is size_t(-1).
    sdb_pnode_* parent;

    std::vector<KeyType> keys;
    std::vector<sdb_pnode_*> children; //it has objCount+1 childrens.
    std::vector<ValueType> values;
public:
    // size_t activeNodeNum;//It is used for cache mechanism.

private:
    CbFileHeader& _fh;
    size_t& _maxKeys;
    size_t& _pageSize;
    size_t& _overFlowSize;
    LockType &_fileLock; //inclusive lock for I/O
    size_t &activeNodeNum;
    static izenelib::am::CompareFunctor<KeyType> _comp;
private:
    //when overflowing occured, we will allocate serverall sequential
    //overflow page at the end of file.
    long _overflowAddress;
    size_t _overflowPageCount;
};

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> CompareFunctor<KeyType> sdb_pnode_< KeyType, ValueType,
LockType, fixed, Alloc>::_comp;

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_pnode_< KeyType, ValueType, LockType, fixed, Alloc>::sdb_pnode_(
    CbFileHeader& fileHeader, LockType& fileLock, size_t& activeNum) :
        objCount(0), fpos(-1), isLeaf(true), isLoaded(false), isDirty(1),
        childNo((size_t)-1), _fh(fileHeader), _maxKeys(_fh.maxKeys),
        _pageSize(_fh.pageSize), _overFlowSize(_fh.pageSize),
        _fileLock(fileLock), activeNodeNum(activeNum)
{
    _overflowAddress = -1;
    _overflowPageCount = 0;

    //activeNodeNum++;
    //cout<<"activeNodeNum: "<<activeNodeNum<<endl;
}

// Read a node page to initialize this node from the disk
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_pnode_< KeyType, ValueType, LockType, fixed,
Alloc>::read(FILE* f)
{

    //#ifdef DEBUG
    //	static int _rcount;
    //	cout<<"reading "<<_rcount++<<endl;
    //#endif

    if (!f)
    {
        return false;
    }

    ScopedWriteLock<LockType> lock(_fileLock);
    if (isLoaded)
    {
        return true;
    }

    //cout<<"read from fpos "<<fpos<<endl;

    // get to the right location
    if (0 != fseek(f, fpos, SEEK_SET))
    {
        return false;
    }

    //keys.resize(_fh.maxKeys);

    char *pBuf = new char[_pageSize];
    if (1 != fread(pBuf, _pageSize, 1, f))
    {
        if (pBuf)
            delete pBuf;
        pBuf = 0;
        return false;
    }

    char *p = pBuf;
    size_t tsz = 0;

    // read the leaf flag and the object count
    byte leafFlag = 0;
    memcpy(&leafFlag, p, sizeof(byte));
    p += sizeof(byte);
    tsz += sizeof(byte);
    memcpy(&objCount, p, sizeof(size_t));
    p += sizeof(size_t);
    tsz += sizeof(size_t);
    isLeaf = (leafFlag == 1);

    if (objCount > 0)
    {

        //cout<<"read leafFlag ="<<isLeaf<<endl;
        //cout<<" read objCount="<<objCount<<endl;

        // read the addresses of the child pages
        if (objCount> 0 && !isLeaf)
        {
            children.resize(_fh.maxKeys);

            long* childAddresses = new long[objCount];
            //memset(childAddresses, 0xff, sizeof(long) * (objCount));
            memcpy(childAddresses, p, sizeof(long)*(objCount));
            p += sizeof(long)*(objCount);
            tsz += sizeof(long)*(objCount);

            //Only allocate childnode when the node is no a leaf node.
            if ( !isLeaf)
            {
                for (size_t i = 0; i < objCount; i++)
                {
                    if (children[i] == 0)
                    {
                        children[i] = new sdb_pnode_(_fh, _fileLock, activeNodeNum );
                    }
                    children[i]->fpos = childAddresses[i];
                    children[i]->childNo = i;
                    //cout<<children[i]->fpos<<endl;
                }
            }
            delete[] childAddresses;
            childAddresses = 0;
        }

        memcpy(&_overflowAddress, p, sizeof(long));
        p += sizeof(long);
        tsz += sizeof(long);
        memcpy(&_overflowPageCount, p, sizeof(size_t));
        p += sizeof(size_t);
        tsz += sizeof(size_t);

        //cout<<" read overFlowAddress="<<_overflowAddress<<"& "<<_overflowPageCount
        //		<<endl;

        size_t ksize, vsize;
        memcpy(&ksize, p, sizeof(size_t));

        //cout<<"keysize: "<<ksize<<endl;
        bool overflow = false;

        //overflow occur
        if (ksize == 0)
        {
            overflow = true;
            char *temp = new char[_pageSize*(_overflowPageCount+1)];
            memcpy(temp, pBuf, tsz);
            delete pBuf;
            pBuf = temp;
            p = pBuf + tsz;
            if (0 != fseek(f, _overflowAddress, SEEK_SET))
            {
                if (pBuf)
                    delete pBuf;
                return false;
            }
            if (1 != fread(p, _pageSize*_overflowPageCount, 1, f))
            {
                if (pBuf)
                    delete pBuf;
                pBuf = 0;
                return false;
            }
            memcpy(&ksize, p, sizeof(size_t));
        }
        p += sizeof(size_t);
        izene_deserialization< std::vector<KeyType> > izd(p, ksize);
        izd.read_image(keys);
        p += ksize;
        tsz += ksize+sizeof(size_t);

        if (isLeaf)
        {
            //values.resize(_fh.maxKeys);
            memcpy(&vsize, p, sizeof(size_t));

            //overflow occur
            if (vsize == 0)
            {
                //only overflow once!
                assert( !overflow);
                char *temp = new char[_pageSize*(_overflowPageCount+1)];
                memcpy(temp, pBuf, tsz);
                delete pBuf;
                pBuf = temp;
                p = pBuf + tsz;
                if (0 != fseek(f, _overflowAddress, SEEK_SET))
                {
                    if (pBuf)
                        delete pBuf;
                    return false;
                }
                if (1 != fread(p, _pageSize*_overflowPageCount, 1, f))
                {
                    if (pBuf)
                        delete pBuf;
                    pBuf = 0;
                    return false;
                }
                memcpy(&vsize, p, sizeof(size_t));
            }

            p += sizeof(size_t);
            izene_deserialization< std::vector<ValueType> > izd(p, vsize);
            izd.read_image(values);
            tsz += vsize+sizeof(size_t);
            //cout<<"vsize="<<vsize<<endl;
        }

    }
    //cout<<"tsz: "<<tsz;

    keys.resize(_fh.maxKeys);
    if (isLeaf)
        values.resize(_fh.maxKeys);
    else
        children.resize(_fh.maxKeys);

    delete [] pBuf;
    pBuf = 0;
    isLoaded = true;
    isDirty = false;

    //increment avtiveNodeNum
    ++activeNodeNum;
    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_pnode_< KeyType, ValueType, LockType, fixed,
Alloc>::write(FILE* f)
{

    //#ifdef DEBUG
    //static int _wcount;
    //cout<<"write "<<_wcount++ <<endl;
    //#endif

    //cout<<"write fpos="<<fpos<<endl;

    // If it is not loaded, it measn that we haven't been changed it ,
    // so we can say that the flush was successful.
    if (!isDirty)
    {
        return false;
    }
    if (!isLoaded)
    {
        return false;
    }
    if (!f)
    {
        return false;
    }

    ScopedWriteLock<LockType> lock(_fileLock);

    size_t tsz=0;

    //cout<<"_pageSize"<<_pageSize<<endl;
    char* pBuf = new char[_pageSize];
    memset(pBuf, 0, _pageSize);
    char* p = pBuf;

    // get to the right location
    if (0 != fseek(f, fpos, SEEK_SET))
    {
        //assert(false);
        return false;
    }

    // write the leaf flag and the object count
    byte leafFlag = isLeaf ? 1 : 0;

    memcpy(p, &leafFlag, sizeof(byte));
    p += sizeof(byte);
    tsz += sizeof(byte);

    //cout<<"write objCount= "<<objCount<<endl;
    memcpy(p, &objCount, sizeof(size_t));
    p += sizeof(size_t);
    tsz += sizeof(size_t);

    // write the addresses of the child pages
    if (objCount> 0 && !isLeaf)
    {
        long* childAddresses = new long[objCount];
        tsz += (objCount)*sizeof(long);
        assert(tsz < _pageSize);
        for (size_t i=0; i<objCount; i++)
        {
            childAddresses[i] = children[i]->fpos;
            //cout<<"write child fpos="<<childAddresses[i]<<endl;
        }
        memcpy(p, childAddresses, sizeof(long)*(objCount));
        p += objCount*sizeof(long);
        delete [] childAddresses;
        childAddresses = 0;

    }

    size_t ovfloff = tsz;
    memcpy(p, &_overflowAddress, sizeof(long));
    p += sizeof(long);
    tsz += sizeof(long);
    memcpy(p, &_overflowPageCount, sizeof(size_t));
    p += sizeof(size_t);
    tsz += sizeof(size_t);

    assert(tsz+sizeof(size_t) <= _pageSize);
    size_t np =1;
    //bool first_ovfl = true;

    char *ptr;
    size_t ksize;
    size_t esize;

    keys.resize(objCount);
    izene_serialization< std::vector<KeyType> > izs(keys);
    izs.write_image(ptr, ksize);
    esize = ksize + sizeof(size_t);

    bool first_overflow = true;
    if (tsz+esize + sizeof(size_t) > np*_pageSize)
    {
        first_overflow = false;
        tsz = _pageSize;
        np = (tsz+esize -1 )/_pageSize+1;
        char *temp = new char[np*_pageSize];
        //memset(temp, 0, np*_pageSize);
        memcpy(temp, pBuf, _pageSize);
        delete [] pBuf;
        pBuf = 0;
        pBuf = temp;
        p = pBuf+tsz;
    }

    memcpy(p, &ksize, sizeof(size_t));
    p += sizeof(size_t);
    memcpy(p, ptr, ksize);
    p += ksize;
    tsz += esize;

    if (isLeaf)
    {
        char* ptr1;
        size_t vsize;

        values.resize(objCount);
        izene_serialization< std::vector<ValueType> > izs1(values);
        izs1.write_image(ptr1, vsize);
        esize = vsize+sizeof(size_t);

        if (tsz+esize+sizeof(size_t) > np*_pageSize)
        {
            if (first_overflow)
                tsz = _pageSize;
            np = (tsz+esize -1 )/_pageSize+1;
            char *temp = new char[np*_pageSize];
            memset(temp, 0, np*_pageSize);
            memcpy(temp, pBuf, tsz);
            delete [] pBuf;
            pBuf = 0;
            pBuf = temp;
            if (first_overflow)
                p = pBuf+_pageSize;
            else
                p = pBuf+tsz;
        }

        memcpy(p, &vsize, sizeof(size_t));
        p += sizeof(size_t);
        memcpy(p, ptr1, vsize);
        p += vsize;
        tsz += esize;
    }

    //if( tsz >  _pageSize)
    //cout<<"fpos: "<<fpos<<" isLeaf: "<<isLeaf<<" tsz: "<<tsz<<endl;

    //no overflow
    if (np <= 1)
    {
        if (1 != fwrite(pBuf, _pageSize, 1, f) )
        {
            return false;
        }
    }
    else
    {
        //oveflow

        if (_overflowAddress <0 || _overflowPageCount < np-1)
        {
            _overflowAddress = sizeof(CbFileHeader)+_pageSize
                               *(_fh.nPages+_fh.oPages);
            _fh.oPages += (np-1);
        }
        _overflowPageCount = np-1;
        memcpy(pBuf+ovfloff, &_overflowAddress, sizeof(long));
        memcpy(pBuf+ovfloff+sizeof(long), &_overflowPageCount, sizeof(size_t));
        if (1 != fwrite(pBuf, _pageSize, 1, f) )
        {
            return false;
        }
        if (0 != fseek(f, _overflowAddress, SEEK_SET))
        {
            return false;
        }
        else
        {
            if (1 != fwrite(pBuf+_pageSize, _pageSize*(np-1), 1, f) )
            {
                return false;
            }
        }
        //cout<<"writing overflow!!!! pos:"<<_overflowAddress<<" & "
        //		<< _overflowPageCount <<endl;
    }

    delete []pBuf;
    pBuf = 0;
    isDirty = false;

    keys.resize(_fh.maxKeys);
    if (isLeaf)
        values.resize(_fh.maxKeys);
    else
        children.resize(_fh.maxKeys);

    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_pnode_<KeyType, ValueType, LockType, fixed, Alloc>* sdb_pnode_<
KeyType, ValueType, LockType, fixed, Alloc>::loadChild(size_t childNum,
        FILE* f)
{
    sdb_pnode_* child;
    child = children[childNum];
    if (isLeaf)
    {
        return NULL;
        //assert(0);
    }
    child->childNo = childNum;
    child->parent = this;
    if (child && !child->isLoaded)
    {
        //_fileLock.acquire_write_lock();
        child->read(f);
        //_fileLock.release_write_lock();
    }
    return child;
}

// Unload a child, which means that we get rid of all
// children in the children vector.
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_pnode_< KeyType, ValueType, LockType, fixed,
Alloc>::unload()
{
    if (isLoaded)
    {
        if ( !isLeaf)
        {
            for (size_t i=0; i<objCount; i++)
            {
                if (children[i])
                {
                    children[i]->unload();
                    if (children[i])
                    {
                        delete children[i];
                        children[i] = 0;
                    }
                }
            }
        }
        objCount = 0;
        //keys.resize(0);
        //values.resize(0);
        //children.resize(0);
        keys.clear();
        values.clear();
        children.clear();
        isLoaded = false;

        --activeNodeNum;
        parent = 0;
    }
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_pnode_< KeyType, ValueType, LockType, fixed,
Alloc>::unloadself()
{
    if (isLoaded)
    {
        objCount = 0;
        //keys.resize(0);
        //values.resize(0);
        //children.resize(0);
        keys.clear();
        values.clear();
        children.clear();
        isLoaded = false;

        --activeNodeNum;
        parent = 0;
    }
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_pnode_< KeyType, ValueType, LockType, fixed,
Alloc>::delFromLeaf(size_t objNo)
{
    bool ret = isLeaf;
    if (ret)
    {
        for (size_t i = objNo + 1; i < objCount; i++)
        {
            keys[i-1] = keys[i];
            values[i-1] = values[i];
        }
        setCount(objCount - 1);
    }
    isDirty = 1;
    return ret;
}

NS_IZENELIB_AM_END
#endif
