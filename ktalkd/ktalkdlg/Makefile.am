## -*- makefile -*-
# Ktalkdlg - Makefile.am

EXTRA_DIST	 =	

if KDE_INSTALLED

bin_PROGRAMS	 =	ktalkdlg
ktalkdlg_SOURCES =	ktalkdlg.cpp

INCLUDES	=	$(all_includes)
LDFLAGS		=	$(all_libraries) $(KDE_RPATH)
LDADD		=	$(LIB_KDEUI)

endif

#for extra warnings during compilation :
#AM_CXXFLAGS = -ansi -pedantic -D_POSIX_SOURCE -D_BSD_SOURCE

