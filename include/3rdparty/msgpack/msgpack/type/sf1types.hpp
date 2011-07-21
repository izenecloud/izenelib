#ifndef MSGPACK_TYPE_SF1TYPES_HPP_
#define MSGPACK_TYPE_SF1TYPES_HPP_

#include "../object.hpp"
#include "int.hpp"
#include "float.hpp"
#include "string.hpp"
#include "UString.hpp"

#include <boost/variant.hpp>

/// sf1r::RankingType::TextRankingType
#ifndef SF1R_RANKINGENUMERATOR_H_
#define MSGPACK_RANKINGENUMERATOR_H_
    namespace sf1r {
    namespace RankingType {
        enum TextRankingType {
            DefaultTextRanker = 0,
            BM25,
            KL,
            PLM,
            NotUseTextRanker
        };
    }}
#endif

//// sf1r::QueryFiltering::FilteringOperation
#ifndef SF1R_QUERYTYPEDEF_H_
#define MSGPACK_QUERYTYPEDEF_H_
    namespace sf1r {
    namespace QueryFiltering {
    enum FilteringOperation
    {
        NULL_OPERATOR = 0,
        EQUAL,
        NOT_EQUAL,
        INCLUDE,
        EXCLUDE,
        GREATER_THAN,
        GREATER_THAN_EQUAL,
        LESS_THAN,
        LESS_THAN_EQUAL,
        RANGE,
        PREFIX,
        SUFFIX,
        SUB_STRING,
        MAX_OPERATOR
    };
    }}
#endif

/// variant type
typedef boost::variant<
    int64_t,
    uint64_t,
    float,
    double,
    std::string,
    izenelib::util::UString,
    std::vector<izenelib::util::UString>,
    std::vector<uint32_t>
> variant_type;


namespace msgpack {
/// RankingType
inline sf1r::RankingType::TextRankingType& operator>>(object o, sf1r::RankingType::TextRankingType& v)
{
    signed int iv = type::detail::convert_integer<signed int>(o);
    v = static_cast<sf1r::RankingType::TextRankingType>(iv);
    return v;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const sf1r::RankingType::TextRankingType& v)
    { o.pack_int(static_cast<int>(v)); return o; }

inline void operator<< (object& o, sf1r::RankingType::TextRankingType v)
    { o.type = type::POSITIVE_INTEGER, o.via.u64 = static_cast<int>(v); }

inline void operator<< (object::with_zone& o, sf1r::RankingType::TextRankingType v)
    { static_cast<object&>(o) << static_cast<int>(v); }


/// QueryFiltering::FilteringOperation
inline sf1r::QueryFiltering::FilteringOperation& operator>>(object o, sf1r::QueryFiltering::FilteringOperation& v)
{
    signed int iv = type::detail::convert_integer<signed int>(o);
    v = static_cast<sf1r::QueryFiltering::FilteringOperation>(iv);
    return v;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const sf1r::QueryFiltering::FilteringOperation& v)
    { o.pack_int(static_cast<int>(v)); return o; }

inline void operator<< (object& o, sf1r::QueryFiltering::FilteringOperation v)
    { o.type = type::POSITIVE_INTEGER, o.via.u64 = static_cast<int>(v); }

inline void operator<< (object::with_zone& o, sf1r::QueryFiltering::FilteringOperation v)
    { static_cast<object&>(o) << static_cast<int>(v); }


/// variant type
inline variant_type& operator>>(object o, variant_type& v)
{
    if(o.type == type::POSITIVE_INTEGER)
    {
        if (o.raw_type == type::RAW_UINT64)
        {
            v = type::detail::convert_integer<uint64_t>(o); return v;
        }
        else
        {
            v = type::detail::convert_integer<int64_t>(o); return v;
        }
    }
    else if(o.type == type::NEGATIVE_INTEGER)
    {
        v = type::detail::convert_integer<int64_t>(o); return v;
    }
    else if (o.type == type::DOUBLE)
    {
        v = (double)o.via.dec;
    }
    else if (o.type == type::RAW)
    {
        if (o.raw_type == type::RAW_USTRING)
        {
            izenelib::util::UString ustr;
            o >> ustr;
            v = ustr;
        }
        else
        {
            std::string str;
            o >> str;
            v = str;
        }
    }

    return v;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const variant_type& cv)
{
    variant_type& v = const_cast<variant_type&>(cv);
    if (int64_t* p = boost::get<int64_t>(&v))
    {
        o.pack_fix_int64(*p);
    }
    else if (uint64_t* p = boost::get<uint64_t>(&v))
    {
        o.pack_fix_uint64(*p);
    }
    else if (const float* p = boost::get<float>(&v))
    {
        o.pack_double(*p);
    }
    else if (double* p = boost::get<double>(&v))
    {
        o.pack_double(*p);
    }
    else if (std::string* p = boost::get<std::string>(&v))
    {
        o.pack_raw((*p).size());
        o.pack_raw_body((*p).c_str(), (*p).size());
    }
    else if (izenelib::util::UString* p = boost::get<izenelib::util::UString>(&v))
    {
        std::string s;
        (*p).convertString(s, izenelib::util::UString::UTF_8);
        o.pack_raw(s.size());
        o.pack_raw_body(s.data(), s.size());
    }
    else
        throw type_error();

    return o;
}

inline void operator<< (object& o, variant_type v)
{
    if (int64_t* p = boost::get<int64_t>(&v))
    {
        *p < 0 ? (o.type = type::NEGATIVE_INTEGER, o.via.i64 = *p, o.raw_type = type::RAW_INT64)
               : (o.type = type::POSITIVE_INTEGER, o.via.u64 = *p);
    }
    else if (uint64_t* p = boost::get<uint64_t>(&v))
    {
        o.type = type::POSITIVE_INTEGER, o.via.u64 = *p,  o.raw_type = type::RAW_UINT64;
    }
    else if (float* p = boost::get<float>(&v))
    {
        o << *p;
    }
    else if (double* p = boost::get<double>(&v))
    {
        o << *p;
    }
    else if (std::string* p = boost::get<std::string>(&v))
    {
        o << *p;
    }
    else if (izenelib::util::UString* p = boost::get<izenelib::util::UString>(&v))
    {
        o << *p;
    }
    else
        throw type_error();
}

inline void operator<< (object::with_zone& o, variant_type v)
{
    if (int64_t* p = boost::get<int64_t>(&v))
    {
        static_cast<object&>(o) << *p;
    }
    else if (uint64_t* p = boost::get<uint64_t>(&v))
    {
        static_cast<object&>(o) << *p;
    }
    else if (float* p = boost::get<float>(&v))
    {
        static_cast<object&>(o) << *p;
    }
    else if (double* p = boost::get<double>(&v))
    {
        static_cast<object&>(o) << *p;
    }
    else if (std::string* p = boost::get<std::string>(&v))
    {
        static_cast<object&>(o) << *p;
    }
    else if (izenelib::util::UString* p = boost::get<izenelib::util::UString>(&v))
    {
        o << *p;
    }
    else
        throw type_error();
}


} // namespace




#endif /* MSGPACK_TYPE_SF1TYPES_HPP_ */
