
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


#include"wmavo_rumble.h"

const int WmRumble::m_gapBetweenMinMaxTremor=WMRUMBLE_MAX_DURATION_TREMOR-WMRUMBLE_MIN_DURATION_TREMOR ;
const int WmRumble::m_gapBetweenMinMaxPose=WMRUMBLE_MIN_DURATION_POSE-WMRUMBLE_MAX_DURATION_POSE ;
const int WmRumble::m_gapBetweenStepTremor=m_gapBetweenMinMaxTremor/100 ;
const int WmRumble::m_gapBetweenStepPose=m_gapBetweenMinMaxPose/100 ;


WmRumble::WmRumble( WmAvo *parent,
          unsigned long durationTremor, unsigned long betweenTremor,
          unsigned int nbTremorBySequence )

    : m_parent(parent)/*, m_rumbleEnabled(false), m_quit(true),
    m_durationTremor(durationTremor),
    m_betweenTremor(betweenTremor), m_nbTremorBySequence(nbTremorBySequence),
    m_betweenSequence(0), m_nbSequence(1),
    m_continu(false), m_loop(true), m_gradual(0)*/
{

  cout << "QAtomicInt::isFetchAndStoreNative():" << QAtomicInt::isFetchAndStoreNative() << endl ;
  cout << "QAtomicInt::isFetchAndStoreWaitFree():" << QAtomicInt::isFetchAndStoreWaitFree() << endl ;

  m_rumbleEnabled.fetchAndStoreRelaxed(0) ; // false
  m_quit.fetchAndStoreRelaxed(1) ; // true
  m_durationTremor.fetchAndStoreRelaxed((int)durationTremor) ;
  m_betweenTremor.fetchAndStoreRelaxed((int)betweenTremor) ;
  m_nbTremorBySequence.fetchAndStoreRelaxed((int)nbTremorBySequence) ;
  m_betweenSequence.fetchAndStoreRelaxed(0) ;
  m_nbSequence.fetchAndStoreRelaxed(1) ;
  m_continu.fetchAndStoreRelaxed(0) ; // false
  m_loop.fetchAndStoreRelaxed(1) ; // true
  m_gradual.fetchAndStoreRelaxed(0) ;
}

WmRumble::~WmRumble(){}

bool WmRumble::getRumbleEnabled()
{
  int a=m_rumbleEnabled ;
  return ( a==0 ? false : true ) ;
  //return m_rumbleEnabled ;
}

unsigned long WmRumble::getDurationTremor()
{
  return (unsigned long)m_durationTremor ;
  //return m_durationTremor ;
}

unsigned long WmRumble::getBetweenSequence()
{
  return (unsigned long)m_betweenSequence ;
  //return m_betweenSequence ;
}

unsigned int WmRumble::getNbTremorBySequence()
{
  return (unsigned int)m_nbTremorBySequence ;
  //return m_nbTremorBySequence ;
}

unsigned long WmRumble::getBetweenTremor()
{
  return (unsigned long)m_betweenTremor ;
  //return m_betweenTremor ;
}

unsigned int WmRumble::getNbSequence()
{
  return (unsigned long)m_nbSequence ;
  //return m_nbSequence ;
}

bool WmRumble::getContinu()
{
  int a=m_continu ;
  return ( a==0 ? false : true ) ;
  //return m_continu ;
}

bool WmRumble::getLoop()
{
  int a=m_loop ;
  return ( a==0 ? false : true ) ;
  //return m_loop ;
}

int WmRumble::getGradual()
{
  return m_gradual ;
  //return m_gradual ;
}

void WmRumble::setRumbleEnabled( bool activeRumble )
{
  if( activeRumble )
    m_rumbleEnabled.fetchAndStoreRelaxed(1) ;
  else
    m_rumbleEnabled.fetchAndStoreRelaxed(0) ;
}
//{ m_rumbleEnabled = activeRumble ; }

void WmRumble::setDurationTremor( unsigned long durationTremor )
{
  if( durationTremor>=WMRUMBLE_MIN_TIME && durationTremor<=WMRUMBLE_MAX_TIME )
    m_durationTremor.fetchAndStoreRelaxed((int)durationTremor) ;
}
/*
{
  if( durationTremor>=WMRUMBLE_MIN_TIME && durationTremor<=WMRUMBLE_MAX_TIME )
  {
    // Pour éviter les problèmes de concurrence avec wmPoll()
    this->m_mutexSet.lock() ;
    m_durationTremor = durationTremor ;
    this->m_mutexSet.unlock() ;
  }
}
*/


