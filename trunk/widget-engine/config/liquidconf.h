#ifndef LIQUID_CONF_H
#define LIQUID_CONF_H

#include <kstddirs.h>
#include <kcolorbutton.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlayout.h>

class QSlider;
class KColorButton;
class QGroupBox;
class QVButtonGroup;
class QRadioButton;
class QCheckBox;
class QLabel;
class LiquidStyleConfigPrefMenu;
class LiquidStyleConfigPrefColors;
class LiquidStyleConfigPrefMisc;

class LiquidStyleConfig : public QWidget
{
    Q_OBJECT
public:

    LiquidStyleConfig(QWidget* parent);
    ~LiquidStyleConfig();
public slots:
    
    void save();
    void load();
    void defaults();
signals:
    void changed(bool);  
// protected:
//     QSlider *opacitySlider;

private:
    LiquidStyleConfigPrefMenu *m_pageOne;    
    LiquidStyleConfigPrefColors *m_pageTwo;
    LiquidStyleConfigPrefMisc *m_pageThree;
    
};

class LiquidStyleConfigPrefMenu : public QFrame {
    Q_OBJECT
public:
    LiquidStyleConfigPrefMenu(QWidget *parent = 0);
    QVButtonGroup *btnGroup;
    QGroupBox *editGroup;
    enum Buttons{None=0, StippledBg, StippledBtn,Custom};
signals:
    void changed(bool);      
protected:
    KColorButton *colorBtn;
    KColorButton *fgBtn;
    QCheckBox *shadowBtn;
    friend void LiquidStyleConfig::load(); 
    friend void LiquidStyleConfig::save();   
protected slots:
    void slotColorOptionChanged();
    void slotBtnGroupClicked(int id);
};

class LiquidStyleConfigPrefColors : public QFrame {
    Q_OBJECT
public:
    LiquidStyleConfigPrefColors(QWidget *parent = 0);
    QCheckBox *usePanelCustomBtn;
    KColorButton *panelColorBtn;
    
signals:
    void changed(bool);  
protected:    
    void adjustHSV(QImage &img, QImage &dest, const QColor &c,const QColor &bgColor);
    QCheckBox *useCustomColorBtn;
    KColorButton *radioOnColorBtn, *radioOffColorBtn;
    KColorButton *cbOnColorBtn, *cbOffColorBtn;
    KColorButton *sbSliderColorBtn, *sbGrooveColorBtn;
    KColorButton *tabOnColorBtn, *tabOffColorBtn;
    QLabel *radioOnLbl, *radioOffLbl;
    QLabel *cbOnLbl, *cbOffLbl;
    QLabel *sbSliderLbl, *sbGrooveLbl;
    QLabel *tabOnLbl, *tabOffLbl;    
    friend void LiquidStyleConfig::load(); 
    friend void LiquidStyleConfig::save();
    friend void LiquidStyleConfig::defaults();  
protected slots:
    void slotWidgetColorChanged(const QColor &c);
    void slotUseCustomColorClicked(bool on);
    void slotPanelColorChanged(const QColor &c);
    void slotUsePanelCustomClicked(bool on);
};

class LiquidStyleConfigPrefMisc : public QFrame {
    Q_OBJECT
public:
    LiquidStyleConfigPrefMisc(QWidget *parent = 0);
signals:
    void changed(bool);  
protected:
    QCheckBox *stippleBgBtn, *stipplePanelBtn;
    QSlider *stippleContrastSlider;    
    QCheckBox *reverseColorBtn, *animProgressBar;
    QCheckBox *useTbFrameBtn;    
    friend void LiquidStyleConfig::load(); 
    friend void LiquidStyleConfig::save(); 
    protected slots:
    void slotColorOptionChanged();
    void slotSliderChanged(int val);

};
#endif
