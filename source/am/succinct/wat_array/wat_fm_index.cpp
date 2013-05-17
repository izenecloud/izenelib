#include <am/succinct/wat_array/wat_fm_index.hpp>

#include <algorithm>
#include <iostream>

namespace wat_array
{

using namespace std;

FMIndex::FMIndex(size_t length) : doctails_(length), ddic_(0), head_(0)
{
}

FMIndex::~FMIndex()
{
}

void FMIndex::clear()
{
    wt_.Clear();
    doctails_.Clear();
    vector<uint64_t>().swap(posdic_);
    vector<uint64_t>().swap(idic_);
    ddic_   = 0;
    head_   = 0;
    izenelib::util::UString().swap(substr_);
}

void FMIndex::push_back(const izenelib::util::UString &doc)
{
    if (doc.length() <= 0)
    {
        throw "wat_array::FMIndex::push_back()";
    }
    substr_ += doc;
    doctails_.SetBit(1, doctails_.length() + doc.length() - 1);
}

void FMIndex::build(char end_marker, uint64_t ddic, bool is_msg)
{
    if (is_msg)
    {
        cerr << "building burrows-wheeler transform." << endl;
    }

    doctails_.Build();
    substr_ += end_marker;
    BWTransform b;
    b.build(substr_.c_str(), substr_.size());
    izenelib::util::UString s;
    b.get(s);
    head_ = b.head();
    b.clear();
    izenelib::util::UString().swap(substr_);
    if (is_msg)
    {
        cerr << "done." << endl;
    }

    if (is_msg)
    {
        cerr << "building wavelet tree." << endl;
    }
    vector<uint64_t> s_64(s.length());
    for (size_t i = 0; i < s.length(); ++i)
    {
        s_64[i] = s[i];
    }
    izenelib::util::UString().swap(s);
    wt_.Init(s_64);
    size_t len = s_64.size();
    vector<uint64_t>().swap(s_64);
    if (is_msg)
    {
        cerr << "done." << endl;
    }

    if (is_msg)
    {
        cerr << "building dictionaries." << endl;
    }
    for (uint64_t c = 0; c < 65536; c++)
    {
        rlt_[c] = wt_.RankLessThan(c, wt_.length());
    }
    ddic_ = ddic;
    posdic_.assign(len / ddic_ + 1, 0);
    idic_.assign(len / ddic_ + 1, 0);
    uint64_t i   = head_;
    uint64_t pos = length() - 1;
    do
    {
        if ((i % ddic_) == 0)
        {
            posdic_[i / ddic_] = pos;
        }
        if ((pos % ddic_) == 0)
        {
            idic_[pos / ddic_] = i;
        }
        uint16_t c = wt_.Lookup(i);
        i = rlt_[c] + wt_.Rank(c, i); //LF
        pos--;
    }
    while (i != head_);
    if (is_msg)
    {
        cerr << "done." << endl;
    }
}

uint64_t FMIndex::GetRows(const izenelib::util::UString &key) const
{
    uint64_t f, l;
    return GetRows(key, f, l);
}

uint64_t FMIndex::GetRows(const izenelib::util::UString &key, uint64_t &first, uint64_t &last) const
{
    uint64_t i = key.length() - 1;
    first = rlt_[uint16_t(key[i])] + 1;
    last  = rlt_[uint16_t(key[i]) + 1];
    while (first <= last)
    {
        if (i == 0)
        {
            first--;
            last--;
            return (last - first  + 1);
        }
        i--;
        uint16_t c = uint16_t(key[i]);
        first = rlt_[c] + wt_.Rank(c, first - 1) + 1;
        last  = rlt_[c] + wt_.Rank(c, last);
    }
    return 0;
}

uint64_t FMIndex::GetPosition(uint64_t i) const
{
    if (i >= length())
    {
        throw "wat_array::FMIndex::GetPosition()";
    }
    uint64_t pos = 0;
    while (i != head_)
    {
        if ((i % ddic_) == 0)
        {
            pos += (posdic_[i / ddic_] + 1);
            break;
        }
        uint16_t c = wt_.Lookup(i);
        i = rlt_[c] + wt_.Rank(c, i); //LF
        pos++;
    }
    return pos % length();
}

const izenelib::util::UString &FMIndex::GetSubstring(uint64_t pos, uint64_t len)
{
    if (pos >= length())
    {
        throw "wat_array::FMIndex::GetSubstring()";
    }
    uint64_t pos_end  = min(pos + len, length());
    uint64_t pos_tmp  = length() - 1;
    uint64_t i        = head_;
    uint64_t pos_idic = (pos_end + ddic_ - 2) / ddic_;
    if (pos_idic < idic_.size())
    {
        pos_tmp = pos_idic * ddic_;
        i       = idic_[pos_idic];
    }

    izenelib::util::UString().swap(substr_);
    while (pos_tmp >= pos)
    {
        uint16_t c = wt_.Lookup(i);
        i = rlt_[c] + wt_.Rank(c, i); //LF
        if (pos_tmp < pos_end)
        {
            substr_.insert(substr_.begin(), c);
        }
        if (pos_tmp == 0) break;
        pos_tmp--;
    }
    return substr_;
}

uint64_t FMIndex::GetDocumentId(uint64_t pos) const
{
    if (pos >= length())
    {
        throw "wat_array::FMIndex::GetDocumentId()";
    }
    return doctails_.Rank(1, pos);
}

void FMIndex::Search(const izenelib::util::UString &key, map<uint64_t, uint64_t> &dids) const
{
    dids.clear();
    uint64_t first, last;
    uint64_t rows = GetRows(key, first, last);
    if (rows > 0)
    {
        for (uint64_t i = first; i <= last; i++)
        {
            uint64_t pos = GetPosition(i);
            uint64_t did = GetDocumentId(pos);
            dids[did]++;
        }
    }
}
const izenelib::util::UString &FMIndex::GetDocument(uint64_t did)
{
    if (did >= docLength())
    {
        throw "wat_array::FMIndex::GetDocument()";
    }
    uint64_t pos = 0;
    if (did > 0)
        pos = doctails_.Select(1, did - 1) + 1;
    uint64_t len = doctails_.Select(1, did) - pos + 1;
    return GetSubstring(pos, len);
}

void FMIndex::Write(ofstream &ofs) const
{
    ofs.write((char *)&(ddic_), sizeof(uint64_t));
    ofs.write((char *)&(head_), sizeof(uint64_t));
    ofs.write((char *)&(rlt_), sizeof(uint64_t) * 65536);
    wt_.Save(ofs);
    doctails_.Save(ofs);

    vector<uint64_t>::const_iterator ip = posdic_.begin();
    vector<uint64_t>::const_iterator ep = posdic_.end();
    while (ip != ep)
    {
        ofs.write((char *)&(*ip), sizeof(uint64_t));
        ip++;
    }
    vector<uint64_t>::const_iterator ii = idic_.begin();
    vector<uint64_t>::const_iterator ei = idic_.end();
    while (ii != ei)
    {
        ofs.write((char *)&(*ii), sizeof(uint64_t));
        ii++;
    }
}

void FMIndex::Write(const char *filename) const
{
    ofstream ofs(filename, ios::out | ios::binary | ios::trunc);
    if (!ofs)
    {
        throw "wat_array::FMIndex::Write()";
    }
    Write(ofs);
}

void FMIndex::Read(ifstream &ifs)
{
    ifs.read((char *)&(ddic_), sizeof(uint64_t));
    if (ifs.eof())
    {
        throw "wat_array::FMIndex::read()";
    }
    ifs.read((char *)&(head_), sizeof(uint64_t));
    if (ifs.eof())
    {
        throw "wat_array::FMIndex::read()";
    }
    ifs.read((char *)&(rlt_), sizeof(uint64_t) * 65536);
    if (ifs.eof())
    {
        throw "wat_array::FMIndex::read()";
    }
    wt_.Load(ifs);
    doctails_.Load(ifs);

    uint64_t length = wt_.length() / ddic_ + 1;
    for (uint64_t i = 0; i < length; i++)
    {
        uint64_t x = 0;
        ifs.read((char *)&x, sizeof(uint64_t));
        if (ifs.eof())
        {
            throw "wat_array::FMIndex::read()";
        }
        posdic_.push_back(x);
    }
    for (uint64_t i = 0; i < length; i++)
    {
        uint64_t x = 0;
        ifs.read((char *)&x, sizeof(uint64_t));
        if (ifs.eof())
        {
            throw "wat_array::FMIndex::read()";
        }
        idic_.push_back(x);
    }
}

void FMIndex::Read(const char *filename)
{
    ifstream ifs(filename, ios::in | ios::binary);
    if (!ifs)
    {
        throw "wat_array::FMIndex::Read()";
    }
    Read(ifs);
}

////////////////////////////////

BWTransform::BWTransform() : length_(0), head_(0)
{
}

BWTransform::~BWTransform()
{
}

void BWTransform::clear()
{
    str_  = NULL;
    length_ = 0;
    head_ = 0;
    vector<uint32_t>().swap(sa_);
}

void BWTransform::build(const uint16_t *str, size_t len)
{
    srand(unsigned(time(NULL)));
    str_ = str;
    sa_.resize(len);
    length_ = 0;
    while (length_ < len)
    {
        sa_[length_] = length_;
        length_++;
    }
    sort_(0, length() - 1, 0);
    for (uint64_t i = 0; i < length(); i++)
    {
        if (sa_[i] == 0)
        {
            head_ = i;
            break;
        }
    }
}

uint16_t BWTransform::get(uint64_t i) const
{
    if (i >= length())
    {
        throw "wat_array::BWTransform::get()";
    }
    return str_[(sa_[i] + length() - 1) % length()];
}

void BWTransform::get(izenelib::util::UString &str) const
{
    str.resize(length());
    for (uint64_t i = 0; i < length(); i++)
    {
        str[i] = get(i);
    }
}

void BWTransform::sort_(int64_t begin, int64_t end, uint64_t depth)
{
    int64_t a = begin;
    int64_t b = begin;
    int64_t c = end;
    int64_t d = end;
    int64_t partial_size = end + 1 - begin;
    if (partial_size <= 1) return;

    uint16_t pivot = sa2char_(begin + rand() % partial_size, depth);

    while (1)
    {
        uint16_t b_ch = sa2char_(b, depth);
        while ((b <= c) && (b_ch <= pivot))
        {
            if (b_ch == pivot)
            {
                swap(sa_[a], sa_[b]);
                a++;
            }
            b++;
            if (b >= int64_t(length())) break;
            b_ch = sa2char_(b, depth);
        }

        uint16_t c_ch = sa2char_(c, depth);
        while ((b <= c) && (c_ch >= pivot))
        {
            if (c_ch == pivot)
            {
                swap(sa_[c], sa_[d]);
                d--;
            }
            c--;
            if (c < 0) break;
            c_ch = sa2char_(c, depth);
        }
        if (b > c) break;
        swap(sa_[b], sa_[c]);
        b++;
        c--;
    }

    int64_t eq_size = 0;
    eq_size = (((a - begin) < (b - a)) ? (a - begin) : (b - a));
    for (int64_t i = 0; i < eq_size; i++)
    {
        swap(sa_[begin + i], sa_[b - eq_size + i]);
    }
    eq_size = (((d - c) < (end - d)) ? (d - c) : (end - d));
    for (int64_t i = 0; i < eq_size; i++)
    {
        swap(sa_[b + i], sa_[end - eq_size + i + 1]);
    }

    sort_(begin,             begin + b - a - 1, depth    );
    sort_(begin + b - a,     end   - d + c,     depth + 1);
    sort_(end   - d + c + 1, end,               depth    );
}

uint16_t BWTransform::sa2char_(uint64_t i, uint64_t depth) const
{
    uint64_t offset = (sa_[i] + depth) % length();
    return uint16_t(str_[offset]);
}

}
