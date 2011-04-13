
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


#ifndef __WMAVO_RUMBLE_H__
#define __WMAVO_RUMBLE_H__

#ifdef _WIN32
#pragma warning( disable : 4365 ) // conversion from 'x' to 'y', signed/unsigned mismatch
#pragma warning( disable : 4820 ) // 'x' bytes padding added after data member '...'
#pragma warning( disable : 4668 ) // '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable : 4514 ) // '...' : unreferenced inline function has been removed
#pragma warning( disable : 4738 ) // storing 32-bit float result in memory, possible loss of performance
#pragma warning( disable : 4710 ) // function not inlined
#pragma warning( disable : 4626 ) // '...' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning( disable : 4625 ) // '...' : copy constructor could not be generated because a base class copy constructor is inaccessible

#pragma warning( push, 0 )
#endif

#include <wmavo_const.h>
#include "wmavo.h"

#include <QThread>
#include <qmutex.h>
#include <QAtomicInt>

#include <iostream>

#ifdef _WIN32
#pragma warning( pop )
#endif

using namespace std ;

// N.B. :
// Generating src/moc_wmavo.cxx !
// Error: Meta object features not supported for nested classes
// So, the WmRumble class can not neste in WmAvo class, and it become friend with the WmAvo class.

// Pas de concurrence gérée pour les vars. booléennes (m_rumbleEnabled, m_quit, m_loop ...).

class WmAvo ; // To solve a problem of mutual calling of the class.


/**
  * @class WmRumble
  * @brief The class manages the rumble feature of the Wiimote.
  */
class WmRumble : public QThread
{
  Q_OBJECT

  public :

    static const int m_gapBetweenMinMaxTremor, m_gapBetweenMinMaxPose ;
    static const int m_gapBetweenStepTremor, m_gapBetweenStepPose ;

    WmRumble( WmAvo *parent,
              unsigned long durationTremor=1000, unsigned long betweenTremor=1,
              unsigned int nbTremorBySequence=0 ) ;
    ~WmRumble() ;

    bool getRumbleEnabled() ;

    /**
      * Be careful, all method uses int value. (unsigned long) and (unsigned int) are here for ease the use with timer method for example.
      */
    unsigned long getDurationTremor() ;
    unsigned long getBetweenSequence() ;
    unsigned int getNbTremorBySequence() ;
    unsigned long getBetweenTremor() ;
    unsigned int getNbSequence() ;
    bool getContinu() ;
    bool getLoop() ;
    int getGradual() ;

    void setRumbleEnabled( bool activeRumble ) ;
    void setDurationTremor( unsigned long durationTremor ) ;
    void setBetweenSequence( unsigned long betweenSequence ) ;
    void setNbTremorBySequence( unsigned int nbTremorBySequence ) ;
    void setBetweenTremor( unsigned long betweenTremor ) ;
    void setNbSequence( unsigned int nbSequence ) ;
    void setContinu( bool continu ) ;
    void setLoop( bool loop ) ;
    void setGradual( int gradual ) ;

    void run() ;
    void quit() ;

    bool isQuit() ;

  private :

    WmAvo *m_parent ; // (shortcut)
    QMutex m_mutexSet ;

    // Avoid concurent problem with wmPoll().
    QAtomicInt m_rumbleEnabled, m_quit ;
    QAtomicInt m_durationTremor, m_betweenTremor ; // ms
    QAtomicInt m_nbTremorBySequence ;
    QAtomicInt m_betweenSequence ; // ms
    QAtomicInt m_nbSequence ;
    QAtomicInt m_continu, m_loop ;
    QAtomicInt m_gradual ;

    /*
    bool m_rumbleEnabled, m_quit ;

    // A rumble is a sequence composed of tremors.

    // Can be configure :
    //    ... ... ...     : 3 sequences of 3 tremors
    // OR .   .   .   .   : 4 sequences of 1 tremor
    unsigned long m_durationTremor, m_betweenTremor ; // ms
    unsigned int m_nbTremorBySequence ;

    // Can be configure :
    // .. .. ..    .. .. ..    .. .. ..   : 3 sequences of 2 tremors
    // ....  ....          ....  ....     : 2 sequences of 4 tremors
    unsigned long m_betweenSequence ; // ms
    unsigned int m_nbSequence ;

    bool m_continu, m_loop ;
    int m_gradual ;
    */
};

#endif
