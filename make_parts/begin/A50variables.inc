ABSSRCROOT=$(shell pwd)
# the protocompiler doesn't currently seem to like absolute paths under some circumstances
SRCROOT=.
OBJROOT=$(ABSSRCROOT)/obj
BINROOT=$(ABSSRCROOT)/bin
MAKEROOT=$(ABSSRCROOT)/make
export OBJROOT BINROOT ABSSRCROOT MAKEROOT

