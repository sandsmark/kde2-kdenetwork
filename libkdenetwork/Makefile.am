KDE_CXXFLAGS = $(USE_RTTI)
INCLUDES= $(all_includes)

lib_LTLIBRARIES = libkdenetwork.la

libkdenetwork_la_SOURCES = kscoring.cpp \
                          kscoringeditor.cpp \
                          qregexp3.cpp \
                          kregexp3.cpp \
			  kmimetypevalidator.cpp \
                          kpgp.cpp \
                          kpgpbase.cpp \
                          kwidgetlister.cpp
libkdenetwork_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0
noinst_HEADERS = kpgpbase.h
libkdenetwork_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

messages: 
	$(XGETTEXT) *.cpp *.h -o $(podir)/libkdenetwork.pot
