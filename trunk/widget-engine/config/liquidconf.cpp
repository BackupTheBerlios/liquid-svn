//port of the old liquid kcmmodule to the new kde 3.2 kstyle API 
//Some cosmetics to the config page to make it look more like a standard kde settings page

#include "liquidconf.h"
#include "previewdata.h"

#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qsettings.h>


#include <kapp.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <kipc.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>


extern "C"
{
    QWidget* allocate_kstyle_config(QWidget* parent)
    {
        return new LiquidStyleConfig(parent);
    }
}
 

LiquidStyleConfig::LiquidStyleConfig(QWidget* parent) : QWidget(parent){

    QVBoxLayout *layout = new QVBoxLayout(this);
    KGlobal::locale()->insertCatalogue("kstyle_liquidplus_config");
    //do the whole stuff here
    QFrame *frame;
    KIconLoader *loader = new KIconLoader;
    KJanusWidget *dlgbase = new KJanusWidget(this, "Liquid+ Configuration", 4 /*KJanusWidget::IconList*/ );
    dlgbase->setMinimumSize(QSize(400, 485));
    
    frame = dlgbase->addPage(i18n("Menus"), i18n("Menu Options."), loader->loadIcon("looknfeel", KIcon::Desktop, 48, KIcon::DefaultState));
    m_pageOne = new LiquidStyleConfigPrefMenu(frame);
    
    frame = dlgbase->addPage(i18n("Colors"), i18n("Custom Colors."), loader->loadIcon("colorize", KIcon::Desktop, 48, KIcon::DefaultState));
    m_pageTwo = new LiquidStyleConfigPrefColors(frame);
    
    frame = dlgbase->addPage(i18n("Misc"), i18n("Stipples and Miscellaneous Settings."), 
                  loader->loadIcon("misc", KIcon::Desktop, 48, KIcon::DefaultState));
    m_pageThree = new LiquidStyleConfigPrefMisc(frame);
    
    load();
    m_pageOne->editGroup->setEnabled(m_pageOne->btnGroup->id(m_pageOne->btnGroup->selected()) == m_pageOne->Custom);
    m_pageTwo->panelColorBtn->setEnabled(m_pageTwo->usePanelCustomBtn->isOn());
    layout->addWidget(dlgbase);
}

LiquidStyleConfigPrefMenu::LiquidStyleConfigPrefMenu(QWidget *parent) : QFrame(parent) {

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStrut(50);
    //layout->addSpacing(10);
    
    btnGroup = new QVButtonGroup(i18n("Menu style"), this);
    (void)new QRadioButton(i18n("Normal Menu"), btnGroup);
    (void)new QRadioButton(i18n("Stippled, background color"), btnGroup);
    (void)new QRadioButton(i18n("Stippled, button color"), btnGroup);
    /*(void)new QRadioButton(i18n("Translucent stippled, background color"), btnGroup);
    (void)new QRadioButton(i18n("Translucent stippled, button color"), btnGroup);*/
    (void)new QRadioButton(i18n("Custom Menu Colors"), btnGroup);
    connect(btnGroup, SIGNAL(clicked(int)), SLOT(slotBtnGroupClicked(int)));
    layout->addWidget(btnGroup);

    editGroup = new QGroupBox(i18n("Color editor"), this);
    QGridLayout *editLayout = new QGridLayout(editGroup, 1, 1, 10);
    QLabel *colorLbl = new QLabel(i18n("Menu color:"), editGroup);
    colorBtn =  new KColorButton(editGroup);
    QLabel *fgLbl = new QLabel(i18n("Text color:"), editGroup);
    fgBtn =  new KColorButton(editGroup);

    /*QLabel *opacityLbl = new QLabel(i18n("Opacity:"), editGroup);
    opacitySlider = new QSlider(0, 20, 1, 10, Qt::Horizontal,editGroup);
    connect(opacitySlider, SIGNAL(valueChanged(int)), this,SLOT(slotSliderChanged(int)));*/

    editLayout->addRowSpacing(0, 12);
    editLayout->addWidget(colorLbl, 1, 0);
    editLayout->addWidget(colorBtn, 1, 1);
    editLayout->addWidget(fgLbl, 1, 2);
    editLayout->addWidget(fgBtn, 1, 3);

    /*editLayout->addMultiCellWidget(opacityLbl, 2, 2, 0, 4);
    editLayout->addMultiCellWidget(opacitySlider, 3, 3, 0, 4);*/
    editLayout->setColStretch(4, 1);
    editLayout->setRowStretch(4, 1);

    layout->addWidget(editGroup);

    layout->addSpacing(16);
    shadowBtn = new QCheckBox(i18n("Use shadowed text"), this);
    connect(shadowBtn, SIGNAL(clicked()), SLOT(slotColorOptionChanged()));
    layout->addWidget(shadowBtn);
    layout->addStretch(1);
    resize( QSize(300, 400) );
}

