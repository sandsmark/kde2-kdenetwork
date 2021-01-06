#ifndef __displaymgrmdi_h__
#define __displaymgrmdi_h__

#include "displayMgr.h"
#include "mditoplevel.h"

#include <qwidget.h>


class DisplayMgrMDI : public QObject, public DisplayMgr
{
    Q_OBJECT
public:
    DisplayMgrMDI();
    virtual ~DisplayMgrMDI();

    virtual void newTopLevel(QWidget *, bool show = FALSE);
    virtual void removeTopLevel(QWidget *);

    virtual void show(QWidget *);
    virtual void raise(QWidget *);

    virtual void setCaption(QWidget *, const QString&);

public slots:
    virtual void slotCycleTabsLeft();
    virtual void slotCycleTabsRight();

private:
    MDITopLevel *topLevel();

    QGuardedPtr<MDITopLevel> m_topLevel;
};


#endif

