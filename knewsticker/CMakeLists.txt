add_subdirectory(common)
add_subdirectory(kcmnewsticker)
add_subdirectory(kntsrcfilepropsdlg)
add_subdirectory(knewstickerstub)

kde2_stdicon()
kde2_kidl(knewsticker_IDL
    SOURCES
        knewsticker.h
    )

qt2_wrap_cpp(knewsticker_MOC
    SOURCES
        knewsticker.h
        newsscroller.h
    )

kde2_module(libknewstickerapplet
    SOURCES
        newsscroller.cpp
        knewsticker.cpp
        knewsticker_skel.cpp
        ${knewsticker_IDL}
        ${knewsticker_MOC}
    LIBS
        kde2::knewsticker
    )

target_include_directories(module_libknewstickerapplet PRIVATE common)

install(FILES knewsticker.desktop DESTINATION ${KDE2_DATADIR}/kicker/applets)
install(FILES eventsrc DESTINATION ${KDE2_DATADIR}/knewsticker)
install(FILES knewsticker.upd knt-0.1-0.2.pl DESTINATION ${KDE2_DATADIR}/kconf_update)
