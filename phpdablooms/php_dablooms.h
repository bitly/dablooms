#ifndef PHP_DABLOOMS_H

#define PHP_DABLOOMS_H

#define PHP_DABLOOMS_EXTNAME "dablooms"
#define PHP_DABLOOMS_EXTVER "0.9.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "dablooms.h"
#include "murmur.h"

#include "zend_interfaces.h"

/* Define the entry point symbol
 * Zend will use when loading this module
 */
extern zend_module_entry dablooms_module_entry;
#define phpext_dablooms_ptr &dablooms_module_entry

extern zend_class_entry *dablooms_class_entry;
extern zend_class_entry *dablooms_scalingbloom_class_entry;

typedef struct{
	zend_object zo;
	scaling_bloom_t *filter;
} php_dablooms_scalingbloom_t;

#endif /* PHP_DABLOOMS_H */
