
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


#include "wmavo.h"

// Init. const.

const double WmAvo::m_PI=(double)103993/(double)33102 ; // pi approximation.
//const double WmAvo::m_PI=((double)20)*atan(((double)1)/((double)7))+((double)8)*atan((double)3/(double)79) ; // pi value approximation by Euler.
const double WmAvo::m_PI180=m_PI/(double)180 ;
const double WmAvo::m_180PI=(double)180/m_PI ;
const Vector3d WmAvo::m_refPoint0(Vector3d(0., 0., 0.)) ;
const int WmAvo::m_XScreen=QApplication::desktop()->width() ;
const int WmAvo::m_YScreen=QApplication::desktop()->height() ;

WmAvo::WmAvo( int operatingMode ) :
  m_wm(NULL), m_wmNunchuk(NULL),
  m_wmGetWiimote(false), m_wmGetNunchuk(false),
  m_wmGetAlreadyWiimote(false), /*m_wmGetAlreadyNunchuk(false),*/
  m_wmOperatingMode(operatingMode), m_wmOtherPoll(false), m_wmSensitive(PLUGIN_WM_SENSITIVE_DEFAULT),

  m_isWhat(0),

  m_posCursor(0,0),
  m_wmCurrentIrCursorPos(m_refPoint0), m_wmLastIrCursorPos(m_refPoint0),

  m_angleNcJoystickCosDeg(0.0), m_angleNcJoystickSinDeg(0.0),
  m_distCamZoom(0.0), m_distCamXTranslate(0.0), m_distCamYTranslate(0.0),

  m_activeZoom(false), m_nbZoom(0), m_timeFirst(0), m_timeSecond(0),
  m_timeTmp1(0), m_timeTmp2(0),

  m_pressedButton(false),
  m_selectRelease(false), m_homeRelease(false),
  m_addRelease(false), m_delRelease(false),
  m_crossRelease(false), m_crossReleaseEnd(false),
  m_lastCrossAction(0), m_crossMenuRelease(false), m_crossMenuTimeOut(false),
  m_okMenuRelease(false), m_okMenuReleaseEnd(false),
  m_delAllAlready(false),

  m_xWidget(0), m_yWidget(0), m_widthWidget(0), m_heightWidget(0)
{

  //cout << "Constructor WmAvo" << endl ;

  m_time.start() ;

  m_mutex = new QMutex() ;

  m_wii = new CWii(1) ;
  m_wmRumble = new WmRumble(this) ;

  // Smoothing.
  m_smoothXY = new WIWO<Vector3d>(WMAVO_SMOOTH) ;
  m_lastSmooth = m_refPoint0 ;
  m_diffSmooth = m_refPoint0 ;
  m_diffPos = m_refPoint0 ;
  m_lastPosSmooth = m_refPoint0 ;
  m_currentPosSmooth = m_refPoint0 ;

  m_smoothAcc = new WIWO<double>(WMAVO_SMOOTH) ;
  m_lastAccSmooth = 0 ;
  m_diffAccSmooth = 0 ;

  // Calibration & Co.
  //m_hitList.clear() ;

  m_smoothedGravCalFile = new WIWO<Vector3d>(10) ;
  m_sumSmoothGravCal = Vector3d(0,0,0) ;
  m_smoothedOrient = new WIWO<Vector3d>(10) ;
  m_sumSmoothOrient = Vector3d(0,0,0) ;

  m_nbTickBySecond = 0 ;
  m_t1 = m_time.elapsed() ;
  m_t2 = m_time.elapsed() ;
}

WmAvo::~WmAvo()
{
  if( m_smoothXY != NULL )
  {
    delete m_smoothXY ;
    m_smoothXY = NULL ;
  }

  if( m_smoothAcc != NULL )
  {
    delete m_smoothAcc ;
  }

  if( m_mutex != NULL )
  {
    delete m_mutex ;
    m_mutex = NULL ;
  }


  if( m_wmRumble != NULL )
  {
    delete m_wmRumble ;
    m_wmRumble = NULL ;
  }

  if( m_smoothedGravCalFile != NULL )
  {
    delete m_smoothedGravCalFile ;
    m_smoothedGravCalFile = NULL ;
  }
}


/**
  * Realize the connection of the Wiimote :
  *  - search a Wiimote ;
  *  - connect to the Wiimote ;
  *  - initiate some Wiimote elements.
  */
int WmAvo::wmConnect()
{
  int nbFound=0, nbConnect=0 ;
  CWiimote *wm=NULL ;

  if( !m_wmGetWiimote )
  {
    cout << "Searching for wiimotes... Turn them on !" << endl ;
    cout << "Press 1+2 !" << endl ;
    nbFound = m_wii->Find( WMAVO_CONNECTION_TIMEOUT ) ;
    //cout << "Found " << nbFound << " wiimotes" << endl ;

    if( nbFound > 0 ) // !!!
    {
      //cout << "Connecting to wiimotes..." << endl ;
      vector<CWiimote*>& wms=m_wii->Connect() ;
      nbConnect = wms.size() ;
      wm = wms.at(0) ;
    }

    if( nbConnect > 0 )
    {
      m_wmGetWiimote = true ;
      m_wm = wm ;

      m_wm->SetLEDs( CWiimote::LED_1 ) ; // Light LED 1.
      m_wm->IR.SetMode( CIR::ON ) ; // Activate IR.
      m_wm->IR.SetSensitivity(WMAVO_IRSENSITIVITY) ; // Enough

      // For continue comm. (update wm info in wmex class)
      //m_wm->SetFlags( CWiimote::FLAG_CONTINUOUS, 0x0 ) ;
      // For smoothed angle.
      //m_wm->SetFlags( CWiimote::FLAG_SMOOTHING, 0x0 ) ;

      m_wmGetNunchuk = false ;
      m_wmNunchuk = NULL ;

      if( m_wm->ExpansionDevice.GetType() == m_wm->ExpansionDevice.TYPE_NUNCHUK )
      {
        m_wmNunchuk = &(m_wm->ExpansionDevice.Nunchuk) ;
        m_wmGetNunchuk = true ;
      }

      // Activate the rumble feature.
      m_wmRumble->setRumbleEnabled( true ) ;
    }
  }

  //cout << "Connected to " << nbConnect << " wiimotes." << endl ;
  //emit wmConnected(nbConnect) ;

  return nbConnect ;
}


/**
  * Realize the disconnection of the Wiimote.
  */
void WmAvo::wmDisconnect()
{
  m_wmRumble->setRumbleEnabled( false ) ;

  if( m_wmGetWiimote )
  {
    m_wmGetWiimote = false ;
    m_wm->Disconnect() ;
  }

  //emit wmDisconnected() ;
}


/**
  * Poll the Wiimote. It watches what the Wiimote is doing.
  * Currently this method return almost every time TRUE because the
  * accelerometer is sensitive, and another thing to "debug"/improve.
  * @return TRUE if something is realizing ; FALSE else.
  */
