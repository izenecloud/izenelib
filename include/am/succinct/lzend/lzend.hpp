/*-----------------------------------------------------------------------------
 *  lzend  - A LZ77-like compression algorithm with random accesses
 *  //TO Be Verified
 *  Authors:
 *      Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#ifndef IZENELIB_AM_SUCCINCT_LZEND_HPP
#define IZENELIB_AM_SUCCINCT_LZEND_HPP

#include <types.h>

#include <am/succinct/sais/sais.h>
#include <am/succinct/wat_array/wat_array.hpp>
#include <am/succinct/dag_vector/dag_vector.hpp>

#include "compress/serialize.hpp"

#include <glog/logging.h>

#include <map>
#include <vector>
#include <cmath>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;
using namespace boost::property_tree;
using namespace boost::archive;
using namespace wat_array;

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace lzend
{

/* LZ_MAX_DICT_SZ MUST be < 2GiB */
#define LZ_MAX_DICT_SZ  (256 * 1048576)
#define LZ_MAX_LEN_SZ   1048576
#define LZ_MAX_WINDOW   65536

#define GZIP_MERGIN     1.10

/* FIXME: Currently, support 1-byte characters only */
#define LZ_ALPHABET_SZ  256
typedef uint8_t lzchar_t;
typedef uint32_t lzsource_t;

typedef pair<uint32_t, uint32_t> range_t;
typedef map<uint32_t, uint32_t> pattern_t;
typedef map<uint32_t, uint32_t>::iterator pattern_it_t;

/* Data I/O types */
typedef enum
{
    LZEND_IO_BOOST_SERIALIZE,
    LZEND_IO_JSON,
    LZEND_IO_COMPRESSED
} io_type;


/* A class to manage Suffix Array */
class sufarray_t
{
private:
    WatArray ssa;
    vector<uint64_t> sav;
    uint32_t *inv;
    uint32_t size;

public:
    sufarray_t(const lzchar_t *text, uint32_t size) : size(size)
    {
        int *sa;

        sa = new int[size];
        if (sa == NULL)
            LOG(FATAL) << "Can't allocate memory: sa";

        inv = new uint32_t[size];
        if (inv == NULL)
            LOG(FATAL) << "Can't allocate memory: inv";

        int ret = sais(static_cast<const unsigned char *>(text),
                       sa, static_cast<int>(size));
        if (ret < 0)
            LOG(FATAL) << "Can't use a sais library correctly";

        /* Initialize Wat-Array */
        for (uint32_t i = 0; i < size; i++)
            sav.push_back(sa[i]);

        ssa.Init(sav);

        /* Make a inverse list of SA */
        for (uint32_t i = 0; i < size; i++)
            inv[sa[i]] = i;

        delete[] sa;
    }

    ~sufarray_t()
    {
        if (inv) delete[] inv;
    }

    uint32_t operator[] (uint32_t idx) const
    {
        if (idx > size)
            throw "Overflowed value: idx";

        return static_cast<uint32_t>(sav[idx]);
    }

    uint32_t inverse(uint32_t idx)
    {
        CHECK(idx < size);
        return static_cast<uint32_t>(inv[idx]);
    }

    uint32_t max(uint32_t start, uint32_t end)
    {
        uint64_t val;
        uint64_t pos;

        CHECK(start <= end);
        ssa.MaxRange(start, end, pos, val);

        return static_cast<uint32_t>(val);
    }

    uint32_t get_size() const
    {
        return size;
    }
};

/*
 * A class to support a backword-search with WatArray
 * in input-reversed text.
 */
class bws_t
{
private:
    WatArray sbwt;
    uint32_t *count;
    uint32_t size;

public:
    bws_t(const lzchar_t *text, uint32_t size) 
        : size(size)
    {
        unsigned char *b;
        int *A;
        int ret;

        count = new uint32_t[LZ_ALPHABET_SZ];
        if (count == NULL)
            LOG(FATAL) << "Can't allocate memory: count";

        b = new unsigned char[size];
        if (b == NULL)
            LOG(FATAL) << "Can't allocate memory: b";

        A = new int[size];
        if (A == NULL)
            LOG(FATAL) << "Can't allocate memory: A";

        ret = sais_bwt(text,b, A, static_cast<int>(size));
        if (ret < 0)
            LOG(FATAL) << "Can't use a sais library correctly";

        for (uint32_t i = 0; i < LZ_ALPHABET_SZ; i++)
            count[i] = 0;
        for (uint32_t i = 0; i < size; i++)
            count[text[i]]++;
        for (uint32_t i = 1; i < LZ_ALPHABET_SZ; i++)
            count[i] += count[i - 1];

        /* Initialize Wat-Array */
        {
            vector<uint64_t> bwt;

            for (uint32_t i = 0; i < size; i++)
                bwt.push_back(b[i]);

            sbwt.Init(bwt);
        }

        delete[] A;
        delete[] b;
    }

