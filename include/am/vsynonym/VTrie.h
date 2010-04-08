/**
 * \file VTrie.h
 * \author vernkin
 * \brief hold the VTrie and the VTrieNode
 */

#include <assert.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <types.h>

NS_IZENELIB_AM_BEGIN

using namespace std;

//#define DEBUGP

#ifndef _VTRIE_H
#define	_VTRIE_H

#define VTPTR_L 4

#define VALUE_L 4
/** Length of the VTrie Entry, INT_LEN+1 */
#define VTENTRY_L 5

/** Type of the VTrie pointer */
typedef uint32_t vtptr_t;

/** max length of the same path */
#define MAX_SAMEP_LEN 256

/** number of the slots in the VTrie keys */
#define VTKEY_NUM 256

#define MIN_REMAIN 3072

#ifndef INCRE_SIZE
#define INCRE_SIZE 204800
#endif

/** length of VTrie's child status byte */
#define VTCHILDS_L 1

/** length of VTrie's same path status byte*/
#define VTSAMEPS_L 1

/* Conversion table for VTKEY_NUM numerical code */
extern uint8_t VTRIE_CODE[];


/**
 * \brief The node of the VTrie
 *
 * VTrieNode hold a int value and a bool attribute which indicates whether has
 * the synonyms with the same prefix.
 *
 * \author vernkin
 */
class VTrieNode{
public:
    /**
     * Default Constructor
     */
    VTrieNode(){
        init();
    }

    /**
     * Assign value to the node of VTrie, And value is over 0.
     * Almost, the value(data) may be a index of other data structure.
     * \param pData new data
     */
    void setData(int pData){
        data = pData;
    }

    /**
     * Get the value of the node
     * \return value of the node
     */
    int getData(){
        return data;
    }

    /**
     * Whether more long string whose prefix is the query exists in trie
     * structure.
     * \return true if more string with same prefix
     */
    bool hasMoreLong(){
        return moreLong;
    }

    /**
     * see hasMoreLong()
     * \param pMoreLong new more long
     */
    void setMoreLong(bool pMoreLong){
        moreLong = pMoreLong;
    }

    /**
     * Reset all the value to initial state.
     */
    void init(){
        data = 0;
        state = 0;
        moreLong = true;
        offset = 0;
    }

    friend ostream& operator << ( ostream& sout, VTrieNode& node ){
        sout<<"(data:"<<node.data<<", moreLong:"<<node.moreLong<<", state:"
                <<node.state<<", offset "<<node.offset<<")";
        return sout;
    }

public:
    /** the integer value */
    int data;

    /** the state of the node */
    uint16_t state;

    /** the offset in the data array of the VTrie */
    vtptr_t offset;

    /** whethe has synonyms with the same prefix */
    bool moreLong;
};

/**
 * \brief VTrie is a variant of the Radix Tree.
 *
 * While the Radix Tree is a space-optimized standard trie, the VTrie is a
 * space-optimized of the Radix Tree.
 *
 * \author vernkin
 */
class VTrie{
public:
    VTrie(){
        init();
    }

    ~VTrie(){
        if(data_){
            free(data_);
            data_ = 0;
        }
    }

