#include <iostream>
#include <net/aggregator/JobInfo.h>
#include <net/aggregator/JobWorker.h>
#include <net/aggregator/WorkerHandler.h>

#include <boost/shared_ptr.hpp>

#include "data_type.h"

using namespace net::aggregator;


class SearchService
{
public:
    SearchService()
    {
        data_ = "abc 中国 ";
    }

public:
    void getKeywordSearchResult(const string& request, string& result)
    {
        result.assign(data_);
    }

private:
    string data_;
};


class SearchWorker : public JobWorker<SearchWorker>
{

public:
    SearchWorker(const std::string& host, uint16_t port, boost::shared_ptr<SearchService> searchService)
    :JobWorker<SearchWorker>(host, port)
    ,searchService_(searchService)
    {
    }

public:

    /*pure virtual*/
    void addHandlers()
    {
        {
            WorkerHandler<SearchWorker> handler(&SearchWorker::getKeywordSearchResult);
            addHandler("getKeywordSearchResult", handler);
        }

        {
            WorkerHandler<SearchWorker> handler(&SearchWorker::add);
            addHandler("add", handler);
        }
    }

    bool getKeywordSearchResult(JobRequest& req)
    {
        // get request param
        msgpack::type::tuple<Data> params;
        req.params().convert(&params);
        Data request = params.get<0>();

        DataResult result;
        searchService_->getKeywordSearchResult(request.s, result.s);

        req.result(result);
        return true;
    }

    bool add(JobRequest& req)
    {
        msgpack::type::tuple<Data> params;
        req.params().convert(&params);
        Data request = params.get<0>();

        DataResult result;
        result.i = request.i * 2;

        req.result(result);
        return true;
    }

private:
    boost::shared_ptr<SearchService> searchService_;
};


int main( int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: t_worker <host> <port>" << std::endl;
        return -1;
    }

    std::string host = argv[1];
    uint16_t port = atoi(argv[2]);
    std::cout <<"[starting job woker..] "<< host << ":" << port <<std::endl;


    boost::shared_ptr<SearchService> searchService(new SearchService());

    SearchWorker worker(host, port, searchService);
    worker.Start();

    return 0;
}
