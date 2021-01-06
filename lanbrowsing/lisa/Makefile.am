##
AM_CXXFALGS = -fno-rtti -fno-exceptions

#CXXFLAGS = -fno-rtti -fno-exceptions

bin_PROGRAMS = lisa reslisa
#
# Programs to compile, Manpages to install and Versions
#
#INCLUDES = $(all_includes)

lisa_SOURCES = \
addressvalidator.cpp \
netmanager.cpp \
netscanner.cpp \
getdebug.cpp \
ipaddress.cpp \
main.cpp \
mystring.cpp \
simplelist.cpp \
client.cpp \
configfile.cpp

reslisa_SOURCES = \
addressvalidator.cpp \
netmanager.cpp \
netscanner.cpp \
getdebug.cpp \
ipaddress.cpp \
strictmain.cpp \
mystring.cpp \
simplelist.cpp \
client.cpp \
configfile.cpp

lisa_LDADD = $(LIBSOCKET)

reslisa_LDADD = $(LIBSOCKET)

misc_DATA = README
miscdir = $(kde_datadir)/lisa
EXTRA_DIST=$(misc_DATA)   

