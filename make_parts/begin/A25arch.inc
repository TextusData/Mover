ARCH?=$(shell uname -s)
ifeq ($(ARCH),Darwin)
    ARCH=OS-X
endif
export ARCH
