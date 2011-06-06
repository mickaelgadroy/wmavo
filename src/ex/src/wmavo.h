
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

#ifndef __WMAVO_H__
#define __WMAVO_H__

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

#define NOMINMAX
// To avoid a conflit with a macro definition in Eigen/Core.
// If you are compiling on Windows, to prevent windows.h from defining these symbols.


#include "wmavo_const.h"
#include "wmavo_rumble.h"
#include "wiwo.h"

#include <wiiusecpp.h>
#include <Eigen/Core>
#include <Eigen/Geometry> // eigen::AngleAxis


#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpoint.h>
#include <qmutex.h>
#include <QTimer>
#include <QThread>
#include <QTime>

#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#pragma warning( pop )
#endif

using Eigen::AngleAxisd ;
using namespace std ;
using namespace Eigen ;

class WmRumble ; // To solve a problem of mutual calling of the class.


/**
  * @class WmAvo
  * @brief The class which intanciate the Wiiuse library to use the Wiimote
  *
  * It is more complicated ... In fact, this class represents plus a wrapper that
  * just a Wiimote class. This wrapper is destined to translate the Wiimote action
  * to the "actions necessary to build a molecule".
  * <br/>This means that the Avogadro plugin "know" what it must do (action to create atom,
  * action to rotate the camera ...). After, more than 1 actions can be realised,
  * so Avogadro can interpret a combinaison of actions :
  * ex. : "move the mouse cursor" + "select one atom" =
  *       = "select all atoms include in a selection rectangle"
  * This is the 1st part of the wrapper. The 2nd (wmavo_thread) manage the thread part and communication with other
  * application.
  */
class WmAvo
{

  //
  // Static attributs.
  public:

    friend class WmRumble ;

    static const double m_PI ; ///< Pi approximation.
    static const double m_PI180 ; ///< 3.14/180 for degree to radian.
    static const double m_180PI ; ///< 180/3.14 for radian to degree.
    static const Vector3d m_refPoint0 ; ///< Point(0,0,0).
    static const int m_XScreen, m_YScreen ; ///< Resolution of the screen.


  //
  // Public methods.
  public :

    /**
      * @name Constructor/destructor methods
      * @{ */
    WmAvo( int operatingMode=0 ) ; ///< Constructor
    ~WmAvo() ; ///< Destructor
    // @}


    /**
      * @name "Transformations informations"
      * All informations to know the move of the Wiimote.
      * @{ */
    QPoint getPosCursor() ;
    Vector3d getPos3dCurrent() ;
    Vector3d getPos3dLast() ;
    double getDistCamZoom() ;
    double getAngleCamRotateXDeg() ;	double getAngleCamRotateYDeg() ;
    double getDistCamTranslateX() ;	double getDistCamTranslateY() ;
    // @}


    /**
      * @name Getter/setter methods.
      * Getter/setter methods for various need, but no the realized actions by the Wiimote.
      * @{ */
    CWii* getWii() ;
    CWiimote* getWiimote() ;

    int getWmOperatingMode() ;
    void setWmOperatingMode(int opMode) ;

    void wmSetRumble( bool start, bool continu=true, bool loop=false, int gradual=-1 ) ;
    void wmSetRumble( int gradual ) ;
    void wmSetRumbleEnable( bool state ) ;

    void wmSetSizeWidget( int x, int y, int width, int height ) ;

    void wmSetMenuMode( bool menuMode ) ;
    void wmSetActionMode( bool actionMode ) ;

    void wmSetSensitive( int wmSensitive ) ;
    // @}


    /**
      * @name Wiimote actions
      * All actions realized by the Wiimote, or calculate directly after the poll.
      * @{ */
    int wmConnect() ;
    void wmDisconnect() ;
    bool wmPoll() ;

    Vector3d wmGetIrCursor() ; ///< Update m_wmCurrentIrCursorPos and m_wmLastIrCursorPos.
    bool wmGetSmoothedCursor() ; ///< Update m_currentPosSmooth and m_lastPosSmooth.
    float wmGetAcc() ;
    // @}


