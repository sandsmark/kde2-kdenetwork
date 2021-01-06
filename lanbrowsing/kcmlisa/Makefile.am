lib_LTLIBRARIES = libkcm_lanbrowser.la

libkcm_lanbrowser_la_SOURCES = main.cpp kcmlisa.cpp kcmreslisa.cpp kcmkiolan.cpp portsettingsbar.cpp findnic.cpp

libkcm_lanbrowser_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_lanbrowser_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)

SUBDIRS = 

libkcm_lanbrowser_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_lanbrowser_la_SOURCES) -o $(podir)/kcmlanbrowser.pot

data_DATA = lanbrowser.desktop
datadir = $(kde_appsdir)/Settings/Network
