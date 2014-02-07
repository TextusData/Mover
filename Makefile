# /Makefile 
# Copyright (c) 2009-2010, 2013 Ross Biro
# 
# Toplevel Makefile to build everything
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
