
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
    : m_operatingMode(0), m_menuMode(false), 
      m_irSensitive(0), m_hasSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF),
      m_updateOpMode(false), m_updateMenuMode(false), 
      m_updateIRSensitive(false), m_updateSleepThread(false)
  {
  }

  ChemicalWrapData_to::~ChemicalWrapData_to()
  {
  }

  ChemicalWrap::ChemicalWrap( InputDevice::Device *dev ) 
    : m_dev(dev), m_isRunning(false), m_hasSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF)
  {
    m_cirBufferFrom = new WIWO_sem<ChemicalWrapData_from*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_cirBufferTo = new WIWO_sem<ChemicalWrapData_to*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_wmToChem = new WmToChem(WMAVO_OPERATINGMODE3) ;
    m_actionsAreApplied.fetchAndStoreRelaxed(0) ;
    m_threadFinished.fetchAndStoreRelaxed(1) ;

    bool isConnect=QObject::connect( this, SIGNAL(runRunPoll()), this, SLOT(runPoll()) ) ;
    if( !isConnect )
      mytoolbox::dbgMsg( "Problem connection signal : ChemicalWrap.runRunPoll() -> ChemicalWrap.runPoll() !!" ) ;
  }

  ChemicalWrap::~ChemicalWrap()
  {
    stopPoll() ;

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
    if( m_isRunning && m_cirBufferFrom!=NULL )
    {
      ChemicalWrapData_from *data=NULL ;
      m_cirBufferFrom->popFront( data ) ;
      return data ;
    }
    else
      return NULL ;
  }

  void ChemicalWrap::setWrapperDataTo( const ChemicalWrapData_to& dataTo )
  { 
    if( m_isRunning && m_cirBufferTo!=NULL )
    {
      ChemicalWrapData_to *data=new ChemicalWrapData_to() ;
      *data = dataTo ;
      m_cirBufferTo->pushBack(data) ;
    }
  }

  bool ChemicalWrap::connectAndStart()
  {
    bool isConnect=this->connect( &m_wrapperThread, SIGNAL(started()), SLOT(runPoll()) ) ;
    if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_wrapperThread.started() -> ChemicalWrap.runPoll() !!" ) ;

    this->moveToThread( &m_wrapperThread ) ;

    m_threadFinished.fetchAndStoreRelaxed(0) ;
    m_wrapperThread.start() ;

    return true ;
  }

  void ChemicalWrap::setOnActionsApplied()
  {
    m_actionsAreApplied.fetchAndStoreRelaxed(1) ;
  }

  void ChemicalWrap::runPoll()
  {
    if( m_dev != NULL )
    {
      bool needUpdateDataFrom=false ;
      bool needUpdateDataTo=false ;
      m_isRunning = true ;

      while( m_isRunning )
      {
        needUpdateDataFrom = false ;
        needUpdateDataTo = false ;

        // Check if there are some works.
        if( m_actionsAreApplied==1 
            && m_dev->hasDeviceDataAvailable() )
          needUpdateDataFrom = true ;

        if( !m_cirBufferTo->isEmpty() )
          needUpdateDataTo = true ;

        if( needUpdateDataFrom || needUpdateDataTo )
        { // Working ...

          if( needUpdateDataFrom && updateDataFrom() )
            m_actionsAreApplied.fetchAndStoreRelaxed(0) ;

          if( needUpdateDataTo )
            updateDataTo() ;
        }
        else
        { // Sleeping ...

          if( m_hasSleepThread )
            m_wrapperThread.msleep(PLUGIN_WM_SLEEPTHREAD_TIME) ;
          else
            m_wrapperThread.yieldCurrentThread() ;
            // With m_deviceThread.setPriority( QThread::LowPriority ) ;
        }
      
        // Call event loop (to get "incoming signals").
        QCoreApplication::processEvents() ; 
        // By default : QEventLoop::AllEvent "& !QEventLoop::WaitForMoreEvents"
        // http://doc.qt.nokia.com/latest/qeventloop.html#ProcessEventsFlag-enum
      }

      // Must be disconnect, else there is a crash when this object is deleted.
      // Once QCoreApplication::processEvents() called, this object is "attached" at QCoreApplication.
      // So with this static method, this object must force to disconnect every signal after use,
      // mainly with QCoreApplication object.
      this->disconnect() ;

      m_threadFinished.fetchAndStoreRelaxed(1) ;
    }
    else
    {
      m_threadFinished.fetchAndStoreRelaxed(1) ;
      m_isRunning = false ;
    }
  }

  void ChemicalWrap::stopPoll()
  {
    if( m_isRunning )
    {
      // Stop the working thread.
      m_isRunning = false ;
      while( m_threadFinished == 0 ) ;

      ChemicalWrapData_from *dataFrom=NULL ;
      ChemicalWrapData_to *dataTo=NULL ;

      while( !m_cirBufferFrom->isEmpty() )
      {
        m_cirBufferFrom->popFront(dataFrom) ;
        if( dataFrom != NULL )
        {
          delete dataFrom ;
          dataFrom = NULL ;
        }
      }

      while( !m_cirBufferTo->isEmpty() )
      {
        m_cirBufferTo->popFront(dataTo) ;
        if( dataTo != NULL )
        {
          delete dataTo ;
          dataTo = NULL ;
        }
      }
    }

    // Stop the event loop thread (run() method).
    m_wrapperThread.quit() ;
  }

  bool ChemicalWrap::updateDataFrom()
  {
    bool r=false ;
    CWiimoteData *wmData=NULL ;

    if( m_dev != NULL )
    {
      InputDevice::WmDevice* dev=dynamic_cast<InputDevice::WmDevice*>(m_dev) ;
      InputDevice::WmDeviceData_from *wmDataFrom=NULL ;

      if( dev != NULL )
      {
        wmDataFrom = dev->getDeviceDataFrom() ;

        if( wmDataFrom != NULL )
          wmData = wmDataFrom->getDeviceData() ;

        if( wmData==NULL 
            || wmData->GetBatteryLevel()<0 
            || wmData->IR.GetNumDots()<0 || wmData->IR.GetNumDots()>4 )
          printf("Error data : in ChemicalWrapper.CWiimoteData *wm!=NULL && no allocated ...\n") ;
      }

      if( wmData!=NULL && wmData->isConnected() 
          && m_wmToChem->convert(wmData) )
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

        // 1st stategy : If no place, no push data.
        if( !m_cirBufferFrom->isFull() )
        {
          m_cirBufferFrom->pushBack( chemData ) ;
          emit newActions() ;
          r = true ;
        }
        else
          delete chemData ;
      }
      else
        r = false ;

      if( wmData != NULL )
        delete wmData ;
    }

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
      //if( !m_cirBufferTo->isEmpty() )
      {
        m_cirBufferTo->popFront(data) ;

        if( data != NULL )
        {
          int opMode ;
          bool menuMode, hasSleepThread ;
          int irSensitive ;
          bool hasUpdated=false ;

          hasUpdated = data->getOperatingMode(opMode) ;
          if( hasUpdated )
            m_wmToChem->setOperatingMode(opMode) ;

          hasUpdated = data->getMenuMode(menuMode) ;
          if( hasUpdated )
            m_wmToChem->setMenuMode(menuMode) ;

          hasUpdated = data->getIRSensitive(irSensitive) ;
          if( hasUpdated )
            m_wmToChem->setIrSensitive(irSensitive) ;

          hasUpdated = data->getHasSleepThread(hasSleepThread) ;
          if( hasUpdated )
            m_hasSleepThread = hasSleepThread ;

          delete data ;
          
          r = true ;
        }
      }
    }

    return r ;
  }

}
