# SPDX-FileCopyrightText: © 2021 Orange
# Author: Bartłomiej Dróżdż <bartlomiej.drozdz@orange.com>

# SPDX-License-Identifier: GPL-2.0-only

TOP = ../..
VARS = $(TOP)/Makefile.conf
include $(VARS)

MOD_NAME = reso

DIRS = agent ui

DIR_LOOP = @set -e; for dir in $(DIRS); do $(MAKE) -s -C $$dir $(1); done

all: VERSIONFILE
	$(call DIR_LOOP, )

coverage: all
	@echo "nothing to do"

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
	rm -f VERSION
	$(call DIR_LOOP,clean)

.PHONY: all test coverage VERSIONFILE install uninstall clean
