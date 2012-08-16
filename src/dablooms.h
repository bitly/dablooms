/* Copyright @2012 by Justin Hines at Bitly under a very liberal license. See LICENSE in the source distribution. */

#ifndef __BLOOM_H__
#define __BLOOM_H__
#include <stdint.h>
#include <stdlib.h>

const char *dablooms_version(void);

typedef struct {
    size_t bytes;
    int    fd;
    char  *array;
} bitmap_t;


bitmap_t *bitmap_resize(bitmap_t *bitmap, size_t old_size, size_t new_size);
bitmap_t *new_bitmap(int fd, size_t bytes);

int bitmap_increment(bitmap_t *bitmap, unsigned int index, unsigned int offset);
int bitmap_decrement(bitmap_t *bitmap, unsigned int index, unsigned int offset);
int bitmap_check(bitmap_t *bitmap, unsigned int index, unsigned int offset);
int bitmap_flush(bitmap_t *bitmap);

void free_bitmap(bitmap_t *bitmap);

typedef struct {
    uint32_t *count;
    uint32_t *id;
} counting_bloom_header_t;


typedef struct {
    counting_bloom_header_t *header;
    unsigned int capacity;
    unsigned int offset;
    unsigned int counts_per_func;
    unsigned int num_salts;
    uint32_t *hashes;
    uint32_t *salts;
    size_t nfuncs;
    size_t size;
    size_t num_bytes;
    double error_rate;
    bitmap_t *parent_bitmap;
} counting_bloom_t;

int free_counting_bloom(counting_bloom_t *bloom);
counting_bloom_t *new_counting_bloom(unsigned int capacity, double error_rate, const char *filename);
counting_bloom_t *new_counting_bloom_from_file(unsigned int capacity, double error_rate, const char *filename);
int counting_bloom_add(counting_bloom_t *bloom, const char *s, size_t len);
int counting_bloom_remove(counting_bloom_t *bloom, const char *s, size_t len);
int counting_bloom_check(counting_bloom_t *bloom, const char *s, size_t len);


typedef struct {
    uint64_t *preseq;
    uint64_t *posseq;
    uint64_t *max_id;
} scaling_bloom_header_t;

typedef struct {
    scaling_bloom_header_t *header;
    unsigned int capacity;
    unsigned int num_blooms;
    size_t num_bytes;
    double error_rate;
    int fd;
    counting_bloom_t **blooms;
    bitmap_t *bitmap;
} scaling_bloom_t;

scaling_bloom_t *new_scaling_bloom(unsigned int capacity, double error_rate, const char *filename);
scaling_bloom_t *new_scaling_bloom_from_file(unsigned int capacity, double error_rate, const char *filename);
int free_scaling_bloom(scaling_bloom_t *bloom);
int scaling_bloom_add(scaling_bloom_t *bloom, const char *s, size_t len, uint32_t id);
int scaling_bloom_remove(scaling_bloom_t *bloom, const char *s, size_t len, uint32_t id);
int scaling_bloom_check(scaling_bloom_t *bloom, const char *s, size_t len);
int scaling_bloom_flush(scaling_bloom_t *bloom);
#endif