    /**
     * If insert failed, return 0, otherwise any value bigger than 0
     * And the information of a current trie node is set to node parameter.
     *
     * \param key the key
     * \param node the node to stote the information
     * \return return 0 if insert fails
     */
    int insert( const char* key, VTrieNode* node ){
        //remaining length of the key
        size_t remainLen = strlen(key);
        ensureDataLength(remainLen);

        //empty string
        if(!remainLen){
            *reinterpret_cast<int*>(data_) = node->data;
            node->moreLong = *(data_ + VALUE_L) != 0;
            return 1;
        }

        if(remainLen > 0){
            *(data_ + VALUE_L) = 1;
        }

        vtptr_t* childPtr = reinterpret_cast<vtptr_t*>(data_ + VALUE_L
                        + VTCHILDS_L + VTPTR_L * VTRIE_CODE[(unsigned char)*key]);
        if(!*childPtr){
            *childPtr = (size_t)(endPtr_ - data_);
			#ifdef DEBUGP
				cout<<"Create "<<(uint8_t*)childPtr - data_ <<endl;
			#endif
            appendLeafNode(key, node);
            return 1;
        }

        uint8_t* dataPtr = data_ + *childPtr;

        while(remainLen){
            uint8_t* nodeStart = dataPtr;
            uint8_t samePathLen = *dataPtr++;
            while(samePathLen){
                if((uint8_t)*key == *dataPtr){
                    //the end of the key
                    if(remainLen == 1){
                        *reinterpret_cast<int*>(dataPtr+1) = node->data;
                        node->moreLong = samePathLen > 1 || *(dataPtr+VTENTRY_L);
                        return 1;
                    }
                }else{
                    vector<int> keyVec;
                    keyVec.push_back(VTRIE_CODE[(unsigned char)*key]);
                    keyVec.push_back(VTRIE_CODE[*dataPtr]);
                    uint16_t minMod = 1 + getModMinSize(keyVec);

                    node->moreLong = false;
                    //compute the current status
                    uint8_t entryNum = (uint8_t)((dataPtr - nodeStart - VTSAMEPS_L) / VTENTRY_L);
                    size_t copyLen = dataPtr - nodeStart;
                    uint8_t diffSamePathLen = *nodeStart - entryNum;
                    //compute wasted bytes, -1 is the same path status byte
                    wastedBytes_ += copyLen - 1;

                    uint8_t* nStart = endPtr_;
                    *childPtr = (vtptr_t)(nStart - data_);
                    //copy the same beginning
                    memcpy(nStart, nodeStart, copyLen);
                    //new same path status byte
                    *nStart = entryNum;
                    nStart += copyLen;
                    //children status byte
                    *nStart++ = (uint8_t)(minMod-1);
                    endPtr_ = nStart + minMod * VTPTR_L;
                    //set the original node (be compared)
                    *reinterpret_cast<vtptr_t*>(nStart + VTRIE_CODE[*dataPtr] % minMod * VTPTR_L)
                            = (vtptr_t)(dataPtr - data_ - VTSAMEPS_L);
                    *(dataPtr - 1) = diffSamePathLen;

                    //different node occur, separate the node
                    dataPtr = endPtr_;
                    *reinterpret_cast<vtptr_t*>(nStart + VTRIE_CODE[(unsigned char)*key] %
                            minMod* VTPTR_L) = (vtptr_t)(dataPtr - data_);
                    //append new leaf node
                    appendLeafNode(key, node);
                    return 1;
                }

                dataPtr += VTENTRY_L;
                --samePathLen;
                ++key;
                --remainLen;
            } //end while(samePathLen > 0)

            if(*dataPtr){
                //move to next node
                vtptr_t* tChildPtr = reinterpret_cast<vtptr_t*>(dataPtr + VTCHILDS_L
                    + VTRIE_CODE[(unsigned char)*key] % (1+*dataPtr) * VTPTR_L);

                //not exists, append new leaf node
                if(!*tChildPtr){
                    *tChildPtr = (vtptr_t)(endPtr_ - data_);
                    appendLeafNode(key, node);
                    return 1;
                }
                //maybe have to rebuild this node
                if((uint8_t)*key != *(data_ + *tChildPtr + VTSAMEPS_L)){
                    *childPtr = resolveConfilct(nodeStart, key, node);
                    return 1;
                }else{
                    childPtr = tChildPtr;
                }

                dataPtr = data_ + *childPtr;
            }
            //this node without children
            else{
                //at the end of the data_
                if((dataPtr+1) == endPtr_){
                    *nodeStart = *nodeStart + remainLen;
                    copyLeafNodeValue(dataPtr, key, node);
                    return 1;
                }

               //ignore the children status byte
                size_t copyLen = (size_t)(dataPtr - nodeStart);
                //this section would be abandoned
                wastedBytes_ += copyLen + VTCHILDS_L;
                dataPtr = endPtr_;
                *childPtr = (vtptr_t)(dataPtr - data_);
                //cout<<"addEnd d:"<<(dataPtr-data_)<<",s:"<<(nodeStart-data_)<<",l:"<<copyLen<<endl;
                memcpy(dataPtr, nodeStart, copyLen);

                //update the length of the same path
                *dataPtr = *dataPtr+ remainLen;
                dataPtr += copyLen;
                copyLeafNodeValue(dataPtr, key, node);
                return 1;
            }
        } //end while(remainLen)
        return 0;
    }

