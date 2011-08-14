
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
#ifndef __CHEMICALWRAPPER_H__
#define __CHEMICALWRAPPER_H__

#include "wrapper.h"
#include "inputdevice.h"
#include "wiwo_sem.h"
#include "qthread_ex.h"

#include "wiiusecpp.h"
#include "wiiusecppdata.h"
#include "wmtochem.h"

namespace WrapperInputToDomain
{
  class ChemicalWrapData_from : public WrapperData_from
  {
  public :
    ChemicalWrapData_from() ;
    ~ChemicalWrapData_from() ;

    inline unsigned int getWrapperType(){ return WRAPPER_ID_CHEMICAL ; } ;
  };

 
  class ChemicalWrapData_to
  {
  public :
    ChemicalWrapData_to() ;
    virtual ~ChemicalWrapData_to() ;

    inline unsigned int getWrapperType(){ return WRAPPER_ID_CHEMICAL ; } ;
    inline bool getOperatingMode( int &opMode_out )
    { bool up=m_updateOpMode ; m_updateOpMode=false ;
      opMode_out = m_operatingMode ; return up ; } ;
    inline bool getMenuMode( bool &menuMode_out )
    { bool up=m_updateMenuMode ; m_updateMenuMode=false ;
      menuMode_out = m_menuMode ; return up ; } ;
    inline bool getIRSensitive( int &irSensitive_out )
    { bool up=m_updateIRSensitive ; m_updateIRSensitive=false ;
      irSensitive_out = m_irSensitive ; return up ; } ;

    inline void setOperatingMode( int opMode )
    { m_operatingMode = opMode ; m_updateOpMode = true ; } ;
    inline void setMenuMode( bool mode )
    { m_menuMode = mode ; m_updateMenuMode = true ; } ;
    inline void setIRSensitive( int sensitive )
    { m_irSensitive = sensitive ; m_updateIRSensitive = true ; } ;

    inline void resetUpdate()
    { m_updateOpMode=false; m_updateMenuMode=false; m_updateIRSensitive=false; }

  private :
    int m_operatingMode ;
    bool m_updateOpMode ;

    bool m_menuMode ;
    bool m_updateMenuMode ;

    int m_irSensitive ;
    bool m_updateIRSensitive ;
  };


  // Chemical wrapper data.
  class ChemicalWrap : public Wrapper
  {
    Q_OBJECT // Signal need.

  public :
    ChemicalWrap( InputDevice::Device *dev ) ;
    virtual ~ChemicalWrap() ;

    inline unsigned int getWrapperType(){ return WRAPPER_ID_CHEMICAL ; } ;
    ChemicalWrapData_from* getWrapperDataFrom() ; //< Blocking call.
    void setWrapperDataTo( const ChemicalWrapData_to& data ) ; //< Blocking call.

    bool connectAndStart() ;
    void stopPoll() ;

  private :
    bool updateDataFrom() ;
    bool updateDataTo() ;

  public slots :
    void runPoll() ;

  signals :
    void newActions() ;

  protected :
    /**
      * @name Manage Chemical data.
      * @{ */
    InputDevice::Device *m_dev ;
    WmToChem *m_wmToChem ;
    WIWO_sem<ChemicalWrapData_from*> *m_cirBufferFrom ;
    WIWO_sem<ChemicalWrapData_to*> *m_cirBufferTo ;
    // @}

    /** @name Manage thread.
      * @{ */
    QThread_ex m_wrapperThread ;
    bool m_isRunning ;
    // @}
  };

}

#endif
