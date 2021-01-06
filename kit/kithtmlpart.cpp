// For copyright and license, see accompanying documentation

#include "kithtmlpart.h"

KitHTMLPart::KitHTMLPart(QWidget *widgetParent, const char *widgetName, QObject *parent, const char *name)
	: KHTMLPart(widgetParent, widgetName, parent, name)
{
}

void KitHTMLPart::urlSelected(const QString &url, int, int, const QString &)
{
	emit urlClicked(url);
}

#include "kithtmlpart.moc"