bool WmAvo::wmPoll()
{
  bool isPoll=false ;

  if( m_wmGetWiimote )
  {
    m_mutex->lock() ;
    isPoll = m_wii->Poll() ;
    m_mutex->unlock() ;

    //m_timeTmp2 = m_timeTmp1 ;
    //m_timeTmp1 = m_time.elapsed() ;
    //printf( "Tps ecart:%d\n", m_timeTmp1-m_timeTmp2 ) ;

    if( !isPoll )
    { 
      // Few initialisations ...
    }
    else
    { // Poll the wiimotes to get the status like pitch or roll.

      isPoll = false ; // The Wiimote poll, but, is it really interested ?

      switch( m_wm->GetEvent() )
      {
      case CWiimote::EVENT_EVENT :
        isPoll = true ;
        break ;

      case CWiimote::EVENT_DISCONNECT :
      case CWiimote::EVENT_UNEXPECTED_DISCONNECT :
        cout << "--- DISCONNECTED ---" << endl << " [wiimote id " << m_wm->GetID() << "]" << endl ;
        //reloadWiimotes = 1; // To update Wiimote always connected ! Not search a Wiimote disconnected !!
        m_wmGetWiimote = false ;
        //emit wmDisconnected() ;
        break;

      case CWiimote::EVENT_NUNCHUK_INSERTED:
        //reloadWiimotes = 1 ; // ?
        // Get Nunchuk.
        if( m_wmNunchuk == NULL )
        {
          if( m_wm->ExpansionDevice.GetType() == m_wm->ExpansionDevice.TYPE_NUNCHUK )  // No concurrence.
          {
            puts( "case CWiimote::EVENT_NUNCHUK_INSERTED:1" ) ;
            m_wmNunchuk = &(m_wm->ExpansionDevice.Nunchuk) ;
            m_wmGetNunchuk = true ;
          }
        }
        puts( "case CWiimote::EVENT_NUNCHUK_INSERTED:2" ) ;
        break ;

      case CWiimote::EVENT_NUNCHUK_REMOVED:
        m_wmNunchuk = NULL ;
        m_wmGetNunchuk = false ;
        //reloadWiimotes = 1;
        puts( "case CWiimote::EVENT_NUNCHUK_REMOVED:" ) ;
        break ;

      case CWiimote::EVENT_NONE :
        //cout << "EVENT_NONE" << endl ;
        break ;
      case CWiimote::EVENT_STATUS :
        //cout << "EVENT_STATUS" << endl ;
        break ;
      case CWiimote::EVENT_CONNECT :
        //cout << "EVENT_CONNECT" << endl ;
        break ;
      case CWiimote::EVENT_READ_DATA :
        //cout << "EVENT_READ_DATA" << endl ;
        break ;
      default:
        //cout << "default" << endl ;
        break ;
      }
    }

    // Verify if Nunchuk polls.
    // In fact, if a 'Z' is pushed and no movement is realized, the Wiimote detects
    // the change state, but does not indicate that 'Z' is always pushed.
    if( m_wmGetNunchuk )
    {
      float angle, magnitude ;
      m_wmNunchuk->Joystick.GetPosition(angle, magnitude) ; // No concurrence.
      if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z) // No concurrence.
          || m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C) // No concurrence.
          || magnitude>=WMAVO_NC_MINJOY_MOVEALLOWED )
        m_wmOtherPoll = true ;
    }

    // If some action are not finished.
    if( WMAVO_IS(WMAVO_CURSOR_MOVE) || WMAVO_IS(WMAVO_ATOM_MOVE)
        || WMAVO_IS(WMAVO_SELECT) || WMAVO_IS(WMAVO_MENU_OK)
        || m_crossReleaseEnd || m_crossRelease || m_okMenuReleaseEnd )
      m_wmOtherPoll = true ;

    // "Poll" of the Wiimote or other poll.
    if( !(isPoll || m_wmOtherPoll) )
      return false ;
    else
    {
      switch( m_wmOperatingMode )
      {
      case WMAVO_OPERATINGMODE1 :
        wmOperatingMode1() ; break ; // !! Retourner si une action est en cours pour la suite !!
      case WMAVO_OPERATINGMODE2 :
        wmOperatingMode2() ; break ;
      case WMAVO_OPERATINGMODE3 :
        wmOperatingMode3() ; break ;
      default :
        break ;
      }

      m_wmOtherPoll = false ;


      //if(
      //    WMAVO_IS( SMTHG ) || m_crossReleaseEnd || m_crossRelease || m_okMenuReleaseEnd
      // Et pour l'accélération/les données à visualiser en continu
      //  )
        return true ;
      //else
        //return false
    }
  }

  return false ;
}


/**
  * Set the rumble of the Wiimote according to a sequence adjusted by a value.
  * @param gradual The level of the rumble sequence, between 0 (nothing) to 100 (continuous).
  */
void WmAvo::wmSetRumble( int gradual )
{
  if( gradual != m_wmRumble->getGradual() ) // Limit mutex.
    m_wmRumble->setGradual( gradual ) ;
}

/**
  * Enable or Disable the rumble feature.
  * @param state Rumble : On/Off
  */
void WmAvo::wmSetRumbleEnable( bool state )
{
  if( state != m_wmRumble->getRumbleEnabled() ) // Limit mutex.
  {
    m_wmRumble->setRumbleEnabled( state ) ;
    puts( "Change state of the rumble" ) ;

    if( !state )
    {
      if( !m_wmRumble->isQuit() || m_wmRumble->getContinu() )
      {
        m_wmRumble->quit() ;
        puts( "Stop the rumble" ) ;
      }
    }
  }
}


// Ajouter des méthodes surchargées pour rendre la méthode plus simple d'utilisation
// ((fr:)tremor : Petite secousse sismique)
/**
  * Set the rumble of the Wiimote according to the parameters.
  * @param start TRUE to begin rumble ; else FALSE
  * @param continu TRUE to have a continuous vibration ; else FALSE to realize a sequence of rumble.
  * @param loop TRUE to have a loop on the rumble sequence ; else FALSE for only 1 sequence
  * @param gradual [0;100] => predetermined sequence from nothing to continuous rumble
  */
