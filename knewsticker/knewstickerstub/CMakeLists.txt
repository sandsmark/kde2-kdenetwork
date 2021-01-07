add_executable(knewstickerstub knewstickerstub.cpp)
target_link_libraries(knewstickerstub kde2::knewsticker)
install(TARGETS knewstickerstub)

install(FILES knewstickerstub.desktop DESTINATION "${KDE2_APPSDIR}/.hidden")
