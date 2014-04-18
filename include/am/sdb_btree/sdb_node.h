/**
 * @file sdb_node.h
 * @brief The header file of sdb_node.
 *
 * This file defines class sdb_node.
 */

#ifndef sdb_node_H_
#define sdb_node_H_

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
typename Alloc=std::allocator<DataType<KeyType,ValueType> > > class sdb_node_
{
    typedef std::pair<sdb_node_*, size_t> NodeKeyLocn;
    typedef typename std::vector<KeyType>::iterator KIT;
public:
    /**
     * \brief constructor
     *
     *  Constructor initialises everything to its default value.
     * Not that we assume that the node is a leaf,
     * and it is not loaded from the disk.
     */
    sdb_node_(CbFileHeader& fileHeader, LockType& fileLock, size_t& activeNum);

    ~sdb_node_()
    {
        unload();
        if (keys)
            delete [] keys;
        //if (values)
        //    delete [] values;
        std::vector<ValueType>().swap(values);
        if (children)
            for (size_t i=0; i<_fh.maxKeys+1; i++)
            {
                delete children[i];
                children[i] = NULL;
            }
        if (children)
            delete [] children;
    }

    /**
     * 	\brief when we want to access to node, we should load it to memory from disk.
     *
     *  Load a child node from the disk. This requires that we
     *  have the filepos already in place.
     */
    inline sdb_node_* loadChild(size_t childNum, FILE* f);

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
    KEYPOS findPos(const KeyType& key);

    /**
     * 	 \brief Find the position of the nearest object in a node,  given the key.
     */
    KEYPOS findPos1(const KeyType& key);

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
                if (children && children[i])
                    children[i]->display(os);
                os<<"----|";
            }
            //keys[i].display(os);
            //os<<keys[i]<<"->"<<values[i];
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
            if (children[i])
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
    sdb_node_* parent;

    KeyType* keys;
    sdb_node_** children;
    std::vector<ValueType> values;
    //std::vector<KeyType> keys;
    //std::vector<sdb_node_*> children; //it has objCount+1 childrens.
    //std::vector<ValueType> values;
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
typename Alloc> CompareFunctor<KeyType> sdb_node_< KeyType, ValueType,
LockType, fixed, Alloc>::_comp;

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_node_< KeyType, ValueType, LockType, fixed, Alloc>::sdb_node_(
    CbFileHeader& fileHeader, LockType& fileLock, size_t& activeNum) :
        objCount(0), fpos(-1), isLeaf(true), isLoaded(false), isDirty(1),
        childNo((size_t)-1), _fh(fileHeader), _maxKeys(_fh.maxKeys),
        _pageSize(_fh.pageSize), _overFlowSize(_fh.pageSize),
        _fileLock(fileLock), activeNodeNum(activeNum)
{
    _overflowAddress = -1;
    _overflowPageCount = 0;

    keys = new KeyType[_fh.maxKeys];
    ::memset(keys, 0x00, sizeof(KeyType)*_fh.maxKeys);
    //values = new ValueType[_fh.maxKeys];
    values.resize(_fh.maxKeys);
    //::memset(&values[0], 0x00, sizeof(ValueType)*_fh.maxKeys);

    children = new sdb_node_*[_fh.maxKeys+1];
    for (size_t i=0; i<_fh.maxKeys+1; i++)
        children[i] = NULL;

    //activeNodeNum++;
    //cout<<"activeNodeNum: "<<activeNodeNum<<endl;
}

