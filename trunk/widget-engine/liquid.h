#ifndef __LIQUID_STYLE_H
#define __LIQUID_STYLE_H

#include <qdrawutil.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qstyleplugin.h>
#include <qbitmap.h>
#include <qintdict.h>
#include <qimage.h>
#include <qpainter.h>
#include <kpixmap.h>
#include <kstyle.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include <kconfig.h>

#include <qwidgetlist.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qsettings.h>
#include <qobjectlist.h>

// various widgets with special handlers
#include <qscrollbar.h>
#include <qcombobox.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qtabbar.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qheader.h>
#include <qtextedit.h>

#define BITMAP_ITEMS 61 // Don't worry, they're demand loaded ;-)

#include <fstream>

class LiquidStyle;

enum BitmapData{RadioOn=0, RadioOff, RadioOnHover, RadioOffHover, VSBSliderTop,
VSBSliderMid, VSBSliderBtm, VSBSliderTopHover, VSBSliderMidHover,
VSBSliderBtmHover, VSBSliderTopBg, VSBSliderMidBg, VSBSliderBtmBg,
HSBSliderTop, HSBSliderMid, HSBSliderBtm, HSBSliderTopHover, HSBSliderMidHover,
HSBSliderBtmHover, HSBSliderTopBg, HSBSliderMidBg,
HSBSliderBtmBg, Tab,belowTab, belowTabDown, TabDown, TabFocus, ButtonShadow, CB, CBDown, CBHover,
CBDownHover, HSlider, VSlider, Progress, HTMLRadioOn, HTMLRadioOff,
HTMLRadioOnHover, HTMLRadioOffHover, HTMLCBDown, HTMLCBDownHover,
HTMLCBHover, HTMLCB};

enum CustomColor{CustomCBOn=0, CustomCBOff, CustomRadioOn, CustomRadioOff,
    CustomTabOn, CustomTabOff, CustomSBSlider, CustomSBGroove};

enum PulseType{PushButton, ComboBox, AppletHandle, Splitter, ToolButton};

enum TransType{None=0, StippledBg, StippledBtn, Custom};

enum Tile{TileTopLeft=0, TileTop, TileTopRight,
    TileLeft, TileMiddle, TileRight, TileBtmLeft, TileBtm, TileBtmRight};

class ButtonTile
{
public:
    ButtonTile(){for(int i=0;i<9;++i){pixmaps[i]=0;};}
    ~ButtonTile(){for(int i=0;i<9;++i){if(pixmaps[i])delete pixmaps[i];};}
    QPixmap* pixmap(Tile pos){return(pixmaps[(int)pos]);}
    void setPixmap(Tile pos, QPixmap *pix){pixmaps[(int)pos]=pix;}    
protected:
    QPixmap *pixmaps[9];
};

class OptionHandler : public QObject
{
    Q_OBJECT
public:
    OptionHandler(QObject *parent);
    ~OptionHandler(){if(fillPixmap)delete fillPixmap;}
    void reloadSettings();
    int transType(){return(type);}
    KPixmap *pixmap(WId id){return(pixDict.find(id));}
    QPixmap* bgPixmap(){return(fillPixmap);}
    bool useShadowText(){return(shadowText);}
    const QColor& textColor();
    const QColor& bgColor();
    void reset(){menusProcessed=false;}

    bool useBgStipple(){return(bgStipple);}
    bool usePanelStipple(){return(panelStipple);}
    int stippleContrast(){return(contrast);}
    bool useReverseBtnColor(){return(reverseBtnColor);}
    bool usePanelCustomColor(){return(panelCustom);}
    bool useanimateProgressBar(){return(animateProgressBar);}
    QColor panelCustomColor(){return(customPanelColor);}
    bool useToolButtonFrame(){return(tbFrame);}
    void prepareMenus();

    bool useCustomColors(){return(customWidgetColor);}
    const QColor& customColor(int idx){return(customColors[idx]);}
protected:
    bool eventFilter(QObject *obj, QEvent *ev);

    bool menusProcessed;
    QColor color, fgColor;
    QPixmap *fillPixmap;
    int opacity;
    int contrast;
    bool shadowText;
    bool customWidgetColor;
    int type;

    bool bgStipple, panelStipple, reverseBtnColor, panelCustom, tbFrame, animateProgressBar;
    QColor customPanelColor;
    QIntDict<KPixmap>pixDict;

    QString colorStr, fgColorStr, panelColorStr;
    QColor customColors[8];
};

class LiquidStyle : public KStyle
{
    Q_OBJECT
public:
    friend class OptionHandler;

    LiquidStyle();
    virtual ~LiquidStyle();
    bool isPlain() const;