    /**
      * @name Actions for the construction of the molecule
      * All Wiimote actions translate to "the construction of the molecule" paradigm.
      * @{ */
    int wmGetActions() ;
    bool wmIsConnected() ; bool wmNcIsConnected() ;
    bool wmIsMovedCursor() ; bool wmIsSelected() ;
    bool wmIsCreated() ; bool wmIsDeleted() ;
    bool wmIsMovedAtom() ; ///< Indicate if wmAtomIsRotate() or wmAtomIsTranslate() == true
    bool wmAtomIsRotate() ;	bool wmAtomIsTranslate() ;/* bool wmAtomIsZoom() ;*/
    bool wmCamIsRotate() ; bool wmCamIsTranslate() ; bool wmCamIsZoom() ;
    bool wmCamIsInitialize() ;
    bool wmMenuIsActive() ;
    // @}


    /**
      * @name Various operating mode
      * Various global methods which interpret the Wiimote signal to "the construction of the
      * molecule" paradigm.
      * @{ */
    void wmOperatingMode1() ;
    void wmOperatingMode2() ;
    void wmOperatingMode3() ;

  //
  // Protected methods.
  protected:

    /**
      * @name Sub-methods for the choice of the operating mode
      * Decomposition of wmOperatingMode*() methods according to the Wiimote or
      * the Nunchuk.
      * @{ */
    void wmOperatingModeWm2() ;
    void wmOperatingModeNc1() ;
    void wmOperatingModeNc2() ;
    // @}

    /**
      * @name Transform Wiimote actions to something.
      * Decomposition of wmOperatingModeWm*() methods.
      * @{ */
    bool transformWmAction1ToMouseMovement() ;
    bool transformWmAction1ToSelectOrTranslateAtom() ;
    bool transformWmAction1ToCreateAtom() ;
    bool transformWmAction1ToDeleteAtom() ;
    bool transformWmAction1ToInitCamera() ;
    bool transformWmAction1ToOKMenu() ;
    bool transformWmAction1ToRotateAtomOrActivateMenu() ;
    bool transformWmAction1ToMoveInMenu() ;
    // @}

    /**
      * @name Transform Nunchuk actions to something.
      * Decomposition of wmOperatingModeNc*() methods.
      * @{ */
    bool transformNcAction1ToZoomIn() ;
    bool transformNcAction1ToZoomOut() ;
    bool transformNcAction1ToTranslateCamera() ;
    bool transformNcAction1ToRotateCameraOrFinalizeTranslateCamera( float angle, float magnitude ) ;

    bool transformNcAction2ToZoom() ;
    bool transformNcAction2ToTranslateCamera() ;
    bool transformNcAction2ToRotateCamera() ;
    bool transformNcAction2ToNothingForCamera() ;
    bool transformNcAction2ToCalculCameraMovement( float angle, float magnitude ) ;
    // @}


    /**
      * @name Calculate some informations
      * This informations are the get data from the Wiimote which must be
      * calibrated/calculated/adjusted.
      */
    void calibrateCamRotateAngles( float angle, float magnitude ) ;
        ///< Update m_angleNcJoystickCosDeg, m_angleNcJoystickSinDeg
    void calibrateCamTranslateDist( float angle, float magnitude ) ;
        ///< Update m_distCamXTranslate, m_distCamYTranslate
    void calibrateCamZoomDist( float angle, float magnitude ) ;
        ///< Update m_distCamZoom

    void translateCursorToScreen() ;
        ///< Update m_posCursor
    bool actionZoom( bool active, bool pause=false ) ;
        ///< Update m_activeZoom & m_nbZoom
    // @}


  //
  // Private attributs
  private:


    /**
      * @name Mutex mecanism to ...
      * ... Avoid to access the wiiuse library during the "poll". This mecanism is used
      * mainly by the rumble feature. In fact, it is in a different thread and asks anytime
      * the begin and the end of the rumble of the Wiimote. This is one of the reason the WmRumble
      * class is a "friend class".
      * @{ */
    QMutex *m_mutex ; // (object)
    // @}

