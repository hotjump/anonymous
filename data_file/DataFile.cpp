#include "DataFile.h"

bool GenDataFile(const string& data_file_name, uint64_t approximate_MB)
{
    cout << "----------GenDataFile-----------" << endl;
    ofstream outfile(data_file_name.c_str(), ofstream::out | ofstream::binary | ofstream::trunc);
    if (!outfile) {
        cerr << "open file failed!" << endl;
        return false;
    }
    uint64_t record_num = 0;
    uint64_t file_cur_bytes = 0;
    uint64_t approximate_bytes = approximate_MB * 1024 * 1024;

    default_random_engine generator;
    KeySizeDistribution key_size_distribution(1, MAX_KEY_SIZE);
    ValSizeDistribution val_size_distribution(1, MAX_VAL_SIZE);
    KeyDistribution key_distribution(0, 255);
    ValDistribution val_distribution(0, 15);

    st_record* p_st_record = (st_record*)malloc(sizeof(st_record));

    while (file_cur_bytes < approximate_bytes) {
        p_st_record->key_size = key_size_distribution(generator);
        p_st_record->val_size = val_size_distribution(generator);

        for (size_t i = 0; i < p_st_record->key_size; i++) {
            p_st_record->key[i] = key_distribution(generator);
        }

        for (size_t i = 0; i < p_st_record->val_size; i++) {
            p_st_record->val[i] = val_distribution(generator);
        }

        outfile.write(reinterpret_cast<const char*>(&p_st_record->key_size), sizeof(p_st_record->key_size));
        outfile.write(&p_st_record->key[0], p_st_record->key_size);
        outfile.write(reinterpret_cast<const char*>(&p_st_record->val_size), sizeof(p_st_record->val_size));
        outfile.write(&p_st_record->val[0], p_st_record->val_size);

        file_cur_bytes += sizeof(p_st_record->key_size) + sizeof(p_st_record->val_size) + p_st_record->key_size + p_st_record->val_size;
        ++record_num;
    }

    free(p_st_record);
    outfile.close();

    cout << "Record num:" << record_num << endl;

    return true;
}