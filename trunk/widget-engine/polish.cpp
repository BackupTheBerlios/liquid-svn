#include "liquid.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <qlabel.h>
#include <qtimer.h>

void LiquidStyle::polish(QPalette &pal)
{
    if(inExitPolish)
        return;
    // clear out all old colorized pixmaps
    int i;
    for(i=0; i < BITMAP_ITEMS; ++i){
        if(pixmaps[i]){
            delete pixmaps[i];
            pixmaps[i] = NULL;
        }
    }
    // clear out all dicts
    btnDict.clear();
    btnShadowedDict.clear();
    bevelFillDict.clear();
    smallBevelFillDict.clear();
    // reset brushes
    pagerHoverBrush = QBrush();
    pagerBrush =  QBrush();

    /*
     * Okay, here's the deal. We might need to set the application
     * background brush with a pixmap stipple or a custom kicker color.
     * It wouldn't be much of a problem but Qt calls this method for a lot
     * of different palettes, not just the application's, so we really don't
     * know what we are changing and if the QPalette parameter is actually
     * the application's colors or something like a tooltip.
     *
     * This used to mean I'd check the timestamp of .qt/qtrc and reload the
     * entire palette if it has changed. Now we just assume the QPalette
     * parameter is the new application palette. Much more efficent :)
     */

    if(!isKicker && isPlain())
        return;

    bool newPalette = false;
    struct stat buffer;
    if(stat(QFile::encodeName(QDir::homeDirPath()+"/.qt/qtrc"), &buffer) == 0){
        unsigned int lastModTime = (unsigned int)buffer.st_mtime;
        if(lastModTime > qtrcModificationTime){
            qtrcModificationTime = lastModTime;
            newPalette = true;
        }
    }
    else if(!initialPaletteLoaded) // Hack, should always have qtrc in KDE
        newPalette = true;
    initialPaletteLoaded = true;

    if(!newPalette){
        pal = polishedPalette;
        return;
    }

    // only executed on first run or if qtrc has changed
    if(isKicker){
        origPanelBrush = pal.brush(QPalette::Active, QColorGroup::Background);
        origPanelPalette = pal;
/*        QColor c(pal.active().button());
        if(optionHandler->usePanelCustomColor() && optionHandler->panelCustomColor().isValid())
            c = optionHandler->panelCustomColor();

        pal.setColor(QColorGroup::Mid, c.dark(110));
        pal.setColor(QColorGroup::Dark, c.dark(130));
        pal.setColor(QColorGroup::Midlight, c.light(110));
        pal.setColor(QColorGroup::Light, c.light(115));
        if(optionHandler->usePanelStipple()){
             QPixmap stipple(64, 64);
             stipple.fill(c.rgb());
             QPainter p;
             p.begin(&stipple);
             p.setPen(c.dark(100+optionHandler->stippleContrast()));
             for(i=0; i < 63; i+=4){
                   p.drawLine(0, i, 63, i);
                   p.drawLine(0, i+1, 63, i+1);
             }
             p.end();
             QBrush brush(c, stipple);
             pal.setBrush(QColorGroup::Background, brush);
        }
        else
            pal.setBrush(QColorGroup::Background, c); */
    }
    else if(!isPlain()){
        origPanelBrush = pal.brush(QPalette::Active, QColorGroup::Button);
        QColor c(pal.active().background());
        QPixmap stipple(64, 64);
        stipple.fill(c.rgb());
        QPainter p;
        p.begin(&stipple);
        p.setPen(c.dark(100+optionHandler->stippleContrast()));
        for(i=0; i < 63; i+=4){
            p.drawLine(0, i, 63, i);
            p.drawLine(0, i+1, 63, i+1);
        }
        p.end();
        QBrush brush(c, stipple);
        pal.setBrush(QColorGroup::Background, brush);

    }

    polishedPalette = pal;
    optionHandler->reset();
}

void LiquidStyle::unPolish(QApplication *app)
{

    QPalette pal(app->palette());
    inExitPolish = true;
    if(isKicker){
        // reset kicker
/*        QColor c = origPanelBrush.color();
        pal.setBrush(QColorGroup::Background, c);
        pal.setBrush(QColorGroup::Mid, c.dark(110));
        pal.setBrush(QColorGroup::Dark, c.dark(130));
        pal.setBrush(QColorGroup::Midlight, c.light(110));
        pal.setBrush(QColorGroup::Light, c.light(115));
        app->setPalette(pal);*/
    }
    else if(!isPlain()){
        // reset any stipples
        if(pal.brush(QPalette::Active, QColorGroup::Background).pixmap())
            pal.setBrush(QColorGroup::Background, pal.active().background());
        if(pal.brush(QPalette::Active, QColorGroup::Button).pixmap())
            pal.setBrush(QColorGroup::Button, pal.active().button());
        app->setPalette(pal);
    }

    /*if(isKicker || !isPlain())
        pal.setBrush(QColorGroup::Highlight, pal.active().highlight());
    */
    inExitPolish = false;
}