void WmAvo::wmSetRumble( bool start, bool continu, bool loop, int gradual )
{
  if( start )
  {
    if( continu )
    { // Remember: continu => no thread active, just rumble to ON.

      if( !m_wmRumble->getContinu() ) // Limit mutex.
      {
        if( !m_wmRumble->isQuit() )
          m_wmRumble->quit() ; // To change mode.

        m_wmRumble->setContinu( true ) ;
      }
    }
    else
    {
      if( m_wmRumble->getContinu() && !m_wmRumble->isQuit() ) // && m_wmRumble->isQuit() with = bug
        m_wmRumble->quit() ; // It's a another mode ...

      //cout << "    wmavo : Rumble sequence" << endl ;
      if( m_wmRumble->getContinu() ) // Limit mutex.
        m_wmRumble->setContinu( false ) ;

      if( loop != m_wmRumble->getLoop() ) // Limit mutex.
        m_wmRumble->setLoop( loop ) ;

      if( gradual != m_wmRumble->getGradual() ) // Limit mutex.
      {
        m_wmRumble->setGradual( gradual ) ;
        /*
        if( gradual != -1 )
          m_wmRumble->setGradual( gradual ) ;
        else
        {
          m_wmRumble->setDurationTremor( 200 ) ;
          m_wmRumble->setBetweenTremor( 1000 ) ;
          m_wmRumble->setNbTremorBySequence( 1 ) ;

          m_wmRumble->setBetweenSequence( 100 ) ;
          m_wmRumble->setNbSequence( 2 ) ;
        }
        */
      }
    }

    //cout << "    m_wmRumble->isQuit():" << m_wmRumble->isQuit() << endl ;
    //cout << "    m_wmRumble->getLoop():" << m_wmRumble->getLoop() << endl ;
    if( m_wmRumble->isQuit() || !m_wmRumble->getLoop() )
    {
      //cout << "    wmavo : Rumble sequence START" << endl ;
      m_wmRumble->start() ;
    }
    // isQuit() is a test for the continue mode, not to "re-start" the thread.
  }
  else
  {
    //cout << "wmavo: Rumble FIN" << endl ;
    if( !m_wmRumble->isQuit() || m_wmRumble->getContinu() ) // "m_wmRumble->getContinu()" redundant
      m_wmRumble->quit() ;
  }
}


/**
  * Set the sensitivity of the Wiimote (15 to 60)
  */
void WmAvo::wmSetSensitive( int wmSensitive )
{
  //cout << "WmAvo::wmSetSensitive" << endl ;
  if( PLUGIN_WM_SENSITIVE_MIN<wmSensitive && wmSensitive<PLUGIN_WM_SENSITIVE_MAX )
    m_wmSensitive = wmSensitive ;
}




// Renommer la méthode, car il s'agit d'une méthode intermédiaire seulement ...
/**
  * Get "(x,y,z)" of the Wiimote.
  * Retrieve and store (fr:récupérer et stoker) IR cursor of the Wiimote.
  * This is "brut values" without adjustment/calibration.
  * Get values : [0;1024] [0;760] with "jump problem".
  * @return The position targeted by the Wiimote.
  */
Vector3d WmAvo::wmGetIrCursor()
{
  if( m_wm == NULL )
    return m_refPoint0 ;
  else
  {
    // Get Values.
    int x=0, y=0 ;
    m_wm->IR.GetCursorPosition(x,y) ;
    //double x=0, y=0, z=0 ;
    //m_wm->Accelerometer.GetPosition( x, y, z ) ;

    // Save data.
    m_wmLastIrCursorPos = m_wmCurrentIrCursorPos ;

    // Get new value of m_wmCurrentIrCursorPos.
    m_wmCurrentIrCursorPos[0] = (double)x ; //* WMAVO_CURSOR_CALIBRATION_X ;
    m_wmCurrentIrCursorPos[1] = (double)y ; //* WMAVO_CURSOR_CALIBRATION_Y ;
    m_wmCurrentIrCursorPos[2] = m_wm->IR.GetPixelDistance() ; //* WMAVO_CURSOR_CALIBRATION_Z ;

    /*
    cout << "          [x,y]=" << x << " " << y << endl ;
    cout << "   Last:[x,y,z]=" << m_wmLastIrCursorPos[0] << " " << m_wmLastIrCursorPos[1] << " " << m_wmLastIrCursorPos[2] << endl ;
    cout << "Current:[x,y,z]=" << m_wmCurrentIrCursorPos[0] << " " << m_wmCurrentIrCursorPos[1] << " " << m_wmCurrentIrCursorPos[2] << endl ;
    */

    return m_wmCurrentIrCursorPos ;
  }
}



// Voir pour obtenir (ajouté aussi une initialisation des valeurs à zéro pour éviter
// d'atteindre [-oo;+oo] ... :
// - 1 méthode acquisition
// - 1 méthode lissage (ajoutée la moyenne pondérée)
// - 1 méthode accés curseur
/**
  * Based on "brut values", a "smoothed calibration" is applied. Null zones are
  * specified where the return of the method is false value (for tiny little or fast movement).
  * Get values : [-oo;+oo] [-oo;+oo]. The get value is used by the difference with
  * the old value.
  * @return TRUE if there is a move ; else FALSE.
  */
bool WmAvo::wmGetSmoothedCursor()
{
  int a=(m_wmCurrentIrCursorPos[0]>m_wmLastIrCursorPos[0] ? m_wmCurrentIrCursorPos[0]-m_wmLastIrCursorPos[0] : m_wmLastIrCursorPos[0]-m_wmCurrentIrCursorPos[0] ) ;
  int b=(m_wmCurrentIrCursorPos[1]>m_wmLastIrCursorPos[1] ? m_wmCurrentIrCursorPos[1]-m_wmLastIrCursorPos[1] : m_wmLastIrCursorPos[1]-m_wmCurrentIrCursorPos[1] ) ;

  m_lastPosSmooth = m_currentPosSmooth ;

  // Get new value to smooth.
  // Manage the min and max movement.
  // Min movement to avoid the parasit move.
  // Max movement to avoid the jump move when the number of LED change.
  if( !(a>=WMAVO_WM_XY_MINPOINTING_MOVEALLOWED && a<WMAVO_WM_XY_MAXPOINTING_MOVEALLOWED
        && b>=WMAVO_WM_XY_MINPOINTING_MOVEALLOWED && b<WMAVO_WM_XY_MAXPOINTING_MOVEALLOWED) )
  {
    // Initiate to zero ...
    /*
    m_diffPos = Vector3d(0,0,0) ;

    if( m_smoothXY->isFull() )
      m_lastSmooth -= m_smoothXY->getQueue() ;

    m_smoothXY->pushAtBegin( m_diffPos ) ;
    m_lastSmooth += m_smoothXY->getTete() ;
    */

    return false ;
  }
  else
  {
    // voir pour diminuer le nombre de valeur à moyenner qd il y a trop de zero
    // pour permet un re-demarrage plus rapide.
    // Ajouter une moyenne pondérée comme expliqué par Mat

    //m_lastPosSmooth = m_currentPosSmooth ;
    m_diffPos = m_wmCurrentIrCursorPos - m_wmLastIrCursorPos ;

    // Smooth cursor.
    if( m_smoothXY->isFull() )
      m_lastSmooth -= m_smoothXY->getQueue() ;

    m_smoothXY->pushAtBegin( m_diffPos ) ;
    m_lastSmooth += m_smoothXY->getTete() ;
    m_diffSmooth = m_lastSmooth/m_smoothXY->getNbCaseUse() ;

    float acc=wmGetAcc() ;
    //printf( "acc:%1.3f, m_wmSensitive:%d, m_diffSmooth:%2.3f\n", acc, m_wmSensitive, m_diffSmooth[2] ) ;

    m_diffSmooth *= (acc*m_wmSensitive) ;
    m_currentPosSmooth += m_diffSmooth ;

    //printf( "m_lastPosSmooth:\t%4.3f %4.3f %4.3f\n", m_lastPosSmooth[0], m_lastPosSmooth[1], m_lastPosSmooth[2] ) ;
    //printf( "m_currentPosSmooth:\t%4.3f %4.3f %4.3f\n", m_currentPosSmooth[0], m_currentPosSmooth[1], m_lastPosSmooth[2] ) ;
    //printf( "m_diffSmooth:\t%4.3f %4.3f %4.3f\n", m_diffSmooth[0], m_diffSmooth[1], m_diffSmooth[2] ) ;

    // Realize movement.
    if( ((int)floor(m_currentPosSmooth[0]+0.5)-(int)floor(m_lastPosSmooth[0]+0.5))!=0
        || ((int)floor(m_currentPosSmooth[1]+0.5)-(int)floor(m_lastPosSmooth[1]+0.5))!=0 )
      return true ;
    else
      return false ;
  }
}



