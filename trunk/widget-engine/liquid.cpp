
#include "liquid.h"
#include "bitmaps.h"
#include "embeddata.h"
//#include "htmlmasks.h"
#include <qrangecontrol.h>
#include <qpalette.h>
#include <qtimer.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

static const int windowsItemFrame		=  1; // menu item frame width
static const int windowsSepHeight		=  2; // separator item height
static const int windowsItemHMargin		=  3; // menu item hor text margin
static const int windowsItemVMargin		=  1; // menu item ver text margin
static const int windowsArrowHMargin		=  6; // arrow horizontal margin
static const int windowsTabSpacing		= 12; // space between text and tab
static const int windowsCheckMarkHMargin	=  2; // horiz. margins of check mark
static const int windowsRightBorder		= 12; // right border on windows
static const int windowsCheckMarkWidth		= 12; // checkmarks width on windows

class LiquidStylePlugin : public QStylePlugin
{
public:
    LiquidStylePlugin(){;}
    ~LiquidStylePlugin(){;}

    QStringList keys() const {return(QStringList() << "Liquid+");}
    QStyle *create(const QString &);
};

QStyle *LiquidStylePlugin::create(const QString &s)
{
    if(s.lower() == "liquid+")
        return(new LiquidStyle());
    return(0);
}

Q_EXPORT_PLUGIN( LiquidStylePlugin )

static const int itemFrame       = 1;
static const int itemHMargin     = 3;
static const int itemVMargin     = 1;
static const int arrowHMargin    = 6;
static const int rightBorder     = 12;

OptionHandler::OptionHandler(QObject *parent) : QObject(parent){
    fillPixmap = NULL;
    menusProcessed = false;
    pixDict.setAutoDelete(true);
    reloadSettings();
}

const QColor& OptionHandler::textColor()
{
    return(type == Custom ? fgColor : qApp->palette().active().text());
}

const QColor& OptionHandler::bgColor()
{
    LiquidStyle *style = (LiquidStyle*)parent();
    if(type == None || type == StippledBg)
        return(style->isKicker ? style->origPanelBrush.color() : qApp->palette().active().background());
    else if(type == StippledBtn)
        return(qApp->palette().active().button());
    return(color);
}

void OptionHandler::reloadSettings()
{
    pixDict.clear();
    QSettings settings;
    type = settings.readNumEntry("/liquid/LiquidMenus/Type", StippledBg);
    color = QColor(settings.readNumEntry("/liquid/LiquidMenus/Color", 0));
     if(type == Custom)
         fgColor = QColor(settings.readNumEntry("/liquid/LiquidMenus/TextColor",0));
    shadowText = settings.readBoolEntry("/liquid/LiquidMenus/ShadowText", true);
    bgStipple = settings.readBoolEntry("/liquid/Liquid/BgStipple", true);
    panelStipple = settings.readBoolEntry("/liquid/Liquid/PanelBgStipple", true);
    contrast = settings.readNumEntry("/liquid/Liquid/StippleContrast", 3);
    reverseBtnColor = settings.readBoolEntry("/liquid/Liquid/ReverseBtnColor",true);
    animateProgressBar = settings.readBoolEntry("/liquid/Liquid/AnimateProgressBar",true);
    panelCustom = settings.readBoolEntry("/liquid/Liquid/CustomPanelColor",false);
    if(panelCustom)
        customPanelColor = QColor(settings.readNumEntry("/liquid/Liquid/PanelColor",0));
    tbFrame = settings.readBoolEntry("/liquid/Liquid/ToolButtonFrame", false);
    customWidgetColor = settings.readBoolEntry("/liquid/Liquid/UseCustomColors",false);
    if(customWidgetColor){
        customColors[CustomRadioOn].setRgb(settings.readNumEntry("/liquid/Liquid/RadioOnColor", (int)bgColor().rgb()));
        customColors[CustomRadioOff].setRgb(settings.readNumEntry("/liquid/Liquid/RadioOffColor",  (int)bgColor().rgb()));
        customColors[CustomCBOn].setRgb(settings.readNumEntry("/liquid/Liquid/CheckBoxOnColor", (int)bgColor().rgb()));
        customColors[CustomCBOff].setRgb(settings.readNumEntry("/liquid/Liquid/CheckBoxOffColor", (int)bgColor().rgb()));
        customColors[CustomTabOn].setRgb(settings.readNumEntry("/liquid/Liquid/TabOnColor", (int)bgColor().rgb()));
        customColors[CustomTabOff].setRgb(settings.readNumEntry("/liquid/Liquid/TabOffColor", (int)bgColor().rgb()));
        customColors[CustomSBSlider].setRgb(settings.readNumEntry("/liquid/Liquid/SBSliderColor", (int)bgColor().rgb()));
        customColors[CustomSBGroove].setRgb(settings.readNumEntry("/liquid/Liquid/SBGrooveColor", (int)bgColor().rgb()));
    }
}

void OptionHandler::prepareMenus()
{
    if(menusProcessed)
        return;

    if(fillPixmap)
        delete fillPixmap;
    fillPixmap = NULL;
    if(type == StippledBg || type == StippledBtn){
        QColor c(bgColor());
        fillPixmap = new QPixmap(32, 32);
        fillPixmap->fill(c.rgb());
        int i;
        QPainter painter;
        painter.begin(fillPixmap);
        painter.setPen(c.dark(103));
        for(i=0; i < 32; i+=4){
            painter.drawLine(0, i, 32, i);
            //painter.drawLine(0, i+1, 32, i+1);
        }
        painter.end();
    }
    menusProcessed = true;
}

bool OptionHandler::eventFilter(QObject *obj, QEvent *ev)
{
    QPopupMenu *p = (QPopupMenu *)obj;

    if(ev->type() == QEvent::ApplicationPaletteChange){
        if(type == StippledBg || type == StippledBtn){
            prepareMenus();
            QBrush brush(bgColor(), *bgPixmap());
            QPalette pal = p->palette();
            pal.setBrush(QColorGroup::Background, brush);
            p->setPalette(pal);
        }
        else if(type == None){
            prepareMenus();
            QBrush brush(bgColor());
            QPalette pal = p->palette();
            pal.setBrush(QColorGroup::Background, brush);
            p->setPalette(pal);
        }
    }
    return(false);
}


LiquidStyle::LiquidStyle() : KStyle(AllowMenuTransparency,ThreeButtonScrollBar)
{
    headerHoverID = -1;
    highlightWidget = NULL;
    currentTaskContainer = NULL;
    taskContainerHover = false;
    isTaskContainer = false;
    isHTMLButton = false;
    inExitPolish = false;
    btnDict.setAutoDelete(true);
    btnShadowedDict.setAutoDelete(true);
    bevelFillDict.setAutoDelete(true);
    smallBevelFillDict.setAutoDelete(true);
    initialPaletteLoaded = false;
    qtrcModificationTime = 0;

    tooltipPalette = qApp->palette();
    tooltipPalette.setBrush(QColorGroup::Background, QColor(255, 255, 220));
    tooltipPalette.setBrush(QColorGroup::Foreground, Qt::black);

    rMatrix.rotate(90.0);
    iMatrix.rotate(180.0);
    btnBorderImg = new QImage(liquid_findImage("button-base"));
    if(btnBorderImg->depth() < 32)
        *btnBorderImg = btnBorderImg->convertDepth(32);
    btnShadowImg = new QImage(liquid_findImage("button-shadow"));
    if(btnShadowImg->depth() < 32)
        *btnShadowImg = btnShadowImg->convertDepth(32);

    bevelFillPix = new QPixmap;
    bevelFillPix->convertFromImage(liquid_findImage("clear_fill_large"));

    smallBevelFillPix = new QPixmap;
    smallBevelFillPix->convertFromImage(liquid_findImage("clear_fill_small"));

    menuPix = NULL;
    tmpBtnPix = NULL;
    progAnimShift = 0;

    optionHandler = new OptionHandler(this);
    if (optionHandler->useanimateProgressBar()){
        QTimer* timer = new QTimer( this );
        timer->start(50, false);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateProgressPos()));
    }

    int i;
    for(i=0; i < BITMAP_ITEMS; ++i){
        pixmaps[i] = NULL;
    }

    sbLeft = new QBitmap(7, 7, sbarrow_left_bits, true);
    sbLeft->setMask(*sbLeft);
    sbRight = new QBitmap(7, 7, sbarrow_right_bits, true);
    sbRight->setMask(*sbRight);
    sbUp = new QBitmap(7, 7, sbarrow_up_bits, true);
    sbUp->setMask(*sbUp);
    sbDown = new QBitmap(7, 7, sbarrow_down_bits, true);
    sbDown->setMask(*sbDown);

    isKicker = (qstrcmp(qApp->argv()[0], "kicker") == 0) ||    (qstrcmp(qApp->argv()[0], "appletproxy") == 0);
}

LiquidStyle::~LiquidStyle()
{
    btnDict.clear();
    btnShadowedDict.clear();
    bevelFillDict.clear();
    smallBevelFillDict.clear();

    delete optionHandler;

    delete sbUp;
    delete sbDown;
    delete sbLeft;
    delete sbRight;

    if(btnBorderImg)
        delete btnBorderImg;
    if(btnShadowImg)
        delete btnShadowImg;

    if(tmpBtnPix)
        delete tmpBtnPix;

    if(bevelFillPix)
        delete bevelFillPix;

    if(smallBevelFillPix)
        delete smallBevelFillPix;

    if(menuPix)
        delete menuPix;

    int i;
    for(i=0; i < BITMAP_ITEMS; ++i){
        if(pixmaps[i])
            delete pixmaps[i];
    }
}

bool LiquidStyle::isPlain() const
{
    return(!optionHandler->useBgStipple());
}
//taken from Baghira . Thank Thomas if u like the animation
inline void LiquidStyle::updateProgressPos()
{
    progAnimShift++;
    if (progAnimShift == 20)    progAnimShift = 0;
}
//reimplemented for server side translucency
void LiquidStyle::renderMenuBlendPixmap( KPixmap& pix, const QColorGroup& /*cg*/, const QPopupMenu* /*popup*/ ) const
{
    //we have to paint the menu background here
    QColor c;
    c = optionHandler->bgColor();
    pix.fill(c.rgb());
    if(optionHandler->transType() == StippledBg || optionHandler->transType() == StippledBtn){
         int i;
         QPainter painter;
         painter.begin(&pix);
         painter.setPen(c.dark(110));
         for(i=0; i < pix.height(); i+=4){
                 painter.drawLine(0, i, pix.width(), i);
                 //painter.drawLine(0, i+1, 32, i+1);
         }
         painter.end();
    }
}

