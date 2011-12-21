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
#include "exception.hpp"
#include "element_io.hpp"
#include "null_dispatcher.hpp"

//Boost.
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/cstdint.hpp>

//STL.
#include <utility>
#include <cstddef>
#include <cassert>
#include <string>
#include <fstream>
#include <vector>

#include <stdint.h>


DRUM_BEGIN_NAMESPACE

//For specialization.
template <class key_t, std::size_t num_buckets_nt> struct BucketIdentififer;

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

template<class key_t,
         class value_t,
         class aux_t,
         std::size_t num_buckets_nt,
         std::size_t bucket_buff_elem_size_nt,
         std::size_t bucket_byte_size_nt,
         template <class, class> class ordered_db_t,
         template <class, class, class> class dispatcher_t = NullDispatcher>
class Drum
{
public:
    typedef key_t KeyType;
    typedef value_t ValueType;
    typedef aux_t AuxType;

private:
    typedef boost::tuple<key_t,
                         value_t,
                         char,         //Operation (check, update, check/update).
                         std::size_t,  //Position in the bucket.
                         char          //Operation result (unique, duplicate).
                        > CompoundType;
    typedef boost::array<CompoundType, bucket_buff_elem_size_nt> CompoundBucketBuffer;
    typedef boost::array<CompoundBucketBuffer, num_buckets_nt> CompoundBucketBufferContainer;
    typedef boost::array<AuxType, bucket_buff_elem_size_nt> AuxBucketBuffer;
    typedef boost::array<AuxBucketBuffer, num_buckets_nt> AuxBucketBufferContainer;
    typedef boost::array<std::size_t, num_buckets_nt> NextBufferPositionsContainer;

    typedef boost::array<
        std::pair<std::string, std::string>, num_buckets_nt> BucketFileNamesContainer;
    typedef boost::array<
        std::pair<std::streampos, std::streampos>, num_buckets_nt> BucketFilePointersContainer;

    typedef ordered_db_t<key_t, value_t> DbType;

    typedef dispatcher_t<key_t, value_t, aux_t> DispatcherType;

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

    Drum(Drum const&);
    Drum const& operator=(Drum const&);

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
    void FeedBucket(std::size_t bucket_id);
    void MergeBuckets();
    void ReadInfoBucketIntoMergeBuffer(std::size_t bucket_id);
    void SortMergeBuffer();
    void UnsortMergeBuffer();
    void SynchronizeWithDisk();
    void ReadAuxBucketForDispatching(std::size_t bucket_id);
    void Dispatch();

public:
    Drum(std::string const& name);
    Drum(std::string const& name, DispatcherType const& dispatcher);
    Drum(std::string const& name, DispatcherType const& dispatcher, boost::uint32_t dds);
    ~Drum(); //Not intended to be inherited.

    //Check/update operations.
    void Check(key_t const& key);
    void Check(key_t const& key, aux_t const& aux);
    void Update(key_t const& key, value_t const& value);
    void Update(key_t const& key, value_t const& value, aux_t const& aux);
    void CheckUpdate(key_t const& key, value_t const& value);
    void CheckUpdate(key_t const& key, value_t const& value, aux_t const& aux);

    void Synchronize(); //Force synchronization (merge).
    void Dispose(); //Done with Drum.

private:
    bool merge_buckets_;
    bool feed_buckets_;
    std::string drum_name_;
    DbType db_; //Disk repository.
    boost::uint32_t const db_default_data_size_;
    bool db_closed_;
    DispatcherType dispatcher_;

    //According to Effective C++ it's not necessary to provide definitions for such integral constant
    //variables as long as their addresses are not taken.
    static const char CHECK = 0;
    static const char CHECK_UPDATE = 1;
    static const char UPDATE = 2;
    static const char UNIQUE_KEY = 3;
    static const char DUPLICATE_KEY = 4;

    std::vector<CompoundType> sorted_merge_buffer_; //Used during merge with disk.
    std::vector<std::size_t> unsorting_helper_; //Original positions of elements in buckets.
    std::vector<AuxType> unsorted_aux_buffer_; //Used in dispatching after merge.

    CompoundBucketBufferContainer kv_buffers_;
    AuxBucketBufferContainer aux_buffers_;
    NextBufferPositionsContainer next_positions_; //For key/value and aux buffers.

    BucketFileNamesContainer file_names_;
    BucketFilePointersContainer current_pointers_;
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DISK_REPOSITORY_UPDATE_MANAGEMENT_HPP
