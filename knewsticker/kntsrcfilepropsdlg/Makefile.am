INCLUDES = -I$(top_srcdir)/knewsticker/common $(all_includes)

lib_LTLIBRARIES = libkntsrcfilepropsdlg.la

libkntsrcfilepropsdlg_la_SOURCES = kntsrcfilepropsdlg.cpp \
	kntsrcfilepropsdlgwidget.ui
libkntsrcfilepropsdlg_la_LDFLAGS = -L$(top_builddir)/knewsticker/common \
  $(KDE_LDFLAGS) $(KDE_PLUGIN)
libkntsrcfilepropsdlg_la_LIBADD = $(LIB_KFILE) $(LIB_KDEUI) \
  $(top_builddir)/knewsticker/common/libknewstickercommon.la
libkntsrcfilepropsdlg_la_METASOURCES = AUTO

service_DATA = kntsrcfilepropsdlg.desktop
servicedir = $(kde_servicesdir)
