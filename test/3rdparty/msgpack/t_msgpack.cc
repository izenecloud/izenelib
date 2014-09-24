#include "echo_server.h"
#include <iostream>
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/server.h>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/cclog/cclog.h>
#include <3rdparty/msgpack/cclog/cclog_tty.h>
#include <signal.h>
#include <util/ustring/UString.h>

#include <boost/variant.hpp>

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

std::ostream& operator<<(std::ostream& out, variant_type& v)
{
    if (int64_t* p = boost::get<int64_t>(&v))
    {
        out << *p ;
    }
    else if (uint64_t* p = boost::get<uint64_t>(&v))
    {
        out << *p ;
    }
    else if (float* p = boost::get<float>(&v))
    {
        out << *p ;
    }
    else if (double* p = boost::get<double>(&v))
    {
        out << *p ;
    }
    else if (std::string* p = boost::get<std::string>(&v))
    {
        out << *p ;
    }
    else if (izenelib::util::UString* p = boost::get<izenelib::util::UString>(&v))
    {
        std::string str;
        (*p).convertString(str, izenelib::util::UString::UTF_8);
        out << str ;
    }
    else
        out << "unknow type ";

    return out;
}

struct Data
{
    bool bv;
    int iv;
    std::string sv;
    izenelib::util::UString usv;
    izenelib::util::UString::EncodingType encoding;
    std::vector<int> ivList;
    std::vector<std::pair<int, std::string> > isvList;
    //variant_type variant;

    MSGPACK_DEFINE(bv,iv,sv,usv,encoding,ivList,isvList);

    friend std::ostream& operator<<(std::ostream& out, Data& data);
};

std::ostream& operator<<(std::ostream& out, Data& data)
{
    out << data.bv <<std::endl
        << data.iv <<std::endl
        << data.sv <<std::endl;

    std::string str;
    data.usv.convertString(str, izenelib::util::UString::UTF_8);
    out << str <<std::endl;
    out << data.encoding<<std::endl;

    for (size_t i =0; i<data.ivList.size(); i ++)
    {
        out << data.ivList[i] << " ";
    }
    out << std::endl;

    for (size_t i =0; i<data.isvList.size(); i ++)
    {
        out << "("<<data.isvList[i].first<<","<<data.isvList[i].second << ") ";
    }
    out << std::endl;

    //cout << "variant: " << data.variant <<endl;


    return out;
}

void pack_unpack()
{
    Data data;
    data.bv = false;
    data.iv = 23;
    data.sv = "hello ";
    data.usv = izenelib::util::UString("hello 中文@#￥", izenelib::util::UString::UTF_8);
    data.encoding = izenelib::util::UString::CP949;
    data.ivList.push_back(3);
    data.ivList.push_back(43);
    data.ivList.push_back(366);
    data.isvList.push_back(std::make_pair(1,"yaya"));
    data.isvList.push_back(std::make_pair(2,"yoyo"));
    data.isvList.push_back(std::make_pair(3,"yyyy"));
//    int64_t i64 = 64;
//    uint64_t ui64 = 64;
//    float fv = .0225;
//    double dv = 3.01;
    izenelib::util::UString ustr("中文 243#%……& ", izenelib::util::UString::UTF_8);
    string str("abc 中文");
    //data.variant = ustr;
    std::cout <<"------ pack: \n" << data <<std::endl;

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, data);

    // unpack
    msgpack::unpacked result;
    msgpack::unpack(&result, sbuf.data(), sbuf.size());

    Data data2;
    data2 = result.get().as<Data>();
//
//    if (std::string* p = boost::get<std::string>(&data2.variant))
//    {
//        cout << "v string " << *p <<endl;
//        std::string str = *p;
//        izenelib::util::UString ustr = izenelib::util::UString(str, izenelib::util::UString::UTF_8);
//        std::string out;
//        ustr.convertString(out, izenelib::util::UString::UTF_8);
//        cout << out <<endl;
//    }

    std::cout <<"------ unpack: \n" << data2 <<std::endl;
}

int main(void)
{
    pack_unpack();

    return 0;

    /*
    cclog::reset(new cclog_tty(cclog::TRACE, std::cout));
    signal(SIGPIPE, SIG_IGN);

    // run server {
    msgpack::rpc::server svr;

    std::unique_ptr<msgpack::rpc::dispatcher> dp(new myecho);
    svr.serve(dp.get());

    svr.listen("0.0.0.0", 18811);

    svr.start(4);
    // }


    // create client
    msgpack::rpc::client cli("127.0.0.1", 18811);

    // call
    std::string msg("MessagePack-RPC");
    A a;
    a.s = "hahahah";
    a.i = 99;
//     std::string ret = cli.call("echo", msg).get<std::string>();
    A ret = cli.call("echo_a", a).get<A>();
    std::cout << "call: echo(\"MessagePack-RPC\") = " << ret << std::endl;

    return 0; */
}
