#include "echo_server.h"
#include <iostream>
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/server.h>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/cclog/cclog.h>
#include <3rdparty/msgpack/cclog/cclog_tty.h>
#include <signal.h>
#include <util/ustring/UString.h>

struct Data
{
    bool bv;
    int iv;
    std::string sv;
    izenelib::util::UString usv;
    std::vector<int> ivList;
    std::vector<std::pair<int, std::string> > isvList;

    MSGPACK_DEFINE(bv,iv,sv,usv,ivList,isvList);

    friend std::ostream& operator<<(std::ostream& out, Data& data);
};

std::ostream& operator<<(std::ostream& out, Data& data)
{
    out << data.bv <<std::endl
        << data.iv <<std::endl
        << data.sv <<std::endl;

    std::string str;
    data.usv.convertString(str, izenelib::util::UString::UTF_8);
    out  << str <<std::endl;

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

    return out;
}

void pack_unpack()
{
    Data data;
    data.bv = false;
    data.iv = 23;
    data.sv = "hello ";
    data.usv = izenelib::util::UString("hello 中文@#￥", izenelib::util::UString::UTF_8);
    data.ivList.push_back(3);
    data.ivList.push_back(43);
    data.ivList.push_back(366);
    data.isvList.push_back(std::make_pair(1,"yaya"));
    data.isvList.push_back(std::make_pair(2,"yoyo"));
    data.isvList.push_back(std::make_pair(3,"yyyy"));
    std::cout <<"pack: \n" << data <<std::endl;

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, data);

    // unpack
    msgpack::unpacked result;
    msgpack::unpack(&result, sbuf.data(), sbuf.size());

    Data data2;
    data2 = result.get().as<Data>();

    std::cout <<"unpack: \n" << data2 <<std::endl;
}

int main(void)
{
    pack_unpack();

    return 0;

    cclog::reset(new cclog_tty(cclog::TRACE, std::cout));
    signal(SIGPIPE, SIG_IGN);

    // run server {
    msgpack::rpc::server svr;

    std::auto_ptr<msgpack::rpc::dispatcher> dp(new myecho);
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

    return 0;
}
