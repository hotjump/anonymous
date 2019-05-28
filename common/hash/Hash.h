#ifndef HASH_H
#define HASH_H

#include <stdint.h>

uint64_t HashFunc(uint8_t* buf, uint16_t siz);
uint64_t NearPrime(uint64_t num);

#endif