#include "liquid.h"

QPixmap* LiquidStyle::adjustHSV(QImage &img, const QColor &c,bool blend, const QColor *bg) const{
    QImage *tmp = adjustHSVImage(img, c, blend, bg);
    QPixmap *pix = new QPixmap;
    pix->convertFromImage(*tmp);
    delete tmp;
    return(pix);
}

QImage* LiquidStyle::adjustHSVImage(QImage &img, const QColor &c,bool blend, const QColor *bg) const{
    QColor bgColor(bg ? *bg : qApp->palette().active().background());

    if(img.depth() != 32)
        img = img.convertDepth(32);
    QImage *dest = new QImage(img.width(), img.height(), 32);
    dest->setAlphaBuffer(true);
    unsigned int *data = (unsigned int *)img.bits();
    unsigned int *destData = (unsigned int*)dest->bits();
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

        if(blend && alpha != 255 && alpha !=0){
            srcPercent = ((float)alpha)/255.0;
            destPercent = 1.0-srcPercent;
            destR = (int)((srcPercent*destR) + (destPercent*bgColor.red()));
            destG = (int)((srcPercent*destG) + (destPercent*bgColor.green()));
            destB = (int)((srcPercent*destB) + (destPercent*bgColor.blue()));
            alpha = 255;
        }
        destData[current] = qRgba(destR, destG, destB, alpha);
    }
    return(dest);
}

void LiquidStyle::clearImage(QImage &img) const{
    int x, y;
    int w = img.width();
    int h = img.height();
    unsigned int pixel = qRgba(0, 0, 0, 0);
    unsigned int *data;

    for(y=0; y < h; ++y){
        data = (unsigned int *)img.scanLine(y);
        for(x=0; x < w; ++x){
            data[x] = pixel;
        }
    }
}

void LiquidStyle::adjustHSV(QPixmap &pix, const QColor &c) const {
    QImage img = pix.convertToImage();
    QPixmap *result = adjustHSV(img, c, false);
    pix = *result;
    delete result;
}

