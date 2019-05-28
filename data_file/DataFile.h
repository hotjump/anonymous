#ifndef DATA_FILE_H
#define DATA_FILE_H

#include <random>
#include <fstream>
#include <iostream>

using namespace std;

const uint64_t MAX_KEY_SIZE = 1024;
const uint64_t MAX_VAL_SIZE = 1024 * 1024;

typedef struct record {
    uint64_t key_size;
    char key[MAX_KEY_SIZE];
    uint64_t val_size;
    char val[MAX_VAL_SIZE];
} st_record;

/*
Distribution Option:
- uniform_int_distribution
- poisson_distribution
- normal_distribution
- and so on
- 不过这里不同的分布算法的参数不一样，还是先都只用uniform_int_distribution
*/

//template <typename KeySizeDistribution, typename ValSizeDistribution, typename KeyDistribution, typename ValDistribution>
//bool GenDataFile(const string& data_file_name, uint64_t approximate_GB = 1);

typedef uniform_int_distribution<int> KeySizeDistribution;
typedef uniform_int_distribution<int> ValSizeDistribution;
typedef uniform_int_distribution<int> KeyDistribution;
typedef uniform_int_distribution<int> ValDistribution;

bool GenDataFile(const string &data_file_name, uint64_t approximate_MB);

#endif