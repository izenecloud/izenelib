class Range
{
    bool empty();

    bool popFront();
    const Key& frontKey();
    const Value& frontValue();
    const DataType<Key, Value>& front();
    bool updateFront(const Value&);
    bool delFront();

    bool popBack();
    const Key& backKey();
    const Value& backValue();
    const DataType<Key, Value>& back();
    bool updateBack(const Value&);
    bool delBack();
};

DefaultConstructable;

AM
{
    bool all(Range& r);
    bool all(ConstRange& r) const;
    bool find(const Key& k, Range& r) const;
}

