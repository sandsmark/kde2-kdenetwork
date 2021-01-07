qt2_wrap_ui(libkcm_newsticker_UI
    SOURCES
        newssourcedlg.ui kcmnewstickerwidget.ui
    )
qt2_wrap_cpp(libkcm_newsticker_MOC
    SOURCES
        kcmnewsticker.h
        newssourcedlgimpl.h
    )
kde2_module(libkcm_newsticker
    SOURCES
	newssourcedlgimpl.cpp kcmnewsticker.cpp
        ${libkcm_newsticker_UI}
        ${libkcm_newsticker_MOC}
    LIBS
        kde2::knewsticker
        kde2::kfile
    )

install(FILES kcmnewsticker.desktop DESTINATION "${KDE2_APPSDIR}/Settings/Network")
install(FILES uninstall.desktop DESTINATION "${KDE2_APPSDIR}/Settings/Personalization" RENAME kcmnewsticker.desktop)
