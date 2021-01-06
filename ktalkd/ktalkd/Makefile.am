## -*- makefile -*-
# Ktalkd - Makefile.am

SUBDIRS 	=	machines

bin_PROGRAMS	=	ktalkd
EXTRA_HEADERS	=	readcfg++.h
INCLUDES	=	$(all_includes)
noinst_HEADERS	=	defs.h includ.h find_user.h announce.h print.h process.h readconf.h table.h threads.h options.h unixsock.h
ktalkd_DEPS	=	machines/libmach.a

#for extra warnings during compilation :
#KDE_CXXFLAGS =	-ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE

###########################################################

# Config file location
TALKD_CONF	=	$(kde_confdir)/ktalkdrc
TALKD_CONF_NAME	=	ktalkdrc
AM_CPPFLAGS 	=      -DHAVE_KDE
ktalkd_SOURCES	=	announce.cpp print.cpp process.cpp readcfg++.cpp table.cpp talkd.cpp find_user.cpp threads.cpp options.cpp unixsock.cpp
LDADD           =       machines/libmach.a $(LIBBSD) $(LIB_KDECORE) $(LIBSOCKET)

###########################################################

LDFLAGS         =       $(all_libraries) $(KDE_RPATH)

EXTRA_DIST	=	.talkdrc talkd.conf ktalkdrc

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
	$(mkinstalldirs) $(DESTDIR)$(kde_sounddir)
	$(INSTALL_DATA) $(srcdir)/ktalkd.wav $(DESTDIR)$(kde_sounddir)
	@echo "**************************************************************************"
	@echo
	@if [ -f $(DESTDIR)$(TALKD_CONF) ]; then \
	  echo "Please check $(TALKD_CONF) to be up-to-date."; \
	else \
	  $(INSTALL_DATA) $(srcdir)/$(TALKD_CONF_NAME) $(DESTDIR)$(TALKD_CONF); \
	fi