/**
  * Get the acc values of each axes of the Wiimote, and calcul other acc values
  * to use it ...
  * <br/>This method is used to get a "jerk" (the acceleration derived) to have
  * a ratio which will be applied on the vector of movement.
  * <br/> 
  */
float WmAvo::wmGetAcc()
{
  if( m_wm->isUsingACC() == 0 )
  {
    //cout << "No accel activate, so 'I' am going to activate it." << endl ;
    m_wm->SetMotionSensingMode( CWiimote::ON ) ;
    return 0.0 ;
  }
  else
  {
    /* Test1.
    double acc=fabs(m_wm->Accelerometer.GetGForceElapse()) ;
    acc = (acc<0.01?0.001:acc) ;

    // Smooth acc.
    if( m_smoothAcc->isFull() )
      m_lastAccSmooth -= m_smoothAcc->getQueue() ;

    m_smoothAcc->pushAtBegin( acc ) ;
    m_lastAccSmooth += acc ;
    m_diffAccSmooth = m_lastAccSmooth/m_smoothAcc->getNbCaseUse() ;

    return m_diffAccSmooth ;
    */

    /* Test2.
    double acc=fabs(m_wm->Accelerometer.GetGForceInMS2() - 9.81 ) ;
    double irDiffX, irDiffY, irDiffZ ;
    m_wm->IR.GetCursorDelta( irDiffX, irDiffY, irDiffZ ) ;

    irDiffX = fabs( irDiffX ) ;
    irDiffY = fabs( irDiffY ) ;
    irDiffZ = fabs( irDiffZ ) ;

    if( acc < 0.2 && irDiffX<10.0 && irDiffY<10.0  && irDiffZ<0.75 )
    {
      return 0.05 ;
    }
    else
      return 0.1 ;    
    */

    bool isPM=m_wm->IR.IsInPrecisionMode() ;

    if( isPM )
      return 0.03 ;
    else
      return 0.1 ; 
  }
}


/**
  * Nothing at this time. See wmOperatingMode3() method.
  */
void WmAvo::wmOperatingMode1()
{
}

/**
  * Nothing at this time. See wmOperatingMode3() method.
  */
void WmAvo::wmOperatingMode2()
{
}

/**
  * Apply the operating mode of the Wiimote. It means :
  * how the Wiimote actions will be translate to the "actions necessary
  * to build a molecule".
  */
void WmAvo::wmOperatingMode3()
{
  wmOperatingModeNc1() ;
  wmOperatingModeWm2() ;
  // In this order to let the Wiimote to change the camera.
  // Else the Nunchuk "kills" all Wiimote action on the camera.

  if( WMAVO_IS(WMAVO_CAM_ROTATE_BYWM) || WMAVO_IS(WMAVO_CAM_ROTATE_BYNC) )
    WMAVO_SETON( WMAVO_CAM_ROTATE ) ;
  else
    WMAVO_SETOFF( WMAVO_CAM_ROTATE ) ;
}


/**
  * Sub-method of wmOperatingMode*() method. Get the Wiimote action.
  */
void WmAvo::wmOperatingModeWm2()
{
  if( !m_wm->isUsingIR() )
  { // If IR tracking is off.

    m_wmLastIrCursorPos = m_refPoint0 ;
    m_wmCurrentIrCursorPos = m_refPoint0 ;

    WMAVO_SETOFF( WMAVO_ATOM_TRANSLATE ) ;
    //m_vectAtomTranslate = m_refPoint0 ;

    WMAVO_SETOFF( WMAVO_ATOM_ROTATE ) ;
    //m_transfAtomRotate.matrix().setIdentity() ;
  }
  else
  { // If IR tracking is on, it can realize translation movement
    // for the mouse and the selected atoms.

    wmGetIrCursor() ;
    m_doWork = wmGetSmoothedCursor() ;


    // "Translation movement of the cursor" action.
    transformWmAction1ToMouseMovement() ;

    // "Translation movement of the atom (when button is pressed & moved).
    // + Selection of the atom (when button is released)"
    // action.
    transformWmAction1ToSelectOrTranslateAtom() ;
  }

  // "Create atom" action.
  transformWmAction1ToCreateAtom() ;

  // "Delete one, all, or selected atom" action.
  transformWmAction1ToDeleteAtom() ;

  // "Initiate the camera" action.
  transformWmAction1ToInitCamera() ;

  // "OK menu" action.
  transformWmAction1ToOKMenu() ;

  // "Rotate atom or activate menu" action.
  transformWmAction1ToRotateAtomOrActivateMenu() ;

  // "Move in menu" action.
  transformWmAction1ToMoveInMenu() ;
}


/**
  * Sub-method of wmOperatingMode*() method. Get the Nunchuck action.
  */
void WmAvo::wmOperatingModeNc1()
{
  if( m_wmGetNunchuk )
  {
    float angle, magnitude ;
    m_wmNunchuk->Joystick.GetPosition( angle, magnitude ) ;

    // "Zoom in" action.
    transformNcAction1ToZoomIn() ;

    // "Zoom out" action.
    transformNcAction1ToZoomOut() ;

    // "Translate camera" action.
    transformNcAction1ToTranslateCamera() ;


    // Cancel zoom action in this case too.
    if( !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C))
          && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)) )
    {
      if( WMAVO_IS(WMAVO_CAM_ZOOM) )
      {
        actionZoom(true, true) ;
        //m_isCamZoom = false ;
        WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
        m_distCamZoom = 0.0 ;
      }
    }

    // "Rotate camera" action.
    transformNcAction1ToRotateCameraOrFinalizeTranslateCamera( angle, magnitude ) ;
  }
}


/**
  * Sub-method of wmOperatingMode*() method. Get the Nunchuk action.
  */
void WmAvo::wmOperatingModeNc2()
{
  if( m_wmGetNunchuk )
  {
    float angle, magnitude ;
    m_wmNunchuk->Joystick.GetPosition( angle, magnitude ) ;

    // "Zoom camera" action.
    transformNcAction2ToZoom() ;

    // "Translate camera" action.
    transformNcAction2ToTranslateCamera() ;

    // "Rotation camera" action.
    transformNcAction2ToRotateCamera() ;

    // "Nothing action for the camera" action.
    transformNcAction2ToNothingForCamera() ;

    // "Calculate camera movement" action.
    transformNcAction2ToCalculCameraMovement( angle, magnitude ) ;
  }
}