void LiquidStyle::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r, const QColorGroup &cg, SFlags flags, const QStyleOption &opt) const
{
    bool down = flags & Style_Down;
    bool on = flags & Style_On;

    switch(pe){
    case PE_TabBarBase:{
        p->setPen(cg.button().dark(158));
        p->drawLine(r.x(), r.y(), r.right()-1, r.y());
        p->drawLine(r.x(), r.y(), r.x(), r.bottom());
        p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());
        p->setPen(cg.button().dark(110));
        p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
        p->drawLine(r.x()+1, r.y()+3, r.right()-1, r.y()+3);
        p->setPen(cg.button().light(110));
        p->drawLine(r.x()+1, r.y()+2, r.right()-1, r.y()+2);
        break;
    }
    case PE_ButtonCommand:
    case PE_ButtonDefault:{
        bool sunken = on || down;
        bool hover = flags & Style_MouseOver;
        bool focused = flags & Style_HasFocus;
        QColor newColor;
        if(!optionHandler->useReverseBtnColor() || cg.button() != qApp->palette().active().button())
            newColor = sunken ? cg.button().dark(110) : hover ? cg.button().light(110) :
                focused ? cg.background() : cg.button();
        else{
            newColor = sunken ? cg.button().dark(110) : hover ? cg.button() : focused ? cg.button() : cg.background();

        }
        if(!isPlain() && !isHTMLButton)
            drawRoundButton(p, cg, newColor, cg.background(), r.x(), r.y(), r.width(), r.height(), !isHTMLButton, sunken, false, isHTMLButton, btnOffset.x(),
                            btnOffset.y());
        else
            drawRoundButton(p, cg, newColor, cg.background(), r.x(), r.y(), r.width(), r.height(), !isHTMLButton, sunken, false, isHTMLButton);
        break;
    }
    case PE_ButtonBevel:
    case PE_ButtonDropDown:{
        bool sunken = on || down;
        bool hover = flags & Style_MouseOver;
        drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), sunken ? cg.button().dark(110) : hover ? cg.button() : cg.background(), cg.background());
        break;
    }
    case PE_ButtonTool:{
        bool sunken = on || down;
        bool hover = flags & Style_MouseOver;
        drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), sunken ? cg.button().dark(110) : hover ? cg.button() : cg.background(), cg.background());
        break;
    }
    case PE_HeaderSection:{
        bool sunken = on || down;
        bool hover = flags & Style_MouseOver;
        if(isTaskContainer){
            drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), sunken ? cg.background() : taskContainerHover ?
                           cg.background().light(110) : origPanelBrush.color(), cg.background());
            const_cast<LiquidStyle*>(this)->isTaskContainer = false;
            const_cast<LiquidStyle*>(this)->taskContainerHover = false;
        }
        else{
            if(currentHeader == p->device()){
                int id = currentHeader->sectionAt(r.x());
                hover = id != -1 && id == headerHoverID;
            }
            drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), sunken ? cg.button().dark(110) : hover ? cg.button() : cg.background(), cg.background());
        }
        break;
    }
    case PE_FocusRect:{
        p->drawWinFocusRect(r);
        break;
    }
    case PE_ScrollBarSlider:
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:{
        QColor sbBgColor = optionHandler->useCustomColors() ? optionHandler->customColor(CustomSBGroove) :
            isKicker ? origPanelBrush.color() : qApp->palette().active().background();

        QPixmap *bgPix = bevelFillDict.find(sbBgColor.rgb());
        if(!bgPix){
            bgPix = new QPixmap(*bevelFillPix);
            adjustHSV(*bgPix, sbBgColor);
            const_cast<LiquidStyle*>(this)->bevelFillDict.insert(sbBgColor.rgb(), bgPix);
        }
        bool isHover = highlightWidget == currentScrollBar;
        if(flags & Style_Horizontal){
            int extent = currentScrollBar->height();
            QRect bgR(extent, 0, currentScrollBar->width()-extent*3+1, extent);
            if(sbBuffer.size() != currentScrollBar->size())
                const_cast<LiquidStyle*>(this)->sbBuffer.resize(currentScrollBar->size());
            QPainter painter;
            painter.begin(&sbBuffer);
            painter.setPen(sbBgColor.light(105));
            painter.drawLine(bgR.x(), bgR.y()+1, bgR.x()+8, bgR.y()+1);
            painter.setPen(sbBgColor.dark(110));
            painter.drawLine(bgR.x(), bgR.y()+13, bgR.x()+8, bgR.y()+13);
            painter.drawPixmap(bgR.x(), bgR.y()+2, *bgPix, bgR.x(), bgR.y()+2, 8, 11);
            painter.drawPixmap(bgR.x(), bgR.y()+1, *getPixmap(HSBSliderBtmBg));
            painter.drawTiledPixmap(bgR.x()+8, bgR.y()+1, bgR.width()-16, 13, *getPixmap(HSBSliderMidBg));
            painter.setPen(sbBgColor.light(105));
            painter.drawLine(bgR.right()-8, bgR.y()+1, bgR.right(), bgR.y()+1);
            painter.setPen(sbBgColor.dark(110));
            painter.drawLine(bgR.right()-8, bgR.y()+13, bgR.right(), bgR.y()+13);
            painter.drawPixmap(bgR.right()-8, bgR.y()+2, *bgPix, 0, 0, 8, 11);
            painter.drawPixmap(bgR.right()-8, bgR.y()+1,*getPixmap(HSBSliderTopBg));
            if(pe == PE_ScrollBarSlider){
                painter.drawPixmap(r.x(), r.y()+1,isHover ?*getPixmap(HSBSliderBtmHover) :*getPixmap(HSBSliderBtm));
                painter.drawTiledPixmap(r.x()+8, r.y()+1, r.width()-16,13, isHover ?*getPixmap(HSBSliderMidHover) :*getPixmap(HSBSliderMid));
                painter.drawPixmap(r.right()-8, r.y()+1, isHover ?*getPixmap(HSBSliderTopHover) : *getPixmap(HSBSliderTop));
            }
            //painter.setPen(cg.background().dark(110));
            painter.setPen(sbBgColor.dark(150));
            painter.drawLine(bgR.x(), bgR.y(), bgR.right(), bgR.y());
            painter.drawLine(bgR.x(), bgR.bottom(), bgR.right(),
                             bgR.bottom());
            painter.end();
        }
        else{
            int extent = currentScrollBar->width();
            QRect bgR(0, extent, extent, currentScrollBar->height()-extent*3+1);

            if(sbBuffer.size() != currentScrollBar->size())
                const_cast<LiquidStyle*>(this)->sbBuffer.resize(currentScrollBar->size());
            QPainter painter;
            painter.begin(&sbBuffer);
            painter.setPen(sbBgColor.light(105));
            painter.drawLine(bgR.x()+1, bgR.y(), bgR.x()+1, bgR.y()+8);
            painter.setPen(sbBgColor.dark(110));
            painter.drawLine(bgR.x()+13, bgR.y(), bgR.x()+13, bgR.y()+8);
            painter.drawPixmap(bgR.x()+2, bgR.y(), *bgPix, bgR.x(), bgR.y(), 11, 8);
            painter.drawPixmap(bgR.x()+1, bgR.y(), *getPixmap(VSBSliderTopBg));
            painter.drawTiledPixmap(bgR.x()+1, bgR.y()+8, 13, bgR.height()-16, *getPixmap(VSBSliderMidBg));

            painter.setPen(sbBgColor.light(105));
            painter.drawLine(bgR.x()+1, bgR.bottom()-8, bgR.x()+1, bgR.bottom());
            painter.setPen(sbBgColor.dark(110));
            painter.drawLine(bgR.x()+13, bgR.bottom()-8, bgR.x()+13, bgR.bottom());
            painter.drawPixmap(bgR.x()+2, bgR.bottom()-8, *bgPix, 0, 0, 11, 8);
            painter.drawPixmap(bgR.x()+1, bgR.bottom()-8, *getPixmap(VSBSliderBtmBg));
            if(pe == PE_ScrollBarSlider){
                painter.drawPixmap(r.x()+1, r.y(), isHover ? *getPixmap(VSBSliderTopHover): *getPixmap(VSBSliderTop));
                painter.drawTiledPixmap(r.x()+1, r.y()+8, 13, r.height()-16, isHover ? *getPixmap(VSBSliderMidHover) : *getPixmap(VSBSliderMid));
                painter.drawPixmap(r.x()+1, r.bottom()-8, isHover ? *getPixmap(VSBSliderBtmHover) : *getPixmap(VSBSliderBtm));
            }
            //painter.setPen(cg.background().dark(110));
            painter.setPen(sbBgColor.dark(150));
            painter.drawLine(bgR.x(), bgR.y(), bgR.x(), bgR.bottom());
            painter.drawLine(bgR.right(), bgR.y(), bgR.right(), bgR.bottom());
            painter.end();
        }
        p->drawPixmap(r.x(), r.y(), sbBuffer, r.x(), r.y(), r.width(), r.height());
        break;
    }
    case PE_ScrollBarAddLine:{
        QColor c(optionHandler->useCustomColors() ? optionHandler->customColor(CustomSBGroove) : isKicker ? origPanelBrush.color() :
                 qApp->palette().active().background());
        QColor pixColor = flags & Style_Down ? qApp->palette().active().button() : c;
        QPixmap *pix = bevelFillDict.find(pixColor.rgb());
        if(!pix){
            pix = new QPixmap(*bevelFillPix);
            adjustHSV(*pix, pixColor);
            const_cast<LiquidStyle*>(this)->bevelFillDict.insert(pixColor.rgb(), pix);
        }
        p->setPen(c.dark(150));
        if(flags & Style_Horizontal){
            p->drawLine(r.x(), r.y(), r.right(), r.y());
            p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
            p->drawLine(r.right(), r.y(), r.right(), r.bottom());
            p->setPen(c.light(105));
            p->drawLine(r.x(), r.y()+1, r.right()-1, r.y()+1);
            p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);
            p->setPen(c.dark(110));
            p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
            p->drawLine(r.x(), r.bottom()-1, r.right()-1, r.bottom()-1);
            p->drawPixmap(r.x()+1, r.y()+2, *pix, 8, r.y()+2, r.width()-3, r.height()-4);
        }
        else{
            p->drawLine(r.x(), r.y(), r.x(), r.bottom());
            p->drawLine(r.right(), r.y(), r.right(), r.bottom());
            p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
            p->setPen(c.light(105));
            p->drawLine(r.x()+1, r.y(), r.right()-1, r.y());
            p->drawLine(r.x()+1, r.y(), r.x()+1, r.bottom()-1);
            p->setPen(c.dark(110));
            p->drawLine(r.right()-1, r.y(), r.right()-1, r.bottom()-1);
            p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
            p->drawPixmap(r.x()+2, r.y()+1, *pix, r.x()+2, 8,
                          r.width()-4, r.height()-3);
        }
        p->setPen(flags & Style_Down ? Qt::white : Qt::black);
        p->drawPixmap(r.x()+4, r.y()+4, flags & Style_Horizontal ?
                      *sbRight : *sbDown);
        break;
    }
    case PE_ScrollBarSubLine:{
        QColor c(optionHandler->useCustomColors() ? optionHandler->customColor(CustomSBGroove) :
                 isKicker ? origPanelBrush.color() : qApp->palette().active().background());
        QColor pixColor = flags & Style_Down ? qApp->palette().active().button() : c;
        QPixmap *pix = bevelFillDict.find(pixColor.rgb());
        if(!pix){
            pix = new QPixmap(*bevelFillPix);
            adjustHSV(*pix, pixColor);
            const_cast<LiquidStyle*>(this)->bevelFillDict.insert(pixColor.rgb(), pix);
        }
        bool top = flags & Style_Horizontal ? r.x() == 0 : r.y() == 0;
        p->setPen(c.dark(150));
        if(flags & Style_Horizontal){
            if(top){
                p->drawLine(r.x(), r.y(), r.x(), r.bottom());
                p->drawLine(r.x(), r.y(), r.right(), r.y());
                p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
                p->setPen(c.light(105));
                p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
                p->drawLine(r.x()+1, r.y()+1, r.right(), r.y()+1);
                p->setPen(c.dark(110));
                p->drawLine(r.x()+1, r.bottom()-1, r.right(), r.bottom()-1);
                p->drawPixmap(r.x()+2, r.y()+2, *pix, r.x()+2, r.y()+2, r.width()-2, r.height()-4);
            }
            else{
                p->drawLine(r.x(), r.y(), r.right(), r.y());
                p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
                p->setPen(c.dark(110));
                p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);
                p->setPen(c.light(105));
                p->drawLine(r.x(), r.y()+1, r.right()-1, r.y()+1);
                p->setPen(c.dark(110));
                p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
                p->drawLine(r.x(), r.bottom()-1, r.right()-1, r.bottom()-1);
                p->drawPixmap(r.x(), r.y()+2, *pix, 8, r.y()+2, r.width()-2, r.height()-4);
            }

        }
        else{
            if(top){
                p->drawLine(r.x(), r.y(), r.right(), r.y());
                p->drawLine(r.x(), r.y(), r.x(), r.bottom());
                p->drawLine(r.right(), r.y(), r.right(), r.bottom());
                p->setPen(c.light(105));
                p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
                p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom());
                p->setPen(c.dark(110));
                p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom());
                p->drawPixmap(r.x()+2, r.y()+2, *pix, r.x()+2, r.y()+2, r.width()-4, r.height()-2);
            }
            else{
                p->drawLine(r.x(), r.y(), r.x(), r.bottom());
                p->drawLine(r.right(), r.y(), r.right(), r.bottom());
                p->setPen(c.dark(110));
                p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
                p->setPen(c.light(105));
                p->drawLine(r.x()+1, r.y(), r.x()+1, r.bottom()-1);
                p->setPen(c.dark(110));
                p->drawLine(r.right()-1, r.y(), r.right()-1, r.bottom()-1);
                p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
                p->drawPixmap(r.x()+2, r.y(), *pix,r.x()+2, 8, r.width()-4, r.height()-2);
            }
        }
        p->setPen(flags & Style_Down ? Qt::white : Qt::black);
        p->drawPixmap(r.x()+4, r.y()+4, flags & Style_Horizontal ? *sbLeft : *sbUp);
        break;
    }
    case PE_Indicator:{
        bool hover = flags & Style_MouseOver;
        bool isMasked = p->device() && p->device()->devType() == QInternal::Widget && ((QWidget *)p->device())->autoMask();
        if(isMasked){
            if(!(flags & Style_Off))
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(HTMLCBDownHover) : *getPixmap(HTMLCBDown));
            else
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(HTMLCBHover) : *getPixmap(HTMLCB));
        }
        else{
            if(!(flags & Style_Off))
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(CBDownHover) : *getPixmap(CBDown));
            else
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(CBHover) : *getPixmap(CB));
        }
        break;
    }
    case PE_IndicatorMask:{
        p->fillRect(r, Qt::color1);
        /*p->setPen(Qt::color0);
        p->drawPoint(r.x(), r.y());
        p->drawPoint(r.right(), r.y());
        p->drawPoint(r.x(), r.bottom());
        p->drawPoint(r.right(), r.bottom());*/
        //p->setPen(Qt::color1);
        //p->drawPixmap(r.x(), r.y(), *getPixmap(CB)->mask());
        break;
    }
    case PE_ExclusiveIndicator:{
        bool hover = flags & Style_MouseOver;;
        bool isMasked = p->device() && p->device()->devType() == QInternal::Widget && ((QWidget *)p->device())->autoMask();
        if(isMasked){
            if(on || down)
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(HTMLRadioOnHover) : *getPixmap(HTMLRadioOn));
            else
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(HTMLRadioOffHover) : *getPixmap(HTMLRadioOff));
        }
        else{
            if(on || down)
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(RadioOnHover) : *getPixmap(RadioOn));
            else
                p->drawPixmap(r.x(), r.y(), hover ? *getPixmap(RadioOffHover) : *getPixmap(RadioOff));
        }
        break;
    }
    case PE_ExclusiveIndicatorMask:{
        p->setPen(Qt::color1);
        p->drawPixmap(r.x(), r.y(), *getPixmap(HTMLRadioOn)->mask());
        break;
    }
    case PE_DockWindowResizeHandle:
    case PE_Splitter:{
        drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), highlightWidget == p->device() ? cg.button().dark(110) : cg.button(), cg.background());
        break;
    }
    /*case PE_GroupBoxFrame:{
        QColor light1(cg.background().light(120));
        QColor dark1(cg.background().dark(110));
        QColor dark2(cg.background().dark(115));
        QColor dark3(cg.background().dark(130));

        p->setPen(dark1);
        p->drawPoint(r.x(), r.y());
        p->drawPoint(r.right()-1, r.y());
        p->drawPoint(r.x(), r.bottom()-1);
        p->drawPoint(r.right()-1, r.bottom()-1);

        p->setPen(dark3);
        p->drawLine(r.x()+1, r.y(), r.right()-2, r.y());
        p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-2);
        p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-2);
        p->drawLine(r.x()+1, r.bottom()-1, r.right()-2, r.bottom()-1);

        p->setPen(dark2);
        p->drawLine(r.x()+1, r.bottom(), r.right(), r.bottom());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());

        p->setPen(light1);
        p->drawLine(r.x()+1, r.y()+1, r.right()-2, r.y()+1);
        p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-2);
        break;
    }*/
    case PE_Panel:{
	bool sunken  = flags & Style_Sunken;
        int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth) : opt.lineWidth();
        QPen oldPen(p->pen());
        if(lw == 1){
            QColor light(cg.background().light(120));
            QColor dark(cg.background().dark(140));
            QColor mid(cg.background().dark(120));
            p->setPen(mid);
            p->drawPoint(r.x(), r.bottom());
            p->drawPoint(r.right(), r.y());
 
            p->setPen(sunken ? dark : light);
            p->drawLine(r.x(), r.y(), r.right()-1, r.y());
            p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);
            p->setPen(sunken ? light : dark);
            p->drawLine(r.x()+1, r.bottom(), r.right(), r.bottom());
            p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);
        }
        else if(lw == 2){
            QColor light1, light2, light3, dark1, dark2, dark3;
            QColor mid(cg.background().dark(110));
            if(!sunken){
                light1 = cg.background().light(101);
                light2 = cg.background().light(120);
                light3 = cg.background().light(110);
                dark1 = cg.background().dark(115);
                dark2 = cg.background().dark(130);
                dark3 = cg.background().dark(110);
            }
            else{
                light1 = cg.background().dark(115);
                light2 = cg.background().dark(130);
                light3 = cg.background().dark(110);
                dark1 = cg.background().light(101);
                dark2 = cg.background().light(120);
                dark3 = cg.background().light(110);
            }

            p->setPen(light1);
            p->drawPoint(r.x(), r.y());
            p->setPen(dark1);
            p->drawPoint(r.right(), r.bottom());
            p->setPen(mid);
            p->drawPoint(r.x(), r.bottom());
            p->drawPoint(r.right(), r.y());

            p->setPen(light2);
            p->drawLine(r.x()+1, r.y(), r.right()-1, r.y());
            p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);

            p->setPen(light3);
            p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
            p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);

            p->setPen(dark2);
            p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
            p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);

            p->setPen(dark3);
            p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
            p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
        }
        else
            KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
        p->setPen(oldPen);
        break;
    }
    case PE_PanelTabWidget:{
        p->setPen(cg.background().dark(140));
        p->drawRect(r);
        p->setPen(cg.background());
        p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
        p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
        p->setPen(cg.background().light(130));
        p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
        p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
        break;
    }
    case PE_PanelLineEdit:{
        bool isHTML = p->device() && p->device()->devType() == QInternal::Widget && isHTMLWidget((QWidget *)p->device());
        drawEditFrame(p, r, cg, isHTML);
        break;
    }
    case PE_PanelPopup:{
        int x, y, w ,h;
        r.rect(&x, &y, &w, &h);

        optionHandler->prepareMenus();
        QColor c(optionHandler->bgColor());
        p->setPen(c.dark(140));
        p->drawRect(x, y, w, h);
        p->setPen(c.light(120));
        p->drawRect(x+1, y+1, w-2, h-2);
        break;
    }
    case PE_PanelMenuBar:{
        if(p->device() && p->device()->devType() == QInternal::Widget){
            QWidget *w = (QWidget *)p->device();
            if(w->backgroundMode() == PaletteButton) w->setBackgroundMode(PaletteBackground);
            if(w->isTopLevel())
                qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), cg, false, 1, &cg.brush(QColorGroup::Background));
        }
    }
    case PE_PanelDockWindow:{
        // This method draws toolbars
        /*if(p->device() && p->device()->devType() == QInternal::Widget){
            // check if background brush is set properly - Qt sometimes
            // resets it to PaletteButton even after my polish() sets it
            // to PaletteBackground. I think they do it just to mess with
            // my head ;-)
            QWidget *w = (QWidget *)p->device();
            if(w->backgroundMode() != Qt::PaletteBackground){
                w->setBackgroundMode(Qt::PaletteBackground);
                w->setBackgroundOrigin(QWidget::WindowOrigin);
                w->erase();
            }
        }*/
        break;
    }
    case PE_DockWindowSeparator:{
        if(!(flags & Style_Horizontal)){
            p->setPen(cg.mid());
            p->drawLine(4, r.height()/2, r.width()-5, r.height()/2);
            p->setPen(cg.light());
            p->drawLine(4, r.height()/2+1, r.width()-5, r.height()/2+1);
        }
        else{
            p->setPen(cg.mid());
            p->drawLine(r.width()/2, 4, r.width()/2, r.height()-5);
            p->setPen(cg.light());
            p->drawLine(r.width()/2+1, 4, r.width()/2+1, r.height()-5);
        }
        break;
    }
    case PE_SpinWidgetUp:
    case PE_SpinWidgetDown:{
        bool sunken = on || down;
        //bool hover = flags & Style_MouseOver;;
        int x = r.x() + (r.width()-7)/2;
        int y = r.y() + (r.height()-7)/2;
        QPen oldPen(p->pen());
        p->setPen(sunken ? Qt::white : cg.buttonText());
        p->drawPixmap(x, y, pe == PE_SpinWidgetUp ? *sbUp : *sbDown);
        p->setPen(oldPen);
        break;
    }

    default:{
        if(pe >= PE_ArrowUp && pe <= PE_ArrowLeft){
            QPen oldPen(p->pen());
            // arrows
            static const QCOORD u_arrow[]={3,1, 4,1, 2,2, 5,2, 1,3, 6,3, 0,4, 7,4, 0,5, 7,5};
            static const QCOORD d_arrow[]={0,2, 7,2, 0,3, 7,3, 1,4, 6,4, 2,5, 5,5, 3,6, 4,6};
            static const QCOORD l_arrow[]={1,3, 1,4, 2,2, 2,5, 3,1, 3,6, 4,0, 4,7, 5,0, 5,7};
            static const QCOORD r_arrow[]={2,0, 2,7, 3,0, 3,7, 4,1, 4,6, 5,2, 5,5, 6,3, 6,4};
            p->setPen(flags & Style_Enabled ? flags & Style_Down  || flags & Style_Sunken ?
                      cg.light() : cg.buttonText() : cg.mid());
            int x = r.x();
            int y = r.y();
            if(r.width() > 8){
                x = x + (r.width()-8)/2;
                y = y + (r.height()-8)/2;
            }
            QPointArray a;
            switch(pe){
            case PE_ArrowUp:
                a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
                break;
            case PE_ArrowDown:
                a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
                break;
            case PE_ArrowLeft:
                a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
                break;
            default:
                a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
                break;
            }
            a.translate(x, y);
            p->drawLineSegments(a);
            p->setPen(oldPen);
        }
        else
            KStyle::drawPrimitive( pe, p, r, cg, flags, opt);
    }
    }
}

