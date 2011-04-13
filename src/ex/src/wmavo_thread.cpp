
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

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


#include "wmavo_thread.h"

WmAvoThread::WmAvoThread(QObject *parent/*, GLWidget *widget*/)
  : QThread(parent),/* m_widget(widget),*/ m_wmavo(NULL),
  m_updateOpMode(false), m_operatingMode(0),
  m_updateRumble(false), m_start(false), m_continu(true), m_loop(true),
  m_addAtom(false), m_delAtom(false),
  m_updateSizeWidget(false), m_x(0), m_y(0), m_width(0), m_height(0),
  m_menuMode(false)
{
  //cout << "Constructor WmAvoThread" << endl ;

  m_posAtomStock.clear() ;
}

WmAvoThread::~WmAvoThread()
{
  // Normaly, it's not necessary.
  if( m_wmavo != NULL )
  {
    delete m_wmavo ;
    m_wmavo = NULL ;
    wait() ;
    // Wait until run() has exited before the base class destructor is invoked.
  }
}

void WmAvoThread::run()
{
  WmAvo *wmavo=new WmAvo(2) ; //widget -> cam -> mutex ...
  int isConnected=0 ;
  bool update=false ; // To know if an Avogadro action is realized.
  bool lastUpdate=false ; // Making a final action even if there is no update listed Wiimote. (fr:pas de mise à jour coté Wiimote).

  m_wmavo = wmavo ;

  //m_mutex.lock() ;
  isConnected=wmavo->wmConnect() ;
  //m_mutex.unlock() ;

  emit wmConnected(isConnected) ;

  if( isConnected )
  {
    // To wake up the extension class earlier.
    // To update the wmTool class to display some Wiimote messages.

    updateDataTransfert() ;
    emit wmPolled( m_wmDataTransfert ) ;

    while( isConnected )
    {
      update = wmavo->wmPoll() ;

      if( update || lastUpdate ) // lastUpdate : Let "wmextension" to finish some action.
      {
        if( WMAVO_IS2(wmavo->wmGetActions(),WMAVO_ATOM_MOVE)) // A last action must be realised.
          lastUpdate = true ;
        else
          lastUpdate = false ;

        updateDataTransfert() ;
        emit wmPolled( m_wmDataTransfert ) ;
      }

      this->msleep( WMAVOTH_SLEEPBEFORE_NEXTROUND ) ;
      isConnected = wmavo->wmIsConnected() ;

      // Normaly the plugin have had time to update attributs.
      updateWmAttributs() ;
      // Update attributs of wmavo, and active some features.
    }

    emit wmDisconnected() ;
  }

  if( wmavo != NULL )
  {
    delete wmavo ;
    m_wmavo = NULL ;
  }
}

void WmAvoThread::quit()
{
  // Stop the thread.
  if( m_wmavo != NULL )
  {
    m_wmavo->wmDisconnect() ;
    wait() ;
    // Wait until run() has exited before the base class destructor is invoked.
  }

  QThread::quit() ;
}

void WmAvoThread::setWmOperatingMode( int opMode )
{
  if( m_wmavo != NULL )
  {
    if( m_operatingMode != opMode )
    {
      m_mutex.lock() ;
      m_updateOpMode = true ;
      m_operatingMode = opMode ;
      m_mutex.unlock() ;
    }
  }
}

void WmAvoThread::setWmRumble( bool start, bool continu, bool loop, int gradual )
{
  if( m_wmavo != NULL )
  {
    m_mutex.lock() ;

    if( m_start != start )
    {
      m_start = start ;
      m_updateRumble = true ;
    }

    if( m_continu != continu )
    {
      m_continu = continu ;
      m_updateRumble = true ;
    }

    if( m_loop!=loop || (!m_continu && !m_loop && m_start) )
      // Particularity, it is necessary to start rumble when we just
      // need ONE rumble (and to compense default value in the prototyp
      // method of setWmRumble)
    {
      m_loop = loop ;
      m_updateRumble = true ;
    }

    if( m_gradual != gradual )
    {
      m_gradual = gradual ;
      m_updateRumble = true ;
    }

    //m_updateRumble = true ;
    //cout << " wmThread.m_updateRumble:" << m_updateRumble << endl ;

    m_mutex.unlock() ;
  }
}

void WmAvoThread::setWmRumble( int gradual )
{
  if( m_wmavo != NULL )
  {
    if( m_gradual != gradual )
    {
      m_mutex.lock() ;
      m_gradual = gradual ;
      m_updateRumbleGrad = true ;
      m_mutex.unlock() ;
    }
  }
}

