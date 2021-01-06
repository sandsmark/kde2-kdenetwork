#ifndef __INCLUD_H
#define __INCLUD_H "$Id"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h> 

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>		// Needed on some systems.
#endif

#include "talkd.h"

#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_UTMP
#define _PATH_UTMP UTMP
#endif
#ifndef _PATH_DEV
#define _PATH_DEV "/dev/"
#endif
#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#endif /* __INCLUD_H */
