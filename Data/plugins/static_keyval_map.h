#ifndef STATIC_KV_MAP_H
#define STATIC_KV_MAP_H

#include <stdio.h>
#include <string.h>

#define STATIC_KV_MAP_SIZE 100

typedef struct
{
    char keys[STATIC_KV_MAP_SIZE][64];
    double values[STATIC_KV_MAP_SIZE];
} STATIC_KV_MAP;

static STATIC_KV_MAP static_kv_map_create()
{
    STATIC_KV_MAP map;
    memset(&map, 0, sizeof(STATIC_KV_MAP));
    return map;
}

int static_kv_map_set(const char *key, double value, STATIC_KV_MAP *map)
{
    int i;
    for (i = 0; map->keys[i][0] && i < STATIC_KV_MAP_SIZE; i++)
    {
    }
    if (i < STATIC_KV_MAP_SIZE)
    {
        strcpy(map->keys[i], key);
        map->values[i] = value;
        return 0;
    }
    return 1;
}

double static_kv_map_get(const char *key, STATIC_KV_MAP *map)
{
    for (int i = 0; map->keys[i][0] && i < STATIC_KV_MAP_SIZE; i++)
    {
        if (!strcmp(key, map->keys[i]))
            return map->values[i];
    }
    static_kv_map_set(key, 0, map);
    // The initial value is 0 so it can be returned without reading from map
    return 0;
}

#endif