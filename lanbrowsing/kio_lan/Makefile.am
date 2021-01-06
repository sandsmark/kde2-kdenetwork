## Makefile.am of kdebase/kioslave/man
SUBDIRS = .

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)

####### Files

lib_LTLIBRARIES = kio_lan.la

kio_lan_la_SOURCES = kio_lan.cpp
kio_lan_la_LIBADD = -lkio
kio_lan_la_LDFLAGS = -module -avoid-version -no-undefined
noinst_HEADERS = kio_lan.h

kdelnk_DATA = lan.protocol rlan.protocol
kdelnkdir = $(kde_servicesdir)

data_DATA = lan.desktop
datadir = $(kde_datadir)/konqueror/dirtree/remote

METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kio_lan.pot

