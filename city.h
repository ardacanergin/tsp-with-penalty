// city.h
#include <stdint.h>

#ifndef CITY_H
#define CITY_H

typedef struct {
    int id;
    int x, y;
    uint64_t morton;
} City;

#endif