void WmAvoThread::setWmSizeWidget( int x, int y, int width, int height )
{
  if( m_wmavo != NULL )
  {
    if( m_x!=x || m_y!=y || m_width!=width || m_height!=height )
    {
      m_mutex.lock() ;
      m_x = x ;
      m_y = y ;
      m_width = width ;
      m_height = height ;
      m_updateSizeWidget = true ;
      m_mutex.unlock() ;
    }
  }
}

void WmAvoThread::setWmMenuMode( bool menuMode )
{
  if( m_wmavo != NULL )
  {
    // if( m_menuMode != menuMode )
    //{
    // In comment, because this is the wmavo class which decides to pass in menu
    // mode, so wmavoThread does not know the state of this attribut.
    m_mutex.lock() ;
    m_menuMode = menuMode ;
    m_updateMenuMode = true ;
    m_mutex.unlock() ;
    //}
  }
}

void WmAvoThread::setWmActionMode( bool actionMode )
{
  if( m_wmavo != NULL )
  {
    //if( m_menuMode == actionMode )
    //{
    m_mutex.lock() ;
    m_menuMode = !actionMode ;
    m_updateMenuMode = true ;
    m_mutex.unlock() ;
    //}
  }
}


void WmAvoThread::setWmSensitive( int wmSensitive )
{
  if( m_wmavo != NULL )
  {
    if( m_wmSensitive != wmSensitive )
    {
      m_mutex.lock() ;
      m_wmSensitive = wmSensitive ;
      m_updateWmSensitive = true ;
      m_mutex.unlock() ;
    }
  }
}


void WmAvoThread::updateWmAttributs()
{
  if( m_wmavo != NULL )
  {
    m_mutex.lock() ;

    if( m_updateOpMode )
    {
      m_updateOpMode = false ;
      m_wmavo->setWmOperatingMode(m_operatingMode) ;
    }

    if( m_updateRumble )
    {
      //cout << "update Rumble: " << m_start << " " << m_continu << " " << m_loop << " " << m_gradual << endl ;
      m_updateRumble = false ;
      m_wmavo->wmSetRumble( m_start, m_continu, m_loop, m_gradual ) ;
    }

    if( m_updateRumbleGrad )
    {
      //cout << "update RumbleGrad" << endl ;
      m_updateRumbleGrad = false ;
      m_wmavo->wmSetRumble( m_gradual ) ;
    }

    if( m_updateSizeWidget )
    {
      m_updateSizeWidget = false ;
      m_wmavo->wmSetSizeWidget( m_x, m_y, m_width, m_height ) ;
    }

    if( m_updateMenuMode )
    {
      m_updateMenuMode = false ;
      m_wmavo->wmSetMenuMode( m_menuMode ) ;
    }

    if( m_updateWmSensitive )
    {
      //cout << "update to wmavo" << endl ;
      m_updateWmSensitive = false ;
      m_wmavo->wmSetSensitive( m_wmSensitive ) ;
    }

    m_mutex.unlock() ;
  }
}

void WmAvoThread::updateDataTransfert()
{
  m_wmDataTransfert.posCursor = m_wmavo->getPosCursor() ;
  m_wmDataTransfert.wmActions = m_wmavo->wmGetActions() ;
  m_wmDataTransfert.pos3dCur = m_wmavo->getPos3dCurrent() ;
  m_wmDataTransfert.pos3dLast = m_wmavo->getPos3dLast() ;
  m_wmDataTransfert.angleCamRotateXDeg = m_wmavo->getAngleCamRotateXDeg() ;
  m_wmDataTransfert.angleCamRotateYDeg = m_wmavo->getAngleCamRotateYDeg() ;
  m_wmDataTransfert.distCamTranslateX = m_wmavo->getDistCamTranslateX() ;
  m_wmDataTransfert.distCamTranslateY = m_wmavo->getDistCamTranslateY() ;
  m_wmDataTransfert.distCamZoom = m_wmavo->getDistCamZoom() ;
  m_wmDataTransfert.nbDotsDetected = m_wmavo->getWiimote()->IR.GetNumDots() ;
  m_wmDataTransfert.nbSourcesDetected = m_wmavo->getWiimote()->IR.GetNumSources() ;
  m_wmDataTransfert.distBetweenSources = (int)m_wmavo->getWiimote()->IR.GetDistance() ;
}
