
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

#include "inputdevice.h"


namespace InputDevice
{
  class WmDeviceData_from : public DeviceData_from
  {
  public :
    WmDeviceData_from() ;
    ~WmDeviceData_from() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;
    inline bool isConnected(){ return m_isConnected ; } ;
    inline int getNbDotsDetected(){ return m_nbDotsDetected ; } ;
    inline int getNbSourceDetected(){ return m_nbSourceDetected ; } ;

  private :
    bool m_isConnected ;
    int m_nbDotsDetected ;
    int m_nbSourceDetected ;
  } ;


  class WmDeviceData_to : public DeviceData_to
  {
  public :
    WmDeviceData_to() ;
    ~WmDeviceData_to() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;
    inline void setOperatingMode( int opMode ){ m_operatingMode = opMode ; } ;
    inline void setMenuMode( bool menuMode ){ m_menuMode = menuMode ; } ;
    inline void setRumble( bool rumble ){ m_setRumble = rumble ; } ;
    inline void setLED( int LED ){ m_setLED = LED ; } ;
    inline void setSensitiveIR( int sensitive ){ m_sensitiveIR = sensitive ; } ;

    inline void initAttributsToZero()
    {
      m_operatingMode = 0 ; m_menuMode = false ;
      m_setRumble = false ; m_setLED = 0 ; m_sensitiveIR = 0 ;
    } ;

  private :
    int m_operatingMode ; ///< A mode defines "what buttons for what actions".
    bool m_menuMode ; 
      ///< Swith between the menu mode (choose option & co with the Wiimote), and the job action.

    bool m_setRumble ;
    int m_setLED ;

    int m_sensitiveIR ;
  } ;

    
  class WmDevice : Device
  {
  public :
    WmDevice() ;
    ~WmDevice() ;

    inline unsigned int getDeviceType(){ return INPUTDEVICE_ID_WIIMOTE ; } ;
    inline WmDeviceData_from* getDeviceDataFrom(){ return m_data_from ; } ;
    inline WmDeviceData_to* getDeviceDataTo(){ return m_data_to ; } ;

  private :
    WmDeviceData_from* m_data_from ;
    WmDeviceData_to* m_data_to ;
  };

}

#endif
