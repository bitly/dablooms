HELPTEXT = "\
\n dablooms Makefile usage       \
\n                               \
\n  Options (current value)      \
\n                               \
\n    BLDDIR     ($(BLDDIR))     \
\n    DESTDIR    ($(DESTDIR))    \
\n    PREFIX     ($(PREFIX))     \
\n    LIBDIR     ($(LIBDIR))     \
\n    INCDIR     ($(INCDIR))     \
\n                               \
\n    CC         ($(CC))         \
\n    CFLAGS     ($(CFLAGS))     \
\n    INCPATH    ($(INCPATH))    \
\n    LDFLAGS    ($(LDFLAGS))    \
\n    LIBPATH    ($(LIBPATH))    \
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

# only bump major on ABI changing release
SO_VER_MAJOR = 1
SO_VER_MINOR = 0

PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include
DESTDIR =
SRCDIR = src
BLDDIR = build

INCPATH =
CFLAGS = -g -Wall -fPIC $(patsubst %, -I%, $(INCPATH))
LIBPATH =
LDFLAGS = -g $(patsubst %, -L%, $(LIBPATH))
LDLIBS = -lm

INSTALL = install
CC = gcc
AR = ar

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

SRCS_LIBDABLOOMS = md5.c dablooms.c
SRCS_TESTS = test_dablooms.c
WORDS = /usr/share/dict/words
OBJS_LIBDABLOOMS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_LIBDABLOOMS))
OBJS_TESTS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_TESTS))

LIB_SYMLNKS = libdablooms.$(SO_NAME) libdablooms.$(SO_EXT_MAJOR)
LIB_FILES = libdablooms.a libdablooms.$(SO_EXT) $(LIB_SYMLNKS)

# default target (needs to be first target)
all: libdablooms

# sort removes duplicates
DEPS := $(sort $(patsubst %.o, %.o.deps, $(OBJS_LIBDABLOOMS) $(OBJS_TESTS)))
-include $(DEPS)

libdablooms: $(patsubst %, $(BLDDIR)/%, $(LIB_FILES))

install: install_libdablooms

install_libdablooms: $(patsubst %, $(DESTDIR)$(LIBDIR)/%, $(LIB_FILES)) $(DESTDIR)$(INCDIR)/dablooms.h

$(DESTDIR)$(LIBDIR)/libdablooms.a: $(BLDDIR)/libdablooms.a

$(DESTDIR)$(LIBDIR)/libdablooms.$(SO_EXT): $(BLDDIR)/libdablooms.$(SO_EXT)

$(patsubst %, $(DESTDIR)$(LIBDIR)/%, $(LIB_SYMLNKS)): %: $(DESTDIR)$(LIBDIR)/libdablooms.$(SO_EXT)
	@echo " SYMLNK " $@
	@$(INSTALL) -d $(dir $@)
	@ln -fs $(notdir $<) $@

$(DESTDIR)$(INCDIR)/dablooms.h: $(SRCDIR)/dablooms.h

$(DESTDIR)$(PREFIX)/%:
	@echo " INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) $< $@

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	@echo " CC " $@
	@mkdir -p $(dir $@)
	@$(CC) -o $@ -c $< $(CFLAGS) -MMD -MF $@.deps

$(BLDDIR)/libdablooms.a: $(OBJS_LIBDABLOOMS)
	@echo " AR " $@
	@rm -f $@
	@$(AR) rcs $@ $^

$(BLDDIR)/libdablooms.$(SO_EXT): $(OBJS_LIBDABLOOMS)
	@echo " SO " $@
	@$(CC) -shared -Wl,$(SO_CMD),libdablooms.$(SO_EXT_MAJOR) -o $@ $^

$(patsubst %, $(BLDDIR)/%, $(LIB_SYMLNKS)): %: $(BLDDIR)/libdablooms.$(SO_EXT)
	@echo " SYMLNK " $@
	@mkdir -p $(dir $@)
	@ln -fs $(notdir $<) $@

$(BLDDIR)/test_dablooms: $(OBJS_TESTS) $(BLDDIR)/libdablooms.a
	@echo " LD " $@
	@$(CC) -o $@ $(OBJS_TESTS) -L$(BLDDIR) $(LDFLAGS) -l dablooms $(LDLIBS)

test: $(BLDDIR)/test_dablooms
	@$(BLDDIR)/test_dablooms $(WORDS)

help:
	@printf $(HELPTEXT)

clean:
	rm -f $(OBJS_LIBDABLOOMS) $(BLDDIR)/libdablooms.a $(BLDDIR)/libdablooms.*$(SO_NAME)* $(OBJS_TESTS) $(BLDDIR)/test_dablooms $(DEPS)
	rmdir $(BLDDIR)

.PHONY: all clean help install test libdablooms install_libdablooms

### pydablooms ###

PYTHON = python
PY_SRCDIR = pydablooms
PY_BLDDIR = $(BLDDIR)/python
PY_MOD_DIR := $(shell $(PYTHON) -c "import distutils.sysconfig ; print(distutils.sysconfig.get_python_lib())")
PY_FLAGS = --build-lib=$(PY_BLDDIR) --build-temp=$(PY_BLDDIR)
PY_BLD_ENV = INCPATH="$(SRCDIR) $(INCPATH)" LIBPATH="$(BLDDIR) $(LIBPATH)"

pydablooms: $(PY_BLDDIR)/pydablooms.so

install_pydablooms: $(DESTDIR)$(PY_MOD_DIR)/pydablooms.so

$(DESTDIR)$(PY_MOD_DIR)/pydablooms.so: $(PY_BLDDIR)/pydablooms.so
	@echo " PY_INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) $< $@

$(PY_BLDDIR)/pydablooms.so: $(BLDDIR)/libdablooms.a $(PY_SRCDIR)/pydablooms.c
	@echo " PY_BUILD" $@
	@$(PY_BLD_ENV) $(PYTHON) $(PY_SRCDIR)/setup.py build $(PY_FLAGS) >/dev/null

test_pydablooms: pydablooms
	@PYTHONPATH=$(PY_BLDDIR) $(PYTHON) $(PY_SRCDIR)/test_pydablooms.py $(WORDS)

clean: clean_pydablooms
clean_pydablooms:
	rm -f $(BLDDIR)/pydablooms.so
	$(PYTHON) $(PY_SRCDIR)/setup.py clean $(PY_FLAGS)

.PHONY: pydablooms install_pydablooms test_pydablooms clean_pydablooms
