#ifndef __mditoplevel_h__
#define __mditoplevel_h__

#include <qlist.h>
#include <qtabwidget.h>

#include <kmainwindow.h>

class MDITabWidget : public QTabWidget
{
    public:
        MDITabWidget(QWidget * = 0, const char * = 0);

        int count() const;
		void selectNext();
		void selectPrevious();
};

class MDITopLevel : public KMainWindow
{
    Q_OBJECT
public:
    MDITopLevel();
    virtual ~MDITopLevel();

    void addWidget( QWidget *widget, bool show );

    void removeWidget( QWidget *widget );

    MDITabWidget *tabWidget() const { return m_tab; }

    QList<QWidget> &widgets() { return m_tabWidgets; }

    virtual bool eventFilter( QObject *obj, QEvent *ev );
	bool closing() const { return m_closing; }

protected:
    virtual void closeEvent( QCloseEvent *ev );

private slots:
    void slotWidgetDestroyed();
    void slotCurrentChanged( QWidget *page );
    void slotMarkPageDirty();
    void slotChangeChannelName( const QString &, const QString &newName );

private:
    MDITabWidget *m_tab;
    QList<QWidget> m_tabWidgets;
    QPixmap m_dirtyIcon;
	bool m_closing;
};

#endif