    ~bws_t()
    {
        if (count) delete[] count;
    }

    range_t backword_search(range_t r, lzchar_t c)
    {
        range_t ret;

        ret.first = sbwt.Rank(c, r.first);
        ret.second = sbwt.Rank(c, r.second);

        if (c > 0)
        {
            ret.first += count[c - 1];
            ret.second += count[c - 1];
        }

        return ret;
    }

    uint32_t get_size() const
    {
        return size;
    }
};

/* A pattern filter for LZ-End */
class lz_pattern_t
{
private:
    pattern_t F;
    bws_t *bws;
    sufarray_t *SA;

    lzchar_t *reverse_text(
        const lzchar_t *text,
        uint32_t len) const
    {
        lzchar_t *rtext;

        rtext = new lzchar_t[len];
        if (rtext == NULL)
            LOG(FATAL) << "Can't allocate memory: rtext";

        for (uint32_t i = 0; i < len; i++)
            rtext[i] = text[len - i - 1];

        return rtext;
    }

public:
    /* Constructor */
    lz_pattern_t() {}

    /* Destructor */
    ~lz_pattern_t()
    {
        clear();
    }

    void build(const lzchar_t *text, uint32_t size)
    {
        lzchar_t *rtext;

        /*
         * size has 32-bit length because sais only
         * support Suffix Array with 32-bit.
         */
        CHECK(size <= LZ_MAX_DICT_SZ);

        rtext = reverse_text(text, size);

        SA = new sufarray_t(rtext, size);
        if (SA == NULL)
            LOG(FATAL) << "Can't allocate memory: SA";

        bws = new bws_t(rtext, size);
        if (bws == NULL)
            LOG(FATAL) << "Can't allocate memory: bws";

        delete[] rtext;
    }

    uint64_t source_search(range_t r)
    {
        CHECK(r.first < r.second);
        return static_cast<uint64_t>(
                   SA->max(r.first, r.second - 1));
    }

    bool is_source(range_t r, uint32_t len, uint32_t& qidx)
    {
        uint32_t val;
        pattern_it_t it;

        val = r.first + len - 1;

        it = F.lower_bound(val);
        if (it == F.end())
            return false;

        if ((*it).first < r.second)
        {
            qidx = (*it).second;
            return true;
        }

        return false;
    }

    range_t backword_search(range_t r, lzchar_t c)
    {
        CHECK(r.first <= r.second);
        return bws->backword_search(r, c);
    }

    void insert(uint32_t pos, uint32_t id)
    {
        F.insert(make_pair(SA->inverse(pos), id));
    }

    bool exist() const
    {
        if (F.size() != 0 &&
                bws != NULL && SA != NULL)
            return true;
        return false;
    }

    uint32_t get_size() const
    {
        CHECK(SA->get_size() == bws->get_size());
        return SA->get_size();
    }

    void clear()
    {
        F.clear();
        if (!SA) delete SA;
        if (!bws) delete bws;

        SA = NULL, bws = NULL;
    }
};

class psiv_t
{
private:
    dag_vector *psiv;

public:
    /* Constructor */
    psiv_t() {}

    /* Destructor */
    ~psiv_t() {}

    void build(const lzchar_t *text,
               uint32_t size)
    {
    }
};

class LZEnd
{
private:
    /* For info. to compress */
    BitArray bv;
    vector<lzchar_t> tails;
    vector<lzsource_t> srcs;

    /* A LZ-End dictionary */
    lz_pattern_t *pt;

    void init_stat()
    {
        bv.Clear();
        tails.clear();
        srcs.clear();
    }

    void build_lzend(const lzchar_t *text,
                     uint32_t size)
    {
        uint32_t idx;
        uint32_t pidx;

        /*
         * size has 32-bit length because sais only
         * support Suffix Array with 32-bit.
         */
        CHECK(size <= LZ_MAX_DICT_SZ);

        bv.Init(size);

        idx = 0;
        pidx = 1;
        while (idx < size)
        {
            uint32_t qidx;
            uint32_t plen;
            range_t r;

            plen = 0;
            qidx = 0;
            r = make_pair(0, size - 1);
            for (uint32_t j = 1; idx + j <= size &&
                    j <= LZ_MAX_WINDOW; j++)
            {
                r = pt->backword_search(r, text[idx + j - 1]);
                if (r.first >= r.second)
                    break;

                uint32_t mpos = pt->source_search(r);
                if (mpos <= size - idx - 1 + (j - 1))
                    break;

                if (pt->is_source(r, j, qidx))
                    plen = j;
            }

            uint32_t epos = idx + plen;
            uint32_t nxpos = epos + 1;

            pt->insert((size <= nxpos)? 0 : size - nxpos, pidx++);

            /* Update the elements of LZ-End */
            tails.push_back(text[epos]);
            srcs.push_back(qidx);
            bv.SetBit(1, epos);

            idx = nxpos;
        }

        bv.Build();
    }

