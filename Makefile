PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
DESTDIR =

LDFLAGS = -g
LDLIBS = -lm
CFLAGS = -g -Wall

INSTALL = install
CC = gcc
AR = ar

SRCDIR = src
BLDDIR = build

SRCS_LIBDABLOOMS = md5.c dablooms.c
SRCS_TESTS = test_dablooms.c

OBJS_LIBDABLOOMS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_LIBDABLOOMS))
OBJS_TESTS = $(patsubst %.c, $(BLDDIR)/%.o, $(SRCS_TESTS))

all: test_dablooms libdablooms

DEPS := $(patsubst %.o, %.o.deps, $(OBJS_LIBDABLOOMS) $(OBJS_TESTS)) 
# sort removes duplicates
-include $(sort $(DEPS))

install: install_libdablooms

$(DESTDIR)$(LIBDIR)/libdablooms.a: $(BLDDIR)/libdablooms.a
	@echo " INSTALL " $@
	@$(INSTALL) -d $(dir $@)
	@$(INSTALL) -D $< $@

install_libdablooms: $(DESTDIR)$(LIBDIR)/libdablooms.a
	
libdablooms: $(BLDDIR)/libdablooms.a
	
$(BLDDIR)/test_dablooms: $(OBJS_TESTS) libdablooms
	@echo " LD " $@
	@$(CC) -o $@ $(OBJS_TESTS) -L$(BLDDIR) $(LDFLAGS) -l dablooms $(LDLIBS)

test: $(BLDDIR)/test_dablooms
	$(BLDDIR)/test_dablooms

$(BLDDIR)/%.o: $(SRCDIR)/%.c
	@echo " CC " $@
	@mkdir -p $(dir $@)
	@$(CC) -o $@ -c $< $(CFLAGS) -MMD -MF $@.deps

$(BLDDIR)/libdablooms.a: $(OBJS_LIBDABLOOMS)
	@echo " AR " $@
	@rm -f $@
	@$(AR) rcs $@ $^

clean:
	rm -f $(OBJS_LIBDABLOOMS) $(BLDDIR)/libdablooms.a $(OBJS_TESTS) $(BLDIR)/test_dablooms

.PHONY: all clean install install_libdablooms libdablooms test