void LiquidStyle::drawKStylePrimitive(KStylePrimitive kpe, QPainter* p,const QWidget* widget, const QRect &r,
                                      const QColorGroup &cg, SFlags flags,const QStyleOption &opt) const
{
    switch(kpe){
    case KPE_ToolBarHandle:{
        drawClearBevel(p, r.x(), r.y(), r.width(), r.height(), cg.button(), cg.background());
        /*
         int x = r.x();
        int y = r.y();
        int x2 = r.x() + r.width()-1;
        int y2 = r.y() + r.height()-1;
        p->setPen(cg.button().dark(120));
        p->drawLine(x+1, y, x2-1, y);
        p->drawLine(x+1, y2, x2-1, y2);
        p->drawLine(x, y+1, x, y2-1);
        p->drawLine(x2, y+1, x2, y2-1);
        p->setPen(cg.background());
        p->drawPoint(x, y);
        p->drawPoint(x2, y);
        p->drawPoint(x, y2);
        p->drawPoint(x2, y2);
        QPixmap *pix = bevelFillDict.find(cg.button().rgb());
        if(!pix){
            pix = new QPixmap(*bevelFillPix);
            adjustHSV(*pix, cg.button());
            const_cast<LiquidStyle*>(this)->
                bevelFillDict.insert(cg.button().rgb(), pix);
        }
        p->drawTiledPixmap(x+1, y+1, r.width()-2, r.height()-2, *pix);
        */
        break;
    }
    case KPE_GeneralHandle:{
        optionHandler->prepareMenus();
        p->fillRect(r.x(), r.y(), r.width(), r.height(),cg.brush(QColorGroup::Background));
        if(widget->inherits("AppletHandleDrag") && optionHandler->usePanelCustomColor()){
            QColor c(optionHandler->panelCustomColor());
            drawClearBevel(p, r.x(), r.y(), r.width(), r.height(),highlightWidget == widget ? c.light(110) : c, c);
        }
        else
            drawClearBevel(p, r.x(), r.y(), r.width(), r.height(),highlightWidget == widget ? cg.button().light(110) : cg.button(), cg.button());
        break;
    }
    case KPE_SliderGroove:{
        QColor c = widget->hasFocus() ? cg.background().dark(120) :
            cg.background();
        int x, y, w, h;
        r.rect(&x, &y, &w, &h);
        if(((const QSlider *)widget)->orientation() == Qt::Horizontal){
            int x2 = x+w-1;
            y = y+(h-5)/2;
            p->setPen(c.dark(130));
            p->drawLine(x+1, y, x2-1, y);
            p->setPen(c.dark(150));
            p->drawLine(x, y+1, x2, y+1);
            p->setPen(c.dark(125));
            p->drawLine(x, y+2, x2, y+2);
            p->setPen(c.dark(130));
            p->drawLine(x, y+3, x2, y+3);
            p->setPen(c.dark(120));
            p->drawLine(x, y+4, x2, y+4);
            p->setPen(c.light(110));
            p->drawLine(x+1, y+5, x2-1, y+5);
        }
        else{
            int y2 = y+h-1;
            x = x+(w-5)/2;
            p->setPen(c.dark(130));
            p->drawLine(x, y+1, x, y2-1);
            p->setPen(c.dark(150));
            p->drawLine(x+1, y, x+1, y2);
            p->setPen(c.dark(125));
            p->drawLine(x+2, y, x+2, y2);
            p->setPen(c.dark(130));
            p->drawLine(x+3, y, x+3, y2);
            p->setPen(c.dark(120));
            p->drawLine(x+4, y, x+4, y2);
            p->setPen(c.light(110));
            p->drawLine(x+5, y+1, x+5, y2-1);
        }
        break;
    }
    case KPE_SliderHandle:{
        p->drawPixmap(r.x(), r.y(), ((const QSlider *)widget)->orientation() == Qt::Horizontal ? *getPixmap(HSlider) : *getPixmap(VSlider));
        break;
    }
    case KPE_ListViewExpander:{
        int radius = (r.width() - 4) / 2;
        int centerx = r.x() + r.width()/2;
        int centery = r.y() + r.height()/2;

        int red, green, blue;
        red = (cg.dark().red() >> 1) + (cg.base().red() >> 1);
        green = (cg.dark().green() >> 1) + (cg.base().green() >> 1);
        blue = (cg.dark().blue() >> 1) + (cg.base().blue() >> 1);
        QColor blendColor(red, green, blue);

        p->setPen(cg.dark());
        p->drawLine(r.x()+1, r.y(), r.right()-1, r.y());
        p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
        p->drawLine(r.x(), r.y()+1, r.x(), r.bottom()-1);
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-1);
        p->setPen(blendColor);
        p->drawPoint(r.x(), r.y());
        p->drawPoint(r.right(), r.y());
        p->drawPoint(r.x(), r.bottom());
        p->drawPoint(r.right(), r.bottom());
        p->setPen(cg.light());
        p->drawRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
        p->fillRect(r.x()+2, r.y()+2, r.width()-4, r.height()-4,
                    cg.background());
        p->setPen(cg.text());
        p->drawLine(centerx - radius, centery, centerx + radius, centery );
        if(flags & Style_On)
            p->drawLine(centerx, centery - radius, centerx, centery + radius);
        break;
    }
    default:
        KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
    }
}

