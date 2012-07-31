PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include
DESTDIR =
SRCDIR = src
BLDDIR = build

LDFLAGS = -g 
LDLIBS = -lm
CFLAGS = -g -Wall -fPIC

PYTHON = python
INSTALL = install
CC = gcc
AR = ar

PY_SRCDIR = pydablooms
PY_MOD_DIR := $(shell $(PYTHON) -c "import distutils.sysconfig ; print distutils.sysconfig.get_python_lib()")
PY_FLAGS = --build-lib=$(PY_BLDDIR) --build-temp=$(PY_BLDDIR)
PY_BLDDIR = $(BLDDIR)/python

SRCS_LIBDABLOOMS = md5.c dablooms.c
SRCS_TESTS = test_dablooms.c
WORDS =
OBJS_LIBDABLOOMS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_LIBDABLOOMS))
OBJS_TESTS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_TESTS))

all:  libdablooms pydablooms $(BLDDIR)/test_dablooms

# sort removes duplicates
DEPS := $(sort $(patsubst %.o, %.o.deps, $(OBJS_LIBDABLOOMS) $(OBJS_TESTS))) 
-include $(DEPS)

install: install_libdablooms install_pydablooms

install_libdablooms: $(DESTDIR)$(LIBDIR)/libdablooms.a $(DESTDIR)$(INCDIR)/dablooms.h

install_pydablooms: $(DESTDIR)$(PY_MOD_DIR)/pydablooms.so

libdablooms: $(BLDDIR)/libdablooms.a

pydablooms: libdablooms $(PY_BLDDIR)/pydablooms.so

$(DESTDIR)$(LIBDIR)/libdablooms.a: $(BLDDIR)/libdablooms.a

$(DESTDIR)$(INCDIR)/dablooms.h: $(SRCDIR)/dablooms.h

$(DESTDIR)$(PY_MOD_DIR)/pydablooms.so: $(PY_BLDDIR)/pydablooms.so
	@echo " PY_INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) -C $< $@

$(DESTDIR)$(PREFIX)/%:
	@echo " INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) -C $< $@

$(BLDDIR)/test_dablooms: $(OBJS_TESTS) $(BLDDIR)/libdablooms.a 
	@echo " LD " $@
	@$(CC) -o $@ $(OBJS_TESTS) -L$(BLDDIR) $(LDFLAGS) -l dablooms $(LDLIBS)

test: $(BLDDIR)/test_dablooms
	$(BLDDIR)/test_dablooms $(WORDS)

$(PY_BLDDIR)/pydablooms.so: $(BLDDIR)/libdablooms.a
	@echo " PY_BUILD" $@
	@$(PYTHON) $(PY_SRCDIR)/setup.py build $(PY_FLAGS)

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	@echo " CC " $@
	@mkdir -p $(dir $@)
	@$(CC) -o $@ -c $< $(CFLAGS) -MMD -MF $@.deps

$(BLDDIR)/libdablooms.a: $(OBJS_LIBDABLOOMS)
	@echo " AR " $@
	@rm -f $@
	@$(AR) rcs $@ $^

clean:		
	$(PYTHON) $(PY_SRCDIR)/setup.py clean $(PY_FLAGS)
	rm -f $(OBJS_LIBDABLOOMS) $(BLDDIR)/libdablooms.a $(OBJS_TESTS) $(BLDDIR)/test_dablooms $(DEPS) $(BLDDIR)/pydablooms.so
	rmdir $(BLDDIR)

.PHONY: all clean install test install_libdablooms install_pydablooms libdablooms pydablooms
