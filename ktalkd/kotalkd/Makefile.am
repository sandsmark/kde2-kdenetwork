# kotalkd - Makefile.am
# $Id: Makefile.am 59761 2000-08-05 04:16:40Z garbanzo $

bin_PROGRAMS	=	kotalkd
INCLUDES	=	$(all_includes)
noinst_HEADERS	=	print.h process.h

kotalkd_SOURCES	=	kotalkd.c print.c process.c vsnprintf.c
LDADD           =       $(LIBBSD) $(LIBSOCKET)

#for extra warnings during compilation :
#AM_CXXFLAGS =	-ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE

messages:
	$(XGETTEXT) -o $(podir)/ktalkd.pot ../*/*.cpp