ButtonTile* LiquidStyle::createButtonTile(const QColor &c,const QColor &bgColor,bool sunken) const {
    int x, y, delta;
    int destR, destG, destB, alpha;
    float srcPercent, destPercent;
    unsigned int bgPixel = bgColor.rgb();

    int srcR = c.red()+20;
    int srcG = c.green()+20;
    int srcB = c.blue()+20;

    unsigned int *data, *destData;
    LiquidStyle *ptr = const_cast<LiquidStyle*>(this);
    ButtonTile *tile;

    QImage img(39, 28, 32);
    if(!isPlain())
        img.setAlphaBuffer(true);
    else
        img.setAlphaBuffer(false);
    clearImage(img);

    // for sunken buttons things are easy, we just adjust the colors at
    // and offset it by 2
    if(sunken){
        for(y=0; y < 26; ++y){
            data = (unsigned int *)btnBorderImg->scanLine(y);
            destData = (unsigned int *)img.scanLine(y+2);
            for(x=0; x < 37; ++x){
                alpha = qAlpha(data[x]);
                delta = 255-qRed(data[x]);
                destR = srcR-delta; destG = srcG-delta; destB = srcB-delta;
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
                destData[x+2] = qRgba(destR, destG, destB, alpha);
            }
        }
    }
    else{
        // For unpressed buttons things are a little more complex. First
        // we color adjust the shadow with the background color then we
        // adjust the button to the button color. Finally we overlay the
        // button on the shadow, (in the same loop as color adjusting).
        // Technically we probably could of gotten away
        // with just adjusting both the shadow and the button to the button
        // color since the shadow is quite dark, but this is more correct and
        // would allow configurable shadow brightness. If the shadow was
        // just the button color, if someone configured it to be a rather
        // light brightness, and they had a dark background color it wouldn't
        // look right if based off the button color.
        //
        // Luckily this is only done once when the button is created for any
        // given color ;-)

        // first do the shadow
        for(y=0; y < 28; ++y){
            data = (unsigned int *)btnShadowImg->scanLine(y);
            destData = (unsigned int *)img.scanLine(y);
            for(x=0; x < 39; ++x){
                alpha = qAlpha(data[x]);
                if(alpha !=0){
                    delta = 255-qRed(data[x]);
                    destR = srcR-delta; destG = srcG-delta; destB = srcB-delta;
                    if(destR < 0) destR = 0;
                    if(destG < 0) destG = 0;
                    if(destB < 0) destB = 0;
                    if(destR > 255) destR = 255;
                    if(destG > 255) destG = 255;
                    if(destB > 255) destB = 255;
                    if(alpha != 255){
                        srcPercent = ((float)alpha)/255.0;
                        destPercent = 1.0-srcPercent;
                        destR = (int)((srcPercent*destR) + (destPercent*bgColor.red()));
                        destG = (int)((srcPercent*destG) + (destPercent*bgColor.green()));
                        destB = (int)((srcPercent*destB) + (destPercent*bgColor.blue()));
                        alpha = 255;
                    }
                    destData[x] = qRgba(destR, destG, destB, alpha);
                }
            }
        }
        // then the button and overlay
        for(y=0; y < 26; ++y){
            data = (unsigned int *)btnBorderImg->scanLine(y);
            destData = (unsigned int *)img.scanLine(y);
            for(x=0; x < 37; ++x){
                alpha = qAlpha(data[x]);
                if(alpha != 0){
                    delta = 255-qRed(data[x]);
                    destR = srcR-delta; destG = srcG-delta; destB = srcB-delta;
                    if(destR < 0) destR = 0;
                    if(destG < 0) destG = 0;
                    if(destB < 0) destB = 0;
                    if(destR > 255) destR = 255;
                    if(destG > 255) destG = 255;
                    if(destB > 255) destB = 255;
                    if(alpha != 255){
                        srcPercent = ((float)alpha)/255.0;
                        destPercent = 1.0-srcPercent;
                        if(qAlpha(destData[x])){
                            destR = (int)((srcPercent*destR) + (destPercent*qRed(destData[x])));
                            destG = (int)((srcPercent*destG) + (destPercent*qGreen(destData[x])));
                            destB = (int)((srcPercent*destB) + (destPercent*qBlue(destData[x])));
                        }
                        else{
                            destR = (int)((srcPercent*destR) + (destPercent*bgColor.red()));
                            destG = (int)((srcPercent*destG) + (destPercent*bgColor.green()));
                            destB = (int)((srcPercent*destB) + (destPercent*bgColor.blue()));
                        }
                        alpha = 255;
                    }
                    destData[x] = qRgba(destR, destG, destB, alpha);
                }
            }
        }
    }
    // Are we plain? If so fill in transparent pixels (must be after
    // overlay)
    if(isPlain()){
        for(y=0; y < 28; ++y){
            destData = (unsigned int *)img.scanLine(y);
            for(x=0; x < 39; ++x){
                if(qAlpha(destData[x]) == 0)
                    destData[x] = bgPixel;
            }
        }
    }
    QPixmap *pix = new QPixmap;
    pix->convertFromImage(img);
    tile = separateTiles(pix, sunken);
    if(sunken)
        ptr->btnDict.insert(c.rgb(), tile);
    else
        ptr->btnShadowedDict.insert(c.rgb(), tile);
    delete pix;
    return(tile);
}

