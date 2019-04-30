# Target library
lib := libuthread.a
objs := \
	queue.o \
	uthread.o \
	context.o \
	preempt.o

# Don't print the commands unless explicitely requested with `make V=1`
ifneq ($(V),1)
Q = @
V = 0
endif

# Default rule
all: $(lib)

# Current directory
CUR_PWD := $(shell pwd)

# GCC flags
CC := gcc
ARC := ar rcs
CFLAGS := -Wall -Werror

# Debug flag for now
CFLAGS += -g

# Generate dependencies
DEPFLAGS = -MMD -MF $(@:.o=.d)

# Include dependencies
deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

# Rule for libuthread.a
$(lib): $(objs)
	@echo "MAKE     $@"
	$(Q)$(ARC) $@ $^

# Generic rule for compiling objects
%.o: %.c
	@echo "CC       $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)	

clean:
	@echo "CLEAN    $(CUR_PWD)"
	$(Q)rm -f $(lib) $(objs) $(deps)

.PHONY: clean $(lib)

