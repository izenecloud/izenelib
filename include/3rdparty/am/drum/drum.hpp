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

#ifndef IZENELIB_DISK_REPOSITORY_UPDATE_MANAGEMENT_HPP
#define IZENELIB_DISK_REPOSITORY_UPDATE_MANAGEMENT_HPP

#include "config.hpp"
#include "bucket_identifier.hpp"
#include "exception.hpp"
#include "null_dispatcher.hpp"

//Boost.
#include <boost/tuple/tuple.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

//STL.
#include <cassert>
#include <sstream>
#include <fstream>
#include <vector>

#include <glog/logging.h>


DRUM_BEGIN_NAMESPACE

/*
 * NOTES:
 *
 * - Drum is not thread-safe.
 * - It is advisable the key_t, value_t, and aux_t are cheap to copy.
 * - Elements are stored in a boost::array, which is an aggregate and consequently provides no
 *   guarantees about initialization. Therefore, when auxiliary data is not passed, I use dummy
 *   (default constructed) objects.
 * - Attempt to use Drum after disposal will eventually result in error.
 * - Check the documentation that comes with the source for an overview.
 */

class DefaultMerger
{
public:
    template <class T>
    bool Append(T& left, const T& right) const
    {
        return false;
    }

    template <class T1, class T2>
    bool Append(std::basic_string<T1, T2>& left, const std::basic_string<T1, T2>& right) const
    {
        left.append(right);

        return true;
    }

    template <class T1, class T2>
    bool Append(std::vector<T1, T2>& left, const std::vector<T1, T2>& right) const
    {
        left.insert(left.end(), right.begin(), right.end());

        return true;
    }

    template <class T1, class T2, class T3>
    bool Append(std::set<T1, T2, T3>& left, const std::set<T1, T2, T3>& right) const
    {
        std::size_t old_size = left.size();
        left.insert(right.begin(), right.end());

        return left.size() > old_size;
    }

    template <class T1, class T2, class T3, class T4>
    bool Append(std::map<T1, T2, T3, T4>& left, const std::map<T1, T2, T3, T4>& right) const
    {
        std::size_t old_size = left.size();
        left.insert(right.begin(), right.end());

        return left.size() > old_size;
    }

    template <class T>
    bool Expel(T& left, const T& right) const
    {
        return false;
    }

    template <class T1, class T2, class T3>
    bool Expel(std::set<T1, T2, T3>& left, const std::set<T1, T2, T3>& right) const
    {
        if (left.empty()) return false;

        std::size_t old_size = left.size();
        left.erase(right.begin(), right.end());

        return left.size() < old_size;
    }

    template <class T1, class T2, class T3, class T4>
    bool Expel(std::map<T1, T2, T3, T4>& left, const std::map<T1, T2, T3, T4>& right) const
    {
        if (left.empty()) return false;

        std::size_t old_size = left.size();
        for (typename std::map<T1, T2, T3, T4>::const_iterator it = right.begin();
                it != right.end(); ++it)
        {
            left.erase(it->first);
        }

        return left.size() < old_size;
    }
};

template<class key_t,
         class value_t,
         class aux_t,
         template <class> class key_comp_t,
         template <class, class, class> class ordered_db_t,
         template <class, class, class> class dispatcher_t = NullDispatcher,
         class merger_t = DefaultMerger>
class Drum : public boost::noncopyable
{
public:
    typedef key_t KeyType;
    typedef value_t ValueType;
    typedef aux_t AuxType;
    typedef key_comp_t<key_t> KeyCompType;
    typedef ordered_db_t<key_t, value_t, key_comp_t<key_t> > DbType;
    typedef dispatcher_t<key_t, value_t, aux_t> DispatcherType;
    typedef merger_t MergerType;

private:
    typedef boost::tuple<key_t,
                         value_t,
                         char,         //Operation (check, update, check/update).
                         std::size_t,  //Position in the bucket.
                         char          //Operation result (unique, duplicate).
                        > CompoundType;
    typedef std::vector<CompoundType> CompoundBucketBuffer;
    typedef std::vector<CompoundBucketBuffer> CompoundBucketBufferContainer;
    typedef std::vector<AuxType> AuxBucketBuffer;
    typedef std::vector<AuxBucketBuffer> AuxBucketBufferContainer;
    typedef std::vector<std::size_t> NextBufferPositionsContainer;

