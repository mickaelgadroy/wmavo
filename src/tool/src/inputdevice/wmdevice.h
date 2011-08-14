
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

#pragma once
#ifndef __WMDEVICE_H__
#define __WMDEVICE_H__

#include "warning_disable_begin.h"
#include "inputdevice.h"
#include "wmavo_const.h"
#include "variousfeatures.h"
#include "wiwo_sem.h"
#include "qthread_ex.h"
#include "wiiusecpp.h"

#if defined WIN32 || defined _WIN32
  #include "wiiusecpp.h"
#else
  #include <wiiusecpp.h>
#endif

#include "warning_disable_end.h"


namespace InputDevice
{
  class WmRumble ;
  class RumbleSettings ;

  class WmDeviceData_from : public DeviceData_from
  {
  public :
    WmDeviceData_from() ;
    WmDeviceData_from( CWiimoteData *wmData ) ; //> Just copy the pointer.
    WmDeviceData_from( const WmDeviceData_from &wmData ) ; //> Copy all object.
    ~WmDeviceData_from() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;
    inline CWiimoteData* getDeviceData(){ return m_data ; } ;

    bool hasPoll() ; // 

    void operator=( const WmDeviceData_from& wmDataFrom ) ; // To be sur to have 2 distinct object.

  private :
    CWiimoteData *m_data ;
  } ;


  class WmDeviceData_to : public DeviceData_to
  {
  public :
    WmDeviceData_to() ;
    WmDeviceData_to( RumbleSettings* rumble, int LED ) ;
    WmDeviceData_to( const WmDeviceData_to& data ) ;
    ~WmDeviceData_to() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;

    bool getRumble( RumbleSettings &rumble_out ) ;
    void setRumble( const RumbleSettings &rumble ) ;

    bool getLED( int &LED ) ;
    void setLED( int LED ) ;

    void operator=( const WmDeviceData_to& wmDataTo ) ;

    inline void resetUpdate()
    { m_updateRumble=false ; m_updateLED=false;} ;

    void initAttributsToZero() ;

  private :
    RumbleSettings *m_setRumble ;
    bool m_updateRumble ;

    int m_setLED ;
    bool m_updateLED ;
  } ;

    
  class WmDevice : public Device
  {
    Q_OBJECT // For the thread.

    friend WmRumble ;

  public :
    WmDevice() ;
    ~WmDevice() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;
    WmDeviceData_from* getDeviceDataFrom() ; //< Blocking call.
    void setDeviceDataTo( const WmDeviceData_to& wmDataTo ) ; //< Blocking call.

    bool connectAndStart() ;
    void stopPoll() ;

  private :
    int  connectWm() ;
    bool connectNc() ;
    bool updateDataFrom() ;
    bool updateDataTo() ;

    // Delete m_wii (use many times).
    void deleteWii() ;

  private slots :
    void runPoll() ;

  private :

    /**
      * @name Manage Wiimote data.
      * @{ */
    WIWO_sem<WmDeviceData_from*> *m_cirBufferFrom ;
    WIWO_sem<WmDeviceData_to*> *m_cirBufferTo ;
    // @}

    /**
      * @name Manage thread.
      * @{ */
    QThread_ex m_deviceThread ;
    QMutex m_mutex ; // Secure Poll() with rumble feature.
    bool m_isRunning ;
    // @}

    /**
      * @name Wiimote things.
      * @{ */
    CWii *m_wii ; ///< Manage the Wiimotes. (object)
    CWiimote *m_wm ; ///< Manage the first Wiimote. (shortcut)
    CNunchuk *m_nc ; // (shortcut)
    WmRumble *m_rumble ; // (object)
    bool m_hasWm, m_hasNc ; ///< Have Wiimote and/or Nunchuk ?
    // @}
  };

}

#endif
