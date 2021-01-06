INCLUDES = -I$(top_srcdir)/knewsticker/common $(all_includes)

bin_PROGRAMS = knewstickerstub

servicedir = $(kde_appsdir)/.hidden
service_DATA = knewstickerstub.desktop

METASOURCES = AUTO

knewstickerstub_SOURCES = knewstickerstub.cpp
knewstickerstub_LDADD = $(LIB_KDECORE) $(LIB_KIO) \
  $(top_builddir)/knewsticker/common/libknewstickercommon.la
knewstickerstub_LDFLAGS = -L$(top_builddir)/knewsticker/common $(KDE_PATH) $(all_libraries)

KDE_ICON = AUTO

