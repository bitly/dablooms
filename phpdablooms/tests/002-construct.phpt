--TEST--
Check for Dablooms\ScalingBloom::__construct
--FILE--
<?php
define("FIXTURE_PATH", __DIR__ . DIRECTORY_SEPARATOR . "fixtures");
define("DABLOOMS_FILE", FIXTURE_PATH . DIRECTORY_SEPARATOR . "test.dat");
define("DABLOOMS_CAPACITY", 100000);
define("DABLOOMS_ERROR_RATE", 0.05);

if (is_file(DABLOOMS_FILE)) {
	unlink(DABLOOMS_FILE);
}

$s = new Dablooms\ScalingBloom(DABLOOMS_CAPACITY, DABLOOMS_ERROR_RATE, DABLOOMS_FILE);

if (is_file(DABLOOMS_FILE)) {
	echo "OK" . PHP_EOL;
} else {
	echo "FAILED" . PHP_EOL;
}

--EXPECT--
OK
