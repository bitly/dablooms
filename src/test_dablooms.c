#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "dablooms.h"

#define CAPACITY 100000
#define ERROR_RATE .05

struct stats {
    int true_positives;
    int true_negatives;
    int false_positives;
    int false_negatives;
};

static void chomp_line(char *word)
{
    char *p;
    if ((p = strchr(word, '\r'))) {
        *p = '\0';
    }
    if ((p = strchr(word, '\n'))) {
        *p = '\0';
    }
}

static void print_results(struct stats *stats)
{
    float false_positive_rate = (float)stats->false_positives /
                                (stats->false_positives + stats->true_negatives);
                                
    printf("True positives:     %7d"   "\n"
           "True negatives:     %7d"   "\n"
           "False positives:    %7d"   "\n"
           "False negatives:    %7d"   "\n"
           "False positive rate: %.4f" "\n",
           stats->true_positives,
           stats->true_negatives,
           stats->false_positives,
           stats->false_negatives,
           false_positive_rate             );
           
    if (stats->false_negatives > 0) {
        printf("TEST FAIL (false negatives exist)\n");
    } else if (false_positive_rate > ERROR_RATE) {
        printf("TEST FAIL (false positive rate too high)\n");
    } else {
        printf("TEST PASS\n");
    }
}

static void bloom_score(int positive, int should_positive, struct stats *stats, const char *key)
{
    if (should_positive) {
        if (positive) {
            stats->true_positives++;
        } else {
            stats->false_negatives++;
            fprintf(stderr, "ERROR: False negative: '%s'\n", key);
        }
    } else {
        if (positive) {
            stats->false_positives++;
        } else {
            stats->true_negatives++;
        }
    }
}

int test_counting_remove_reopen(const char *bloom_file, const char *words_file)
{
    FILE *fp;
    char word[256];
    counting_bloom_t *bloom;
    int i, key_removed;
    struct stats results = { 0 };
    
    printf("\n* test counting remove & reopen\n");
    
    if ((fp = fopen(bloom_file, "r"))) {
        fclose(fp);
        remove(bloom_file);
    }
    
    if (!(bloom = new_counting_bloom(CAPACITY, ERROR_RATE, bloom_file))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    if (!(fp = fopen(words_file, "r"))) {
        fprintf(stderr, "ERROR: Could not open words file\n");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, sizeof(word), fp) && (i < CAPACITY); i++) {
        chomp_line(word);
        counting_bloom_add(bloom, word, strlen(word));
    }
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp) && (i < CAPACITY); i++) {
        if (i % 5 == 0) {
            chomp_line(word);
            counting_bloom_remove(bloom, word, strlen(word));
        }
    }
    
    free_counting_bloom(bloom);
    bloom = new_counting_bloom_from_file(CAPACITY, ERROR_RATE, bloom_file);
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; (fgets(word, sizeof(word), fp)) && (i < CAPACITY); i++) {
        chomp_line(word);
        key_removed = (i % 5 == 0);
        bloom_score(counting_bloom_check(bloom, word, strlen(word)), !key_removed, &results, word);
    }
    fclose(fp);
    
    printf("Elements added:   %6d" "\n"
           "Elements removed: %6d" "\n"
           "Total size: %d KiB"  "\n\n",
           i, i / 5,
           (int) bloom->num_bytes / 1024);
           
    free_counting_bloom(bloom);
    
    print_results(&results);
    return EXIT_SUCCESS;
}

