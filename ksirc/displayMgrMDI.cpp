#include "displayMgrMDI.h"
#include "config.h"

#include "displayMgr.h"
#include "toplevel.h"

#include <qtabwidget.h>

#include <kaccel.h>
#include <klocale.h>
#include <qwidget.h>

#include <assert.h>

DisplayMgrMDI::DisplayMgrMDI()
{
}

DisplayMgrMDI::~DisplayMgrMDI()
{
    if ( m_topLevel )
        delete static_cast<MDITopLevel *>( m_topLevel );
}

void DisplayMgrMDI::newTopLevel( QWidget *w, bool show )
{
    topLevel()->addWidget( w, show );
}

void DisplayMgrMDI::removeTopLevel(QWidget *w )
{
    if ( !m_topLevel )
        return;

    assert( w );

    m_topLevel->removeWidget( w );

    if ( m_topLevel->widgets().count() == 0 )
    {
		if ( !m_topLevel->closing() )
			delete static_cast<MDITopLevel *>( m_topLevel );
        m_topLevel = 0;
    }
}

void DisplayMgrMDI::show(QWidget *w)
{
    if ( !m_topLevel )
        return;

    m_topLevel->tabWidget()->showPage( w );
}

void DisplayMgrMDI::raise(QWidget *w)
{
    assert( m_topLevel );

    m_topLevel->tabWidget()->showPage( w );
}


void DisplayMgrMDI::setCaption(QWidget *w, const QString& cap)
{
    assert( m_topLevel );

    w->setCaption(cap);

    m_topLevel->tabWidget()->changeTab( w, cap );
}

void DisplayMgrMDI::slotCycleTabsLeft()
{

    assert( m_topLevel );

    m_topLevel->tabWidget()->selectPrevious();
}

void DisplayMgrMDI::slotCycleTabsRight()
{

    assert( m_topLevel );

    m_topLevel->tabWidget()->selectNext();
}


MDITopLevel *DisplayMgrMDI::topLevel()
{
    if ( !m_topLevel )
    {
        m_topLevel = new MDITopLevel;
        m_topLevel->show();

        KAccel *a = new KAccel( m_topLevel );
        a->insertItem( i18n( "Cycle left" ), "cycle left",  ALT + Key_Left );
        a->insertItem( i18n( "Cycle right" ), "cycle right", ALT + Key_Right );
        a->connectItem("cycle left", this, SLOT(slotCycleTabsLeft() ) );
        a->connectItem("cycle right", this, SLOT(slotCycleTabsRight() ) );
    }

    return m_topLevel;
}

#include "displayMgrMDI.moc"
