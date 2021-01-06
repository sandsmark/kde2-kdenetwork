## Makefile.am for kdict

KDE_CXXFLAGS = $(USE_THREADS)

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = applet pics

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kdict

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# the library search path.
kdict_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
kdict_LDADD   = $(LIB_KFILE) $(LIB_KHTML) $(LIBPTHREAD) $(LIBRESOLV)

# which sources should be compiled for kdict
kdict_SOURCES = dcopinterface.skel main.cpp actions.cpp dict.cpp options.cpp queryview.cpp toplevel.cpp sets.cpp matchview.cpp application.cpp

# these are the headers for your project
noinst_HEADERS = actions.h dict.h options.h queryview.h toplevel.h sets.h matchview.h application.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(METASOURCES)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kdict.pot

KDE_ICON = AUTO

# this is where the kdelnk file will go
kdelnkdir   = $(kde_appsdir)/Utilities
kdelnk_DATA = kdict.desktop

rcdir = $(kde_datadir)/kdict
rc_DATA = kdictui.rc
