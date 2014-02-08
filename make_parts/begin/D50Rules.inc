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
define make-depend
  $(CC)	 -MM            \
         -MF $3         \
         -MP            \
         -MT $2         \
         $(CFLAGS)      \
         $(CPPFLAGS)    \
         $(TARGET_ARCH) \
         $1
endef

define run_tests
       $(if $(filter-out %.deps dependencies, $1), echo running $1 && export LD_LIBRARY_PATH=$(LIB_PATHS) && textus/scripts/runtest $(if $(timeallowed_$1), --timeallowed=$(timeallowed_$1)) -- "$1",)
endef

define run_all_tests
	$(foreach test,$1,$(call run_tests,$(test)))
endef

define valgrind_tests
       $(if $(filter-out %.deps dependencies, $1), echo running $1 && export LD_LIBRARY_PATH=$(LIB_PATHS) && export GLIBCXX_FORCE_NEW=1 && export GLIBCPP_FORCE_NEW=1 && textus/scripts/runtest $(if $(timeallowed_$1), --timeallowed=$(timeallowed_$1)) -- $(VALGRIND) "$1",)
endef

define valgrind_all_tests
	$(foreach test,$1,$(call valgrind_tests,$(test)))
endef


$(OBJROOT)/%.d: %.c
	mkdir -p $(dir $@)
	$(call make-depend,$<,$(subst .d,.o,$@),$@)

$(OBJROOT)/%.d: %.cc
	mkdir -p $(dir $@)
	$(call make-depend,$<,$(subst .d,.o,$@),$@)

$(OBJROOT)/%.o: %.cc
	mkdir -p $(dir $@)
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) -c $(CCFLAGS) -o $@ $<

$(OBJROOT)/%.o: %.c
	mkdir -p $(dir $@)
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJROOT)/%.d: $(OBJROOT)/%.c
	mkdir -p $(dir $@)
	$(call make-depend,$<,$(subst .d,.o,$@),$@)

$(OBJROOT)/%.d: $(OBJROOT)/%.cc
	mkdir -p $(dir $@)
	$(call make-depend,$<,$(subst .d,.o,$@),$@)

$(OBJROOT)/%.o: $(OBJROOT)/%.cc
	mkdir -p $(dir $@)
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) -c $(CCFLAGS) -o $@ $<

$(OBJROOT)/%.o: $(OBJROOT)/%.c
	mkdir -p $(dir $@)
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJROOT)/%.pb.h %(OBJROOT)/%.pb.cc: %.proto
	mkdir -p $(dir $@)
	$(CCPROTOCOMPILERLIBS) $(CCPROTOCOMPILER) --proto_path=$(SRCROOT)/ --cpp_out=$(OBJROOT) $<

$(MAKEROOT)/%.inc $(MAKEROOT)/%.make: %.mod
	echo new file: $?
	mkdir -p $(dir $@)
	./textus/scripts/dependencies.pl $<

$(MAKEROOT)/%.inc $(MAKEROOT)/%.make: %.mod2
	echo new file: $?
	mkdir -p $(dir $@)
	./textus/scripts/dependencies.pl $<