    /**
     * Optimize the data structure of the VTrie, it makes querying
     * faster and use less memory.
     */
    void optimize(){
       if(!wastedBytes_)
            return;
        curDataSize_ -= wastedBytes_;
        uint8_t* nData = (uint8_t*)malloc(curDataSize_);
        memset(nData, 0x0, curDataSize_);
        memcpy(nData, data_, VALUE_L + VTCHILDS_L);
        vtptr_t* nChild = (vtptr_t*)(nData + VALUE_L + VTCHILDS_L);
        vtptr_t* oChild = (vtptr_t*)(data_ + VALUE_L + VTCHILDS_L);
        uint8_t* childNData = (uint8_t*)nChild + VTKEY_NUM * VTPTR_L;
        for(int i=0; i<VTKEY_NUM; ++i){
            //have child
            if(*oChild){
                //set new child
                *nChild = (vtptr_t)(childNData - nData);
                childNData = optimize(childNData, data_+*oChild, nData, data_);
            }
            ++nChild;
            ++oChild;
        }

        free(data_);
        data_ = nData;
        endPtr_ = childNData;
        wastedBytes_ = 0;
    }

    /**
     * If string is not found, return 0, otherwise any value bigger than 0.
     * And the information of a searched trie node is set to node parameter.
     * \param key the key
     * \param node the node to stote the information
     * \return return 0 if search fails
     */
    int search( const char* key, VTrieNode* node){
        if(!data_) return 0;
        size_t remainLen = strlen(key);

        //empty string
        if(!remainLen){
            node->data = *reinterpret_cast<int*>(data_);
            node->moreLong = *(data_ + VALUE_L) != 0;
            return node->data != 0;
        }

        //find the find level
        vtptr_t childOffset = *reinterpret_cast<vtptr_t*>(data_ + VALUE_L
                        + VTCHILDS_L + VTPTR_L * VTRIE_CODE[(unsigned char)*key]);
        if(!childOffset){
            node->moreLong = false;
            node->data = 0;
            return 0;
        }

        //set the starting second level node
        uint8_t* dataPtr = data_ + childOffset;

        while( remainLen){
            uint8_t samePathLen = *dataPtr++;
            //should be ending here
            if(remainLen <= samePathLen){
                samePathLen -= remainLen;
                for(; remainLen; --remainLen){
                    //not equals, moreLong default is false
                    if((uint8_t)*key != *dataPtr){
                        node->moreLong = false;
                        node->data = 0;
                        return 0;
                    }
                    dataPtr += VTENTRY_L;
                    ++key;
                }
                //still have more content in the samePath or have children
                node->moreLong = samePathLen > 0 || *dataPtr;
                //rollback to the value
                dataPtr -= VALUE_L;
                break;
            }else{
                //upper size in the current path
                size_t lower = remainLen - samePathLen;
                for(; remainLen > lower; --remainLen){
                    //not equals, moreLong default is false
                    if((uint8_t)*key != *dataPtr){
                        node->moreLong = false;
                        node->data = 0;
                        return 0;
                    }
                    dataPtr += VTENTRY_L;
                    ++key;
                }

                //there is not children
                if(!*dataPtr){
                    node->moreLong = false;
                    node->data = 0;
                    return 0;
                }
                childOffset = *reinterpret_cast<vtptr_t*>(dataPtr + VTCHILDS_L
                        + VTRIE_CODE[(unsigned char)*key] % (1 + *dataPtr) * VTPTR_L);
                //no children slot
                if(!childOffset){
                    node->moreLong = false;
                    node->data = 0;
                    return 0;
                }
                dataPtr = data_ + childOffset;
            } //end else
        } //end while( remain )

        node->data = *reinterpret_cast<int*>(dataPtr);
        return node->data != 0;
    }

