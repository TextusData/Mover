# /Makefile 
# Copyright (c) 2009-2010, 2013, 2014 Ross Biro
# 
# Toplevel Makefile to build everything
#
#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, version 3 of the
#   License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see
#   <http://www.gnu.org/licenses/>.
#
#

$(foreach mfile,$(sort $(wildcard make_parts/begin/*.inc)),$(eval -include $(mfile)))

define make_one

$(MAKEROOT)/$(1:.mod=.make) : $1 ./textus/scripts/dependencies.pl

$(1:.mod=$2):$(MAKEROOT)/$(1:.mod=.make)  
	echo $(1:.mod=)$2 >.last_target
	make -f $$< $(patsubst .%,%,$2)


ifdef $2
.PHONY: $(1:.mod=$2)
endif

endef

.PHONY: all tests last runtests clean alldeps thirdparty valgrind

MOD_FILES=$(shell find textus -name "*.mod" -print)
ALL_FILES=$(shell find textus -name "*" -print)
LAST=$(shell cat .last_target 2>/dev/null)

last: $(LAST)

all: $(foreach modfile,$(MOD_FILES),$(modfile:.mod=))
	rm .last_target

alldeps: $(foreach modfile, $(MOD_FILES), $(modfile:.mod=.deps))
	rm .last_target

tests: $(foreach modfile, $(MOD_FILES), $(modfile:.mod=.tests))
	rm .last_target

runtests: $(foreach modfile, $(MOD_FILES), $(modfile:.mod=.runtests))
	rm .last_target

valgrind: $(foreach modfile, $(MOD_FILES), $(modfile:.mod=.valgrind))
	rm .last_target

docs: thirdparty $(ALL_FILES)
	cd textus && $(DOXYGEN)

thirdparty:
	cd thirdparty && make all

clean:
	rm -rf "$(MAKEROOT)" "$(OBJROOT)" "$(BINROOT)" # "$(DOCROOT)"
	rm .last_target

$(foreach modfile, $(MOD_FILES), $(eval $(call make_one,$(modfile))))
$(foreach modfile, $(MOD_FILES), $(eval $(call make_one,$(modfile),.tests)))
$(foreach modfile, $(MOD_FILES), $(eval $(call make_one,$(modfile),.runtests)))
$(foreach modfile, $(MOD_FILES), $(eval $(call make_one,$(modfile),.deps)))
$(foreach modfile, $(MOD_FILES), $(eval $(call make_one,$(modfile),.valgrind)))
