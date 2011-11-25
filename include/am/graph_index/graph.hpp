/**
@file graph.hpp
*/
#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <types.h>
#include "sorter.hpp"
#include "dyn_array.hpp"
#include "id_transfer.hpp"
#include "addr_bucket.hpp"
#include <string>
#include <vector>
#include <sys/time.h>
#include <boost/filesystem.hpp>
//#include <fstream>

NS_IZENELIB_AM_BEGIN

/**
@brief frequency structure
*/
template<typename VALUE_TYPE = uint32_t>
struct FREQ_STRUCT
{
    char freq[4];
    char nid[sizeof(VALUE_TYPE)];

    uint32_t& FREQ_()
    {
        return *(uint32_t*)freq;
    }

    VALUE_TYPE& NID_()
    {
        return *(VALUE_TYPE*)nid;
    }

    uint32_t FREQ()const
    {
        return *(uint32_t*)freq;
    }

    VALUE_TYPE NID()const
    {
        return *(VALUE_TYPE*)nid;
    }

    inline FREQ_STRUCT(uint32_t i, VALUE_TYPE j)
    {
        FREQ_() = i;
        NID_() = j;
    }

    inline FREQ_STRUCT(uint32_t i)
    {
        FREQ_() = i;
        NID_() = 0;
    }

    inline FREQ_STRUCT()
    {
        FREQ_() = 0;
        NID_() = 0;
    }

    inline FREQ_STRUCT(const FREQ_STRUCT& other)
    {
        FREQ_() = other.FREQ();
        NID_() = other.NID();
    }

    inline FREQ_STRUCT& operator = (const FREQ_STRUCT& other)
    {
        FREQ_() = other.FREQ();
        NID_() = other.NID();
        return *this;
    }

    inline bool operator == (const FREQ_STRUCT& other)const
    {
        return (FREQ() == other.FREQ());
    }

    inline bool operator != (const FREQ_STRUCT& other)const
    {
        return (FREQ() != other.FREQ());
    }

    inline bool operator < (const FREQ_STRUCT& other)const
    {
        return (FREQ() > other.FREQ());
    }

    inline bool operator > (const FREQ_STRUCT& other)const
    {
        return (FREQ() < other.FREQ());
    }

    inline bool operator <= (const FREQ_STRUCT& other)const
    {
        return (FREQ() >= other.FREQ());
    }

    inline bool operator >= (const FREQ_STRUCT& other)const
    {
        return (FREQ() <= other.FREQ());
    }

    inline uint32_t operator % (uint32_t e)const
    {
        return (FREQ() % e);
    }


    friend std::ostream& operator << (std::ostream& os, const FREQ_STRUCT& v)
    {
        os<<"<"<<v.FREQ()<<","<<v.NID()<<">";
        return os;
    }



}
;

/**
@brief this is edge structure.
*/
template<typename VALUE_TYPE = uint32_t>
struct EDGE_STRUCT
{
    char edge[4];
    char nid[sizeof(VALUE_TYPE)];

    uint32_t& EDGE_()
    {
        return *(uint32_t*)edge;
    }

    VALUE_TYPE& NID_()
    {
        return *(VALUE_TYPE*)nid;
    }

    uint32_t EDGE()const
    {
        return *(uint32_t*)edge;
    }

    VALUE_TYPE NID()const
    {
        return *(VALUE_TYPE*)nid;
    }

    inline EDGE_STRUCT(uint32_t i, VALUE_TYPE j)
    {
        EDGE_() = i;
        NID_() = j;
    }

    inline EDGE_STRUCT(uint32_t i)
    {
        EDGE_() = i;
        NID_() = 0;
    }

    inline EDGE_STRUCT()
    {
        EDGE_() = 0;
        NID_() = 0;
    }

    inline EDGE_STRUCT(const EDGE_STRUCT& other)
    {
        EDGE_() = other.EDGE();
        NID_() = other.NID();
    }

    inline EDGE_STRUCT& operator = (const EDGE_STRUCT& other)
    {
        EDGE_() = other.EDGE();
        NID_() = other.NID();
        return *this;
    }

    inline bool operator == (const EDGE_STRUCT& other)const
    {
        return (EDGE() == other.EDGE());
    }

    inline bool operator != (const EDGE_STRUCT& other)const
    {
        return (EDGE() != other.EDGE());
    }

    inline bool operator < (const EDGE_STRUCT& other)const
    {
        return (EDGE() < other.EDGE());
    }

    inline bool operator > (const EDGE_STRUCT& other)const
    {
        return (EDGE() > other.EDGE());
    }

    inline bool operator <= (const EDGE_STRUCT& other)const
    {
        return (EDGE() <= other.EDGE());
    }

    inline bool operator >= (const EDGE_STRUCT& other)const
    {
        return (EDGE() >= other.EDGE());
    }

    inline uint32_t operator % (uint32_t e)const
    {
        return (EDGE() % e);
    }

    friend std::ostream& operator << (std::ostream& os, const EDGE_STRUCT& v)
    {
        os<<"<"<<v.EDGE()<<","<<v.NID()<<">";
        return os;
    }



}
;
/**
@brief leaf node in tree
*/
struct LEAF_STRUCT
{
    char freq[4];
    char docs[8];

    uint32_t& FREQ_()
    {
        return *(uint32_t*)freq;
    }

    uint64_t& DOCS_()
    {
        return *(uint64_t*)docs;
    }

    uint32_t FREQ()const
    {
        return *(uint32_t*)freq;
    }

    uint64_t DOCS()const
    {
        return *(uint64_t*)docs;
    }