    /**
     * The function of this method is to iterate trie node by each character.
     * Parameter (VTrieNode*)node is including current node state.
     * \param ch specific character
     * \param node the last VTrieNode
     * \return If following node does not exist, return value is 0,
     * other wise bigger than 0.
     */
    int find( char ch, VTrieNode* node ){
        //not has more length any more
        if(!node->moreLong){
            node->data = 0;
            return 0;
        }
        //the header of VTrieNode
        if(!node->offset){
            vtptr_t firstOffset = *reinterpret_cast<vtptr_t*>(data_ + VALUE_L +
                    VTCHILDS_L + VTPTR_L * VTRIE_CODE[(unsigned char)ch]);
            //no child in the second level
            if(!firstOffset){
                node->data = 0;
                node->moreLong = false;
                return 0;
            }
            node->offset = firstOffset + 1;
            node->state = *(data_ + firstOffset);
            assert(node->state);
        }

        uint8_t* dataPtr = data_ + node->offset;

        //the child status byte
        if(!node->state){
            if(!*dataPtr){
                node->data = 0;
                node->moreLong = false;
                return 0;
            }
            vtptr_t childOffset = *reinterpret_cast<vtptr_t*>(dataPtr + VTCHILDS_L +
                    VTRIE_CODE[(unsigned char)ch] % (1 + *dataPtr) * VTPTR_L);
            //not exist children slot
            if(!childOffset){
                node->data = 0;
                node->moreLong = false;
                return 0;
            }
            node->offset = childOffset + 1;
            node->state = *(data_ + childOffset);
            assert(node->state);
            dataPtr = data_ + node->offset;
        }

		#ifdef DEBUGP
			cout<<"Compare Value Bit: ("<<(unsigned int)(uint8_t)ch<<" == "<<(unsigned int)*dataPtr <<
				") = "<<((uint8_t)ch == *dataPtr)<<" at "<<(unsigned int)(dataPtr - data_)<<endl;
		#endif
        if((uint8_t)ch == *dataPtr){
            node->data = *reinterpret_cast<int*>(dataPtr+1);
        }else{
            node->data = 0;
            node->moreLong = false;
            return 0;
        }
        --node->state;
        node->offset += VTENTRY_L;
        node->moreLong = node->state || *(dataPtr + VTENTRY_L);
        return node->data != 0;

    }

    /**
     * Get the size of the structure
     */
    size_t size(){
        return curDataSize_ + sizeof(VTrie);
    }

private:
    /** initialize the VTrie and add assert statement*/
    void init(){
        assert(sizeof(int) == VALUE_L);
        assert(sizeof(vtptr_t) == VTPTR_L);

        data_ = 0;
        endPtr_ = 0;
        wastedBytes_ = 0;
        curDataSize_ = 0;
    }

    /**
     * ensure the length of the data
     */
    void ensureDataLength(size_t keyLen){
        //has wasted too much space
        if(wastedBytes_ && wastedBytes_ > curDataSize_ * 0.3){
            optimize();
        }

        size_t remain = 0;
        size_t usedLen = 0;
        if( curDataSize_ > 0 ){
            usedLen = (size_t)(endPtr_ - data_);
            remain = curDataSize_ - usedLen;
        }

        if( remain > MIN_REMAIN )
            return;

        curDataSize_ += INCRE_SIZE;
        uint8_t* nData = (uint8_t*)malloc(curDataSize_);
        if(data_){
            memcpy(nData, data_, usedLen);
            //free the original data
            free(data_);
        }

        endPtr_ = nData + usedLen;

        memset(endPtr_, 0x0, curDataSize_ - usedLen);
        if(usedLen == 0){
            endPtr_ = nData + VALUE_L + VTCHILDS_L + VTKEY_NUM * VTPTR_L;
        }
        data_ = nData;
    }

    /**
     * return the ending pointer (exclude that location)
     */
    uint8_t* optimize(uint8_t* nData, uint8_t* oData, uint8_t* nRoot, uint8_t* oRoot){
        uint8_t samePathLen = *oData;
        uint8_t* dataPtr = oData + VTSAMEPS_L + VTENTRY_L * samePathLen;
        memcpy(nData, oData, dataPtr - oData + VTCHILDS_L);

        nData += dataPtr - oData + VTCHILDS_L;

        //cout<<(dataPtr-oRoot)<<":"<<(int)*dataPtr<<","<<(int)samePathLen<<endl;
        //has no child
        if(!*dataPtr){
            return nData;
        }
        int minMod = 1 + *dataPtr;

        vtptr_t* nChild = (vtptr_t*)nData;
        vtptr_t* oChild = (vtptr_t*)(dataPtr + 1);
        uint8_t* childNData = nData + minMod * VTPTR_L;
        for(int i=0; i<minMod; ++i){
            //have child
            if(*oChild){
                //set new child
                *nChild = (vtptr_t)(childNData - nRoot);
                childNData = optimize(childNData, oRoot+*oChild, nRoot, oRoot);
            }
            ++nChild;
            ++oChild;
        }
        return childNData;
    }

    /**
     * append the leaf node to the end of the data_
     */
    inline void appendLeafNode(const char* key, VTrieNode* node){
        *endPtr_ = (uint8_t)strlen(key);
        copyLeafNodeValue(endPtr_ + 1, key, node);
    }

