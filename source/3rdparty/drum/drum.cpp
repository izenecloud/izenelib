/*****************************************************************************
 The MIT License

 Copyright (c) 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include <3rdparty/drum/drum.hpp>

#include <sstream>


DRUM_BEGIN_NAMESPACE

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Drum(std::string const& name)
    : merge_buckets_(false)
    , feed_buckets_(false)
    , drum_name_(name)
    , db_default_data_size_(128)
    , db_closed_(false)
{
    this->SetUp();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Drum(std::string const& name, DispatcherType const& dispatcher)
    : merge_buckets_(false)
    , feed_buckets_(false)
    , drum_name_(name)
    , db_default_data_size_(128)
    , db_closed_(false)
    , dispatcher_(dispatcher)
{
    this->SetUp();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Drum(std::string const& name, DispatcherType const& dispatcher, uint32_t dds)
    : merge_buckets_(false)
    , feed_buckets_(false)
    , drum_name_(name)
    , db_default_data_size_(dds)
    , db_closed_(false)
    , dispatcher_(dispatcher)
{
    this->SetUp();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
~Drum()
{
    try
    {
        if (!db_closed_) this->Dispose();
    }
    catch (std::exception & e) { /* Do logging... */ }
    catch (...) { /* Do logging... */ }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
SetUp()
{
    this->AssignFileNames();
    this->CreateRepository();
    this->Init();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
AssignFileNames()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_nt; ++bucket_id)
    {
        std::ostringstream kv_file;
        kv_file << "bucket" << bucket_id << ".kv";
        file_names_[bucket_id].first = kv_file.str();

        std::ostringstream aux_file;
        aux_file << "bucket" << bucket_id << ".aux";
        file_names_[bucket_id].second = aux_file.str();
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CreateRepository()
{
    if (!db_.open(drum_name_))
        throw DrumException("Error creating repository.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Dispose()
{
    db_.close();
    db_closed_ = true;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Init()
{
    this->ResetFilePointers();
    this->ResetNextBufferPositions();
    this->ResetSynchronizationBuffers();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
ResetFilePointers()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_nt; ++bucket_id)
        current_pointers_[bucket_id].first = current_pointers_[bucket_id].second = std::ios_base::beg;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
ResetNextBufferPositions()
{
    //Boost array's size() method cannot be used for the number of *valid* elements since it's
    //statically bound to N.
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_nt; ++bucket_id)
        next_positions_[bucket_id] = 0;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
ResetSynchronizationBuffers()
{
    sorted_merge_buffer_.clear();
    sorted_merge_buffer_.reserve(bucket_buff_elem_size_nt); //At least.
    unsorting_helper_.clear();
    unsorting_helper_.reserve(bucket_buff_elem_size_nt);
    unsorted_aux_buffer_.clear();
    unsorted_aux_buffer_.reserve(bucket_buff_elem_size_nt);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
OpenFile(std::string const& file_name, std::fstream & file)
{
    file.open(file_name.c_str(), std::ios_base::out | std::ios_base::in | std::ios_base::binary);
    if (!file.good()) throw DrumException("Error opening disk bucket.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CloseFile(std::fstream & file)
{
    if (file.is_open()) file.close();
    if (file.fail()) throw DrumException("Exception closing disk bucket.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
GetBucketAndBufferPos(key_t const& key)
{
    std::size_t bucket_id = BucketIdentififer<key_t, num_buckets_nt>::Calculate(key);
    std::size_t pos = next_positions_[bucket_id]++; //Notice this increments the position.

    assert(pos <= bucket_buff_elem_size_nt && "Position must not be larger than buffer size.");

    if (next_positions_[bucket_id] == bucket_buff_elem_size_nt)
        feed_buckets_ = true;

    return std::make_pair(bucket_id, pos);
}
template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Add(key_t const& key, char op)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->GetBucketAndBufferPos(key);
    CompoundType & element = kv_buffers_[bucket_pos.first][bucket_pos.second];
    this->Make(element, key, op);
    return bucket_pos;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Add(key_t const& key, value_t const& value, char op)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->GetBucketAndBufferPos(key);
    CompoundType & element = kv_buffers_[bucket_pos.first][bucket_pos.second];
    this->Make(element, key, value, op);
    return bucket_pos;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Make(CompoundType & element, key_t const& key, char op) const
{
    element.template get<0>() = key;
    element.template get<2>() = op;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Make(CompoundType & element, key_t const& key, value_t const& value, char op) const
{
    this->Make(element, key, op);
    element.template get<1>() = value;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Make(CompoundType & element,
        key_t const& key,
        value_t const& value,
        char op,
        std::size_t position) const
{
    this->Make(element, key, value, op);
    element.template get<3>() = position;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CheckTimeToFeed()
{
    if (feed_buckets_) this->FeedBuckets();

    this->CheckTimeToMerge();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CheckTimeToMerge()
{
    if (merge_buckets_) this->MergeBuckets();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
FeedBuckets()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_nt; ++bucket_id)
        this->FeedBucket(bucket_id);

    this->ResetNextBufferPositions();
    feed_buckets_ = false;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
FeedBucket(std::size_t bucket_id)
{
    //Positions in each aux buffer is controlled by position in the corresponding info buffer.
    std::size_t current = next_positions_[bucket_id];
    if (0 == current) return;

    std::fstream kv_file;
    this->OpenFile(file_names_[bucket_id].first, kv_file);
    std::streampos kv_begin = kv_file.tellp();

    std::fstream aux_file;
    this->OpenFile(file_names_[bucket_id].second, aux_file);
    std::streampos aux_begin = aux_file.tellp();

    //Set current pointers of the files.
    kv_file.seekp(current_pointers_[bucket_id].first);
    aux_file.seekp(current_pointers_[bucket_id].second);

    for (std::size_t i = 0; i < current; ++i)
    {
        //Write the following sequentially:
        // - operation;
        // - key length;
        // - key;
        // - value length;
        // - value.
        CompoundType const& element = kv_buffers_[bucket_id][i];

        //Operation.
        char op = element.template get<2>();
        kv_file.write(&op, sizeof(char));

        //Key.
        std::size_t key_size;
        const char* key_serial;
        KeyType const& key = element.template get<0>();
        ElementIO<KeyType>::Serialize(key, key_size, key_serial);
        kv_file.write(reinterpret_cast<const char*>(&key_size), sizeof(std::size_t));
        kv_file.write(key_serial, key_size);

        //Value.
        std::size_t value_size;
        const char* value_serial;
        ValueType const& value = element.template get<1>();
        ElementIO<ValueType>::Serialize(value, value_size, value_serial);
        kv_file.write(reinterpret_cast<const char*>(&value_size), sizeof(std::size_t));
        kv_file.write(value_serial, value_size);

        //Write the following sequentially:
        // - aux length;
        // - aux.
        AuxType const& aux = aux_buffers_[bucket_id][i];

        std::size_t aux_size;
        const char* aux_serial;
        ElementIO<AuxType>::Serialize(aux, aux_size, aux_serial);
        aux_file.write(reinterpret_cast<const char*>(&aux_size), sizeof(std::size_t));
        aux_file.write(aux_serial, aux_size);
    }

    //Store pointers for the next feed.
    current_pointers_[bucket_id].first = kv_file.tellp();
    current_pointers_[bucket_id].second = aux_file.tellp();

    this->CloseFile(kv_file);
    this->CloseFile(aux_file);

    //Is it time to merge?
    if (current_pointers_[bucket_id].first - kv_begin > bucket_byte_size_nt ||
            current_pointers_[bucket_id].second - aux_begin > bucket_byte_size_nt)
    {
        merge_buckets_ = true;
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
MergeBuckets()
{
    //Merge with one-pass through disk repository.
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_nt; ++bucket_id)
    {
        this->ReadInfoBucketIntoMergeBuffer(bucket_id);
        this->SortMergeBuffer();
        this->SynchronizeWithDisk();
        this->UnsortMergeBuffer();
        this->ReadAuxBucketForDispatching(bucket_id);
        this->Dispatch();
        this->ResetSynchronizationBuffers();
    }

    this->ResetFilePointers();
    merge_buckets_ = false;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
ReadInfoBucketIntoMergeBuffer(std::size_t bucket_id)
{
    std::fstream kv_file;
    this->OpenFile(file_names_[bucket_id].first, kv_file);
    std::streampos kv_written = current_pointers_[bucket_id].first;

    while (kv_file.tellg() < kv_written)
    {
        //It would be nice to have semantics to move the element into the container. To avoid an
        //extra copy I insert the element and use a reference to it.
        sorted_merge_buffer_.push_back(CompoundType());
        CompoundType & element = sorted_merge_buffer_.back();

        //Keep track of the element's original order in file.
        std::size_t & order = element.template get<3>();
        order = sorted_merge_buffer_.size() - 1;

        //Operation.
        char & op = element.template get<2>();
        kv_file.read(&op, sizeof(char));

        //Key.
        KeyType & key = element.template get<0>();
        std::size_t key_size;
        kv_file.read(reinterpret_cast<char*>(&key_size), sizeof(std::size_t));
        std::vector<char> key_serial(key_size);
        kv_file.read(&key_serial[0], key_size);
        ElementIO<KeyType>::Deserialize(key, key_size, &key_serial[0]);

        //Value.
        ValueType & value = element.template get<1>();
        std::size_t value_size;
        kv_file.read(reinterpret_cast<char*>(&value_size), sizeof(std::size_t));
        std::vector<char> value_serial(value_size);
        kv_file.read(&value_serial[0], value_size);
        ElementIO<ValueType>::Deserialize(value, value_size, &value_serial[0]);
    }

    this->CloseFile(kv_file);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
SortMergeBuffer()
{
    std::sort(sorted_merge_buffer_.begin(), sorted_merge_buffer_.end(), KeyCompare());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
SynchronizeWithDisk()
{
    //Fast Berkeley DB queries are expected due to the locality of the keys in the bucket (and
    //consequently in the merge buffer).

    int ret;
    for (typename std::vector<CompoundType>::size_type i = 0; i < sorted_merge_buffer_.size(); ++i)
    {
        CompoundType & element = sorted_merge_buffer_[i];

        KeyType const& key = element.template get<0>();

        char op = element.template get<2>();
        assert((op == CHECK || op == CHECK_UPDATE || op == UPDATE) && "Impossible operation.");

        if (CHECK == op || CHECK_UPDATE == op)
        {
            ValueType value;
            if (!db_.get(key, value))
                element.template get<4>() = UNIQUE_KEY;
            else
            {
                element.template get<4>() = DUPLICATE_KEY;

                //Retrieve the value associated to the key only if it's a check operation.
                if (CHECK == op)
                {
                    //Set the info.
                    element.template get<1>() = value;
                }
            }
        }

        if (UPDATE == op || CHECK_UPDATE == op)
        {
            ValueType const& value = element.template get<1>();

            if (!db_.put(key, value)) //Overwrite if the key is already present.
                throw DrumException("Error merging with repository.");
        }
    }

    //Persist.
    if (!db_.flush()) throw DrumException("Error persisting repository.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
UnsortMergeBuffer()
{
    //When elements were read into the merge buffer their original positions were stored.
    //Now I use those positions as keys to "sort" them back in linear time.
    //Traversing unsorting_helper_ gives the indexes into sorted_merge_buffer considering the original
    //order. (I use only the indexes to avoid element copies.)
    std::size_t total = static_cast<std::size_t>(sorted_merge_buffer_.size());
    unsorting_helper_.resize(total);
    for (std::size_t i = 0; i < total; ++i)
    {
        CompoundType const& element = sorted_merge_buffer_[i];
        std::size_t original = element.template get<3>();
        unsorting_helper_[original] = i;
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
ReadAuxBucketForDispatching(std::size_t bucket_id)
{
    std::fstream aux_file;
    this->OpenFile(file_names_[bucket_id].second, aux_file);
    std::streampos aux_written = current_pointers_[bucket_id].second;

    while (aux_file.tellg() < aux_written)
    {
        unsorted_aux_buffer_.push_back(AuxType());
        AuxType & aux = unsorted_aux_buffer_.back();

        std::size_t aux_size;
        aux_file.read(reinterpret_cast<char*>(&aux_size), sizeof(std::size_t));
        std::vector<char> aux_serial(aux_size);
        aux_file.read(&aux_serial[0], aux_size);
        ElementIO<AuxType>::Deserialize(aux, aux_size, &aux_serial[0]);
    }

    this->CloseFile(aux_file);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Dispatch()
{
    std::size_t total = static_cast<std::size_t>(unsorting_helper_.size());
    for (std::size_t i = 0; i < total; ++i)
    {
        std::size_t element_index = unsorting_helper_[i];
        CompoundType const& element = sorted_merge_buffer_[element_index];
        KeyType const& key = element.template get<0>();
        char op = element.template get<2>();
        char result = element.template get<4>();

        AuxType const& aux = unsorted_aux_buffer_[i];

        if (CHECK == op && UNIQUE_KEY == result)
            dispatcher_.UniqueKeyCheck(key, aux);
        else
        {
            ValueType const& value = element.template get<1>();

            if (CHECK == op && DUPLICATE_KEY == result)
                dispatcher_.DuplicateKeyCheck(key, value, aux);
            else if (CHECK_UPDATE == op && UNIQUE_KEY == result)
                dispatcher_.UniqueKeyUpdate(key, value, aux);
            else if (CHECK_UPDATE == op && DUPLICATE_KEY == result)
                dispatcher_.DuplicateKeyUpdate(key, value, aux);
            else if (UPDATE == op)
                dispatcher_.Update(key, value, aux);
            else assert("Invalid combination of operation and result.");
        }
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Check(key_t const& key)
{
    this->Check(key, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Check(key_t const& key, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, CHECK);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();

}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CheckUpdate(key_t const& key, value_t const& value)
{
    CheckUpdate(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
CheckUpdate(key_t const& key, value_t const& value, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, value, CHECK_UPDATE);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Update(key_t const& key, value_t const& value)
{
    Update(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Update(key_t const& key, value_t const& value, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, value, UPDATE);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    std::size_t num_buckets_nt,
    std::size_t bucket_buff_elem_size_nt,
    std::size_t bucket_byte_size_nt,
    template <class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    num_buckets_nt,
    bucket_buff_elem_size_nt,
    bucket_byte_size_nt,
    ordered_db_t,
    dispatcher_t>::
Synchronize()
{
    this->FeedBuckets();
    this->MergeBuckets();
}

DRUM_END_NAMESPACE
