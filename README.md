# anonymous

## 1. Intro

anonymous是一个自以为高性能的索引库。

适合场景：

1. 为使用HDD盘又想使用很低的内存/磁盘比达到高性能。
2. 随机读。
3. 相对较大的Value。

设计原则：

1. 内存和磁盘使用均高效的Hash索引。
2. 使用压缩算法尽可能减少IOPS和Throughput。

## 2. 目录结构

+ common:  哈希函数和LZ4/ZSTD等两个压缩库代码。
+ data_file:  一个可以生成key/value任意大小和key/value任意分布的数据文件的函数。最早写的是一个模板函数，由于C++的不同分布算法的参数不同，后来改成全使用uniform_int_distribution，并且为了Value有较好的压缩率，有意限制了Value的分布范围。
+ doc:  暂时没有文档
+ test_dir:  test用例会将数据文件和索引文件放置在这个目录下，并且这个目录下有一个输出的example可以参考。
+ index:  索引库的代码所在。

## 3. 类设计

IndexReader和IndexBuilder继承了IndexBase中相同的成员变量。

1. IndexBuilder可以将数据文件转化成索引。
2. IndexReader可以加载索引文件，加快数据的读取。

```
                    +-------------+
                    |             |
        +-----------+  IndexBase  +-----------+
        |           |             |           |
        |           +-------------+           |
        |                                     |
        |                                     |
        |                                     |
        |                                     |
        |                                     |
+-------v--------+                  +---------v-----+
|                |                  |               |
|  IndexBuilder  |                  |  IndexReader  |
|                |                  |               |
+----------------+                  +---------------+

```

## 4.索引结构

索引结构包括：

1. key_zone: 包括所有key的信息，每条index_record是变长的。
2. hash_zone: hash table数组，是定长的。
3. footer：key_zone和hash_zone的基本信息，是定长的。

索引结构在文件和内存里结构完全一致。key_zone和hash_zone都使用连续的内存空间，减少了没必要的内存碎片。

并且通过index_reocrd结构在key_zone里索引记录。

```
struct index_record {
    uint64_t next_offset;
    uint8_t val_location; // 0. raw data file; 1. compressed data file
    uint64_t val_offset;
    uint32_t val_size;
    uint32_t key_size;
    char key[0];
}
```

```
                   +------------+
                   | index file |
                   +-----+------+
                         |
                         v
               X+--------+-----------+                           +-----------------------+
             XX |  index_record [0]  +-------------------------> |                       |
           XXX  +--------------------+                           | compressed data file  |
+--------XXX    |  index_record [1]  <--------+                  |                       |
|key_zone|X     +--------------------+        |                  +-----------------------+
+--------+ XX   |     .......        +<------------+
            XX  +--------------------+        |    |             +-----------------------+
             XXX|     .......        +-------------------------> |                       |
             XXX+--------------------+        |    |             | raw data file         |
+---------+ XX X|     hash[0]        +-------------+             |                       |
|hash_zone|XX   +--------------------+        |                  |                       |
+---------+ XX  |     .......        +--------+                  +-----------------------+
             XXX+--------------------+
              XX|     hash[n]        |
                +--------------------+
                |     footer         |
                +--------------------+

```

### 4.1 生成索引流程

1. 遍历读出每一条完整的记录，构造索引记录。
2. 使用类似头插法，将该索引记录的地址放入哈希数组中。
3. 构造索引记录的结构并写入索引文件里，适合压缩(当前默认大于64K的Val且压缩率在87.5%以下的)的val写到compressed data file里。
4. 待所有记录都处理结束，将hash_zone和footer写入索引文件里。

TODO: 这里可以进行多核优化。

### 4.2 随机读流程

1. 计算对应的key哈希函数，找到hash_zone对应的数组中的offset；
2. 如果offset非NULL_OFFSET，通过下标在key_zone找到第一个index_record；如果命中，根据val_location读到压缩文件或者原文件的Value；
3. 如果没命中，沿着逻辑链表往下查找，直到链表末尾。

## 5. 如何使用？

在根目录下运行:

1. git submodule init
2. git submodule update (从Github下载ZSTD和LZ4的代码)
3. cmake .
4. make

在index目录下会生成 `libindex.a` 的静态文件，在根目录下会生成 `test` 的可执行文件。