    inline LEAF_STRUCT(uint32_t i, uint64_t j)
    {
        FREQ_() = i;
        DOCS_() = j;
    }

    inline LEAF_STRUCT(uint32_t i)
    {
        FREQ_() = i;
        DOCS_() = 0;
    }

    inline LEAF_STRUCT()
    {
        FREQ_() = 0;
        DOCS_() = 0;
    }

    inline LEAF_STRUCT(const LEAF_STRUCT& other)
    {
        FREQ_() = other.FREQ();
        DOCS_() = other.DOCS();
    }

    inline LEAF_STRUCT& operator = (const LEAF_STRUCT& other)
    {
        FREQ_() = other.FREQ();
        DOCS_() = other.DOCS();
        return *this;
    }

    inline bool operator == (const LEAF_STRUCT& other)const
    {
        return (FREQ() == other.FREQ());
    }

    inline bool operator != (const LEAF_STRUCT& other)const
    {
        return (FREQ() != other.FREQ());
    }

    inline bool operator < (const LEAF_STRUCT& other)const
    {
        return (FREQ() < other.FREQ());
    }

    inline bool operator > (const LEAF_STRUCT& other)const
    {
        return (FREQ() > other.FREQ());
    }

    inline bool operator <= (const LEAF_STRUCT& other)const
    {
        return (FREQ() <= other.FREQ());
    }

    inline bool operator >= (const LEAF_STRUCT& other)const
    {
        return (FREQ() >= other.FREQ());
    }

    inline uint32_t operator % (uint32_t e)const
    {
        return (FREQ() % e);
    }

    friend std::ostream& operator << (std::ostream& os, const LEAF_STRUCT& v)
    {
        os<<"<"<<v.FREQ()<<","<<v.DOCS()<<">";
        return os;
    }



}
;


template<
uint32_t BUCKET_NUM = 800,//!< the max size per bucket
uint32_t BATCH_SAVE_SIZE = 1000, //!< number of branch of root for one time saving.
bool LEAN_MODE = false,
class TERM_TYPE = uint32_t,
//uint32_t SAVE_RATIO = 500,//number of branch for saving
class NID_LEN_TYPE = uint32_t,//!< the length type of nodes table
uint32_t ADDING_BUF_SIZE = 100000000//!< used for adding and fetch terms
>
class Graph
{
    typedef Graph<BUCKET_NUM, BATCH_SAVE_SIZE, LEAN_MODE, TERM_TYPE, NID_LEN_TYPE, ADDING_BUF_SIZE> self_t;

    typedef DynArray<uint64_t> array64_t;
    typedef DynArray<uint32_t> array32_t;

    typedef Sorter<ADDING_BUF_SIZE, TERM_TYPE> sorter_t;

    typedef EDGE_STRUCT<NID_LEN_TYPE> edge_t;
    typedef FREQ_STRUCT<NID_LEN_TYPE> sort_freq_t;

    typedef DynArray<edge_t> edges_t;
    typedef DynArray<edge_t, true> sorted_edges_t;
    typedef DynArray<sorted_edges_t*, false, 1, false, NID_LEN_TYPE> nids_t;

    typedef DynArray<uint64_t, false, 1, false, NID_LEN_TYPE> docs_t;
    typedef DynArray<uint32_t, false, 1, false, NID_LEN_TYPE> freqs_t;
    typedef DynArray<uint8_t, false, 1, false, NID_LEN_TYPE> loads_t;

    typedef DynArray<sort_freq_t, false, 1, true, NID_LEN_TYPE> sort_freqs_t;

    typedef struct LEAF_STRUCT leaf_t;
    typedef DynArray<leaf_t> leafs_t;

    #define LEAF_BOUND 1000000000

    mutable int error_;
    nids_t nodes_;//!< vector to store nodes pointer
    freqs_t freqs_;//!< vector to store frequency of nodes
    loads_t  loads_;//!< vector to store loads flag of nodes
    docs_t docs_;//!< vector to store docid vector pointer
    leafs_t leafs_;//!< vector to store leafs
    //IdTransfer<60000> id_mgr_;
    uint32_t doc_num_;//!< document number
    sorter_t* sorter_;//!< sorter for sorting all the suffix string
    std::string filenm_;//!< prefix of file name
    FILE*  nid_f_;//!< file handler for nodes pointer vector
    FILE*  doc_f_;//!< file handler for document pointer vector
    FILE*  leaf_f_;//!< file handler for leaf pointer vector
    uint32_t max_term_len_;//!< max number of term for one string.

