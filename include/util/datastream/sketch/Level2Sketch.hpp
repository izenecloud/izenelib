#ifndef IZENELIB_UTIL_bucket_LEVEL2SKETCH_H_
#define IZENELIB_UTIL_bucket_LEVEL2SKETCH_H_

#include <util/hashFunction.h>
#include <vector>
#include <iostream>

NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class Level2Bucket
{
public:
    typedef ElemType DataTypeT;
private:
    typedef std::vector<uint32_t>  Level2BucketT;
    typedef Level2Bucket<DataTypeT> ThisType;

public:
    Level2Bucket()
        : total_cnt_(0)
    {
        bucket_.resize(sizeof(DataTypeT)*8);
    }

    const uint32_t& getBitCnt(size_t index) const
    {
        return bucket_[index];
    }

    void load(std::istream& is)
    {
        is.read((char*)&total_cnt_, sizeof(total_cnt_));
        size_t bit_cnt_num = 0;
        is.read((char*)&bit_cnt_num, sizeof(bit_cnt_num));
        if(bit_cnt_num != sizeof(DataTypeT)*8)
        {
            cout << "load Level2Bucket failed. The DataTypeT is not compatible with current." << endl;
            return;
        }
        bucket_.resize(bit_cnt_num);
        for(size_t i = 0; i < bit_cnt_num; ++i)
        {
            uint32_t data;
            is.read((char*)&data, sizeof(data));
            bucket_[i] = data;
        }
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&total_cnt_, sizeof(total_cnt_));
        size_t bit_cnt_num = bucket_.size();
        os.write((const char*)&bit_cnt_num, sizeof(bit_cnt_num));
        for(size_t i = 0; i < bit_cnt_num; ++i)
        {
            const uint32_t& data = bucket_[i];
            os.write((const char*)&data, sizeof(data));
        }
    }

    void updateBucket(const DataTypeT& data)
    {
        total_cnt_++;
        for(size_t i = 0; i < bucket_.size(); ++i)
        {
            bucket_[i] += (data & ((DataTypeT)1 << i)) == 0?0:1;
        }
    }

    void unionBucket(const ThisType& other)
    {
        total_cnt_ += other.total_cnt_;
        for(size_t i = 0; i < bucket_.size(); ++i)
        {
            bucket_[i] += other.bucket_[i];
        }
    }

    bool emptyBucket() const
    {
        return total_cnt_ == 0;
    }

    bool singletonBucket() const
    {
        if(total_cnt_ == 0)
            return false;
        for(size_t j = 0; j < bucket_.size(); ++j)
        {
            if(bucket_[j] > 0 && total_cnt_ > bucket_[j])
                return false;
        }
        return true;
    }

    bool identicalSingletonBucket(const ThisType& other) const
    {
        if(!singletonBucket() || !other.singletonBucket())
            return false;
        for(size_t j = 0; j < bucket_.size(); ++j)
        {
            if( (bucket_[j] > 0 && other.bucket_[j] == 0) ||
                (bucket_[j] == 0 && other.bucket_[j] > 0))
                return false;
        }
        return true;
    }

    bool singletonUnionBucket(const ThisType& other) const
    {
        if((singletonBucket() && other.emptyBucket()) ||
            (other.singletonBucket() && emptyBucket()) )
            return true;
        return identicalSingletonBucket(other);
    }

private:
    Level2BucketT bucket_;
    uint32_t total_cnt_;
};

template <typename ElemType>
class Level2Sketch
{
public:
    typedef ElemType DataTypeT;

private:
    typedef Level2Bucket<DataTypeT> Level2BucketType;
    typedef Level2Sketch<DataTypeT> ThisType;

public:
    Level2Sketch()
    {
    }

    Level2Sketch(size_t bucket_num)
    {
        level2sketch_.resize(bucket_num);
    }

    void resize(size_t bucket_num)
    {
        level2sketch_.resize(bucket_num);
    }

    size_t size() const
    {
        return level2sketch_.size();
    }

    const Level2BucketType& getBucket(size_t index) const
    {
        return level2sketch_[index];
    }

    void load(std::istream& is)
    {
        size_t bucket_num = 0;
        is.read((char*)&bucket_num, sizeof(bucket_num));
        if(bucket_num != level2sketch_.size())
        {
            cout << "load Level2Sketch failed. The size is not compatible with current." << endl;
            return;
        }
        for(size_t i = 0; i < bucket_num; ++i)
        {
            level2sketch_[i].load(is);
        }
    }

    void save(std::ostream& os) const
    {
        const size_t bucket_num = level2sketch_.size();
        os.write((const char*)&bucket_num, sizeof(bucket_num));
        for(size_t i = 0; i < bucket_num; ++i)
        {
            level2sketch_[i].save(os);
        }
    }

    void updateBucket(size_t index, const DataTypeT& data)
    {
        assert(index < level2sketch_.size());
        level2sketch_[index].updateBucket(data);
    }

    void unionBucket(size_t index, const ThisType& other)
    {
        level2sketch_[index].unionBucket(other.level2sketch_[index]);
    }