/**
  * Get the values of the Nunchuk joystick, and calculate/calibrate the angle value on the
  * X-axis and Y-axis to rotate a camera.
  * @param angle The sens of the direction takes by the Nunchuk joystick
  * @param magnitude The magnitude takes by the joystick
  */
void WmAvo::calibrateCamRotateAngles( float angle, float magnitude )
{
  // Manage rotation joystick.
  double angleJoyDeg = (angle-90.0) * (-1.0) ; // Calibrate degres value to look like unit circle ((fr)cercle trigonométrique).
  double angleJoyRad = angleJoyDeg * m_PI180 ; // deg -> rad
  double angleJoyCosRad = cos( angleJoyRad ) ;
  double angleJoySinRad = sin( angleJoyRad ) ;
  m_angleNcJoystickCosDeg = angleJoyCosRad * m_180PI ;
  m_angleNcJoystickSinDeg = angleJoySinRad * m_180PI ;

  // Apply magnetude & calibrate for rotation.
  m_angleNcJoystickCosDeg *= (magnitude/WMAVO_CAM_CALIBRATION_ROTATION) ;
  m_angleNcJoystickSinDeg *= -(magnitude/WMAVO_CAM_CALIBRATION_ROTATION) ;
}


/**
  * Get the values of the Nunchuk joystick, and calculate/calibrate the vector
  * value on the X-axis and Y-axis to translate a camera.
  * @param angle The sens of the direction takes by the Nunchuk joystick
  * @param magnitude The magnitude takes by the joystick
  */
void WmAvo::calibrateCamTranslateDist( float angle, float magnitude )
{
  // Manage rotation joystick.
  double angleJoyDeg = -(angle-90.0) ; // Calibrate degres value to look like unit circle ((fr)cercle trigonométrique).
  double angleJoyRad = angleJoyDeg * m_PI180 ; // deg -> rad
  double angleJoyCosRad = cos( angleJoyRad ) ;
  double angleJoySinRad = sin( angleJoyRad ) ;

  // Calculate translate. Go by radian angle. ((fr) Se baser sur)
  m_distCamXTranslate = (magnitude*10) * angleJoyCosRad * WMAVO_CAM_CALIBRATION_TRANSLATION ;
  m_distCamYTranslate = (magnitude*10) * angleJoySinRad * (-WMAVO_CAM_CALIBRATION_TRANSLATION) ;

  // !!! Définir une vitesse rad/s min et max ...
}


/**
  * Get the values of the Nunchuk joystick, and calculate/calibrate the vector
  * value on the X-axis and Y-axis to zoom a camera.
  * @param angle The sens of the direction takes by the Nunchuk joystick
  * @param magnitude The magnitude takes by the joystick
  */
void WmAvo::calibrateCamZoomDist( float angle, float magnitude )
{
  // Manage rotation joystick.
  double angleJoyDeg = -(angle-90.0) ; // Calibrate degres value to look like unit circle ((fr)cercle trigonométrique).
  double angleJoyRad = angleJoyDeg * m_PI180 ; // deg -> rad
  double angleJoySinRad = sin( angleJoyRad ) ;

  // Calculate translate. Go by radian angle. ((fr) Se baser sur)
  m_distCamZoom = angleJoySinRad * WMAVO_CAM_CALIBRATION_ZOOM_DISTANCE ;
}


/**
  * Adjust the camera speed according to "the time".
  * @param active Zoom or not zoom action
  * @param pause Let the reduce of the response time when the button is pressed,
  * and let to be more precise for the little movements.
  */
bool WmAvo::actionZoom( bool active, bool pause )
{
  if( active )
  {
    if( pause == false )
    { // Zoom mode enabled.

      if( (m_nbZoom++)>=10 && m_activeZoom == false )
        m_activeZoom = true ;
    }
    else
    { // Zoom mode paused.
      m_activeZoom = false ;
      m_nbZoom = 0 ;
    }
  }
  else
  { // Zoom mode stoped.

    m_activeZoom = false ;
    m_nbZoom = 0 ;
  }

  if( m_activeZoom )
  {
    m_activeZoom = false ;
    return true ;
  }
  else
    return false ;
}


/**
 * Limit the movement of the cursor.
 */
void WmAvo::translateCursorToScreen()
{
  // Méthode 1 : ne se servir que de la différences des
  // curseurs lissés qd A est appuyé.

  int x=m_posCursor.x()+(int)floor(m_diffSmooth[0]+0.5) ;
  int y=m_posCursor.y()+(int)floor(m_diffSmooth[1]+0.5) ;

  //cout << "x:" << x << " y:" << y << " w:" << w << " h:" << h << endl ;
  if( x >= m_XScreen )
    m_posCursor.setX( m_XScreen-1 ) ;
  else if( x < 0 )
    m_posCursor.setX( 0 ) ;
  else
    m_posCursor.setX( x ) ;

  /*
  cout << "x:" << m_xWidget << " " << m_widthWidget << endl ;
  if( x >= (m_xWidget+m_widthWidget) )
    m_posCursor.setX( (m_xWidget+m_widthWidget)-1 ) ;
  else if( x < m_xWidget )
    m_posCursor.setX( m_xWidget ) ;
  else
    m_posCursor.setX( x ) ;
  */


  if( y >= m_YScreen )
    m_posCursor.setY( m_YScreen-1 ) ;
  else if( y < 0 )
    m_posCursor.setY( 0 ) ;
  else
    m_posCursor.setY( y ) ;

  /*
  cout << "y:" << m_yWidget << " " << m_heightWidget << endl ;
  if( y >= (m_yWidget+m_heightWidget) )
    m_posCursor.setY( (m_yWidget+m_heightWidget)-1 ) ;
  else if( y < m_yWidget )
    m_posCursor.setY( m_yWidget ) ;
  else
    m_posCursor.setY( y ) ;
  */
}