void WmRumble::setBetweenSequence( unsigned long betweenSequence )
{
  if( /*betweenSequence>=0 &&*/ betweenSequence<=WMRUMBLE_MAX_TIME )
    m_betweenSequence.fetchAndStoreRelaxed((int)betweenSequence) ;
}
/*
{
  if( betweenSequence>=0 && betweenSequence<=WMRUMBLE_MAX_TIME )
  {
    // Pour éviter les problèmes de concurrence avec wmPoll()
    this->m_mutexSet.lock() ;
    m_betweenSequence = betweenSequence ;
    this->m_mutexSet.unlock() ;
  }
}
*/

void WmRumble::setNbTremorBySequence( unsigned int nbTremorBySequence )
{
  if( /*m_nbTremorBySequence>=0 &&*/ m_nbTremorBySequence<=WMRUMBLE_MAX_TIME ) // ~
    m_nbTremorBySequence.fetchAndStoreRelaxed((int)nbTremorBySequence) ;
}
/*
{
  if( m_nbTremorBySequence>=0 && m_nbTremorBySequence<=WMRUMBLE_MAX_TIME ) // ~
  {
    this->m_mutexSet.lock() ;
    m_nbTremorBySequence = nbTremorBySequence ;
    this->m_mutexSet.unlock() ;
  }
}
*/

void WmRumble::setBetweenTremor( unsigned long betweenTremor )
{
  if( /*betweenTremor>=0 &&*/ betweenTremor<=WMRUMBLE_MAX_TIME )
    m_betweenTremor.fetchAndStoreRelaxed((int)betweenTremor) ;
}
/*
{
  if( betweenTremor>=0 && betweenTremor<=WMRUMBLE_MAX_TIME )
  {
    this->m_mutexSet.lock() ;
    m_betweenTremor = betweenTremor ;
    this->m_mutexSet.unlock() ;
  }
}
*/

void WmRumble::setNbSequence( unsigned int nbSequence )
{
  if( /*nbSequence>=0 &&*/ nbSequence<=WMRUMBLE_MAX_TIME ) // ~
    m_nbSequence.fetchAndStoreRelaxed((int)nbSequence) ;
}
/*
{
  if( nbSequence>=0 && nbSequence<=WMRUMBLE_MAX_TIME ) // ~
  {
    this->m_mutexSet.lock() ;
    m_nbSequence = nbSequence ;
    this->m_mutexSet.unlock() ;
  }
}
*/

void WmRumble::setContinu( bool continu )
{
  if( continu )
    m_continu.fetchAndStoreRelaxed(1) ;
  else
    m_continu.fetchAndStoreRelaxed(0) ;
}
//{ m_continu = continu ; }

void WmRumble::setLoop( bool loop )
{
  if( loop )
    m_loop.fetchAndStoreRelaxed(1) ;
  else
    m_loop.fetchAndStoreRelaxed(0) ;
}
//{ m_loop = loop ; }

