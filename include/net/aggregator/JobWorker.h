/**
 * @file JobWorker.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief JobWorker performs as a rpc server, it grab jobs to run and return result.
 *
 */
#ifndef JOB_WORKER_H_
#define JOB_WORKER_H_

//#include <hash_map>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/msgpack/rpc/server.h>

#include "AggregatorConfig.h"

namespace net{
namespace aggregator{


class JobWorker : public msgpack::rpc::server::base
{
public:
    JobWorker(const std::string& host, uint16_t port)
    : srvInfo_(host, port)
    {}

public:
    void Start()
    {
        instance.listen(srvInfo_.host_, srvInfo_.port_);
        instance.run(4); //xxx
    }

public:
    /*virtual*/
    void dispatch(msgpack::rpc::request req)
    {
        try
        {
            std::string method;
            req.method().convert(&method);

            if(method == "add")
            {
                msgpack::type::tuple<int, int> params;
                req.params().convert(&params);
                add(req, params.get<0>(), params.get<1>());
            }
            else
            {
                req.error(msgpack::rpc::NO_METHOD_ERROR);
            }
        }
        catch (msgpack::type_error& e)
        {
            req.error(msgpack::rpc::ARGUMENT_ERROR);
            return;
        }
        catch (std::exception& e)
        {
            req.error(std::string(e.what()));
            return;
        }
    }

    // XXX
    void add(msgpack::rpc::request req, int a1, int a2)
    {
        req.result(a1 + a2);
    }

private:
    ServerInfo srvInfo_;

    //typedef  callback_t;
    typedef rde::hash_map<std::string, float > hashmap_t;
    typedef rde::hash_map<std::string, float >::iterator hashmap_iterator_t;


};

}} // end - namespace

#endif /* JOB_WORKER_H_ */
