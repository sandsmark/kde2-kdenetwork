SUBDIRS = common kcmnewsticker kntsrcfilepropsdlg knewstickerstub
INCLUDES = -I$(top_srcdir)/knewsticker/common $(all_includes)

lib_LTLIBRARIES = libknewstickerapplet.la

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = knewsticker.desktop

noinst_HEADERS = newsscroller.h knewsticker.h

METASOURCES = AUTO

libknewstickerapplet_la_SOURCES = newsscroller.cpp \
	knewsticker.cpp knewsticker.skel
libknewstickerapplet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libknewstickerapplet_la_LIBADD = common/libknewstickercommon.la \
	$(LIB_KIO) $(LIB_KDEUI)

datadir = $(kde_datadir)/knewsticker
data_DATA = eventsrc

updatedir = $(kde_datadir)/kconf_update
update_DATA = knewsticker.upd knt-0.1-0.2.pl

KDE_ICON = AUTO

messages:
	$(XGETTEXT) *.cpp *.h knewstickerstub/*.cpp common/*.h common/*.cpp kntsrcfilepropsdlg/*.cpp -o $(podir)/knewsticker.pot