void LiquidStyle::polish(QWidget *w)
{
    if(qstrcmp(w->name(), "kde toolbar widget") == 0){
        // can't just set the background mode to background because the widget
        // resets it to button :P
        QPalette pal(w->palette());
        pal.setBrush(QColorGroup::Button, qApp->palette().active().brush(QColorGroup::Background));
        w->setPalette(pal);
        if(!isPlain() && !isKicker)
            w->setBackgroundOrigin(QWidget::WindowOrigin);
        return;
    }

    if(!isPlain () && w->inherits("KonqIconViewWidget") || w->inherits("KHTMLView")){
        // reset pixmap, since Konq animation can't handle it
        QPalette pal(w->palette());
        pal.setBrush(QColorGroup::Background, pal.active().background());
        w->setPalette(pal);
        return;
    }
    if(w->inherits("QScrollView") || w->isA("QViewportWidget") || w->inherits("QClipperWidget"))
        return;

    if(w->inherits("QPopupMenu")){
        optionHandler->prepareMenus();
        w->setBackgroundMode(QWidget::NoBackground);
        w->installEventFilter(optionHandler);
        return;
    }
    if(!isPlain()){
        if(w->inherits("KActiveLabel")){
            w->installEventFilter(this);
            return;
        }
        else if(w->inherits("QTipLabel")){
            w->setPalette(tooltipPalette);
            w->installEventFilter(this);
            return;
        }
       /* else if(w->inherits("KdmClock")){
            w->setBackgroundColor(qApp->palette().active().background());
            ((QFrame*)w)->setLineWidth(1);
            ((QFrame*)w)->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
            return;
        }*/
    }

    if(w->inherits("QMenuBar")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WindowOrigin);
        return;
    }
    else if(w->inherits("QDockWindow")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WindowOrigin);
        w->installEventFilter(this);
        return;
    }
    else if(w->inherits("QComboBox") || w->inherits("QPushButton")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        w->installEventFilter(this);
    }
    //Sarathexp -- comment TaskContainer
    else if(w->inherits("KToolBarButton") || w->inherits("QRadioButton") || w->inherits("QCheckBox") || w->isA("AppletHandleDrag") ||
            w->inherits("KMiniPagerButton") || w->inherits("QSplitterHandle") || /*w->inherits("TaskContainer") || */w->inherits("QLineEdit")){
        w->installEventFilter(this);
    }
    
    else if(w->inherits("QScrollBar")){
        w->setBackgroundMode(QWidget::NoBackground);
        w->installEventFilter(this);
        return;
    }
    else if(w->inherits("QHeader")){
        w->setMouseTracking(true);
        w->installEventFilter(this);
    }

    if(w->inherits("QToolButton")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WindowOrigin);
        return;
    }
    //hack for the animated progressbar .. again thank god there is Thomas or i wouldn't have noticed it.
    //Baghira does this in a better way, but for a (slight) drop in performance.
    if (w->inherits("QProgressBar")){
        w->setBackgroundMode( NoBackground );
            QTimer* timer = new QTimer(w);
        timer->start(50);
        connect(timer, SIGNAL(timeout()), w, SLOT(update()));
    }
    if(w->isTopLevel()){
        if(isKicker){
            qWarning("Got panel toplevel %s", w->className());
            // force extensions and child panels to use proper palette.
/*            if(!w->inherits("Panel") && !w->inherits("KPanelApplet")){
                if(!isPlain() && !origPanelPalette.active().brush(QColorGroup::Background).pixmap() ){
                    QBrush brush(origPanelPalette.active().brush(QColorGroup::Background));
                    QColor c(brush.color());
                    QPixmap stipple(64, 64);
                    stipple.fill(c.rgb());
                    QPainter p;
                    p.begin(&stipple);
                    p.setPen(c.dark(100+optionHandler->stippleContrast()));
                    int i;
                    for(i=0; i < 63; i+=4){
                        p.drawLine(0, i, 63, i);
                        p.drawLine(0, i+1, 63, i+1);
                    }
                    p.end();
                    brush.setPixmap(stipple);
                    origPanelPalette.setBrush(QColorGroup::Background, brush);
                }
                w->setPalette(origPanelPalette);
            } */
        }
        return;
    }
    bool isViewport = qstrcmp(w->name(), "qt_viewport") == 0 || qstrcmp(w->name(), "qt_clipped_viewport") == 0;
    bool isViewportChild = w->parent() &&
        ((qstrcmp(w->parent()->name(), "qt_viewport") == 0) ||
         (qstrcmp(w->parent()->name(), "qt_clipped_viewport") == 0));

    if (w->inherits("TaskBar")){
        w->setBackgroundOrigin(QWidget::WidgetOrigin);
	qWarning("Iam a taskbar ...... yipee");
        return;
    }
    if(isViewportChild){
        if(w->inherits("QRadioButton")){
            if(isHTMLWidget(w)){
                w->setAutoMask(true);
                w->setBackgroundMode(QWidget::NoBackground);
                return;
            }
        }
    }

    if(!isPlain()){
        if(!isViewport && w->parent() && qstrcmp(w->parent()->name(), "proxyview") == 0){
            w->setBackgroundOrigin(QWidget::WindowOrigin);
            return;
        }
        if(w->ownPalette() && !w->inherits("QButton") && !w->inherits("QComboBox"))
            return;
        if(w->inherits("PanelButtonBase"))
            return;

        if(!isKicker&& !isViewport && !isViewportChild && !w->testWFlags(WType_Popup) && !w->inherits("KDesktop") && !w->inherits("PasswordDlg")){
            if(w->backgroundMode() == QWidget::PaletteBackground || w->backgroundMode() == QWidget::PaletteButton){
                w->setBackgroundOrigin(QWidget::WindowOrigin);
            }
        }
    }
    KStyle::polish(w);
}