// This separates, masks, and caches the separate button components. I used
// to do this in drawRoundButton() into temporary pixmaps whenever a button
// was drawn, but it's obviously faster to just do it once on creation.
ButtonTile* LiquidStyle::separateTiles(QPixmap *pix, bool sunken) const
{
    ButtonTile *tile = new ButtonTile();
    QPixmap *tmp;
    QBitmap *tmpMask;
    if(!sunken){
        // Top tiles
        tmp = new QPixmap(10, 10);
        bitBlt(tmp, 0, 0, pix, 0, 0, 10, 10);
        tile->setPixmap(TileTopLeft, tmp);
        tmp = new QPixmap(17, 10);
        bitBlt(tmp, 0, 0, pix, 10, 0, 17, 10);
        tile->setPixmap(TileTop, tmp);
        tmp = new QPixmap(12, 10);
        bitBlt(tmp, 0, 0, pix, 27, 0, 12, 10);
        tile->setPixmap(TileTopRight, tmp);

        // Middle tiles
        tmp = new QPixmap(10, 6);
        bitBlt(tmp, 0, 0, pix, 0, 10, 10, 6);
        tile->setPixmap(TileLeft, tmp);
        tmp = new QPixmap(17, 6);
        bitBlt(tmp, 0, 0, pix, 10, 10, 17, 6);
        tile->setPixmap(TileMiddle, tmp);
        tmp = new QPixmap(12, 6);
        bitBlt(tmp, 0, 0, pix, 27, 10, 12, 6);
        tile->setPixmap(TileRight, tmp);

        // Bottom tiles
        tmp = new QPixmap(10, 12);
        bitBlt(tmp, 0, 0, pix, 0, 16, 10, 12);
        tile->setPixmap(TileBtmLeft, tmp);
        tmp = new QPixmap(17, 12);
        bitBlt(tmp, 0, 0, pix, 10, 16, 17, 12);
        tile->setPixmap(TileBtm, tmp);
        tmp = new QPixmap(12, 12);
        bitBlt(tmp, 0, 0, pix, 27, 16, 12, 12);
        tile->setPixmap(TileBtmRight, tmp);

        if(!isPlain()){
            tmpMask = new QBitmap(10, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 0, 10, 10);
            tile->pixmap(TileTopLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 10, 0, 17, 10);
            tile->pixmap(TileTop)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(12, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 27, 0, 12, 10);
            tile->pixmap(TileTopRight)->setMask(*tmpMask);
            delete tmpMask;

            tmpMask = new QBitmap(10, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 10, 10, 6);
            tile->pixmap(TileLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 10, 10, 17, 6);
            tile->pixmap(TileMiddle)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(12, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 27, 10, 12, 6);
            tile->pixmap(TileRight)->setMask(*tmpMask);
            delete tmpMask;

            tmpMask = new QBitmap(10, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 16, 10, 12);
            tile->pixmap(TileBtmLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 10, 16, 17, 12);
            tile->pixmap(TileBtm)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(12, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 27, 16, 12, 12);
            tile->pixmap(TileBtmRight)->setMask(*tmpMask);
            delete tmpMask;
        }
    }
    else{
        // Top tiles
        tmp = new QPixmap(12, 12);
        bitBlt(tmp, 0, 0, pix, 0, 0, 12, 12);
        tile->setPixmap(TileTopLeft, tmp);
        tmp = new QPixmap(17, 12);
        bitBlt(tmp, 0, 0, pix, 12, 0, 17, 12);
        tile->setPixmap(TileTop, tmp);
        tmp = new QPixmap(10, 12);
        bitBlt(tmp, 0, 0, pix, 29, 0, 10, 12);
        tile->setPixmap(TileTopRight, tmp);

        // Middle tiles
        tmp = new QPixmap(12, 6);
        bitBlt(tmp, 0, 0, pix, 0, 12, 12, 6);
        tile->setPixmap(TileLeft, tmp);
        tmp = new QPixmap(17, 6);
        bitBlt(tmp, 0, 0, pix, 12, 12, 17, 6);
        tile->setPixmap(TileMiddle, tmp);
        tmp = new QPixmap(10, 6);
        bitBlt(tmp, 0, 0, pix, 29, 12, 10, 6);
        tile->setPixmap(TileRight, tmp);

        // Bottom tiles
        tmp = new QPixmap(10, 10);
        bitBlt(tmp, 0, 0, pix, 0, 18, 10, 10);
        tile->setPixmap(TileBtmLeft, tmp);
        tmp = new QPixmap(17, 10);
        bitBlt(tmp, 0, 0, pix, 12, 18, 17, 12);
        tile->setPixmap(TileBtm, tmp);
        tmp = new QPixmap(12, 10);
        bitBlt(tmp, 0, 0, pix, 27, 18, 12, 10);
        tile->setPixmap(TileBtmRight, tmp);

        if(!isPlain()){
            tmpMask = new QBitmap(12, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 0, 12, 12);
            tile->pixmap(TileTopLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 12, 0, 17, 12);
            tile->pixmap(TileTop)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(10, 12);
            bitBlt(tmpMask, 0, 0, pix->mask(), 29, 0, 10, 12);
            tile->pixmap(TileTopRight)->setMask(*tmpMask);
            delete tmpMask;

            tmpMask = new QBitmap(12, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 12, 12, 6);
            tile->pixmap(TileLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 12, 12, 17, 6);
            tile->pixmap(TileMiddle)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(10, 6);
            bitBlt(tmpMask, 0, 0, pix->mask(), 29, 12, 10, 6);
            tile->pixmap(TileRight)->setMask(*tmpMask);
            delete tmpMask;

            tmpMask = new QBitmap(10, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 0, 18, 10, 10);
            tile->pixmap(TileBtmLeft)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(17, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 10, 18, 17, 12);
            tile->pixmap(TileBtm)->setMask(*tmpMask);
            delete tmpMask;
            tmpMask = new QBitmap(12, 10);
            bitBlt(tmpMask, 0, 0, pix->mask(), 27, 18, 12, 10);
            tile->pixmap(TileBtmRight)->setMask(*tmpMask);
            delete tmpMask;
        }

    }
    return(tile);
}

