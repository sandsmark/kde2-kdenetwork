INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdictapplet.la

libkdictapplet_la_SOURCES = kdictapplet.cpp

METASOURCES = AUTO
noinst_HEADERS = kdictapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = kdictapplet.desktop

EXTRA_DIST = $(lnk_DATA)

libkdictapplet_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libkdictapplet_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdictapplet.pot
