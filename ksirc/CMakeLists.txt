qt2_wrap_cpp(ksirc_MOC
    SOURCES
        ahistlineedit.h
        alistbox.h
        dccDialogData.h
        dccDialog.h
        displayMgrMDI.h
        FilterRuleEditorData.h
        FilterRuleEditor.h
        iocontroller.h
        ioDCC.h
        ioLAG.h
        ioNotify.h
        kapopupmenu.h
        KSircColour.h
        ksircprocess.h
        mditoplevel.h
        NewWindowDialog.h
        objFinder.h
        servercontroller.h
        ssfepromptdata.h
        ssfeprompt.h
        toplevel.h
        welcomeWinData.h
        welcomeWin.h

        KSColourPicker/kscolourpickerData.h
        KSColourPicker/kscolourpicker.h
        KSircListBox/irclistbox.h
        KSircListBox/irclistitem.h
        KSOpenkSirc/enter_combo.h
        KSOpenkSirc/open_ksirc.h
        KSPrefs/defaultfiltersdata.h
        KSPrefs/defaultfilters.h
        KSPrefs/generalData.h
        KSPrefs/general.h
        KSPrefs/ksprefs.h
        KSPrefs/serverchanneldata.h
        KSPrefs/serverchannel.h
        KSPrefs/startupdata.h
        KSPrefs/startup.h
        KSPrefs/UserMenuRefData.h
        KSPrefs/UserMenuRef.h
        KSProgress/ksprogressdata.h
        KSProgress/ksprogress.h
        KSTicker/ksticker.h
        KSTicker/speeddialogData.h
        KSTicker/speeddialog.h

        puke/controller.h
        puke/ksirclistbox.h
        puke/palistbox.h
        puke/pbutton.h
        puke/pframe.h
        puke/pkfiledialog.h
        puke/plabel.h
        puke/playout.h
        puke/plined.h
        puke/plistbox.h
        puke/pmenudta.h
        puke/pobject.h
        puke/pobjfinder.h
        puke/ppopmenu.h
        puke/pprogress.h
        puke/ppushbt.h
        puke/ptabdialog.h
        puke/ptablevw.h
        puke/pwidget.h
    )

qt2_wrap_ui(ksirc_UI
    SOURCES
        KSOpenkSirc/open_ksircData.ui
    )

remove_definitions(-DQT_NO_ASCII_CAST)

kde2_kinit_executable(ksirc
    SOURCES
        FilterRuleEditor.cpp
        KSircColour.cpp
        NewWindowDialog.cpp
        ahistlineedit.cpp
        alistbox.cpp
        baserules.cpp
        chanparser.cpp
        displayMgrMDI.cpp
        displayMgrSDI.cpp
        ioBroadcast.cpp
        ioDCC.cpp
        ioDiscard.cpp
        ioLAG.cpp
        ioNotify.cpp
        iocontroller.cpp
        kapopupmenu.cpp
        ksirc.cpp
        ksircprocess.cpp
        linelistitem.cpp
        mditoplevel.cpp
        messageReceiver.cpp
        objFinder.cpp
        servercontroller.cpp
        ssfeprompt.cpp
        toplevel.cpp
        usercontrolmenu.cpp
        welcomeWin.cpp
        FilterRuleEditorData.cpp
        dccDialog.cpp
        dccDialogData.cpp
        ssfepromptdata.cpp
        welcomeWinData.cpp

        KSOpenkSirc/open_ksirc.cpp
        KSOpenkSirc/serverFileParser.cpp
        KSOpenkSirc/enter_combo.cpp

        KSircListBox/irclistitem.cpp
        KSircListBox/irclistbox.cpp
        KSircListBox/kspainter.cpp

        KSColourPicker/kscolourpicker.cpp
        KSColourPicker/kscolourpickerData.cpp

        KSTicker/ksticker.cpp
        KSTicker/speeddialog.cpp
        KSTicker/speeddialogData.cpp

        puke/controller.cpp
        puke/ksirclistbox.cpp
        puke/palistbox.cpp
        puke/pbutton.cpp
        puke/pframe.cpp
        puke/pkfiledialog.cpp
        puke/plabel.cpp
        puke/playout.cpp
        puke/plined.cpp
        puke/plistbox.cpp
        puke/pmenudta.cpp
        puke/pobject.cpp
        puke/pobjfinder.cpp
        puke/ppopmenu.cpp
        puke/pprogress.cpp
        puke/ppushbt.cpp
        puke/ptabdialog.cpp
        puke/ptablevw.cpp
        puke/pwidget.cpp

        KSProgress/ksprogress.cpp
        KSProgress/ksprogressdata.cpp

        KSPrefs/UserMenuRef.cpp
        KSPrefs/UserMenuRefData.cpp
        KSPrefs/defaultfilters.cpp
        KSPrefs/defaultfiltersdata.cpp
        KSPrefs/general.cpp
        KSPrefs/generalData.cpp
        KSPrefs/ksprefs.cpp
        KSPrefs/serverchannel.cpp
        KSPrefs/serverchanneldata.cpp
        KSPrefs/startup.cpp
        KSPrefs/startupdata.cpp

        ${ksirc_MOC}
        ${ksirc_UI}
    LIBS
        kde2::kfile
    COMPILE_DEFINITIONS
        -DPACKAGE=\"ksirc\"
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/KSOpenkSirc
    )

qt2_wrap_cpp(ksticker_MOC
    SOURCES
        KSTicker/main.h
        KSTicker/ksticker.h
        KSTicker/speeddialog.h
    )
add_executable(ksticker
        KSTicker/main.cpp
        KSTicker/kspainter.cpp
        KSTicker/ksticker.cpp
        KSTicker/speeddialog.cpp
        KSTicker/speeddialogData.cpp
        ${ksticker_MOC}
    )
target_include_directories(ksticker PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ksticker kde2::kdeui)
install(TARGETS ksticker)

install(FILES
    dsirc
    PERMISSIONS WORLD_EXECUTE WORLD_READ
    DESTINATION ${KDE2_BINDIR}
    )

install(FILES ksirc.desktop DESTINATION ${KDE2_APPSDIR}/Internet)
install(
    FILES
        ksirc.pl filters.pl sirc.help.gz relnotes

        puke/commands-handler.pl
        puke/commands-perl.pl
        puke/convert_commands.pl
        puke/dcc_progress.pm
        puke/dcc_status.pm
        puke/ksirclistbox.pm
        puke/load_all.pm
        puke/palistbox.pm
        puke/pbase.pm
        puke/pboxlayout.pm
        puke/pbutton.pm
        puke/pframe.pm
        puke/pkfiledialog.pm
        puke/plabel.pm
        puke/plined.pm
        puke/plistbox.pm
        puke/pmenudta.pm
        puke/pobjfinder.pm
        puke/ppopmenu.pm
        puke/pprogress.pm
        puke/ppushbt.pm
        puke/ptabdialog.pm
        puke/ptablevw.pm
        puke/puke.pl
        puke/pwidget.pm
        puke/small.pl
        puke/test.pl
        puke/tester.pl

        KSOpenkSirc/servers.ini
        KSOpenkSirc/servers.txt

    DESTINATION ${KDE2_DATADIR}/ksirc
    )

install(FILES ksircrc DESTINATION ${KDE2_CONFDIR})

add_subdirectory(img)

