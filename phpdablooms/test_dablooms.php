<?php
printf("dablooms version: %s\n", Dablooms::VERSION);

define("DABLOOMS_FILE", "/tmp/bloom_php.bin");
define("DABLOOMS_CAPACITY", 100000);
define("DABLOOMS_ERROR_RATE", 0.05);

function print_results($stats)
{
    $false_positive_rate = (float)$stats['false_positives'] /
                                ($stats['false_positives'] + $stats['true_negatives']);
                                
    printf("True positives:     %7d" .  "\n" .
           "True negatives:     %7d" .  "\n" .
           "False positives:    %7d" .  "\n" .
           "False negatives:    %7d" .  "\n" .
           "False positive rate: %.4f" . "\n",
           $stats['true_positives'],
           $stats['true_negatives'],
           $stats['false_positives'],
           $stats['false_negatives'],
           $false_positive_rate             );
           
    if ($stats['false_negatives'] > 0) {
        printf("TEST FAIL (false negatives exist)\n");
    } else if ($false_positive_rate > DABLOOMS_ERROR_RATE) {
        printf("TEST FAIL (false positive rate too high)\n");
    } else {
        printf("TEST PASS\n");
    }
}

function bloom_check_and_score($bloom, $key,
                                  $should_positive, &$stats)
{
    $positive = $bloom->check($key);
    if ($should_positive) {
        if ($positive) {
            $stats['true_positives']++;
        } else {
            $stats['false_negatives']++;
            printf("ERROR: False negative: '%s'\n", $key);
        }
    } else {
        if ($positive) {
            $stats['false_positives']++;
        } else {
            $stats['true_negatives']++;
        }
    }
}

function test_remove_reopen($bloom_file, $words_file)
{
    $results = array(
        "true_positives" => 0,
        "false_negatives" => 0,
        "false_positives" => 0,
        "true_negatives" => 0,
    );

    printf("\n* test remove & reopen\n");
    
    if (($fp = @fopen($bloom_file, "r"))) {
        fclose($fp);
        unlink($bloom_file);
    }
    
    if (!($bloom = new Dablooms\ScalingBloom(DABLOOMS_CAPACITY, DABLOOMS_ERROR_RATE, $bloom_file))) {
        printf("ERROR: Could not create bloom filter\n");
        return -1;
    }
    
    if (!($fp = @fopen($words_file, "r"))) {
        printf("ERROR: Could not open words file\n");
        return -1;
    }
    
    for ($i = 0; $word = fgets($fp, 8192); $i++) {
        $word = trim($word);
        $bloom->add($word, $i);
    }

    fseek($fp, 0, SEEK_SET);
    for ($i = 0; $word = fgets($fp, 8192); $i++) {
        if ($i % 5 == 0) {
            $word = trim($word);
            $bloom->remove($word, $i);
        }
    }
    
    $bloom->bitmapFlush();
    unset($bloom);
    
    $bloom = Dablooms\ScalingBloom::loadFromFile(DABLOOMS_CAPACITY, DABLOOMS_ERROR_RATE, $bloom_file);
    
    fseek($fp, 0, SEEK_SET);
    for ($i = 0; $word = fgets($fp); $i++) {
        $word = trim($word);
        $key_removed = ($i % 5 == 0);
        bloom_check_and_score($bloom, $word, !$key_removed, $results);
    }
    fclose($fp);
    
    printf("Elements added:   %6d" ."\n".
           "Elements removed: %6d" ."\n".
           "Total size: %d KiB"  ."\n\n",
           $i, $i / 5,
           $bloom->getSize() / 1024);
           
    unset($bloom);
    
    print_results($results);
    return 0;
}

function test_accuracy($bloom_file, $words_file)
{
    printf("\n* test accuracy\n");
    $results = array(
        "true_positives" => 0,
        "false_negatives" => 0,
        "false_positives" => 0,
        "true_negatives" => 0,
    );

    if (($fp = @fopen($bloom_file, "r"))) {
        fclose($fp);
        unlink($bloom_file);
    }
    
    if (!($bloom = new Dablooms\ScalingBloom(DABLOOMS_CAPACITY, DABLOOMS_ERROR_RATE, $bloom_file))) {
        printf("ERROR: Could not create bloom filter\n");
        return -1;
    }
    
    if (!($fp = @fopen($words_file, "r"))) {
        printf("ERROR: Could not open words file\n");
        return -1;
    }
    
    for ($i = 0; $word = fgets($fp, 8192); $i++) {
        if ($i % 2 == 0) {
            $word = trim($word);
            $bloom->add($word, $i);
        }
    }
    
    fseek($fp, 0, SEEK_SET);
    for ($i = 0; $word = fgets($fp, 8192); $i++) {
        if ($i % 2 == 1) {
            $word = trim($word);
            bloom_check_and_score($bloom, $word, 0, $results);
        }
    }
    
    fclose($fp);
    
    printf("Elements added:   %6d" . "\n".
           "Elements checked: %6d" . "\n".
           "Total size: %d KiB" . "\n\n",
           ($i + 1) / 2, $i / 2,
           (int) $bloom->getSize()/ 1024);
           
    unset($bloom);
    
    print_results($results);
    return 0;
}

function main()
{
    printf("\n** dablooms version: %s\n", Dablooms::VERSION);
    
    if ($_SERVER['argc'] != 3) {
        printf("Usage: %s <bloom_file> <words_file>\n", $_SERVER['argv'][0]);
        return -1;
    }
    if (test_remove_reopen($_SERVER['argv'][1], $_SERVER['argv'][2]) != 0) {
        return -1;
    }
    if (test_accuracy($_SERVER['argv'][1], $_SERVER['argv'][2]) != 0) {
        return -1;
    }
    return 0;
}

main();