void LiquidStyle::drawControl(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r,
                              const QColorGroup &cg, SFlags flags,const QStyleOption &opt) const {                                                          
    
    switch (element){
    case CE_ProgressBarGroove:{
        p->setPen(cg.background().dark(150));
        p->drawRect(r);
        p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2, cg.base());
        break;
    }
    case CE_ProgressBarContents:{
        const QProgressBar *progress = (const QProgressBar *)widget;
        QRect contentsR(subRect(SR_ProgressBarContents, widget));
        double val = progress->progress();
        bool reverse = QApplication::reverseLayout();
        val = val/progress->totalSteps();
        if(val > 0.0){
            int w = QMIN(contentsR.width(), (int)(val * contentsR.width()));
            if(w > 1){
                int x = contentsR.x();
                int y = contentsR.y();
                int x2 = contentsR.x()+w-1;
                int y2 = contentsR.bottom();
                QRect progressRect;
                if (reverse)
                    progressRect = QRect(contentsR.x()+(contentsR.width()-w), contentsR.y(), w, contentsR.height()+1);
                else
                progressRect = QRect(contentsR.x(), contentsR.y(), w, contentsR.height()+1);
                //Clip to the old rectangle
                p->setClipRect(progressRect, QPainter::CoordPainter);
                //Expand the paint region slightly to get the animation offset.
                progressRect.setLeft(progressRect.x() - 20 + progAnimShift);
                p->setPen(cg.button().dark(120));
                p->drawLine(x, y, x2, y);
                p->drawLine(x, y, x, y2);
                //if(optionHandler->useanimateProgressBar())                     p->drawLine(x+1, y2+1, x2-1, y2+1);
                p->setPen(cg.button().dark(110));
                p->drawLine(x2, y, x2, y2);
                p->drawLine(x, y2, x2, y2);
                QPixmap * pix = getPixmap(Progress);
                p->drawTiledPixmap(/*x+1, y+1, w-2, contentsR.height()-2*/progressRect, *pix);
            }
        }
        break;
    }
    case CE_ProgressBarLabel:{
        QRect contentsR(subRect(SR_ProgressBarContents, widget));
        if(!contentsR.isValid())
            return;
        QFont font(p->font());
        font.setBold(true);
        p->setFont(font);
        p->setPen(widget->colorGroup().buttonText());
        p->drawText(contentsR, AlignCenter, ((const QProgressBar *)widget)->progressString());
        break;
    }
    case CE_TabBarTab:{
        //this fix was the hardest to spot  ... without Thomas i would have spend months on it ... thank u buddy :)
        if(!widget || !widget->parentWidget())
            break;
        const QTabBar *tabBar = (const QTabBar *)widget;
        bool selected = flags & Style_Selected;
        QPixmap *pix;
        bool above = tabBar->shape() == QTabBar::RoundedAbove || tabBar->shape() == QTabBar::TriangularAbove;
        if(!above){
            //KStyle::drawControl(element, p, widget, r, cg, flags, opt);
            pix = selected ? getPixmap(belowTabDown) : getPixmap(belowTab);
        }
        else pix = selected ? getPixmap(TabDown) : getPixmap(Tab);
        QPixmap tilePix;
        p->drawPixmap(r.x(), above ? r.y() : r.y()-8, *pix, 0, 0, 9, pix->height());
        p->drawPixmap(r.right()-9, above ? r.y() : r.y()-8, *pix, pix->width()-9, 0, 9, pix->height());
        tilePix.resize(pix->width()-18, pix->height());
        bitBlt(&tilePix, 0, 0, pix, 9, 0, pix->width()-18, pix->height());
        p->drawTiledPixmap(r.x()+9, above ? r.y() : r.y()-8, r.width()-18, pix->height(), tilePix);
        QColor c = tabBar->colorGroup().button();
        if(!selected){
            p->setPen(c.dark(158));
            p->drawLine(r.x(), above ? r.bottom() : r.top(), r.right(), above ? r.bottom() : r.top());
        }
        break;    
    }
    case CE_TabBarLabel:{
        if(opt.isDefault())
            return;
        QTab *t = opt.tab();
        if((flags & Style_HasFocus) && !t->text().isEmpty()){
            const QColor tmp(cg.button().dark(130));
            drawItem(p, QRect(r.x()+1, r.y()+1, r.width(), r.height()),AlignCenter | ShowPrefix, cg, flags & Style_Enabled, 0, t->text(), -1, &tmp);
            drawItem(p, QRect(r.x()+2, r.y()+2, r.width(), r.height()),AlignCenter | ShowPrefix, cg, flags & Style_Enabled, 0, t->text(), -1, &tmp);
        }
        drawItem(p, r, AlignCenter | ShowPrefix, cg, flags & Style_Enabled,0, t->text());
        break;
    }
    case CE_PushButton:{
        const QPushButton *btn = (const QPushButton*)widget;
        const_cast<LiquidStyle*>(this)->isHTMLButton = isHTMLWidget(btn);
        if(widget == highlightWidget)
            flags |= Style_MouseOver;
        // Qt messes this up with WindowOrigin, so we do it ourselves :P
        if(!isPlain() && !btn->autoMask())
            const_cast<LiquidStyle*>(this)->btnOffset = btn->backgroundOffset();

        if(btn->isDefault())
            drawPrimitive(PE_ButtonDefault, p, r, cg, flags);
        else
            drawPrimitive(PE_ButtonCommand, p, r, cg, flags);
        break;
    }
    case CE_PushButtonLabel:{
        const QPushButton *btn = (const QPushButton*)widget;
        int x, y, w, h;
        r.rect(&x, &y, &w, &h);
        bool highlighted = btn->isOn() || btn->isDown() || btn->hasFocus();

        if(btn->isOn() || btn->isDown()){
            ++x;
            ++y;
            flags |= Style_Sunken;
        }
        if(btn->isDefault()){
            int tmpX = x+4;
            int tmpY = y+(h-11)/2;
            p->setPen(highlighted ? cg.background().dark(140) :
                      cg.button().dark(140));
            p->drawLine(tmpX, tmpY, tmpX, tmpY+11);
            p->drawLine(tmpX, tmpY+11, tmpX+5, tmpY+6);
            p->setPen(highlighted ? cg.background().dark(125) :
                      cg.button().dark(125));
            p->drawLine(tmpX, tmpY, tmpX+5, tmpY+5);
            x += 8;
            w -= 8;
        }
        if(btn->isMenuButton()){
            int ih = r.height() / 3;
            int iw = ih;
            int ix = r.width() - iw - 7;
            int iy = (r.height() - ih) / 2;
            drawPrimitive(PE_ArrowDown, p, QRect(ix, iy, iw, ih) , cg, flags,
                          opt);
            w -= iw;
        }
        if(btn->iconSet() && !btn->iconSet()->isNull()){
            QIconSet::Mode mode = btn->isEnabled() ? QIconSet::Normal :QIconSet::Disabled;
            if(mode == QIconSet::Normal && btn->hasFocus())
                mode = QIconSet::Active;

            QIconSet::State state = QIconSet::Off;
            if(btn->isToggleButton() && btn->isOn())
                state = QIconSet::On;

            QPixmap pix(btn->iconSet()->pixmap(QIconSet::Small, mode, state));
            int pixW = pix.width();
            int pixH = pix.height();
            p->drawPixmap(x+2, y+(h-pixH)/2, pix);
            x += pixW+4;
            w -= pixW+4;
        }

        QColor tmp;
        if(highlighted){
            tmp = cg.background().dark(130);
            drawItem(p, QRect(x+1, y+1, w, h), AlignCenter | ShowPrefix, cg, btn->isEnabled(), btn->pixmap(), btn->text(), -1, &tmp);
            drawItem(p, QRect(x+2, y+2, w, h), AlignCenter | ShowPrefix, cg, btn->isEnabled(), btn->pixmap(), btn->text(), -1, &tmp);
        }
        tmp = btn->colorGroup().buttonText();
        drawItem(p, QRect(x, y, w, h), AlignCenter | ShowPrefix, cg, btn->isEnabled(), btn->pixmap(), btn->text(), -1, &tmp);
        break;
    }
    case CE_MenuBarItem:{
        QMenuItem *mi = opt.menuItem();
        bool active  = flags & Style_Active;
        bool focused = flags & Style_HasFocus;

        if(active && focused){
            drawClearBevel(p, r.x(), r.y(), r.width(), r.height(),
                           qApp->palette().active().button(), cg.background());
        }
        QColor tmpColor(cg.background().dark(120));
        if(optionHandler->useShadowText())
        {
                drawItem(p, QRect(r.x()+1, r.y()+1, r.width(), r.height()), AlignCenter | AlignVCenter | ShowPrefix | DontClip |
                        SingleLine, cg, flags & Style_Enabled, mi->pixmap(), mi->text(), -1, &tmpColor);
        }
        drawItem(p, r, AlignCenter | AlignVCenter | ShowPrefix | DontClip | SingleLine, cg, flags & Style_Enabled, mi->pixmap(), mi->text());
        break;
    }
    case CE_PopupMenuItem:{
        int x, y, w, h;
        r.rect( &x, &y, &w, &h );
        // Don't leave blank holes if we set NoBackground for the QPopupMenu.
        // This only happens when the popupMenu spans more than one column.
        if (! ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ) )
                p->fillRect( r, cg.background().light( 105 ) );
        
        const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
        QMenuItem *mi = opt.menuItem();

        int tab = opt.tabWidth();
        int maxpmw = opt.maxIconWidth();
        bool dis = mi && !mi->isEnabled();
        bool checkable = popupmenu->isCheckable();
        bool act = flags & Style_Active;
        int checkcol = maxpmw;

        optionHandler->prepareMenus();
        QColor menuBgColor(optionHandler->bgColor());

        if(checkable)
            checkcol = QMAX(maxpmw, 20);

        if(mi && mi->isSeparator()){
            p->setPen(menuBgColor.dark(130));
            p->drawLine(x, y, x+w, y);
            p->setPen(menuBgColor.light(120));
            p->drawLine(x, y+1, x+w, y+1);
            return;
        }

        if(act && mi)
            drawClearBevel(p, x, y, w, h, cg.button(), cg.background());
        // Draw the transparency pixmap
        else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ){
            p->drawPixmap( x, y , *widget->erasePixmap(), x,y, w, h );    
            //p->drawWinFocusRect( r );
        }
        else{
            p->fillRect(x, y, w, h, menuBgColor);
        }
        if(!mi)
            return;

        // Menu contents drawing code based on Qt's styles. Qt is property of
        // TrollTech: www.trolltech.com. Used with permission.
        int xpos = x;
        QRect vrect = visualRect(QRect(xpos, y, checkcol, h), r);
        int xvis = vrect.x();

        // checkbox
        if(mi->isChecked()){
            if(act && !dis)
                qDrawShadePanel(p, xvis, y, checkcol, h, cg, TRUE, 1, &cg.brush(QColorGroup::Button));
            else{
                qDrawShadePanel(p, xvis, y, checkcol, h, cg, TRUE, 1);
            }
        }

        // icon
        if(mi->iconSet()){
            QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
            if(act && !dis)
                mode = QIconSet::Active;
            QPixmap pixmap;
            if(checkable && mi->isChecked())
                pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode, QIconSet::On);
            else
                pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            QRect pmr(0, 0, pixw, pixh);
            pmr.moveCenter(vrect.center());
            p->setPen(cg.text());
            p->drawPixmap(pmr.topLeft(), pixmap);
        }
        else if(checkable){
            if(mi->isChecked()){
                int xp = xpos + windowsItemFrame;

                SFlags cflags = Style_Default;
                if(!dis)
                    cflags |= Style_Enabled;
                if(act)
                    cflags |= Style_On;
                drawPrimitive(PE_CheckMark, p,
                              visualRect(QRect(xp, y + windowsItemFrame, checkcol - 2*windowsItemFrame,h - 2*windowsItemFrame), r), cg, cflags);
            }
        }
        QColor discol;
        if(dis){
            discol = cg.text();
            p->setPen( discol );
        }

        int xm = windowsItemFrame + checkcol + windowsItemHMargin;
        xpos += xm;

        vrect = visualRect(QRect(xpos, y+windowsItemVMargin,w-xm-tab+1, h-2*windowsItemVMargin), r);
        xvis = vrect.x();
        if(mi->custom()){
            p->save();
            if(dis && !act) {
                p->setPen(cg.light());
                mi->custom()->paint(p, cg, act, !dis,xvis+1, y+windowsItemVMargin+1,w-xm-tab+1, h-2*windowsItemVMargin);
                p->setPen(discol);
            }
            mi->custom()->paint(p, cg, act, !dis, xvis, y+windowsItemVMargin,w-xm-tab+1, h-2*windowsItemVMargin);
            p->restore();
        }

        // Text
        QString s = mi->text();
        if(!s.isNull()){
            QColor textColor, shadowColor;
            shadowColor = optionHandler->bgColor().dark(130);

            if(dis)
                textColor = shadowColor.light(115);
            else if(optionHandler->transType() == Custom)
                textColor = optionHandler->textColor();
            else
                textColor = cg.text();

            int t = s.find( '\t' );
            int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
            text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft);
            if(t >= 0){                         // draw tab text
                int xp;
                xp = x + w - tab - 20 - windowsItemHMargin - windowsItemFrame + 1;
                QString tabStr(s.right(s.length()-t-1));
                if(!tabStr.isEmpty()){
                    if(optionHandler->useShadowText()){
                        p->setPen(shadowColor);
                        p->drawText(xp+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin, text_flags,tabStr);
                    }
                    p->setPen(textColor);
                    p->drawText(xp, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin, text_flags,tabStr);
                }
                s = s.left(t);
            }
            if(optionHandler->useShadowText()){
                p->setPen(shadowColor);
                p->drawText(xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin, text_flags, s, t);
            }
            p->setPen(textColor);
            p->drawText(xvis, y+windowsItemVMargin, w-xm-tab+1,
                        h-2*windowsItemVMargin, text_flags, s, t);
        }
        else if(mi->pixmap()){                        // draw pixmap
            QPixmap *pixmap = mi->pixmap();
            if(pixmap->depth() == 1)
                p->setBackgroundMode(OpaqueMode);
            p->drawPixmap(xvis, y+windowsItemFrame, *pixmap);
            if(pixmap->depth() == 1)
                p->setBackgroundMode(TransparentMode);
        }

        // Child menu
        if(mi->popup()){
            int dim = (h-2*windowsItemFrame) / 2;
            PrimitiveElement arrow;
            arrow = (QApplication::reverseLayout() ? PE_ArrowLeft :
                     PE_ArrowRight);
            xpos = x+w - windowsArrowHMargin - windowsItemFrame - dim;
            vrect = visualRect(QRect(xpos, y + h / 2 - dim / 2, dim, dim), r);
            if(act){
                if(!dis)
                    discol = white;
                QColorGroup g2(discol, cg.highlight(),
                               white, white,
                               dis ? discol : white,
                               discol, white);

                drawPrimitive(arrow, p, vrect, g2, Style_Enabled);
            } else {
                drawPrimitive(arrow, p, vrect, cg, mi->isEnabled() ? Style_Enabled : Style_Default);
            }
        }
        break;
    }
    default:
        KStyle::drawControl(element, p, widget, r, cg, flags, opt);
    }
}