/**
 * Transform a Wiimote action to a mouse action
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToMouseMovement()
{
  if( m_wm->Buttons.isPressed(CButtons::BUTTON_B) )
  {
    WMAVO_SETON(WMAVO_CURSOR_MOVE) ;

    if( m_doWork )
      translateCursorToScreen() ;
  }
  else
    WMAVO_SETOFF(WMAVO_CURSOR_MOVE) ;

  return true ;
}

/**
 * Transform a Wiimote action to either "select atom" action, or "translate atom" action.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToSelectOrTranslateAtom()
{
  // Translation movement of the atom (when button is pressed & moved).
  // + Selection of the atom (when button is released).
  // Nota bene : Everything works round by round, poll after poll.
  if( !WMAVO_IS(WMAVO_MENU_ACTIVE)
      && m_wm->Buttons.isPressed(CButtons::BUTTON_A) )
  {
    if( m_wm->Buttons.isPressed(CButtons::BUTTON_B) 
        || (WMAVO_IS(WMAVO_SELECT_MULTI)&&m_pressedButton) ) // To avoid a bug during the end of the action : A+B -> B -> 0
    { // Here A+B => Selection multiple atom.

      if( !WMAVO_IS(WMAVO_SELECT_MULTI) )
      {
        m_pressedButton = true ; // To avoid having another action.
        m_selectRelease = false ;

        // Activate selection.
        WMAVO_SETON( WMAVO_SELECT_MULTI ) ;

        // Desactivate others actions & possibilities.
        WMAVO_SETOFF( WMAVO_ATOM_TRANSLATE ) ;
        WMAVO_SETOFF( WMAVO_ATOM_MOVE ) ;
      }
    }
    else
    { // Here is the management of movement of an atom.
      // And selection a single atom.

      if( !WMAVO_IS(WMAVO_ATOM_MOVE) )
      { // Activate the possibility of movement of an atom.

        m_pressedButton = true ; // To avoid having another action.

        WMAVO_SETON( WMAVO_ATOM_MOVE ) ;
        m_isMoveAtom_savePoint0 = m_currentPosSmooth ;

        // Desactivate others actions & possibilities.
        //WMAVO_SETOFF( WMAVO_SELECT ) ;
        //WMAVO_SETOFF( WMAVO_SELECT_MULTI ) ;
      }
      else
      { // Try to activate the movement of an atom.

        if( !WMAVO_IS(WMAVO_ATOM_TRANSLATE) )
        {
          if( m_timeFirst == 0 )
            m_timeFirst = m_time.elapsed() ;
          m_timeSecond = m_time.elapsed() ;

          if( (m_timeSecond-m_timeFirst) > 500 )
            WMAVO_SETON( WMAVO_ATOM_TRANSLATE ) ;
        }
      }

      // Movement of an atom.
      //if( m_doWork && WMAVO_IS(WMAVO_ATOM_TRANSLATE) )
        //calculAtomMove() ;

      // Activate possibility of selection of an atom.
      if( !m_selectRelease && !WMAVO_IS(WMAVO_SELECT) )
      {
        //cout << "Active la possibilité d'une sélection" << endl ;
        m_selectRelease = true ;
      }
    }
  }
  else
  {
    // WMAVO_IS(WMAVO_SELECT) & WMAVO_IS(WMAVO_ATOM_TRANSLATE)
    // here to finish this actions.

    if( m_selectRelease || WMAVO_IS(WMAVO_ATOM_TRANSLATE) 
        || WMAVO_IS(WMAVO_SELECT) || WMAVO_IS(WMAVO_SELECT_MULTI) )
    {
      m_pressedButton = false ;
      m_timeFirst = 0 ;
      m_timeSecond = 0 ;

      if( m_selectRelease && !WMAVO_IS(WMAVO_ATOM_TRANSLATE) )
      { // Try to activate the selection of an atom.

        m_selectRelease  = false ; // Will disable the feature in the next round.

        WMAVO_SETON( WMAVO_SELECT ) ;
      }
      else
      { // Desactivate all actions & possibilities.

        m_selectRelease  = false ;

        WMAVO_SETOFF( WMAVO_ATOM_TRANSLATE ) ;
        WMAVO_SETOFF( WMAVO_ATOM_MOVE ) ;
        WMAVO_SETOFF( WMAVO_SELECT ) ;
        WMAVO_SETOFF( WMAVO_SELECT_MULTI ) ;
      }
    }
  }

  return true ;
}


/**
 * Transform a Wiimote action to "create atom" action.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToCreateAtom()
{
  if( m_wm->Buttons.isPressed(CButtons::BUTTON_PLUS) )
  {
    //cout << "Bouton PLUS" << endl ;

    if( !m_pressedButton )
    {
      m_pressedButton = true ;
      m_addRelease = true ;
      WMAVO_SETON( WMAVO_CREATE ) ;
    }

    if( WMAVO_IS(WMAVO_CREATE) )
    {
      if( m_timeFirst == 0 )
        m_timeFirst = m_time.elapsed() ;
      m_timeSecond = m_time.elapsed() ;

      // Need to move cursor.
      if( m_doWork && WMAVO_IS(WMAVO_CREATE) && (m_timeSecond-m_timeFirst)>500 )
        translateCursorToScreen() ;
    }
  }
  else
  {
    if( m_addRelease )
    {
      m_addRelease = false ;
      m_pressedButton = false ;
      WMAVO_SETOFF( WMAVO_CREATE ) ;

      m_timeFirst = 0 ;
      m_timeSecond = 0 ;
    }
  }

  return true ;
}


/**
 * Transform a Wiimote action to "delete atom" action.
 * Either "delete atom" action if the user presses less than some time,
 * or "delete all atoms" action if the user presses more then some time.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToDeleteAtom()
{
  if( m_wm->Buttons.isPressed(CButtons::BUTTON_MINUS) )
  {

    // Just a secure to "close" an action which need a 2nd passage in the "end of the action".
    // This code is redundant in "else". ==>>
    if( m_delRelease && !m_delAllAlready && WMAVO_IS(WMAVO_DELETE) )
    {
      m_pressedButton = false ;
      m_delRelease = false ;
      WMAVO_SETOFF( WMAVO_DELETE ) ;
    }
    // <<==

    if( !m_pressedButton )
    {
      m_pressedButton = true ;
      m_delRelease = true ;
      m_delAllAlready = false ;

      m_timeFirst = m_time.elapsed() ;
      m_timeSecond = m_timeFirst ;
    }

    if( m_delRelease )
    {
      if( m_timeFirst == 0 )
        m_timeFirst = m_time.elapsed() ;
      m_timeSecond = m_time.elapsed() ;

      if( !m_delAllAlready && (m_timeSecond-m_timeFirst)>3000 )
      {
        WMAVO_SETON( WMAVO_DELETEALL ) ;
        m_delAllAlready = true ;
      }
      else
        WMAVO_SETOFF( WMAVO_DELETEALL ) ;
    }
  }
  else
  {
    if( m_delRelease )
    {
      if( m_delAllAlready )
      {
        m_pressedButton = false ;
        m_delRelease = false ;
        WMAVO_SETOFF( WMAVO_DELETEALL ) ;

        m_timeFirst = 0 ;
        m_timeSecond = 0 ;
      }
      else
      {
        if( !WMAVO_IS(WMAVO_DELETE) )
          WMAVO_SETON( WMAVO_DELETE ) ;
        else
        {
          m_pressedButton = false ;
          m_delRelease = false ;
          WMAVO_SETOFF( WMAVO_DELETE ) ;

          m_timeFirst = 0 ;
          m_timeSecond = 0 ;
        }
      }
    }
  }

  return true ;
}



/**
 * Transform a Wiimote action to "initiate camera" action.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToInitCamera()
{
  if( m_wm->Buttons.isPressed(CButtons::BUTTON_HOME) )
  {
    if( !m_pressedButton )
    {
      m_pressedButton = true ;
      m_homeRelease = true ;
      //m_isCamInit = true ;
      WMAVO_SETON( WMAVO_CAM_INITIAT ) ;
    }
    else
      //m_isCamInit = false ;
      WMAVO_SETOFF( WMAVO_CAM_INITIAT ) ;
  }
  else
  {
    if( m_homeRelease )
    {
      m_homeRelease = false ;
      m_pressedButton = false ;
      //m_isCamInit = false ;
      WMAVO_SETOFF( WMAVO_CAM_INITIAT ) ;
    }
  }

  return true ;
}


/**
 * Transform a Wiimote action to "OK menu" action.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToOKMenu()
{
  // "MENU_OK" is active on the release else there is a "conflit" with "SELECT".
  // Ex. : When the menu is active, press A for "MENU_OK",
  //       release A for "SELECT" ... 2 OK for 2 use in "once".
  if( WMAVO_IS( WMAVO_MENU_ACTIVE )
      && m_wm->Buttons.isPressed(CButtons::BUTTON_A) )
  {

    // Just a secure to "close" an action which need a 2nd passage in the "end of the action".
    // This code is redundant in "else". ==>>
    if( m_okMenuReleaseEnd && WMAVO_IS(WMAVO_MENU_OK) )
    {
      m_okMenuReleaseEnd = false ;
      WMAVO_SETOFF( WMAVO_MENU_OK ) ; // Only once !
    }
    // <<==

    if( !m_pressedButton )
    {
      m_pressedButton = true ;
      m_okMenuRelease = true ;
      m_okMenuReleaseEnd = true ;
      //WMAVO_SETON( WMAVO_MENU_OK ) ;
    }
    /*
    else
    {
      WMAVO_SETOFF( WMAVO_MENU_OK ) ; // Only once !
      WMAVO_SETOFF( WMAVO_MENU_ACTIVE ) ;
    }
    */
  }
  else
  {
    if( m_okMenuRelease || m_okMenuReleaseEnd  )
    {
      if( m_okMenuRelease )
      {
        m_okMenuRelease = false ;
        m_pressedButton = false ;
        WMAVO_SETON( WMAVO_MENU_OK ) ;
      }
      else
      {
        m_okMenuReleaseEnd = false ;
        WMAVO_SETOFF( WMAVO_MENU_OK ) ; // Only once !
      }
    }
  }

  return true ;
}