void LiquidStyle::drawClearBevel(QPainter *p, int x, int y, int w, int h, const QColor &c, const QColor &bg) const
{
    LiquidStyle *ptr = const_cast<LiquidStyle*>(this);
    QPen oldPen = p->pen(); // headers need this
    int x2 = x+w-1;
    int y2 = y+h-1;
    // outer dark rect
    p->setPen(c.dark(115));
    p->drawLine(x+2, y, x2-2, y); // t
    p->drawLine(x, y+2, x, y2-2); // l
    p->drawPoint(x+1, y+1); // tl
    p->setPen(c.dark(150));
    p->drawLine(x+2, y2, x2-2, y2); // b
    p->drawLine(x2, y+2, x2, y2-2); // r
    p->drawPoint(x2-1, y2-1); // br
    p->setPen(c.dark(132));
    p->drawPoint(x2-1, y+1); // tr
    p->drawPoint(x+1, y2-1); // bl

    // inner top light lines
    p->setPen(c.light(105));
    p->drawLine(x+2, y+1, x2-2, y+1);
    p->drawLine(x+1, y+2, x2-1, y+2);
    p->drawLine(x+1, y+3, x+2, y+3);
    p->drawLine(x2-2, y+3, x2-1, y+3);
    p->drawPoint(x+1, y+4);
    p->drawPoint(x2-1, y+4);

    // inner bottom light lines
    p->setPen(c.light(110));
    p->drawLine(x+2, y2-1, x2-2, y2-1);
    p->drawLine(x+1, y2-2, x2-1, y2-2);
    p->drawLine(x+1, y2-3, x+2, y2-3);
    p->drawLine(x2-2, y2-3, x2-1, y2-3);
    p->drawPoint(x+1, y2-4);
    p->drawPoint(x2-1, y2-4);

    // inner left mid lines
    //p->setPen(c.light(105));
    p->setPen(c);
    p->drawLine(x+1, y+5, x+1, y2-5);
    p->drawLine(x+2, y+4, x+2, y2-4);

    // inner right mid lines
    p->drawLine(x2-1, y+5, x2-1, y2-5);
    p->drawLine(x2-2, y+4, x2-2, y2-4);

    // fill
    QPixmap *pix;
    if(h >= 32){
        pix = bevelFillDict.find(c.rgb());
        if(!pix){
            pix = new QPixmap(*bevelFillPix);
            adjustHSV(*pix, c);
            ptr->bevelFillDict.insert(c.rgb(), pix);
        }
    }
    else{
        pix = smallBevelFillDict.find(c.rgb());
        if(!pix){
            pix = new QPixmap(*smallBevelFillPix);
            adjustHSV(*pix, c);
            ptr->smallBevelFillDict.insert(c.rgb(), pix);
        }
    }
    p->drawTiledPixmap(x+3, y+3, w-6, h-6, *pix);
    // blend
    int red, green, blue;
    QColor btnColor(c.dark(130));
    red = (btnColor.red() >> 1) + (bg.red() >> 1);
    green = (btnColor.green() >> 1) + (bg.green() >> 1);
    blue = (btnColor.blue() >> 1) + (bg.blue() >> 1);
    btnColor.setRgb(red, green, blue);

    p->setPen(btnColor);
    p->drawPoint(x+1, y);
    p->drawPoint(x, y+1);
    p->drawPoint(x+1, y2);
    p->drawPoint(x, y2-1);

    p->drawPoint(x2-1, y);
    p->drawPoint(x2, y+1);
    p->drawPoint(x2-1, y2);
    p->drawPoint(x2, y2-1);

    p->setPen(oldPen);

}

