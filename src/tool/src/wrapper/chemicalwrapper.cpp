
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

#include "chemicalwrapper.h"

namespace WrapperInputToDomain
{

  ChemicalWrapData_from::ChemicalWrapData_from()
  {
  }

  ChemicalWrapData_from::~ChemicalWrapData_from()
  {
  }

  ChemicalWrapData_to::ChemicalWrapData_to()
    : m_operatingMode(0), m_menuMode(false), m_irSensitive(0),
      m_updateOpMode(false), m_updateMenuMode(false), m_updateIRSensitive(false)
  {
  }

  ChemicalWrapData_to::~ChemicalWrapData_to()
  {
  }

  ChemicalWrap::ChemicalWrap( InputDevice::Device *dev ) 
    : m_dev(dev), m_isRunning(false)
  {
    m_cirBufferFrom = new WIWO_sem<ChemicalWrapData_from*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_cirBufferTo = new WIWO_sem<ChemicalWrapData_to*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_wmToChem = new WmToChem(WMAVO_OPERATINGMODE3) ;
  }

  ChemicalWrap::~ChemicalWrap()
  {
    if( m_cirBufferFrom != NULL )
    {
      delete( m_cirBufferFrom ) ;
      m_cirBufferFrom = NULL ;
    }

    if( m_cirBufferTo != NULL )
    {
      delete( m_cirBufferTo ) ;
      m_cirBufferTo = NULL ;
    }

    if( m_wmToChem != NULL )
    {
      delete( m_wmToChem ) ;
      m_wmToChem = NULL ;
    }
  }

  ChemicalWrapData_from* ChemicalWrap::getWrapperDataFrom()
  { 
    ChemicalWrapData_from *data=NULL ;
    m_cirBufferFrom->popFront( data ) ;
    return data ;
  }

  void ChemicalWrap::setWrapperDataTo( const ChemicalWrapData_to& dataTo )
  { 
    ChemicalWrapData_to *data=new ChemicalWrapData_to() ;
    *data = dataTo ;
    m_cirBufferTo->pushBack(data) ;
  }

  bool ChemicalWrap::connectAndStart()
  {
    this->connect( &m_wrapperThread, SIGNAL(started()), SLOT(runPoll()) ) ;
    this->moveToThread( &m_wrapperThread ) ;
    m_wrapperThread.start() ;
    return true ;
  }


  void ChemicalWrap::runPoll()
  {
    if( m_dev != NULL )
    {
      //m_isRunning = true ;

      // Update local data.
      //while( m_isRunning )
      {
        updateDataFrom() ;
        updateDataTo() ;
      }
    }
  }

  void ChemicalWrap::stopPoll()
  {
    // Stop the working thread.
    m_isRunning = false ;

    // Stop the event loop thread (run() method).
    m_wrapperThread.quit() ;
  }

  bool ChemicalWrap::updateDataFrom()
  {
    bool r=false ;
    CWiimoteData *wmData=NULL ;

    if( m_dev )
    {
      InputDevice::WmDevice* dev=dynamic_cast<InputDevice::WmDevice*>(m_dev) ;
      InputDevice::WmDeviceData_from *wmDataFrom=NULL ;

      if( dev != NULL )
      {
        wmDataFrom = dev->getDeviceDataFrom() ;

        if( wmDataFrom != NULL )
          wmData = wmDataFrom->getDeviceData() ;
      }

      if( wmData!=NULL && wmData->isConnected() && wmData->isPolled() && m_wmToChem->convert(wmData) )
      {
        // Initiate data.
        ChemicalWrapData_from *chemData=new ChemicalWrapData_from() ;
        WrapperData_from::wrapperActions_t waData ;
        WrapperData_from::positionCamera_t camData ;
        WrapperData_from::positionPointed_t posData ;

        waData.actionsGlobal = 0 ;
        waData.actionsWrapper = m_wmToChem->getActions() ;
        waData.wrapperType = WRAPPER_ID_CHEMICAL ;
        chemData->setWrapperAction( waData ) ;

        camData.angleRotateDegree[0] = m_wmToChem->getAngleCamRotateXDeg() ;
        camData.angleRotateDegree[1] = m_wmToChem->getAngleCamRotateYDeg() ;
        camData.angleRotateDegree[2] = 0 ;
        camData.distanceTranslate[0] = m_wmToChem->getDistCamTranslateX() ;
        camData.distanceTranslate[1] = m_wmToChem->getDistCamTranslateY() ;
        camData.distanceTranslate[2] = m_wmToChem->getDistCamZoom() ;
        chemData->setPositionCamera( camData ) ;

        posData.posCursor = m_wmToChem->getPosCursor() ;
        posData.pos3dCur = m_wmToChem->getPos3dCurrent() ;
        posData.pos3dLast = m_wmToChem->getPos3dLast() ;
        chemData->setPositionPointed( posData ) ;

        /*
        chemData->nbDotsDetected = m_wmD->IR.GetNumDots() ;
        chemData->nbSourcesDetected = m_wmavo->getWiimote()->IR.GetNumSources() ;
        chemData->distBetweenSources = (int)m_wmavo->getWiimote()->IR.GetDistance() ;
        */

        m_cirBufferFrom->pushBack( chemData ) ;
        emit newActions() ;

        r = true ;
      }
      else
        r = false ;
    }

    if( wmData != NULL )
      delete wmData ;

    return r ;
  }

  bool ChemicalWrap::updateDataTo()
  {
    bool r=false ; 

    // Let a minimum before a set
    //this->msleep( WMAVOTH_SLEEPBEFORE_NEXTROUND ) ;
    
    //if( isConnected = wmavo->wmIsConnected() )
    {
      ChemicalWrapData_to *data=NULL ; 
      if( !m_cirBufferTo->isEmpty() )
      {
        m_cirBufferTo->popFront(data) ;

        if( data != NULL )
        {
          int opMode ;
          bool menuMode ;
          int irSensitive ;

          if( data->getOperatingMode(opMode) )
            m_wmToChem->setOperatingMode(opMode) ;

          if( data->getMenuMode(menuMode) )
            m_wmToChem->setMenuMode(menuMode) ;

          if( data->getIRSensitive(irSensitive) )
            m_wmToChem->setIrSensitive(irSensitive) ;

          delete data ;
          
          r = true ;
        }
      }
    }

    return r ;
  }

}
