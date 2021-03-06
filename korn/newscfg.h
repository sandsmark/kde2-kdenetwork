/*
* newscfg.h -- Declaration of class KNewsCfg.
* Generated by newclass on Mon Aug  3 15:41:58 EST 1998.
*/
#ifndef SSK_NEWSCFG_H
#define SSK_NEWSCFG_H

#include"moncfg.h"

class QLineEdit;
class KNewsDrop;

/**
* Configuration manager for @ref KNewsDrop monitors.
* @author Sirtaj Singh Kang (taj@kde.org)
* @version $Id: newscfg.h 53240 2000-06-14 19:37:24Z rikkus $
*/
class KNewsCfg : public KMonitorCfg
{
public:
	/**
	* KNewsCfg Constructor
	*/
	KNewsCfg( KNewsDrop * );

	/**
	* KNewsCfg Destructor
	*/
	virtual ~KNewsCfg() {}
	
	virtual QString name() const;
	virtual QWidget *makeWidget( QWidget *parent );
	virtual void updateConfig();

private:
	KNewsCfg& operator=( KNewsCfg& );
	KNewsCfg( const KNewsCfg& );

	QLineEdit *_serverEdit;
	QLineEdit *_groupEdit;
};

#endif // SSK_NEWSCFG_H
