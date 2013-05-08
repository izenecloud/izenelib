#ifndef SF1V5_DOCUMENT_MANAGER_DOC_CONTAINER_H
#define SF1V5_DOCUMENT_MANAGER_DOC_CONTAINER_H

#include "Document.h"

#include <3rdparty/am/luxio/array.h>
#include <util/izene_serialization.h>
#include <util/bzip.h>

#include <ir/index_manager/utility/BitVector.h>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/scoped_array.hpp>

#include <compression/compressor.h>
#include <unistd.h>

// Define BDB if you want to compile this to use Berkeley DB
#include <stdint.h>
#include <3rdparty/tokufractaltreeindex/configure.h> 
#include <inttypes.h>
#ifdef BDB
#include <sys/types.h>
#include <3rdparty/tokufractaltreeindex/db.h>
#define DIRSUF bdb
#else
#include <3rdparty/tokufractaltreeindex/tokudb.h>
#define DIRSUF tokudb
#endif
namespace sf1r
{

class DocContainer
{
    typedef Lux::IO::Array containerType;

public:
    DocContainer(const std::string&path)
        : path_(path)
        , fileName_(path + "DocumentPropertyTable")
        , maxDocIdDb_(path + "MaxDocID.xml")
        , containerPtr_(NULL)
        , maxDocID_(0)
    {
        containerPtr_ = new containerType(Lux::IO::NONCLUSTER);
        containerPtr_->set_noncluster_params(Lux::IO::Linked);
        containerPtr_->set_lock_type(Lux::IO::LOCK_THREAD);
        restoreMaxDocDb_();
        string dbf=path+"/bench";
        cout<<dbf<<endl;
        parenttid=0;
        tid=0;
        setupDb_(dbf.data());
    }

    ~DocContainer()
    {
        //cout<<"    ~DocContainer()"<<endl;
        //cout<<"maxDocID"<<maxDocID_<<endl;
        delete parenttid;
        delete tid;
        int r = db->close(db, 0);
        assert(r == 0);
        r = dbenv->close(dbenv, 0);
        assert(r == 0);
      
        if (containerPtr_)
        {
            containerPtr_->close();
            delete containerPtr_;
        }
    }

    bool open()
    {
        try
        {
            if ( !boost::filesystem::exists(fileName_) )
            {
                containerPtr_->open(fileName_.c_str(), Lux::IO::DB_CREAT);
            }
            else
            {
                containerPtr_->open(fileName_.c_str(), Lux::IO::DB_RDWR);
            }
        }
        catch (...)
        {
            return false;
        }

        return true;

    }

    bool insert(const unsigned int docId, const Document& doc)
    {
        CREATE_SCOPED_PROFILER(insert_document, "Index:SIAProcess", "Indexer : insert_document")
        CREATE_PROFILER(proDocumentCompression, "Index:SIAProcess", "Indexer : DocumentCompression")
        maxDocID_ = docId>maxDocID_? docId:maxDocID_;
        izene_serialization<Document> izs(doc);
        char* src;
        size_t srcLen;
        izs.write_image(src, srcLen);
        //string strdocid=boost::lexical_cast<string>(docId);
        DBT  kt=id_to_dbt_(docId), vt;
        return db->put(db, tid, &kt, fill_dbt_(&vt, src, srcLen), put_flags)==0;
        //   return containerPtr_->put(docId, destPtr.get(), destLen + sizeof(uint32_t), Lux::IO::APPEND);

    }

    bool get(const unsigned int docId, Document& doc)
    {
        //CREATE_SCOPED_PROFILER(get_document, "Index:SIAProcess", "Indexer : get_document")
        //CREATE_PROFILER(proDocumentDecompression, "Index:SIAProcess", "Indexer : DocumentDecompression")
        if (docId > maxDocID_ )
        {
            return false;
        }
        int r;
        DBT key=id_to_dbt_(docId), val;
        memset(&val, 0, sizeof val);

        //DB_TXN *txn = NULL;
        //r = dbenv->txn_begin(dbenv, NULL, &txn, 0); assert(r == 0);
        r = db->get(db, tid, &key, &val, 0);
        if(r==0)
        {
            izene_deserialization<Document> izd((char*)val.data, val.size);
            izd.read_image(doc);
            return true;
        }
        else
        {
            Document doc1;
            cout<<"false"<<"   "<<docId<<"  "<<(r==DB_NOTFOUND)<<endl;
            if(docId!=1)
            {
                cout<<"docid 1"<<get(1, doc1)<<endl;
            }
            return false;
        }
    }

