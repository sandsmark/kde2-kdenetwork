// tmessage.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TMESSAGE_H
#define TMESSAGE_H

#include <kmainwindow.h>

class QFile;
class KColorButton;
class KHTMLPart;
class KAction;
class KToggleAction;
class KLineEdit;
class QVBox;

class TMessage : public KMainWindow
{
Q_OBJECT

public:
	TMessage(const QString &display, const QString &internal,
		const QString &mine, bool log, bool tms, bool icq,
		const QColor &foregroundColor, const QColor &backgroundColor, bool Bold,
		const char * = 0);
	~TMessage();

	inline QString getName(void) {return name;};
signals:
	// message, this->name, auto
	void messageOut(QString, QString, bool);
	// this->name, anon
	void warningOut(QString, bool);
	void getInfo(QString);
	void awayPicker(TMessage &);
	// closing
	void closing(const QString &);
public slots:
	void messageIn(const QString &, bool);
	void setLogging(bool);
	void setTimestamping(bool);
	void setAway(bool, const QString &);
	void setMyName(const QString &);
	void setColors(const QColor &, const QColor &, bool);
	void setICQ(bool);

	void urlClicked(const QString &);
protected:
	virtual void closeEvent(QCloseEvent *);
	virtual void resizeEvent(QResizeEvent *);
protected slots:
	void send(void);
	void sendAuto(void);
	// actions
	void closePressed(void);
	void clearPressed(void);
	void infoPressed(void);
	void loggingPressed(void);
	void timestampingPressed(void);
	void soundPressed(void);
	void warningPressed(void);
	void anonWarningPressed(void);
	void forePressed(void);
	void backPressed(void);
	void forePressed2(const QColor &);
	void backPressed2(const QColor &);
	void boldPressed(void);
	void icqPressed(void);
private:
	void initActions(void);
	void openLogFile(void);
	void initializeHTML(void);
	void updateHTML(QString, QString, QString, bool);

	QVBox *centralWidget;
	KHTMLPart *htmlpart;
	KLineEdit *edit;
	KColorButton *editFore, *editBack;
	KAction *aClear, *aClose, *aInfo, *aWarning, *aAnonWarning, *aFore, *aBack;
	KToggleAction *aLogging, *aTimestamping, *aSound, *aBold, *aICQ;
	QString displayName, name, myName;
	bool isAway, awayMessageSent;
	QString awayMessage;
	bool isTimestamping;
	QFile *logFile;
	bool isLogging;
	bool isICQ;
	QColor fore, back;
	bool bold;
	QString conversationBuffer;
	int conversationLength;
	bool useSound;
};

#endif
