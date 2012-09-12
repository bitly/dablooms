phpdablooms
===========

## Install

```
cd phpdablooms
phpize
./configure
make
make install
# and add `extension=dablooms.so` to your php.ini
```
`phpize` comes with the `php5-dev` package in Debian and derivatives (including Ubuntu).

You can run some tests with `make test` or `time WORDS=/usr/share/dict/words ./test.sh`
after building phpdablooms.


## Usage

This extension provides `Dablooms\ScalingBloom` class.

```php
<?php
$capacity = 10000;
$error_rate = 0.05;
$file_path = "/tmp/dablooms_php.bin";
$id = 1;

$bloom = new Dablooms\ScalingBloom($capacity, $error_rate, $file_path);
$bloom->add("Hello", $id);
$ret = $bloom->check("Hello");
var_dump($ret);
```

## Available methods

```
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
```

## Limitations

`Dablooms\ScalingBloom::memSeqnum()` and `Dablooms\ScalingBloom::diskSeqnum()` will return
a double (float) value on 32bit machines. This is only accurate up to around 2^52
(4503599627370496), so in practice it should be ok.


## License

MIT License