int test_scaling_remove_reopen(const char *bloom_file, const char *words_file)
{
    FILE *fp;
    char word[256];
    scaling_bloom_t *bloom;
    int i, key_removed;
    struct stats results = { 0 };
    
    printf("\n* test scaling remove & reopen\n");
    
    if ((fp = fopen(bloom_file, "r"))) {
        fclose(fp);
        remove(bloom_file);
    }
    
    if (!(bloom = new_scaling_bloom(CAPACITY, ERROR_RATE, bloom_file))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    
    if (!(fp = fopen(words_file, "r"))) {
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
    
    bloom = new_scaling_bloom_from_file(CAPACITY, ERROR_RATE, bloom_file);
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        chomp_line(word);
        key_removed = (i % 5 == 0);
        bloom_score(scaling_bloom_check(bloom, word, strlen(word)), !key_removed, &results, word);
    }
    fclose(fp);
    
    printf("Elements added:   %6d" "\n"
           "Elements removed: %6d" "\n"
           "Total size: %d KiB"  "\n\n",
           i, i / 5,
           (int) bloom->num_bytes / 1024);
           
    free_scaling_bloom(bloom);
    
    print_results(&results);
    return EXIT_SUCCESS;
}

int test_counting_accuracy(const char *bloom_file, const char *words_file)
{
    FILE *fp;
    char word[256];
    counting_bloom_t *bloom;
    int i;
    struct stats results = { 0 };
    
    printf("\n* test counting accuracy\n");
    
    if ((fp = fopen(bloom_file, "r"))) {
        fclose(fp);
        remove(bloom_file);
    }
    
    if (!(bloom = new_counting_bloom(CAPACITY, ERROR_RATE, bloom_file))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    if (!(fp = fopen(words_file, "r"))) {
        fprintf(stderr, "ERROR: Could not open words file\n");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, sizeof(word), fp) && (i < CAPACITY * 2); i++) {
        if (i % 2 == 0) {
            chomp_line(word);
            counting_bloom_add(bloom, word, strlen(word));
        }
    }
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp) && (i < CAPACITY * 2); i++) {
        if (i % 2 == 1) {
            chomp_line(word);
            bloom_score(counting_bloom_check(bloom, word, strlen(word)), 0, &results, word);
        }
    }
    
    fclose(fp);
    
    printf("Elements added:   %6d" "\n"
           "Elements checked: %6d" "\n"
           "Total size: %d KiB"  "\n\n",
           (i + 1) / 2, i / 2,
           (int) bloom->num_bytes / 1024);
           
    free_counting_bloom(bloom);
    
    print_results(&results);
    return EXIT_SUCCESS;
}

int test_scaling_accuracy(const char *bloom_file, const char *words_file)
{
    FILE *fp;
    char word[256];
    scaling_bloom_t *bloom;
    int i;
    struct stats results = { 0 };
    
    printf("\n* test scaling accuracy\n");
    
    if ((fp = fopen(bloom_file, "r"))) {
        fclose(fp);
        remove(bloom_file);
    }
    
    if (!(bloom = new_scaling_bloom(CAPACITY, ERROR_RATE, bloom_file))) {
        fprintf(stderr, "ERROR: Could not create bloom filter\n");
        return EXIT_FAILURE;
    }
    
    if (!(fp = fopen(words_file, "r"))) {
        fprintf(stderr, "ERROR: Could not open words file\n");
        return EXIT_FAILURE;
    }
    
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        if (i % 2 == 0) {
            chomp_line(word);
            scaling_bloom_add(bloom, word, strlen(word), i);
        }
    }
    
    fseek(fp, 0, SEEK_SET);
    for (i = 0; fgets(word, sizeof(word), fp); i++) {
        if (i % 2 == 1) {
            chomp_line(word);
            bloom_score(scaling_bloom_check(bloom, word, strlen(word)), 0, &results, word);
        }
    }
    
    fclose(fp);
    
    printf("Elements added:   %6d" "\n"
           "Elements checked: %6d" "\n"
           "Total size: %d KiB"  "\n\n",
           (i + 1) / 2, i / 2,
           (int) bloom->num_bytes / 1024);
           
    free_scaling_bloom(bloom);
    
    print_results(&results);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    printf("\n** dablooms version: %s\n", dablooms_version());
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <bloom_file> <words_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (test_counting_remove_reopen(argv[1], argv[2]) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (test_counting_accuracy(argv[1], argv[2]) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (test_scaling_remove_reopen(argv[1], argv[2]) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (test_scaling_accuracy(argv[1], argv[2]) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
