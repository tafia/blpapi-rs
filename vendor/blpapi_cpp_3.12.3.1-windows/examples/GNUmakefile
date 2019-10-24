# Makefile for examples
UNAME := $(shell uname)

ifdef IS_64BIT
BITS=64
else
BITS=32
endif

BUILD_DIR:=$(UNAME)

ifeq ($(UNAME), SunOS)
  ifeq ($(shell uname -p), i386)
    CXXFLAGS+=-m$(BITS) -D_POSIX_PTHREAD_SEMANTICS -g
  else
    CXX=CC
    CXXFLAGS+=-m$(BITS) -mt -D_POSIX_PTHREAD_SEMANTICS -g0
  endif
endif

ifeq ($(UNAME), Linux)
CXXFLAGS+=-m$(BITS) -D_POSIX_PTHREAD_SEMANTICS -g
endif

EXAMPLES = $(basename $(wildcard *.cpp))

all: $(EXAMPLES)

clean:
	cd ../$(BUILD_DIR); $(RM) $(addsuffix _??,$(EXAMPLES))

define EXAMPLE_rule
.PHONY: $(1)
$(1): ../$(BUILD_DIR)/$(1)_$(BITS)
../$(BUILD_DIR)/$(1)_$(BITS): $(1).cpp
	$(LINK.cc) -I../include $(1).cpp -o ../$(BUILD_DIR)/$(1)_$(BITS) \
		-L../$(BUILD_DIR) -lblpapi3_$(BITS)
endef
$(foreach example,$(EXAMPLES),$(eval $(call EXAMPLE_rule,$(example))))
