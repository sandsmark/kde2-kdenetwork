// For copyright and license, see accompanying documentation

#ifndef KITHTMLPART_H
#define KITHTMLPART_H

#include <khtml_part.h>

class KitHTMLPart : public KHTMLPart
{
Q_OBJECT

public:
	KitHTMLPart(QWidget *widgetParent, const char *widgetName = 0, QObject *parent = 0, const char *name = 0);

signals:
	void urlClicked(const QString &);

protected:
	virtual void urlSelected(const QString &url, int = 0, int = 0, const QString & = QString::null);
};

#endif
