KDE_CXXFLAGS = $(USE_EXCEPTIONS) -UQT_NO_ASCII_CAST

INCLUDES= $(all_includes)

####### This part is very ksirc specific
# you can add here more. This one gets installed
noinst_LTLIBRARIES = libksprefs.la

libksprefs_la_SOURCES = ksprefs.cpp general.cpp generalData.cpp \
	startup.cpp startupdata.cpp serverchannel.cpp \
	serverchanneldata.cpp UserMenuRef.cpp UserMenuRefData.cpp \
	defaultfilters.cpp defaultfiltersdata.cpp

noinst_HEADERS = ksprefs.h serverchanneldata.h generalData.h \
	startupdata.h serverchannel.h general.h startup.h \
	UserMenuRef.h UserMenuRefData.h defaultfilters.h \
	defaultfiltersdata.h

libksprefs_la_METASOURCES = USE_AUTOMOC

