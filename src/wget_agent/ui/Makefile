# SPDX-FileCopyrightText: © 2012 Hewlett-Packard Development Company, L.P.

# SPDX-License-Identifier: GPL-2.0-only

TOP = ../../..
VARS = $(TOP)/Makefile.conf
include $(VARS)

MOD_NAME = wget_agent
MOD_SUBDIR = ui

all:
	@echo "nothing to do"

install:
	mkdir -p $(DESTDIR)$(MODDIR)/$(MOD_NAME)/$(MOD_SUBDIR)
	$(INSTALL_DATA) ./*.php $(DESTDIR)$(MODDIR)/$(MOD_NAME)/$(MOD_SUBDIR)

uninstall:
	rm -rf $(DESTDIR)$(MODDIR)/$(MOD_NAME)/$(MOD_SUBDIR)

clean:
	@echo "nothing to do"

.PHONY: all install uninstall clean