void LiquidStyle::drawControlMask(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r, const QStyleOption& opt) const {
    switch(element){
    case CE_PushButton:{
        p->fillRect(r, Qt::color1);
        break;
    }
    default:
        KStyle::drawControlMask(element, p, widget, r, opt);
        break;
    }
}

void LiquidStyle::drawComplexControlMask(ComplexControl control, QPainter *p, const QWidget *widget, const QRect &r,
                                         const QStyleOption &opt ) const {
    switch (control){
    case CC_ComboBox:{
        drawControlMask(CE_PushButton, p, widget, r, opt);
        break;
    }
    default:
        KStyle::drawComplexControlMask(control, p, widget, r, opt);
    }
}

void LiquidStyle::drawComplexControl(ComplexControl control, QPainter *p,
                                     const QWidget *widget, const QRect &r,
                                     const QColorGroup &cg, SFlags flags,
                                     SCFlags controls, SCFlags active,
                                     const QStyleOption &opt) const
{
    switch(control){
    case CC_ComboBox:{
        QPen oldPen(p->pen());
        if(controls & SC_ComboBoxArrow){
            if(isHTMLWidget(widget)){
                drawRectangularButton(p, cg.button(), r.x(), r.y(),
                                      r.width(), r.height(),
                                      active == SC_ComboBoxArrow,
                                      widget == highlightWidget, true);
                return;
            }
            bool sunken = (active == SC_ComboBoxArrow);
            bool hover = widget == highlightWidget;
            int w = r.width()+1; // cut off 1 pixel edge
            int h = r.height();
            LiquidStyle *ptr = const_cast<LiquidStyle*>(this);
            QColor c(hover || sunken ? cg.button() : cg.background());
            ButtonTile *tile = btnShadowedDict.find(c.rgb());

            if(!tile){
                tile = createButtonTile(c, cg.background(), false);
            }
            if(!tile){
                qWarning("Combo Button tile is NULL!");
                return;
            }

            if(!tmpBtnPix)
                ptr->tmpBtnPix = new QPixmap(w, h);
            else if(w > tmpBtnPix->width() || h > tmpBtnPix->height()){
                // make temp pixmap size == largest button
                delete tmpBtnPix;
                ptr->tmpBtnPix = new QPixmap(w, h);
            }

            QPainter painter;
            painter.begin(tmpBtnPix);

            QPixmap *stipple = cg.brush(QColorGroup::Background).pixmap();
            if(!stipple) // button may have custom colorgroup
                stipple = qApp->
                    palette().active().brush(QColorGroup::Background).pixmap();
            if(!isPlain() && stipple){
                QPoint point(widget->backgroundOffset());
                painter.drawTiledPixmap(0, 0, w, h, *stipple,
                                        point.x(), point.y());
            }
            else
                painter.fillRect(0, 0, w, h, cg.background());

            if(hover || sunken){
                painter.drawPixmap(0, 0, *tile->pixmap(TileTopLeft));
                painter.drawTiledPixmap(10, 0, w-22, 10, *tile->pixmap(TileTop));
                painter.drawPixmap(w-12, 0, *tile->pixmap(TileTopRight));

                painter.drawTiledPixmap(0, 10, 10, h-22, *tile->pixmap(TileLeft));
                painter.drawTiledPixmap(10, 10, w-22, h-22, *tile->pixmap(TileMiddle));
                painter.drawTiledPixmap(w-12, 10, w-22, h-22, *tile->pixmap(TileRight));

                painter.drawPixmap(0, h-12, *tile->pixmap(TileBtmLeft));
                painter.drawTiledPixmap(10, h-12, w-22, 12, *tile->pixmap(TileBtm));
                painter.drawPixmap(w-12, h-12, *tile->pixmap(TileBtmRight));
            }
            else{
                painter.drawPixmap(0, 0, *tile->pixmap(TileTopLeft));
                painter.drawTiledPixmap(10, 0, w-22, 10, *tile->pixmap(TileTop));

                painter.drawTiledPixmap(0, 10, 10, h-22, *tile->pixmap(TileLeft));
                painter.drawTiledPixmap(10, 10, w-22, h-22, *tile->pixmap(TileMiddle));

                painter.drawPixmap(0, h-12, *tile->pixmap(TileBtmLeft));
                painter.drawTiledPixmap(10, h-12, w-22, 12, *tile->pixmap(TileBtm));

                ButtonTile *tile = btnShadowedDict.find(cg.button().rgb());
                if(!tile){
                    tile = createButtonTile(cg.button(), cg.background(), false);
                }
                if(!tile){
                    qWarning("Combo Button tile is NULL!");
                    return;
                }

                painter.drawTiledPixmap(w-19, 0, 7, 10, *tile->pixmap(TileTop));
                painter.drawTiledPixmap(w-19, 10, 7, h-22, *tile->pixmap(TileMiddle));
                painter.drawTiledPixmap(w-19, h-12, 7, 12, *tile->pixmap(TileBtm));
        
                painter.drawPixmap(w-12, 0, *tile->pixmap(TileTopRight));
                painter.drawTiledPixmap(w-12, 10, w-22, h-22, *tile->pixmap(TileRight));
                painter.drawPixmap(w-12, h-12, *tile->pixmap(TileBtmRight));

            }
            int ah = r.height() / 3;
            int aw = ah;
            int ax = r.width() - aw - 7;
            int ay = (r.height() - ah) / 2;
            // Are we enabled?
            if (widget->isEnabled())
                flags |= Style_Enabled;
            // Are we "pushed" ?
            if(active & Style_Sunken)
                flags |= Style_Sunken;
            drawPrimitive(PE_ArrowDown, &painter, QRect(ax, ay, aw, ah), cg,
                          flags);
            painter.end();
            p->drawPixmap(r.x(), r.y(), *tmpBtnPix, 0, 0,
                          r.width(), r.height());
        }
        if(controls & SC_ComboBoxEditField){
            if(((const QComboBox *)widget)->editable()){
                p->setPen(cg.button().dark(115));
                //p->drawRect(r.x()+9, r.y()+1, r.width()-26, r.height()-3);
                p->drawLine(r.x()+9, r.y()+1, r.x()+9, r.bottom()-3);
                p->drawLine(r.x()+10, r.y()+1, r.right()-18, r.y()+1);
            }
        }
        p->setPen(oldPen);
        break;
    }
    case CC_ToolButton:{
        const QToolButton *toolbutton = (const QToolButton *) widget;

        QRect button, menuarea;
        button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
        menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

        SFlags bflags = flags,
            mflags = flags;

        if(active & SC_ToolButton)
            bflags |= Style_Down;
        if(active & SC_ToolButtonMenu)
            mflags |= Style_Down;

        if(widget == highlightWidget)
            bflags |= Style_MouseOver;

        if(controls & SC_ToolButton){
            bool sunken = mflags & (Style_Down | Style_On);
            bool hover = bflags & Style_MouseOver;
            if(!optionHandler->useToolButtonFrame()){
                if(toolbutton->parent() &&
                   (toolbutton->parent()->inherits("KToolBar") ||
                   toolbutton->parent()->inherits("QToolBar"))){
                    if(hover || sunken)
                        drawClearBevel(p, button.x(), button.y(),button.width(), button.height(),sunken ? cg.button().dark(110) :cg.button(), cg.background());
                }
                else
                    drawClearBevel(p, button.x(), button.y(),button.width(), button.height(),
                                   sunken ? cg.button().dark(110) : hover ?cg.button() : cg.background(),cg.background());
            }
            else
                drawClearBevel(p, r.x(), r.y(), r.width(), r.height(),sunken ? cg.button().dark(110) : hover ?cg.button() : cg.background(),cg.background());
        }
        // Draw a toolbutton menu indicator if required
        if(controls & SC_ToolButtonMenu){
            if(mflags & (Style_Down | Style_On | Style_Raised))
                drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
            drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
        }

        if(toolbutton->hasFocus() && !toolbutton->focusProxy()){
            QRect fr = toolbutton->rect();
            fr.addCoords(3, 3, -3, -3);
            drawPrimitive(PE_FocusRect, p, fr, cg);
        }
        break;
    }
    case CC_ScrollBar:{
        const_cast<LiquidStyle*>(this)->currentScrollBar =(QScrollBar *)widget;
        KStyle::drawComplexControl(control, p, widget, r, cg, flags,controls, active, opt);
        break;
    }
    /*case CC_Slider:{
        QRect groove = querySubControlMetrics(CC_Slider, widget,
                                              SC_SliderGroove, opt);
        QRect handle = querySubControlMetrics(CC_Slider, widget,
                                              SC_SliderHandle, opt);
        if((controls & SC_SliderGroove) && groove.isValid())
            drawKStylePrimitive(KPE_SliderGroove, p, widget, groove, cg,
                                flags, opt);
        if(controls & SC_SliderTickmarks)
            QCommonStyle::drawComplexControl(control, p, widget, r, cg,
                                             flags, SC_SliderTickmarks,
                                             active, opt);
        if((controls & SC_SliderHandle) && handle.isValid())
            drawKStylePrimitive(KPE_SliderHandle, p, widget, handle, cg,
                                flags, opt);
        break;
    }*/
    default:
        KStyle::drawComplexControl(control, p, widget,r, cg, flags, controls, active, opt);
        break;
    }
}

