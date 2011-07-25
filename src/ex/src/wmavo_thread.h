
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


#pragma once
#ifndef __WMAVO_THREAD_H__
#define __WMAVO_THREAD_H__

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "wmavo.h"

#include <iostream>
#include <vector>

#include <QMutex>
#include <QThread>
#include <qcursor.h>

#include <Eigen/Core>
#include <avogadro/atom.h>
#include "warning_disable_end.h"


/**
  * @class WmAvoThread
  * @brief It manages the thread part and the communication of the wrapper.
  *
  * It is managed the 2nd part of the wrapper. It manages the thread part and the
  * communication of the wrapper between the final application and the 1st part of
  * the wrapper (which manage the conversion input->"chemistry signal").
  */
class WmAvoThread : public QThread
{
  Q_OBJECT


  //
  // Publics attributs & definitions.
  public :

    /**
      * Structure used to transfert informations between the wrapper and the WmExtension objects.
      */
    struct wmDataTransfert
    {
      QPoint posCursor ;
      int wmActions ;
      Eigen::Vector3d pos3dCur, pos3dLast ;

      double angleCamRotateXDeg, angleCamRotateYDeg ;
      double distCamTranslateX, distCamTranslateY ;
      double distCamZoom ;

      int nbDotsDetected ;
      int nbSourcesDetected ;
      int distBetweenSources ;

    } ;

    wmDataTransfert m_wmDataTransfert ;


  //
  // Signals methods.
  signals :

    void wmPolled( WmAvoThread::wmDataTransfert wmData ) ;
        // This writting is to respect the same writting use in other class to connect this signal.

    void wmConnected( int nbConnected ) ;
    void wmDisconnected() ;


  public:
    WmAvoThread( QObject *parent ) ;
    ~WmAvoThread() ;

    void run() ; ///< Execute when this->start() is called.
    void quit() ; ///< Overload method.

    void setWmOperatingMode(int opMode) ;
    void setWmRumble( bool start, bool continu=true, bool loop=false, int gradual=-1 ) ;
    void setWmRumble( int gradual ) ;
    void setWmRumbleEnable( bool state ) ;
    void setWmSizeWidget( int x, int y, int width, int height ) ;
    void setWmMenuMode( bool menuMode ) ; // == !actionMode
    void setWmActionMode( bool actionMode ) ; // == !menuMode
    void setWmSensitive( int wmSensitive ) ;


  //
  // Protected methods.
  protected :
    void updateWmAttributs() ;
    void updateDataTransfert() ;


  //
  // Private attributs.
  private :
    WmAvo *m_wmavo ; // (shortcut)
    QMutex m_mutex ; // Run works into a process, and every other thing (setWmOp...) work into another process.

    bool m_updateOpMode ; // Need update ?
    int m_operatingMode ;

    bool m_updateRumble, m_updateRumbleGrad, m_updateRumbleEnable ; // Need update ?
    bool m_start, m_continu, m_loop, m_rumbleState ;
    int m_gradual ;

    bool m_addAtom, m_delAtom ; // "Need update ?" and data to use.
    std::vector<Eigen::Vector3d> m_posAtomStock ;

    bool m_updateSizeWidget ; // Need update ?
    int m_x, m_y, m_width, m_height ;

    bool m_updateMenuMode ; // Need update ?
    bool m_menuMode ;

    bool m_updateWmSensitive ;
    int m_wmSensitive ;
};

#endif
