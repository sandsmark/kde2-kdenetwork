/**********************************************************************

 Colour Setter for the text colours in TopLevel list box.

 GUI done with qtarch.

 Nothing special, just pops up a box with buttons, you click on them
 and up comes a colours selector.  Pretty basic.

 Only strange things is the ammount of dorking with pointers, you have
 to set the value os pointed to item, not change the pointer, hence
 the *ptr notation everywhere.

 *********************************************************************/

#include "KSircColour.h"
#include "config.h"

#include <kconfig.h>
#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

extern KConfig *kConfig;

KSircColour::KSircColour( QWidget* parent, const char* name )
 : KDialogBase( parent, name, false, QString::null, Apply|Close, Ok, true )
{
	init();
	setCaption( i18n("Color Settings") );

	// Set the text colour for the 4 main buttons.
	// Get the pointer values and off you go.

	ColourText->setColor( *kSircConfig->colour_text );
	ColourInfo->setColor( *kSircConfig->colour_info );
	ColourChan->setColor( *kSircConfig->colour_chan );
	ColourError->setColor( *kSircConfig->colour_error );
	if(kSircConfig->colour_background == 0){
	  kConfig->setGroup("Colours");
	  kSircConfig->colour_background = new QColor(kConfig->readColorEntry("Background", new QColor(colorGroup().mid())));
	}
	ColourBackground->setColor( *kSircConfig->colour_background );

	connect(ColourText, SIGNAL(changed(const QColor &)),
		this, SLOT(colourChange(const QColor &)));
	connect(ColourInfo, SIGNAL(changed(const QColor &)),
		this, SLOT(colourChange(const QColor &)));
	connect(ColourChan, SIGNAL(changed(const QColor &)),
		this, SLOT(colourChange(const QColor &)));
	connect(ColourError, SIGNAL(changed(const QColor &)),
		this, SLOT(colourChange(const QColor &)));
	connect(ColourBackground, SIGNAL(changed(const QColor &)),
		this, SLOT(colourChange(const QColor &)));
	connect(this, SIGNAL(applyClicked()),
		this, SLOT(apply()));
}

KSircColour::~KSircColour()
{
}

void KSircColour::init()
{
  QWidget *page = new QWidget(this);
  setMainWidget(page);
  QGridLayout *layout = new QGridLayout(page, 5, 2, 0, 10);
  QLabel* dlgedit_Label;
  dlgedit_Label = new QLabel( page, "Label_1" );
  dlgedit_Label->setText( i18n("Generic Text") );
  dlgedit_Label->setAlignment( AlignLeft | AlignVCenter );
  layout->addWidget(dlgedit_Label, 0, 0);

  ColourText = new KColorButton( page, "User_1" );
  ColourText->setMinimumSize(ColourText->sizeHint());
  layout->addWidget(ColourText, 0, 1);

  dlgedit_Label = new QLabel( page, "Label_2" );
  dlgedit_Label->setText( i18n("Info") );
  dlgedit_Label->setAlignment( AlignLeft | AlignVCenter );
  dlgedit_Label->setMinimumSize(dlgedit_Label->sizeHint());
  layout->addWidget(dlgedit_Label, 1, 0);

  ColourInfo = new KColorButton( page, "User_2" );
  ColourInfo->setMinimumSize(ColourInfo->sizeHint());
  layout->addWidget(ColourInfo, 1, 1);

  dlgedit_Label = new QLabel( page, "Label_3" );
  dlgedit_Label->setText( i18n("Chan Messages") );
  dlgedit_Label->setAlignment( AlignLeft | AlignVCenter );
  dlgedit_Label->setMinimumSize(dlgedit_Label->sizeHint());
  layout->addWidget(dlgedit_Label, 2, 0);

  ColourChan = new KColorButton( page, "User_3" );
  ColourChan->setMinimumSize(ColourChan->sizeHint());
  layout->addWidget(ColourChan, 2, 1);

  dlgedit_Label = new QLabel( page, "Label_4" );
  dlgedit_Label->setText( i18n("Errors") );
  dlgedit_Label->setAlignment( AlignLeft | AlignVCenter );
  dlgedit_Label->setMinimumSize(dlgedit_Label->sizeHint());
  layout->addWidget(dlgedit_Label, 3, 0);

  ColourError = new KColorButton( page, "User_4" );
  ColourError->setMinimumSize(ColourError->sizeHint());
  layout->addWidget(ColourError, 3, 1);

  dlgedit_Label = new QLabel( page, "Label_5" );
  dlgedit_Label->setText( i18n("Background") );
  dlgedit_Label->setAlignment( AlignLeft | AlignVCenter );
  dlgedit_Label->setMinimumSize(dlgedit_Label->sizeHint());
  layout->addWidget(dlgedit_Label, 4, 0);

  ColourBackground = new KColorButton( page, "User_5" );
  ColourBackground->setMinimumSize(ColourBackground->sizeHint());
  layout->addWidget(ColourBackground, 4, 1);
}

void KSircColour::apply()
{
  // Write the values back again.  This will change the painter 
  // colours on the fly.
  
  *kSircConfig->colour_text = ColourText->color();
  *kSircConfig->colour_info = ColourInfo->color();
  *kSircConfig->colour_chan = ColourChan->color();
  *kSircConfig->colour_error = ColourError->color();
  *kSircConfig->colour_background = ColourBackground->color();
  kConfig->setGroup("Colours");
  kConfig->writeEntry("text", *kSircConfig->colour_text);
  kConfig->writeEntry("info", *kSircConfig->colour_info);
  kConfig->writeEntry("chan", *kSircConfig->colour_chan);
  kConfig->writeEntry("error", *kSircConfig->colour_error);
  kConfig->writeEntry("Background", *kSircConfig->colour_background);
  kConfig->sync();
  emit update();
}

void KSircColour::colourChange(const QColor &)
{
  *kSircConfig->colour_text = ColourText->color();
  *kSircConfig->colour_info = ColourInfo->color();
  *kSircConfig->colour_chan = ColourChan->color();
  *kSircConfig->colour_error = ColourError->color();
  *kSircConfig->colour_background = ColourBackground->color();
}
#include "KSircColour.moc"