    edge_t* insert_(TERM_TYPE term, edge_t* ep, bool ordered = true, uint32_t add_freq = 1)
    {
        NID_LEN_TYPE nid = ep->NID();

        IASSERT (nodes_.length() == freqs_.length());
        IASSERT (nodes_.length() == loads_.length());
        IASSERT (nodes_.length() == docs_.length());

        if (nid>=LEAF_BOUND)
        {
            nodes_.push_back(new sorted_edges_t());
            nid = nodes_.length()-1;

            nodes_.at(nid)->push_back(edge_t(term, nodes_.length()));
            freqs_.push_back(leafs_.at(ep->NID()-LEAF_BOUND).FREQ()+add_freq);

            loads_.push_back(1);

            docs_.push_back(leafs_.at(ep->NID()-LEAF_BOUND).DOCS());
            ep->NID_() = nid;

            return nodes_.at(nid)->data();
        }

        if (nid == nodes_.length())
        {
            nodes_.push_back(new sorted_edges_t());
            nid = nodes_.length()-1;

            nodes_.at(nid)->push_back(edge_t(term, nodes_.length()));

            freqs_.push_back(add_freq);

            loads_.push_back(1);

            docs_.push_back(0);

            return nodes_.at(nid)->data();
        }

        if (!ordered && !loads_.at(nid))
            load_edge_(nid);

        sorted_edges_t* edges = nodes_.at(nid);
//         IASSERT (edges != (sorted_edges_t*)-1);
        if( edges == (sorted_edges_t*)-1 )
        {
            set_error_("a");
            return NULL;
        }
        //if (nid == 5632)
        //std::cout<<(*edges)<<std::endl;

        freqs_[nid] += add_freq;

        //exist
        if (nid == 0)
        {
            if (ordered && edges->length()>0 && edges->back() == edge_t(term))
            {
                return edges->data()+edges->length()-1;
            }

            if (!ordered)
            {
                typename sorted_edges_t::size_t i = edges->find(edge_t(term));
                if (i != sorted_edges_t::NOT_FOUND)
                    return edges->data() + i;
            }

            typename sorted_edges_t::size_t i= edges->push_back(edge_t(term, nodes_.length()));
            //std::cout<<*edges<<":"<<i<<" "<<edges->length()<<std::endl;
            return edges->data() + i;
        }

        typename sorted_edges_t::size_t i  = edges->find(edge_t(term));

        if (sorted_edges_t::NOT_FOUND != i)
        {
            return edges->data()+i;
        }

        i= edges->push_back(edge_t(term, nodes_.length()));
        //std::cout<<*edges<<": "<<i<<" "<<edges->length()<<std::endl;
        return i + edges->data();
    }

    void save_redundant_(FILE* nid_f, FILE* doc_f)
    {
        fseek(nid_f, 0, SEEK_END);
        fseek(doc_f, 0, SEEK_END);

        for (NID_LEN_TYPE i=0; i<loads_.length(); ++i)
            if (!freqs_.at(i))
            {
                if (!loads_.at(i))
                    load_edge_(i);

                uint64_t addr = ftell(nid_f);
                nodes_[i]->save(nid_f);
                delete nodes_.at(i);
                nodes_[i] = (sorted_edges_t*)addr;
                if (docs_.at(i)!=0 && docs_.at(i)!=(uint64_t)-1)
                {
                    addr = ftell(doc_f);
                    ((array32_t*)docs_.at(i))->save(doc_f);
                    delete (array32_t*)docs_.at(i);
                    docs_[i] = addr;
                }

                loads_[i] = 0;
            }
    }

    void save_edge_(FILE* nid_f, FILE* doc_f, FILE* leaf_f,  NID_LEN_TYPE nid, bool root = false)
    {
        if (loads_.length()==0 || !loads_.at(nid))
            return;

        if (nid!=0 || root)
        {
            uint64_t addr = -1;
            loads_[nid] = 0;
            if (docs_.at(nid) != 0 && docs_.at(nid) != (uint64_t)-1)
            {
                addr = ftell(doc_f);
                ((array32_t*)docs_.at(nid))->save(doc_f);
                delete (array32_t*)docs_.at(nid);
            }

            docs_[nid] = addr;
        }

//         IASSERT (nodes_.at(nid) != (sorted_edges_t*)-1);
        if( nodes_.at(nid) == (sorted_edges_t*)-1 )
        {
            set_error_("b");
            return;
        }
        sorted_edges_t* e = nodes_.at(nid);

        for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
        {
            if (e->at(i).NID()>=LEAF_BOUND)
            {
                if (nid!=0 || root)
                {
                    uint64_t addr = ftell(leaf_f);
                    //if (e->at(i).NID()-LEAF_BOUND >= leafs_.length())
                    //std::cout<<nid<<" "<<e->at(i).NID()<<" "<<leafs_.length()<<std::endl;

                    leaf_t le = leafs_.at(e->at(i).NID()-LEAF_BOUND);
                    (*e)[i].NID_() = LEAF_BOUND + addr;

                    addr = ftell(doc_f);
                    ((array32_t*)le.DOCS())->save(doc_f);
                    delete (array32_t*)le.DOCS();
                    le.DOCS_() = addr;

//                     IASSERT(fwrite(&le, sizeof(leaf_t), 1, leaf_f)==1);
                    if( fwrite(&le, sizeof(leaf_t), 1, leaf_f)!=1 )
                    {
                        set_error_("c");
                        return;
                    }
                }

                continue;
            }

//             IASSERT(e->at(i).NID()<nodes_.length());
            if( e->at(i).NID()>=nodes_.length() )
            {
                set_error_("d");
                return;
            }
            save_edge_(nid_f, doc_f, leaf_f,  e->at(i).NID(), root);
        }

        if (nid==0 && !root)
            return;

        uint64_t addr = ftell(nid_f);
        e->save(nid_f);
        delete e;
        nodes_[nid] = (sorted_edges_t*)addr;

    }

    void load_edge_(NID_LEN_TYPE nid)
    {
        if (loads_.length()==0 || loads_.at(nid))
            return;

        sorted_edges_t* e = new sorted_edges_t();
        e->load(nid_f_, (uint64_t)nodes_.at(nid));
        nodes_[nid] = e;
        loads_[nid] = 1;

        for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
            if (e->at(i).NID()>=LEAF_BOUND)
            {
                fseek(leaf_f_, e->at(i).NID()-LEAF_BOUND, SEEK_SET);
                leaf_t le;
                array32_t* docs = new array32_t();

//                 IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                {
                    set_error_("e");
                    return;
                }
                docs->load(doc_f_, le.DOCS());
                le.DOCS_() = (uint64_t)docs;
                leafs_.push_back(le);
                (*e)[i].NID_() = LEAF_BOUND + leafs_.length()-1;
            }