    /**
      * @name Wiimote objects
      * It is many things directly about the Wiimote :
      *  - the instantiation of the wiiuse library
      *  - a boolean value to know if some "Wiimote objects" are initiated
      *  - ...
      * @{ */
    CWii *m_wii ; ///< Manage the Wiimotes. (object)
    CWiimote *m_wm ; ///< Manage the first Wiimote. (shortcut)
    CNunchuk *m_wmNunchuk ; // (shortcut)
    WmRumble *m_wmRumble ; // (object)
    bool m_wmGetWiimote, m_wmGetNunchuk ; ///< Have Wiimote and/or Nunchuk ?
    bool m_wmGetAlreadyWiimote/*, m_wmGetAlreadyNunchuk */; ///< Help for update some data once.
    int m_wmOperatingMode ;
    bool m_wmOtherPoll ;
    int m_wmSensitive ;
    // @}

    /**
      * @name Wiimote/Avogadro states
      * @{ */
    int m_isWhat ;
    // @}

    /**
      * @name Differents positions of the IR cursor points.
      * @{ */
    QPoint m_posCursor ;
    Vector3d m_wmCurrentIrCursorPos, m_wmLastIrCursorPos ;
    // @}

    /**
      * @name Manage the accelerometers
      * @{ */
    struct acc_t
    {
      float pitch, roll, yaw ;
      float gForceX, gForceY, gForceZ ;
          ///< Acceleration/gForce in each axe = "acceleration vector", in g unit.
      float gForce ;
          ///< gForce applies on the Wiimote : gForce == acceleration (in physic) == "acceleration vector" norm, in g unit.
      float accVarX, accVarY, accVarZ ; ///< Acceleration variations = "move vector", in g/s unit.
      float accVarNorm ; ///< Acceleration variations norm, in g/s unit.

      int timeAcc ;
      float velocityX, velocityY, velocityZ ;
      float distanceX, distanceY, distanceZ ;
    };
    acc_t m_accPrec, m_accCur ;
    // @}

    /**
      * @name Movements informations
      * @{ */
    Vector3d m_vectAtomTranslate ;
    Transform3d m_transfAtomRotate ;
    double m_angleNcJoystickCosDeg, m_angleNcJoystickSinDeg ;
    double m_distCamZoom, m_distCamXTranslate, m_distCamYTranslate ;
    // @}

    /**
      * @name Manage smoothing
      * @{ */
    WIWO<Vector3d> *m_smoothXY ;
    Vector3d m_lastSmooth, m_diffSmooth, m_diffPos ;
    Vector3d m_lastPosSmooth, m_currentPosSmooth ;
    // @}

    /**
      * @name Calibration & Co
      * @{ */
    bool m_activeZoom ;
    int m_nbZoom ;
    QTime m_time ;
    int m_timeFirst, m_timeSecond ; // In ms
    // @}

    /**
      * @name Use to translate ...
      * ... The Wiimote actions to "the construction of the molecule" paradigm.
      * Some attributs let to decrease number of signal,
      * other, let an action to finish itself, etc.
      * @{ */
    bool m_doWork ; ///< If the cursor has really moved.
    bool m_pressedButton ;
    bool m_selectRelease, m_homeRelease ;
    bool m_addRelease, m_delRelease ;
    bool m_crossRelease, m_crossReleaseEnd ;
    int m_lastCrossAction ; ///< 0:Nothing, 1:Right, 2:Down ...
    bool m_crossMenuRelease, m_crossMenuTimeOut ;
    bool m_okMenuRelease, m_okMenuReleaseEnd ;
    bool m_delAllAlready ;

    Vector3d m_isMoveAtom_savePoint0 ;
    // @}


    /**
      * @name Test for acceleration values
      * @{ */
    WIWO<Vector3d> *m_smoothedGravCalFile ;
    Vector3d m_sumSmoothGravCal ;

    WIWO<Vector3d> *m_smoothedOrient ;
    Vector3d m_sumSmoothOrient ;

    int m_nbTickBySecond ;
    int m_t1, m_t2 ;
    // @}


    /**
      * @name Size of the widget
      * @{ */
    int m_xWidget, m_yWidget, m_widthWidget, m_heightWidget ;
    // @}

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      ///< http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html

};


#include "wmavo.inl"

#endif
