INCLUDES= $(all_includes)

SUBDIRS = icons

bin_PROGRAMS =	kit

kit_SOURCES = aim.cpp\
kit.cpp\
kitapp.cpp\
kitawaypicker.cpp\
kitdirectorysearch.cpp\
kithtmlpart.cpp\
kitpasswordchanger.cpp\
kitprofileeditor.cpp\
kitprofileinfo.cpp\
kitsocket.cpp\
kituserprofile.cpp\
taim.cpp\
tbuddylist.cpp\
tdockwindow.cpp\
tedit.cpp\
tmessage.cpp\
tsfbuffer.cpp\
twindow.cpp

kit_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kit_LDADD = $(LIB_KHTML)

METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kit_SOURCES) -o $(podir)/kit.pot

appsdir = $(kde_appsdir)/Internet
apps_DATA = kit.desktop

datadir = $(kde_datadir)/kit
data_DATA = eventsrc

#sounddir = $(kde_sounddir)
#sound_DATA = whatever.wav
