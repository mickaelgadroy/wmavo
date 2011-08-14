
/*******************************************************************************
  Copyright (C) 2011 Mickael Gadroy

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

#include "wmdevice.h"
#include "wmrumble.h"

namespace InputDevice
{
  WmDeviceData_from::WmDeviceData_from()
    : DeviceData_from(), m_data(NULL)
  {
  }

  WmDeviceData_from::WmDeviceData_from( CWiimoteData *wmData )
    : DeviceData_from(), m_data(wmData)
  {
  }

  WmDeviceData_from::WmDeviceData_from( const WmDeviceData_from &wmData )
    : DeviceData_from()
  {
    m_data = new CWiimoteData() ;
    *m_data = *wmData.m_data ;
  }

  WmDeviceData_from::~WmDeviceData_from()
  {
    if( m_data != NULL )
    {
      delete m_data ;
      m_data = NULL ;
    }
  }

  // To be sur to have 2 distinct objects.
  void WmDeviceData_from::operator=( const WmDeviceData_from& wmDataFrom )
  {
    if( this!=&wmDataFrom && m_data!=wmDataFrom.m_data )
    {
      if( m_data == NULL )
        m_data = new CWiimoteData() ;

      *m_data = *(wmDataFrom.m_data) ;
    }
  }

  WmDeviceData_to::WmDeviceData_to()
    : DeviceData_to(),
      m_setRumble(NULL), m_updateRumble(false),
      m_setLED(0), m_updateLED(false)
  {
    m_setRumble = new RumbleSettings() ;
  }

  WmDeviceData_to::WmDeviceData_to( RumbleSettings* rumble, int LED )
    : DeviceData_to(), 
      m_setRumble(rumble), m_updateRumble(true),
      m_setLED(LED), m_updateLED(false)
  {
    if( rumble == NULL )
      m_setRumble = new RumbleSettings() ;
  }

  WmDeviceData_to::WmDeviceData_to( const WmDeviceData_to& data )
    : DeviceData_to(), 
      m_updateRumble(true),
      m_updateLED(false)
  {
    m_setLED = data.m_setLED ;
    *m_setRumble = *(data.m_setRumble) ;
  }

  WmDeviceData_to::~WmDeviceData_to()
  {
    if( m_setRumble != NULL )
    {
      delete m_setRumble ;
      m_setRumble = NULL ;
    }
  }

  bool WmDeviceData_to::getRumble( RumbleSettings &rumble_out )
  { 
    bool up=m_updateRumble ; 
    m_updateRumble = false ; 
    rumble_out = *m_setRumble ; 
    return up ; 
  }
  
  void WmDeviceData_to::setRumble( const RumbleSettings &rumble )
  { 
    m_updateRumble = true ; 
    *m_setRumble = rumble ; 
  }

  bool WmDeviceData_to::getLED( int &LED )
  { 
    bool up=m_updateLED ; 
    m_updateLED = false ; 
    LED = m_setLED ; 
    return up ; 
  }
   
  void WmDeviceData_to::setLED( int LED )
  { 
    m_updateLED = true ; 
    m_setLED = LED ; 
  }

  
  void WmDeviceData_to::operator=( const WmDeviceData_to& wmDataTo )
  {
    if( this!=&wmDataTo && m_setRumble!=wmDataTo.m_setRumble )
    {
      if( m_setRumble == NULL )
        m_setRumble = new RumbleSettings() ;

      *m_setRumble = *(wmDataTo.m_setRumble) ;
      m_updateRumble = wmDataTo.m_updateRumble ;
      m_setLED = wmDataTo.m_setLED ;
      m_updateLED = wmDataTo.m_updateLED ;
    }
  }

  void WmDeviceData_to::initAttributsToZero()
    {
      if( m_setRumble != NULL )
      {
        delete m_setRumble ;
        m_setRumble = NULL ;
      }
      
      m_setRumble = new RumbleSettings() ;
      m_updateRumble = false ;
      m_setLED = 0 ;
      m_updateLED = false ;
    } ;

  
  WmDevice::WmDevice()
    : Device(),
      m_cirBufferFrom(NULL), m_cirBufferTo(NULL),
      m_isRunning(false),
      m_wii(NULL), m_wm(NULL), m_nc(NULL),
      m_rumble(NULL), 
      m_hasWm(false), m_hasNc(false)
  {
    m_cirBufferFrom = new WIWO_sem<WmDeviceData_from*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_cirBufferTo = new WIWO_sem<WmDeviceData_to*>( CIRBUFFER_DEFAULT_SIZE ) ;
  }

  WmDevice::~WmDevice()
  {
    if( m_cirBufferFrom != NULL )
    {
      delete m_cirBufferFrom ;
      m_cirBufferFrom = NULL ;
    }

    if( m_cirBufferTo != NULL )
    {
      delete m_cirBufferTo ;
      m_cirBufferTo = NULL ;
    }

    if( m_rumble != NULL )
    {
      delete m_rumble ;
      m_rumble = NULL ;
    }

    deleteWii() ;
  }

  void WmDevice::deleteWii()
  {
    if( m_wii != NULL )
    { delete m_wii ; m_wii = NULL ; }

    m_wm = NULL ;
    m_nc = NULL ;
    m_hasWm = false ;
    m_hasNc = false ;
  };

  /**
    * Get the last state data of Wiimote (blocking call).
    */
  WmDeviceData_from* WmDevice::getDeviceDataFrom()
  { 
    WmDeviceData_from *data=NULL ;
    m_cirBufferFrom->popFront( data ) ;
    return data ;
  }

  /**
    * Set the last wanted state for Wiimote (blocking call).
    */
  void WmDevice::setDeviceDataTo( const WmDeviceData_to& wmDataTo )
  { 
    WmDeviceData_to *data=new WmDeviceData_to() ;
    *data = wmDataTo ;
    m_cirBufferTo->pushBack(data) ;
  }

  /**
    * Realize the connection of the Wiimote :
    *  - search a Wiimote ;
    *  - connect to the Wiimote ;
    *  - initiate some Wiimote elements.
    */
  int WmDevice::connectWm()
  {
    int nbFound=0, nbConnect=0 ;
    CWiimote *wm=NULL ;

    // Let to restart the connection properly.
    deleteWii() ;
    m_wii = new CWii(1) ;

    // Searching.
    mytoolbox::dbgMsg( "Searching for wiimotes... Turn them on !" ) ;
    mytoolbox::dbgMsg( "Press 1+2 !" ) ;
    nbFound = m_wii->Find( (int)WMAVO_CONNECTION_TIMEOUT ) ;
    //cout << "Found " << nbFound << " wiimotes" << endl ;

    // Under windows, even not found and not connected, 
    // if there is a wiimote in the device manager,
    // it connects ...
    if( nbFound > 0 )
    {
      //cout << "Connecting to wiimotes..." << endl ;
      std::vector<CWiimote*>& wms=m_wii->Connect() ;
      nbConnect = wms.size() ;
      wm = wms.at(0) ;
    }

    if( nbConnect <= 0 )
    {
      deleteWii() ;
    }
    else
    {
      m_hasWm = true ;
      m_wm = wm ;

      m_wm->SetLEDs( CWiimote::LED_1 ) ; // Light LED 1.
      m_wm->IR.SetMode( CIR::ON ) ; // Activate IR.
      m_wm->IR.SetSensitivity(WMAVO_IRSENSITIVITY) ;

      // For continue comm. (already with IR enable)
      //m_wm->SetFlags( CWiimote::FLAG_CONTINUOUS, 0x0 ) ;
      // For smoothed angle. (not effective)
      //m_wm->SetFlags( CWiimote::FLAG_SMOOTHING, 0x0 ) ;        

      //Rumble for 0.2 seconds as a connection ack
      m_wm->SetRumbleMode(CWiimote::ON);
      #if defined WIN32 || defined _WIN32
      Sleep(200);
      #else
      usleep(200000);    
      #endif
      //wiimote->SetRumbleMode(CWiimote::OFF); !!! (wiimote) should come directly from wiimote_t !!!
      m_wm->SetRumbleMode(CWiimote::OFF);

      // Activate the advanced rumble feature.
      m_rumble = new WmRumble(this) ;
      m_rumble->setRumbleEnabled( true ) ;

      // Connect (=Get) the nunchuk.
      connectNc() ;
    }

    return nbConnect ;
  }

  bool WmDevice::connectNc()
  {
    m_nc = NULL ;

    if( m_wm != NULL
        && m_wm->ExpansionDevice.GetType() == m_wm->ExpansionDevice.TYPE_NUNCHUK )
    {
      m_nc = &(m_wm->ExpansionDevice.Nunchuk) ;
    }

    return m_hasNc ;
  }


  bool WmDevice::connectAndStart()
  {
    if( connectWm() > 0 )
    { // Running!

      this->connect( &m_deviceThread, SIGNAL(started()), SLOT(runPoll()) ) ;
      this->moveToThread( &m_deviceThread ) ;
      m_deviceThread.start() ;
      return true ;
    }
    else
      return false ;
  }

  void WmDevice::runPoll()
  {
    if( m_hasWm )
    {
      m_isRunning = true ;

      // Update local data.
      while( m_isRunning )
      {
        updateDataFrom() ;
        updateDataTo() ;
      }

      // Let to restart the connection properly.
      deleteWii() ;
    }
  }

  void WmDevice::stopPoll()
  {
    // Stop the working thread.
    m_isRunning = false ;

    // Stop the event loop thread (run() method).
    m_deviceThread.quit() ;
  }

  bool WmDevice::updateDataFrom()
  {
    bool r=false ;

    if( m_hasWm )
    {
      bool isPoll=false ;
      bool updateButton, updateAcc, updateIR ;

      m_mutex.lock() ; // Protect for rumble feature.
      m_wii->Poll( updateButton, updateAcc, updateIR ) ;
      m_mutex.unlock() ;

      //Poll the wiimotes to get the status like pitch or roll
      if( updateButton )
      {
        switch( m_wm->GetEvent())
        {
        case CWiimote::EVENT_EVENT :
          isPoll = true ;
          break ;

        case CWiimote::EVENT_DISCONNECT :
        case CWiimote::EVENT_UNEXPECTED_DISCONNECT :
          mytoolbox::dbgMsg( "--- DISCONNECTED ---" ) ;
          stopPoll() ;
          break;

        case CWiimote::EVENT_NUNCHUK_INSERTED:
          connectNc() ;
          break ;

        case CWiimote::EVENT_NUNCHUK_REMOVED:
          m_nc = NULL ;
          break ;

        case CWiimote::EVENT_NONE :
        case CWiimote::EVENT_STATUS :
        case CWiimote::EVENT_CONNECT :
        case CWiimote::EVENT_READ_DATA :
        case CWiimote::EVENT_GUITAR_HERO_3_CTRL_REMOVED :
        case CWiimote::EVENT_GUITAR_HERO_3_CTRL_INSERTED :
        case CWiimote::EVENT_CLASSIC_CTRL_REMOVED :
        case CWiimote::EVENT_CLASSIC_CTRL_INSERTED :
        default:
          break ;
        }
      }

      if( updateButton || updateAcc || updateIR )
      {
        m_cirBufferFrom->pushBack( new WmDeviceData_from(m_wm->copyData()) ) ;
        r = true ;
      }
    }

    return r ;
  }

  bool WmDevice::updateDataTo()
  {
    bool r=false ;

    if( !m_cirBufferTo->isEmpty() )
    {
      WmDeviceData_to *data=NULL ;
      m_cirBufferTo->popFront( data ) ;

      if( data != NULL )
      {
        RumbleSettings rumble ;
        if( data->getRumble(rumble) )
          m_rumble->setSettings(rumble) ;

        delete data ;
        r = true ;
      }
    }

    return r ;
  }

}