void LiquidStyle::drawRoundButton(QPainter *painter, const QColorGroup &cg, const QColor &c, const QColor &back, int x, int y, int w, int h,
                                  bool supportPushDown, bool pushedDown, bool autoDefault, bool isHTML, int bgX, int bgY) const
{

    if(isHTML){
        drawRectangularButton(painter, c, x, y, w, h, pushedDown);
        return;
    }
    if((w < 21 || h < 21) && !autoDefault){
        drawClearBevel(painter, x, y, w, h, c, back);
        return;
    }

    LiquidStyle *ptr = const_cast<LiquidStyle*>(this);
    ButtonTile *tile = pushedDown ? btnDict.find(c.rgb()) :
        btnShadowedDict.find(c.rgb());

    if(!tile){
        tile = createButtonTile(c, cg.background(), pushedDown);
    }
    if(!tile){
        qWarning("Button tile is NULL!");
        return;
    }

    if(!tmpBtnPix)
        ptr->tmpBtnPix = new QPixmap(w, h);
    else if(w > tmpBtnPix->width() || h > tmpBtnPix->height()){
        // make temp pixmap size == largest button
        delete tmpBtnPix;
        ptr->tmpBtnPix = new QPixmap(w, h);
    }

    QPainter p;
    p.begin(tmpBtnPix);

    QPixmap *stipple = cg.brush(QColorGroup::Background).pixmap();
    if(!stipple) // button may have custom colorgroup
        stipple = qApp->
            palette().active().brush(QColorGroup::Background).pixmap();
    if(!isPlain() && bgX != -1 && bgY != -1 && stipple){
        p.drawTiledPixmap(0, 0, w, h, *stipple, bgX, bgY);
    }
    else
        p.fillRect(0, 0, w, h, back);

    if(!pushedDown){
        // corners
        // tiled fills
        if(w > 22){
            p.drawTiledPixmap(10, 0, w-22, 10, *tile->pixmap(TileTop));
            p.drawTiledPixmap(10, h-12, w-22, 12, *tile->pixmap(TileBtm));
        }
        if(h > 22){
            p.drawTiledPixmap(0, 10, 10, h-22, *tile->pixmap(TileLeft));
            p.drawTiledPixmap(w-12, 10, 12, h-22, *tile->pixmap(TileRight));
        }
        if(w > 22 && h > 22)
            p.drawTiledPixmap(10, 10, w-22, h-22, *tile->pixmap(TileMiddle));
        p.drawPixmap(0, 0, *tile->pixmap(TileTopLeft));
        p.drawPixmap(w-12, 0, *tile->pixmap(TileTopRight));
        p.drawPixmap(0, h-12, *tile->pixmap(TileBtmLeft));
        p.drawPixmap(w-12, h-12, *tile->pixmap(TileBtmRight));
    }
    else{
        if(w > 22){
            p.drawTiledPixmap(12, 0, w-22, 12, *tile->pixmap(TileTop));
            p.drawTiledPixmap(10, h-10, w-22, 12, *tile->pixmap(TileBtm));
        }
        if(h > 22){
            p.drawTiledPixmap(0, 12, 12, h-22, *tile->pixmap(TileLeft));
            p.drawTiledPixmap(w-10, 12, 10, h-22, *tile->pixmap(TileRight));
        }
        if(w > 22 && h > 22)
            p.drawTiledPixmap(12, 12, w-22, h-22, *tile->pixmap(TileMiddle));
        p.drawPixmap(0, 0, *tile->pixmap(TileTopLeft));
        p.drawPixmap(w-10, 0, *tile->pixmap(TileTopRight));
        p.drawPixmap(0, h-10, *tile->pixmap(TileBtmLeft));
        p.drawPixmap(w-12, h-10, *tile->pixmap(TileBtmRight));
    }
    p.end();
    painter->drawPixmap(x, y, *tmpBtnPix, 0, 0, w, h);
}

