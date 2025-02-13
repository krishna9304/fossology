# SPDX-FileCopyrightText: © 2014-2015 Siemens AG

# SPDX-License-Identifier: FSFAP

TOP = ../..
VARS = $(TOP)/Makefile.conf
include $(VARS)

MOD_NAME = ninka

DIRS = agent ui
TESTDIR = agent_tests

DIR_LOOP = @set -e; for dir in $(DIRS); do $(MAKE) -s -C $$dir $(1); done

all: VERSIONFILE ninka-all
	$(call DIR_LOOP, )

test: all
	$(TOP)/install/scripts/install-ninka.sh
	$(MAKE) -C $(TESTDIR) test

coverage: all
	$(MAKE) -C $(TESTDIR) coverage

VERSIONFILE:
	$(call WriteVERSIONFile,$(MOD_NAME))

install: all
	$(call DIR_LOOP,install)
	$(INSTALL_DATA) VERSION $(DESTDIR)$(MODDIR)/$(MOD_NAME)/VERSION
	$(INSTALL_DATA) $(MOD_NAME).conf $(DESTDIR)$(MODDIR)/$(MOD_NAME)/$(MOD_NAME).conf
	mkdir -p $(DESTDIR)$(SYSCONFDIR)/mods-enabled
	if test ! -e $(DESTDIR)$(SYSCONFDIR)/mods-enabled/$(MOD_NAME); then \
		ln -s $(MODDIR)/$(MOD_NAME) $(DESTDIR)$(SYSCONFDIR)/mods-enabled; \
	fi

uninstall:
	$(call DIR_LOOP,uninstall)
	rm -rf $(DESTDIR)$(MODDIR)/$(MOD_NAME)
	rm -f $(DESTDIR)$(SYSCONFDIR)/mods-enabled/$(MOD_NAME)

clean:
	$(call DIR_LOOP,clean)
	rm -f VERSION

.PHONY: all test coverage VERSIONFILE install uninstall clean
.PHONY: ninka-all ninka-install ninka-uninstall ninka-clean ninka-Makefile
