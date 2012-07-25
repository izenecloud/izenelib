/**
 * @file bucket_chain.h
 * @brief The header file of bucket_chain.
 * @author peisheng wang *
 *
 * This file defines class bucket_chain.
 */

#ifndef bucket_chain_H_
#define bucket_chain_H_

#include "sdb_hash_types.h"
//#include "sdb_hash_header.h"

/**
 *  \brief bucket_chain, represents a bucket of our array hash.
 *
 *   It has fixed size and an pointer to next bucket_chain element.
 *   It uses a char* member str to store item(binary) sequentially. When full, items will
 *   be stored to next bucket_chain element.
 *
 */

template<typename LockType = izenelib::util::NullLock> class bucket_chain_
{
    size_t bucketSize_;
public:
    char *str;
    long fpos;
    int num;
    bucket_chain_* next;
    bool isLoaded;
    bool isDirty;
    long nextfpos;
    int level;

    //It indicates how many active bucket_chains in memory.
    //It increases when allocateBlock() called, or reading from disk,
    //and decreases when unload() called.
    //static size_t activeNum;
private:
    //static LockType fileLock_;
    LockType& fileLock_;

public:
    /**
     *  constructor
     */
    bucket_chain_(size_t bucketSize, LockType& fileLock) :
            bucketSize_(bucketSize), fileLock_(fileLock)
    {
        str = NULL;
        num = 0;
        next = 0;
        isLoaded = false;
        isDirty = true;
        fpos = 0;
        nextfpos = 0;

        level = 0;
        //++activeNum;
    }

    /**
     *  deconstructor
     */
    virtual ~bucket_chain_()
    {
        /*if( str ){
         delete str;
         str = 0;
         }*/
        unload();
        if (next)
        {
            delete next;
            next = 0;
        }
        isLoaded = false;
    }

    /**
     *  write to disk
     */
    bool write(FILE* f)
    {
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

        izenelib::util::ScopedWriteLock<LockType> lock(fileLock_);

        //cout<<"write fpos="<<fpos<<endl;
        if ( 0 != fseek(f, fpos, SEEK_SET) )
            return false;

        if (1 != fwrite(&num, sizeof(int), 1, f) )
        {
            return false;
        }
        //cout<<"write num="<<num<<endl;

        size_t blockSize = bucketSize_ - sizeof(int) - sizeof(long);

        if (1 != fwrite(str, blockSize, 1, f) )
        {
            return false;
        }

        //long nextfpos = 0;

        if (next)
            nextfpos = next->fpos;

        //cout<<"write nextfpos = "<< nextfpos<<endl;
        if (1 != fwrite(&nextfpos, sizeof(long), 1, f) )
        {
            return false;
        }

        isDirty = false;
        isLoaded = true;

        return true;
    }

    bool load(FILE* f)
    {
        //fileLock_.acquire_write_lock();
        bool ret = this->read(f);
        //fileLock_.release_write_lock();
        return ret;
    }
    /**
     *  read from disk
     */
    bool read(FILE* f)
    {
        if (!f)
        {
            return false;
        }

        izenelib::util::ScopedWriteLock<LockType> lock(fileLock_);

        if (isLoaded)
        {
            return true;
        }

        //cout<<"read from "<<fpos<<endl;
        if ( 0 != fseek(f, fpos, SEEK_SET) )
            return false;

        if (1 != fread(&num, sizeof(int), 1, f) )
        {
            return false;
        }

        //cout<<"read num="<<num<<endl;
        size_t blockSize = bucketSize_ - sizeof(int) - sizeof(long);

        if ( !str)
        {
            str = new char[blockSize];
            // memset(str, 0, blockSize);
        }

        //cout<<"read blocksize="<<blockSize<<endl;
        if (1 != fread(str, blockSize, 1, f) )
        {
            return false;
        }

        //long nextfpos = 0;

        if (1 != fread(&nextfpos, sizeof(long), 1, f) )
        {
            return false;
        }

        //cout<<"read next fpos="<<nextfpos<<endl;
        if (nextfpos !=0)
        {
            if ( !next)
                next = new bucket_chain_(bucketSize_, fileLock_);
            next->fpos = nextfpos;
        }

        isLoaded = true;
        isDirty = false;

        //++activeNum;

        return true;
    }

    /**
     *    load next bucket_chain element.
     */
    bucket_chain_* loadNext(FILE* f, bool& loaded)
    {
        loaded = false;
        if (next && !next->isLoaded)
        {
            //cout<<"reading next"<<endl;
            //fileLock_.acquire_write_lock();
            next->read(f);
            loaded = true;
            //fileLock_.release_write_lock();
        }
        if (next)
            next->level = level+1;
        return next;
    }

    /**
     *   unload a buck_chain element.
     *   It releases most of the memory, and was used to recycle memory when cache is full.
     */
    bool unload()
    {
        if (str)
        {
            delete str;
            str = 0;
            //--activeNum;
            isLoaded = false;
            return true;
        }
        isLoaded = false;
        return false;

        //cout<<"unload fpos="<<fpos<<endl;
        //cout<<"activeNode: "<<activeNum<<endl;
    }

    /**
     *   display string_chain info.
     */
    void display(std::ostream& os = std::cout)
    {
        os<<"(level: "<<level;
        os<<"  isLoaded: "<<isLoaded;
        os<<"  bucketSize: "<<bucketSize_;
        os<<"  numitems: "<<num;
        os<<"  fpos: "<<fpos;
        if (next)
            os<<"  nextfpos: "<<next->fpos;
        //os<<"str: "<<str;
        os<<")- > ";
        if (next)
            next->display(os);
    }
};

#endif /*bucket_chain_H_*/
