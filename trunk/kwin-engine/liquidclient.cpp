/*
// LiquidClient.cpp
// -------------------
// Liquid+ Window Decoration for KDE 3.2.x 
// Copyright (c) 2004 Sarath Menon
// Source inspired from :
// Example window decoration for KDE
// -------------------
// Copyright (c) 2003, 2004 David Johnson
// Please see the header file for copyright and license information.
*/

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include <qbitmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qpixmap.h>

#include "liquidclient.h"
#include "bitmapdata.h"
#include "embeddata.h"

using namespace LiquidPlus;

// global constants

static const int BUTTONSIZE      = 18;
static const int DECOSIZE        = 12;
static const int TITLESIZE       = 18;
static const int FRAMESIZE       = 4;

bool LiquidPlusFactory::initialized_  = false;
Qt::AlignmentFlags LiquidPlusFactory::titlealign_ = Qt::AlignHCenter;
bool LiquidPlusFactory::useshadowedtext_ = true;

static FrameSet *midFrameSet = 0;
static QPixmap* frame_pix = 0;
static QPixmap* dis_frame_pix = 0;

static void create_pixmaps(){
    //taken from liquid --- need something like this anyway
    QColor aColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBar, true);
    QColor iColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBar, false);
    QColor aBlendColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBlend, true);
    QColor iBlendColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBlend, false);

    
 
    if (!LiquidPlusFactory::initialized()){ //took me quite some time to see this bug .. So much for pasting code from the original deco:(
        midFrameSet = new FrameSet("wm_mid");
        frame_pix = new QPixmap(BUTTONSIZE, BUTTONSIZE);
        dis_frame_pix = new QPixmap(BUTTONSIZE, BUTTONSIZE);
    }
    
    QImage img, *tmpImg;    
    QColor tbColor(aColor.light(115));
    QColor tbiColor(iColor.light(115));
    
    QPainter painter;
    painter.begin(frame_pix);
    img = uic_findImage_KWinLiquidDefault("wm_mid");
    img.detach();
    tmpImg = adjustHSVImage(img, aBlendColor, aColor, true);
    painter.drawImage(0,0, *tmpImg);
    delete tmpImg;
    painter.end();
    
    tmpImg = adjustHSVImage(img, iBlendColor, iColor, true);    
    painter.begin(dis_frame_pix);
    painter.drawImage(0,0, *tmpImg);
    delete tmpImg;
    painter.end();    
}

static void delete_pixmaps(){
    
    delete midFrameSet;
    midFrameSet = 0;        
    delete frame_pix;
    frame_pix = 0;
    delete dis_frame_pix;
    dis_frame_pix = 0;
}

FrameSet::FrameSet(const QString &embed_name){
    
    img = uic_findImage_KWinLiquidDefault(embed_name);
    if(img.isNull()){
        // debug - shouldn't happen
        qWarning("Couldn't find embedded image: %s!", embed_name.latin1());
        return;
    }
    img.detach();
    int i;

    for (i=0;i<=4;i++){
        activeFrames[i] = new QPixmap;
        inactiveFrames[i] = new QPixmap;
    }
    paintFrame();
}

void FrameSet::paintFrame(){
    QImage *tmpImg;
    //we need this cos if the user changes the color, we need repaint only the buttons
    QColor activeBgColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBar, true);
    QColor inactiveBgColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBar, false);
    QColor btnColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBlend, true);
    QColor inactiveBtnColor = KDecoration::options()->color(KDecorationOptions::ColorTitleBlend, false);
    
    for (int i=1;i<=2;i++){
        // do active buttons
        tmpImg = adjustHSVImage(img, btnColor.light(100 + 5*i), activeBgColor, true);
        activeFrames[2+i]->convertFromImage(*tmpImg);
        delete tmpImg;
        tmpImg = adjustHSVImage(img, btnColor.dark(100 + 5*i), activeBgColor, true);
        activeFrames[2-i]->convertFromImage(*tmpImg);
        delete tmpImg;
        // do inactive buttons
        tmpImg = adjustHSVImage(img, inactiveBtnColor.light(100 + 5*i),inactiveBgColor, true);
        inactiveFrames[2+i]->convertFromImage(*tmpImg);
        delete tmpImg;
        tmpImg = adjustHSVImage(img, inactiveBtnColor.dark(100 + 5*i),inactiveBgColor, true);
        inactiveFrames[2-i]->convertFromImage(*tmpImg);
        delete tmpImg;        
    }
    
    tmpImg = adjustHSVImage(img, btnColor, activeBgColor, true);
    activeFrames[2]->convertFromImage(*tmpImg);
    delete tmpImg;

    tmpImg = adjustHSVImage(img, inactiveBtnColor, inactiveBgColor,true);
    inactiveFrames[2]->convertFromImage(*tmpImg);
    delete tmpImg;
}

