#ifndef DISPLAYMGR_H
#define DISPLAYMGR_H

#include <qwidget.h>

class DisplayMgr {
public:
    virtual ~DisplayMgr() {}

    virtual void newTopLevel(QWidget *, bool show = FALSE) = 0;

    virtual void removeTopLevel(QWidget *) = 0;

    virtual void show(QWidget *) = 0;

    virtual void raise(QWidget *) = 0;

    virtual void setCaption(QWidget *, const QString&) = 0;

};


#endif

