PHP_ARG_ENABLE(dablooms, Whether to enable the "dablooms" extension,
[  --enable-dablooms     Enable "dablooms" extension support])

if test $PHP_DABLOOMS != "no"; then
    PHP_NEW_EXTENSION(dablooms, php_dablooms.c src/dablooms.c src/murmur.c, $ext_shared)

    PHP_ADD_INCLUDE([src])
    PHP_ADD_BUILD_DIR([src])

    CFLAGS=" $CFLAGS -Wunused-variable -Wpointer-sign -Wimplicit-function-declaration -Winline -Wunused-macros -Wredundant-decls -Wstrict-aliasing=2 -Wswitch-enum -Wdeclaration-after-statement"

    PHP_SUBST(DABLOOMS_SHARED_LIBADD)
    PHP_SUBST([CFLAGS])
fi
