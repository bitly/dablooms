# php-dablooms

## Usage

this extension provides `Dablooms\ScalingBloom` class.

````php
<?php
$capacity = 10000;
$error_rate = 0.05;
$file_path = "/tmp/dablooms_php.bin";
$id = 1;

$bloom = new Dablooms\ScalingBloom($capacity, $error_rate, $file_path);
$bloom->add("Hello", $id);
$ret = $bloom->check("Hello");
var_dump($ret);
````

## How to install?

````
cd phpdablooms
phpize
./configure
make
make install
# and add `extension=dablooms.so` to your php.ini
````

- Note: you can install phpize with `apt-get install php5-dev`


you can test with following command

````
time php test_dablooms.php /tmp/php_dablooms.bin /usr/share/dict/words

# dablooms version: 0.8.2
# 
# ** dablooms version: 0.8.2
# 
# * test remove & reopen
# Elements added:   235886
# Elements removed:  47177
# Total size: 1130 KiB
# 
# True positives:      188708
# True negatives:       45952
# False positives:       1226
# False negatives:          0
# False positive rate: 0.0260
# TEST PASS
# 
# * test accuracy
# Elements added:   117943
# Elements checked: 117943
# Total size: 717 KiB
# 
# True positives:           0
# True negatives:      113816
# False positives:       4127
# False negatives:          0
# False positive rate: 0.0350
# TEST PASS
# 
# real	0m5.686s
# user	0m5.279s
# sys	0m0.046s
# 
````

you know, PHP is weak in test case like this. but useful.

## Available methods

````
Dablooms\ScalingBloom::__construct(long $capacity, double $error_rate, string $filepath)
Dablooms\ScalingBloom::loadFromFile(long $capacity, double $error_rate, string $filepath)
long Dablooms\ScalingBloom::add(string $hash, long $id)
long Dablooms\ScalingBloom::remove(string $hash)
bool Dablooms\ScalingBloom::check(string $hash)
void Dablooms\ScalingBloom::flush()
void Dablooms\ScalingBloom::bitmapFlush()
long Dablooms\ScalingBloom::memSeqnum()
long Dablooms\ScalingBloom::diskSeqnum()
long Dablooms\ScalingBloom::getSize()
````

## Limitations

`Dablooms\ScalingBloom::memSeqnum()` and `Dablooms\ScalingBloom::diskSeqnum()` will return float value on 32bit machine.
this is the known limitation. it's depends on your platform.


## License

MIT License