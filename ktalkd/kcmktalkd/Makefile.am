appsdir   = $(kde_appsdir)/Settings/Network
apps_DATA = kcmktalkd.desktop
KDE_ICON  = ktalkd

INCLUDES = $(all_includes)
METASOURCES=AUTO

kde_module_LTLIBRARIES = libkcm_ktalkd.la

libkcm_ktalkd_la_SOURCES = main.cpp soundpage.cpp answmachpage.cpp \
		forwmachpage.cpp
libkcm_ktalkd_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
libkcm_ktalkd_la_LIBADD = $(LIB_KDEUI) -lkfile

#for extra warnings during compilation :
# AMDEFS		=	-ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE

messages:
	$(XGETTEXT) $(libkcm_ktalkd_la_SOURCES) -o $(podir)/kcmktalkd.pot