void LiquidStyle::drawRectangularButton(QPainter *p, const QColor &c, int x, int y, int w, int h, bool sunken, bool hover, bool isCombo) const
{
    QPen oldPen(p->pen());
    int x2 = x+w-1;
    int y2 = y+h-1;

    QColor c2(isCombo ? hover || sunken ? c :
              qApp->palette().active().background() : c);

    p->setPen(c2.dark(130));
    p->drawRect(x, y, w, h);
    p->setPen(sunken ? c2.dark(120) : c2.light(110));
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);
    p->setPen(sunken ? c2.light(110) : c2.dark(120));
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);

    QPixmap *pix;
    if(h >= 32){
        pix = bevelFillDict.find(c2.rgb());
        if(!pix){
            pix = new QPixmap(*bevelFillPix);
            adjustHSV(*pix, c2);
            const_cast<LiquidStyle*>(this)->
                bevelFillDict.insert(c2.rgb(), pix);
        }
    }
    else{
        pix = smallBevelFillDict.find(c2.rgb());
        if(!pix){
            pix = new QPixmap(*smallBevelFillPix);
            adjustHSV(*pix, c2);
            const_cast<LiquidStyle*>(this)->
                smallBevelFillDict.insert(c2.rgb(), pix);
        }
    }
    p->drawTiledPixmap(x+2, y+2, w-4, h-4, *pix);

    if(isCombo){
        SFlags flags = Style_Default;
        int ah = h / 3;
        int aw = ah;
        int ax = w - aw - 6;
        int ay = (h - ah) / 2;
        x = x2-16;

        p->setPen(c.dark(130));
        p->drawRect(x, y, 17, h);
        p->setPen(sunken ? c.dark(120) : c.light(110));
        p->drawLine(x+1, y+1, x2-1, y+1);
        p->drawLine(x+1, y+1, x+1, y2-1);
        p->setPen(sunken ? c.light(110) : c.dark(120));
        p->drawLine(x2-1, y+1, x2-1, y2-1);
        p->drawLine(x+1, y2-1, x2-1, y2-1);
        pix = smallBevelFillDict.find(c.rgb());
        if(!pix){
            pix = new QPixmap(*smallBevelFillPix);
            adjustHSV(*pix, c);
            const_cast<LiquidStyle*>(this)->
                smallBevelFillDict.insert(c.rgb(), pix);
        }
        p->drawTiledPixmap(x+2, y+2, 13, h-4, *pix);

        p->setPen(Qt::black);

        flags |= Style_Enabled;
        // Are we "pushed" ?
        if(sunken)
            flags |= Style_Sunken;
        drawPrimitive(PE_ArrowDown, p, QRect(ax, ay, aw, ah),
                      qApp->palette().active(), flags);
    }
    p->setPen(oldPen);

}