QRect LiquidStyle::subRect(SubRect r, const QWidget *widget) const
{
    return(KStyle::subRect(r, widget));
}

int LiquidStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    switch(m){
    case PM_ButtonMargin:
        return(5);
        break;
    case PM_ButtonDefaultIndicator:
        return(0);
        break;
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return(16);
        break;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return(16);
        break;
    case PM_ScrollBarExtent:
        return(15);
        break;
    case PM_ScrollBarSliderMin:
        return(16);
        break;
    case PM_SliderControlThickness:
    case PM_SliderThickness:
        return(11);
        break;
    case PM_SliderLength:
        return(10);
        break;
    case PM_TabBarTabOverlap:
    case PM_TabBarBaseOverlap:
        return(1);
        break;
    case PM_TabBarTabHSpace:
        return(18);
        break;
    case PM_TabBarTabVSpace:
        return(8);
        break;
    case PM_TabBarBaseHeight:
        return(5);
        break;
    default:
        break;
    }
    return KStyle::pixelMetric(m, widget);
}

QSize LiquidStyle::sizeFromContents(ContentsType contents,const QWidget *widget,const QSize &contentSize,const QStyleOption& opt ) const
{
    switch (contents){
    case CT_PushButton:{
        // this is a little funky - we give values not based on pixelMetric
        // because we want a custom width/height and we can only give one
        // value in pixelMetric (used in sizeHint). Odd but works well
	const QPushButton* button = (const QPushButton*) widget;
	
	int w = contentSize.width() + 26 > 80 ? contentSize.width() + 26 : 80;
	int h = contentSize.height() > 23 ? contentSize.height() : 23;

	if (button->text().isEmpty()) return (QSize(contentSize.width() + 2 * pixelMetric(PM_ButtonMargin, widget), h));
        
        return(QSize(w, h));
    }
    case CT_ComboBox:{
        QSize sz = KStyle::sizeFromContents(contents, widget, contentSize, opt);
        return(QSize(sz.width(), sz.height() > 23 ? sz.height() : 23));
    }
    case CT_PopupMenuItem:{
        if(! widget || opt.isDefault())
            break;

        const QPopupMenu *popup = (const QPopupMenu *) widget;
        bool checkable = popup->isCheckable();
        QMenuItem *mi = opt.menuItem();
        int maxpmw = opt.maxIconWidth();
        int w = contentSize.width();
        int h = contentSize.height();

        if(mi->custom()){
            w = mi->custom()->sizeHint().width();
            h = mi->custom()->sizeHint().height();
            if(!mi->custom()->fullSpan())
                h += 2*windowsItemVMargin + 2*windowsItemFrame;
        }
        else if(mi->widget()){
        }
        else if(mi->isSeparator()){
            w = 10;
            h = windowsSepHeight;
        }
        else{
            if(mi->pixmap())
                h = QMAX(h, mi->pixmap()->height() + 2*windowsItemFrame);
            else if(! mi->text().isNull())
                h = QMAX(h, popup->fontMetrics().height() + 2*windowsItemVMargin + 2*windowsItemFrame);
            if(mi->iconSet() != 0)
                h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,QIconSet::Normal).height() + 2*windowsItemFrame);
        }
        if(!mi->text().isNull() && mi->text().find('\t') >= 0)
            w += windowsTabSpacing;
        else if(mi->popup())
            w += 2*windowsArrowHMargin;
        if(checkable && maxpmw < 20)
            w += 20 - maxpmw;
        if(maxpmw)
            w += maxpmw + 6;
        if(checkable || maxpmw > 0)
            w += windowsCheckMarkHMargin;
        w += 20;
        return(QSize(w, h));
    }
    default:
        break;
    }
    return KStyle::sizeFromContents(contents, widget, contentSize, opt);
}


QPixmap LiquidStyle::stylePixmap(StylePixmap stylepixmap,const QWidget* widget,const QStyleOption& opt) const{
    return KStyle::stylePixmap(stylepixmap, widget, opt);
}

