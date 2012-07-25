/* Copyright @2012 by Justin Hines at Bitly under a very liberal license. See LICENSE in the source distribution. */

#ifndef __BLOOM_H__
#define __BLOOM_H__
#include<stdint.h>
#include<stdlib.h>

typedef struct {
    size_t bytes;
    int    fd;
    char  *array;
} bitmap_t;


bitmap_t *bitmap_resize(bitmap_t *bitmap, size_t old_size, size_t new_size, int fromfile);
bitmap_t *bitmap_create(int fd, size_t bytes, int fromfile);

int bitmap_increment(bitmap_t *bitmap, unsigned int index, unsigned int offset);
int bitmap_decrement(bitmap_t *bitmap, unsigned int index, unsigned int offset);
int bitmap_check(bitmap_t *bitmap, unsigned int index, unsigned int offset);

int bitmap_flush(bitmap_t *bitmap);
void bitmap_destroy(bitmap_t *bitmap);

int bitmap_get_bit(bitmap_t *bitmap, unsigned int index);
int bitmap_set_bit(bitmap_t *bitmap, unsigned int index, unsigned int val);

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
    unsigned char *salts;
    unsigned int *hashes;
    size_t nfuncs;
    size_t size;
    size_t num_bytes;
    double error_rate;
    bitmap_t *bitmap;
    
} counting_bloom_t;

int counting_bloom_destroy(counting_bloom_t *bloom);
int counting_bloom_add(counting_bloom_t *bloom, const char *s);
int counting_bloom_remove(counting_bloom_t *bloom, const char *s);
int counting_bloom_check(counting_bloom_t *bloom, const char *s);

typedef struct {
    uint64_t *preseq;
    uint64_t *posseq;
} scaling_bloom_header_t;

typedef struct {
    scaling_bloom_header_t *header;
    unsigned int capacity;
    unsigned int num_blooms;
    size_t num_bytes;
    size_t size;
    double error_rate;
    int fd;
    counting_bloom_t **blooms;
    bitmap_t *bitmap;
} scaling_bloom_t;

scaling_bloom_t *scaling_bloom_create(unsigned int capacity, double error_rate, const char *filename, uint32_t id);
scaling_bloom_t *scaling_bloom_from_file(unsigned int capacity, double error_rate, const char *filename);
int scaling_bloom_destroy(scaling_bloom_t *bloom);
int scaling_bloom_add(scaling_bloom_t *bloom, const char *s, uint32_t id);
int scaling_bloom_remove(scaling_bloom_t *bloom, const char *s, uint32_t id);
int scaling_bloom_check(scaling_bloom_t *bloom, const char *s);
int scaling_bloom_flush(scaling_bloom_t *bloom);
#endif