            if (docs_.at(nid)!= (uint64_t)-1)
            {
                array32_t* docs = new array32_t();
                docs->load(doc_f_, docs_.at(nid));
                docs_[nid] = (uint64_t)docs;
            }
    }

    void load_edge_(NID_LEN_TYPE nid, double ratio)
    {
        if (loads_.length()==0 || loads_.at(nid))
            return;

        if (nodes_.at(nid) == (sorted_edges_t*)-1)
            return;

        sorted_edges_t* e = new sorted_edges_t();
        e->load(nid_f_, (uint64_t)nodes_.at(nid));
        nodes_[nid] = e;
        loads_[nid] = 1;

        if (ratio>=1.)
        {
            for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
                if (e->at(i).NID()<LEAF_BOUND )
                    load_edge_(e->at(i).NID(), ratio);
                else
                {
                    fseek(leaf_f_, e->at(i).NID()-LEAF_BOUND, SEEK_SET);
                    leaf_t le;
                    array32_t* docs = new array32_t();

//                     IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                    if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                    {
                        set_error_("f");
                        return;
                    }
                    docs->load(doc_f_, le.DOCS());
                    le.DOCS_() = (uint64_t)docs;
                    leafs_.push_back(le);
                    (*e)[i].NID_() = LEAF_BOUND + leafs_.length()-1;
                }

                if (docs_.at(nid)!= (uint64_t)-1)
                {
                    array32_t* docs = new array32_t();
                    docs->load(doc_f_, docs_.at(nid));
                    docs_[nid] = (uint64_t)docs;
                }
                return;
        }


        sort_freqs_t sf;
        for (typename sorted_edges_t::size_t i=0; i<e->length(); ++i)
            if (e->at(i).NID()<LEAF_BOUND)
                sf.push_back(sort_freq_t(freqs_.at(e->at(i).NID()), e->at(i).NID()));

            typename sort_freqs_t::size_t p = 0;
        for (typename sort_freqs_t::size_t i=1; i<sf.length(); ++i)
        {
            while (p<sf.length() && sf.at(p).FREQ()==1)
                ++p;

            if (p == sf.length())
                break;

            if (sf.at(i).FREQ()==1)
                std::swap(sf[i], sf[p]);
        }

        typename sort_freqs_t::size_t len = (typename sort_freqs_t::size_t)(sf.length()*ratio+0.5);
        for (typename sort_freqs_t::size_t i=0; i<p && i<len; ++i)
            load_edge_(sf.at(i).NID(), ratio);
    }

    NID_LEN_TYPE get_next_(NID_LEN_TYPE nid, TERM_TYPE term)const
    {
        if (nid>=nodes_.length())
            return -1;

        if (loads_.at(nid))
        {
            edge_t e(term);
            typename sorted_edges_t::size_t i = ((sorted_edges_t*)nodes_.at(nid))->find(e);

            //std::cout<<e<<"+"<<nid<<"+"<<i<<std::endl;

            if (i == sorted_edges_t::NOT_FOUND)
                return -1;
            return nodes_.at(nid)->at(i).NID();
        }

        if (nodes_.at(nid) == (sorted_edges_t*)-1)
            return -1;

        sorted_edges_t edges;
        edges.load(nid_f_, (uint64_t)nodes_.at(nid));

        edge_t e(term);
        typename sorted_edges_t::size_t i = edges.find(e);
        if (i == sorted_edges_t::NOT_FOUND)
            return -1;
        return edges.at(i).NID();
    }

    void get_docs_(NID_LEN_TYPE nid, array32_t& doclist)
    {
        array32_t docs;

        if (nid>=LEAF_BOUND)
        {
            leaf_t le;
            fseek(leaf_f_, nid-LEAF_BOUND, SEEK_SET);
//             IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
            if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
            {
                set_error_("g");
                return;
            }
            docs.load(doc_f_, le.DOCS());
            doclist += docs;
            return;
        }

        if (docs_.at(nid)!= (uint64_t)-1)
            docs.load(doc_f_, docs_.at(nid));

        if (loads_.at(nid))
        {
            sorted_edges_t* edges = nodes_.at(nid);

            for (typename edges_t::size_t i =0; i<edges->length(); ++i)
                get_docs_(edges->at(i).NID(), docs);

            doclist += docs;
            return;
        }

        if (nodes_.at(nid) == (sorted_edges_t*)-1)
        {
            doclist += docs;
            return;
        }

        edges_t edges;
        edges.load(nid_f_, (uint64_t)nodes_.at(nid));
        for (typename edges_t::size_t i = 0; i<edges.length(); ++i)
            get_docs_(edges.at(i).NID(), docs);
        doclist += docs;
    }


    void leaf_reset()
    {
        leafs_t lea;
        edges_t* e = (edges_t*)nodes_.at(0);

        for (typename edges_t::size_t i =0; i<e->length(); ++i)
            if (e->at(i).NID()>=LEAF_BOUND)
            {
                lea.push_back(leafs_.at(e->at(i).NID()-LEAF_BOUND));
                (*e)[i].NID_() = lea.length()-1+LEAF_BOUND;
            }
            leafs_ = lea;
    }

    void free_mem_(uint32_t start=0)
    {
        for (NID_LEN_TYPE i=start; i<loads_.length(); ++i)
            if (loads_.at(i))
            {
                delete nodes_.at(i);
                loads_[i] = 0;
            }
    }

    void open_files_()
    {
        if (nid_f_ == NULL)
            nid_f_ = fopen((filenm_+".nid").c_str(), "r+");
        if (doc_f_ == NULL)
            doc_f_ = fopen((filenm_+".doc").c_str(), "r+");
        if (leaf_f_ == NULL)
            leaf_f_ = fopen((filenm_+".lea").c_str(), "r+");
    }

    void set_error_(const char* msg) const
    {
        error_ = 1;
        std::cout<<"Graph error "<<msg<<std::endl;
    }

    public:
        inline Graph(const char* nm)
        {
            boost::filesystem::create_directories(nm);
            filenm_ = nm;
            filenm_ += "/graph";
            doc_num_ = 0;

            nid_f_ = doc_f_ = leaf_f_ = NULL;
            max_term_len_ = -1;

            sorter_ = NULL;
            error_ = 0;
        }

        ~Graph()
        {
            release();
        }

        /**
        @brief deallocate all the memory
        */
        void release()
        {
            free_mem_();
            nodes_.clear();
            loads_.clear();
            freqs_.clear();
            docs_.clear();
            leafs_.clear();
            if (nid_f_ != NULL)
                fclose(nid_f_);
            if (doc_f_ != NULL)
                fclose(doc_f_);
            if (leaf_f_ != NULL)
                fclose(leaf_f_);
            nid_f_ = NULL;
            doc_f_ = NULL;
            leaf_f_ = NULL;
        }

        int get_error_code() const
        {
            int r = error_;
            error_ = 0;
            return r;
        }

        /**
        @brief This function must be called before adding terms.
        */
        void ready4add()
        {
            if (sorter_)
                delete sorter_;

            sorter_ = new sorter_t((filenm_+".sort").c_str());
            sorter_->set_max_term_len(max_term_len_);
            sorter_->ready4add();
        }


        /**
        @return document number
        */
        uint32_t doc_num()const
        {
            return doc_num_;
        }

        inline void set_max_term_len(uint32_t t)
        {
            max_term_len_ = t;
        }

        /**
        @brief add terms of a document. Function indexing() must be called after all addings are done.
        */
        void add_terms(std::vector<uint32_t> terms, uint32_t docid)
        {
            static uint32_t last = -1;
            if (docid != last)
            {
                last = docid;
                ++doc_num_;
            }

            //id transfer
            array32_t ids;
            ids.reserve(terms.size());
            for (uint32_t i=0; i<terms.size(); ++i)
                ids.push_back(terms[i]/*id_mgr_.insert(terms[i])*/);

//             IASSERT(ids.length() == terms.size());
            if( ids.length() != terms.size() )
            {
                set_error_("h");
                return;
            }
            sorter_->add_terms(ids, docid);
        }

        /**
        @brief call this function to finish indexing
        */
        void indexing()
        {
            if (sorter_->num() == 0)
            {
                FILE* v_f = fopen((filenm_+".v").c_str(), "w+");
                nodes_.save(v_f);
                freqs_.save(v_f);
                docs_.save(v_f);
                fclose(v_f);

                nodes_.clear();
                loads_.clear();
                freqs_.clear();
                docs_.clear();
                leafs_.clear();

                delete sorter_;
                sorter_ = NULL;
                return;
            }

            struct timeval tvafter,tvpre;
            struct timezone tz;

            //sorter_ = new sorter_t(filenm_.c_str());
            gettimeofday (&tvpre , &tz);

            //FILE* id_f = fopen((filenm_+".id").c_str(), "w+");
            //id_mgr_.save(id_f);
            //fclose(id_f);

            sorter_->flush();

            sorter_->sort();

            gettimeofday (&tvafter , &tz);
            std::cout<<"\nSorting is done. ("<<doc_num()<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

            std::cout<<"Indexing is going on...\n";

            FILE* nid_f = fopen((filenm_+".nid").c_str(), "w+");
            FILE* doc_f = fopen((filenm_+".doc").c_str(), "w+");
            FILE* leaf_f = fopen((filenm_+".lea").c_str(), "w+");

            sorter_->ready4fetch();

            TERM_TYPE last_term = 0;
            uint32_t  batch_size = 0;

            array32_t t;
            uint32_t docid;

            for (uint32_t i=0; sorter_->next(t, docid); ++i)
            {
                //std::cout<<t<<std::endl;

//                 IASSERT(last_term<=t.at(0));
                if( last_term>t.at(0) )
                {
                    set_error_("i");
                    return;
                }
                if (last_term != t.at(0))
                {
                    last_term = t.at(0);
                    ++batch_size;

                    if (batch_size > BATCH_SAVE_SIZE)
                    {
                        save_edge_(nid_f, doc_f, leaf_f, 0);
                        leaf_reset();
                        batch_size = 0;
                    }
                }

                edge_t tmp(0,0);
                edge_t* next = &tmp;
                for (array32_t::size_t j=0; j<t.length(); ++j)
                {
                    next = insert_(t.at(j), next);
                    if( next == NULL )
                    {
                        set_error_("j");
                        return;
                    }
                    //docid should be added at the last term
                    if (j == t.length()-1)
                    {//new leaf
                    if (next->NID() == nodes_.length())
                    {
                        array32_t* docs = new array32_t();
                        docs->push_back(docid);
                        leaf_t le(1, (uint64_t)docs);
                        next->NID_() = leafs_.length()+LEAF_BOUND;
                        leafs_.push_back(le);
                        // std::cout<<"------------------------------------\n";
                        //           std::cout<<t<<std::endl;
                        //           std::cout<<*this;
                        break;
                    }

                    //leaf is already there
                    if (next->NID()>=LEAF_BOUND)
                    {
                        ((array32_t*)(leafs_[next->NID()-LEAF_BOUND].DOCS()))->push_back(docid);
                        leafs_[next->NID()-LEAF_BOUND].FREQ_()++;
                        // std::cout<<"------------------------------------\n";
                        //           std::cout<<t<<std::endl;
                        //           std::cout<<*this;
                        break;
                    }

//                     IASSERT(next->NID()<nodes_.length());
                    if( next->NID()>=nodes_.length() )
                    {
                        set_error_("k");
                        return;
                    }
                    ++freqs_[next->NID()];

                    if (docs_.at(next->NID()) == 0)
                        docs_[next->NID()] = (uint64_t)(new array32_t());

                    ((array32_t*)docs_.at(next->NID()))->push_back(docid);
                    // std::cout<<"------------------------------------\n";
                    //           std::cout<<t<<std::endl;
                    //           std::cout<<*this;
                    }
                }
            }

            sorter_->clear_files();
            //std::cout<<*this;

            save_edge_(nid_f, doc_f, leaf_f, 0, true);
            fclose(nid_f);
            fclose(doc_f);
            fclose(leaf_f);

            std::cout<<"Nodes amount: "<<nodes_.length()<<std::endl;

            FILE* v_f = fopen((filenm_+".v").c_str(), "w+");
            nodes_.save(v_f);
            freqs_.save(v_f);
            docs_.save(v_f);
            fclose(v_f);

            nodes_.clear();
            loads_.clear();
            freqs_.clear();
            docs_.clear();
            leafs_.clear();

            delete sorter_;
            sorter_ = NULL;
        }

        /**
        @brief flush and deallocate
        */
        void flush()
        {
            fseek(nid_f_, 0, SEEK_END);
            fseek(doc_f_, 0, SEEK_END);
            fseek(leaf_f_, 0, SEEK_END);

            save_edge_(nid_f_, doc_f_, leaf_f_, 0, true);

            save_redundant_(nid_f_, doc_f_);

            fclose(nid_f_);
            fclose(doc_f_);
            fclose(leaf_f_);

            nid_f_ = doc_f_ = leaf_f_ = NULL;

            FILE* v_f = fopen((filenm_+".v").c_str(), "w+");
            nodes_.save(v_f);
            freqs_.save(v_f);
            docs_.save(v_f);
            fclose(v_f);

            nodes_.clear();
            loads_.clear();
            freqs_.clear();
            docs_.clear();
            leafs_.clear();
        }


        /**
        @brief load nodes by this ratio
        @return false if it's empty
        */
        bool ratio_load(double ratio = 0.9)
        {
            free_mem_();
            if(ratio > 1.0) ratio = 1.0;
            FILE* v_f = fopen((filenm_+".v").c_str(), "r");
            if (v_f == NULL)
                return false;

            fseek(v_f, 0, SEEK_END);
            if (ftell(v_f)==0)
                return false;

            fseek(v_f, 0, SEEK_SET);
            nodes_.load(v_f);
            freqs_.load(v_f);
            docs_.load(v_f);
            fclose(v_f);

            //FILE* id_f = fopen((filenm_+".id").c_str(), "r");
            //id_mgr_.load(id_f);
            //fclose(id_f);


            loads_.clear();
            loads_.reserve(nodes_.length());
            for (NID_LEN_TYPE i=0; i<nodes_.length(); ++i)
                loads_.push_back(0);

            open_files_();

            //     nid_f_ = fopen((filenm_+".nid").c_str(), "r");
            //     doc_f_ = fopen((filenm_+".doc").c_str(), "r");
            //     leaf_f_ = fopen((filenm_+".lea").c_str(), "r");

            load_edge_(0, ratio);

            //std::cout<<docs_<<std::endl;
            return true;
        }

        /**
        @brief it will return frequency of the terms
        */
        uint32_t get_freq(const std::vector<uint32_t>& terms)const
        {
            NID_LEN_TYPE next = 0;
            for (std::size_t i=0; i<terms.size(); ++i)
            {
                //std::cout<<terms[i]<<"-"<<id_mgr_.get32(terms[i])<<std::endl;

                next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
                //std::cout<<next<<std::endl;

                if (next == (NID_LEN_TYPE)-1)
                    return 0;

                if (next>=LEAF_BOUND && i!=terms.size()-1)
                    return 0;

                if (next>=LEAF_BOUND)
                {
                    leaf_t le;
                    fseek(leaf_f_, next-LEAF_BOUND, SEEK_SET);
//                     IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                    if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                    {
                        set_error_("l");
                        return 0;
                    }
                    return le.FREQ();
                }

                if (LEAN_MODE && freqs_.at(next) == 1)
                    return 1;
            }

            return freqs_.at(next);
        }

        /**
        @brief it will return all the suffix term and their frequency
        */
        void get_suffix(const std::vector<uint32_t>& terms, std::vector<uint32_t>& suffix, std::vector<uint32_t>& freqs)const
        {
            freqs.clear();
            suffix.clear();

            NID_LEN_TYPE next = 0;
            for (std::size_t i=0; i<terms.size(); ++i)
            {
                next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
                if (next == (NID_LEN_TYPE)-1)
                    return ;

                if (next>=LEAF_BOUND)
                    return;

                if (LEAN_MODE && freqs_.at(next) == 1)
                {
                    suffix.push_back(0);
                    freqs.push_back(1);
                    return ;
                }
            }


            if (loads_.at(next))
            {
                sorted_edges_t* edges = nodes_.at(next);

                suffix.reserve(edges->length());
                freqs.reserve(edges->length());
                for (typename edges_t::size_t i =0; i<edges->length(); ++i)
                {
                    suffix.push_back(edges->at(i).EDGE());//(id_mgr_.get64(edges->at(i).EDGE()));
                    if (edges->at(i).NID()>=LEAF_BOUND)
                    {
                        leaf_t le;
                        fseek(leaf_f_, edges->at(i).NID()-LEAF_BOUND, SEEK_SET);
//                         IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                        if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                        {
                            set_error_("m");
                            return;
                        }
                        freqs.push_back(le.FREQ());
                        continue;
                    }

                    freqs.push_back(freqs_.at(edges->at(i).NID()));
                }

                return;
            }