FrameSet::~FrameSet(){
    int i;
    for(i=0; i < 5; ++i){
        delete activeFrames[i];
        delete inactiveFrames[i];
    }
}

extern "C" KDecorationFactory* create_factory(){
    return new LiquidPlus::LiquidPlusFactory();
}

LiquidPlusFactory::LiquidPlusFactory(){
    readConfig();
    qInitImages_KWinLiquidDefault();
    create_pixmaps();
    initialized_ = true;
}

LiquidPlusFactory::~LiquidPlusFactory() { 
    delete_pixmaps();
    qCleanupImages_KWinLiquidDefault();
    initialized_ = false;
}

KDecoration* LiquidPlusFactory::createDecoration(KDecorationBridge* b){
    return new LiquidPlusClient(b, this);
}

bool LiquidPlusFactory::reset(unsigned long changed){
    // read in the configuration
    initialized_ = false;
    bool confchange = readConfig();
    initialized_ = true;
    //we still have old liquid baggage -- have to repaint the whole deco for even a font or color change. //FIXME
    if (confchange || (changed & (SettingDecoration | SettingButtons | SettingBorder))) {
            return true;
    } 
    else if(changed & SettingColors){
            create_pixmaps();
            midFrameSet->paintFrame();
            resetDecorations(changed);
            return false;
    } else return false;
}

bool LiquidPlusFactory::readConfig(){
    // create a config object
    KConfig config("kwinliquidplusrc");
    config.setGroup("General");

    // grab settings
    Qt::AlignmentFlags oldalign = titlealign_;
    QString value = config.readEntry("TitleAlignment", "AlignHCenter");
    if (value == "AlignLeft") titlealign_ = Qt::AlignLeft;
    else if (value == "AlignHCenter") titlealign_ = Qt::AlignHCenter;
    else if (value == "AlignRight") titlealign_ = Qt::AlignRight;

    bool old_useshadowedtext_ = useshadowedtext_;
    useshadowedtext_ = config.readBoolEntry("UseShadowedText", true);
    
    if (oldalign == titlealign_  && useshadowedtext_ == old_useshadowedtext_)
        return false;
    else
        return true;
}

LiquidPlusButton::LiquidPlusButton(LiquidPlusClient *parent, const char *name, const QString& tip, ButtonType type,
                             const unsigned char *bitmap)
    : QButton(parent->widget(), name), client_(parent), type_(type), deco_(0), lastmouse_(0){
    setBackgroundMode(NoBackground);
    setFixedSize(BUTTONSIZE, BUTTONSIZE);
    setCursor(arrowCursor);
    frames = midFrameSet;
    if (bitmap) setBitmap(bitmap);
    QToolTip::add(this, tip);
    currentFrame = 2;
    ascending = true;
}

LiquidPlusButton::~LiquidPlusButton(){
    if (deco_) delete deco_;
}

void LiquidPlusButton::setBitmap(const unsigned char *bitmap){
    if (!bitmap) return; // no bitmap, probably the menu button

    if (deco_) delete deco_;
    deco_ = new QBitmap(DECOSIZE, DECOSIZE, bitmap, true);
    deco_->setMask(*deco_);
    repaint(false);
}