void LiquidStyle::unPolish(QWidget *w)
{
    if(qstrcmp(w->name(), "kde toolbar widget") == 0){
        w->setBackgroundOrigin(QWidget::WidgetOrigin);
        w->unsetPalette();
        return;
    }
    if(w->inherits("QPopupMenu")){
        w->unsetPalette();
        w->setBackgroundMode(QWidget::PaletteButton);
        return;
    }
    if(!isPlain () && w->inherits("KonqIconViewWidget") || w->inherits("KHTMLView")){
        w->unsetPalette();
        return;
    }
    if(w->inherits("QScrollView") || w->isA("QViewportWidget") || w->inherits("QClipperWidget"))
        return;

    if(!isPlain()){
        if(w->inherits("KActiveLabel") || w->inherits("QTipLabel")){
            w->unsetPalette();
            w->removeEventFilter(this);
            return;
        }
        /*
        else if(w->inherits("KdmClock")){
            ; // Check this!
            return;
        }*/
    }

    else if(w->inherits("QMenuBar")){
        w->setBackgroundMode(QWidget::PaletteButton);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
        return;
    }
    else if(w->inherits("QDockWindow")){
        w->setBackgroundMode(QWidget::PaletteButton);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
        w->removeEventFilter(this);
        return;
    }
    else if(w->inherits("QComboBox") || w->inherits("QPushButton")){
        w->setBackgroundMode(QWidget::PaletteButton);
        w->removeEventFilter(this);
    }
    //Sarathexp -- comment TaskContainer
    else if(w->inherits("KToolBarButton") || w->inherits("QRadioButton") || w->inherits("QCheckBox") || w->isA("AppletHandleDrag") ||
            w->inherits("KMiniPagerButton") || w->inherits("QSplitterHandle") || /*w->inherits("TaskContainer") || */w->inherits("QLineEdit")){
        w->removeEventFilter(this);
    }
    else if(w->inherits("QScrollBar")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        w->removeEventFilter(this);
        return;
    }
    else if(w->inherits("QHeader")){
        w->setMouseTracking(false);
        w->removeEventFilter(this);
    }

    if(w->inherits("QToolButton")){
        w->setBackgroundMode(QWidget::PaletteButton);
        if(!isPlain())
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
        return;
    }

    if(w->isTopLevel() && isKicker && !w->inherits("Panel") && !w->inherits("KPanelApplet")){
        w->unsetPalette();
        return;
    }

    bool isViewport = qstrcmp(w->name(), "qt_viewport") == 0 || qstrcmp(w->name(), "qt_clipped_viewport") == 0;
    bool isViewportChild = w->parent() && ((qstrcmp(w->parent()->name(), "qt_viewport") == 0) ||
         (qstrcmp(w->parent()->name(), "qt_clipped_viewport") == 0));

    if(isViewportChild){
        if(w->inherits("QRadioButton")){
            if(isHTMLWidget(w)){
                w->setAutoMask(false);
                w->setBackgroundMode(QWidget::PaletteBackground);
                return;
            }
        }
    }

    if(!isPlain()){
        if(!isViewport && w->parent() && qstrcmp(w->parent()->name(), "proxyview") == 0){
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
            return;
        }
        if(w->ownPalette() && !w->inherits("QButton") && !w->inherits("QComboBox"))
            return;
        if(w->inherits("PanelButtonBase"))
            return;

        if(!isViewport && !isViewportChild && !w->testWFlags(WType_Popup) && !w->inherits("KDesktop") && !w->inherits("PasswordDlg")){
            if(w->backgroundOrigin() == QWidget::WindowOrigin)
                w->setBackgroundOrigin(QWidget::WidgetOrigin);
        }
    }
    KStyle::unPolish(w);
}