//             IASSERT (nodes_.at(next) != (sorted_edges_t*)-1);
            if( nodes_.at(next) == (sorted_edges_t*)-1 )
            {
                set_error_("n");
                return;
            }
            edges_t edges;
            edges.load(nid_f_, (uint64_t)nodes_.at(next));

            suffix.reserve(edges.length());
            freqs.reserve(edges.length());
            for (typename edges_t::size_t i =0; i<edges.length(); ++i)
            {
                suffix.push_back(edges.at(i).EDGE());//(id_mgr_.get64(edges.at(i).EDGE()));
                if (edges.at(i).NID()>=LEAF_BOUND)
                {
                    leaf_t le;
                    fseek(leaf_f_, edges.at(i).NID()-LEAF_BOUND, SEEK_SET);
//                     IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                    if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                    {
                        set_error_("o");
                        return;
                    }
                    freqs.push_back(le.FREQ());
                    continue;
                }

                freqs.push_back(freqs_.at(edges.at(i).NID()));
            }
        }

        /**
        @brief it will return docid list of a terms
        */
        void get_doc_list(const std::vector<uint32_t>& terms, std::vector<uint32_t>& docids)
        {
            docids.clear();
            if (terms.size()==0)
                return;

            NID_LEN_TYPE next = 0;
            for (std::size_t i=0; i<terms.size(); ++i)
            {
                //std::cout<<terms[i]<<" ";
                next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
                if (next == (NID_LEN_TYPE)-1)
                    return ;

                if (next>=LEAF_BOUND && i!=terms.size()-1)
                    return;
            }

            array32_t docs;
            if (next>=LEAF_BOUND)
            {
                leaf_t le;
                fseek(leaf_f_, next-LEAF_BOUND, SEEK_SET);
//                 IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                {
                    set_error_("p");
                    return;
                }
                docs.load(doc_f_, le.DOCS());

                docids.reserve(docs.length());
                for (typename array32_t::size_t i=0; i<docs.length(); ++i)
                    docids.push_back(docs.at(i));
                return;
            }

            if (docs_.at(next)!= (uint64_t)-1)
                docs.load(doc_f_, docs_.at(next));

            if (loads_.at(next))
            {
                sorted_edges_t* edges = nodes_.at(next);

                for (typename edges_t::size_t i =0; i<edges->length(); ++i)
                    get_docs_(edges->at(i).NID(), docs);

                docids.reserve(docs.length());
                for (array32_t::size_t i=0; i<docs.length(); ++i)
                    docids.push_back(docs.at(i));

                return;
            }

            if (nodes_.at(next) == (sorted_edges_t*)-1)
            {
                docids.reserve(docs.length());
                for (typename array32_t::size_t i=0; i<docs.length(); ++i)
                    docids.push_back(docs.at(i));
                return;
            }

            edges_t edges;
            edges.load(nid_f_, (uint64_t)nodes_.at(next));
            for (typename edges_t::size_t i =0;i<edges.length(); ++i)
                get_docs_(edges.at(i).NID(), docs);

            docids.reserve(docs.length());
            for (typename array32_t::size_t i=0; i<docs.length(); ++i)
                docids.push_back(docs.at(i));

        }

        friend std::ostream& operator <<(std::ostream& os, const self_t& g)
        {
            //os<<g.nodes_.length()<<std::endl;
            for (NID_LEN_TYPE i=0; i<g.nodes_.length(); ++i)
            {
                os<<"----"<<i<<"-----\n";

                if ((uint64_t)g.nodes_.at(i) == (uint64_t)-1)
                    continue;

                if (g.loads_.at(i))
                {
                    os <<"+"<<g.freqs_.at(i)<<"|"<<*g.nodes_.at(i)<<std::endl;
                    continue;
                }

                sorted_edges_t edges;
                std::cout<<(uint64_t)g.nodes_.at(i)<<std::endl;
                edges.load(g.nid_f_, (uint64_t)g.nodes_.at(i));
                os<<g.freqs_.at(i)<<"|"<<edges<<std::endl;
            }

            return os;
        }

        /**
        @brief it will return doc ID list of a node indexed by nid
        */
        std::vector<uint32_t> get_docs(NID_LEN_TYPE nid)
        {
            array32_t docs;
            if (nid>=LEAF_BOUND)
            {
                leaf_t le;
                fseek(leaf_f_, nid-LEAF_BOUND, SEEK_SET);
//                 IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                {
                    set_error_("q");
                    return std::vector<uint32_t>();
                }
                docs.load(doc_f_, le.DOCS());
                std::vector<uint32_t> v(docs.length());
                memcpy(v.data(), docs.data(), docs.size());
                return v;
            }

            else if (docs_.at(nid)!= (uint64_t)-1)
                docs.load(doc_f_, docs_.at(nid));

            std::vector<uint32_t> v(docs.length());
            memcpy(v.data(), docs.data(), docs.size());

            edges_t edges;
            edges.load(nid_f_, (uint64_t)nodes_.at(nid));
            for (typename edges_t::size_t i =0;i<edges.length(); ++i)
                get_docs_(edges.at(i).NID(), docs);

            v.reserve(docs.length());
            for (typename array32_t::size_t i=0; i<docs.length(); ++i)
                v.push_back(docs.at(i));

            return v;
        }

        /**
        @brief it will return frequency of a node indexed by nid
        */
        uint32_t get_freq(NID_LEN_TYPE nid)const
        {
            if (nid>=LEAF_BOUND)
            {
                leaf_t le;
                fseek(leaf_f_, nid-LEAF_BOUND, SEEK_SET);
//                 IASSERT(fread(&le, sizeof(leaf_t), 1, leaf_f_)==1);
                if( fread(&le, sizeof(leaf_t), 1, leaf_f_)!=1 )
                {
                    set_error_("r");
                    return 0;
                }
                return le.FREQ();
            }

            return freqs_.at(nid);
        }



        class NodeIterator;

        /**
        @class Node
        */
        class Node
        {
            self_t* graph_;
            typename self_t::edge_t edge_;
            sorted_edges_t edges_;

            public:
                inline Node(self_t* graph, typename self_t::edge_t edge)
                :graph_(graph),edge_(edge)
                {
                    if (edge_.NID()>=LEAF_BOUND)
                        return;

                    if (graph_->loads_.length()==0)
                        return;

                    if (graph_->loads_.at(edge_.NID()))
                    {
                        edges_ = *graph_->nodes_.at(edge_.NID());
                        return;
                    }

                    edges_.load(graph_->nid_f_, (uint64_t)graph_->nodes_.at(edge_.NID()));

                }

                inline Node()
                :graph_(NULL)
                {
                }


                Node& operator  = (const Node& node)
                {
                    graph_ = node.graph_;
                    edge_ = node.edge_;
                    edges_ = node.edges_;
                    return *this;
                }

                uint32_t get_term()const
                {
                    //return graph_->id_mgr_.get64(edge_.EDGE());
                    return edge_.EDGE();
                }

                uint32_t get_freq()const
                {
                    return graph_->get_freq(edge_.NID());
                }

                std::vector<uint32_t> get_docs()const
                {
                    return graph_->get_docs(edge_.NID());
                }

                uint32_t get_nid()const
                {
                    return edge_.NID();
                }

                typename sorted_edges_t::size_t children_num()const
                {
                    return edges_.length();
                }

                NodeIterator children_begin() const
                {
                    if (edge_.NID()>=LEAF_BOUND)
                        return children_end();

                    return NodeIterator(edges_, graph_);
                }

                NodeIterator children_end()const
                {
                    return NodeIterator(edges_, graph_, edges_.length());
                }

                bool has_child()
                {
                    return edge_.NID()<LEAF_BOUND;
                    //return  (graph_->nodes_.at(edge_.NID())!= (sorted_edges_t*)-1);
                }

                Node find_child(uint32_t term)const
                {
                    typename self_t::edge_t edge(term);
                    typename sorted_edges_t::size_t i = edges_.find(edge);
                    if (i != sorted_edges_t::NOT_FOUND)
                        return Node(graph_, edges_.at(i));

                    return Node();
                }

                bool is_null()
                {
                    return (graph_== NULL || edge_.EDGE()==(uint32_t)-1);
                }

        };

        Node get_root()
        {
            return Node(this, edge_t(-1, 0));
        }

        bool get_node(const std::vector<uint32_t>& terms, Node& node)const
        {
            if (terms.size()==0)
                return false;

            NID_LEN_TYPE next = 0;
            for (std::size_t i=0; i<terms.size(); ++i)
            {
                //std::cout<<terms[i]<<" ";
                next = get_next_(next, terms[i]/*id_mgr_.get32(terms[i])*/);
                if (next == (NID_LEN_TYPE)-1)
                    return false;

                if (next>=LEAF_BOUND && i!=terms.size()-1)
                    return false;
            }

            node = Node(this, edge_t(terms.back(), next));
            return true;
        }

        bool get_node(const Node& node, uint32_t term, Node& r)const
        {
            r = node.find_child(term);
            return (!r.is_null());

            //     NodeIterator ni = node.children_begin();
            //     while (ni!=node.children_end())
            //     {
                //       if ((*ni).get_term()==term)
                //       {
                    //         r = *ni;
                    //         return true;
                    //       }
                    //       ++ni;
                    //     }

                    //     return false;
        }

        /**
        @class NodeIterator
        */
        class NodeIterator
        {
            sorted_edges_t edges_;
            self_t* graph_;
            typename edges_t::size_t p_;

            public:
                NodeIterator(const sorted_edges_t edges, self_t* graph, typename edges_t::size_t p = 0)
                :edges_(edges),graph_(graph),p_(p)
                {
                }


                NodeIterator(const NodeIterator& other)
                : edges_(other.edges_), graph_(other.graph_),p_(other.p_)
                {
                }

                NodeIterator& operator = (const NodeIterator& other)
                {
                    edges_ = other.edges_;
                    graph_ = other.graph_;
                    p_ = other.p_;

                    return *this;
                }

                Node operator *()
                {
                    return Node(graph_, edges_.at(p_));
                }

                NodeIterator& operator++()
                {
                    ++p_;
                    return(*this);
                }

                NodeIterator operator++(int)
                {
                    NodeIterator tmp(*this);

                    ++p_;

                    return(tmp);
                }

                bool operator != (const NodeIterator& other)
                {
                    return p_ != other.p_;
                }

        }
        ;

}
;


NS_IZENELIB_AM_END
#endif
