INCLUDES= -I$(top_srcdir)/knewsticker/common $(all_includes)

lib_LTLIBRARIES = libkcm_newsticker.la

libkcm_newsticker_la_SOURCES = newssourcedlg.ui kcmnewstickerwidget.ui \
	newssourcedlgimpl.cpp kcmnewsticker.cpp

libkcm_newsticker_la_LDFLAGS = -L$(top_builddir)/knewsticker/common \
	$(all_libraries) -module -avoid-version -no-undefined
libkcm_newsticker_la_LIBADD = $(LIB_KDEUI) $(LIB_KFILE) \
  $(top_builddir)/knewsticker/common/libknewstickercommon.la

METASOURCES = AUTO

noinst_HEADERS = newssourcedlg.h newssourcedlgimpl.h \
	kcmnewstickerwidget.h kcmnewsticker.h

KDE_ICON = kcmnewsticker

messages: rc.cpp
	$(XGETTEXT) *.cpp ../common/newsengine.cpp -o $(podir)/kcmnewsticker.pot

data_DATA = kcmnewsticker.desktop
datadir = $(kde_appsdir)/Settings/Network

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(kde_appsdir)/Settings/Personalization
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(kde_appsdir)/Settings/Personalization/kcmnewsticker.desktop
