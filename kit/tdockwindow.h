// tdockwindow.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TDOCKWINDOW_H
#define TDOCKWINDOW_H

#include <ksystemtray.h>

class TDockWindow : public KSystemTray
{
Q_OBJECT

public:
	TDockWindow(QWidget * = 0, const char * = 0);

	inline KPopupMenu *contextMenu(void) {return KSystemTray::contextMenu();};
};

#endif
