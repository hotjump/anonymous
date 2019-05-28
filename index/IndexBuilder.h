#ifndef INDEX_BUILDER_H
#define INDEX_BUILDER_H

#include "IndexBase.h"

class IndexBuilder : public IndexBase {
public:
    bool Init(const string &data_file_name, uint64_t hash_size);

    /*
    * 读取数据文件生成索引文件
    * TODO优化点：这里可以充份使用多核CPU进行压缩
    */
    void Proc();
};

#endif