    void append_lzend(const lzchar_t *text, uint64_t size)
    {
        uint64_t idx;
        uint64_t base;

        /*
         * Re-build the bit-vector
         * FIXME: Need a more smart way?
         */
        {
            vector<uint64_t> temp;

            base = bv.length();

            for (uint64_t i = 0; i < base; i++)
            {
                if (bv.Lookup(i))
                    temp.push_back(i);
            }

            bv.Init(base + size);

            for (uint64_t i = 0;
                    i < temp.size(); i++)
                bv.SetBit(1, temp[i]);
        }

        idx = 0;
        while (idx < size)
        {
            uint32_t qidx;
            uint32_t plen;
            range_t r;

            plen = 0;
            qidx = 0;
            r = make_pair(0, pt->get_size() - 1);
            for (uint32_t j = 1; idx + j <= size &&
                    j <= LZ_MAX_WINDOW; j++)
            {
                r = pt->backword_search(r, text[idx + j - 1]);
                if (r.first >= r.second)
                    break;

                if (pt->is_source(r, j, qidx))
                    plen = j;
            }

            /* Update the elements of LZ-End */
            tails.push_back(text[idx + plen]);
            srcs.push_back(qidx);
            bv.SetBit(1, base + idx + plen);

            idx = idx + plen + 1;
        }

        bv.Build();
    }

    void access_text_recsv(uint64_t start, uint64_t len,
                           lzchar_t *addr, uint64_t& wpos) const
    {
        uint64_t end;
        uint64_t p1;
        uint64_t p2;

        if (len > 0)
        {
            end = start + len - 1;
            p1 = bv.Rank(1, end);

            if (bv.Lookup(end))
            {
                this->access_text_recsv(start, len - 1, addr, wpos);
                addr[wpos++] = tails[p1];
            }
            else
            {
                p2 = bv.Select(1, p1);

                if (start <= p2)
                {
                    this->access_text_recsv(start,
                                            p2 - start + 1, addr, wpos);
                    len = end - p2;
                }

                this->access_text_recsv(
                    bv.Select(1, srcs[p1]) - len + 1,
                    len, addr, wpos);
            }
        }
    }

    /* Serialization functions below */
    void wt_boost_serialize(ostream& os) const
    {
        binary_oarchive oa(os);
        oa << *this;
    }

    void wt_json(ostream& os) const
    {
        ptree pt;
        write_json(os, pt);
    }

    void wt_compressed(ostream& os) const
    {
        uint8_t *v_tails;
        uint32_t *v_srcs;
        uint32_t *v_bv;
        uint64_t v_size;
        uint64_t v_len;

        /* FIXME: Need to fix a way to allocate memory */

        /* Write a sequence of tails[] */
        v_len = tails.size();
        v_tails = new uint8_t[static_cast<uint64_t>(
                                  GZIP_MERGIN * (double)v_len)];

        compressor::zlib_encode(tails, v_size, v_tails);

        os.write((const char *)&v_len, sizeof(v_len));
        os.write((const char *)&v_size, sizeof(v_size));
        os.write((const char *)v_tails, v_size);

        //delete[] v_tails;

        /* Write a sequence of srcs[] */
        v_len = srcs.size();
        v_srcs = new uint32_t[static_cast<uint64_t>(
                                  GZIP_MERGIN * (double)v_len)];

        compressor::int_encode(srcs, v_size, v_srcs);

        os.write((const char *)&v_len, sizeof(v_len));
        os.write((const char *)&v_size, sizeof(v_size));
        os.write((const char *)v_srcs, v_size * sizeof(*v_srcs));

        delete[] v_srcs;

        /* Write a sequence of a bit vector */
        {
            vector<uint32_t> tbv;

            uint64_t prev = 0;
            uint64_t bsize = bv.length();
            for (uint64_t i = 0; i < bsize; i++)
            {
                if (bv.Lookup(i))
                {
                    CHECK(i - prev < std::numeric_limits<uint32_t>::max());
                    tbv.push_back(i - prev);
                    prev = i;
                }
            }

            v_len = tbv.size();
            v_bv = new uint32_t[static_cast<uint64_t>(
                                    GZIP_MERGIN * (double)v_len)];

            compressor::int_encode(tbv, v_size, v_bv);

            os.write((const char *)&bsize, sizeof(bsize));

            os.write((const char *)&v_len, sizeof(v_len));
            os.write((const char *)&v_size, sizeof(v_size));
            os.write((const char *)v_bv, v_size * sizeof(*v_bv));

            delete[] v_bv;
        }
    }

