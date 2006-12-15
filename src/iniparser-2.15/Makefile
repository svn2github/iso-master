#
# iniparser Makefile
#

# Compiler settings
# CFLAGS  = -O3 -pedantic -std=c99

# Ar settings to build the library
ARFLAGS = rcv

# Set RANLIB to ranlib on systems that require it (Sun OS < 4, Mac OSX)
# RANLIB  = ranlib
RANLIB = true

# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
.c.o:
	@(echo "compiling $< ...")
	@($(COMPILE.c) -o $@ $<)


SRCS = src/iniparser.c \
	   src/dictionary.c \
	   src/strlib.c

OBJS = $(SRCS:.c=.o)


default:	libiniparser.a

libiniparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) libiniparser.a $(OBJS))
	@($(RANLIB) libiniparser.a)

clean:
	$(RM) $(OBJS) libiniparser.a

veryclean:
	$(RM) $(OBJS) libiniparser.a
	rm -rf ./html ; mkdir html
	cd test ; $(MAKE) veryclean

docs:
	@(cd doc ; $(MAKE))
	
check:
	@(cd test ; $(MAKE))
