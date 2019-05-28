#ifndef INDEX_READER_H
#define INDEX_READER_H

#include "IndexBase.h"

class IndexReader : public IndexBase {
public:
    bool Init(const string &data_file_name);

    /*
    * 这里只实现了一个简单的Get, 没有实现迭代器，无法一次返回多条key相同的value
    */
    bool Get(const char* key, uint64_t key_size, char* val, uint64_t& val_size);

    /*
    * 从索引中逐条读出
    */
    void Traverse(bool silence);

    /*
    * 从原数据文件中逐条读出，从索引出发检查每条数据的正确性
    */
    void Verify(bool silence);
};

#endif