    /**
     * Exclude same path status byte
     * this method would update endPtr_
     */
    inline void copyLeafNodeValue(uint8_t* dataPtr, const char* key, VTrieNode* node){
        while(*key){
            *dataPtr = (uint8_t)*key++;
			#ifdef DEBUGP
				cout<<"Set Ptr "<< (unsigned int)(*dataPtr)<<" at "
					<<(unsigned int)(dataPtr-data_)<<endl;
			#endif
            dataPtr += VTENTRY_L;
        }
        *reinterpret_cast<int*>(dataPtr-VALUE_L) = node->data;
		#ifdef DEBUGP
			cout<<"Copy value at "<<(unsigned int)(dataPtr-data_ - VALUE_L)
				<<" with "<<*reinterpret_cast<int*>(dataPtr-VALUE_L)<<endl;
		#endif
        //with no child status
        *dataPtr++ = 0;
        node->moreLong = false;

        endPtr_ = dataPtr;
    }

    /**
     * Get the minimum mod value of the children
     */
    inline uint8_t getModMinSize(const vector<int>& vec){
        int len = (int)vec.size();
        if(len == 0)
            return 0;

        int max = vec[0]; //max value in vector add 1
        for(int i=1;i<len;++i){
            if(vec[i] > max)
                max = vec[i];
        }
        ++max;
        bool tmpA[max];
        int k;

        for(int i=len;i<max; ++i){
            //reset again
            memset(tmpA, 0x0, sizeof(tmpA));
            for(k=0; k<len; ++k){
                int mod = vec[k] % i;
                if(tmpA[mod]) //have been set
                    break;
                tmpA[mod] = 1;
            }
            if(k == len){
                return (uint8_t)(i-1);
            }
        }
        return (uint8_t)(max-1);
    }

    inline vtptr_t resolveConfilct(uint8_t* nodeStart, const char* key, VTrieNode* node){
        uint8_t* modByte = nodeStart + VTSAMEPS_L + *nodeStart * VTENTRY_L;
        size_t copyLen = (size_t)(modByte - nodeStart);
        int minMod = 1 + *modByte;
        //update wasted bytes
        wastedBytes_ += copyLen + VTCHILDS_L + minMod * VTENTRY_L;

        vector<int> keyVec;
        //append the new char first
        keyVec.push_back(VTRIE_CODE[(unsigned char)*key]);

        vtptr_t* childPtr = (vtptr_t*)(modByte+1);

        for(int i=0; i<minMod; ++i){
            if(*childPtr){
                keyVec.push_back(VTRIE_CODE[*(data_ + *childPtr + VTSAMEPS_L)]);
            }
            ++childPtr;
        }

        uint16_t nMinMod = 1 + getModMinSize(keyVec);
        if(minMod == nMinMod){
            //cout<<"same mod "<<minMod<<endl;
            for(size_t i=0; i<keyVec.size(); ++i){
                cout<<keyVec[i]<<",";
            }
            cout<<endl;
        }
        assert(minMod != nMinMod);

        uint8_t* dataPtr = endPtr_;
        vtptr_t ret = (size_t)(dataPtr - data_);
        memcpy(dataPtr, nodeStart, copyLen);

        dataPtr += copyLen;

        *dataPtr++ = (uint8_t)(nMinMod - 1);

        endPtr_ = dataPtr + nMinMod * VTENTRY_L;

        childPtr = (vtptr_t*)(modByte+1);
        for(int i=0; i<minMod; ++i){
            if(*childPtr){
                vtptr_t code = VTRIE_CODE[*(data_ + *childPtr + VTSAMEPS_L)] % nMinMod * VTPTR_L;
                *reinterpret_cast<vtptr_t*>(dataPtr + code) = *childPtr;
            }
            ++childPtr;
        }

        vtptr_t code = VTRIE_CODE[(unsigned char)*key] % nMinMod * VTPTR_L;
        *reinterpret_cast<vtptr_t*>(dataPtr + code) = (vtptr_t)(endPtr_ - data_);
        appendLeafNode(key, node);
        return ret;
    }

    void printBytes(uint8_t* start, size_t num){
        if(start - data_ + num > curDataSize_){
            num = curDataSize_ - (start - data_);
        }
        for(size_t i=0; i<num; ++i){
            cout<<(int)*start++<<",";
        }
        cout<<endl;
    }

private:
    /** uint8_t array to store the trie */
    uint8_t* data_;

    /** ending used pointer in data_ */
    uint8_t* endPtr_;

    /** Bytes have been wasted */
    size_t wastedBytes_;

    /** current data size */
    size_t curDataSize_;
};

NS_IZENELIB_AM_END

#endif	/* _VTRIE_H */

