#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>

#include"dablooms.h"

#define FILEPATH "/tmp/bloom.bin"
#define CAPACITY 100000
#define ERROR_RATE .05

int test_bitmap()
{
    int fd;
    int fail = 0;
    int pass = 0;
    int i = 0;
    bitmap_t *map;
    
    FILE *file;
    if ((file = fopen(FILEPATH, "r"))) {
        fclose(file);
        remove(FILEPATH);
    }
    
    if (!(fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600))) {
        fprintf(stderr, "ERROR: Could not open file %s with open\n", FILEPATH);
        return EXIT_FAILURE;
    }
    
    if (!(map = bitmap_create(fd, 1000, 0))) {
        fprintf(stderr, "ERROR: Could not create bitmap with file");
        return EXIT_FAILURE;
    }
    
    for (i = 0; i < 2000; i++) {
        bitmap_increment(map, i, 0);
    }
    
    for (i = 0; i < 2000; i++) {
        if (bitmap_check(map, i, 0)) {
            pass++;
        } else {
            fail++;
        }
    }
    
    for (i = 0; i < 2000; i++) {
        bitmap_decrement(map, i, 0);
    }
    
    for (i = 0; i < 2000; i++) {
        if (bitmap_check(map, i, 0)) {
            fail++;
        } else {
            pass++;
        }
    }
    
    if (fail) {
        fprintf(stderr, "failures %i\n", fail);
    } else {
        fprintf(stderr, ".");
    }
    bitmap_destroy(map);
    
    return 0;
}

void chomp_line(char *word)
{
    char *p;
    if ((p = strchr(word, '\r'))) {
        *p = '\0';
    }
    if ((p = strchr(word, '\n'))) {
        *p = '\0';
    }
}

int test_scale()
{
    FILE *fp, *file;
    char word[128];
    scaling_bloom_t *bloom;
    int i = 0;
    int not_exist_pass = 0, not_exist_fail = 0;
    int exist_pass = 0, exist_fail = 0;
    
    if ((file = fopen(FILEPATH, "r"))) {
        fclose(file);
        remove(FILEPATH);
    }
    
    if (!(bloom = scaling_bloom_create(CAPACITY, ERROR_RATE, FILEPATH, 0))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    
    if (!(fp = fopen("/usr/share/dict/words", "r"))) {
        fprintf(stderr, "ERROR: Could not open words file");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, 128, fp); i++) {
        if (word != NULL) {
            chomp_line(word);
            scaling_bloom_add(bloom, word, i);
        }
    }
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, 128, fp); i++) {
        if (word != NULL) {
            if (i % 5 == 0) {
                chomp_line(word);
                scaling_bloom_remove(bloom, word, i);
            }
        }
    }
    
    
    bitmap_flush(bloom->bitmap);
    scaling_bloom_destroy(bloom);
    
    bloom = scaling_bloom_from_file(CAPACITY, ERROR_RATE, FILEPATH);
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, 128, fp); i++) {
        if (word != NULL) {
            chomp_line(word);
            if (i % 5 == 0) {
                if (!(scaling_bloom_check(bloom, word))) {
                    not_exist_pass ++;
                } else {
                    not_exist_fail ++;
                }
            } else {
                if (scaling_bloom_check(bloom, word)) {
                    exist_pass ++;
                } else {
                    fprintf(stderr, "%s\n", word);
                    exist_fail ++;
                }
            }
        }
    }
    
    fprintf(stderr, "non exist pass: %i\n", not_exist_pass);
    fprintf(stderr, "non exist fail: %i\n", not_exist_fail);
    fprintf(stderr, "exist pass:     %i\n", exist_pass);
    fprintf(stderr, "exist fail:     %i\n", exist_fail);
    
    fclose(fp);
    scaling_bloom_destroy(bloom);
    
    return 0;
}

int main(int argc, char *argv[])
{
    /*test_bitmap(); */
    test_scale();
    
    fprintf(stderr, "\n");
    
    return EXIT_SUCCESS;
}