    void unionLevel2Sketch(const ThisType& other)
    {
        for(size_t i = 0; i < level2sketch_.size(); ++i)
            unionBucket(i, other);
    }

    bool emptyBucket(size_t index) const
    {
        assert(index < level2sketch_.size());
        return level2sketch_[index].emptyBucket();
    }

    bool singletonBucket(size_t index) const
    {
        assert(index < level2sketch_.size());
        return level2sketch_[index].singletonBucket();
    }

    bool identicalSingletonBucket(size_t index, const ThisType& other) const
    {
        assert(index < level2sketch_.size());
        assert(index < other.size());
        return level2sketch_[index].identicalSingletonBucket(other.level2sketch_[index]);
    }

    bool singletonUnionBucket(size_t index, const ThisType& other) const
    {
        assert(index < level2sketch_.size());
        assert(index < other.size());
        return level2sketch_[index].singletonUnionBucket(other.level2sketch_[index]);
    }

    int atomicIntersectEstimator(size_t index, const ThisType& other) const
    {
        if(!singletonUnionBucket(index, other))
            return -1;
        int estimate = 0;
        if( singletonBucket(index) && other.singletonBucket(index) )
            estimate = 1;
        return estimate;
    }

    int atomicDiffEstimator(size_t index, const ThisType& other) const
    {
        if(!singletonUnionBucket(index, other))
            return -1;
        int estimate = 0;
        if( singletonBucket(index) && other.emptyBucket(index) )
            estimate = 1;
        return estimate;
    }

private:
    std::vector<Level2BucketType>  level2sketch_;
};

template <typename ElemType>
bool identicalSingletonBucket(const std::vector<Level2Sketch<ElemType> >& sketches, size_t index)
{
    if(sketches.empty())
        return false;
    for(size_t i = 0; i < sketches.size(); ++i)
    {
        if(!sketches[i].singletonBucket(index))
            return false;
    }
    for(size_t bit_cnt_num = 0; bit_cnt_num < sizeof(ElemType)*8; ++bit_cnt_num)
    {
        uint32_t cur_cnt = sketches[0].getBucket(index).getBitCnt(bit_cnt_num);
        for(size_t sketch_i = 1; sketch_i < sketches.size(); ++sketch_i)
        {
            uint32_t other_cnt = sketches[sketch_i].getBucket(index).getBitCnt(bit_cnt_num);
            if( (cur_cnt > 0 && other_cnt == 0) ||
                (cur_cnt == 0 && other_cnt > 0) )
                return false;
        }
    }
    return true;
}

template <typename ElemType>
bool singletonUnionBucket(const std::vector<Level2Sketch<ElemType> >& union_sketches, size_t index)
{
    std::vector<Level2Sketch<ElemType> > non_empty_sketches;
    for(size_t i = 0; i < union_sketches.size(); ++i)
    {
        if(!union_sketches[i].emptyBucket(index))
            non_empty_sketches.push_back(union_sketches[i]);
    }
    if(non_empty_sketches.size() == 1)
    {
        return non_empty_sketches[0].singletonBucket(index);
    }
    return identicalSingletonBucket(non_empty_sketches, index);
}

// do (and (or (union_sketches)) (or (union_sketches_2)))
template <typename ElemType>
int atomicUnionWithIntersectBucketEstimator(size_t index, const std::vector<Level2Sketch<ElemType> >& union_sketches,
    const std::vector<Level2Sketch<ElemType> >& union_sketches_2)
{
    std::vector<Level2Sketch<ElemType> > all_sketches = union_sketches;
    all_sketches.insert(all_sketches.end(), union_sketches_2.begin(), union_sketches_2.end());
    if(!singletonUnionBucket(all_sketches, index))
        return -1;
    int estimate = 0;
    for(size_t i = 0; i < union_sketches.size(); ++i)
    {
        if(!union_sketches[i].emptyBucket(index))
        {
            estimate = 1;
            break;
        }
    }
    if(estimate == 0)
        return 0;

    for(size_t i = 0; i < union_sketches_2.size(); ++i)
    {
        if(!union_sketches_2[i].emptyBucket(index))
        {
            return 1;
        }
    }
    return 0;
}

template <typename ElemType>
int atomicIntersectBucketEstimator(size_t index, const std::vector<Level2Sketch<ElemType> >& level2sketches)
{
    if(!singletonUnionBucket(level2sketches, index))
        return -1;
    for(size_t i = 0; i < level2sketches.size(); ++i)
    {
        if(level2sketches[i].emptyBucket(index))
        {
            return 0;
        }
    }
    return 1;
}

template <typename ElemType>
int atomicUnionBucketEstimator(size_t index, const std::vector<Level2Sketch<ElemType> >& level2sketches)
{
    for(size_t i = 0; i < level2sketches.size(); ++i)
    {
        if(!level2sketches[i].emptyBucket(index))
        {
            return 1;
        }
    }
    return 0;
}

NS_IZENELIB_UTIL_END

#endif
