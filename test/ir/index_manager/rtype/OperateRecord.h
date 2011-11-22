#ifndef IZENELIB_IR_OPERATERECORD_
#define IZENELIB_IR_OPERATERECORD_

#include <iostream>
#include <string>
#include <vector>

template<class KeyType>
class OperateRecord
{
    typedef OperateRecord<KeyType> ThisType;
    struct ItemType
    {
        int op;
        KeyType key;
        uint32_t docid;
    };
public:
    void append(int op, const KeyType& key, uint32_t docid)
    {
        ItemType item;
        item.op = op;
        item.key = key;
        item.docid = docid;
        item_list.push_back(item);
    }
    
    void clear()
    {
        item_list.clear();
    }
    
    friend std::ostream& operator<<(std::ostream& output, const ThisType& v) {
        output<<"["<<v.item_list.size()<<"]"<<std::endl;
        for(std::size_t i=0;i<v.item_list.size();i++)
        {
            output<<v.item_list[i].op<<","<<v.item_list[i].key<<","<<v.item_list[i].docid<<std::endl;
        }
        return output;
    }
    
    
public:
    std::vector<ItemType> item_list;
};

#endif
