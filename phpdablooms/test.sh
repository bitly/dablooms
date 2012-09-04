#!/bin/sh
php --php-ini test.ini test_dablooms.php testbloom.bin ${WORDS:-/usr/share/dict/words}
