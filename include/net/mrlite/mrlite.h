#ifndef MRLITE_MAPREDUCE_LITE_H_
#define MRLITE_MAPREDUCE_LITE_H_

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <util/singleton.h>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include "sorted_buffer.h"
#include "communicator.h"

namespace net{namespace mrlite{

typedef SortedBufferIterator ReduceInputIterator;

using std::map;
using std::string;
using std::vector;

//-----------------------------------------------------------------------------
//
// Mapper class
//
// *** Basic API ***
//
// In MapReduce Lite, for each map input shard file, a map worker (OS process)
// is created.  The worker creates an object of a derived-class of
// Mapper, then invokes its member functions in the following
// procedure:
//
//  1. Before processing the first key-value pair in the map input
//     shard, it invokes Start().  You may want to override Start() to
//     do shard-specific initializations.
//  2. For each key-value pair in the shard, it invokes Map().
//  3. After all map input pairs were processed, it invokes Flush().
//
// *** Multi-pass Map ***
//
// A unique feature of MapReduce Lite (differs from Google MapReduce
// and Hadoop) is that above procedure may be repeated for multiple
// time, if the command line paramter --multipass_map is set with
// a positive value.  In this case, derived classes can invoke
// GetCurrentPass() to get the current (zero-based) pass id.
//
// *** Sharding ***
//
// As both Google MapReduce API and Hadoop API, programmers can
// specify to where a map output goes by overriding Shard().  In
// addition, similar to Google API (but differs from Hadoop API),
// programmers can also invoke OutputToShard() with a parameter
// specifying the target reduce shard.
//
// *** Output to All Shards ***
//
// A unique feature of MapReduce Lite is OutputToAllShards(), which
// allows a map output goes to all shards.  Some machine learning
// algorithms (e.g., AD-LDA) might find this API useful.
//
// *** NOTE ***
//
// OutputToShard and OutputToAllShards are forbidden in map-only mode.
//
//-----------------------------------------------------------------------------
class Mapper
{
public:
    Mapper():mapOnly_(false){}
    virtual ~Mapper() {}

    virtual void Start() {}
    virtual void Map(const string& key, const string& value) = 0;
    virtual void Flush() {}
    virtual int Shard(const string& key, int num_reduce_shards);
    virtual bool IsMapOnly() const;
    virtual void SetMapOnly(bool mapOnly);
protected:
    virtual void Output(const string& key, const string& value);
    virtual void OutputToShard(int reduce_shard, const string& key, const string& value);
    virtual void OutputToAllShards(const string& key, const string& value);

    const string& CurrentInputFilename() const;
    int GetNumReduceShards() const;

    bool mapOnly_;
};

//-----------------------------------------------------------------------------
//
// IncrementalReducer class (providing a MapReduce Lite-specific API)
//
// *** Combiner ***
//
// IncrementalReducer has two member functions: Start() and Flush():
//
//  1. Before processing a reduce shard, invokes Start();
//  2. For each reduce input (consisting a key and one or more
//     values), invokes BeginReduce(), PartialReduce(), and
//     EndReduce().
//  3. After processing a reduce shard, invokes Flush();
//
// This is similar to Mapper, and allows the implementation of
// combiner pattern in reduce class.
//
// *** Incremental Reduce ***
//
// In standard MapReduce API, thre is a Reduce() function which takes
// all values associated with a key, and programmers override this
// function to process all these values as a whole.  However, in
// practice (both Google MapReduce and Hadoop), the access to these
// values are constrained to be through a forward-only iterator.  This
// constraint, in logic, is equivalent to _incremental reduce_.  In
// MapReduce Lite, we represent incremental reduce by three member functions:
//
//  1. void* BeginReduce(key, value): Given the first value in a
//     reduce input, returns a pointer to intermediate reducing
//     result.
//
//  2. void PartialReduce(key, value, partial_result): For each of the
//     rest values (except for the first value) in current reduce
//     input, update intermediate reducing result.
//
//  3. void EndReduce(partial_result, output): When this function is
//     invoked, partial_result points the _final_ result.  This
//     function should output the final result into a string, which,
//     together with the key of the current reduce input, will be save
//     as a reduce output pair.
//
//-----------------------------------------------------------------------------
class ReducerBase
{
public:
    virtual ~ReducerBase() {}

    virtual void Start() {}
    virtual void Flush() {}

    // Output to the first output channel (the first output file).
    virtual void Output(
        const string& key,
        const string& value
    );

    // Output to the specified output channel.
    virtual void OutputToChannel(
        int channel, 
        const string& key, 
        const string& value
    );

    // The number of output channels. The parameter channel in Output*
    // must be in the range [0, NumOutputChannels() - 1].
    virtual int NumOutputChannels() const;
};


class IncrementalReducer : public ReducerBase
{
public:
    virtual ~IncrementalReducer() {}

