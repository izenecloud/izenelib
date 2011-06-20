#ifndef ECHO_SERVER_H_
#define ECHO_SERVER_H_

#include <3rdparty/msgpack/rpc/server.h>

// namespace rpc {
// 	using namespace msgpack;
// 	using namespace msgpack::rpc;
// }  // namespace rpc
// using namespace msgpack;


class A
{
    public:
    std::string s;
    int i;
    
    friend inline std::ostream& operator<<(std::ostream& os, A& a)
    {
        os<<a.s<<","<<a.i;
        return os;
    }
    MSGPACK_DEFINE(s, i);
};

class myecho : public msgpack::rpc::dispatcher {
public:
	typedef msgpack::rpc::request request;

	void dispatch(request req)
	try {
		std::string method;
		req.method().convert(&method);

		if(method == "add") {
			msgpack::type::tuple<int, int> params;
			req.params().convert(&params);
			add(req, params.get<0>(), params.get<1>());

		} else if(method == "echo") {
			msgpack::type::tuple<std::string> params;
			req.params().convert(&params);
			echo(req, params.get<0>());

		} else if(method == "echo_a") {
            msgpack::type::tuple<A> params;
            req.params().convert(&params);
            echo_a(req, params.get<0>());

        }else if(method == "echo_huge") {
			msgpack::type::tuple<msgpack::type::raw_ref> params;
			req.params().convert(&params);
			echo_huge(req, params.get<0>());

		} else if(method == "err") {
			msgpack::type::tuple<> params;
			req.params().convert(&params);
			err(req);

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

	void add(request req, int a1, int a2)
	{
		req.result(a1 + a2);
	}

	void echo(request req, const std::string& msg)
	{
		req.result(msg);
	}
    
    void echo_a(request req, const A& a)
    {
        A a2 = a;
        a2.s += "-added-by-server";
        a2.i *= 2;
        req.result(a2);
    }

	void echo_huge(request req, const msgpack::type::raw_ref& msg)
	{
		req.result(msg);
	}

	void err(request req)
	{
		req.error(std::string("always fail"));
	}
};


#endif /* echo_server.h */

