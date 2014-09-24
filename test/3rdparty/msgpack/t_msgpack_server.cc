#include "echo_server.h"
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/server.h>



class myserver : public msgpack::rpc::server::base {
public:
    void add(msgpack::rpc::request req, int a1, int a2)
    {
        req.result(a1 + a2);
    }

public:
    void dispatch(msgpack::rpc::request req)
    try {
        std::string method;
        req.method().convert(&method);

        if(method == "add") {
            msgpack::type::tuple<int, int> params;
            req.params().convert(&params);
            add(req, params.get<0>(), params.get<1>());

        } else {
            req.error(msgpack::rpc::NO_METHOD_ERROR);
        }

    } catch (msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;

    } catch (std::exception& e) {
        req.error(std::string(e.what()));
        return;
    }
};


void simple_server()
{
    myserver svr;
    svr.instance.listen("0.0.0.0", 9090);
    svr.instance.run(4);  // run 4 threads
}

void myecho_server()
{
    msgpack::rpc::server svr;

    std::unique_ptr<msgpack::rpc::dispatcher> dp(new myecho);
    svr.serve(dp.get());

    svr.listen("0.0.0.0", 18811);

    //svr.start(4);
    svr.run(4);
}

int main(void)
{
    //simple_server();

    //myecho_server();
}