    virtual void polish(QWidget *w);
    virtual void unPolish(QWidget *w);
    virtual void polish(QPalette &p);
    virtual void unPolish(QApplication *a);
    virtual void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup &cg, const QPopupMenu* ) const;
    void drawKStylePrimitive(KStylePrimitive kpe, QPainter* p,
                             const QWidget* widget, const QRect &r,
                             const QColorGroup &cg,
                             SFlags flags = Style_Default,
                             const QStyleOption& = QStyleOption::Default) const;
    void drawPrimitive(PrimitiveElement pe, QPainter* p, const QRect &r,
                       const QColorGroup &cg, SFlags flags = Style_Default,
                       const QStyleOption& = QStyleOption::Default) const;
    void drawControl(ControlElement element, QPainter *p,
                     const QWidget *widget, const QRect &r,
                     const QColorGroup &cg, SFlags flags = Style_Default,
                     const QStyleOption& = QStyleOption::Default) const;
    void drawControlMask(ControlElement element, QPainter *p,
                         const QWidget *widget, const QRect &r,
                         const QStyleOption& = QStyleOption::Default) const;
    void drawComplexControl(ComplexControl control, QPainter *p,
                            const QWidget *widget, const QRect &r,
                            const QColorGroup &cg,
                            SFlags flags = Style_Default,
                            SCFlags controls = SC_All,
                            SCFlags active = SC_None,
                            const QStyleOption& = QStyleOption::Default) const;
    void drawComplexControlMask(ComplexControl control, QPainter *p,
                                const QWidget *widget, const QRect &r,
                                const QStyleOption& = QStyleOption::Default) const;

    int pixelMetric(PixelMetric m, const QWidget *widget=0) const;

    QSize sizeFromContents(ContentsType contents, const QWidget *widget,
                           const QSize &contentSize, const QStyleOption& opt) const;
    QRect subRect(SubRect r, const QWidget *widget ) const;
    QRect querySubControlMetrics(ComplexControl control, const QWidget *widget,
                                 SubControl subcontrol, const QStyleOption &opt = QStyleOption::Default) const;
    // Fix Qt3's wacky image positions
    QPixmap stylePixmap(StylePixmap stylepixmap, const QWidget *widget=0, const QStyleOption& = QStyleOption::Default ) const;
    bool eventFilter(QObject *object, QEvent *event);
public slots:
    void updateProgressPos();    
protected:
    void clearImage(QImage &img) const;
    ButtonTile* createButtonTile(const QColor &c, const QColor &bg, bool sunken) const;
    ButtonTile* separateTiles(QPixmap *pix, bool sunken) const;
    QPixmap* getPixmap(BitmapData item) const;
    QPixmap* adjustHSV(QImage &img, const QColor &c, bool blend=false, const QColor *bg=NULL) const;
    QImage* adjustHSVImage(QImage &img, const QColor &c, bool blend=false, const QColor *bg=NULL) const;
    void adjustHSV(QPixmap &pix, const QColor &c) const;
    QPixmap* processEmbedded(const char *label, const QColor &c,bool blend = false, const QColor *bg=NULL) const;
    void drawRoundButton(QPainter *p, const QColorGroup &cg, const QColor &c,const QColor &bg, int x, int y, int w, int h,
                         bool supportPushDown = false, bool pushedDown = false,bool autoDefault = false,  bool isHTML = false,
                         int bgX=-1, int bgY=-1) const;
    void drawClearBevel(QPainter *p, int x, int y, int w, int h, const QColor &c, const QColor &bg) const;
    void drawRectangularButton(QPainter *p, const QColor &c, int x, int y, int w, int h,
                               bool sunken=false, bool hover=false, bool isCombo=false) const;
    void drawEditFrame(QPainter *p, const QRect &r, const QColorGroup &cg, bool isHTML=false) const;
    void drawHTMLCBBorder(const QPixmap &pix, const QColor &c) const;
    bool isHTMLWidget(const QWidget *w) const;
private:
    LiquidStyle( const LiquidStyle & );
    LiquidStyle& operator=( const LiquidStyle & );

    QBitmap *sbLeft, *sbRight, *sbUp, *sbDown;
    QImage *btnBorderImg, *btnShadowImg;
    QPixmap *btnBlendPix, *bevelFillPix, *smallBevelFillPix, *menuPix;
    QBrush pagerBrush, pagerHoverBrush;
    QBrush origPanelBrush;
    QPalette origPanelPalette;

    QPixmap *pixmaps[BITMAP_ITEMS];
    QPixmap sbBuffer;
    QScrollBar *currentScrollBar;
    QWMatrix rMatrix;
    QWMatrix iMatrix;

    bool isKicker, isHTMLButton, initialPaletteLoaded, inExitPolish;

    QHeader *currentHeader;
    int headerHoverID;
    QPoint btnOffset;
    QWidget *currentTaskContainer;
    bool isTaskContainer, taskContainerHover;

    QPalette polishedPalette, tooltipPalette;
    unsigned int qtrcModificationTime;

    OptionHandler *optionHandler;

    QIntDict<ButtonTile>btnDict;
    QIntDict<ButtonTile>btnShadowedDict;
    QIntDict<QPixmap>bevelFillDict;
    QIntDict<QPixmap>smallBevelFillDict;
    QIntDict<ButtonTile>tabDict;
    QIntDict<ButtonTile>inverseTabDict;

    QPixmap *tmpBtnPix;

    QWidget *highlightWidget;
    int progAnimShift;

};

#endif