    virtual void* BeginReduce(const string& key, const string& value) = 0;
    virtual void PartialReduce(const string& key, const string& value, void* partial_result) = 0;
    virtual void EndReduce(const string& key, void* partial_result) = 0;
};

//-----------------------------------------------------------------------------
//
// In addition to the IncrementalReducer API, MapReduce Lite has BatchReducer
// which provides the same API as Google MapReduce and is suitable for
// processing large scale data; in particular, without the limitation on the
// number of unique map output keys.
//
// *** Batch Reduction ***
//
// The initial design of MRLite is to provide a way which makes it
// possible for map workers and reduce workers work simultaneously.
// The design was realized by Reducer class.  However, this
// design has a limit on the number of unique map output keys, which
// would becomes a servere problem in applications like parallel
// training of language models.  Therefore, we add Reducer, a
// reducer API which is identical to that published in Google papers.
//
//-----------------------------------------------------------------------------
class BatchReducer : public ReducerBase
{
public:
    virtual ~BatchReducer() {}

    virtual void Reduce(const string& key, ReduceInputIterator* values) = 0;
};


//-----------------------------------------------------------------------------
// MRWorkerContext class
// Used to initialized MRWorker, including worker Id, number of tasks,
// parameters to initialize communicators

struct MRWorkerContext
{
    enum MRWorkerMode
    {
        MAP_WORKER,
        REDUCE_WORKER
    };

    MRWorkerContext()
        :mapWorkerId(0)
        ,reduceWorkerId(0)
        ,numMapWorkers(0)
        ,batchReduction(false)
        ,queueSize(1)
    {}

    int workerId() const{
        return workerMode == REDUCE_WORKER? reduceWorkerId : mapWorkerId;
    }

    int mapWorkerId;

    int reduceWorkerId;

    MRWorkerMode workerMode;

    int numMapWorkers;

    vector<string> reduceWorkers;

    //True if mapper and reducer tasks work together
    bool batchReduction;

    string mrOutputFiles;

    //MB size for communicator queue
    int queueSize; 

    int maxMapOutputSize_;

    string reduceInputFileBase_;

    string mapInputFileBase_;
};


//-----------------------------------------------------------------------------
//
// MRWorker class
//
// In MRLite, MRWorker is the practical class that runs Map and Reduce tasks
// During scheduling, the number of mappers or reducers should be kept same for 
// all tasks. MRWorker is always to be kepted as a singleton. For simplicity, we suppose
// that the scheduler will guarantee the tasks to be sequential
// *** Basic API ***
//  1. Initialize  Using MRWorkerContext to initialize MRWorker, including the worker Ids,
//                  number of mappers as well as reducers. Initialize is called once after MRWorker
//                  is constructed.  If the working mode of a MRWorker has changed, such as it
//                  changes from mappers to reducers, scheduler could call Initialize again with
//                  new MRWorkerContext after calling Finalize
//  2. Finalize  Destroy comunicators
//  3. MapWork  Practical process for mapper tasks, which will be called by scheduler
//  4. ReduceWork Practical process for reducer tasks, which will be called by scheduler
//
//  Other APIs are status indicators
class MRWorker
{
public:
    MRWorker();
    ~MRWorker();

    static MRWorker* Get() {
        return ::izenelib::util::Singleton<MRWorker>::get();
    }

    bool Initialize(const MRWorkerContext& config);
    void Finalize();
    void MapWork(boost::shared_ptr<Mapper> mapper);
    void ReduceWork(boost::shared_ptr<ReducerBase> reducer); 

    boost::shared_ptr<Communicator> GetCommunicator(){
        return communicator_;
    }

    boost::shared_ptr<vector<FILE*> > GetOutputFiles() {
        return output_files_;
    }

    boost::shared_array<char> GetMapOutputSendBuffer() { 
        return map_output_send_buffer_;
    }

    boost::shared_ptr<vector<SortedBuffer*> > GetReduceInputBuffers() {
        return reduce_input_buffers_;	
    }

    bool IamMapWorker() const{
        return workerConfig_.workerMode == MRWorkerContext::MAP_WORKER;
    }

    bool IamReduceWorker() const{
        return workerConfig_.workerMode == MRWorkerContext::REDUCE_WORKER;
    }

    int NumReduceWorkers(){
        return workerConfig_.reduceWorkers.size();
    }

    int MapOutputBufferSize(){
        return workerConfig_.maxMapOutputSize_;
    }

    bool IsBatchReduction(){
        return workerConfig_.batchReduction;
    }
private:
    MRWorkerContext workerConfig_;
    boost::shared_ptr<Communicator> communicator_;
    boost::shared_ptr<vector<FILE*> > output_files_;
    boost::shared_array<char> map_output_send_buffer_;
    boost::shared_array<char> map_output_receive_buffer_;
    boost::shared_ptr<vector<SortedBuffer*> > reduce_input_buffers_;	

    DISALLOW_COPY_AND_ASSIGN(MRWorker);
};

}}  // namespace mrlite

#endif  // MRLITE_MAPREDUCE_LITE_H_

