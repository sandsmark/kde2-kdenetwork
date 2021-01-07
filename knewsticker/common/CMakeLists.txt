qt2_wrap_cpp(libknewsticker_MOC
    SOURCES
        newsengine.h
        newsiconmgr.h
        xmlnewsaccess.h
    )

kde2_kidl(libknewsticker_IDL
    SOURCES
        newsiconmgr.h
    )
kde2_library(libknewsticker
    SOURCES
        newsiconmgr.cpp xmlnewsaccess.cpp
        configaccess.cpp newsiconmgr_skel.cpp newsengine.cpp
        ${libknewsticker_MOC}
        ${libknewsticker_IDL}
    LIBS
        kde2::kio
        kde2::kdenetwork
    )