LiquidStyleConfigPrefColors::LiquidStyleConfigPrefColors(QWidget *parent) : QFrame(parent) {
    QVBoxLayout *layout =new QVBoxLayout(this);
    QVGroupBox *colorGrp = new QVGroupBox(i18n("Custom colors"), this);
    usePanelCustomBtn = new QCheckBox(i18n("Use custom panel color"), colorGrp);
    connect(usePanelCustomBtn, SIGNAL(toggled(bool)), this, SLOT(slotUsePanelCustomClicked(bool)));
    QHBox *colorBox = new QHBox(colorGrp);
    QLabel *panelColorLbl = new QLabel(i18n("Custom color:"), colorBox);
    panelColorBtn = new KColorButton(colorBox);
    connect(panelColorBtn, SIGNAL(changed(const QColor &)), this, SLOT(slotPanelColorChanged(const QColor &)));
    colorBox->setStretchFactor(new QWidget(colorBox), 1);
    layout->addWidget(colorGrp);

    QVGroupBox *widgetColorGrp = new QVGroupBox(i18n("Color usage"), this);
    QWidget *colorFrame = new QWidget(widgetColorGrp);
    QGridLayout *cLayout = new QGridLayout(colorFrame, 1, 1, 5);
    useCustomColorBtn = new QCheckBox(i18n("Use custom widget colors"), colorFrame);
    connect(useCustomColorBtn, SIGNAL(toggled(bool)), this, SLOT(slotUseCustomColorClicked(bool)));

    cLayout->addMultiCellWidget(useCustomColorBtn, 1, 1, 0, 3);
    cLayout->addWidget(new QLabel(i18n("Widget:"), colorFrame), 2, 0);
    cLayout->addWidget(new QLabel(i18n("Color:"), colorFrame), 2, 1);
    cLayout->addWidget(new QLabel(i18n("Preview:"), colorFrame), 2, 2);

    cLayout->addWidget(new QLabel(i18n("Radio button (on)"), colorFrame), 3, 0);
    cLayout->addWidget(new QLabel(i18n("Radio button (off)"), colorFrame), 4, 0);
    cLayout->addWidget(new QLabel(i18n("CheckBox (on)"), colorFrame), 5, 0);
    cLayout->addWidget(new QLabel(i18n("CheckBox (off)"), colorFrame), 6, 0);
    cLayout->addWidget(new QLabel(i18n("Tab (on)"), colorFrame), 7, 0);
    cLayout->addWidget(new QLabel(i18n("Tab (off)"), colorFrame), 8, 0);
    cLayout->addWidget(new QLabel(i18n("Scrollbar slider"), colorFrame), 9, 0);
    cLayout->addWidget(new QLabel(i18n("Scrollbar groove"), colorFrame), 10, 0);

    radioOnColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(radioOnColorBtn, 3, 1);
    radioOffColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(radioOffColorBtn, 4, 1);
    cbOnColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(cbOnColorBtn, 5, 1);
    cbOffColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(cbOffColorBtn, 6, 1);
    tabOnColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(tabOnColorBtn, 7, 1);
    tabOffColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(tabOffColorBtn, 8, 1);
    sbSliderColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(sbSliderColorBtn, 9, 1);
    sbGrooveColorBtn = new KColorButton(colorFrame);
    cLayout->addWidget(sbGrooveColorBtn, 10, 1);

    radioOnLbl = new QLabel(colorFrame);
    cLayout->addWidget(radioOnLbl, 3, 2);
    radioOffLbl = new QLabel(colorFrame);
    cLayout->addWidget(radioOffLbl, 4, 2);
    cbOnLbl = new QLabel(colorFrame);
    cLayout->addWidget(cbOnLbl, 5, 2);
    cbOffLbl = new QLabel(colorFrame);
    cLayout->addWidget(cbOffLbl, 6, 2);
    tabOnLbl = new QLabel(colorFrame);
    cLayout->addWidget(tabOnLbl, 7, 2);
    tabOffLbl = new QLabel(colorFrame);
    cLayout->addWidget(tabOffLbl, 8, 2);
    sbSliderLbl = new QLabel(colorFrame);
    cLayout->addWidget(sbSliderLbl, 9, 2);
    sbGrooveLbl = new QLabel(colorFrame);
    cLayout->addWidget(sbGrooveLbl, 10, 2);

    radioOnLbl->setAlignment(AlignCenter);
    radioOffLbl->setAlignment(AlignCenter);
    cbOnLbl->setAlignment(AlignCenter);
    cbOffLbl->setAlignment(AlignCenter);
    tabOnLbl->setAlignment(AlignCenter);
    tabOffLbl->setAlignment(AlignCenter);
    sbSliderLbl->setAlignment(AlignCenter);
    sbGrooveLbl->setAlignment(AlignCenter);


    connect(radioOnColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(radioOffColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(cbOnColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(cbOffColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(tabOnColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(tabOffColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(sbSliderColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));
    connect(sbGrooveColorBtn, SIGNAL(changed(const QColor &)), this,SLOT(slotWidgetColorChanged(const QColor &)));

    cLayout->setColStretch(3, 1);
    layout->addWidget(widgetColorGrp);
    layout->addStretch(1);
    resize( QSize(300, 475) );
}

LiquidStyleConfigPrefMisc::LiquidStyleConfigPrefMisc(QWidget *parent) :  QFrame(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QVGroupBox *stippleGrp = new QVGroupBox(i18n("Stipples"), this);
    stippleBgBtn = new QCheckBox(i18n("Enable background stipple"), stippleGrp);
    connect(stippleBgBtn, SIGNAL(clicked()), this, SLOT(slotColorOptionChanged()));
    stipplePanelBtn = new QCheckBox(i18n("Enable panel stipple"), stippleGrp);
    connect(stipplePanelBtn, SIGNAL(clicked()),this, SLOT(slotColorOptionChanged()));

    QHBox *contrastBox = new QHBox(stippleGrp);
    QLabel *contrastLbl = new QLabel(i18n("Stipple contrast:"), contrastBox);
    stippleContrastSlider = new QSlider(1, 6, 1, 3, Qt::Horizontal,contrastBox);
    connect(stippleContrastSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));
    contrastBox->setStretchFactor(stippleContrastSlider, 1);
    layout->addWidget(stippleGrp);

    QVGroupBox *miscGrp = new QVGroupBox(i18n("Misc"), this);
    useTbFrameBtn = new QCheckBox(i18n("Draw toolbar button frames"), miscGrp);
    connect(useTbFrameBtn, SIGNAL(clicked()),  this, SLOT(slotColorOptionChanged()));
    reverseColorBtn = new QCheckBox(i18n("Use background, not button color \n for unpressed buttons"), miscGrp);
    connect(reverseColorBtn, SIGNAL(clicked()), this, SLOT(slotColorOptionChanged()));
    animProgressBar = new QCheckBox(i18n("Animate the Progress Bar"), miscGrp);
    connect(animProgressBar, SIGNAL(clicked()), this, SLOT(slotColorOptionChanged()));

    layout->addWidget(miscGrp);
    layout->addStretch(1);
    resize( QSize(300, 400));
}

LiquidStyleConfig::~LiquidStyleConfig()
{
    KGlobal::locale()->removeCatalogue("kstyle_liquid_config");
}

void LiquidStyleConfigPrefColors::slotWidgetColorChanged(const QColor &)
{
    QPixmap pix;
    QImage img, dest;

    img = uic_findImage("radio_down.png");
    adjustHSV(img, dest, radioOnColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    radioOnLbl->setPixmap(pix);
    img = uic_findImage("radio.png");
    adjustHSV(img, dest, radioOffColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    radioOffLbl->setPixmap(pix);

    img = uic_findImage("checkboxdown.png");
    adjustHSV(img, dest, cbOnColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    cbOnLbl->setPixmap(pix);
    img = uic_findImage("checkbox.png");
    adjustHSV(img, dest, cbOffColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    cbOffLbl->setPixmap(pix);

    img = uic_findImage("tab.png");
    adjustHSV(img, dest, tabOnColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    tabOnLbl->setPixmap(pix);
    adjustHSV(img, dest, tabOffColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    tabOffLbl->setPixmap(pix);

    img = uic_findImage("sbslider.png");
    adjustHSV(img, dest, sbSliderColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    sbSliderLbl->setPixmap(pix);

    img = uic_findImage("sbgroove.png");
    adjustHSV(img, dest, sbGrooveColorBtn->color(), colorGroup().background());
    pix.convertFromImage(dest);
    sbGrooveLbl->setPixmap(pix);

    emit changed(true);
}

void LiquidStyleConfigPrefColors::adjustHSV(QImage &img, QImage &dest, const QColor &c,
                             const QColor &bgColor)
{
    if(img.depth() != 32)
        img = img.convertDepth(32);
    dest.reset();
    dest.create(img.width(), img.height(), 32);
    dest.setAlphaBuffer(true);
    unsigned int *data = (unsigned int *)img.bits();
    unsigned int *destData = (unsigned int*)dest.bits();
    int total = img.width()*img.height();
    int current;
    int delta;
    int destR, destG, destB, alpha;
    int srcR = c.red()+20;
    int srcG = c.green()+20;
    int srcB = c.blue()+20;
    float srcPercent, destPercent;
    for(current=0; current < total; ++current){
        alpha = qAlpha(data[current]);
        delta = 255-qRed(data[current]);
        destR = srcR-delta;
        destG = srcG-delta;
        destB = srcB-delta;
        if(destR < 0) destR = 0;
        if(destG < 0) destG = 0;
        if(destB < 0) destB = 0;
        if(destR > 255) destR = 255;
        if(destG > 255) destG = 255;
        if(destB > 255) destB = 255;

        if(alpha != 255 && alpha !=0){
            srcPercent = ((float)alpha)/255.0;
            destPercent = 1.0-srcPercent;
            destR = (int)((srcPercent*destR) + (destPercent*bgColor.red()));
            destG = (int)((srcPercent*destG) + (destPercent*bgColor.green()));
            destB = (int)((srcPercent*destB) + (destPercent*bgColor.blue()));
            alpha = 255;
        }
        destData[current] = qRgba(destR, destG, destB, alpha);
    }
}

void LiquidStyleConfigPrefColors::slotUseCustomColorClicked(bool en)
{
    radioOnLbl->setEnabled(en);
    radioOffLbl->setEnabled(en);
    cbOnLbl->setEnabled(en);
    cbOffLbl->setEnabled(en);
    tabOnLbl->setEnabled(en);
    tabOffLbl->setEnabled(en);
    sbSliderLbl->setEnabled(en);
    sbGrooveLbl->setEnabled(en);

    radioOnColorBtn->setEnabled(en);
    radioOffColorBtn->setEnabled(en);
    cbOnColorBtn->setEnabled(en);
    cbOffColorBtn->setEnabled(en);
    tabOnColorBtn->setEnabled(en);
    tabOffColorBtn->setEnabled(en);
    sbSliderColorBtn->setEnabled(en);
    sbGrooveColorBtn->setEnabled(en);
    emit changed(true);
}

void LiquidStyleConfigPrefColors::slotUsePanelCustomClicked(bool on)
{
    panelColorBtn->setEnabled(on);
    emit changed(true);
}

void LiquidStyleConfigPrefMisc::slotSliderChanged(int)
{
    emit changed(true);
}

void LiquidStyleConfigPrefColors::slotPanelColorChanged(const QColor &)
{
    emit changed(true);
}

void LiquidStyleConfigPrefMisc::slotColorOptionChanged()
{
    emit changed(true);
}

void LiquidStyleConfigPrefMenu::slotColorOptionChanged()
{
    emit changed(true);
}

void LiquidStyleConfigPrefMenu::slotBtnGroupClicked(int id){
    editGroup->setEnabled(id == Custom);
    emit changed(true);
}

void LiquidStyleConfig::defaults(){
    load();
    if(!m_pageTwo->useCustomColorBtn->isOn()){
        m_pageTwo->radioOnColorBtn->setColor(colorGroup().button().rgb());
        m_pageTwo->radioOffColorBtn->setColor(colorGroup().background().rgb());
        m_pageTwo->cbOnColorBtn->setColor(colorGroup().button().rgb());
        m_pageTwo->cbOffColorBtn->setColor(colorGroup().background().rgb());
        m_pageTwo->tabOnColorBtn->setColor(colorGroup().button().rgb());
        m_pageTwo->tabOffColorBtn->setColor(colorGroup().background().rgb());
        m_pageTwo->sbSliderColorBtn->setColor(colorGroup().button().rgb());
        m_pageTwo->sbGrooveColorBtn->setColor(colorGroup().background().rgb());
    }
    emit changed(true);
}

void LiquidStyleConfig::load() {
    QSettings settings;

    int id = settings.readNumEntry("/liquid/LiquidMenus/Type", m_pageOne->StippledBg);
    m_pageOne->btnGroup->setButton(id);
    m_pageOne->colorBtn->setColor(QColor(settings.readNumEntry("/liquid/LiquidMenus/Color", (int)colorGroup().button().rgb())));
    m_pageOne->fgBtn->setColor(QColor(settings.readNumEntry("/liquid/LiquidMenus/TextColor", (int)colorGroup().text().rgb())));
    //opacitySlider->setValue(settings.readNumEntry("/liquid/LiquidMenus/Opacity", 10));
    m_pageOne->shadowBtn->setChecked(settings.readBoolEntry("/liquid/LiquidMenus/ShadowText", true));

    m_pageThree->stippleBgBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/BgStipple", true));
    m_pageThree->stipplePanelBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/PanelBgStipple", true));
    m_pageThree->reverseColorBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/ReverseBtnColor", true));
    m_pageThree->animProgressBar->setChecked(settings.readBoolEntry("/liquid/Liquid/AnimateProgressBar", true));
    m_pageThree->stippleContrastSlider->setValue(settings.readNumEntry("/liquid/Liquid/StippleContrast", 3));
    m_pageThree->useTbFrameBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/ToolButtonFrame", false));

    m_pageTwo->usePanelCustomBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/CustomPanelColor", false));
    if(m_pageTwo->usePanelCustomBtn->isOn())
        m_pageTwo->panelColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/PanelColor", (int)colorGroup().button().rgb())));
    else
        m_pageTwo->panelColorBtn->setColor(colorGroup().button().rgb());
    
    m_pageTwo->useCustomColorBtn->setChecked(settings.readBoolEntry("/liquid/Liquid/UseCustomColors", false));
    m_pageTwo->radioOnColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/RadioOnColor", (int)colorGroup().button().rgb())));
    m_pageTwo->radioOffColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/RadioOffColor", (int)colorGroup().background().rgb())));
    m_pageTwo->cbOnColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/CheckBoxOnColor", (int)colorGroup().button().rgb())));
    m_pageTwo->cbOffColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/CheckBoxOffColor", (int)colorGroup().background().rgb())));
    m_pageTwo->tabOnColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/TabOnColor", (int)colorGroup().button().rgb())));
    m_pageTwo->tabOffColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/TabOffColor", (int)colorGroup().background().rgb())));
    m_pageTwo->sbSliderColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/SBSliderColor", (int)colorGroup().button().rgb())));
    m_pageTwo->sbGrooveColorBtn->setColor(QColor(settings.readNumEntry("/liquid/Liquid/SBGrooveColor", (int)colorGroup().background().rgb())));
    m_pageTwo->slotUseCustomColorClicked(m_pageTwo->useCustomColorBtn->isChecked());
    m_pageTwo->slotWidgetColorChanged(colorGroup().background());
    emit changed(false);
}

void LiquidStyleConfig::save(){
    QSettings *settings = new QSettings;
    int id = m_pageOne->btnGroup->id(m_pageOne->btnGroup->selected());
    settings->writeEntry("/liquid/LiquidMenus/Type", id);
    settings->writeEntry("/liquid/LiquidMenus/ShadowText", m_pageOne->shadowBtn->isOn());
    if(id == m_pageOne->Custom){
        settings->writeEntry("/liquid/LiquidMenus/Color", (int)m_pageOne->colorBtn->color().rgb());
        settings->writeEntry("/liquid/LiquidMenus/TextColor", (int)m_pageOne->fgBtn->color().rgb());
      //  settings->writeEntry("/liquid/LiquidMenus/Opacity", opacitySlider->value());
    }

    settings->writeEntry("/liquid/Liquid/BgStipple", m_pageThree->stippleBgBtn->isOn());
    settings->writeEntry("/liquid/Liquid/PanelBgStipple", m_pageThree->stipplePanelBtn->isOn());
    settings->writeEntry("/liquid/Liquid/StippleContrast", m_pageThree->stippleContrastSlider->value());
    settings->writeEntry("/liquid/Liquid/ReverseBtnColor", m_pageThree->reverseColorBtn->isOn());
    settings->writeEntry("/liquid/Liquid/AnimateProgressBar", m_pageThree->animProgressBar->isOn());
    settings->writeEntry("/liquid/Liquid/ToolButtonFrame", m_pageThree->useTbFrameBtn->isOn());
        
    settings->writeEntry("/liquid/Liquid/CustomPanelColor", m_pageTwo->usePanelCustomBtn->isOn());
    if(m_pageTwo->usePanelCustomBtn->isOn()){
        settings->writeEntry("/liquid/Liquid/PanelColor", (int)m_pageTwo->panelColorBtn->color().rgb());
    }

    settings->writeEntry("/liquid/Liquid/UseCustomColors", m_pageTwo->useCustomColorBtn->isOn());

    if(m_pageTwo->useCustomColorBtn->isOn()){
        settings->writeEntry("/liquid/Liquid/RadioOnColor", (int)m_pageTwo->radioOnColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/RadioOffColor", (int)m_pageTwo->radioOffColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/CheckBoxOnColor", (int)m_pageTwo->cbOnColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/CheckBoxOffColor", (int)m_pageTwo->cbOffColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/TabOnColor", (int)m_pageTwo->tabOnColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/TabOffColor", (int)m_pageTwo->tabOffColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/SBSliderColor", (int)m_pageTwo->sbSliderColorBtn->color().rgb());
        settings->writeEntry("/liquid/Liquid/SBGrooveColor", (int)m_pageTwo->sbGrooveColorBtn->color().rgb());
    }

    delete settings; // force sync
    KIPC::sendMessageAll(KIPC::StyleChanged, 0);
    emit changed(false);
}
