#include "IndexReader.h"
#include "DataFile.h"
#include "Hash.h"
#include "lz4.h"
#include "zstd.h"
#include <fstream>
#include <iostream>
#include <string.h>

bool IndexReader::Init(const string &data_file_name)
{
    _data_file_name = data_file_name;
    _raw_data_file.open(_data_file_name, ios::in | ios::binary);
    if (!_raw_data_file) {
        cerr << "open " << _data_file_name << " failed!" << endl;
        return false;
    }

    _index_file.open(_data_file_name + INDEX_SUFFIX, ios::in | ios::binary);
    if (!_index_file) {
        cerr << "open " << _data_file_name << INDEX_SUFFIX << " failed!" << endl;
        return false;
    }

    _compressed_data_file.open(_data_file_name + COMPRESSED_SUFFIX, ios::in | ios::binary);
    if (!_compressed_data_file) {
        cerr << "open " << _data_file_name << COMPRESSED_SUFFIX << " failed!" << endl;
        return false;
    }

    // 0. 初始化footer部份
    _index_file.seekg(-sizeof(st_footer), ios::end);
    _index_file.read(reinterpret_cast<char*>(&_footer), sizeof(st_footer));

    // 1. 初始化hash_zone
    _hash_zone = new uint64_t[_footer.hash_num];
    _index_file.seekg(_footer.hash_zone_offset, ios::beg);
    _index_file.read(reinterpret_cast<char*>(_hash_zone), _footer.hash_zone_size);

    // 2. 初始化key_zone
    _key_zone = new char[_footer.key_zone_size];
    _index_file.seekg(_footer.key_zone_offset, ios::beg);
    _index_file.read(reinterpret_cast<char*>(_key_zone), _footer.key_zone_size);

    return false;
}

bool IndexReader::Get(const char* key, uint64_t key_size, char* val, uint64_t& val_size)
{
    uint64_t hash_slot = HashFunc((uint8_t*)key, key_size) % _footer.hash_num;
    uint64_t offset = _hash_zone[hash_slot];

    if (offset == NULL_OFFSET)
        return false;

    st_index_record* p_index_record = (st_index_record*)(_key_zone + offset);
    while (p_index_record) {

        if (key_size == p_index_record->key_size && !memcmp((void*)key, (void*)p_index_record->key, key_size)) {
            if (p_index_record->val_location == LOCATION_RAW_DATA_FILE) {
                val_size = p_index_record->val_size;
                _raw_data_file.seekg(p_index_record->val_offset, ios::beg);
                _raw_data_file.read(val, val_size);
            } else { //LOCATION_COM_DATA_FILE
                char* decompress_buffer = new char[MAX_VAL_SIZE];
                _compressed_data_file.seekg(p_index_record->val_offset, ios::beg);
                _compressed_data_file.read(decompress_buffer, p_index_record->val_size);
                // val_size = LZ4_decompress_safe(decompress_buffer, val, p_index_record->val_size, MAX_VAL_SIZE);
                val_size = ZSTD_decompress(val, MAX_VAL_SIZE, decompress_buffer, p_index_record->val_size);
                delete[] decompress_buffer;
            }

            return true;
        } else if (p_index_record->next_offset != NULL_OFFSET) {
            p_index_record = (st_index_record*)(_key_zone + p_index_record->next_offset);
        } else {
            p_index_record = NULL;
        }
    }

    return false;
}

void IndexReader::Traverse(bool silence)
{
    cout << "----------Traverse-----------" << endl;
    char* val = new char[MAX_VAL_SIZE];
    uint64_t val_size = 0;

    for (uint64_t i = 0; i < _footer.hash_num; i++) {
        uint64_t offset = _hash_zone[i];
        if (offset == NULL_OFFSET)
            continue;

        st_index_record* p_index_record = (st_index_record*)(_key_zone + offset);

        cout << "bucket[" << i << "]:" << endl;

        while (p_index_record) {

            if (p_index_record->val_location == LOCATION_RAW_DATA_FILE) {
                val_size = p_index_record->val_size;
                _raw_data_file.seekg(p_index_record->val_offset, ios::beg);
                _raw_data_file.read(val, val_size);
            } else { //LOCATION_COM_DATA_FILE
                char* decompress_buffer = new char[MAX_VAL_SIZE];
                _compressed_data_file.seekg(p_index_record->val_offset, ios::beg);
                _compressed_data_file.read(decompress_buffer, p_index_record->val_size);
                val_size = ZSTD_decompress(val, MAX_VAL_SIZE, decompress_buffer, p_index_record->val_size);
                delete[] decompress_buffer;
            }

            if (!silence) {
                cout << "key_size: " << p_index_record-> key_size;
                cout << " val_size: " << val_size << endl;
            }

            if (p_index_record->next_offset != NULL_OFFSET) {
                p_index_record = (st_index_record*)(_key_zone + p_index_record->next_offset);
            } else {
                p_index_record = NULL;
            }
        }
    }

    free(val);
}

void IndexReader::Verify(bool silence)
{
    cout << "----------Verify-----------" << endl;
    _raw_data_file.seekg(0, ios_base::end);
    uint64_t file_size = _raw_data_file.tellg();
    _raw_data_file.seekg(0, ios_base::beg);
    uint64_t cur_file_size = 0;

    st_record* p_st_record = (st_record*)malloc(sizeof(st_record));
    char* val_buffer = new char[MAX_VAL_SIZE];
    uint64_t val_size = 0;

    while (cur_file_size < file_size) {
        // 0. 读出一条完整的记录
        _raw_data_file.read(reinterpret_cast<char*>(&p_st_record->key_size), sizeof(p_st_record->key_size));
        _raw_data_file.read(&p_st_record->key[0], p_st_record->key_size);
        _raw_data_file.read(reinterpret_cast<char*>(&p_st_record->val_size), sizeof(p_st_record->val_size));
        _raw_data_file.read(&p_st_record->val[0], p_st_record->val_size);

        // 1. 通过索引读出对应的Value
        Get(&p_st_record->key[0], p_st_record->key_size, val_buffer, val_size);

        // 2. 校验读出的Value是否完全正确
        if (p_st_record->val_size == val_size && !memcmp((void*)p_st_record->val, (void*)val_buffer, val_size)) {
            if (!silence) {
                cout<< "key_size: " << p_st_record->key_size
                    << " val_size: " << p_st_record->val_size <<endl;
            }
        } else {
            cout << " key_size: " << p_st_record->key_size
                 << " val_size: " << p_st_record->val_size << "get_val_size:" << val_size << endl;
        }

        cur_file_size += sizeof(p_st_record->key_size) + sizeof(p_st_record->val_size) + p_st_record->key_size + p_st_record->val_size;
    }

    free(p_st_record);
    free(val_buffer);
}