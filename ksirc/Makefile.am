
KDE_CXXFLAGS = $(USE_EXCEPTIONS) -UQT_NO_ASCII_CAST

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -IKSOpenkSirc

# claim, which subdirectories you want to install
SUBDIRS = KSTicker KSircListBox KSProgress KSPrefs \
	KSColourPicker KSOpenkSirc puke img

METASOURCES = AUTO

bin_PROGRAMS = ksirc

lib_LTLIBRARIES = ksirc.la

noinst_PROGRAMS = getdate

# Which sources should be compiled for ksirc.

ksirc_la_SOURCES = \
	kapopupmenu.cpp \
	toplevel.cpp \
	ksirc.cpp \
	alistbox.cpp \
	ahistlineedit.cpp \
	iocontroller.cpp \
	NewWindowDialog.cpp \
	messageReceiver.cpp \
	ioBroadcast.cpp \
	ksircprocess.cpp \
	servercontroller.cpp \
	KSircColour.cpp \
	FilterRuleEditor.cpp FilterRuleEditorData.cpp \
	ioDiscard.cpp \
	ioDCC.cpp dccDialog.cpp dccDialogData.cpp \
	linelistitem.cpp \
	ioLAG.cpp \
	ioNotify.cpp \
	ssfeprompt.cpp ssfepromptdata.cpp \
	usercontrolmenu.cpp \
	baserules.cpp \
	chanparser.cpp \
	objFinder.cpp \
	displayMgrSDI.cpp \
	displayMgrMDI.cpp \
	welcomeWin.cpp welcomeWinData.cpp \
	mditoplevel.cpp


# the library search path
ksirc_la_LDFLAGS = $(all_libraries) -module -avoid-version

all_ksirc_libraries = ./KSTicker/libksticker.la \
	./KSPrefs/libksprefs.la \
	./KSProgress/libksprogress.la  \
	./KSColourPicker/libkscolourpicker.la \
	./KSOpenkSirc/libksopenksirc.la \
	./KSircListBox/libksirclistbox.la \
	./puke/libpuke.la $(LIB_KFILE) $(LIBDL)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
ksirc_la_LIBADD   = $(all_ksirc_libraries)

ksirc_SOURCES = dummy.C
ksirc_LDADD = ksirc.la
ksirc_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kdoc:
	kdoc -ddoc ksirc *.h

i18n:
	perl conv-i18n-h `find . -name '*Data.h' -print |grep -Ev '.*/m.*'`	
	perl -n -i conv-i18n-cpp `find . -name '*Data.cpp' -print |grep -Ev '.*/m.*'`

getdate_SOURCES = getdate.c

cdate.h: getdate
	./getdate > cdate.h

$(srcdir)/ksirc.cpp: cdate.h

# this option you can leave out. Just, if you use "make dist", you need it
#
# DSIRC is on the bottom so make dist includes it!!!!!!!!!
#
noinst_HEADERS = \
FilterRuleEditor.h        control_message.h         linelistitem.h \
FilterRuleEditorData.h    dccDialog.h               messageReceiver.h \
KSircColour.h             dccDialogData.h           ioBroadcast.h     \
ioDCC.h                   usercontrolmenu.h \
ioDiscard.h \
ahistlineedit.h           iocontroller.h            servercontroller.h \
alistbox.h                chanparser.h \
config.h                  toplevel.h \
ksircprocess.h            ksirc.desktop              cdate.h \
ioLAG.h                   ioNotify.h                ssfeprompt.h \
ssfepromptdata.h          usercontrolmenu.h         objFinder.h \
dsirc                     filters.pl                ksirc.pl \
sirc.help.gz              baserules.h \
displayMgr.h displayMgrSDI.h \
NewWindowDialog.h

messages: rc.cpp
	$(EXTRACTRC) KSOpenkSirc/*.ui >> rc.cpp
	$(XGETTEXT) *.cpp */*.cpp -o $(podir)/ksirc.pot

bin_SCRIPTS = dsirc

apps_DATA = ksirc.desktop
appsdir = $(kde_appsdir)/Internet

misc_DATA = ksirc.pl filters.pl sirc.help.gz relnotes
miscdir = $(kde_datadir)/ksirc

conf_DATA = ksircrc
confdir = $(kde_confdir)

dummy.C:
	echo > dummy.C
