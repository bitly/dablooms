/**
 * php-dablooms extension.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *    
 * Authors: Shuhei Tanuma <chobieeee@php.net>
 */
#include "php_dablooms.h"
#include "ext/standard/info.h"
#include "ext/spl/spl_exceptions.h"

#ifndef PHP_DABLOOMS_DEBUG
#define PHP_DABLOOMS_DEBUG 0
#endif

/* this macro will returned as double as we cannot store potential 64-bit values in longs. */
#define PHP_DABLOOMS_RETURN_DOUBLE(value) \
	if (sizeof(long) > 4) { \
		RETURN_LONG(value); \
	} else { \
		RETURN_DOUBLE((double)value); \
	}\

zend_class_entry *dablooms_class_entry;
zend_class_entry *dablooms_scalingbloom_class_entry;

static void php_dablooms_init(TSRMLS_D);
static void php_dablooms_scalingbloom_init(TSRMLS_D);

static void php_dablooms_scalingbloom_free_storage(php_dablooms_scalingbloom_t *obj TSRMLS_DC)
{
	zend_object_std_dtor(&obj->zo TSRMLS_CC);
	if (obj->filter != NULL) {
		free_scaling_bloom(obj->filter);
		obj->filter = NULL;
	}

	efree(obj);
}

zend_object_value php_dablooms_scalingbloom_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	php_dablooms_scalingbloom_t *obj;

	obj = ecalloc(1, sizeof(*obj));
	obj->filter = NULL;
	zend_object_std_init( &obj->zo, ce TSRMLS_CC );
#if ZEND_MODULE_API_NO >= 20100525
	object_properties_init(&(obj->zo), ce); 
#else
	{
		zval *tmp;
		zend_hash_copy(obj->zo.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	}
#endif
	
	retval.handle = zend_objects_store_put(obj, 
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)php_dablooms_scalingbloom_free_storage,
		NULL TSRMLS_CC);
	retval.handlers = zend_get_std_object_handlers();
	return retval;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_dablooms_scalingbloom___construct, 0, 0, 3)
	ZEND_ARG_INFO(0, capacity)
	ZEND_ARG_INFO(0, error_rate)
	ZEND_ARG_INFO(0, filepath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dablooms_scalingbloom_load_from_file, 0, 0, 3)
	ZEND_ARG_INFO(0, capacity)
	ZEND_ARG_INFO(0, error_rate)
	ZEND_ARG_INFO(0, filepath)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dablooms_scalingbloom_add, 0, 0, 2)
	ZEND_ARG_INFO(0, hash)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dablooms_scalingbloom_remove, 0, 0, 2)
	ZEND_ARG_INFO(0, hash)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_dablooms_scalingbloom_check, 0, 0, 1)
	ZEND_ARG_INFO(0, hash)
ZEND_END_ARG_INFO()

