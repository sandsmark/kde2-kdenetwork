# New ports collection makefile for:   kdenetwork
# Version required:    1.0
# Date created:        18 June 1998
# Whom:                Alex Zepeda <garbanzo@hooked.net>
#
# $Id: Makefile 8638 1998-06-24 19:11:42Z bieker $
#
PKGNAME=	kdenetwork-`date "+%y%m%d"`
CATEGORIES=	net kde

MAINTAINER=	garbanzo@hooked.net

BUILD_DEPENDS=	moc:${PORTSDIR}/x11/qt \
		autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT?=	${.CURDIR}/${PKGNAME}
LIB_DEPENDS=	qt\\.1\\.3[0-9]:${PORTSDIR}/x11/qt \
		gif\\.[23]\\.:${PORTSDIR}/graphics/giflib \
		jpeg\\.9\\.:${PORTSDIR}/graphics/jpeg \
		uulib\\.1\\.:${PORTSDIR}/converters/kdesupport \
		kdecore\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		kdeui\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		jscript\\.0\\.9[0-9]:${PORTSDIR}/x11/kdelibs \
		khtmlw\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		kfm\\.0\\.9[0-9]:${PORTSDIR}/x11/kdelibs \
		mediatool\\.0\\.9[1-9]:${PORTSDIR}/x11/kdelibs \
		mimelib\\.1\\.[0-9]:${PORTSDIR}/converters/kdesupport \
		gdbm\\.1\\\.7\\.:${PORTSDIR}/databases/gdbm
NO_MTREE=	yup
USE_GMAKE=	"without a doubt"
NO_WRKDIR=	"Cause, you've obviously got the source in the parent directory"
WRKSRC=		../
NO_EXTRACT=	"yes, please"
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	"--prefix=$(PREFIX)" \
		"--x-inc=$(X11BASE)/include" \
		"--x-lib=$(X11BASE)/lib" \
		"--with-install-root=$(INSTALL_ROOT)" \
		"--with-extra-includes=$(PREFIX)/include/giflib"

.include "kde-snapshot.mk"
.include <bsd.port.mk>