    typedef std::vector<std::pair<std::string, std::string> > BucketFileNamesContainer;
    typedef std::vector<std::pair<std::streampos, std::streampos> > BucketFilePointersContainer;

    //Other compare method different than "less than" doesn't fit well in the Drum architecture.
    struct KeyCompare
    {
        bool operator()(CompoundType const& a, CompoundType const& b)
        {
            KeyType const& ka = a.template get<0>();
            KeyType const& kb = b.template get<0>();
            return ka < kb; //Must be available for key type.
        }
    };

    //Setup/cleanup.
    void SetUp();
    void AssignFileNames();
    void CreateRepository();

    //Initialization.
    void Init();
    void ResetFilePointers();
    void ResetNextBufferPositions();
    void ResetSynchronizationBuffers();

    //Utility.
    void InitFile(std::string const& file_name);
    void OpenFile(std::string const& file_name, std::fstream & bucket_file);
    void CloseFile(std::fstream & bucket_file);

    //Business.
    std::pair<std::size_t, std::size_t> GetBucketAndBufferPos(key_t const&);
    void Make(CompoundType &, key_t const&, char) const;
    void Make(CompoundType &, key_t const&, value_t const&, char) const;
    void Make(CompoundType &, key_t const&, value_t const&, char, std::size_t) const;
    std::pair<std::size_t, std::size_t> Add(key_t const&, char);
    std::pair<std::size_t, std::size_t> Add(key_t const&, value_t const&, char);
    void CheckTimeToFeed();
    void CheckTimeToMerge();
    void FeedBuckets();
    void FeedBucket(std::size_t);
    void MergeBuckets();
    void ReadInfoBucketIntoMergeBuffer(std::size_t);
    void SortMergeBuffer();
    void UnsortMergeBuffer();
    void SynchronizeWithDisk();
    void ReadAuxBucketForDispatching(std::size_t);
    void Dispatch();
    void CaptureBufferSpace();
    void ReleaseBufferSpace();
    void TruncateBuckets();

public:
    Drum(std::string const& name,
         std::size_t num_buckets,
         std::size_t bucket_buff_elem_size,
         std::size_t bucket_byte_size,
         DispatcherType const& dispatcher = DispatcherType(),
         MergerType const& merger = MergerType()
    );
    ~Drum(); //Not intended to be inherited.

    //Check/update operations.
    void Check(key_t const&);
    void Check(key_t const&, aux_t const&);
    void Update(key_t const&, value_t const&);
    void Update(key_t const&, value_t const&, aux_t const&);
    void CheckUpdate(key_t const&, value_t const&);
    void CheckUpdate(key_t const&, value_t const&, aux_t const&);
    void Delete(key_t const&);
    void Delete(key_t const&, aux_t const&);
    void CheckDelete(key_t const&);
    void CheckDelete(key_t const&, aux_t const&);
    void Append(key_t const&, value_t const&);
    void Append(key_t const&, value_t const&, aux_t const&);
    void Expel(key_t const&, value_t const&);
    void Expel(key_t const&, value_t const&, aux_t const&);

    void Synchronize(); //Force synchronization (merge).
    void Dispose(); //Done with Drum.

    bool GetValue(key_t const&, value_t &);

private:
    bool merge_buckets_;
    bool feed_buckets_;
    std::string drum_name_;
    DbType db_; //Disk repository.
    bool db_closed_;
    DispatcherType const& dispatcher_;
    MergerType const& merger_;

    //According to Effective C++ it's not necessary to provide definitions for such integral constant
    //variables as long as their addresses are not taken.
    static const char CHECK = 0;
    static const char UPDATE = 1;
    static const char CHECK_UPDATE = 2;
    static const char DELETE = 3;
    static const char CHECK_DELETE = 4;
    static const char APPEND = 5;
    static const char EXPEL = 6;

