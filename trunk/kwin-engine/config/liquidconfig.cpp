//////////////////////////////////////////////////////////////////////////////
// exampleconfig.cc
// -------------------
// Config module for Example window decoration
// -------------------
// Copyright (c) 2003 David Johnson <david@usermode.org>
// Please see the header file for copyright and license information.
//////////////////////////////////////////////////////////////////////////////

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

#include "liquidconfig.h"
#include "configdialog.h"

LiquidConfig::LiquidConfig(KConfig* config, QWidget* parent) : QObject(parent), config_(0), dialog_(0) {
    // create the configuration object
    config_ = new KConfig("kwinliquidplusrc");
    KGlobal::locale()->insertCatalogue("kwin_liquidplus_config");

    // create and show the configuration dialog
    dialog_ = new ConfigDialog(parent);
    dialog_->show();

    // load the configuration
    load(config_);

    // setup the connections
    connect(dialog_->titlealign, SIGNAL(clicked(int)), this, SIGNAL(changed()));
    connect(dialog_->useShadowedText, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
}

LiquidConfig::~LiquidConfig(){
    if (dialog_) delete dialog_;
    if (config_) delete config_;
}

void LiquidConfig::defaults(){
    QRadioButton *button = (QRadioButton*)dialog_->titlealign->child("AlignHCenter");
    if (button) button->setChecked(true);    
    dialog_->useShadowedText->setChecked(true);
}

void LiquidConfig::load(KConfig*){
    config_->setGroup("General");
    QString value = config_->readEntry("TitleAlignment", "AlignHCenter");
    QRadioButton *button = (QRadioButton*)dialog_->titlealign->child((const char *)value.latin1());
    if (button) button->setChecked(true);
    bool useShadowedText = config_->readBoolEntry("UseShadowedText", true);
    dialog_->useShadowedText->setChecked(useShadowedText);
}

void LiquidConfig::save(KConfig*){
    config_->setGroup("General");

    QRadioButton *button = (QRadioButton*)dialog_->titlealign->selected();
    if (button) config_->writeEntry("TitleAlignment", QString(button->name()));
    config_->writeEntry("UseShadowedText", dialog_->useShadowedText->isChecked());
    config_->sync();
}

extern "C" {
    QObject* allocate_config(KConfig* config, QWidget* parent) {
        return (new LiquidConfig(config, parent));
    }
}

#include "liquidconfig.moc"
