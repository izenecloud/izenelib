#ifndef _NODE_RELATION_TABLE_H_
#define _NODE_RELATION_TABLE_H_

NS_IZENELIB_AM_BEGIN

template <typename NodeIDType,
          typename LockType = izenelib::util::NullLock>
class NodeRelationTable
{
    typedef NodeIDType KeyType;
    typedef std::vector<NodeIDType> ValueType;
    typedef izenelib::sdb::ordered_sdb<KeyType, ValueType , LockType> DBType;

public:

    NodeRelationTable(const std::string& dbname)
    :   dbname_(dbname),
        db(dbname_ + ".sdb")
    {
        db_.open();
    }

    virtual ~NodeRelationTable(){}

    void push_back(const NodeIDType& srcNID, const NodeIDType& dstNID)
    {
        std::vector<NodeIDType> value;
        if( db_.getValue() )
    }

    void get(const NodeIDType& srcNID, std::vector<NodeIDType>& dstNIDList) const
    {
    }

private:

    std::string dbname_;

    DBType db_;
}

NS_IZENELIB_AM_END

#endif