bool LiquidStyle::eventFilter(QObject *obj, QEvent *ev){
    if(KStyle::eventFilter(obj, ev))
        return true;
    if(obj->inherits("QDockWindow")){
        QWidget *w = (QWidget *)obj;
        if(ev->type() == QEvent::Paint){
            QPaintEvent *pev = (QPaintEvent *)ev;
            w->erase(pev->rect());
            return(true);
        }
        else if(w->backgroundMode() != Qt::PaletteBackground){
            // Qt resets the background mode during init. We want to
            // change if back right away because it does an erase before
            // painting which would look ugly in the wrong color and toolbar
            // separators got the wrong palette :P
            w->setBackgroundMode(Qt::PaletteBackground);
            w->setBackgroundOrigin(QWidget::WindowOrigin);
            return(false);
        }
    }
    else if(obj->inherits("QPushButton") || obj->inherits("QComboBox") ||
            obj->inherits("QSplitterHandle") ||
            obj->inherits("AppletHandleDrag")){
        QWidget *btn = (QWidget *)obj;
        if(ev->type() == QEvent::Enter){
            if(btn->isEnabled()){
                highlightWidget = btn;
                btn->repaint(false);
            }
        }
        else if(ev->type() == QEvent::Leave){
            if(btn == highlightWidget){
                highlightWidget = NULL;
                btn->repaint(false);
            }
        }
    }
    else if(obj->inherits("KToolBarButton")){
        QToolButton *btn = (QToolButton *)obj;
        if(ev->type() == QEvent::Enter){
            if(btn->isEnabled()){
                highlightWidget = btn;
                btn->repaint(false);
            }
        }
        else if(ev->type() == QEvent::Leave){
            QWidget *btn = (QWidget *)obj;
            if(btn == highlightWidget){
                highlightWidget = NULL;
                btn->repaint(false);
            }
        }
    }
    else if(obj->inherits("QScrollBar")){
        QScrollBar *sb = (QScrollBar *)obj;
        if(ev->type() == QEvent::Enter){
            if(sb->isEnabled()){
                highlightWidget = sb;
                sb->repaint(false);
            }
        }
        else if(ev->type() == QEvent::Leave){
            if(sb == highlightWidget && !sb->draggingSlider()){
                highlightWidget = NULL;
                sb->repaint(false);
            }
        }
        else if(ev->type() == QEvent::MouseButtonRelease){
            QMouseEvent *me = (QMouseEvent *)ev;
            if(sb == highlightWidget && !sb->rect().contains(me->pos())){
                highlightWidget = NULL;
                sb->repaint(false);
            }
        }
    }
    else if(obj->inherits("TaskContainer")){
        QButton *btn = (QButton *)obj;
        if(ev->type() == QEvent::Enter){
            currentTaskContainer = btn;
            btn->repaint(false);
        }
        else if(ev->type() == QEvent::Leave){
            currentTaskContainer = NULL;
            btn->repaint(true);
        }
        else if(ev->type() == QEvent::Paint){
            isTaskContainer = true;
            if(currentTaskContainer == btn)
                taskContainerHover = true;
            else
                taskContainerHover = false;
        }
    }
    else if(obj->inherits("KMiniPagerButton")){
        QButton *btn = (QButton *)obj;
        if(ev->type() == QEvent::Paint){
            if(pagerBrush.pixmap() == NULL){
                // color scheme has changed
                QColor c(optionHandler->usePanelCustomColor() ? optionHandler->panelCustomColor().dark(110) :
                         qApp->palette().active().button().dark(110));
                QPixmap *pix = smallBevelFillDict.find(c.rgb());
                if(!pix){
                    pix = new QPixmap(*smallBevelFillPix);
                    adjustHSV(*pix, c);
                    smallBevelFillDict.insert(c.rgb(), pix);
                }
                pagerHoverBrush.setColor(c);
                pagerHoverBrush.setPixmap(*pix);

                c = c.dark(115);
                pix = smallBevelFillDict.find(c.rgb());
                if(!pix){
                    pix = new QPixmap(*smallBevelFillPix);
                    adjustHSV(*pix, c);
                    smallBevelFillDict.insert(c.rgb(), pix);
                }
                pagerBrush.setColor(c);
                pagerBrush.setPixmap(*pix);
            }

            if(!(btn->isOn() || btn->isDown())){
                QPalette pal = qApp->palette();
                pal.setBrush(QColorGroup::Dark, btn == highlightWidget ?
                             pagerHoverBrush : pagerBrush);
                btn->setPalette(pal);
            }
            else{
                QPalette pal = qApp->palette();
                pal.setBrush(QColorGroup::Dark,
                             QApplication::palette().active().brush(QColorGroup::Dark));
                btn->setPalette(pal);

            }
        }
        else if(ev->type() == QEvent::Enter){
            highlightWidget = btn;
            btn->repaint(false);
        }
        else if(ev->type() == QEvent::Leave){
            highlightWidget = NULL;
            btn->repaint(false);
        }
    }
    else if(obj->inherits("QLineEdit")){
        if(obj->parent() && obj->parent()->inherits("QComboBox")){
            QWidget *btn = (QWidget *)obj->parent();
            if(ev->type() == QEvent::Enter){
                if (btn->isEnabled()){
                    highlightWidget = btn;
                    btn->repaint(false);
                }
            }
            else if(ev->type() == QEvent::Leave){
                if (btn == highlightWidget)
                    highlightWidget = NULL;
                btn->repaint(false);
            }
        }
    }
    else if(obj->inherits("QRadioButton") || obj->inherits("QCheckBox")){
        QButton *btn = (QButton *)obj;
        bool isStatusChild = btn->parent() &&
            (btn->parent()->inherits("QStatusBar") ||
             btn->parent()->inherits("KonqFrameStatusBar"));
        bool isRadio = obj->inherits("QRadioButton");
        if(ev->type() == QEvent::Paint){
            btn->erase();
            QPainter p;
            p.begin(btn);
            QFontMetrics fm = btn->fontMetrics();
            QSize lsz = fm.size(ShowPrefix, btn->text());
            QSize sz = isRadio ?
                QSize(pixelMetric(PM_ExclusiveIndicatorWidth),
                      pixelMetric(PM_ExclusiveIndicatorHeight)) :
                QSize(pixelMetric(PM_IndicatorWidth),
                      pixelMetric(PM_IndicatorHeight));
            int x = 0;
            int y = isStatusChild ? 0 :
                (btn->height()-lsz.height()+fm.height()-sz.height())/2;
            SFlags flags = Style_Default;
            if (btn->isEnabled())
                flags |= Style_Enabled;
            if (btn->hasFocus())
                flags |= Style_HasFocus;
            if (btn->isDown())
                flags |= Style_Down;
            if (highlightWidget == btn)
                flags |= Style_MouseOver;

            if (btn->state() == QButton::On)
                flags |= Style_On;
            else if (btn->state() == QButton::Off)
                flags |= Style_Off;
            if(isRadio)
                drawControl(CE_RadioButton, &p, btn,
                            QRect(x, y, sz.width(), sz.height()),
                            btn->colorGroup(), flags);
            else
                drawControl(CE_CheckBox, &p, btn,
                            QRect(x, y, sz.width(), sz.height()),
                            btn->colorGroup(), flags);
            x = sz.width() + 6;
            y = 0;
            if(btn->hasFocus()){
                QColor tmp(btn->colorGroup().background().dark(130));
                drawItem(&p, QRect(sz.width()+6+2, 1, btn->width()-(sz.width()+6+1), btn->height()), AlignLeft|AlignVCenter|ShowPrefix,
                         btn->colorGroup(), btn->isEnabled(), btn->pixmap(), btn->text(), -1, &tmp);
                drawItem(&p, QRect(sz.width()+6+3, 2, btn->width()-(sz.width()+6+1), btn->height()), AlignLeft|AlignVCenter|ShowPrefix,
                         btn->colorGroup(), btn->isEnabled(), btn->pixmap(), btn->text(), -1, &tmp);

            }
            drawItem(&p, QRect(sz.width()+6+1, 0, btn->width()-(sz.width()+6+1), btn->height()), AlignLeft|AlignVCenter|ShowPrefix,
                     btn->colorGroup(), btn->isEnabled(), btn->pixmap(), btn->text());
            p.end();
            return(true);
        }
        // for hover, just redraw the indicator (not the text)
        else if((ev->type() == QEvent::Enter && btn->isEnabled()) ||
                (ev->type() == QEvent::Leave && btn == highlightWidget)){
            QButton *btn = (QButton *)obj;
            bool isRadio = obj->inherits("QRadioButton");

            if(ev->type() == QEvent::Enter)
                highlightWidget = btn;
            else 
                highlightWidget = NULL;
            QFontMetrics fm = btn->fontMetrics();
            QSize lsz = fm.size(ShowPrefix, btn->text());
            QSize sz = isRadio ?
                QSize(pixelMetric(PM_ExclusiveIndicatorWidth),
                      pixelMetric(PM_ExclusiveIndicatorHeight)) :
                QSize(pixelMetric(PM_IndicatorWidth),
                      pixelMetric(PM_IndicatorHeight));
            int x = 0;
            int y = isStatusChild ? 0 :
                (btn->height()-lsz.height()+fm.height()-sz.height())/2;
            //if(btn->autoMask())
            //    btn->erase(x+1, y+1, sz.width()-2, sz.height()-2);
            QPainter p;
            p.begin(btn);
            SFlags flags = Style_Default;
            if (btn->isEnabled())
                flags |= Style_Enabled;
            if (btn->hasFocus())
                flags |= Style_HasFocus;
            if (btn->isDown())
                flags |= Style_Down;
            if (highlightWidget == btn)
                flags |= Style_MouseOver;

            if (btn->state() == QButton::On)
                flags |= Style_On;
            else if (btn->state() == QButton::Off)
                flags |= Style_Off;
            if(isRadio)
                drawControl(CE_RadioButton, &p, btn,
                            QRect(x, y, sz.width(), sz.height()),
                            btn->colorGroup(), flags);
            else
                drawControl(CE_CheckBox, &p, btn,
                            QRect(x, y, sz.width(), sz.height()),
                            btn->colorGroup(), flags);
            p.end();
        }
    }
    else if(obj->inherits("QHeader")){
        QHeader *hw = (QHeader *)obj;
        if(ev->type() == QEvent::Enter){
            currentHeader = hw;
            headerHoverID = -1;
        }
        else if(ev->type() == QEvent::Leave){
            currentHeader = NULL;
            if(headerHoverID != -1){
                hw->repaint(hw->sectionPos(headerHoverID), 0,
                            hw->sectionSize(headerHoverID), hw->height());
            }
            headerHoverID = -1;
        }
        else if(ev->type() == QEvent::MouseMove){
            QMouseEvent *me = (QMouseEvent *)ev;
            int oldHeader = headerHoverID;
            headerHoverID = hw->sectionAt(me->x());
            if(oldHeader != headerHoverID){
                // reset old header
                if(oldHeader != -1){
                    hw->repaint(hw->sectionPos(oldHeader), 0,
                                hw->sectionSize(oldHeader), hw->height());
                }
                if(headerHoverID != -1){
                    hw->repaint(hw->sectionPos(headerHoverID), 0,
                                hw->sectionSize(headerHoverID), hw->height());
                }
            }
        }
    }
    else if(obj->inherits("QTipLabel")){
        if(ev->type() == QEvent::PaletteChange ||
           ev->type() == QEvent::ApplicationPaletteChange){
            qWarning("Got tiplabel palette change"); // Debug
            ((QWidget*)obj)->setPalette(tooltipPalette);
            return(true);
        }
    }
    else if(obj->inherits("KActiveLabel") && !isPlain()){
        if(ev->type() == QEvent::Move || ev->type() == QEvent::Resize ||
           ev->type() == QEvent::Show){
            QWidget *w = (QWidget *)obj;
            QPalette pal(w->palette());
            QPixmap *tile = pal.brush(QPalette::Active,
                                      QColorGroup::Background).pixmap();
            if(tile){
                QPoint pos(0, 0);
                pos = w->mapTo(w->topLevelWidget(), pos);
                QSize sz(ev->type() == QEvent::Resize ?
                         ((QResizeEvent*)ev)->size() : w->size());
                QPixmap pix(24, sz.height());
                QPainter p;
                p.begin(&pix);
                p.drawTiledPixmap(0, 0, 24, sz.height(), *tile,
                                  0, pos.y());
                p.end();
                QBrush brush(pal.active().background(), pix);
                pal.setBrush(QColorGroup::Base, brush);
                w->setPalette(pal);
            }
        }
    }
    return(false);
}

QRect LiquidStyle::querySubControlMetrics(ComplexControl control, const QWidget *widget, SubControl subcontrol, const QStyleOption &opt) const {
    if(control == CC_ComboBox && subcontrol == SC_ComboBoxEditField){
        return(QRect(10, 2, widget->width()-28, widget->height()-6));
    }
    else
        return(KStyle::querySubControlMetrics(control, widget, subcontrol, opt));
}

