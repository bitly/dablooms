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

int test_scale(const char * filepath)
{
    FILE *fp, *file;
    char word[128];
    scaling_bloom_t *bloom;
    int i, iremove = 0;
    int not_exist_pass = 0, not_exist_fail = 0;
    int exist_pass = 0, exist_fail = 0;
    
    if ((file = fopen(FILEPATH, "r"))) {
        fclose(file);
        remove(FILEPATH);
    }
    
    if (!(bloom = new_scaling_bloom(CAPACITY, ERROR_RATE, FILEPATH, 0))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    
    if (!(fp = fopen(filepath, "r"))) {
        fprintf(stderr, "ERROR: Could not open words file\n");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, 128, fp); i++) {
        if (word != NULL) {
            chomp_line(word);
            scaling_bloom_add(bloom, word, i);
        }
    }
    
    fseek(fp, 0, SEEK_SET);
    for (iremove = 0; fgets(word, 128, fp); iremove++) {
        if (word != NULL) {
            if (iremove % 5 == 0) {
                chomp_line(word);
                scaling_bloom_remove(bloom, word, iremove);
            }
        }
    }
    
    
    bitmap_flush(bloom->bitmap);
    free_scaling_bloom(bloom);
    
    bloom = new_scaling_bloom_from_file(CAPACITY, ERROR_RATE, FILEPATH);
    
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
    
    fprintf(stderr, "\nElements Added:   %i\n", i);
    fprintf(stderr, "Elements Removed: %i\n\n", i/5);
    fprintf(stderr, "True positives:   %i\n", exist_pass);
    fprintf(stderr, "True negatives:   %i\n", not_exist_pass);
    fprintf(stderr, "False positives:  %i\n", not_exist_fail);
    fprintf(stderr, "False negatives:  %i\n\n", exist_fail);
    fprintf(stderr, "Total size: %i kB\n", (int) bloom->num_bytes/1024);
    
    fclose(fp);
    free_scaling_bloom(bloom);
    
    return 0;
}

int main(int argc, char *argv[])
{
    /*test_bitmap(); */
    const char *filepath; 
    if (argc != 2) {  
        fprintf(stderr, "Usage: %s <words_file>\n", argv[0]);
        return EXIT_FAILURE;
    } 
    filepath = argv[1]; 
    test_scale(filepath);
    return EXIT_SUCCESS;
}
