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

SRCS_LIBDABLOOMS = md5.c dablooms.c
SRCS_TESTS = test_dablooms.c
WORDS = /usr/share/dict/words
OBJS_LIBDABLOOMS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_LIBDABLOOMS))
OBJS_TESTS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_TESTS))

# default target (needs to be first target)
all: libdablooms

# sort removes duplicates
DEPS := $(sort $(patsubst %.o, %.o.deps, $(OBJS_LIBDABLOOMS) $(OBJS_TESTS)))
-include $(DEPS)

install: install_libdablooms

libdablooms: $(BLDDIR)/libdablooms.a

install_libdablooms: $(DESTDIR)$(LIBDIR)/libdablooms.a $(DESTDIR)$(INCDIR)/dablooms.h

$(DESTDIR)$(LIBDIR)/libdablooms.a: $(BLDDIR)/libdablooms.a

$(DESTDIR)$(INCDIR)/dablooms.h: $(SRCDIR)/dablooms.h

$(DESTDIR)$(PREFIX)/%:
	@echo " INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) -C $< $@

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	@echo " CC " $@
	@mkdir -p $(dir $@)
	@$(CC) -o $@ -c $< $(CFLAGS) -MMD -MF $@.deps

$(BLDDIR)/libdablooms.a: $(OBJS_LIBDABLOOMS)
	@echo " AR " $@
	@rm -f $@
	@$(AR) rcs $@ $^

$(BLDDIR)/test_dablooms: $(OBJS_TESTS) $(BLDDIR)/libdablooms.a
	@echo " LD " $@
	@$(CC) -o $@ $(OBJS_TESTS) -L$(BLDDIR) $(LDFLAGS) -l dablooms $(LDLIBS)

test: $(BLDDIR)/test_dablooms
	@$(BLDDIR)/test_dablooms $(WORDS)

help:
	@printf $(HELPTEXT)

clean:
	rm -f $(OBJS_LIBDABLOOMS) $(BLDDIR)/libdablooms.a $(OBJS_TESTS) $(BLDDIR)/test_dablooms $(DEPS)
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
	@$(INSTALL) -C $< $@

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
