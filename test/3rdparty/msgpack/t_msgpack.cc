#include "echo_server.h"
#include <iostream>
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/server.h>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/cclog/cclog.h>
#include <3rdparty/msgpack/cclog/cclog_tty.h>
#include <signal.h>




int main(void)
{
    
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
