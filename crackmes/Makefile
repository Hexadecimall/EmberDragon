# build:  make            (all crackmes, -O2)
# strip:  make strip       (strip symbols for a more realistic target)
# clean:  make clean
CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wno-format-security
SRCS    := $(wildcard crackme*.c)
BINS    := $(SRCS:.c=)

all: $(BINS)

crackme%: crackme%.c
	$(CC) $(CFLAGS) -o $@ $<

strip: all
	strip $(BINS)

clean:
	rm -f $(BINS)

.PHONY: all strip clean
