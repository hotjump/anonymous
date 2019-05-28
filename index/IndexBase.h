#ifndef INDEX_BASE_H
#define INDEX_BASE_H

#include <stdint.h>
#include <fstream>

using namespace std;

const string COMPRESSED_SUFFIX = ".COMPRESSED";
const string INDEX_SUFFIX = ".INDEX";

const uint8_t LOCATION_RAW_DATA_FILE = 0;
const uint8_t LOCATION_COM_DATA_FILE = 1;

const uint64_t NULL_OFFSET = 0xFFFFFFFFFFFFFFFF;
const uint64_t COMPRESSION_THRESHOLD_SIZE = 64 * 1024;

typedef struct index_record {
    index_record()
        : next_offset(NULL_OFFSET)
        , val_location(0)
        , val_offset(NULL_OFFSET)
        , val_size(0)
        , key_size(0)
    {
    }
    uint64_t next_offset;
    uint8_t val_location; // 0. raw data file; 1. compressed data file
    uint64_t val_offset;
    uint32_t val_size;
    uint32_t key_size;
    char key[0];
} st_index_record;

typedef struct footer {
    footer()
        : key_zone_offset(0)
        , key_zone_size(0)
        , key_num(0)
        , hash_zone_offset(0)
        , hash_zone_size(0)
        , hash_num(0)
    {
    }
    uint64_t key_zone_offset;
    uint64_t key_zone_size;
    uint64_t key_num;
    uint64_t hash_zone_offset;
    uint64_t hash_zone_size;
    uint64_t hash_num;
} st_footer;

class IndexBase {
public:
    IndexBase()
        : _key_zone(NULL)
        , _hash_zone(NULL)
    {
    }
    ~IndexBase();

    string _data_file_name;
    ifstream _raw_data_file;
    fstream _index_file;
    fstream _compressed_data_file;

    char* _key_zone;
    uint64_t* _hash_zone;
    st_footer _footer;
};

#endif