QSize LiquidPlusButton::sizeHint() const{
    return QSize(BUTTONSIZE, BUTTONSIZE);
}

void LiquidPlusButton::enterEvent(QEvent *e){
    // if we wanted to do mouseovers, we would keep track of it here
    QButton::enterEvent(e);
    repaint(false);
}

void LiquidPlusButton::leaveEvent(QEvent *e){
    // if we wanted to do mouseovers, we would keep track of it here
    QButton::leaveEvent(e);
}

void LiquidPlusButton::mousePressEvent(QMouseEvent* e){
    lastmouse_ = e->button();

    // translate and pass on mouse event
    int button = LeftButton;
    if ((type_ != ButtonMax) && (e->button() != LeftButton)) {
        button = NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me(e->type(), e->pos(), e->globalPos(), button, e->state());
    QButton::mousePressEvent(&me);
}

void LiquidPlusButton::mouseReleaseEvent(QMouseEvent* e){
    lastmouse_ = e->button();

    // translate and pass on mouse event
    int button = LeftButton;
    if ((type_ != ButtonMax) && (e->button() != LeftButton)) {
        button = NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me(e->type(), e->pos(), e->globalPos(), button, e->state());
    QButton::mouseReleaseEvent(&me);
}

void LiquidPlusButton::drawButton(QPainter *painter){
    if (!LiquidPlusFactory::initialized()) return;

    QColorGroup group;
    int dx, dy;
    QPixmap *bgPix = client_->isActive() ? frame_pix : dis_frame_pix;
    if(!bgPix)
        return;
    painter->drawTiledPixmap(0, 0, width(), height(), *bgPix, x(), y());
    //painter->setPen(client_->options()->color(KDecoration::ColorButtonBg, client_->isActive()));
    if(timer.isActive())
        timer.stop();    
    bool hover = hasMouse() && !isDown();
    dx = (width() - DECOSIZE) / 2 ;
    dy = (height() - DECOSIZE) / 2;
    if (hover)
        painter->drawPixmap(0,0, *frames->frame(client_->isActive(), currentFrame));
    else 
        painter->drawPixmap(0,0, isDown() ? *frames->sunkenFrame(client_->isActive()) : *frames->normalFrame(client_->isActive()));
    if (isDown()) { dx++; dy++; }
    painter->setPen(group.dark());
    if(deco_) painter->drawPixmap(dx, dy, *deco_);
    if(hover)
        timer.singleShot(150, this, SLOT(slotTimer()));
    else {
        currentFrame = 2;
        ascending = true;
    }
    if (type_ == ButtonMenu) {
        // we paint the mini icon (which is 16 pixels high)
        dx = (width() - 16) / 2;
        dy = (height() - 16) / 2;
        if (isDown()) { dx++; dy++; }
        painter->drawPixmap(dx, dy, client_->icon().pixmap(QIconSet::Small, QIconSet::Normal));
    }
    //}
}

void LiquidPlusButton::slotTimer(){
    if(ascending){
        ++currentFrame;
        if(currentFrame == 5){
            currentFrame = 3;
            ascending = false;
        }
    }
    else{
        --currentFrame;
        if(currentFrame == -1){
            currentFrame = 1;
            ascending = true;
        }
    }
    repaint(false);
}

LiquidPlusClient::LiquidPlusClient(KDecorationBridge *b, KDecorationFactory *f) : KDecoration(b, f) { ;}

LiquidPlusClient::~LiquidPlusClient(){
    for (int n=0; n<ButtonTypeCount; n++) {
        if (button[n]) delete button[n];
    }
    delete tbBuffer;
}

void LiquidPlusClient::init(){
    createMainWidget(WResizeNoErase | WRepaintNoErase);
    widget()->installEventFilter(this);

    // for flicker-free redraws
    widget()->setBackgroundMode(NoBackground);

    // setup layout
    QGridLayout *mainlayout = new QGridLayout(widget(), 4, 3); // 4x3 grid
    QHBoxLayout *titlelayout = new QHBoxLayout();
    titlebar_ = new QSpacerItem(1, TITLESIZE, QSizePolicy::Expanding, QSizePolicy::Fixed);

    mainlayout->setResizeMode(QLayout::FreeResize);
    //mainlayout->addRowSpacing(0, FRAMESIZE);
    mainlayout->addRowSpacing(3, FRAMESIZE);
    mainlayout->addColSpacing(0, FRAMESIZE);
    mainlayout->addColSpacing(2, FRAMESIZE);

    mainlayout->addLayout(titlelayout, 1, 1);
    if (isPreview()) {
        mainlayout->addWidget(
        new QLabel(i18n("<b><center>Liquid+ Preview</center></b>"),widget()), 2, 1);
    } else {
        mainlayout->addItem(new QSpacerItem(0, 0), 2, 1);
    }

    // the window should stretch
    mainlayout->setRowStretch(2, 10);
    mainlayout->setColStretch(1, 10);

    // setup titlebar buttons
    for (int n=0; n<ButtonTypeCount; n++) button[n] = 0;
    addButtons(titlelayout, options()->titleButtonsLeft());
    titlelayout->addItem(titlebar_);
    addButtons(titlelayout, options()->titleButtonsRight());
    tbBuffer = new QPixmap;
}

void LiquidPlusClient::addButtons(QBoxLayout *layout, const QString& s){
    const unsigned char *bitmap;
    QString tip;

    if (s.length() > 0) {
        for (unsigned n=0; n < s.length(); n++) {
            switch (s[n]) {
              case 'M': // Menu button
                  if (!button[ButtonMenu]) {
                      button[ButtonMenu] = new LiquidPlusButton(this, "menu", i18n("Menu"), ButtonMenu, 0);
                      connect(button[ButtonMenu], SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
                      layout->addWidget(button[ButtonMenu]);
                  }
                  break;

              case 'S': // Sticky button
                  if (!button[ButtonSticky]) {
              if (isOnAllDesktops()) {
              bitmap = stickydown_bits;
              tip = i18n("Un-Sticky");
              } else {
              bitmap = sticky_bits;
              tip = i18n("Sticky");
              }
                      button[ButtonSticky] = new LiquidPlusButton(this, "sticky", tip, ButtonSticky, bitmap);
                      connect(button[ButtonSticky], SIGNAL(clicked()), this, SLOT(toggleOnAllDesktops()));
                      layout->addWidget(button[ButtonSticky]);
                  }
                  break;

              case 'H': // Help button
                  if ((!button[ButtonHelp]) && providesContextHelp()) {
                      button[ButtonHelp] = new LiquidPlusButton(this, "help", i18n("Help"), ButtonHelp, help_bits);
                      connect(button[ButtonHelp], SIGNAL(clicked()), this, SLOT(showContextHelp()));
                      layout->addWidget(button[ButtonHelp]);
                  }
                  break;

              case 'I': // Minimize button
                  if ((!button[ButtonMin]) && isMinimizable())  {
                      button[ButtonMin] = new LiquidPlusButton(this, "iconify", i18n("Minimize"), ButtonMin, min_bits);
                      connect(button[ButtonMin], SIGNAL(clicked()), this, SLOT(minimize()));
                      layout->addWidget(button[ButtonMin]);
                  }
                  break;

              case 'A': // Maximize button
                  if ((!button[ButtonMax]) && isMaximizable()) {
              if (maximizeMode() == MaximizeFull) {
              bitmap = minmax_bits;
              tip = i18n("Restore");
              } else {
              bitmap = max_bits;
              tip = i18n("Maximize");
              }
                      button[ButtonMax]  =
                          new LiquidPlusButton(this, "maximize", tip, ButtonMax, bitmap);
                      connect(button[ButtonMax], SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
                      layout->addWidget(button[ButtonMax]);
                  }
                  break;

              case 'X': // Close button
                  if ((!button[ButtonClose]) && isCloseable()) {
                      button[ButtonClose] = new LiquidPlusButton(this, "close", i18n("Close"), ButtonClose, close_bits);
                      connect(button[ButtonClose], SIGNAL(clicked()), this, SLOT(closeWindow()));
                      layout->addWidget(button[ButtonClose]);
                  }
                  break;

              case '_': // Spacer item
                  layout->addSpacing(FRAMESIZE);
            }
    }
    }
}

void LiquidPlusClient::activeChange(){
    for (int n=0; n<ButtonTypeCount; n++)
        if (button[n]) button[n]->reset();
    widget()->repaint(false);
}

void LiquidPlusClient::captionChange(){
    widget()->repaint(titlebar_->geometry(), false);
}

void LiquidPlusClient::desktopChange(){
    bool d = isOnAllDesktops();
    if (button[ButtonSticky]) {
        button[ButtonSticky]->setBitmap(d ? stickydown_bits : sticky_bits);
    QToolTip::remove(button[ButtonSticky]);
    QToolTip::add(button[ButtonSticky], d ? i18n("Un-Sticky") : i18n("Sticky"));
    }
}

void LiquidPlusClient::iconChange(){
    if (button[ButtonMenu]) {
        button[ButtonMenu]->setBitmap(0);
        button[ButtonMenu]->repaint(false);
    }
}

void LiquidPlusClient::maximizeChange(){
    bool m = (maximizeMode() == MaximizeFull);
    if (button[ButtonMax]) {
        button[ButtonMax]->setBitmap(m ? minmax_bits : max_bits);
    QToolTip::remove(button[ButtonMax]);
    QToolTip::add(button[ButtonMax], m ? i18n("Restore") : i18n("Maximize"));
    }
}

void LiquidPlusClient::shadeChange()
{ ; }

void LiquidPlusClient::borders(int &l, int &r, int &t, int &b) const{
    l = r = FRAMESIZE;
    t = TITLESIZE;
    b = FRAMESIZE;
}

void LiquidPlusClient::resize(const QSize &size){
    widget()->resize(size);
}

QSize LiquidPlusClient::minimumSize() const{
    return widget()->minimumSize();
}

KDecoration::Position LiquidPlusClient::mousePosition(const QPoint &point) const {
    const int corner = 24;
    Position pos;

    if (point.y() <= FRAMESIZE) {
        // inside top frame
        if (point.x() <= corner)                 pos = PositionTopLeft;
        else if (point.x() >= (width()-corner))  pos = PositionTopRight;
        else                                     pos = PositionTop;
    } else if (point.y() >= (height()-FRAMESIZE*2)) {
        // inside handle
        if (point.x() <= corner)                 pos = PositionBottomLeft;
        else if (point.x() >= (width()-corner))  pos = PositionBottomRight;
        else                                     pos = PositionBottom;
    } else if (point.x() <= FRAMESIZE) {
        // on left frame
        if (point.y() <= corner)                 pos = PositionTopLeft;
        else if (point.y() >= (height()-corner)) pos = PositionBottomLeft;
        else                                     pos = PositionLeft;
    } else if (point.x() >= width()-FRAMESIZE) {
        // on right frame
        if (point.y() <= corner)                 pos = PositionTopRight;
        else if (point.y() >= (height()-corner)) pos = PositionBottomRight;
        else                                     pos = PositionRight;
    } else {
        // inside the frame
        pos = PositionCenter;
    }
    return pos;
}

bool LiquidPlusClient::eventFilter(QObject *obj, QEvent *e){
    if (obj != widget()) return false;

    switch (e->type()) {
      case QEvent::MouseButtonDblClick: {
          mouseDoubleClickEvent(static_cast<QMouseEvent *>(e));
          return true;
      }
      case QEvent::MouseButtonPress: {
          processMousePressEvent(static_cast<QMouseEvent *>(e));
          return true;
      }
      case QEvent::Paint: {
          paintEvent(static_cast<QPaintEvent *>(e));
          return true;
      }
      case QEvent::Resize: {
          resizeEvent(static_cast<QResizeEvent *>(e));
          return true;
      }
      case QEvent::Show: {
          showEvent(static_cast<QShowEvent *>(e));
          return true;
      }
      default: {
          return false;
      }
    }

    return false;
}

void LiquidPlusClient::mouseDoubleClickEvent(QMouseEvent *e){
    if (titlebar_->geometry().contains(e->pos())) titlebarDblClickOperation();
}

void LiquidPlusClient::paintEvent(QPaintEvent*){
    if (!LiquidPlusFactory::initialized()) return;
    // Note that most of the painting code is taken from the original Liquid decoration. (Do i have a choice here ???) 

    QPainter painter(widget());
    QRect title(titlebar_->geometry());
    // draw the titlebar
    if(tbBuffer->width() != (width() - 2*FRAMESIZE))
        tbBuffer->resize(width() - 2*FRAMESIZE, TITLESIZE);
    QPainter tmpPainter;
    tmpPainter.begin(tbBuffer);
    tmpPainter.setFont(options()->font(isActive(), false));
    tmpPainter.setPen(options()->color(KDecoration::ColorFont, isActive()).dark(120));
    int textLen = tmpPainter.fontMetrics().width(caption());
    bool clippedText = false;
    tmpPainter.drawTiledPixmap(0, 0, tbBuffer->width(), tbBuffer->height(), isActive() ? *frame_pix : *dis_frame_pix);
    //draw title text
    title.moveTopLeft(QPoint(title.x()-3, title.y()-1));
    if(title.width() > TITLESIZE){
        int fillLen = textLen + TITLESIZE -2;
        if(fillLen > title.width()){
            fillLen = title.width();
            clippedText = true;
        }
	int fx = 0;
       if (LiquidPlusFactory::titleAlign() == AlignLeft){
           fx = title.x() -2 ;
       }
       if (LiquidPlusFactory::titleAlign() == AlignRight){
           fx = title.right() - fillLen + 2;
       }
       if (LiquidPlusFactory::titleAlign() == AlignHCenter){
           fx = (title.x()+(title.width()-fillLen)/2);
       }
    }
    //A word about the shadowed text - This part is based upon how liquid (style) sets shadowed text. 
    //So while it may be not like plastik's shadow, it kind of complements the style.
    
    if(!clippedText){
        if(LiquidPlusFactory::useShadowedText()){
              tmpPainter.setPen(options()->color(ColorTitleBar, isActive()).dark(130));
              tmpPainter.drawText(title.x() + FRAMESIZE+1, title.y(), title.width() - FRAMESIZE * 2, title.height(),LiquidPlusFactory::titleAlign() | 
               AlignVCenter|SingleLine,caption());
        }
        tmpPainter.setPen(options()->color(ColorFont, isActive()).light(115));
        tmpPainter.drawText(title.x() + FRAMESIZE, title.y()+1, title.width() - FRAMESIZE * 2, title.height(), LiquidPlusFactory::titleAlign() | 
               AlignVCenter|SingleLine,caption());
    }
    else{
        if(LiquidPlusFactory::useShadowedText()){
              tmpPainter.setPen(options()->color(ColorTitleBar, isActive()).dark(130));
              tmpPainter.drawText(title.x() + FRAMESIZE+8, title.y(), title.width() - 16 - FRAMESIZE * 2, title.height(),
                    LiquidPlusFactory::titleAlign()|AlignVCenter|SingleLine, caption());
        }
        tmpPainter.setPen(options()->color(KDecorationOptions::ColorFont, isActive()).light(115));
        tmpPainter.drawText(title.x() + FRAMESIZE+7, title.y()+1, title.width() - 16 - FRAMESIZE * 2, title.height(), 
              LiquidPlusFactory::titleAlign() |AlignVCenter|SingleLine, caption());
    }
    tmpPainter.end();
    painter.drawPixmap(FRAMESIZE, 0, *tbBuffer);
        
    //draw the border
    QRect frame;
    QColor framebg = options()->color(KDecorationOptions::ColorFrame, isActive());
    QColor borderColor = QColor(110,110,110);
    
    //left
    frame.setRect(0, 0, FRAMESIZE, height());
    painter.fillRect(frame, framebg);
    painter.setPen(borderColor);
    painter.drawLine(frame.left(), frame.top(), frame.right(), frame.top());
    painter.drawLine(0, 0, 0, height());
    painter.drawLine(frame.right(), frame.top()+ TITLESIZE-1 , frame.right(), frame.bottom() - FRAMESIZE);
    painter.setPen(framebg.dark(110));
    painter.drawLine(frame.left()+1, frame.top()+1, frame.right()-1, frame.top()+1);
    painter.drawLine(frame.left()+1, frame.top()+3, frame.right()-1, frame.top()+3);
    painter.setPen(framebg.light(110));
    painter.drawLine(frame.left()+1, frame.top()+2, frame.right()-1, frame.top()+2);
    
    //bottom
    frame.setRect(FRAMESIZE, height() - FRAMESIZE, width() -2*FRAMESIZE, FRAMESIZE);
    painter.fillRect(frame, framebg);
    painter.setPen(borderColor);
    painter.drawLine(frame.left(), frame.top(), frame.right(), frame.top());
    painter.setPen(framebg.dark(110));
    painter.drawLine(frame.left()+1, frame.top()+1, frame.right(), frame.top()+1);
    painter.drawLine(frame.left()+1, frame.top()+3, frame.right(), frame.top()+3);
    painter.setPen(framebg.light(110));
    painter.drawLine(frame.left()+1, frame.top()+2, frame.right(), frame.top()+2);
    
    //right
    frame.setRect(width()-FRAMESIZE, 0, FRAMESIZE, height());
    painter.fillRect(frame, framebg);
    painter.setPen(borderColor);
    painter.drawLine(frame.left(), frame.top(), frame.right()-1, frame.top());
    painter.drawLine(frame.left(), frame.top() +TITLESIZE -1 , frame.left(), frame.bottom()-FRAMESIZE);
    painter.drawLine(frame.right(), frame.top()+1, frame.right(), height());
    painter.drawLine(0, height()-1 , width(), height()-1 );
    painter.setPen(framebg.dark(110));
    painter.drawLine(frame.left()+1, frame.top()+1, frame.right()-1, frame.top()+1);
    painter.drawLine(frame.left()+1, frame.top()+3, frame.right()-1, frame.top()+3);
    painter.setPen(framebg.light(110));
    painter.drawLine(frame.left()+1, frame.top()+2, frame.right()-1, frame.top()+2);
}

void LiquidPlusClient::resizeEvent(QResizeEvent *){
    if (widget()->isShown()) {
        QRegion region = widget()->rect();
        region = region.subtract(titlebar_->geometry());
    widget()->erase(region);
    }
}

void LiquidPlusClient::showEvent(QShowEvent *){
    widget()->repaint();
}

void LiquidPlusClient::maxButtonPressed(){
    if (button[ButtonMax]) {
        switch (button[ButtonMax]->lastMousePress()) {
          case MidButton:
              maximize(maximizeMode() ^ MaximizeVertical);
              break;
          case RightButton:
              maximize(maximizeMode() ^ MaximizeHorizontal);
              break;
          default:
              (maximizeMode() == MaximizeFull) ? maximize(MaximizeRestore)
                  : maximize(MaximizeFull);
        }
    }
}

void LiquidPlusClient::menuButtonPressed(){
    if (button[ButtonMenu]) {
        static QTime* t = 0;
        static LiquidPlusClient* lastClient = 0;
        if (!t)        t = new QTime;
        bool dbl = (lastClient == this && t->elapsed() <= QApplication::doubleClickInterval());
        lastClient = this;
	t->start();
        if(dbl)      closeWindow();
        else {
                QPoint p(button[ButtonMenu]->rect().bottomLeft().x(), button[ButtonMenu]->rect().bottomLeft().y());
                KDecorationFactory* f = factory();
                showWindowMenu(button[ButtonMenu]->mapToGlobal(p));
                if (!f->exists(this)) return; // decoration was destroyed
                button[ButtonMenu]->setDown(false);
        }
    }
}

#include "liquidclient.moc"
