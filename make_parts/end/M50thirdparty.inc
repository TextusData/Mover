#
# M50thirdparty.inc -*- makefile -*-
#

.PHONY: thirdparty thirdparty_clean

thirdparty:
	cd thirdparty && make all

clean:: thirdparty_clean

thirdparty_clean:
	rm -rf $(BINROOT)/thirdparty
	rm -rf $(OBJROOT)/thirdparty

