/* This is a kwin client rewritten from scratch. This is not the original client by mosfet ported to KDE 3.2
*  The ported version has been seperately released, although incomplete
*  Copyright (c) 2004 Sarath Menon <s_menon@users.sourceforge.net>
*  All original bitmap data, drawing routines Copyright (c) Daniel M. Duley a.k.a Mosfet <mosfet@idunno>
*
*  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
*  License as published by the Free Software Foundation; either  version 2 of the License, or (at your option) any later version.
*  The original client was licensed under QPL. (Becomes GPL anyway now)
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  This client was based upon the example client Copyright (c) 2003, 2004 David Johnson.
*  His terms of license are reproduced below:
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to
*  deal in the Software without restriction, including without limitation the
*  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
*  sell copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
*  IN THE SOFTWARE.
*/

#ifndef LIQUIDCLIENT_H
#define LIQUIDCLIENT_H

#include <qbutton.h>
#include <qtimer.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <qimage.h>

class QSpacerItem;
class QPoint;

namespace LiquidPlus {

class LiquidPlusClient;

enum ButtonType {
    ButtonHelp=0,
    ButtonMax,
    ButtonMin,
    ButtonClose, 
    ButtonMenu,
    ButtonSticky,
    ButtonTypeCount
};

//taken from Liquid -- too good to think of an alternative
//mosfet is one gr8 coder ... too bad we still dont have him :(
class FrameSet
{
public:
    FrameSet(const QString &embed_name);
    ~FrameSet();
    void paintFrame();
    QPixmap* frame(bool active, int i) {return(i >= 0 && i < 5 ? active ? activeFrames[i] : inactiveFrames[i] : 0);}
    QPixmap* normalFrame(bool active){return(active ? activeFrames[2] : inactiveFrames[2]);}
    QPixmap* sunkenFrame(bool active){return(active ? activeFrames[0] : inactiveFrames[0]);}
protected:
    QPixmap *activeFrames[5];
    QPixmap *inactiveFrames[5];
    QImage img;
};

class LiquidPlusFactory: public KDecorationFactory
{
public:
    LiquidPlusFactory();
    virtual ~LiquidPlusFactory();
    virtual KDecoration *createDecoration(KDecorationBridge *b);
    virtual bool reset(unsigned long changed);

    static bool initialized();
    static Qt::AlignmentFlags titleAlign();
    static bool useShadowedText();

private:
    bool readConfig();
    static bool initialized_;
    static Qt::AlignmentFlags titlealign_;
    static bool useshadowedtext_;
};

inline bool LiquidPlusFactory::initialized()
    { return initialized_; }

inline Qt::AlignmentFlags LiquidPlusFactory::titleAlign()
    { return titlealign_; }
inline bool LiquidPlusFactory::useShadowedText()
    {return useshadowedtext_;}

class LiquidPlusButton : public QButton
{
    Q_OBJECT
public:
    LiquidPlusButton(LiquidPlusClient *parent=0, const char *name=0, const QString &tip=NULL, ButtonType type=ButtonHelp,
                  const unsigned char *bitmap=0);
    ~LiquidPlusButton();

    void setBitmap(const unsigned char *bitmap);
    QSize sizeHint() const;
    int lastMousePress() const;
    void reset();

protected:
    QTimer timer;
    FrameSet *frames;
    int currentFrame;
    bool ascending;
    
protected slots:    
    void slotTimer();

private:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void drawButton(QPainter *painter);
    LiquidPlusClient *client_;
    ButtonType type_;
    QBitmap *deco_;
    int lastmouse_;
    QBitmap *bitmap_;
    QPixmap bgPicture;
};

inline int LiquidPlusButton::lastMousePress() const { return lastmouse_; }

inline void LiquidPlusButton::reset() { repaint(false); }

class LiquidPlusClient : public KDecoration
{
    Q_OBJECT
public:
    LiquidPlusClient(KDecorationBridge *b, KDecorationFactory *f);
    virtual ~LiquidPlusClient();

    virtual void init();

    virtual void activeChange();
    virtual void desktopChange();
    virtual void captionChange();
    virtual void iconChange();
    virtual void maximizeChange();
    virtual void shadeChange();

    virtual void borders(int &l, int &r, int &t, int &b) const;
    virtual void resize(const QSize &size);
    virtual QSize minimumSize() const;
    virtual Position mousePosition(const QPoint &point) const;
    
private:
    void addButtons(QBoxLayout* layout, const QString& buttons);
    bool eventFilter(QObject *obj, QEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    LiquidPlusButton *button[ButtonTypeCount];
    QSpacerItem *titlebar_;
    QPixmap *tbBuffer;

private slots:
    void maxButtonPressed();
    void menuButtonPressed();
};

} // namespace LiquidPlus

#endif // LIQUIDCLIENT_H