    void rd_boost_serialize(istream& is)
    {
        binary_iarchive ia(is);
        ia >> *this;
    }

    void rd_json(istream& is)
    {
        ptree   pt;
        read_json(is, pt);
    }

    void rd_compressed(istream& is)
    {
        uint8_t *v_tails;
        uint32_t *v_srcs;
        uint32_t *v_bv;
        uint64_t v_size;
        uint64_t v_len;

        /* Read a sequence of tails[] */
        is.read((char *)&v_len, sizeof(v_len));
        is.read((char *)&v_size, sizeof(v_size));
        v_tails = new uint8_t[v_size];
        is.read((char *)v_tails, v_size);
        compressor::zlib_decode(tails, v_tails, v_size, v_len);
        delete[] v_tails;

        /* Read a sequence of srcs[] */
        is.read((char *)&v_len, sizeof(v_len));
        is.read((char *)&v_size, sizeof(v_size));
        v_srcs = new uint32_t[v_size];
        is.read((char *)v_srcs, v_size * sizeof(*v_srcs));
        compressor::int_decode(srcs, v_srcs, v_size, v_len);
        delete[] v_srcs;

        /* Read a sequence of a bit vector */
        {
            uint64_t bsize;
            vector<uint32_t> tbv;

            is.read((char *)&bsize, sizeof(bsize));

            is.read((char *)&v_len, sizeof(v_len));
            is.read((char *)&v_size, sizeof(v_size));

            v_bv = new uint32_t[v_size];
            is.read((char *)v_bv, v_size * sizeof(*v_bv));
            compressor::int_decode(tbv, v_bv, v_size, v_len);

            bv.Init(bsize);

            uint64_t sum = 0;
            for (uint64_t i = 0; i < tbv.size(); i++)
            {
                sum += tbv[i];
                bv.SetBit(1, sum);
            }

            bv.Build();
        }
    }

public:
    /* Constructor */
    LZEnd()
    {
        pt = new lz_pattern_t();
        init_stat();
    }

    /* Destructor */
    ~LZEnd()
    {
        if (pt) delete pt;
    }

    /*
     * Initializer
     */
    void lz_init(lzchar_t *text,
                 uint64_t size,
                 bool appendable = false)
    {
        if (text == NULL || size == 0)
            throw "Invalid input: text/size";

        /* Initialize all states */
        init_stat();

        /* Build the elements of LZ-End */
        if (size > LZ_MAX_DICT_SZ)
        {
            pt->build(text, LZ_MAX_DICT_SZ);
            build_lzend(text, LZ_MAX_DICT_SZ);
            append_lzend(text, size - LZ_MAX_DICT_SZ);
        }
        else
        {
            pt->build(text, size);
            build_lzend(text, size);
        }

        /*
         * NOTE: De-allocate the following
         * areas, since no data needs to
         * be appneded.
         */
        if (!appendable) pt->clear();
    }

    /*
     * The length of *addr MUST have 'len'. This interface
     * prevents wastful memory allocations.
     */
    void lz_strcpy(uint64_t start, uint64_t len,
                   lzchar_t *addr) const
    {
        /* FIXME: How do I validate the length of *addr? */
        if (!addr)
            throw "Can't access a given memory space";

        if (!get_size_pattern())
            throw "Not initialize yet";

        if (get_size_text() < start)
            throw "Overflowed value: start";

        /* Validate the length of text */
        if (start + len > get_size_text())
            len = get_size_text() - start;
        if (len >= LZ_MAX_LEN_SZ)
            throw "Too big to return the size of len";

        uint64_t wpos = 0;
        access_text_recsv(
            start, len, addr, wpos);

        CHECK(len == wpos);
    }

    string lz_to_string(uint64_t start,
                        uint64_t len) const
    {
        lzchar_t *addr;

        addr = new lzchar_t[len + 1];
        if (addr == NULL)
            LOG(FATAL) << "Can't allocate memory: addr";

        this->lz_strcpy(start, len, addr);

        addr[len] = '\0';
        string ret((char *)addr);

        delete[] addr;

        return ret;
    }

