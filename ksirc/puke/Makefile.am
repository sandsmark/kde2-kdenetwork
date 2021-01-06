KDE_CXXFLAGS = $(USE_EXCEPTIONS) -UQT_NO_ASCII_CAST

INCLUDES= $(all_includes)

METASOURCES = AUTO

noinst_LTLIBRARIES = libpuke.la

libpuke_la_SOURCES = pwidget.cpp pobject.cpp \
	playout.cpp controller.cpp pframe.cpp \
	plined.cpp pbutton.cpp ppushbt.cpp \
	pprogress.cpp ptablevw.cpp plistbox.cpp \
	plabel.cpp pmenudta.cpp	ppopmenu.cpp \
	palistbox.cpp ptabdialog.cpp ksirclistbox.cpp \
	pkfiledialog.cpp pobjfinder.cpp

$(srcdir)/HOWTO-PUKE.txt: HOWTO-PUKE.pod
	pod2text $(srcdir)/HOWTO-PUKE.pod > HOWTO-PUKE.txt

# no install headers

noinst_HEADERS = \
	commands.h plined.h pwidget.h widgethdlr.h \
	controller.h pframe.h pmessage.h HOWTO-PUKE.pod \
	commands-handler.pl  pboxlayout.pm pwidget.pm \
	commands-perl.pl pframe.pm test.pl convert_commands.pl \
	plined.pm tester.pl pbase.pm puke.pl ppushbt.h \
	pbutton.h pprogress.h ptablevw.h plistbox.h \
	plabel.h pobject.h playout.h pmenudta.h ppopmenu.h \
	palistbox.h ptabdialog.h ksirclistbox.h

# Datafiles to install / uninstall : *.pl *.pm
data_DATA = commands-handler.pl commands-perl.pl convert_commands.pl \
	puke.pl small.pl test.pl tester.pl \
	dcc_progress.pm dcc_status.pm ksirclistbox.pm load_all.pm \
	palistbox.pm pbase.pm pboxlayout.pm pbutton.pm pframe.pm \
	pkfiledialog.pm plabel.pm plined.pm plistbox.pm pmenudta.pm \
	ppopmenu.pm pprogress.pm ppushbt.pm ptabdialog.pm ptablevw.pm \
	pwidget.pm pkfiledialog.pm pobjfinder.pm

# Where to install them
datadir = $(kde_datadir)/ksirc