/**
 * Transform a Wiimote action to :
 * either "rotate camera" action,
 * or "activate menu" if the right d-pad is pressed more than some time.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToRotateAtomOrActivateMenu()
{
  // Move in 90° rotate on XYZ axis.
  if( !WMAVO_IS(WMAVO_MENU_ACTIVE)
      && (m_wm->Buttons.isPressed(CButtons::BUTTON_RIGHT)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_LEFT)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_UP)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_DOWN) )
    )
  {

    // Just a secure to "close" an action which need a 2nd passage in the "end of the action".
    // This code is redundant in "else". ==>>
    if( m_crossReleaseEnd && WMAVO_IS(WMAVO_CAM_ROTATE_BYWM) )
    {
      WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYWM ) ;

      m_crossReleaseEnd = false ;
      m_lastCrossAction = 0 ;

      m_angleNcJoystickCosDeg = 0.0 ;
      m_angleNcJoystickSinDeg = 0.0 ;
    } // <<==

    if( !m_pressedButton && !WMAVO_IS(WMAVO_CAM_ROTATE) )
    {
      m_pressedButton = true ;
      m_crossRelease = true ;

      //if( m_timeFirst == 0 )
        m_timeFirst = m_time.elapsed() ;

      if( m_wm->Buttons.isPressed(CButtons::BUTTON_RIGHT) )
        m_lastCrossAction = 1 ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_LEFT) )
        m_lastCrossAction = 3 ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_UP) )
        m_lastCrossAction = 4 ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_DOWN) )
        m_lastCrossAction = 2 ;
    }

    if( !WMAVO_IS(WMAVO_MENU_ACTIVE) )
    {
      //if( m_crossRelease )
        m_timeSecond = m_time.elapsed() ;

      if( m_wm->Buttons.isPressed(CButtons::BUTTON_RIGHT)
          && (m_timeSecond-m_timeFirst)>WMAVO_ACTIVATEMENU_AFTER )
      { // Activate menu.

          //m_menuActivate = true ;
          WMAVO_SETON( WMAVO_MENU_ACTIVE ) ;
      }
    }
  }
  else
  {
    if( m_crossRelease )
    {
      // !!! Voir pour l'ajouter sur les autres fonctionnalités l'utilisant !!!
      m_timeSecond = m_time.elapsed() ;

      if( (m_timeSecond-m_timeFirst) < WMAVO_ACTIVATEMENU_AFTER )
      {
        //m_isCamRotateWm = true ;
        WMAVO_SETON( WMAVO_CAM_ROTATE_BYWM ) ;

        m_crossReleaseEnd = true ;

        // For a 90° rotation.
        //double cstRot=45.0/Avogadro::ROTATION_SPEED ; // 0.005 : cst. in Avogadro, apply in the camera rotation.

        double cstRot=90 ;

        m_angleNcJoystickCosDeg = 0.0 ;
        m_angleNcJoystickSinDeg = 0.0 ;

        switch( m_lastCrossAction )
        {
        case 1 :
          m_angleNcJoystickCosDeg = cstRot ;
          m_angleNcJoystickSinDeg = 0 ; break ;
        case 2 :
          m_angleNcJoystickCosDeg = 0.0 ;
          m_angleNcJoystickSinDeg = -cstRot ; break ;
        case 3 :
          m_angleNcJoystickCosDeg = -cstRot ;
          m_angleNcJoystickSinDeg = 0.0 ; break ;
        case 4 :
          m_angleNcJoystickCosDeg = 0.0 ;
          m_angleNcJoystickSinDeg = cstRot ; break ;
        default : break ;
        }
      }
      else
      {
        //m_isCamRotateWm = false ;
        WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYWM ) ;
        m_crossReleaseEnd = false ;
      }

      m_pressedButton = false ;
      m_crossRelease = false ;

      m_timeFirst = 0 ;
      m_timeSecond = 0 ;
    }
    else
    {
      if( m_crossReleaseEnd )
      {
        //m_isCamRotateWm = false ;
        WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYWM ) ;

        m_crossReleaseEnd = false ;
        m_lastCrossAction = 0 ;

        m_angleNcJoystickCosDeg = 0.0 ;
        m_angleNcJoystickSinDeg = 0.0 ;
      }
    }
  }

  return true ;
}


/**
 * Transform a Wiimote action to "Menu in mode" action.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformWmAction1ToMoveInMenu()
{

  // Move in menu mode.
  if( WMAVO_IS(WMAVO_MENU_ACTIVE)
      && (m_wm->Buttons.isPressed(CButtons::BUTTON_RIGHT)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_LEFT)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_UP)
          || m_wm->Buttons.isPressed(CButtons::BUTTON_DOWN) )
    )
  {

    m_timeSecond = m_time.elapsed() ;

    // m_timeFirst : for the 1st passage, we do not care about it value.
    if( (m_timeSecond-m_timeFirst) > 200 )
      m_crossMenuTimeOut = false ;

    if( !m_pressedButton
        || (m_crossMenuRelease && !m_crossMenuTimeOut) ) // Else, it is not come back here to realize a loop of certain ms.
    {
      m_pressedButton = true ;
      m_crossMenuRelease = true ;
      m_crossMenuTimeOut = true ;

      m_timeFirst = m_time.elapsed() ;

      if( m_wm->Buttons.isPressed(CButtons::BUTTON_RIGHT) )
        WMAVO_SETON( WMAVO_MENU_RIGHT ) ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_DOWN) )
        WMAVO_SETON( WMAVO_MENU_DOWN ) ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_LEFT) )
        WMAVO_SETON( WMAVO_MENU_LEFT ) ;
      else if( m_wm->Buttons.isPressed(CButtons::BUTTON_UP) )
        WMAVO_SETON( WMAVO_MENU_UP ) ;
    }
    else
    {
      WMAVO_SETOFF( WMAVO_MENU_RIGHT ) ;
      WMAVO_SETOFF( WMAVO_MENU_DOWN ) ;
      WMAVO_SETOFF( WMAVO_MENU_LEFT ) ;
      WMAVO_SETOFF( WMAVO_MENU_UP ) ;
    }

  }
  else
  {
    if( m_crossMenuRelease )
    {
      m_crossMenuRelease = false ;
      m_pressedButton = false ;

      WMAVO_SETOFF( WMAVO_MENU_RIGHT ) ;
      WMAVO_SETOFF( WMAVO_MENU_DOWN ) ;
      WMAVO_SETOFF( WMAVO_MENU_LEFT ) ;
      WMAVO_SETOFF( WMAVO_MENU_UP ) ;

      m_timeFirst = 0 ;
      m_timeSecond = 0 ;
    }
  }

  return true ;
}



/**
 * Transform a Nunchuk action to "Zoom in" action.
 * <br/>for the 1st nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction1ToZoomIn()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C)
        && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)) )
  {
    if( actionZoom(true) == true )
    {
      //calibrateCamZoomDist(true) ;
      //m_isCamZoom = true ;
      WMAVO_SETON( WMAVO_CAM_ZOOM ) ;

      if( m_distCamZoom > 0 )
        m_distCamZoom = 0.0 ;
      else
        m_distCamZoom = m_nbZoom*(-WMAVO_ATOM_MAX_MOVE_FOR_PROHITED_MOVE) ;
    }
  }

  return true ;
}



/**
 * Transform a Nunchuk action to "Zoom out" action.
 * <br/>for the 1st nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction1ToZoomOut()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)
        && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C)) )
  {

    if( actionZoom(true) == true )
    {
      //calibrateCamZoomDist(true) ;
      //m_isCamZoom = true ;
      WMAVO_SETON( WMAVO_CAM_ZOOM ) ;

      if( m_distCamZoom < 0 )
        m_distCamZoom = 0.0 ;
      else
        m_distCamZoom = m_nbZoom*WMAVO_ATOM_MAX_MOVE_FOR_PROHITED_MOVE ;
    }
  }

  return true ;
}


/**
 * Transform a Nunchuk action to "Translate camera" action.
 * <br/>for the 1st nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction1ToTranslateCamera()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_ALL) )
  {
    // Cancel zoom.
    if( WMAVO_IS(WMAVO_CAM_ZOOM) )
    {
      actionZoom(false) ;

      WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
      m_distCamZoom = 0.0 ;
    }

    //m_isCamTranslate = true ;
    WMAVO_SETON( WMAVO_CAM_TRANSLATE ) ;
  }
  else
  { // Every other case, must cancel translate action.
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }

  return true ;
}



/**
 * Transform a Nunchuk action to "Rotate camera" action,
 * <br/>for the 1st nunchuk operating mode, and manage the 
 * <br/>camera translation.
 * @return TRUE if an action has been realised ; FALSE else.
 * @param angle The sens of the direction takes by the Nunchuk joystick
 * @param magnitude The magnitude takes by the joystick
 */