    lzchar_t operator[] (uint64_t idx) const
    {
        lzchar_t c;

        if (idx > get_size_text())
            throw "Overflowed value: idx";

        this->lz_strcpy(idx, 1, &c);

        return c;
    }

    /* Stream appending for alphabets */
    void lz_stream_append(lzchar_t *text, uint64_t size)
    {
        if (!pt->exist())
            throw "Can't append input text";

        append_lzend(text, size);
    }

    void lz_stream_close()
    {
        pt->clear();
    }

    void lz_save(ostream& os, io_type type) const
    {
        switch (type)
        {
        case LZEND_IO_BOOST_SERIALIZE:
            wt_boost_serialize(os);
            break;

        case LZEND_IO_JSON:
            wt_json(os);
            break;

        case LZEND_IO_COMPRESSED:
            wt_compressed(os);
            break;

        default:
            throw "No supported type";
            break;
        }
    }

    void lz_load(istream& is, io_type type)
    {
        switch (type)
        {
        case LZEND_IO_BOOST_SERIALIZE:
            rd_boost_serialize(is);
            break;

        case LZEND_IO_JSON:
            rd_json(is);
            break;

        case LZEND_IO_COMPRESSED:
            rd_compressed(is);
            break;

        default:
            throw "No supported type";
            break;
        }
    }

    /* Some utility functions below */
    uint64_t get_size() const
    {
        return get_size_pattern() *
               (sizeof(lzchar_t) + sizeof(lzsource_t)) +
               ((bv.length() + 63) / 64) * sizeof(uint64_t);
    }

    uint64_t get_size_pattern() const
    {
        CHECK(tails.size() == srcs.size());
        return tails.size();
    }

    uint64_t get_size_text() const
    {
        return bv.length();
    }

    void log_stats() const
    {
#ifndef NDEBUG
        /* Count the acc. of tails[] */
        uint64_t tails_count[LZ_ALPHABET_SZ];
        for (uint32_t i = 0;
                i < LZ_ALPHABET_SZ; i++)
            tails_count[i] = 0;
        for (uint64_t i = 0;
                i < get_size_pattern(); i++)
            tails_count[tails[i]]++;

        /* Nomalized stats. of tails[] */
        double tails_rt = 0.0;
        uint64_t tails_avg = get_size_pattern() / LZ_ALPHABET_SZ;
        for (uint32_t i = 0;
                i < LZ_ALPHABET_SZ; i++)
        {
            if (tails_count[i] >= tails_avg)
                tails_rt +=
                    (double)(tails_count[i] -
                             tails_avg) / get_size_pattern();
            else
                tails_rt +=
                    (double)(tails_avg -
                             tails_count[i]) / get_size_pattern();
        }

        /* Summation of srcs[] */
        uint64_t srcs_total = 0;
        for (uint32_t i = 0;
                i < get_size_pattern(); i++)
            srcs_total += srcs[i];

        LOG(INFO) << "Compression Statistics: " <<
                  "Pattern Ratio: " <<
                  (double)get_size_pattern() / bv.length() <<
                  "Bit Density: " <<
                  (double)bv.one_num() / bv.length() <<
                  "Normalized occ.(tails[]): " <<
                  tails_rt <<
                  "Ave. bit length(srcs[]): " <<
                  log2((double)srcs_total / get_size_pattern());
#endif /* NDEBUG */
    }

private:
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER();

    template <class Archive>
    void save(Archive& archive, unsigned int version) const
    {
        static_cast<void>(version);

        /* For patterns */
        archive & tails;
        archive & srcs;

        /* For a bit vector */
        {
            vector<uint32_t> tbv;

            uint64_t bsize = bv.length();
            archive & bsize;

            uint64_t prev = 0;
            for (uint64_t i = 0;
                    i < bv.length(); i++)
            {
                if (bv.Lookup(i))
                {
                    CHECK(i - prev < std::numeric_limits<uint32_t>::max());
                    tbv.push_back(i - prev);
                    prev = i;
                }
            }

            archive & tbv;
        }

        /* Show statistics */
        log_stats();
    }

    template <class Archive>
    void load(Archive& archive, unsigned int version)
    {
        static_cast<void>(version);

        archive & tails;
        archive & srcs;

        {
            uint64_t bsize;
            vector<uint32_t> tbv;

            archive & bsize;
            archive & tbv;

            bv.Init(bsize);

            uint64_t sum = 0;
            for (uint64_t i = 0;
                    i < tbv.size(); i++)
            {
                sum += tbv[i];
                bv.SetBit(1, sum);
            }

            bv.Build();
        }
    }
};

}
}
NS_IZENELIB_AM_END
#endif