/* {{{ proto Dablooms\ScalingBloom::__construct(long $capacity, double $error_rate, string $filepath)
*/
PHP_METHOD(dablooms_scalingbloom, __construct)
{
	long capacity = 0;
	double error_rate;
	char *filepath;
	int filepath_len = 0;
	php_dablooms_scalingbloom_t *dablooms;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"lds", &capacity, &error_rate, &filepath, &filepath_len) == FAILURE) {
		return;
	}
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	dablooms->filter = new_scaling_bloom(capacity, error_rate, filepath);

	if (!dablooms->filter) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, "could not create bloom filter");
	}
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::add(string $hash, long $id)
*/
PHP_METHOD(dablooms_scalingbloom, add)
{
	char *hash;
	int hash_len = 0;
	long id = 0;
	php_dablooms_scalingbloom_t *dablooms;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"sl", &hash, &hash_len, &id) == FAILURE) {
		return;
	}
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(scaling_bloom_add(dablooms->filter, hash, hash_len, id));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::remove(string $hash, long $id)
*/
PHP_METHOD(dablooms_scalingbloom, remove)
{
	char *hash;
	int hash_len = 0;
	long id = 0;
	php_dablooms_scalingbloom_t *dablooms;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"sl", &hash, &hash_len, &id) == FAILURE) {
		return;
	}
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(scaling_bloom_remove(dablooms->filter, hash, hash_len, id));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::check(string $hash)
*/
PHP_METHOD(dablooms_scalingbloom, check)
{
	char *hash;
	int hash_len = 0;
	php_dablooms_scalingbloom_t *dablooms;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s", &hash, &hash_len) == FAILURE) {
		return;
	}
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_BOOL(scaling_bloom_check(dablooms->filter, hash, hash_len));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::flush()
*/
PHP_METHOD(dablooms_scalingbloom, flush)
{
	php_dablooms_scalingbloom_t *dablooms;
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_BOOL(scaling_bloom_flush(dablooms->filter));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::bitmapFlush()
*/
PHP_METHOD(dablooms_scalingbloom, bitmapFlush)
{
	php_dablooms_scalingbloom_t *dablooms;
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_BOOL(bitmap_flush(dablooms->filter->bitmap));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::memSeqnum()
*/
PHP_METHOD(dablooms_scalingbloom, memSeqnum)
{
	php_dablooms_scalingbloom_t *dablooms;
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	PHP_DABLOOMS_RETURN_DOUBLE((double)scaling_bloom_mem_seqnum(dablooms->filter));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::diskSeqnum()
*/
PHP_METHOD(dablooms_scalingbloom, diskSeqnum)
{
	php_dablooms_scalingbloom_t *dablooms;
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	PHP_DABLOOMS_RETURN_DOUBLE((double)scaling_bloom_disk_seqnum(dablooms->filter));
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::getSize()
*/
PHP_METHOD(dablooms_scalingbloom, getSize)
{
	php_dablooms_scalingbloom_t *dablooms;
	
	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG((long)dablooms->filter->num_bytes);
}
/* }}} */

/* {{{ proto Dablooms\ScalingBloom::loadFromFile(long $capacity, double $error_rate, string $filepath)
*/
PHP_METHOD(dablooms_scalingbloom, loadFromFile)
{
	long capacity = 0;
	double error_rate;
	char *filepath;
	int filepath_len = 0;
	php_dablooms_scalingbloom_t *dablooms;
	zend_object_value val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"lds", &capacity, &error_rate, &filepath, &filepath_len) == FAILURE) {
		return;
	}
	
	val = php_dablooms_scalingbloom_new(dablooms_scalingbloom_class_entry TSRMLS_CC);

	return_value->type = IS_OBJECT;
	return_value->value.obj = val;

	dablooms = (php_dablooms_scalingbloom_t *) zend_object_store_get_object(return_value TSRMLS_CC);
	dablooms->filter = new_scaling_bloom_from_file(capacity, error_rate, filepath);

	if (!dablooms->filter) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not load bloom filter");
		RETURN_FALSE;
	}
}
/* }}} */

static zend_function_entry php_dablooms_methods[] = {
	{NULL, NULL, NULL}
};

static zend_function_entry php_dablooms_scalingbloom_methods[] = {
	PHP_ME(dablooms_scalingbloom, __construct, arginfo_dablooms_scalingbloom___construct, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, loadFromFile, arginfo_dablooms_scalingbloom_load_from_file, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(dablooms_scalingbloom, add, arginfo_dablooms_scalingbloom_add, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, remove, arginfo_dablooms_scalingbloom_remove, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, check, arginfo_dablooms_scalingbloom_check, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, flush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, bitmapFlush, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, memSeqnum, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, diskSeqnum, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(dablooms_scalingbloom, getSize, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static void php_dablooms_init(TSRMLS_D)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Dablooms", php_dablooms_methods);
	dablooms_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	zend_declare_class_constant_string(dablooms_class_entry, "VERSION", sizeof("VERSION")-1, dablooms_version() TSRMLS_CC);
}

static void php_dablooms_scalingbloom_init(TSRMLS_D)
{
	zend_class_entry ce;
	INIT_NS_CLASS_ENTRY(ce, "Dablooms", "ScalingBloom", php_dablooms_scalingbloom_methods);
	dablooms_scalingbloom_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	dablooms_scalingbloom_class_entry->create_object = php_dablooms_scalingbloom_new;
}

PHP_MINIT_FUNCTION(dablooms)
{
	php_dablooms_init(TSRMLS_C);
	php_dablooms_scalingbloom_init(TSRMLS_C);
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(dablooms)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(dablooms)
{
	php_printf("PHP dablooms Extension\n");
	php_info_print_table_row(2, "Version", PHP_DABLOOMS_EXTVER);
	php_info_print_table_row(2, "Dablooms Version", dablooms_version());
}

zend_module_entry dablooms_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"dablooms",
	NULL,					/* Functions */
	PHP_MINIT(dablooms),	/* MINIT */
	NULL,					/* MSHUTDOWN */
	NULL,					/* RINIT */
	PHP_RSHUTDOWN(dablooms),		/* RSHUTDOWN */
	PHP_MINFO(dablooms),	/* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
	PHP_DABLOOMS_EXTVER,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_DABLOOMS
ZEND_GET_MODULE(dablooms)
#endif
