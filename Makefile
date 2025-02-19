.PHONY: all install clean distclean

LIB_FATALSIG  = libfatalsig.so

CPPFLAGS     ?= -DNDEBUG

CFLAGS		 ?= -Wall \
                -pthread \
                -fPIC \
                -shared \
                -fasynchronous-unwind-tables \
                -fno-omit-frame-pointer \
			    -fcaller-saves

LDFLAGS		 ?= -lpthread -lunwind

all: $(LIB_FATALSIG)

$(LIB_FATALSIG): fatalsig.c fatalsig.h Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) -o $@

install: $(LIB_FATALSIG)
	install $(LIB_FATALSIG) /usr/lib

clean:
	rm -fv *.o *~ $(LIB_FATALSIG)

distclean: clean
