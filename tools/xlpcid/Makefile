#
# tools/xl/Makefile
#

XEN_ROOT = $(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

CFLAGS += -Wno-format-zero-length -Wmissing-declarations \
	-Wno-declaration-after-statement -Wformat-nonliteral
CFLAGS += -fPIC

CFLAGS += $(PTHREAD_CFLAGS)
LDFLAGS += $(PTHREAD_LDFLAGS)

CFLAGS_XLPCID += $(CFLAGS_libxenlight)
CFLAGS_XLPCID += $(CFLAGS_libxenutil)
CFLAGS_XLPCID += -Wshadow
CFLAGS_XLPCID += $(CFLAGS_libxenvchan)

#XL_OBJS-$(CONFIG_X86) = xl_psr.o
XLPCID_OBJS = xlpcid.o $(XLPCID_OBJS-y)

$(XLPCID_OBJS): CFLAGS += $(CFLAGS_libxentoollog)
$(XLPCID_OBJS): CFLAGS += $(CFLAGS_XLPCID)
$(XLPCID_OBJS): CFLAGS += -include $(XEN_ROOT)/tools/config.h # libxl_json.h needs it.

.PHONY: all
all: xlpcid

xlpcid: $(XLPCID_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(XLPCID_OBJS) $(LDLIBS_libxenutil) $(LDLIBS_libxenlight) $(LDLIBS_libxentoollog) $(LDLIBS_libxenvchan) -lyajl $(APPEND_LDFLAGS)

.PHONY: install
install: all
	$(INSTALL_DIR) $(DESTDIR)$(sbindir)
	$(INSTALL_DIR) $(DESTDIR)$(BASH_COMPLETION_DIR)
	$(INSTALL_PROG) xlpcid $(DESTDIR)$(sbindir)
	$(INSTALL_DATA) bash-completion $(DESTDIR)$(BASH_COMPLETION_DIR)/xlpcid

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(BASH_COMPLETION_DIR)/xlpcid
	rm -f $(DESTDIR)$(sbindir)/xlpcid

.PHONY: clean
clean:
	$(RM) *.o xlpcid $(DEPS_RM)

distclean: clean


-include $(DEPS_INCLUDE)
