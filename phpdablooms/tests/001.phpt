--TEST--
Check for dablooms presence
--SKIPIF--
<?php if (!extension_loaded("dablooms")) print "skip"; ?>
--FILE--
<?php
echo "dablooms extension is available";
--EXPECT--
dablooms extension is available
