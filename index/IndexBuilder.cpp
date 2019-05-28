#include "IndexBuilder.h"
#include "DataFile.h"
#include "Hash.h"
#include "lz4.h"
#include "zstd.h"
#include <fstream>
#include <iostream>

bool IndexBuilder::Init(const string &data_file_name, uint64_t hash_size)
{
    _data_file_name = data_file_name;

    _raw_data_file.open(_data_file_name, ios::in | ios::binary);
    if (!_raw_data_file) {
        cerr << "open " << _data_file_name << " failed!" << endl;
        return false;
    }

    _index_file.open(_data_file_name + INDEX_SUFFIX, ios::out | ios::binary);
    if (!_index_file) {
        cerr << "open " << _data_file_name << INDEX_SUFFIX << " failed!" << endl;
        return false;
    }

    _compressed_data_file.open(_data_file_name + COMPRESSED_SUFFIX, ios::out | ios::binary);
    if (!_compressed_data_file) {
        cerr << "open " << _data_file_name << COMPRESSED_SUFFIX << " failed!" << endl;
        return false;
    }

    _footer.hash_num = NearPrime(hash_size);
    _hash_zone = new uint64_t[_footer.hash_num];

    for (uint64_t i = 0; i < _footer.hash_num; i++) {
        _hash_zone[i] = NULL_OFFSET;
    }

    return true;
}

bool GoodCompressionRatio(size_t compressed_size, size_t raw_size)
{
    // Check to see if compressed less than 12.5%
    return compressed_size < raw_size - (raw_size / 8u);
}

void IndexBuilder::Proc()
{
    cout << "----------IndexBuilder::Proc-----------" << endl;
    st_record* p_st_record = (st_record*)malloc(sizeof(st_record));
    st_index_record* p_st_index_record = (st_index_record*)malloc(sizeof(st_index_record));
    char* compressed_buffer = new char[MAX_VAL_SIZE*10];

    _raw_data_file.seekg(0, ios_base::end);
    uint64_t file_size = _raw_data_file.tellg();
    _raw_data_file.seekg(0, ios_base::beg);
    uint64_t cur_file_size = 0;

    uint64_t compressed_num = 0;
    uint64_t no_compressed_num = 0;

    while (cur_file_size < file_size) {
        // 0. 读出一条完整的记录
        _raw_data_file.read(reinterpret_cast<char*>(&p_st_record->key_size), sizeof(p_st_record->key_size));
        _raw_data_file.read(&p_st_record->key[0], p_st_record->key_size);
        _raw_data_file.read(reinterpret_cast<char*>(&p_st_record->val_size), sizeof(p_st_record->val_size));
        _raw_data_file.read(&p_st_record->val[0], p_st_record->val_size);

        // 1. 使用类似头插法，将该索引记录的地址放入哈希数组中
        uint64_t hash_slot = HashFunc((uint8_t*)p_st_record->key, p_st_record->key_size) % _footer.hash_num;
        p_st_index_record->next_offset = _hash_zone[hash_slot];
        _hash_zone[hash_slot] = _index_file.tellp();

        // 2. 构造索引记录的结构并写入对应的文件里
        // 2.1 适合压缩的val写到compressed data file里
        // 2.2 其他照旧
        // compressed_size = LZ4_compress_default(&p_st_record->val[0], compressed_buffer, p_st_record->val_size, MAX_VAL_SIZE*10)
        // compressed_size = ZSTD_compress(compressed_buffer, MAX_VAL_SIZE*10, (void*)&p_st_record->val[0], p_st_record->val_size, 6)

        int compressed_size = 0;
        if (p_st_record->val_size > COMPRESSION_THRESHOLD_SIZE &&
            (compressed_size = ZSTD_compress(compressed_buffer, MAX_VAL_SIZE*10, (void*)&p_st_record->val[0], p_st_record->val_size, 6)) &&
            GoodCompressionRatio(compressed_size, p_st_record->val_size)) {
            p_st_index_record->val_location = LOCATION_COM_DATA_FILE;
            p_st_index_record->val_offset = _compressed_data_file.tellp();
            p_st_index_record->val_size = compressed_size;
            _compressed_data_file.write(compressed_buffer, compressed_size);
            compressed_num++;
        } else {
            p_st_index_record->val_location = LOCATION_RAW_DATA_FILE;
            p_st_index_record->val_offset = (uint64_t)_raw_data_file.tellg() - p_st_record->val_size;
            p_st_index_record->val_size = p_st_record->val_size;
            no_compressed_num++;
        }

        p_st_index_record->key_size = p_st_record->key_size;
        _index_file.write(reinterpret_cast<const char*>(p_st_index_record), sizeof(st_index_record));
        _index_file.write(&p_st_record->key[0], p_st_record->key_size);

        ++_footer.key_num;
        cur_file_size += sizeof(p_st_record->key_size) + sizeof(p_st_record->val_size) + p_st_record->key_size + p_st_record->val_size;
    }

    // 3. 生成footer
    _footer.key_zone_offset = 0;
    _footer.key_zone_size = _index_file.tellp();
    _footer.hash_zone_offset = _footer.key_zone_offset + _footer.key_zone_size;
    _footer.hash_zone_size = _footer.hash_num * sizeof(uint64_t);

    // 4. 将hash_zone和footer刷入索引文件
    _index_file.write(reinterpret_cast<const char*>(_hash_zone), _footer.hash_zone_size);
    _index_file.write(reinterpret_cast<const char*>(&_footer), sizeof(_footer));

    free(p_st_record);
    free(p_st_index_record);
    free(compressed_buffer);

    cout << "no_compressed_num: " << no_compressed_num << endl;
    cout << "compressed_num: " << compressed_num << endl;
    cout << "key_num: " << _footer.key_num << endl;
    cout << "key_zone_offset" << _footer.key_zone_offset << endl;
    cout << "key_zone_size: " << _footer.key_zone_size << endl;
    cout << "hash_zone_offset: " << _footer.hash_zone_offset << endl;
    cout << "hash_zone_size: " << _footer.hash_zone_size << endl;
    cout << "hash_num: " << _footer.hash_num << endl;
}