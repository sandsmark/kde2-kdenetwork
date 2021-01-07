qt2_wrap_cpp(libkntsrcfilepropsdlg_MOC
    SOURCES
        kntsrcfilepropsdlg.h
    )
qt2_wrap_ui(libkntsrcfilepropsdlg_UI
    SOURCES
        kntsrcfilepropsdlgwidget.ui
    )
kde2_module(libkntsrcfilepropsdlg
    SOURCES
        kntsrcfilepropsdlg.cpp
        ${libkntsrcfilepropsdlg_MOC}
        ${libkntsrcfilepropsdlg_UI}
    LIBS
        kde2::knewsticker
        kde2::kfile
    )

install(FILES kntsrcfilepropsdlg.desktop DESTINATION  ${KDE2_SERVICESDIR})
