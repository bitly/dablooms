import sys, os
import pydablooms

bloom_fname = '/tmp/bloom.bin'
initial_capacity = 100000
error_rate = 0.05

print("pydablooms version: %s" % pydablooms.__version__)

if len(sys.argv) != 2:
    sys.stderr.write("Usage: %s <words_file>\n" % sys.argv[0])
    sys.exit(1)

words_fname = sys.argv[1]

bloom = pydablooms.Dablooms(capacity=initial_capacity,
                           error_rate=error_rate,
                           filepath=bloom_fname)

words_file = open(words_fname, 'rb')
i = 0
for line in words_file:
    bloom.add(line.rstrip(), i)
    i += 1

words_file.seek(0)
i = 0
for line in words_file:
    if i % 5 == 0:
        bloom.delete(line.rstrip(), i)
    i += 1

bloom.flush()
del bloom

bloom = pydablooms.load_dabloom(capacity=initial_capacity,
                                error_rate=error_rate,
                                filepath=bloom_fname)

true_positives = 0
true_negatives = 0
false_positives = 0
false_negatives = 0

words_file.seek(0)
i = 0
for line in words_file:
    exists = bloom.check(line.rstrip())
    
    if i % 5 == 0:
        if exists:
            false_positives += 1
        else:
            true_negatives += 1
    else:
        if exists:
            true_positives += 1
        else:
            false_negatives += 1
            sys.stderr.write("ERROR: False negative: '%s'\n" % line.rstrip())
    i += 1

words_file.close()
del bloom

false_positive_rate = float(false_positives) / (false_positives + true_negatives)

print('''
Elements Added:   %6d
Elements Removed: %6d

True Positives:   %6d
True Negatives:   %6d
False Positives:  %6d
False Negatives:  %6d

False positive rate: %.4f
Total Size: %d KiB''' % (
                         i, i/5,
                         true_positives,
                         true_negatives,
                         false_positives,
                         false_negatives,
                         false_positive_rate,
                         os.stat(bloom_fname).st_size / 1024
                        )
)

if false_negatives > 0:
    print("TEST FAIL (false negatives exist)")
elif false_positive_rate > error_rate:
    print("TEST FAIL (false positive rate too high)")
else:
    print("TEST PASS")
print("")

sys.exit(0)
