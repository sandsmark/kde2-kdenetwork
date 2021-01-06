/**********************************************************************

	--- Qt Architect generated file ---

	File: KSircColour.h
	Last generated: Fri Dec 12 22:54:52 1997

 *********************************************************************/

#ifndef KSircColour_included
#define KSircColour_included

#include <kdialogbase.h>
#include <kcolorbutton.h>

class KSircColour : public KDialogBase
{
    Q_OBJECT

public:
    KSircColour(QWidget *parent=0, const char *name=0);
    virtual ~KSircColour();

    void init();

signals:
    void update();

protected slots:
    virtual void apply();
    virtual void colourChange(const QColor &);
protected:
    KColorButton* ColourText;
    KColorButton* ColourInfo;
    KColorButton* ColourChan;
    KColorButton* ColourError;
    KColorButton* ColourBackground;
};
#endif // KSircColour_included


