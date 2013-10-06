BINARIES:= unpackbootimg mkbootimg

CC=gcc
CFLAGS= -g -Werror

all: $(BINARIES)

unpackbootimg: bootimg.h unpackbootimg.c
	$(CC) $(CFLAGS) unpackbootimg.c -o $@

mkbootimg: bootimg.h mkbootimg.c sha.c sha.h
	$(CC) $(CFLAGS) mkbootimg.c sha.c -o $@

clean:
	$(RM) $(BINARIES)

install: $(BINARIES)
	@for bin in "$(BINARIES)"; do \
	sudo cp $$bin /usr/local/bin/; \
	done

uninstall:
	@for bin in "$(BINARIES)"; do \
	sudo rm /usr/local/bin/$$bin; \
	done

.PHONY: all clean install uninstall
