# Some third party software needs makedepend which is in /usr/X11/bin,
# but /usr/X11/bin is not alway sin the path for non-X terminals.
ifeq ($(filter /usr/X11/bin,$(PATH)), )
PATH:=$(PATH):/usr/X11/bin
export PATH
endif