QPixmap* LiquidStyle::processEmbedded(const char *label, const QColor &c,bool blend, const QColor *bg) const {
    QImage img(liquid_findImage(label));
    img.detach();
    if(img.isNull()){ // shouldn't happen, been tested
        qWarning("Invalid embedded label %s", label);
        return(NULL);
    }
    return(adjustHSV(img, c, blend, bg));
}

QPixmap* LiquidStyle::getPixmap(BitmapData item) const {
    QColor bgColor(isKicker ? origPanelBrush.color() : qApp->palette().active().background());
    QColor btnColor(qApp->palette().active().button());
    QColor btnHoverColor(btnColor.light(110));
    QColor sbGrooveColor(optionHandler->useCustomColors() ? optionHandler->customColor(CustomSBGroove) : bgColor);
    QColor sbSliderColor(optionHandler->useCustomColors() ? optionHandler->customColor(CustomSBSlider) : btnColor);

    if(pixmaps[item])
        return(pixmaps[item]);
    LiquidStyle *ptr = const_cast<LiquidStyle*>(this);
    switch(item){
    case RadioOn:
        ptr->pixmaps[RadioOn] = processEmbedded("radio_down",optionHandler->useCustomColors() ?
                                                optionHandler->customColor(CustomRadioOn) : btnColor, true);
        break;
    case RadioOnHover:
        ptr->pixmaps[RadioOnHover] = processEmbedded("radio_down", optionHandler->useCustomColors() ?
                                                     optionHandler->customColor(CustomRadioOn).light(110) : btnHoverColor, true);
        break;
    case RadioOffHover:
        ptr->pixmaps[RadioOffHover] = processEmbedded("radio",optionHandler->useCustomColors() ?optionHandler->customColor(CustomRadioOn) :
                                                      btnColor, true);
        break;
    case TabDown:
        ptr->pixmaps[TabDown] = processEmbedded("tab",optionHandler->useCustomColors() ?optionHandler->customColor(CustomTabOn) :
                                                btnColor, true);
        break;
    case belowTabDown:
        ptr->pixmaps[belowTabDown] = processEmbedded("tab",optionHandler->useCustomColors() ?optionHandler->customColor(CustomTabOn) :
                                                btnColor, true);
        *ptr->pixmaps[belowTabDown] = ptr->pixmaps[belowTabDown]->xForm(iMatrix);
        break;
    case TabFocus:
        ptr->pixmaps[TabFocus] = processEmbedded("tab",optionHandler->useCustomColors() ?optionHandler->customColor(CustomTabOn).light(110) :
                                                 btnHoverColor, true);
        break;
    case CBDown:
        ptr->pixmaps[CBDown] = processEmbedded("checkboxdown", optionHandler->useCustomColors() ?
                                               optionHandler->customColor(CustomCBOn) : btnColor, true);
        break;
    case CBDownHover:
        ptr->pixmaps[CBDownHover] = processEmbedded("checkboxdown", optionHandler->useCustomColors() ?
                                                    optionHandler->customColor(CustomCBOn).light(110) : btnHoverColor, true);
        break;
    case CBHover:
        ptr->pixmaps[CBHover] = processEmbedded("checkbox", optionHandler->useCustomColors() ?
                                                optionHandler->customColor(CustomCBOn) : btnColor, true);
        break;
    case HSlider:
        ptr->pixmaps[HSlider] = processEmbedded("sliderarrow", btnColor, true);
        break;
    case VSlider:
        ptr->pixmaps[VSlider] = processEmbedded("sliderarrow", btnColor, true);
        *ptr->pixmaps[VSlider] = ptr->pixmaps[VSlider]->xForm(rMatrix);
        break;
    case RadioOff:
        ptr->pixmaps[RadioOff] = processEmbedded("radio",optionHandler->useCustomColors() ?
                                                 optionHandler->customColor(CustomRadioOff) : bgColor, true);
        break;
    case Tab:
        ptr->pixmaps[Tab] = processEmbedded("tab",optionHandler->useCustomColors() ? optionHandler->customColor(CustomTabOff) :
                                            bgColor, true);
        break;
    case belowTab:
        ptr->pixmaps[belowTab] = processEmbedded("tab",optionHandler->useCustomColors() ? optionHandler->customColor(CustomTabOff) :
                                            bgColor, true);
        *ptr->pixmaps[belowTab] = ptr->pixmaps[belowTab]->xForm(iMatrix);
        break;
    
    case CB:
        ptr->pixmaps[CB] = processEmbedded("checkbox",optionHandler->useCustomColors() ? optionHandler->customColor(CustomCBOff) 
                                            :bgColor, true);
        break;
    case VSBSliderTop:
        ptr->pixmaps[VSBSliderTop] = processEmbedded("sbslider_top", sbSliderColor, true, &sbGrooveColor);
        break;
    case VSBSliderBtm:
        ptr->pixmaps[VSBSliderBtm] = processEmbedded("sbslider_btm", sbSliderColor, true, &sbGrooveColor);
        break;
    case VSBSliderMid:
        ptr->pixmaps[VSBSliderMid] = processEmbedded("sbslider_mid", sbSliderColor, &sbGrooveColor);
        break;
    case VSBSliderTopHover:
        ptr->pixmaps[VSBSliderTopHover] = processEmbedded("sbslider_top", sbSliderColor.dark(110), true, &sbGrooveColor);
        break;
    case VSBSliderBtmHover:
        ptr->pixmaps[VSBSliderBtmHover] = processEmbedded("sbslider_btm", sbSliderColor.dark(110), true, &sbGrooveColor);
        break;
    case VSBSliderMidHover:
        ptr->pixmaps[VSBSliderMidHover] = processEmbedded("sbslider_mid", sbSliderColor.dark(110), false, &sbGrooveColor);
        break;

    case HSBSliderTop:
        ptr->pixmaps[HSBSliderTop] = processEmbedded("sbslider_top", sbSliderColor, true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderTop] = ptr->pixmaps[HSBSliderTop]->xForm(rMatrix);
        break;
    case HSBSliderBtm:
        ptr->pixmaps[HSBSliderBtm] = processEmbedded("sbslider_btm", sbSliderColor, true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderBtm] = ptr->pixmaps[HSBSliderBtm]->xForm(rMatrix);
        break;
    case HSBSliderMid:
        ptr->pixmaps[HSBSliderMid] = processEmbedded("sbslider_mid", sbSliderColor, false, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderMid] = ptr->pixmaps[HSBSliderMid]->xForm(rMatrix);
        break;
    case HSBSliderTopHover:
        ptr->pixmaps[HSBSliderTopHover] = processEmbedded("sbslider_top", sbSliderColor.dark(110), true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderTopHover] = ptr->pixmaps[HSBSliderTopHover]->xForm(rMatrix);
        break;
    case HSBSliderBtmHover:
        ptr->pixmaps[HSBSliderBtmHover] = processEmbedded("sbslider_btm", sbSliderColor.dark(110), true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderBtmHover] = ptr->pixmaps[HSBSliderBtmHover]->xForm(rMatrix);
        break;
    case HSBSliderMidHover:
        ptr->pixmaps[HSBSliderMidHover] = processEmbedded("sbslider_mid", sbSliderColor.dark(110), false, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderMidHover] = ptr->pixmaps[HSBSliderMidHover]->xForm(rMatrix);
        break;
    case VSBSliderTopBg:
        ptr->pixmaps[VSBSliderTopBg] = processEmbedded("sbgroove_top", sbGrooveColor, true, &sbGrooveColor);
        break;
    case VSBSliderBtmBg:
        ptr->pixmaps[VSBSliderBtmBg] = processEmbedded("sbgroove_btm", sbGrooveColor, true, &sbGrooveColor);
        break;
    case VSBSliderMidBg:
        ptr->pixmaps[VSBSliderMidBg] = processEmbedded("sbgroove_mid", sbGrooveColor, false, &sbGrooveColor);
        break;
    case HSBSliderTopBg:
        ptr->pixmaps[HSBSliderTopBg] = processEmbedded("sbgroove_top", sbGrooveColor, true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderTopBg] = ptr->pixmaps[HSBSliderTopBg]->xForm(rMatrix);
        break;
    case HSBSliderBtmBg:
        ptr->pixmaps[HSBSliderBtmBg] = processEmbedded("sbgroove_btm", sbGrooveColor, true, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderBtmBg] = ptr->pixmaps[HSBSliderBtmBg]->xForm(rMatrix);
        break;
    case HSBSliderMidBg:
        ptr->pixmaps[HSBSliderMidBg] = processEmbedded("sbgroove_mid", sbGrooveColor, false, &sbGrooveColor);
        *ptr->pixmaps[HSBSliderMidBg] = ptr->pixmaps[HSBSliderMidBg]->xForm(rMatrix);
        break;

    case Progress:
        ptr->pixmaps[Progress] = processEmbedded("progress", btnColor);
        break;

    case HTMLRadioOn:
        ptr->pixmaps[HTMLRadioOn] = processEmbedded("htmlradio_down",optionHandler->useCustomColors() ?
                                                    optionHandler->customColor(CustomRadioOn) :btnColor, true);
        break;
    case HTMLRadioOnHover:
        ptr->pixmaps[HTMLRadioOnHover] = processEmbedded("htmlradio_down",optionHandler->useCustomColors() ?
                                                         optionHandler->customColor(CustomRadioOn).light(110) :btnHoverColor, true);
        break;
    case HTMLRadioOff:
        ptr->pixmaps[HTMLRadioOff] = processEmbedded("htmlradio",optionHandler->useCustomColors() ?
                                                     optionHandler->customColor(CustomRadioOff) :bgColor, true);
        break;
    case HTMLRadioOffHover:
        ptr->pixmaps[HTMLRadioOffHover] = processEmbedded("htmlradio",optionHandler->useCustomColors() ?
                                                          optionHandler->customColor(CustomRadioOn) : btnColor, true);
        break;

    case HTMLCBDown:
        ptr->pixmaps[HTMLCBDown] = processEmbedded("checkboxdown", optionHandler->useCustomColors() ?
                                                   optionHandler->customColor(CustomCBOn) :btnColor, true);
        drawHTMLCBBorder(*ptr->pixmaps[HTMLCBDown],optionHandler->useCustomColors() ?
                         optionHandler->customColor(CustomCBOn) : btnColor);
        break;
    case HTMLCBDownHover:
        ptr->pixmaps[HTMLCBDownHover] = processEmbedded("checkboxdown",
                                                        optionHandler->useCustomColors() ?
                                                        optionHandler->customColor(CustomCBOn).light(110) :
                                                        btnHoverColor, true);
        drawHTMLCBBorder(*ptr->pixmaps[HTMLCBDownHover],
                         optionHandler->useCustomColors() ?
                         optionHandler->customColor(CustomCBOn).light(110) :
                         btnHoverColor);
        break;
    case HTMLCBHover:
        ptr->pixmaps[HTMLCBHover] = processEmbedded("checkbox",
                                                    optionHandler->useCustomColors() ?optionHandler->customColor(CustomCBOn) : btnColor, true);
        drawHTMLCBBorder(*ptr->pixmaps[HTMLCBHover],
                         optionHandler->useCustomColors() ?optionHandler->customColor(CustomCBOn) : btnColor);
        break;
    case HTMLCB:
        ptr->pixmaps[HTMLCB] = processEmbedded("checkbox",
                                               optionHandler->useCustomColors() ?
                                               optionHandler->customColor(CustomCBOff) :
                                               bgColor, true);
        drawHTMLCBBorder(*ptr->pixmaps[HTMLCB],
                         optionHandler->useCustomColors() ?
                         optionHandler->customColor(CustomCBOff) :
                         bgColor);
        break;
    default:
        break;
    }
    return(pixmaps[item]);
}

#include "liquid.moc"

