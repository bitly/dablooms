HELPTEXT = "\
\n dablooms Makefile usage       \
\n                               \
\n  Options (current value)      \
\n                               \
\n    BLDDIR     ($(BLDDIR))     \
\n    DESTDIR    ($(DESTDIR))    \
\n    prefix     ($(prefix))     \
\n    libdir     ($(libdir))     \
\n    includedir ($(includedir)) \
\n                               \
\n    CC         ($(CC))         \
\n    CFLAGS     ($(ALL_CFLAGS)) \
\n    LDFLAGS    ($(ALL_LDFLAGS))\
\n    INSTALL    ($(INSTALL))    \
\n                               \
\n    PYTHON     ($(PYTHON))     \
\n    PY_MOD_DIR ($(PY_MOD_DIR)) \
\n                               \
\n  Targets                      \
\n                               \
\n    all        (c libdablooms) \
\n    install                    \
\n    test                       \
\n    clean                      \
\n    help                       \
\n                               \
\n    pydablooms                 \
\n    install_pydablooms         \
\n    test_pydablooms            \
\n\n"

prefix = /usr/local
libdir = $(prefix)/lib
includedir = $(prefix)/include
DESTDIR =
BLDDIR = build

CFLAGS = -g -Wall -O2
LDFLAGS =
ALL_CFLAGS = -fPIC $(CFLAGS)
ALL_LDFLAGS = -lm $(LDFLAGS)

INSTALL = install
CC = gcc
AR = ar

### dynamic shared object ###

# shared-object version does not follow software release version
SO_VER_MAJOR = 1
SO_VER_MINOR = 1

SO_VER = $(SO_VER_MAJOR).$(SO_VER_MINOR)
SO_NAME = so
SO_CMD = -soname
SO_EXT_MAJOR = $(SO_NAME).$(SO_VER_MAJOR)
SO_EXT = $(SO_NAME).$(SO_VER)
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	SO_NAME = dylib
	SO_CMD = -install_name
	SO_EXT_MAJOR = $(SO_VER_MAJOR).$(SO_NAME)
	SO_EXT = $(SO_VER).$(SO_NAME)
endif
SHARED_LDFLAGS = -shared -Wl,$(SO_CMD),libdablooms.$(SO_EXT_MAJOR)

### sources and outputs ###

SRCS_LIBDABLOOMS = dablooms.c murmur.c
SRCS_TESTS = test_dablooms.c

OBJS_LIBDABLOOMS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_LIBDABLOOMS))
OBJS_TESTS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_TESTS))

LIB_SYMLNKS = libdablooms.$(SO_NAME) libdablooms.$(SO_EXT_MAJOR)
LIB_FILES = libdablooms.a libdablooms.$(SO_EXT) $(LIB_SYMLNKS)

# for tests
WORDS = /usr/share/dict/words

### rules ###

# default target (needs to be first target)
all: libdablooms

# sort removes duplicates
DEPS := $(sort $(patsubst %.o, %.o.deps, $(OBJS_LIBDABLOOMS) $(OBJS_TESTS)))
-include $(DEPS)

libdablooms: $(patsubst %, $(BLDDIR)/%, $(LIB_FILES))

install: install_libdablooms

install_libdablooms: $(patsubst %, $(DESTDIR)$(libdir)/%, $(LIB_FILES)) $(DESTDIR)$(includedir)/dablooms.h

$(DESTDIR)$(libdir)/libdablooms.a: $(BLDDIR)/libdablooms.a

$(DESTDIR)$(libdir)/libdablooms.$(SO_EXT): $(BLDDIR)/libdablooms.$(SO_EXT)

$(patsubst %, $(DESTDIR)$(libdir)/%, $(LIB_SYMLNKS)): %: $(DESTDIR)$(libdir)/libdablooms.$(SO_EXT)
	@echo " SYMLNK " $@
	@$(INSTALL) -d $(dir $@)
	@ln -fs $(notdir $<) $@

$(DESTDIR)$(includedir)/dablooms.h: src/dablooms.h

$(DESTDIR)$(prefix)/%:
	@echo " INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) $< $@

$(BLDDIR)/%.o: src/%.c
	@echo " CC " $@
	@mkdir -p $(dir $@)
	@$(CC) -o $@ -c $< $(ALL_CFLAGS) -MMD -MF $@.deps

$(BLDDIR)/libdablooms.a: $(OBJS_LIBDABLOOMS)
	@echo " AR " $@
	@rm -f $@
	@$(AR) rcs $@ $^

$(BLDDIR)/libdablooms.$(SO_EXT): $(OBJS_LIBDABLOOMS)
	@echo " SO " $@
	@$(CC) -o $@ $(ALL_CFLAGS) $(SHARED_LDFLAGS) $(ALL_LDFLAGS) $^

$(patsubst %, $(BLDDIR)/%, $(LIB_SYMLNKS)): %: $(BLDDIR)/libdablooms.$(SO_EXT)
	@echo " SYMLNK " $@
	@mkdir -p $(dir $@)
	@ln -fs $(notdir $<) $@

$(BLDDIR)/test_dablooms: $(OBJS_TESTS) $(BLDDIR)/libdablooms.a
	@echo " LD " $@
	@$(CC) -o $@ $(ALL_CFLAGS) $(ALL_LDFLAGS) $(OBJS_TESTS) $(BLDDIR)/libdablooms.a

test: $(BLDDIR)/test_dablooms
	@$(BLDDIR)/test_dablooms $(BLDDIR)/testbloom.bin $(WORDS)

help:
	@printf $(HELPTEXT)

clean:
	rm -f $(DEPS) $(OBJS_LIBDABLOOMS) $(patsubst %, $(BLDDIR)/%, $(LIB_FILES)) $(OBJS_TESTS) $(BLDDIR)/test_dablooms $(BLDDIR)/testbloom.bin
	rmdir $(BLDDIR)

.PHONY: all clean help install test libdablooms install_libdablooms

### pydablooms ###

PYTHON = python
PY_BLDDIR = $(BLDDIR)/python
PY_MOD_DIR_ARG = # optional: --user or --system
PY_MOD_DIR := $(shell $(PYTHON) pydablooms/modpath.py $(PY_MOD_DIR_ARG))
PY_FLAGS = --build-lib=$(PY_BLDDIR) --build-temp=$(PY_BLDDIR)
PY_BLD_ENV = BLDDIR="$(BLDDIR)"

pydablooms: $(PY_BLDDIR)/pydablooms.so

install_pydablooms: $(DESTDIR)$(PY_MOD_DIR)/pydablooms.so

$(DESTDIR)$(PY_MOD_DIR)/pydablooms.so: $(PY_BLDDIR)/pydablooms.so
	@echo " PY_INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) $< $@

$(PY_BLDDIR)/pydablooms.so: pydablooms/pydablooms.c src/dablooms.c src/murmur.c
	@echo " PY_BUILD" $@
	@$(PY_BLD_ENV) $(PYTHON) pydablooms/setup.py build $(PY_FLAGS) >/dev/null

test_pydablooms: pydablooms
	@PYTHONPATH=$(PY_BLDDIR) $(PYTHON) pydablooms/test_pydablooms.py $(BLDDIR)/testbloom_py.bin $(WORDS)

clean: clean_pydablooms
clean_pydablooms:
	rm -f $(BLDDIR)/pydablooms.so $(BLDDIR)/testbloom_py.bin
	$(PYTHON) pydablooms/setup.py clean $(PY_FLAGS)

.PHONY: pydablooms install_pydablooms test_pydablooms clean_pydablooms
