#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "dablooms.h"

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
    char word[256];
    scaling_bloom_t *bloom;
    float false_positive_rate;
    int i, exists;
    int true_positives = 0, true_negatives = 0,
        false_positives = 0, false_negatives = 0;
    
    if ((file = fopen(FILEPATH, "r"))) {
        fclose(file);
        remove(FILEPATH);
    }
    
    if (!(bloom = new_scaling_bloom(CAPACITY, ERROR_RATE, FILEPATH))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    
    if (!(fp = fopen(filepath, "r"))) {
        fprintf(stderr, "ERROR: Could not open words file\n");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        chomp_line(word);
        scaling_bloom_add(bloom, word, strlen(word), i);
    }
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        if (i % 5 == 0) {
            chomp_line(word);
            scaling_bloom_remove(bloom, word, strlen(word), i);
        }
    }
    
    
    bitmap_flush(bloom->bitmap);
    free_scaling_bloom(bloom);
    
    bloom = new_scaling_bloom_from_file(CAPACITY, ERROR_RATE, FILEPATH);
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        chomp_line(word);
        exists = scaling_bloom_check(bloom, word, strlen(word));
        if (i % 5 == 0) {
            /* this element was removed above */
            if (exists) {
                false_positives++;
            } else {
                true_negatives++;
            }
        } else {
            /* this element should still exist */
            if (exists) {
                true_positives++;
            } else {
                false_negatives++;
                fprintf(stderr, "ERROR: False negative: '%s'\n", word);
            }
        }
    }
    fclose(fp);
    
    false_positive_rate = (float)false_positives / (false_positives + true_negatives);
    
    printf(                         "\n"
           "Elements added:   %6d"  "\n"
           "Elements removed: %6d"  "\n"
                                    "\n"
           "True positives:   %6d"  "\n"
           "True negatives:   %6d"  "\n"
           "False positives:  %6d"  "\n"
           "False negatives:  %6d"  "\n"
                                    "\n"
           "False positive rate: %.4f\n"
           "Total size: %d KiB"     "\n",
           i, i/5,
           true_positives, true_negatives,
           false_positives, false_negatives,
           false_positive_rate,
           (int) bloom->num_bytes/1024
          );
    
    free_scaling_bloom(bloom);
    
    if (false_negatives > 0) {
        printf("TEST FAIL (false negatives exist)\n");
    } else if (false_positive_rate > ERROR_RATE) {
        printf("TEST FAIL (false positive rate too high)\n");
    } else {
        printf("TEST PASS\n");
    }
    printf("\n");
    
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    printf("dablooms version: %s\n", dablooms_version());
    
    /*test_bitmap(); */
    const char *filepath; 
    if (argc != 2) {  
        fprintf(stderr, "Usage: %s <words_file>\n", argv[0]);
        return EXIT_FAILURE;
    } 
    filepath = argv[1]; 
    return test_scale(filepath);
}
