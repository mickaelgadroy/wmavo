
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Benoit Jacob

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#include "settingswidget.h"


SettingsWidget::SettingsWidget()
{
  createSettingsWidget() ;
  connectSignal() ;
}


SettingsWidget::~SettingsWidget()
{
  if( m_settingsWidget != NULL )
  {
    // All feature are deleted by the object itself.
    delete m_settingsWidget ;
  }
}


/**
  * Get the widget which contains the "setting widget".
  */
QWidget* SettingsWidget::getSettingsWidget()
{
  return m_settingsWidget ;
}


/**
  * Help to redirect a signal to transmit an information about the sensitive of the Wiimote.
  * @param wmSens The new sensitive of the Wiimote.
  */
void SettingsWidget::changedWmSensitiveRedirect( int wmSens )
{
  emit changedWmSensitive( wmSens ) ;
}


/**
  * Create the settings widget.
  */
void SettingsWidget::createSettingsWidget()
{
  if( m_settingsWidget == NULL )
  {
    m_settingsWidget = new QWidget ;

    QLabel *lblWmSensitive = new QLabel(tr("Wiimote sensitive :")) ;
    QLabel *lblWmSMoins = new QLabel(tr("(-)")) ;
    QLabel *lblWmSPlus = new QLabel(tr("(+)")) ;
    m_wmSensitiveSlider = new QSlider( Qt::Horizontal ) ;
    m_wmSensitiveSlider->setMaximum( PLUGIN_WM_SENSITIVE_MAX ) ;
    m_wmSensitiveSlider->setMinimum( PLUGIN_WM_SENSITIVE_MIN ) ;
    m_wmSensitiveSlider->setValue( PLUGIN_WM_SENSITIVE_DEFAULT ) ;
    m_wmSensitiveSlider->setTickInterval( 1 ) ;
    //m_addHydrogensCheck = new QCheckBox( "Adjust Hydrogen" ) ;

    QLabel *lblWmPointSize = new QLabel(tr("Font Size :")) ;
    QLabel *lblWmMoins = new QLabel(tr("(-)")) ;
    QLabel *lblWmPlus = new QLabel(tr("(+)")) ;
    m_wmPointSizeFontSlider = new QSlider( Qt::Horizontal ) ;
    m_wmPointSizeFontSlider->setMaximum( (int)(WMTOOL_POINTSIZE_RATIO_MAX * 10.0f) ) ;
    m_wmPointSizeFontSlider->setMinimum( (int)(WMTOOL_POINTSIZE_RATIO_MIN * 10.0f) ) ;
    m_wmPointSizeFontSlider->setValue( (int)(WMTOOL_POINTSIZE_RATIO_DEFAULT * 10.0f) ) ;
    m_wmPointSizeFontSlider->setTickInterval( 1 ) ;

    QLabel *lblVibration = new QLabel(tr("Vibration :")) ;
    m_checkBoxActivateVibration = new QCheckBox( "ON/OFF" ) ;
    m_checkBoxActivateVibration->setChecked(PLUGIN_WM_VIBRATION_ONOFF) ;


    QVBoxLayout *vBoxSens=new QVBoxLayout() ;
    QHBoxLayout *hBoxSens=new QHBoxLayout() ;
    hBoxSens->addWidget( lblWmSMoins ) ;
    hBoxSens->addWidget( m_wmSensitiveSlider ) ;
    hBoxSens->addWidget( lblWmSPlus ) ;
    vBoxSens->addWidget( lblWmSensitive ) ;
    vBoxSens->addLayout( hBoxSens ) ;
    vBoxSens->addStretch( 1 ) ;

    QVBoxLayout *vBoxPointSize=new QVBoxLayout() ;
    QHBoxLayout *hBoxPointSize=new QHBoxLayout() ;
    hBoxPointSize->addWidget( lblWmMoins ) ;
    hBoxPointSize->addWidget( m_wmPointSizeFontSlider ) ;
    hBoxPointSize->addWidget( lblWmPlus ) ;
    vBoxPointSize->addWidget( lblWmPointSize ) ;
    vBoxPointSize->addLayout( hBoxPointSize ) ;
    vBoxPointSize->addStretch( 1 ) ;

    QHBoxLayout *hBoxVibration=new QHBoxLayout() ;
    hBoxVibration->addWidget( lblVibration ) ;
    hBoxVibration->addWidget( m_checkBoxActivateVibration ) ;


    QVBoxLayout *vBox=new QVBoxLayout() ;
    vBox->addLayout( vBoxSens ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( vBoxPointSize ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( hBoxVibration ) ;
    vBox->addStretch( 1 ) ;

    m_settingsWidget->setLayout( vBox ) ;
  }
}


/**
  * Connect all signal used by this settings Widget ...
  */
void SettingsWidget::connectSignal()
{
  // Signals connected in wmEx class by the rederected signal ...
  bool isConnect = connect( m_wmSensitiveSlider,  SIGNAL(valueChanged(int)),
                            this, SLOT(changedWmSensitiveRedirect(int)) ) ;
  if( !isConnect )
    mytoolbox::dbgMsg( "Problem connection signal : m_wmSensitiveSlider.valueChanged() -> wmTool.changedWmSensitiveRedirect() !!" ) ;

  isConnect = connect( m_wmPointSizeFontSlider,  SIGNAL(valueChanged(int)),
                       this, SLOT(setSizeRatioFont(int)) ) ;
  if( !isConnect )
    mytoolbox::dbgMsg( "Problem connection signal : m_wmPointSizeFontSlider.valueChanged() -> wmTool.setSizeRatioFont() !!" ) ;
}