void WmRumble::setGradual( int gradual )
{
  if( gradual < 0 )
    //m_gradual = -1 ;
    m_gradual.fetchAndStoreRelaxed(-1) ;

  if( gradual>0 && gradual<100 )
  {
    int g=m_gradual ;

    //if( m_gradual != gradual )
    if( g != gradual )
    {
      //if( (m_gradual<=0 || m_gradual>=100) && !isRunning() )
      if( (g<=0 || g>=100) && !isRunning() )
      {
        //cout << "wmrumble: setGradual(): start()" << endl ;
        this->start() ;
      }

      //m_gradual = gradual ;
      m_gradual.fetchAndStoreRelaxed(gradual) ;

      //this->m_mutexSet.lock() ;

      //m_continu = false ;
      m_continu.fetchAndStoreRelaxed(0) ;

      //m_nbTremorBySequence = 1 ;
      m_nbTremorBySequence.fetchAndStoreRelaxed(1) ;
      //m_betweenTremor = 0 ;
      m_betweenTremor.fetchAndStoreRelaxed(0) ;
      //m_nbSequence = 1 ;
      m_nbSequence.fetchAndStoreRelaxed(1) ;

      // Method 1
      //m_durationTremor = WMRUMBLE_MIN_DURATION_TREMOR + m_gapBetweenStepTremor * gradual ;
      //m_betweenTremor = WMRUMBLE_MAX_DURATION_POSE + m_gapBetweenStepPose * gradual ;

      // Method 2
      if( gradual>0 && gradual<=30 )
        //m_durationTremor = 80 ;
        m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 150 // 80

      if( gradual>30 && gradual<=60 )
        //m_durationTremor = 120 ;
        m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 200 // 120

      if( gradual>60 && gradual<100 )
        //m_durationTremor = 150 ;
        m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 250 // 150

      //m_betweenTremor = WMRUMBLE_MIN_TIME + (WMRUMBLE_MAX_DURATION_POSE + m_gapBetweenStepPose * gradual) ;
      m_betweenTremor.fetchAndStoreRelaxed( WMRUMBLE_MIN_TIME + (WMRUMBLE_MAX_DURATION_POSE + m_gapBetweenStepPose * int(float(gradual)/1.3))) ; // /1.3 : to reduce the rumble : TODO : code better that ...

      //this->m_mutexSet.unlock() ;
    }
  }
  else
  {
    //m_quit = true ; // Stop the thread.
    m_quit.fetchAndStoreRelaxed(1) ;

    //m_gradual = 0 ;
    m_gradual.fetchAndStoreRelaxed(0) ;
    //m_continu = false ;
    m_continu.fetchAndStoreRelaxed(0) ;
    //m_nbTremorBySequence = 1 ;
    m_nbTremorBySequence.fetchAndStoreRelaxed(1) ;
    //m_nbSequence = 1 ;
    m_nbSequence.fetchAndStoreRelaxed(1) ;
    //m_betweenTremor = 0 ;
    m_betweenTremor.fetchAndStoreRelaxed(0) ;

    //m_durationTremor = 0 ;
    m_durationTremor.fetchAndStoreRelaxed(0) ;
    //m_betweenTremor = 0 ;
    m_betweenTremor.fetchAndStoreRelaxed(0) ;
  }
}