void LiquidStyle::drawEditFrame(QPainter *p, const QRect &r,
                                const QColorGroup &cg, bool isHTML) const
{
    QColor fill(cg.background().dark(105));
    QColor light1(cg.background().dark(115));
    QColor light2(cg.background().dark(120));

    QColor dark1(cg.background().dark(130));
    QColor dark2(cg.background().dark(150));
    QColor dark3(cg.background().dark(200));

    if(!isHTML){
        p->setPen(fill);
        p->drawPoint(r.x(), r.y());
        p->drawPoint(r.x(), r.bottom());
        p->drawPoint(r.right(), r.y());
        p->drawPoint(r.right(), r.bottom());

        // outer rect
        // top
        p->setPen(light1);
        p->drawPoint(r.x()+1, r.y());
        p->drawPoint(r.right()-1, r.y());
        p->drawPoint(r.x(), r.y()+1);
        p->drawPoint(r.right(), r.y()+1);
        // bottom
        p->drawPoint(r.x(), r.bottom()-1);
        p->drawPoint(r.right(), r.bottom()-1);
        p->drawPoint(r.x()+1, r.bottom());
        p->drawPoint(r.right()-1, r.bottom());
        // top and left
        p->setPen(light2);
        p->drawLine(r.x()+2, r.y(), r.right()-2, r.y());
        p->drawLine(r.x(), r.y()+2, r.x(), r.bottom()-2);
        // bottom and right
        p->setPen(dark1);
        p->drawLine(r.x()+2, r.bottom(), r.right()-2, r.bottom());
        p->drawLine(r.right(), r.y()+2, r.right(), r.bottom()-2);
    }
    else{
        p->setPen(light2);
        p->drawLine(r.x(), r.y(), r.right(), r.y());
        p->drawLine(r.x(), r.y(), r.x(), r.bottom());
        p->setPen(dark1);
        p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
        p->drawLine(r.right(), r.y(), r.right(), r.bottom());
    }
    // inner rect
    p->setPen(dark2);
    p->drawPoint(r.x()+1, r.y()+1);
    p->drawPoint(r.x()+1, r.bottom()-1);
    p->drawPoint(r.right()-1, r.y()+1);
    p->drawPoint(r.right()-1, r.bottom()-1);
    p->setPen(dark3);
    p->drawLine(r.x()+2, r.y()+1, r.right()-2, r.y()+1);
    p->drawLine(r.x()+2, r.bottom()-1, r.right()-2, r.bottom()-1);
    p->drawLine(r.x()+1, r.y()+2, r.x()+1, r.bottom()-2);
    p->drawLine(r.right()-1, r.y()+2, r.right()-1, r.bottom()-2);
}

bool LiquidStyle::isHTMLWidget(const QWidget *widget) const
{
    const QObject *w = widget->parent();
    if(w){
        if(!w->inherits("QClipperWidget"))
            return(false);
        w = w->parent();
        if(w){
            w = w->parent();
            if(w && w->inherits("KHTMLView"))
                return(true);
        }
    }
    return(false);
}

void LiquidStyle::drawHTMLCBBorder(const QPixmap &pix, const QColor &c) const
{
    QPainter p;
    p.begin(&pix);
    p.setPen(c.dark(200));
    p.drawRect(0, 0, 16, 16);
    p.end();
}
