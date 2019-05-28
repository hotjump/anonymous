#include "IndexBase.h"

IndexBase::~IndexBase()
{
    if (_raw_data_file)
        _raw_data_file.close();

    if (_index_file)
        _index_file.close();

    if (_compressed_data_file)
        _compressed_data_file.close();

    if (_key_zone)
        delete[] _key_zone;

    if (_hash_zone)
        delete[] _hash_zone;
}