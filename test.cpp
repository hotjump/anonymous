
#include "data_file/DataFile.h"
#include <random>
#include <stdint.h>

#include "index/IndexBuilder.h"
#include "index/IndexReader.h"

using namespace std;

int main()
{
    const string data_file_name = "test_dir/anonymous";
    uint64_t data_file_mb = 10;

    /*
    * 这里不同的分布算法的参数不一样，没调好，还是先都只用uniform_int_distribution
    * 为了使得value被压缩的可能性大一些，Value的范围随机值比较小
    * 另外，此处生成文件是单线程的，会比较慢
    */
    GenDataFile(data_file_name, data_file_mb);

    IndexBuilder* index_builder = new IndexBuilder();
    index_builder->Init(data_file_name, 3);
    index_builder->Proc();
    delete index_builder;

    IndexReader *index_reader = new IndexReader();
    index_reader->Init(data_file_name);
    index_reader->Traverse(false);
    index_reader->Verify(false);

    delete index_reader;

    return 0;
}