void WmRumble::run()
{
  //if( m_quit == false ) Do nothing ... The thread is running.

  unsigned int i=0, j=0 ;
  unsigned long k=0, l=0 ;

  //cout << "wmrumble: run(): debut, m_quit=false" << endl ;
  //m_quit = false ;
  m_quit.fetchAndStoreRelaxed(0) ;


  // rumbleEnabled<-True par wmConnect, wmDisconnect
  //if( m_parent!=NULL && m_rumbleEnabled && !m_quit)
  if( m_parent!=NULL && m_rumbleEnabled && !m_quit)
  {
    if( m_continu )
    {
      // Pour éviter les problèmes de concurrence avec wmPoll()
      /*
      while( !m_parent->m_mutex->tryLock() )
      {
        cout << "mutex : WmRumble.run(), runble(true, continu)" << endl ;
        this->usleep(TIME_TRYLOCK) ;
      }
      */

      m_parent->m_mutex->lock() ;
      m_parent->m_wm->SetRumbleMode( CWiimote::ON ) ;
      m_parent->m_mutex->unlock() ;
      //cout << "wmrumble: run(): continu, rumble(ON)" << endl ;
    }
    else
    {
      do
      {
        do
        {
          do
          {
            // Activate Rumble.

            /*
            while( !m_parent->m_mutex->tryLock() )
            {
              cout << "mutex : WmRumble.run(), runble(true, graduel)" << endl ;
              this->usleep(TIME_TRYLOCK) ;
            }
            */
            m_parent->m_mutex->lock() ;
            m_parent->m_wm->SetRumbleMode( CWiimote::ON ) ;
            m_parent->m_mutex->unlock() ;
            //cout << "wmrumble: run(): graduel, rumble(ON)" << endl ;

            // Operating time of rumble.

            k = 0 ;
            /*
            this->m_mutexSet.lock() ;
            l = m_durationTremor ;
            this->m_mutexSet.unlock() ;
            */
            l = m_durationTremor ;

            while( m_rumbleEnabled && !m_quit && k<l )
            {
              k += WMRUMBLE_TIME_SLEEP ;
              this->msleep(WMRUMBLE_TIME_SLEEP) ;

              /*
              this->m_mutexSet.lock() ;
              l = m_durationTremor ;
              this->m_mutexSet.unlock() ;
              */
              l = m_durationTremor ;
            }


            // Desactivate rumble.

            /*
            while( !m_parent->m_mutex->tryLock() )
            {
              cout << "mutex : WmRumble.run(), runble(false, graduel)" << endl ;
              this->usleep(TIME_TRYLOCK) ;
            }
            */
            m_parent->m_mutex->lock() ;
            m_parent->m_wm->SetRumbleMode( CWiimote::OFF ) ;
            m_parent->m_mutex->unlock() ;
            //cout << "wmrumble: run(): graduel, rumble(OFF)" << endl ;


            // Sleep time of rumble between tremors.

            k = 0 ;
            /*
            this->m_mutexSet.lock() ;
            l = m_betweenTremor ;
            this->m_mutexSet.unlock() ;
            */
            l = m_betweenTremor ;

            while( m_rumbleEnabled && !m_quit && m_loop && k<l )
            {
              k += WMRUMBLE_TIME_SLEEP ;
              this->msleep(WMRUMBLE_TIME_SLEEP) ;

              /*
              this->m_mutexSet.lock() ;
              l = m_betweenTremor ;
              this->m_mutexSet.unlock() ;
              */
              l = m_betweenTremor ;
            }


            /*
            this->m_mutexSet.lock() ;
            l = m_nbTremorBySequence ;
            this->m_mutexSet.unlock() ;
            */
            l = m_nbTremorBySequence ;

          }while( m_rumbleEnabled && !m_quit && m_loop && (++j)<(unsigned int)l ) ;


          // Sleep time of rumble between sequences.

          j = 0 ;
          k = 0 ;
          /*
          this->m_mutexSet.lock() ;
          l = m_betweenSequence ;
          this->m_mutexSet.unlock() ;
          */
          l = m_betweenSequence ;

          while( m_rumbleEnabled && !m_quit && m_loop && k<l )
          {
            k += WMRUMBLE_TIME_SLEEP ;
            this->msleep(WMRUMBLE_TIME_SLEEP) ;

            /*
            this->m_mutexSet.lock() ;
            l = m_betweenSequence ;
            this->m_mutexSet.unlock() ;
            */
            l = m_betweenSequence ;
          }

          //cout << "m_durationTremor:" << m_durationTremor << " m_betweenTremor:" << m_betweenTremor << " m_nbTremorBySequence:" << m_nbTremorBySequence << endl ;
          //cout << "m_betweenSequence:" << m_betweenSequence << " m_nbSequence:" << m_nbSequence << " m_loop:" << m_loop << endl ;


          /*
          this->m_mutexSet.lock() ;
          l = m_nbSequence ;
          this->m_mutexSet.unlock() ;
          */
          l = m_nbSequence ;

        }while( m_rumbleEnabled && !m_quit && m_loop && (++i)<(unsigned int)l ) ;

        i = 0 ;

      }while( m_rumbleEnabled && !m_quit && m_loop ) ;

      m_quit.fetchAndStoreRelaxed(1) ;
    }
  }
}

void WmRumble::quit()
{
  if( m_continu && m_quit==0 )
  {
    /*
    while( !m_parent->m_mutex->tryLock() )
    {
      cout << "mutex : WmRumble.run(), runble(false, continu)" << endl ;
      this->usleep(TIME_TRYLOCK) ;
    }
    */
    m_parent->m_mutex->lock() ;
    m_parent->m_wm->SetRumbleMode( CWiimote::OFF ) ;
    m_parent->m_mutex->unlock() ;
    //cout << "wmrumble: quit(): continu, rumble(OFF)" << endl ;
  }

  //m_quit = true ;
  m_quit.fetchAndStoreRelaxed(1) ; // true
  //cout << "wmrumble: quit():m_quit=true" << endl ;

  wait() ;
  QThread::quit() ;
}

bool WmRumble::isQuit()
{
  return m_quit==1 ;
}