    static const char UNIQUE_KEY = 100;
    static const char DUPLICATE_KEY = 101;

    std::size_t const num_buckets_;
    std::size_t const bucket_buff_elem_size_;
    std::size_t const bucket_byte_size_;
    std::size_t num_bucket_bits_;

    std::vector<CompoundType> sorted_merge_buffer_; //Used during merge with disk.
    std::vector<std::size_t> unsorting_helper_; //Original positions of elements in buckets.
    std::vector<AuxType> unsorted_aux_buffer_; //Used in dispatching after merge.

    CompoundBucketBufferContainer kv_buffers_;
    AuxBucketBufferContainer aux_buffers_;
    NextBufferPositionsContainer next_positions_; //For key/value and aux buffers.

    BucketFileNamesContainer file_names_;
    BucketFilePointersContainer current_pointers_;

    boost::mutex mutex_;
};

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Drum(std::string const& name,
     std::size_t num_buckets,
     std::size_t bucket_buff_elem_size,
     std::size_t bucket_byte_size,
     DispatcherType const& dispatcher,
     MergerType const& merger
)
    : merge_buckets_(false)
    , feed_buckets_(false)
    , drum_name_(name)
    , db_closed_(false)
    , dispatcher_(dispatcher)
    , merger_(merger)
    , num_buckets_(num_buckets)
    , bucket_buff_elem_size_(bucket_buff_elem_size)
    , bucket_byte_size_(bucket_byte_size)
    , num_bucket_bits_(0)
    , next_positions_(num_buckets_)
    , file_names_(num_buckets_)
    , current_pointers_(num_buckets_)
{
    while (num_buckets >>= 1)
    {
        ++num_bucket_bits_;
    }
    if (num_buckets_ != static_cast<std::size_t>(1) << num_bucket_bits_)
        ++num_bucket_bits_;

    this->SetUp();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
~Drum()
{
    try
    {
        if (!db_closed_)
        {
            this->Synchronize();
            this->Dispose();
        }
    }
    catch (std::exception & e) { /* Do logging... */ }
    catch (...) { /* Do logging... */ }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
AssignFileNames()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
    {
        std::ostringstream kv_file;
        kv_file << drum_name_ << "/" << "bucket" << bucket_id << ".kv";
        file_names_[bucket_id].first = kv_file.str();
        InitFile(file_names_[bucket_id].first);

        std::ostringstream aux_file;
        aux_file << drum_name_ << "/"  << "bucket" << bucket_id << ".aux";
        file_names_[bucket_id].second = aux_file.str();
        InitFile(file_names_[bucket_id].second);
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CreateRepository()
{
    if (!db_.open(drum_name_ + "/ordered_db"))
        throw DrumException("Error creating repository.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Dispose()
{
    db_.close();
    db_closed_ = true;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Init()
{
    this->ResetFilePointers();
    this->ResetNextBufferPositions();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ResetFilePointers()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
        current_pointers_[bucket_id].first = current_pointers_[bucket_id].second = std::ios_base::beg;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ResetNextBufferPositions()
{
    //Boost array's size() method cannot be used for the number of *valid* elements since it's
    //statically bound to N.
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
        next_positions_[bucket_id] = 0;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ResetSynchronizationBuffers()
{
    sorted_merge_buffer_.clear();
    unsorting_helper_.clear();
    unsorted_aux_buffer_.clear();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
InitFile(std::string const& file_name)
{
    // Create empty file if not existed yet
    std::ifstream in_f;
    in_f.open(file_name.c_str());

    if (!in_f.is_open())
    {
        std::ofstream out_f;
        out_f.open(file_name.c_str());
        if (!out_f.good())
        {
            std::string ex("Error creating disk bucket: ");
            ex.append(file_name);
            throw DrumException(ex.c_str());
        }
        out_f.close();
    }

    in_f.close();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
OpenFile(std::string const& file_name, std::fstream & file)
{
    file.open(file_name.c_str(), std::ios_base::out | std::ios_base::in | std::ios_base::binary);
    if (!file.good()) throw DrumException("Error opening disk bucket.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CloseFile(std::fstream & file)
{
    if (file.is_open()) file.close();
    if (file.fail()) throw DrumException("Exception closing disk bucket.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
GetBucketAndBufferPos(key_t const& key)
{
    std::size_t bucket_id = std::min(num_buckets_ - 1, BucketIdentifier<KeyType>::Calculate(key, num_bucket_bits_));
    std::size_t pos = next_positions_[bucket_id]++; //Notice this increments the position.

    assert(pos < bucket_buff_elem_size_ && "Position must not be larger than buffer size.");

    if (next_positions_[bucket_id] == bucket_buff_elem_size_)
        feed_buckets_ = true;

    return std::make_pair(bucket_id, pos);
}
template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Add(key_t const& key, char op)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->GetBucketAndBufferPos(key);
    if (kv_buffers_.empty()) this->CaptureBufferSpace();
    CompoundType & element = kv_buffers_[bucket_pos.first][bucket_pos.second];
    this->Make(element, key, op);
    return bucket_pos;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
std::pair<std::size_t, std::size_t>
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Add(key_t const& key, value_t const& value, char op)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->GetBucketAndBufferPos(key);
    if (kv_buffers_.empty()) this->CaptureBufferSpace();
    CompoundType & element = kv_buffers_[bucket_pos.first][bucket_pos.second];
    this->Make(element, key, value, op);
    return bucket_pos;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Make(CompoundType & element, key_t const& key, char op) const
{
    element.template get<0>() = key;
    element.template get<2>() = op;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Make(CompoundType & element, key_t const& key, value_t const& value, char op) const
{
    this->Make(element, key, op);
    element.template get<1>() = value;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CheckTimeToFeed()
{
    if (feed_buckets_) this->FeedBuckets();

    this->CheckTimeToMerge();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CheckTimeToMerge()
{
    if (merge_buckets_) this->MergeBuckets();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
FeedBuckets()
{
    std::cout << "start FeedBuckets (" << num_buckets_ << ")" << std::endl;

    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
        this->FeedBucket(bucket_id);

    this->ResetNextBufferPositions();
    feed_buckets_ = false;

    std::cout << "end FeedBuckets" << std::endl;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
        {
            std::size_t key_size;
            char* key_serial;
            KeyType const& key = element.template get<0>();
            izenelib::util::izene_serialization<KeyType> izsKey(key);
            izsKey.write_image(key_serial, key_size);
            kv_file.write(reinterpret_cast<const char*>(&key_size), sizeof(std::size_t));
            kv_file.write(key_serial, key_size);
        }

        //Value.
        {
            std::size_t value_size;
            char* value_serial;
            ValueType const& value = element.template get<1>();
            izenelib::util::izene_serialization<ValueType> izsValue(value);
            izsValue.write_image(value_serial, value_size);
            kv_file.write(reinterpret_cast<const char*>(&value_size), sizeof(std::size_t));
            kv_file.write(value_serial, value_size);
        }

        //Write the following sequentially:
        // - aux length;
        // - aux.
        {
            AuxType const& aux = aux_buffers_[bucket_id][i];

            std::size_t aux_size;
            char* aux_serial;
            izenelib::util::izene_serialization<AuxType> izsAux(aux);
            izsAux.write_image(aux_serial, aux_size);
            aux_file.write(reinterpret_cast<const char*>(&aux_size), sizeof(std::size_t));
            aux_file.write(aux_serial, aux_size);
        }
    }

    //Store pointers for the next feed.
    current_pointers_[bucket_id].first = kv_file.tellp();
    current_pointers_[bucket_id].second = aux_file.tellp();

    this->CloseFile(kv_file);
    this->CloseFile(aux_file);

    //Is it time to merge?
    std::streamoff kv_feed_size = current_pointers_[bucket_id].first - kv_begin;
    std::streamoff aux_feed_size = current_pointers_[bucket_id].second - aux_begin;
    if ((kv_feed_size > 0 && static_cast<std::size_t>(kv_feed_size) > bucket_byte_size_) ||
            (aux_feed_size > 0 && static_cast<std::size_t>(aux_feed_size) > bucket_byte_size_))
    {
        merge_buckets_ = true;
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
MergeBuckets()
{
    std::cout << "start MergeBuckets (" << num_buckets_ << ") " << std::endl;

    //Merge with one-pass through disk repository.
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
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

    std::cout << "end MergeBuckets" << std::endl;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ReadInfoBucketIntoMergeBuffer(std::size_t bucket_id)
{
    std::fstream kv_file;
    this->OpenFile(file_names_[bucket_id].first, kv_file);
    std::streampos kv_written = current_pointers_[bucket_id].first;

    while (kv_file.tellg() < (kv_written - std::streampos(0)))
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
        {
            KeyType & key = element.template get<0>();
            std::size_t key_size;
            kv_file.read(reinterpret_cast<char*>(&key_size), sizeof(std::size_t));
            char key_serial[key_size];
            kv_file.read(key_serial, key_size);
            izenelib::util::izene_deserialization<KeyType> izsKey(key_serial, key_size);
            izsKey.read_image(key);
        }

        //Value.
        {
            ValueType & value = element.template get<1>();
            std::size_t value_size;
            kv_file.read(reinterpret_cast<char*>(&value_size), sizeof(std::size_t));
            char value_serial[value_size];
            kv_file.read(value_serial, value_size);
            izenelib::util::izene_deserialization<ValueType> izsValue(value_serial, value_size);
            izsValue.read_image(value);
        }
    }

    this->CloseFile(kv_file);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
SortMergeBuffer()
{
    std::sort(sorted_merge_buffer_.begin(), sorted_merge_buffer_.end(), KeyCompare());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
SynchronizeWithDisk()
{
    //Fast Berkeley DB queries are expected due to the locality of the keys in the bucket (and
    //consequently in the merge buffer).

    KeyType merge_key;
    ValueType merge_value;
    bool merge_dirty = false;

    for (typename std::vector<CompoundType>::size_type i = 0; i < sorted_merge_buffer_.size(); ++i)
    {
        CompoundType & element = sorted_merge_buffer_[i];

        KeyType const& key = element.template get<0>();

        char op = element.template get<2>();
//      assert((op == CHECK || op == UPDATE || op == CHECK_UPDATE || op == DELETE || op == CHECK_DELETE || op == APPEND || op == EXPEL) && "Impossible operation.");

        if (CHECK == op || CHECK_UPDATE == op || CHECK_DELETE == op)
        {
            ValueType value;

            boost::lock_guard<boost::mutex> lock(mutex_);
            if (!db_.get(key, value))
                element.template get<4>() = UNIQUE_KEY;
            else
            {
                element.template get<4>() = DUPLICATE_KEY;

                //Retrieve the value associated to the key only if it's a check operation.
                if (CHECK_UPDATE != op)
                {
                    //Set the info.
                    element.template get<1>() = value;
                }
            }
        }

        if (UPDATE == op || CHECK_UPDATE == op)
        {
            ValueType const& value = element.template get<1>();

            boost::lock_guard<boost::mutex> lock(mutex_);
            if (!db_.update(key, value)) //Overwrite if the key is already present.
                throw DrumException("Error merging with repository.");
        }

        if (DELETE == op || CHECK_DELETE == op)
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            if (!db_.del(key) && DUPLICATE_KEY == element.template get<4>())
                throw DrumException("Error merging with repository.");
        }

        if (APPEND == op)
        {
            ValueType const& value = element.template get<1>();

            if (i == 0)
            {
                merge_key = key;

                boost::lock_guard<boost::mutex> lock(mutex_);
                db_.get(merge_key, merge_value);
            }
            else if (merge_key != key)
            {
                if (merge_dirty)
                {
                    merge_dirty = false;
                    boost::lock_guard<boost::mutex> lock(mutex_);
                    if (!db_.update(merge_key, merge_value))
                        throw DrumException("Error merging with repository.");
                }

                merge_key = key;
                merge_value = ValueType();

                boost::lock_guard<boost::mutex> lock(mutex_);
                db_.get(merge_key, merge_value);
            }

            if (merger_.Append(merge_value, value))
            {
                merge_dirty = true;
                element.template get<4>() = UNIQUE_KEY;
            }
            else
                element.template get<4>() = DUPLICATE_KEY;
        }

        if (EXPEL == op)
        {
            ValueType const& value = element.template get<1>();

            if (i == 0)
            {
                merge_key = key;

                boost::lock_guard<boost::mutex> lock(mutex_);
                db_.get(merge_key, merge_value);
            }
            else if (merge_key != key)
            {
                if (merge_dirty)
                {
                    merge_dirty = false;
                    boost::lock_guard<boost::mutex> lock(mutex_);
                    if (!db_.update(merge_key, merge_value))
                        throw DrumException("Error merging with repository.");
                }

                merge_key = key;
                merge_value = ValueType();

                boost::lock_guard<boost::mutex> lock(mutex_);
                db_.get(merge_key, merge_value);
            }

            if (merger_.Expel(merge_value, value))
            {
                merge_dirty = true;
                element.template get<4>() = UNIQUE_KEY;
            }
            else
                element.template get<4>() = DUPLICATE_KEY;
        }
    }

    if (merge_dirty)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        if (!db_.update(merge_key, merge_value))
            throw DrumException("Error merging with repository.");
    }

    //Persist.
    boost::lock_guard<boost::mutex> lock(mutex_);
    if (!db_.flush()) throw DrumException("Error persisting repository.");
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
UnsortMergeBuffer()
{
    //When elements were read into the merge buffer their original positions were stored.
    //Now I use those positions as keys to "sort" them back in linear time.
    //Traversing unsorting_helper_ gives the indexes into sorted_merge_buffer considering the original
    //order. (I use only the indexes to avoid element copies.)
    std::size_t total = sorted_merge_buffer_.size();
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ReadAuxBucketForDispatching(std::size_t bucket_id)
{
    std::fstream aux_file;
    this->OpenFile(file_names_[bucket_id].second, aux_file);
    std::streampos aux_written = current_pointers_[bucket_id].second;

    while (aux_file.tellg() < (aux_written - std::streampos(0)))
    {
        unsorted_aux_buffer_.push_back(AuxType());
        AuxType & aux = unsorted_aux_buffer_.back();

        std::size_t aux_size;
        aux_file.read(reinterpret_cast<char*>(&aux_size), sizeof(std::size_t));
        char aux_serial[aux_size];
        aux_file.read(aux_serial, aux_size);
        izenelib::util::izene_deserialization<AuxType> izsAux(aux_serial, aux_size);
        izsAux.read_image(aux);
    }

    this->CloseFile(aux_file);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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

        ValueType const& value = element.template get<1>();
        AuxType const& aux = unsorted_aux_buffer_[i];

        switch (op)
        {
            case CHECK:
                if (UNIQUE_KEY == result)
                    dispatcher_.UniqueKeyCheck(key, aux);
                else if (DUPLICATE_KEY == result)
                    dispatcher_.DuplicateKeyCheck(key, value, aux);
                break;

            case UPDATE:
                dispatcher_.Update(key, value, aux);
                break;

            case CHECK_UPDATE:
                if (UNIQUE_KEY == result)
                    dispatcher_.UniqueKeyUpdate(key, value, aux);
                else if (DUPLICATE_KEY == result)
                    dispatcher_.DuplicateKeyUpdate(key, value, aux);
                break;

            case DELETE:
                dispatcher_.Delete(key, aux);
                break;

            case CHECK_DELETE:
                if (UNIQUE_KEY == result)
                    dispatcher_.UniqueKeyDelete(key, aux);
                else if (DUPLICATE_KEY == result)
                    dispatcher_.DuplicateKeyDelete(key, value, aux);
                break;

            case APPEND:
                if (UNIQUE_KEY == result)
                    dispatcher_.UniqueKeyAppend(key, value, aux);
                else if (DUPLICATE_KEY == result)
                    dispatcher_.DuplicateKeyAppend(key, value, aux);
                break;

            case EXPEL:
                if (UNIQUE_KEY == result)
                    dispatcher_.UniqueKeyExpel(key, value, aux);
                else if (DUPLICATE_KEY == result)
                    dispatcher_.DuplicateKeyExpel(key, value, aux);
                break;

            default:
                assert("Invalid combination of operation and result.");
                break;
        }
    }
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Check(key_t const& key)
{
    this->Check(key, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Update(key_t const& key, value_t const& value)
{
    Update(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CheckUpdate(key_t const& key, value_t const& value)
{
    CheckUpdate(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
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
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Delete(key_t const& key)
{
    Delete(key, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Delete(key_t const& key, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, DELETE);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CheckDelete(key_t const& key)
{
    CheckDelete(key, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CheckDelete(key_t const& key, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, CHECK_DELETE);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Append(key_t const& key, value_t const& value)
{
    Append(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Append(key_t const& key, value_t const& value, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, value, APPEND);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Expel(key_t const& key, value_t const& value)
{
    Expel(key, value, aux_t());
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Expel(key_t const& key, value_t const& value, aux_t const& aux)
{
    std::pair<std::size_t, std::size_t> bucket_pos = this->Add(key, value, EXPEL);
    aux_buffers_[bucket_pos.first][bucket_pos.second] = aux;
    this->CheckTimeToFeed();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
Synchronize()
{
    this->FeedBuckets();
    this->MergeBuckets();
    this->ReleaseBufferSpace();
    this->TruncateBuckets();
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
bool
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
GetValue(key_t const& key, value_t & value)
{
    bool ret;
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        ret = db_.get(key, value);
    }

    return ret;
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
CaptureBufferSpace()
{
    kv_buffers_.resize(num_buckets_);
    aux_buffers_.resize(num_buckets_);

    for (std::size_t i = 0; i < num_buckets_; i++)
    {
        kv_buffers_[i].resize(bucket_buff_elem_size_);
        aux_buffers_[i].resize(bucket_buff_elem_size_);
    }

    sorted_merge_buffer_.reserve(bucket_buff_elem_size_);
    unsorting_helper_.reserve(bucket_buff_elem_size_);
    unsorted_aux_buffer_.reserve(bucket_buff_elem_size_);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
ReleaseBufferSpace()
{
    CompoundBucketBufferContainer().swap(kv_buffers_);
    AuxBucketBufferContainer().swap(aux_buffers_);

    std::vector<CompoundType>().swap(sorted_merge_buffer_);
    std::vector<std::size_t>().swap(unsorting_helper_);
    std::vector<AuxType>().swap(unsorted_aux_buffer_);
}

template <
    class key_t,
    class value_t,
    class aux_t,
    template <class> class key_comp_t,
    template <class, class, class> class ordered_db_t,
    template <class, class, class> class dispatcher_t,
    class merger_t>
void
Drum<
    key_t,
    value_t,
    aux_t,
    key_comp_t,
    ordered_db_t,
    dispatcher_t,
    merger_t>::
TruncateBuckets()
{
    for (std::size_t bucket_id = 0; bucket_id < num_buckets_; ++bucket_id)
    {
        std::ofstream out_f;
        const std::pair<std::string, std::string>& file_name = file_names_[bucket_id];

        out_f.open(file_name.first.c_str(), std::ios_base::trunc);
        if (!out_f.good())
        {
            std::string ex("Error truncating disk bucket: ");
            ex.append(file_name.first);
            throw DrumException(ex.c_str());
        }
        out_f.close();

        out_f.open(file_name.second.c_str(), std::ios_base::trunc);
        if (!out_f.good())
        {
            std::string ex("Error truncating disk bucket: ");
            ex.append(file_name.second);
            throw DrumException(ex.c_str());
        }
        out_f.close();
    }
}

DRUM_END_NAMESPACE

#endif //IZENELIB_DISK_REPOSITORY_UPDATE_MANAGEMENT_HPP