// Read a node page to initialize this node from the disk
template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_node_< KeyType, ValueType, LockType, fixed,
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

    //cout<<"read from fpos "<<fpos<<endl;
    ScopedWriteLock<LockType> lock(_fileLock);

    if (isLoaded)
    {
        return true;
    }

    // get to the right location
    if (0 != fseek(f, fpos, SEEK_SET))
    {
        return false;
    }

    //keys.resize(_fh.maxKeys);
    //values.resize(_fh.maxKeys);
    //children.resize(_fh.maxKeys+1);

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

    if (keys == NULL)
    {
        keys = new KeyType[_fh.maxKeys];
        ::memset(keys, 0x00, sizeof(KeyType)*_fh.maxKeys);
    }
    if (values.empty())
    {
        //values = new ValueType[_fh.maxKeys];
        values.resize(_fh.maxKeys);
        //::memset(&values[0], 0x00, sizeof(ValueType)*_fh.maxKeys);
    }

    //cout<<"read leafFlag ="<<isLeaf<<endl;
    //cout<<" read objCount="<<objCount<<endl;

    // read the addresses of the child pages
    if (objCount> 0 && !isLeaf)
    {
        long* childAddresses = new long[objCount + 1];
        //memset(childAddresses, 0xff, sizeof(long) * (objCount + 1));
        memcpy(childAddresses, p, sizeof(long)*(objCount+1));
        p += sizeof(long)*(objCount+1);
        tsz += sizeof(long)*(objCount+1);

        //Only allocate childnode when the node is no a leaf node.
        if ( !isLeaf)
        {
            if ( !children )
                children = new sdb_node_*[_fh.maxKeys+1];
            for (size_t i=0; i<_fh.maxKeys+1; i++)
                children[i] = NULL;
            for (size_t i = 0; i <= objCount; i++)
            {
                if (children[i] == 0)
                {
                    children[i] = new sdb_node_(_fh, _fileLock, activeNodeNum );
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

    char *povfl=0;
    //size_t np = 0;
    //size_t next_page_off = 0;

    bool first_ovfl = true;

    //cout<<" read overFlowAddress="<<_overflowAddress<<endl;

    //read the key/vaue pairs
    for (size_t i = 0; i < objCount; i++)
    {

        if ( !fixed)
        {

            //cout<<" read data idx="<<i<<endl;
            size_t ksize, vsize;

            //DbObjPtr ptr, ptr1;

            memcpy(&ksize, p, sizeof(size_t));

            //cout<<"ksize"<<ksize<<endl;

            //read from overflow page
            if (ksize == 0)
            {
                assert(first_ovfl);
                if (first_ovfl)
                    first_ovfl =false;
                if ( !povfl)
                {
                    //cout<<"read overflowaddress="<<_overflowAddress<<" | "<<_overflowPageCount<<endl;

                    povfl = new char[_pageSize*_overflowPageCount];
                    if (0 != fseek(f, _overflowAddress, SEEK_SET))
                    {
                        if (pBuf)
                            delete pBuf;
                        delete povfl;
                        return false;
                    }
                    if (1 != fread(povfl, _pageSize*_overflowPageCount, 1, f))
                    {
                        if (pBuf)
                            delete pBuf;
                        delete povfl;
                        return false;
                    }
                }
                //p = povfl + np*_pageSize;
                p= povfl;
                memcpy(&ksize, p, sizeof(size_t));
                assert(ksize != 0);
                //is_incr_page = true;
            }

            p += sizeof(size_t);

            izene_deserialization<KeyType> izd(p, ksize);
            izd.read_image(keys[i]);
            p += ksize;

            memcpy(&vsize, p, sizeof(size_t));
            p += sizeof(size_t);

            //if value is of NULLType, the vsize is 0
            if (vsize != 0)
            {
                izene_deserialization<ValueType> izd1(p, vsize);
                izd1.read_image(values[i]);
                p += vsize;
            }

        }
        else
        {
            size_t esize = objCount*(sizeof(KeyType) + sizeof(ValueType));
            if (tsz + esize > _pageSize)
            {
                if ( !povfl)
                {
                    //cout<<"read overflowaddress="<<_overflowAddress<<" | "<<_overflowPageCount<<endl;

                    povfl = new char[_pageSize*_overflowPageCount];
                    if (0 != fseek(f, _overflowAddress, SEEK_SET))
                    {
                        if (pBuf)
                            delete pBuf;
                        delete povfl;
                        return false;
                    }
                    if (1 != fread(povfl, _pageSize*_overflowPageCount, 1, f))
                    {
                        if (pBuf)
                            delete pBuf;
                        delete povfl;
                        return false;
                    }
                }
                //p = povfl + np*_pageSize;
                p= povfl;

            }
            memcpy(&keys[0], p, objCount*sizeof(KeyType));
            p += objCount*sizeof(KeyType);
            memcpy(&values[0], p, objCount*sizeof(ValueType));
            p += objCount*sizeof(ValueType);
            break;
        }

    }

    if (povfl)
    {
        delete [] povfl;
        povfl = 0;
    }
    delete [] pBuf;
    pBuf = 0;
    isLoaded = true;
    isDirty = false;

    //increment avtiveNodeNum
    ++activeNodeNum;
    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_node_< KeyType, ValueType, LockType, fixed,
Alloc>::write(FILE* f)
{

    //#ifdef DEBUG
    //static int _wcount;
    //cout<<"write "<<_wcount++ <<endl;
    //#endif

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

    size_t tsz=0;

    //cout<<"_pageSize"<<_pageSize<<endl;
    char* pBuf = new char[_pageSize];
    memset(pBuf, 0, _pageSize);
    char* p = pBuf;

    //cout<<"write fpos="<<fpos<<endl;
    ScopedWriteLock<LockType> lock(_fileLock);

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
        long* childAddresses = new long[objCount + 1];
        tsz += (objCount+1)*sizeof(long);
        assert(tsz < _pageSize);
        for (size_t i=0; i<=objCount; i++)
        {
            childAddresses[i] = children[i]->fpos;
            //cout<<"write child fpos="<<childAddresses[i]<<endl;
        }
        memcpy(p, childAddresses, sizeof(long)*(objCount+1));
        p += (objCount+1)*sizeof(long);
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

    bool first_ovfl = true;
    for (size_t i=0; i<objCount; i++)
    {
        //cout<<"idx = "<<i<<endl;


        char *ptr, *ptr1;
        size_t ksize, vsize;
        size_t esize;
        izene_serialization<KeyType> izs(keys[i]);
        izene_serialization<ValueType> izs1(values[i]);

        if ( !fixed)
        {
            izs.write_image(ptr, ksize);
            izs1.write_image(ptr1, vsize);
            esize = 2*sizeof(size_t)+ksize+vsize;
        }
        else
        {
            //ptr = (char*)&keys[i];
            //ptr1 = (char*)&values[i];
            //ksize = sizeof(KeyType);
            //vsize = sizeof(ValueType);
            //esize = ksize+vsize;
            esize = objCount*(sizeof(KeyType) + sizeof(ValueType) )
                    -sizeof(size_t);
        }

        //when overflowing occurs, append the overflow buf
        if (tsz+esize+sizeof(size_t) > np*_pageSize)
        {
            //size_t endflag = 0;
            //memcpy(p, &endflag, sizeof(size_t));

            //tsz += sizeof(size_t);

            assert(size_t(p - pBuf) < np*_pageSize);
            if (first_ovfl)
            {
                tsz = _pageSize;
                first_ovfl = false;
            }
            //int incr_np = (tsz+esize+sizeof(size_t)-1 )/_pageSize+1;
            np = (tsz+esize+sizeof(size_t)-1 )/_pageSize+1;

            /*if(first_ovfl) {
             incr_np =(esize+sizeof(size_t)-1)/_pageSize + 1;
             first_ovfl = false;
             }
             else
             incr_np = (tsz+esize+sizeof(size_t)-1 )/_pageSize+1;*/

            char *temp = new char[np*_pageSize];
            //cout<<"alloc page num: "<<np<<endl;
            memset(temp, 0, np*_pageSize);
            memcpy(temp, pBuf, tsz);
            delete [] pBuf;
            pBuf = 0;
            pBuf = temp;

            p = pBuf+tsz;

            //cout<<"incr_np: "<< incr_np <<endl;

            /*	size_t endflag = 0;
             memcpy(p, &endflag, sizeof(size_t));

             int incr_np = ( tsz+esize+sizeof(size_t)-1)/_pageSize;
             cout<<"incr_np: "<< incr_np <<endl;
             //int incr_np = 1;
             char *temp = new char[(np+incr_np)*_pageSize];
             memset(temp, 0, (np+incr_np)*_pageSize );
             memcpy(temp, pBuf, np*_pageSize);
             delete pBuf;
             pBuf = temp;
             p = pBuf+np*_pageSize;
             tsz = 0;
             np+=incr_np;*/
        }
        if (!fixed)
        {
            memcpy(p, &ksize, sizeof(size_t));
            p += sizeof(size_t);
            memcpy(p, ptr, ksize);
            p += ksize;
            memcpy(p, &vsize, sizeof(size_t));
            p += sizeof(size_t);
            memcpy(p, ptr1, vsize);
            p += vsize;
            tsz += esize;
        }
        else
        {
            memcpy(p, &keys[0], sizeof(KeyType)*objCount);
            p += sizeof(KeyType)*objCount;
            memcpy(p, &values[0], sizeof(ValueType)*objCount);
            p += sizeof(ValueType)*objCount;
            tsz += esize;
            break;
        }
    }

    //cout<<"tsz="<<tsz<<endl;

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
        //cout<<"writing overflow!!!!"<<endl;
        if (_overflowAddress <0 || _overflowPageCount < np-1)
        {
            _overflowAddress = 1024 +_pageSize *(_fh.nPages +_fh.oPages);
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
    }
    delete []pBuf;
    pBuf = 0;
    isDirty = false;
    return true;
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> sdb_node_<KeyType, ValueType, LockType, fixed, Alloc>* sdb_node_<
KeyType, ValueType, LockType, fixed, Alloc>::loadChild(size_t childNum,
        FILE* f)
{
    sdb_node_* child;
    child = children[childNum];
    if (isLeaf || child == 0)
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
typename Alloc> void sdb_node_< KeyType, ValueType, LockType, fixed,
Alloc>::unload()
{
    if (isLoaded)
    {
        if ( !isLeaf)
        {
            for (size_t i=0; i<objCount+1; i++)
            {
                if (children && children[i])
                {
                    children[i]->unload();
                    if (children && children[i])
                    {
                        delete children[i];
                        children[i] = 0;
                    }
                }
            }
        }
        objCount = 0;
        delete [] keys;
        keys = NULL;
        //delete [] values;
        //values = NULL;
        std::vector<ValueType>().swap(values);
        delete [] children;
        children = NULL;

        //keys.resize(0);
        //values.resize(0);
        //children.resize(0);

        isLoaded = false;

        --activeNodeNum;
        parent = 0;
    }
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> void sdb_node_< KeyType, ValueType, LockType, fixed,
Alloc>::unloadself()
{
    if (isLoaded)
    {
        objCount = 0;
        //keys.resize(0);
        //values.resize(0);
        //children.resize(0);
        delete [] keys;
        keys = NULL;
        //delete [] values;
        //values = NULL;
        std::vector<ValueType>().swap(values);
        delete [] children;
        children = NULL;
        isLoaded = false;

        --activeNodeNum;
        parent = 0;
    }
}

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> bool sdb_node_< KeyType, ValueType, LockType, fixed,
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

// Find the position of the object in a node. If the key is at pos
// the function returns (pos, ECP_INTHIS). If the key is in a child to
// the left of pos, the function returns (pos, ECP_INLEFT). If the node
// is an internal node, the function returns (objCount, ECP_INRIGHT).
// Otherwise, the function returns ((size_t)-1, false).
// The main assumption here is that we won't be searching for a key
// in this node unless it (a) is not in the tree, or (b) it is in the
// subtree rooted at this node.

template<typename KeyType, typename ValueType, typename LockType, bool fixed,
typename Alloc> KEYPOS sdb_node_< KeyType, ValueType, LockType, fixed,
Alloc>::findPos(const KeyType& key)
{

    KEYPOS ret((size_t)-1, CCP_NONE);
    //KIT kit = keys.begin();
    size_t i = 0;
    while (i<objCount)
    {
        int compVal = _comp(key, keys[i]);
        if (compVal == 0)
        {
            return KEYPOS(i, CCP_INTHIS);
        }
        else if (compVal < 0)
        {
            if (isLeaf)
            {
                return ret;
            }
            else
            {
                return KEYPOS(i, CCP_INLEFT);
            }
        }
        ++i;
    }
    if (!isLeaf)
    {
        return KEYPOS(i - 1, CCP_INRIGHT);
    }
    return ret;
}

NS_IZENELIB_AM_END
#endif
