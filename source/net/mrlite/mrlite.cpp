#include <stdio.h>

#include <map>
#include <new>
#include <set>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>

#include <net/mrlite/mrlite.h>
#include <net/mrlite/common.h>
#include <net/mrlite/sorted_buffer.h>
#include <net/mrlite/sorted_buffer_iterator.h>
#include <net/mrlite/util.h>

#include "reader.h"
#include "simple_hash.h"
#include "socket_communicator.h"

#include <util/stringprintf.h>

using namespace boost::filesystem;
namespace bfs = boost::filesystem;

namespace net{namespace mrlite{
//-----------------------------------------------------------------------------
// MapReduce context, using poor guy's singleton.
//-----------------------------------------------------------------------------

boost::shared_ptr<Communicator> GetCommunicator()
{
    return MRWorker::Get()->GetCommunicator();
}

boost::shared_ptr<vector<FILE*> > GetOutputFileDescriptors()
{
    return MRWorker::Get()->GetOutputFiles();
}

boost::shared_array<char> GetMapOutputSendBuffer()
{
    return MRWorker::Get()->GetMapOutputSendBuffer();
}

boost::shared_ptr<vector<SortedBuffer*> > GetReduceInputBuffers()
{
    return MRWorker::Get()->GetReduceInputBuffers();
}

int NumReduceWorkers()
{
    return MRWorker::Get()->NumReduceWorkers();
}

int MapOutputBufferSize()
{
    return MRWorker::Get()->MapOutputBufferSize();
}

bool IsBatchReduction()
{
    return MRWorker::Get()->IsBatchReduction();
}

std::string MapOutputBufferFilebase(
    int mapper_id, 
    int reducer_id,
    const string& reduceInputFilebase
) 
{
  // For map worker, to distinguish reduce input buffer files for different
  // reducer workers, mapper-id and reducer-id are appended to filebase.
  return StringPrintf("%s-mapper-%05d-reducer-%05d",
                      reduceInputFilebase.c_str(),
                      mapper_id, reducer_id);
}

//-----------------------------------------------------------------------------
// Implementation of reduce output facilities.
//
// Used by Mapper::Output* in map-only mode and ReducerBase::Output*.
//-----------------------------------------------------------------------------
void WriteText(FILE* output_stream, const string& key, const string& value)
{
    fprintf(output_stream, "%s\n", value.c_str());
}

void ReduceOutput(int channel, const string& key, const string& value)
{
    CHECK_LE(0, channel);
    CHECK_LT(channel, GetOutputFileDescriptors()->size());

    WriteText((*GetOutputFileDescriptors())[channel], key, value);
}

//-----------------------------------------------------------------------------
// Implementation of map output facilities.
//
// Used by Mapper::Output* if it is not in map-only mode.
//-----------------------------------------------------------------------------
void MapOutput(int reduce_worker_id, const string& key, const string& value)
{
    CHECK_LE(0, reduce_worker_id);
    CHECK_LT(reduce_worker_id, NumReduceWorkers());

    uint32* key_size = reinterpret_cast<uint32*>(GetMapOutputSendBuffer().get());
    uint32* value_size = key_size + 1;
    char* data = GetMapOutputSendBuffer().get() + 2 * sizeof(uint32);

    *key_size = key.size();
    *value_size = value.size();

    if (*key_size + *value_size + 2 * sizeof(uint32) > (unsigned)MapOutputBufferSize())
    {
        LOG(FATAL) << "Too large map output, with key = " << key;
    }

    memcpy(data, key.data(), key.size());
    memcpy(data + *key_size, value.data(), value.size());

    if (!IsBatchReduction())
    {
        if (reduce_worker_id >= 0)
        {
            if (GetCommunicator()->Send(GetMapOutputSendBuffer().get(),
                                        *key_size + *value_size + 2 * sizeof(uint32),
                                        reduce_worker_id) < 0)
            {
                LOG(FATAL) << "Send error to reduce worker: " << reduce_worker_id;
            }
        }
        else
        {
            for (int r_id = 0; r_id < NumReduceWorkers(); ++r_id)
            {
                if (GetCommunicator()->Send(
                            GetMapOutputSendBuffer().get(),
                            *key_size + *value_size + 2 * sizeof(uint32),
                            r_id) < 0)
                {
                    LOG(FATAL) << "Send error to reduce worker: " << r_id;
                }
            }
        }
    }
    else
    {
        if (reduce_worker_id >= 0)
        {
            (*GetReduceInputBuffers())[reduce_worker_id]->Insert(
                string(data, *key_size),
                string(data + *key_size, *value_size));
        }
        else
        {
            for (int r_id = 0; r_id < NumReduceWorkers(); ++r_id)
            {
                (*GetReduceInputBuffers())[r_id]->Insert(
                    string(data, *key_size),
                    string(data + *key_size, *value_size));
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Implementation of Mapper:
//-----------------------------------------------------------------------------
int Mapper::Shard(const string& key, int num_reduce_workers)
{
    return JSHash(key) % num_reduce_workers;
}

void Mapper::Output(const string& key, const string& value)
{
    if (IsMapOnly())
    {
        ReduceOutput(0, key, value);
    }
    else
    {
        MapOutput(Shard(key, NumReduceWorkers()), key, value);
    }
}

void Mapper::OutputToShard(
    int reduce_shard,
    const string& key, 
    const string& value
)
{
    if (IsMapOnly())
    {
        LOG(FATAL) << "Must not invoke OutputToShard in map-only mode.";
    }
    else
    {
        MapOutput(reduce_shard, key, value);
    }
}

void Mapper::OutputToAllShards(
    const string& key, 
    const string& value
)
{
    if (IsMapOnly())
    {
        LOG(FATAL) << "Must not invoke OutputToAllShards in map-only mode.";
    }
    else
    {
        MapOutput(-1, key, value);
    }
}

int Mapper::GetNumReduceShards() const
{
    return NumReduceWorkers();
}

bool Mapper::IsMapOnly() const
{
    return mapOnly_;
}

void Mapper::SetMapOnly(bool mapOnly)
{
    mapOnly_ = mapOnly;
}

//-----------------------------------------------------------------------------
// Implementation of ReducerBase:
//-----------------------------------------------------------------------------
void ReducerBase::Output(const string& key, const string& value)
{
    ReduceOutput(0, key, value);
}

void ReducerBase::OutputToChannel(
    int channel,
    const string& key, 
    const string& value
)
{
    ReduceOutput(channel, key, value);
}

int ReducerBase::NumOutputChannels() const
{
    return GetOutputFileDescriptors()->size();
}


//-----------------------------------------------------------------------------
// Implementation of MRWorker:
//-----------------------------------------------------------------------------
MRWorker::MRWorker()
{
    communicator_.reset(new SocketCommunicator);
    output_files_.reset(new vector<FILE*>);
    reduce_input_buffers_.reset(new vector<SortedBuffer*>);	
}

MRWorker::~MRWorker()
{
}

bool MRWorker::Initialize(const MRWorkerContext& config)
{
    workerConfig_ = config;

    // Initialize the communicator.
    if (!config.batchReduction)
    {
        if (!communicator_->Initialize(IamMapWorker(),
                                           config.numMapWorkers,
                                           config.reduceWorkers,
                                           config.queueSize*1024*1024,
                                           config.queueSize*1024*1024,
                                           config.workerId()))
        {
            LOG(ERROR) << "Cannot initialize communicator.";
            return false;
        }
    }

    vector<string> output_files;
    SplitStringUsing(config.mrOutputFiles, ",", &output_files);

    if (IamReduceWorker())
    {
        // Open output files.
        for (unsigned i = 0; i < output_files.size(); ++i)
        {
            FILE* file = fopen(output_files[i].c_str(), "w+");
            if (file == NULL)
            {
                LOG(ERROR) << "Cannot open output file: " << output_files[i];
                return false;
            }
            output_files_->push_back(file);
        }

        // Create map output receive buffer for reduce worker.
        try
        {
            map_output_receive_buffer_.reset(new char[config.maxMapOutputSize_]);
        }
        catch (std::bad_alloc&)
        {
            LOG(ERROR) << "Cannot allocation map output receive buffer with size = "
            << config.maxMapOutputSize_;
            return false;
        }
    }

    if (IamMapWorker())
    {
        // Create map output sending buffer for map worker
        try
        {
            map_output_send_buffer_.reset(new char[config.maxMapOutputSize_]);
        }
        catch (std::bad_alloc&)
        {
            LOG(ERROR) << "Cannot allocation map output send buffer with size = "
            << config.maxMapOutputSize_;
            return false;
        }

        // Create reduce input buffer files for map worker, if in batch mode
        if(config.batchReduction)	
        {
            reduce_input_buffers_->resize(config.reduceWorkers.size());
            try
            {
                for (unsigned i = 0; i < config.reduceWorkers.size(); ++i)
                {
                    (*reduce_input_buffers_)[i] = new SortedBuffer(
                        MapOutputBufferFilebase(config.mapWorkerId,i,config.reduceInputFileBase_), 
                        config.queueSize*1024*1024);
                    LOG(INFO) << "create map output buffer"
                                << i
                                << MapOutputBufferFilebase(config.mapWorkerId,i,config.reduceInputFileBase_);
                }
            }
            catch (const std::bad_alloc&)
            {
                LOG(FATAL) << "Insufficient memory for creating reduce input buffer.";
            }
        }
    }
    return true;
}

void MRWorker::Finalize()
{
    // For map workers, Communicator::Finalize() notifies all reduce
    // workers that this map worker has finished it work and will no
    // longer output anything.  For reduce workers,
    // Communicator::Finalize() releases binding and listening of TCP
    // sockets.
    if (!workerConfig_.batchReduction)
    {
        communicator_->Finalize();
    }

    LOG(INFO) << "MapReduce Lite job finalized.";
}

void MRWorker::MapWork(boost::shared_ptr<Mapper> mapper)
{
    // Clear counters.
    int count_map_input = 0;
    int count_input_shards = 0;

    if( !exists(workerConfig_.mapInputFileBase_) || !is_directory(workerConfig_.mapInputFileBase_) ) 
        LOG(FATAL) << "Failed matching: " << workerConfig_.mapInputFileBase_;

    bfs::directory_iterator iter(workerConfig_.mapInputFileBase_), end_iter;
    for(; iter!= end_iter; ++iter)
    {
        if(bfs::is_regular_file(*iter))
        {
            string currentInputFileName = bfs::path(*iter).string();
            LOG(INFO) << "Mapping input file: " << currentInputFileName;

            boost::scoped_ptr<Reader> reader(CREATE_READER("text"));
            if (reader.get() == NULL)
            {
                LOG(FATAL) << "Creating reader for: " << currentInputFileName;
            }
            reader->Open(currentInputFileName.c_str());

            mapper->Start();

            string key, value;
            while (true)
            {
                if (!reader->Read(&key, &value))
                {
                    break;
                }

                mapper->Map(key, value);
                ++count_map_input;
                if ((count_map_input % 1000) == 0)
                {
                    LOG(INFO) << "Processed " << count_map_input << " records.";
                }
            }

            mapper->Flush();
            ++count_input_shards;
            LOG(INFO) << "Finished mapping file: " << currentInputFileName;
    	}
    }
    LOG(INFO) << "Map worker succeeded:\n"
    << " count_map_input = " << count_map_input << "\n"
    << " count_input_shards = " << count_input_shards << "\n";

    map_output_send_buffer_.reset();

    // Flush all reduce_input_buffers
    if (!mapper->IsMapOnly())
    {
        STLDeleteElementsAndClear(reduce_input_buffers_.get());
    }

}

//-----------------------------------------------------------------------------
// Implementation of reduce worker:
//-----------------------------------------------------------------------------
void MRWorker::ReduceWork(boost::shared_ptr<ReducerBase> reducer)
{
    LOG(INFO) << "Reduce worker in "
    << (workerConfig_.batchReduction? "batch " : "incremental ")
    << "reduction mode.";

    // In order to implement the classical MapReduce API, which defines
    // reduce operation in a ``batch'' way -- reduce is invoked after
    // all reduce values were collected for a map output key.  we
    // employes Berkeley DB to sort and store map outputs arrived in
    // this reduce worker.  Berkeley DB is in response to keep a small
    // memory footprint and does external sort using disk.

    // MRML supports in addition ``incremental'' reduction, where
    // reduce() accepts an intermediate reduce result (represented by a
    // void*, and is NULL for the first value in a reduce input comes)
    // and a reduce value.  It should update the intermediate result
    // using the value.
    typedef map<string, void*> PartialReduceResults;
    boost::scoped_ptr<PartialReduceResults> partial_reduce_results;

    // Initialize partial reduce results, or reduce input buffer.
    if (!workerConfig_.batchReduction)
    {
        partial_reduce_results.reset(new PartialReduceResults);
    }

    // Loop over map outputs arrived in this reduce worker.
    LOG(INFO) << "Start receiving and processing arriving map outputs ...";
    int32 count_reduce = 0;
    int32 count_map_output = 0;
    int receive_status = 0;

    reducer->Start();

    if (!workerConfig_.batchReduction)
    {
        while ((receive_status =
                    GetCommunicator()->Receive(map_output_receive_buffer_.get(),
                                               workerConfig_.maxMapOutputSize_)) > 0)
        {
            ++count_map_output;
            uint32* p = reinterpret_cast<uint32*>(map_output_receive_buffer_.get());
            uint32 key_size = *p;
            uint32 value_size = *(p + 1);
            char* data = map_output_receive_buffer_.get() + sizeof(uint32) * 2;

            string key(data, key_size);
            string value(data + key_size, value_size);

            // Begin a new reduce, which insert a partial result, or does
            // partial reduce, which updates a partial result.
            PartialReduceResults::iterator iter = partial_reduce_results->find(key);
            if (iter == partial_reduce_results->end())
            {
                (*partial_reduce_results)[key] =
                    reinterpret_cast<IncrementalReducer*>(reducer.get())->
                    BeginReduce(key, value);
            }
            else
            {
                reinterpret_cast<IncrementalReducer*>(reducer.get())->
                PartialReduce(key, value, iter->second);
            }

            if ((count_map_output % 5000) == 0)
            {
                LOG(INFO) << "Processed " << count_map_output << " map outputs.";
            }
        }

        if (receive_status < 0)    // Check the reason of breaking while loop.
        {
            LOG(FATAL) << "Communication error at reducer receiving.";
        }
    }

    map_output_receive_buffer_.reset();

    // Invoke EndReduce in incremental reduction mode, or invoke Reduce
    // in batch reduction mode.
    if (!workerConfig_.batchReduction)
    {
        LOG(INFO) << "Finalizing incremental reduction ...";
        for (PartialReduceResults::const_iterator iter =
                    partial_reduce_results->begin();
                iter != partial_reduce_results->end(); ++iter)
        {
            reinterpret_cast<IncrementalReducer*>(reducer.get())->
            EndReduce(iter->first, iter->second);
            // Note: the deletion of iter->second must be done by the user
            // program in EndReduce, because mrml.cc does not know the type of
            // ReducePartialResult defined by the user program.
            ++count_reduce;
        }
        LOG(INFO) << "Succeeded finalizing incremental reduction.";
    }
    else
    {
        LOG(INFO) << "Start batch reduction ...";
        LOG(INFO) << "Creating reduce input iterator ... filebase = "
        << workerConfig_.reduceInputFileBase_
        << " with file num = "
        << workerConfig_.numMapWorkers;
        SortedBufferIteratorImpl reduce_input_iterator(workerConfig_.reduceInputFileBase_,
                workerConfig_.numMapWorkers);
        LOG(INFO) << "Succeeded creating reduce input iterator.";

        for (count_reduce = 0;
                !(reduce_input_iterator.FinishedAll());
                reduce_input_iterator.NextKey(), ++count_reduce)
        {
            reinterpret_cast<BatchReducer*>(reducer.get())->
            Reduce(reduce_input_iterator.key(), &reduce_input_iterator);
            if (count_reduce > 0 && (count_reduce % 5000) == 0)
            {
                LOG(INFO) << "Invoked " << count_reduce << " reduce()s.";
            }
        }

        // remove reduce input buffer files
        for (int i_file = 0; i_file < workerConfig_.numMapWorkers; ++i_file)
        {
            string filename = SortedBuffer::SortedFilename(
                                  workerConfig_.reduceInputFileBase_, i_file);
            LOG(INFO) << "Removing : " << filename
            << " size = "<< boost::filesystem::file_size(filename);
            boost::filesystem::remove(filename);
        }

        LOG(INFO) << "Finished batch reduction.";
    }

    reducer->Flush();

    LOG(INFO) << " count_reduce = " << count_reduce << "\n"
    << " count_map_output = " << count_map_output << "\n";
}

}}  // namespace mrlite