bool WmAvo::transformNcAction1ToRotateCameraOrFinalizeTranslateCamera( float angle, float magnitude )
{
  if( magnitude < WMAVO_NC_MINJOY_MOVEALLOWED )
  {
    WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }
  else
  {
    if( !WMAVO_IS(WMAVO_CAM_TRANSLATE) )
    { // Rotation.

      WMAVO_SETON( WMAVO_CAM_ROTATE_BYNC ) ;
      calibrateCamRotateAngles( angle, magnitude ) ;
    }
    else
    { // Translation.

      calibrateCamTranslateDist( angle, magnitude ) ;
    }
  }

  return true ;
}



/**
 * Transform a Nunchuk action to "Zoom camera" action,
 * <br/>for the 2nd nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction2ToZoom()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C)
      && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)) )
  {
    WMAVO_SETON( WMAVO_CAM_ZOOM ) ;
    WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }

  return true ;
}


/**
 * Transform a Nunchuk action to "Translate camera" action,
 * <br/>for the 2nd nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction2ToTranslateCamera()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)
      && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C)) )
  {
    WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
    WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETON( WMAVO_CAM_TRANSLATE ) ;
  }

  return true ;
}


/**
 * Transform a Nunchuk action to "Rotate camera" action,
 * <br/>for the 2nd nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction2ToRotateCamera()
{
  if( !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_C))
        && !(m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_Z)) )
  {
    WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
    WMAVO_SETON( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }

  return true ;
}


/**
 * Transform a Nunchuk action to "Nothing action for the camera" action,
 * <br/>for the 2nd nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 */
bool WmAvo::transformNcAction2ToNothingForCamera()
{
  if( m_wmNunchuk->Buttons.isPressed(CNunchukButtons::BUTTON_ALL) )
  { // Cancel every movement.

    WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
    WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }

  return true ;
}


/**
 * Transform a Nunchuk action to "Calculate camera movement" action,
 * <br/>for the 2nd nunchuk operating mode.
 * @return TRUE if an action has been realised ; FALSE else.
 * @param angle The sens of the direction takes by the Nunchuk joystick
 * @param magnitude The magnitude takes by the joystick
 */
bool WmAvo::transformNcAction2ToCalculCameraMovement( float angle, float magnitude )
{
  if( magnitude < WMAVO_NC_MINJOY_MOVEALLOWED )
  {
    WMAVO_SETOFF( WMAVO_CAM_ZOOM ) ;
    WMAVO_SETOFF( WMAVO_CAM_ROTATE_BYNC ) ;
    WMAVO_SETOFF( WMAVO_CAM_TRANSLATE ) ;
  }
  else
  {
    if( WMAVO_IS(WMAVO_CAM_ROTATE_BYNC) )
      calibrateCamRotateAngles( angle, magnitude ) ;

    if( WMAVO_IS(WMAVO_CAM_TRANSLATE) )
      calibrateCamTranslateDist( angle, magnitude ) ;

    if( WMAVO_IS(WMAVO_CAM_ZOOM) )
      calibrateCamZoomDist( angle, magnitude ) ;
  }

  return true ;
}
