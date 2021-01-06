
#include <qtabbar.h>

#include <kiconloader.h>

#include "mditoplevel.h"
#include "toplevel.h"

// FIXME: This can be removed when Qt 3 is used.
// This is really hacky and relies on undocumented QTabBar behaviour (malte)
class MDITabBar : public QTabBar
{
public:
	MDITabBar(QWidget *parent = 0, const char *name = 0)
		: QTabBar(parent, name)
	{
	};
	void selectNext()
	{
		int current = currentTab();
		int first = INT_MAX;
		int next = INT_MAX;
		for (QListIterator<QTab> it(*tabList()); it.current(); ++it)
		{
			if (it.current()->id < first)
				first = it.current()->id;
			if (it.current()->id > current && it.current()->id < next)
				next = it.current()->id;
		}
		setCurrentTab(tab(next < INT_MAX ? next : first));
	};
	void selectPrevious()
	{
		int current = currentTab();
		int last = -1;
		int prev = -1;
		for (QListIterator<QTab> it(*tabList()); it.current(); ++it)
		{
			if (it.current()->id > last)
				last = it.current()->id;
			if (it.current()->id < current && it.current()->id > prev)
				prev = it.current()->id;
		}
		setCurrentTab(tab(prev >= 0 ? prev : last));
	};
};

MDITabWidget::MDITabWidget(QWidget *parent, const char *name)
    : QTabWidget(parent, name)
{
	setTabBar(new MDITabBar(this));
}

int MDITabWidget::count() const
{
    return tabBar()->count();
}

void MDITabWidget::selectNext()
{
	static_cast<MDITabBar *>(tabBar())->selectNext();
}

void MDITabWidget::selectPrevious()
{
	static_cast<MDITabBar *>(tabBar())->selectPrevious();
}

MDITopLevel::MDITopLevel()
{
	m_closing = false;
    m_tab = new MDITabWidget( this );
    m_tab->setTabPosition( QTabWidget::Bottom );
    setCentralWidget( m_tab );

    connect( m_tab, SIGNAL( currentChanged( QWidget * ) ),
             this, SLOT( slotCurrentChanged( QWidget * ) ) );

    resize( 600, 300 );

    m_dirtyIcon = UserIcon( "star" );
}

MDITopLevel::~MDITopLevel()
{
    QListIterator<QWidget> it( m_tabWidgets );
    for (; it.current(); ++it )
        it.current()->disconnect( this, 0 );
}

void MDITopLevel::addWidget( QWidget *widget, bool show )
{
    if ( m_tabWidgets.containsRef( widget ) )
        return;

    widget->reparent( m_tab, 0, QPoint( 0, 0 ), show );

    int space = widget->caption().find(" ");
    m_tab->addTab( widget, space < 1 ? widget->caption():widget->caption().left(space) );
    m_tab->showPage( widget );

    m_tabWidgets.append( widget );

    connect( widget, SIGNAL( destroyed() ) ,
             this, SLOT( slotWidgetDestroyed() ) );

    connect( widget, SIGNAL( changeChannel( const QString &, const QString & ) ),
             this, SLOT( slotChangeChannelName( const QString &, const QString & ) ) );

    widget->installEventFilter( this );

    connect( widget, SIGNAL( changed() ),
             this, SLOT( slotMarkPageDirty() ) );
}

void MDITopLevel::removeWidget( QWidget *widget )
{
    if (m_closing)
        return;
    m_tabWidgets.removeRef( widget );
    m_tab->removePage( widget );
    widget->removeEventFilter( this );
    widget->disconnect( this, 0 );
}

void MDITopLevel::closeEvent( QCloseEvent *ev )
{
    m_closing = true;
    QListIterator<QWidget> it( m_tabWidgets );
    for (; it.current(); ++it )
        it.current()->close( false );

    KMainWindow::closeEvent( ev );
    m_closing = false;
}

void MDITopLevel::slotWidgetDestroyed()
{
    const QWidget *widget = static_cast<const QWidget *>( sender() );

    m_tabWidgets.removeRef( widget );
}

bool MDITopLevel::eventFilter( QObject *obj, QEvent *ev )
{
    if ( ev->type() != QEvent::CaptionChange )
        return false;

    QWidget *widget = dynamic_cast<QWidget *>( obj );

    if ( !widget || !m_tabWidgets.containsRef( widget ) )
        return false;

    if ( m_tab->currentPage() == widget )
        setPlainCaption( widget->caption() );

    return false;
}

void MDITopLevel::slotCurrentChanged( QWidget *page )
{
    KSircTopLevel *window = dynamic_cast<KSircTopLevel *>( page );

    if ( !window )
        return;

    m_tab->changeTab( window, QIconSet(), m_tab->tabLabel( window ) );

    window->lineEdit()->setFocus();
    setPlainCaption( window->QWidget::caption() );
}

void MDITopLevel::slotMarkPageDirty()
{
    KSircTopLevel *window = dynamic_cast<KSircTopLevel *>( const_cast<QObject *>( sender() ) );

    if ( !window )
        return;

    if ( window != m_tab->currentPage() )
        m_tab->changeTab( window, m_dirtyIcon, m_tab->tabLabel( window ) );
}

void MDITopLevel::slotChangeChannelName( const QString &, const QString &channelName )
{
    KSircTopLevel *window = dynamic_cast<KSircTopLevel *>( const_cast<QObject *>( sender() ) );

    if ( !window )
        return;

    // ### loose iconset here?
    m_tab->changeTab( window, channelName );
}

#include "mditoplevel.moc"
