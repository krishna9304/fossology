# SPDX-FileCopyrightText: © 2015 Siemens AG

# SPDX-License-Identifier: FSFAP
TOP = ../../..
VARS = $(TOP)/Makefile.conf
include $(VARS)

DIRS = Functional Unit
LOCALAGENTDIR = ../agent/

DIR_LOOP = @set -e; for dir in $(DIRS); do $(MAKE) -s -C $$dir $(1); done

all:
	$(call DIR_LOOP, )

test:
	$(call DIR_LOOP,test)

coverage:
	$(call DIR_LOOP,coverage)

clean:
	$(call DIR_LOOP,clean)

.PHONY: all test coverage clean