    bool exist(const unsigned int docId)
    {
        if (docId > maxDocID_ )
        {
            return false;
        }
        int r;
        DBT key=id_to_dbt_(docId), val;
        memset(&val, 0, sizeof val);

        DB_TXN *txn = NULL;
        r = dbenv->txn_begin(dbenv, NULL, &txn, 0);
        assert(r == 0);
        if(r==0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool del(const unsigned int docId)
    {
        DBT key=id_to_dbt_(docId);
        return  db->del(db, tid, &key, 0)==0;
    }

    bool update(const unsigned int docId, const Document& doc)
    {
        //  return true;
        if (docId > maxDocID_)
            return false;

        izene_serialization<Document> izs(doc);
        char* src;
        size_t srcLen;
        izs.write_image(src, srcLen);
        DBT key=id_to_dbt_(docId), extra;
        int r = db->update(db, tid, &key, &extra, 0); //assert(r == 0);
        return r==0;
    }

    docid_t getMaxDocId() const
    {
        return maxDocID_;
    }

    void flush()
    {
        // return;
        saveMaxDocDb_();
    }

private:
    bool saveMaxDocDb_() const
    {

        try
        {
            //  Document doc;
            // insert(maxDocID_, doc);
            ///Not Used. Array[0] could be used to store maxDocID since all doc ids start from 1
            containerPtr_->put(0, &maxDocID_, sizeof(unsigned int), Lux::IO::OVERWRITE);
            std::ofstream ofs(maxDocIdDb_.c_str());
            if (ofs)
            {
                boost::archive::xml_oarchive oa(ofs);
                oa << boost::serialization::make_nvp(
                       "MaxDocID", maxDocID_
                   );
            }

            return ofs;
        }
        catch (boost::archive::archive_exception& e)
        {
            return false;
        }

    }

    bool restoreMaxDocDb_()
    {
        try
        {
            std::ifstream ifs(maxDocIdDb_.c_str());
            if (ifs)
            {
                boost::archive::xml_iarchive ia(ifs);
                ia >> boost::serialization::make_nvp(
                       "MaxDocID", maxDocID_
                   );
            }
            return ifs;
        }
        catch (boost::archive::archive_exception& e)
        {
            maxDocID_ = 0;
            return false;
        }
    }

    void setupDb_ (const char *dbdir = "./bench")
    {
        //main1();
        //  main2();
        int r;
        bool do_append=false;
        string strdbdir(dbdir);
        if (boost::filesystem::is_directory(dbdir))
        {

            do_append=true;
        }
        if (!do_append)
        {
            char unlink_cmd[strlen(dbdir) + strlen("rm -rf ") + 1];
            snprintf(unlink_cmd, sizeof(unlink_cmd), "rm -rf %s", dbdir);
            system(unlink_cmd);

            if (strcmp(dbdir, ".") != 0)
            {
                r = mkdir(dbdir,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
                assert(r == 0);
            }
        }
        r = db_env_create(&dbenv, 0);
        assert(r == 0);

#if DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR <= 4
        if (dbenv->set_lk_max)
        {
            r = dbenv->set_lk_max(dbenv, items_per_transaction*2);
            assert(r==0);
        }
#endif
        if (dbenv->set_lk_max_locks)
        {
            r = dbenv->set_lk_max_locks(dbenv, items_per_transaction*2);
            assert(r == 0);
        }

        if (dbenv->set_cachesize)
        {
            r = dbenv->set_cachesize(dbenv, cachesize / (1024*1024*1024), cachesize % (1024*1024*1024), 1);
            if (r != 0)
                printf("WARNING: set_cachesize %d\n", r);
        }

        {
            r = dbenv->open(dbenv, dbdir, DB_CREATE, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            assert(r == 0);
        }

#if defined(TOKUDB)
        if (do_checkpoint_period)
        {
            r = dbenv->checkpointing_set_period(dbenv, checkpoint_period);
            assert(r == 0);
            u_int32_t period;
            r = dbenv->checkpointing_get_period(dbenv, &period);
            assert(r == 0 && period == checkpoint_period);
        }
#endif

        r = db_create(&db, dbenv, 0);
        assert(r == 0);
        r = db->open(db, tid, dbdir, NULL, DB_BTREE, DB_CREATE, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        r = dbenv->txn_begin(dbenv, 0, &tid, 0);
        assert(r==0);

    }


    DBT id_to_dbt_(const unsigned int& docId)
    {
        DBT key;
        string strdocid=boost::lexical_cast<string>(docId);
        char da[10];
        strcpy(da,strdocid.data());
        key.data = &da;
        key.size = strdocid.length();
        return key;
    }

    DBT *fill_dbt_(DBT *dbt, const void *data, int size)
    {
        memset(dbt, 0, sizeof *dbt);
        dbt->size = size;
        dbt->data = (void *) data;
        return dbt;
    }

    bool insert_(const  char* kc,const int& keysize,const  char* vc,const int& valsize)
    {

        DBT  kt, vt;
        int r=db->put(db, tid, fill_dbt_(&kt, kc, keysize), fill_dbt_(&vt, vc, valsize), put_flags);
        return r==0;
    }

private:
    std::string path_;
    std::string fileName_;
    std::string maxDocIdDb_;
    containerType* containerPtr_;
    docid_t maxDocID_;
    DocumentCompressor compressor_;
    DB_ENV *dbenv;
    DB *db;
    DB_TXN *parenttid;
    DB_TXN *tid;


};

}
#endif /*DOCCONTAINER_H_*/
