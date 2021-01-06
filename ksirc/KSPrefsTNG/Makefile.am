KDE_CXXFLAGS = $(USE_RTTI) $(USE_EXCEPTIONS) -UQT_NO_ASCII_CAST

INCLUDES= $(all_includes)

####### This part is very ksirc specific
# you can add here more. This one gets installed
noinst_LTLIBRARIES = libksprefs.la

libksprefs_la_SOURCES = \
	ksprefs.cpp \
	page_colorsbase.ui\
	page_generalbase.ui\
	page_rmbmenubase.ui \
	page_servchanbase.ui \
	page_startupbase.ui \
	page_colors.cpp \
	page_general.cpp \
	page_rmbmenu.cpp \
	page_servchan.cpp \
	page_startup.cpp

libksprefs_la_METASOURCES = USE_AUTOMOC

noinst_HEADERS = ksprefs.h \
		 page_colorsbase.h \
		 page_generalbase.h \
		 page_rmbmenubase.h \
		 page_servchanbase.h \
		 page_startupbase.h \
                 page_colors.h \
                 page_general.h \
                 page_rmbmenu.h \
                 page_servchan.h \
                 page_startup.h
