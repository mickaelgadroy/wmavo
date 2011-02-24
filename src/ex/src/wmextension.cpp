
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2006,2008,2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch

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


#include "wmextension.h"

namespace Avogadro
{

  /**
    * Constructor.
    * @param parent Instanciate and initiate by Avogadro
    */
  WmExtension::WmExtension(QObject *parent) :
    Extension(parent),

    m_widget(NULL), m_wmTool(NULL), m_drawTool(NULL),
    m_wmavoThread(NULL), m_wmIsConnected(false),

    m_isMoveAtom(false), m_wmIsAlreadyConnected(false), m_wmSensitive(PLUGIN_WM_SENSITIVE_DEFAULT),
    m_pointRefBarycenter(Vector3d(0,0,0)), m_sumOfWeights(0), m_atomsBarycenter(Vector3d(0,0,0)),
    m_testEventPress(false),

    m_addHydrogens(WMEX_ADJUST_HYDROGEN),
    m_isAtomDraw(false), m_isBondOrder(false),
    m_drawBeginAtom(false), m_drawCurrentAtom(false), m_drawBond(false),
    m_hasAddedBeginAtom(false), m_hasAddedCurAtom(false), m_hasAddedBond(false),
    m_beginPosDraw(Vector3d(0,0,0)), m_curPosDraw(Vector3d(0,0,0)),
    m_lastCursor(QPoint(0,0)),
    m_beginAtomDraw(NULL), m_curAtomDraw(NULL), m_bondDraw(NULL),
    m_atomicNumberCurrent(WMEX_CREATEDATOMDEFAULT),
    m_timeFirst(0), m_timeSecond(0), m_canDrawOther(false),

    m_isCalculDistDiedre(false),
    m_isRenderRect(false), m_rectP1(QPoint(0,0)), m_rectP2(QPoint(0,0)),

    m_menuActive(false),
    m_contextMenuCurrent(NULL), m_contextMenuMain(NULL),
    m_cancelAct(NULL), m_periodicTableAct(NULL),
    m_contextMenuMeasure(NULL),
    m_noDistAct(NULL), m_distAct(NULL), m_angleAct(NULL), m_diedreAct(NULL),
    m_contextMenuFragment(NULL),
    m_insertFragAct(NULL),
    m_addSubstituteFragAct(NULL)
  {
    // Initiate some data.
    m_time.start() ;
    m_cameraInitialViewPoint.matrix().setIdentity() ;
    m_periodicTable = new PeriodicTableView(/*m_widget*/) ;

    // Initiate the Wiimote pull-down menu before IHM starts.
    initPullDownMenu() ;

    // Initiate some attributs to realize mouse simulations.
    m_p = new QPoint(1,1) ;
    m_me1 = new QMouseEvent(QEvent::MouseButtonPress, *m_p, Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
    m_me2 = new QMouseEvent(QEvent::MouseMove, *m_p, Qt::NoButton, Qt::LeftButton, Qt::NoModifier) ;
    m_me3 = new QMouseEvent(QEvent::MouseButtonRelease, *m_p, Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
  }


  /**
    * Destructor.
    */
  WmExtension::~WmExtension()
  {
    delete( m_me1 ) ;
    delete( m_me2 ) ;
    delete( m_me3 ) ;
    delete( m_p ) ;

    if( m_wmavoThread != NULL )
    {
      m_wmavoThread->quit() ;
      delete( m_wmavoThread ) ;
    }
  }


  /**
    * Define the attribut which contains all actions in the Wiimote pull-down menu.
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    */
  QList<QAction*> WmExtension::actions() const
  {
    return m_pullDownMenuActions;
  }


  /**
    * Indicates where the Wiimote menu bar is located in the Avogadro menu bar.
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    * @param action Instanciate and initiate by Avogadro
    */
  QString WmExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&Wiimote") /*+ '>' + tr("4nd WiimoteAppz")*/ ;
  }


  /**
    * Really, I do not know when it is called. I never see the cout() displays it.
    * @param event A mouse event ...
    */
  void WmExtension::mouseMoveEvent( QMouseEvent *event )
  {
    cout << "WmExtension::mouseMoveEvent !?" << endl ;
    //m_widget->update() ;
  }


  /**
    * Perform the action realises by the user through the action of clicking on the menu bar (on the path defined in menuPath() method).
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    * @param action The id of the action define in m_wmMenuState structure.
    * @param widget The GLWidget object instanciate by Avogadro
    */
  QUndoCommand* WmExtension::performAction( QAction *action, GLWidget *widget )
  {
    //cout << "WmExtension::performAction" << endl ;

    QUndoCommand *undo=NULL ;
    int menuAction = action->data().toInt() ;


    // Initiate some data if it is not realize again.
    if( m_widget == NULL )
    {
      m_widget = widget ;

      // Let the user choose the global quality in the option.
      // So let it in commentary.
      //m_widget->setQuality(-1) ; // PAINTER_MAX_DETAIL_LEVEL

      // If quick render activates, it can produce a visual bug. It is du in openGL code
      // optimisation which can no update some figure and blabla and blabla ...
      //m_widget->setQuickRender(true) ;

      if( !initContextMenu() )
        emit message( "A problem appears in initContextMenu() method. At least signal is not connected. (Launch Avogadro in command line to see much informations." ) ;
    }

    // Realize other initialization according to the case.
    switch( menuAction )
    {

    case ConnectWm :

      if( !wmBeginUse() )
      {
        QString msg = "A problem appears in beginWiimoteUse() method. (Launch Avogadro in command line to see much informations." ;

        emit message( msg ) ;
        qDebug() << msg ;
      }
      else
      {
        // The camera initialization is here because some data in Avogadro are not initialize
        // when constructor is called.
        m_cameraInitialViewPoint = m_widget->camera()->modelview() ;

        m_pullDownMenuActions.at(ConnectWm)->setEnabled(false) ;
        m_pullDownMenuActions.at(DisconnectWm)->setEnabled(true) ;
      }
      break ;

    case DisconnectWm :

      /* Dead battery bug !

          [INFO] Wiimote disconnected [id 1].
          --- DISCONNECTED ---
           [wiimote id 1]
          QThread: Destroyed while thread is still running
          0xbfd34848
          The Wiimote is not connected, so it can not be disconnected ...
          Erreur de segmentation
        */


      /*
      It is in commentary because the Wiimote library has some difficulties to this job ...
      Be careful if you erase commentary, there is a part in commentary in beginWiimoteUse() method.

      if( m_wmavoThread != NULL )
      {
        cout << "wmex::PerformAction case Disconnect 1" << endl ;
        m_wmavoThread->wmDisconnect() ;
        cout << "wmex::PerformAction case Disconnect 2" << endl ;

        m_pullDownMenuActions.at(ConnectWm)->setEnabled(true) ;
        m_pullDownMenuActions.at(DisconnectWm)->setEnabled(false) ;
      }
      */
      break ;

    case OpMode1 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE1) ;
      break ;

    case OpMode2 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE2) ;
      break ;

    case OpMode3 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE3) ;
      break ;

    default :
      break ;
    }

    return undo;
  }


  /**
    * This slot is used to let the wrapper to communicate all change of the input device (Wiimote).
    * When the state of the input device changes, it informs the WmExtension class by a signal to
    * this method.
    * @param wmData Structure uses to store all informations between the WmAvo class et the WmExtension class.
    */
  void WmExtension::wmActions( WmAvoThread::wmDataTransfert wmData )
  {
    //cout << endl << "WmExtension::wmActions : Signal received" << endl ;

    //m_widget->getQuickRender() ; ??? Test if is activate and desactivate it !

    QPoint posCursor=wmData.posCursor ;
    int wmavoAction=wmData.wmActions ;
    Vector3d pos3dCurrent=wmData.pos3dCur ;
    Vector3d pos3dLast=wmData.pos3dLast ;
    double rotCamAxeXDeg=wmData.angleCamRotateXDeg ;
    double rotCamAxeYDeg=wmData.angleCamRotateYDeg ;
    double distCamXTranslate=wmData.distCamTranslateX ;
    double distCamYTranslate=wmData.distCamTranslateY ;
    double distCamZoom=wmData.distCamZoom ;
    int nbDotsDetected=wmData.nbDotsDetected ;
    int nbSourcesDetected=wmData.nbSourcesDetected ;
    int distBetweenSource=wmData.distBetweenSources ;
    //cout << nbDotsDetected << " " << nbSourcesDetected << " " << distBetweenSource << endl ;

    if( m_wmIsConnected == true )
    {
      //
      // Active WmTool, connect signal & few initialisation.

      if( WMAVO_IS2(wmavoAction,WMAVO_CURSOR_MOVE)
          || WMAVO_IS2(wmavoAction,WMAVO_SELECT) || WMAVO_IS2(wmavoAction,WMAVO_CREATE)
          || WMAVO_IS2(wmavoAction,WMAVO_DELETE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_MOVE)
          || WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE)
          || WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE) || WMAVO_IS2(wmavoAction,WMAVO_CAM_ZOOM)
          || WMAVO_IS2(wmavoAction,WMAVO_CAM_TRANSLATE) || WMAVO_IS2(wmavoAction,WMAVO_CAM_INITIAT)
          || m_wmTool==NULL
        )
      {
        //initSizeWidgetForWmAvo() ;

        // Here to avoid to activate the wmTool always.
        initAndActiveForWmToolMenu() ;
      }

      // Here, because the m_wmTool must be initiated before.
      sendWmInfoToWmTool( m_wmIsConnected, nbDotsDetected, nbSourcesDetected, distBetweenSource ) ;

      //
      // Avogadro actions.

      // Adjustment for the rotation of atoms.
      if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE_BYWM) && m_widget->selectedPrimitives().size()>=2 )
      {
        WMAVO_SETOFF2( wmavoAction, WMAVO_CAM_ROTATE_BYWM ) ;
        WMAVO_SETON2( wmavoAction, WMAVO_ATOM_MOVE ) ;
        WMAVO_SETON2( wmavoAction, WMAVO_ATOM_ROTATE ) ;
      }

      transformWrapperActionToMoveAtom( wmavoAction, pos3dCurrent, pos3dLast, rotCamAxeXDeg, rotCamAxeYDeg ) ;
      transformWrapperActionToMoveMouse( wmavoAction, posCursor ) ;
      transformWrapperActionToSelectAtom( wmavoAction, posCursor ) ;
      transformWrapperActionToCreateAtomBond( wmavoAction, posCursor, m_pointRefBarycenter ) ;
      transformWrapperActionToDeleteAllAtomBond( wmavoAction ) ;
      transformWrapperActionToRemoveAtomBond( wmavoAction, posCursor ) ;

      transformWrapperActionToRotateCamera( wmavoAction, m_pointRefBarycenter, rotCamAxeXDeg, rotCamAxeYDeg ) ;
      transformWrapperActionToTranslateCamera( wmavoAction, m_pointRefBarycenter, distCamXTranslate, distCamYTranslate ) ;
      transformWrapperActionToZoomCamera( wmavoAction, m_pointRefBarycenter, distCamZoom ) ;
      transformWrapperActionToInitiateCamera( wmavoAction, m_pointRefBarycenter ) ;

      transformWrapperActionToShowContextMenu( wmavoAction, posCursor ) ;

      //
      // Update Avogadro to see modification.

      updateForAvoActions1( wmavoAction ) ;
      updateForAvoActions2( wmavoAction ) ;
      updateForAvoActions3( wmavoAction ) ;
    }
    else
    {
      qDebug() << "Wiimote not connected." ;

      int nbDotsDetected=wmData.nbDotsDetected ;
      int nbSourcesDetected=wmData.nbSourcesDetected ;
      int distBetweenSource=wmData.distBetweenSources ;
      sendWmInfoToWmTool( m_wmIsConnected, nbDotsDetected, nbSourcesDetected, distBetweenSource ) ;
    }
  }


  /**
    * This Method realizes various things about the Wiimote :
    * - Initiate the Wiimote class (it is more complicated, but it looks like at this).
    * - Initiate some signal between the WmAvoThread ("Wiimote class") and the WmExtension class ;
    * - Start the Wiimote connection.

    * @return TRUE if there is no problem through the method ; FALSE else.
    */
  bool WmExtension::wmBeginUse()
  {
    bool ok=false ;

    if( m_wmavoThread == NULL )
    {
      bool isConnect=false ;
      ok = true ;

      //
      // Initiate the "Wiimote class".

      m_wmavoThread = new WmAvoThread(this) ;

      //
      // Connect all signal between the "Wiimote class" and WmExtension class.

      // connect() must have this for non-primitive type.
      //Q_DECLARE_METATYPE(...) ; // In .h .
      qRegisterMetaType<Vector3d>("Vector3d") ;
      qRegisterMetaType<WmAvoThread::wmDataTransfert>("WmAvoThread::wmDataTransfert") ;
      isConnect = connect(
                          m_wmavoThread, SIGNAL(wmPolled(WmAvoThread::wmDataTransfert)),
                          this, SLOT(wmActions(WmAvoThread::wmDataTransfert))
                          ) ;

      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmavoThread.wmPolled() -> wmextension.wmActions() !!" ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(wmConnected(int)), this, SLOT(wmConnected(int)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmavoThread.wmConnected() -> wmextension.wmConnected() !!" ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(wmDisconnected()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmavoThread.wmDisconnected() -> wmextension.wmDisconnected() !!" ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(finished()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmavoThread.finished() -> wmextension.wmDisconnected() !!" ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_periodicTable, SIGNAL(elementChanged(int)), this, SLOT(setAtomicNumberCurrent(int)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_periodicTable.elementChanged() -> wmextension.setAtomicNumberCurrent() !!" ;
        isConnect = false ;
        ok = false ;
      }

      /*
      isConnect = connect( m_wmavoThread, SIGNAL(terminated()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmavoThread.terminated() -> wmextension.wmDisconnected() !!" ;
        isConnect = false ;
        ok = false ;
      }
      */
    }

    //
    // Begin the Wiimote connection.

    m_wmavoThread->start() ;

    return ok ;
  }


  /**
    * Slot which waits a special Wiimote event :
    * Notify and initiate the WmExtension class that the Wiimote is connected or not.
    * @param nbFound The number of Wiimote connected (the max is 1).
    */
  void WmExtension::wmConnected( int nbFound )
  {
    // Really connected ?
    if( nbFound > 0 )
    {
      m_wmIsConnected = true ;

      ostringstream ossWm ;
      ossWm << "Number of Wiimote connected : " << nbFound ;
      emit message( ossWm.str().c_str() ) ;

      // No paint() available in extension class ...
      qDebug() << ossWm.str().c_str() ;
    }
    else
      qDebug() << "No Wiimote find" ;
  }


  /**
    * Send some Wiimote informations to the WmTool class to display its.
    * @param connect Wiimote connected ?
    * @param nbDots The number of LEDs detected by the Wiimote
    * @param nbSources The number of "final LEDs" really used
    * @param distance The "Wiimote distance"
    */
  void WmExtension::sendWmInfoToWmTool( bool connect, int nbDots, int nbSources, int distance )
  {
    if( m_wmTool != NULL )
    {
      if( connect != m_wmIsAlreadyConnected )
        m_wmIsAlreadyConnected = connect ; // Not use ...

      emit displayedWmInfo( connect, nbDots, nbSources, distance ) ;
    }
    else
    {
      qDebug() << "m_wmTool not initialized." ;
    }
  }


  /**
    * Slot which waits a special Wiimote event :
    * Notify and initiate the WmExtension class that the Wiimote is disconnected.
    */
  void WmExtension::wmDisconnected()
  {
    if( m_wmIsConnected )
    {
      m_wmIsConnected = false ;
      ostringstream ossWm ;
      ossWm << "The Wiimote is disconnected now." ;
      emit message( ossWm.str().c_str() ) ;
      qDebug() << ossWm ;

      emit displayedWmInfo( false, 0, 0, 0 ) ;
    }
    else
      qDebug() << "The Wiimote is not connected, so it can not be disconnected ..." ;
  }

  /**
    * Initiate the pull-down menu before use.
    */
  void WmExtension::initPullDownMenu()
  {
    QAction *action=NULL ;
    m_pullDownMenuActions.clear() ;

    /*
    action = new QAction( this ) ;
    action->setSeparator( true ) ;
    m_pullDownMenuActions.append( action ) ;
    */

    action =  new QAction(this) ;
    action->setText( tr("Connect Wiimote") ) ;
    action->setData( ConnectWm ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Disconnect Wiimote") ) ;
    action->setData( DisconnectWm ) ;
    m_pullDownMenuActions.append( action ) ;

    action = new QAction( this ) ;
    action->setSeparator( true ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm1 + Nc1") ) ;
    action->setData( OpMode1 ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm1 + Nc2") ) ;
    action->setData( OpMode2 ) ;
    m_pullDownMenuActions.append( action ) ;

    action =  new QAction(this) ;
    action->setText( tr("Wm2 + Nc1") ) ;
    action->setData( OpMode3 ) ;
    m_pullDownMenuActions.append( action ) ;
  }


  /**
    * Activate the Wiimote tool menu (of the Wiimote tool plugin), and initiate signal
    * between WmExtension class and WmTool class.
    */
  void WmExtension::initAndActiveForWmToolMenu()
  {
    QString wmPluginName=PLUGIN_WMTOOL_NAME ;
    QString drawPluginName=PLUGIN_DRAWTOOL_NAME ;

    if( m_widget->toolGroup()->activeTool()->name().compare(wmPluginName) != 0 )
    { // Current tool isn't the WmTool.

      ToolGroup *tg=m_widget->toolGroup() ;
      Tool *t=NULL ;
      int nbTools=tg->tools().size() ;

      // 1. The easy way : Do not work ...
      //tg->setActiveTool(wmPluginName) ;

      // 2. The longest way.
      // Search the WmTool & draw Tool.
      for( int i=0 ; i<nbTools ; i++ )
      {
        t = tg->tool(i) ;
        //qDebug() << t->name() ;

        if( m_drawTool==NULL && t->name()==drawPluginName )
        { // Find !
          m_drawTool = t ;
        }

        if( t->name() == wmPluginName )
        { // Find ! Now initiate.

          if( m_wmTool == NULL )
          { // Connect signals.

            m_wmTool = t ;
            initSignalBetweenWmExtWmTool() ;
          }

          // Activate Wmtool.
          // It is realized here because it is safer with some openGL and draw call.
          // In fact, it lets to avoid that another tool class is called to realize its jobs
          // instead of expected jobs by WmTool. (fr:job prevu)
          tg->setActiveTool( m_wmTool ) ;
          m_widget->update() ; // Must be realized to display the change in the Avogadro IHM.

        }
      }
    }
    else
    {
      if( m_wmTool == NULL )
      { // Connect signals.

        m_wmTool = m_widget->toolGroup()->activeTool() ;
        initSignalBetweenWmExtWmTool() ;
      }
    }
  }


  /**
    * Initiate the signals between WmExtension class and WmTool class.
    * @return TRUE if no problem (with connections of the signals mainly) ; FALSE else.
    */
  bool WmExtension::initSignalBetweenWmExtWmTool()
  {
    bool ok=true ;

    if( m_wmTool == NULL )
    {
      qDebug() << "m_wmTool not initiate in WmExtension::initConnectionBetweenWmExtWmTool() !!" ;
      ok = false ;
    }
    else
    {
      qRegisterMetaType<Vector3d>("Vector3d") ;
      bool isConnect=connect( this, SIGNAL(renderedAtomBond( Vector3d, Vector3d, bool, bool, bool)),
                              m_wmTool, SLOT(renderAtomBond( Vector3d, Vector3d, bool, bool, bool)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.renderedAtomBond() -> m_wmTool.renderAtomBond() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(renderedSelectRect(bool,QPoint,QPoint)), m_wmTool, SLOT(setActiveRect(bool,QPoint,QPoint)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.renderedSelectRect() -> m_wmTool.setActiveRect() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(setToolWmExt(Extension*)), m_wmTool, SLOT(setWmExt(Extension*)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.setToolWmExt() -> m_wmTool.setWmExt() !!" ;
        ok = false ;
      }
      else
      {
        emit setToolWmExt(this) ;
      }

      isConnect = connect( this, SIGNAL(initiatedCalculDistDiedre(int)), m_wmTool, SLOT(setCalculDistDiedre(int)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.initiatedCalculDistDiedre() -> m_wmTool.setCalculDistDiedre() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(setCalculDistDiedre(Atom*)),
                           m_wmTool, SLOT(calculDistDiedre(Atom*)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.setCalculDistDiedre() -> m_wmTool.calculDistDiedre() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(displayedMsg(QList<QString>,QPoint)),
                           m_wmTool, SLOT(setDisplayMsg(QList<QString>,QPoint)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.displayedMsg() -> m_wmTool.setDisplay() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(displayedWmInfo(bool, int, int, int)),
                           m_wmTool, SLOT(setWmInfo(bool, int, int, int)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.displayedWmInfo() -> m_wmTool.setWmInfo() !!" ;
        ok = false ;
      }

      isConnect = connect( this, SIGNAL(displayedAtomicNumberCurrent(int)),
                           m_wmTool, SLOT(setAtomicNumberCurent(int)) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmextension.displayedAtomicNumberCurrent() -> m_wmTool.setAtomicNumberCurent() !!" ;
        ok = false ;
      }


      isConnect = connect( m_wmTool, SIGNAL(askDistDiedre()),
                           this, SLOT(receiveRequestToCalculDistance()) ) ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmTool.askDistDiedre() -> m_wmextension.receiveRequestToCalculDistance() !!" ;
        ok = false ;
      }

      isConnect = connect( m_wmTool, SIGNAL(changedWmSensitive(int)),
                           this, SLOT(setWmSensitive(int)) ); ;
      if( !isConnect )
      {
        qDebug() << "Problem connection signal : m_wmTool.changedWmSensitive() -> m_wmextension.setWmSensitive() !!" ;
        ok = false ;
      }

      //isConnect = connect( m_wmTool, SIGNAL(adjustedHydrogen(int)),
      //                     this, SLOT(setAddHydrogen(int)) ); ;
      //if( !isConnect )
      //{
      //  qDebug() << "Problem connection signal : m_wmTool.adjustedHydrogen() -> m_wmextension.setAddHydrogen() !!" ;
      //  ok = false ;
      //}
    }

    return ok ;
  }


  /**
    * Initiate the context menu before use.
    * @return TRUE if all signals are connected (between the menu actions and the associated method) ; FALSE else.
    */
  bool WmExtension::initContextMenu()
  {
    bool isConnect=false, ok=true ;

    //
    // Initiate actions.

    m_cancelAct = new QAction( tr("Close menu"), this ) ;
    m_cancelAct->setStatusTip( tr("Close this context menu") ) ;
    QIcon icon( "/home/mickaelgadroy/Dropbox/Photos/Plan9bunnysmblack.jpg" ) ;
    m_cancelAct->setIcon( icon ) ;
    m_cancelAct->setIconVisibleInMenu(true) ;
    isConnect=connect( m_cancelAct, SIGNAL(triggered()), this, SLOT(closeContextMenu()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_cancelAct.triggered() -> WmExtension.closeContextMenu() !!" ;
      isConnect = false ;
      ok = false ;
    }

    m_periodicTableAct = new QAction( tr("Periodic Table"), this ) ;
    m_periodicTableAct->setStatusTip( tr("Display a periodic table") ) ;
    isConnect=connect( m_periodicTableAct, SIGNAL(triggered()), m_periodicTable, SLOT(show()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_periodicTableAct.triggered() -> m_periodicTable.show() !!" ;
      isConnect = false ;
      ok = false ;
    }

    m_noDistAct = new QAction( tr("Clear measure"), this ) ;
    m_noDistAct->setStatusTip( tr("Clear current display of distance, angle & angle diedre") ) ;
    isConnect=connect( m_noDistAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculNothing()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_noDistAct.triggered() -> WmExtension.askWmToolToCalculNothing() !!" ;
      isConnect = false ;
      ok = false ;
    }

    m_distAct = new QAction( tr("Measure distances"), this ) ;
    m_distAct->setStatusTip( tr("Measure & display the distance between atoms") ) ;
    isConnect=connect( m_distAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculDistance()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_distAct.triggered() -> WmExtension.askWmToolToCalculDistance() !!" ;
      isConnect = false ;
      ok = false ;
    }

    m_angleAct = new QAction( tr("Measure angle between atoms"), this ) ;
    m_angleAct->setStatusTip( tr("Measure & display the angle between atoms") ) ;
    isConnect=connect( m_angleAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculAngle()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_angleAct.triggered() -> WmExtension.askWmToolToCalculAngle() !!" ;
      isConnect = false ;
      ok = false ;
    }

    m_diedreAct = new QAction( tr("Measure dihedral angle between atoms"), this ) ;
    m_diedreAct->setStatusTip( tr("Measure & display dihedral angle between atoms") ) ;
    isConnect=connect( m_diedreAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculDiedre()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_diedreAct.triggered() -> WmExtension.askWmToolToCalculDiedre() !!" ;
      isConnect = false ;
      ok = false ;
    }


    if( WMEX_ADJUST_HYDROGEN )
      m_changeAddHAct = new QAction( tr("No Adjust Hydrogen ..."), this ) ;
    else
      m_changeAddHAct = new QAction( tr("Adjust Hydrogen ..."), this ) ;

    m_changeAddHAct->setStatusTip( tr("Measure & display dihedral angle between atoms") ) ;
    isConnect=connect( m_changeAddHAct, SIGNAL(triggered()), this, SLOT(invertAddHydrogen()) ) ;
    if( !isConnect )
    {
      qDebug() << "Problem connection signal : m_changeAddHAct.m_changeAddHAct() -> WmExtension.invertAddHydrogen() !!" ;
      isConnect = false ;
      ok = false ;
    }

    //
    // Then, initiate the context menu object.

    m_contextMenuMain = new ContextMenu( m_widget, NULL ) ;
    m_contextMenuMain->setTitle( "Wiimote Context Menu" ) ;

    m_contextMenuMain->addAction( m_periodicTableAct ) ;
    m_contextMenuMain->addSeparator() ;

    m_contextMenuMeasure = new ContextMenu( "Measure", m_contextMenuMain, m_contextMenuMain ) ;
    m_contextMenuMeasure->addAction( m_noDistAct ) ;
    m_contextMenuMeasure->addAction( m_distAct ) ;
    m_contextMenuMeasure->addAction( m_angleAct ) ;
    m_contextMenuMeasure->addAction( m_diedreAct ) ;

    m_contextMenuMain->addMenu( m_contextMenuMeasure ) ;

    m_contextMenuMain->addMenu( createMenuSubstituteAtomByFragment() ) ;
    m_contextMenuMain->addSeparator() ;

    m_contextMenuMain->addAction( m_changeAddHAct ) ;
    m_contextMenuMain->addAction( m_cancelAct ) ;

    // Init default actions.
    m_contextMenuMain->setDefaultAction( m_cancelAct ) ;
    m_contextMenuMain->setActiveAction( m_periodicTableAct ) ;

    return ok ;
  }


  /**
    * Create the "Substitute Atom By Fragment" sub-menu for the context menu.
    * It lets to search in an Avogadro repository a lot of fragment files.
    * @return A (ContextMenu*) object which represents the "Substitute Atom By Fragment" sub-menu.
    */
  ContextMenu* WmExtension::createMenuSubstituteAtomByFragment()
  {
    QDir fragRootDir( QCoreApplication::applicationDirPath()+"/../share/avogadro/fragments/" ) ;

    //qDebug() << fragRootDir.absolutePath() ;
    //QStringList filtersName ;
    //filtersName << "*.cpp" << "*.cxx" << "*.cc" ;
    //fragRootDir.setNameFilters( filtersName ) ;

    m_contextMenuFragment = createMenuSABF(m_contextMenuMain, fragRootDir) ;
    m_contextMenuFragment->setTitle( "Substitute atom by fragment" ) ;

    return m_contextMenuFragment ;
  }


  /**
    * Help to create the "Substitute Atom By Fragment" sub-menu for the context menu.
    * This is a recursive method to search in an Avogadro repertory.
    * It is called by WmExtension::createMenuSubstituteAtomByFragment() .
    * @return A (ContextMenu*) object which represents a big part of "Substitute Atom By Fragment" sub-menu.
    * @param parent The (ContextMenu*) parent object.
    * @param dirCur The directory which contains all fragments
    */
  ContextMenu* WmExtension::createMenuSABF( ContextMenu *parent, QDir dirCur )
  {
    bool isConnect=false ;

    ActionModified *aTmp=NULL ;
    QString tmpStr ;
    ContextMenu *cmCur=NULL ; // Current QMenu.
    QDir::Filters filterDir( QDir::Dirs | QDir::NoDotAndDotDot ) ;
    QDir::Filters filterFile( QDir::Files ) ;
    QStringList dirs=dirCur.entryList( filterDir, QDir::Name ) ; // Get directories entries.
    QStringList files=dirCur.entryList( filterFile, QDir::Name ) ; // Get files entries.
    QString absPath=dirCur.absolutePath()+"/" ;

    // Create & append current menu.
    m_famillyFragAct.append( new ContextMenu(dirCur.dirName(), parent, parent) ) ;
    cmCur = m_famillyFragAct.last() ;

    // Get directories.
    foreach( QString d, dirs )
    {
      tmpStr = absPath ;
      tmpStr += d ;
      //qDebug() << tmpStr ;

      cmCur->addMenu( createMenuSABF( cmCur, QDir(tmpStr)) ) ;
    }

    // Get files.
    foreach( QString f, files )
    {
      tmpStr = absPath ;
      tmpStr += f ;
      //qDebug() << tmpStr ;

      aTmp = new ActionModified(f, this) ;
      m_fragAct.append( aTmp ) ;
      aTmp->setStatusTip( tmpStr ) ;

      isConnect = connect( aTmp, SIGNAL(triggeredInfo(QString)),
               this, SLOT(substituteAtomByFrag(QString)) ) ;

      if( !isConnect )
      {
        qDebug() << "Problem connection signal : ActionsModified.triggeredInfo() -> WmExtension.substituteAtomByFrag() !!" ;
      }

      cmCur->addAction( m_fragAct.last() ) ;
    }

    return cmCur ;
  }


  /**
    * Transform a wrapper action to an Avogadro action : move the selected atoms.
    * @param wmavoAction All actions ask by the wrapper
    * @param pos3dCurrent The current position calculate by the Wiimote
    * @param pos3dLast The last position calculate by the Wiimote
    * @param rotAtomDegX The desired X-axis angle
    * @param rotAtomDegY The desired Y-axis angle
    */
  void WmExtension::transformWrapperActionToMoveAtom( int wmavoAction, Vector3d pos3dCurrent, Vector3d pos3dLast, double rotAtomDegX, double rotAtomDegY )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_MOVE) )
    { // Object is in "travel mode", but just in the mode.
      // It is necessary to know what is the movement.

      if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) )
      { // Work when an atom is moving. If no movement, do not pass here.

        //cout << "wmExtension::transformWrapperActionToMoveAtom" << endl ;

        calculateTransformationMatrix( wmavoAction, pos3dCurrent, pos3dLast, m_pointRefBarycenter, rotAtomDegX, rotAtomDegY ) ;

        QList<Primitive*> pList=m_widget->selectedPrimitives().subList(Primitive::AtomType) ;
        moveAtomBegin( wmavoAction, pList, m_vectAtomTranslate, m_transfAtomRotate ) ;

        // Active rumble in the Wiimote only if one atom is selected.
        if( pList.size() == 1 )
        {
          Atom *a=static_cast<Atom*>(pList.at(0)) ;

          if( a != NULL )
            adjustRumble( true, a->pos(), a ) ;
        }
      }
    }
    else
    { // Finish the action.

      if( m_isMoveAtom ) // Caution, this attribut is initialised/used in moveAtom*() methods.
      {
        QList<Primitive*> pList=m_widget->selectedPrimitives().subList(Primitive::AtomType) ;
        moveAtomEnd( pList ) ;

        adjustRumble( false, NULL, NULL ) ;
        m_tmpBarycenter = WmAvo::m_refPoint0 ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : move the mouse cursor.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void WmExtension::transformWrapperActionToMoveMouse( int wmavoAction, QPoint posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CURSOR_MOVE) || WMAVO_IS2(wmavoAction,WMAVO_CREATE) )
    {
      //cout << "wmExtension::transformWrapperActionToMoveMouse" << endl ;
      QCursor::setPos(posCursor) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : select an atom.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position where a "click" is realised
    */
  void WmExtension::transformWrapperActionToSelectAtom( int wmavoAction, QPoint posCursor)
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_SELECT) )
    {
      //cout << "wmExtension::transformWrapperActionToSelectAtom" << endl ;

      if( !WMAVO_IS2(wmavoAction,WMAVO_CURSOR_MOVE) )
      { // Select only 1 object.

        // Just one rumble.
        m_wmavoThread->setWmRumble( true, false, false, 10 ) ;

        QPoint p=m_widget->mapFromGlobal(posCursor) ;
        QList<Primitive*> hitList ;

        Atom* atom=m_widget->computeClickedAtom( p ) ;
        // OR,
        // use the method below :
        //
        //QPoint p=m_widget->mapFromGlobal(posCursor) ;
        //QList<GLHit> hits=m_widget->hits( p.x()-5, p.y()-5, 10, 10 ) ;
        //QList<Primitive*> hitList ;

        //foreach( const GLHit& hit, hits )
        //{
        //  if( hit.type() == Primitive::AtomType )
        //  {
        //    //cout << "atom !!" << endl ;
        //    Atom *atom = m_widget->molecule()->atom( hit.name() ) ;
        //    hitList.append(atom) ;
        //  }
        //}

        if( atom != NULL )
        {
          if( m_isCalculDistDiedre )
          { // Manage the selection of atom. It works with an association of the WmTool class.

            // Put the "calcul distance" mode of the WmExtension class to off.
            m_isCalculDistDiedre = false ;

            // Inform the WmTool class of the selected atom.
            emit setCalculDistDiedre( atom ) ; // To wmTool.

            // Nota Bene : it is the WmTool class which infoms the WmExtension class when
            // it is necessary to select an atom for the "calcul distance" mode.
          }
          else
          {
            hitList.append(atom) ;
            m_widget->toggleSelected(hitList) ;
            //m_widget->setSelected(hitList, true) ;

            // Select H-neighbors.
            if( m_addHydrogens
                && m_widget->isSelected(static_cast<Primitive*>(atom))
                && !atom->isHydrogen() )
            {
              Atom *a=NULL ;
              PrimitiveList pl ;

              foreach( unsigned long ai, atom->neighbors() )
              {
                a = m_widget->molecule()->atomById(ai) ;

                if( a!=NULL && a->isHydrogen() )
                  pl.append( a ) ;
              }

              m_widget->setSelected( pl, true) ;
            }
          }
        }
      }
      else
      { // Multiple selection.

        // For the display of the selection rectangle.
        if( !m_isRenderRect )
        { // Save the 1st point of the rectangle.

          m_isRenderRect = true ;
          m_rectP1 = posCursor ;
        }

        // Save the 2nd point of the rectangle.
        m_rectP2 = posCursor ;

        QPoint p1=m_widget->mapFromGlobal(m_rectP1) ;
        QPoint p2=m_widget->mapFromGlobal(m_rectP2) ;

        // Adjust the 1st point always at bottom/left,
        // the 2nd point always at up/right.
        int x1=( p1.x()<p2.x() ? p1.x() : p2.x() ) ;
        int y1=( p1.y()<p2.y() ? p1.y() : p2.y() ) ;

        int x2=( p1.x()>p2.x() ? p1.x() : p2.x() ) - 1 ;
        int y2=( p1.y()>p2.y() ? p1.y() : p2.y() ) - 1 ;

        // Inform the WmTool class of the 2 points of the selection rectangle for the display.
        emit renderedSelectRect( true, QPoint(x1,y1), QPoint(x2,y2) ) ;
      }
    }
    else
    {
      if( m_isRenderRect )
      {
        //
        // 1. Realize the selection

        QList<GLHit> hitList ;
        PrimitiveList pList ;
        Primitive *p=NULL ;
        Atom *a=NULL ;

        QPoint p1=m_widget->mapFromGlobal(m_rectP1) ;
        QPoint p2=m_widget->mapFromGlobal(m_rectP2) ;

        int x1=( p1.x()<p2.x() ? p1.x() : p2.x() ) ;
        int y1=( p1.y()<p2.y() ? p1.y() : p2.y() ) ;

        int x2=( p1.x()>p2.x() ? p1.x() : p2.x() ) ; // - 1 ;
        int y2=( p1.y()>p2.y() ? p1.y() : p2.y() ) ; // - 1 ;


        // Perform an OpenGL selection and retrieve the list of hits.
        hitList = m_widget->hits( x1, y1, x2-x1, y2-y1 ) ;

        if( hitList.empty() )
        {
          m_widget->clearSelected() ;
        }
        else
        {
          // Build a primitiveList for toggleSelected() method.
          foreach( const GLHit& hit, hitList )
          {
            if( hit.type() == Primitive::AtomType )
            {
              a = m_widget->molecule()->atom( hit.name() ) ;
              p = static_cast<Primitive *>( a ) ;

              if( p != NULL )
                pList.append( p ) ;
              else
                qDebug() << "Bug in WmExtension::transformWrapperActionToSelectAtom : a NULL-object not expected." ;
            }
          }

          // Toggle the selection.
          m_widget->toggleSelected( pList ) ; // or setSelected()
        }

        //
        // 2. Finish the action.

        m_isRenderRect = false ;
        m_rectP1 = QPoint( 0, 0 ) ;
        m_rectP2 = QPoint( 0, 0 ) ;

        p1 = m_widget->mapFromGlobal(m_rectP1) ;
        p2 = m_widget->mapFromGlobal(m_rectP2) ;
        emit renderedSelectRect( false, p1, p2) ;
      }
    }
  }




  /**
    * Transform a wrapper action to an Avogadro action : create atom(s) and bond(s).
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position of the cursor
    * @param pointRef The position of the reference point.
    */
  void WmExtension::transformWrapperActionToCreateAtomBond( int wmavoAction, QPoint posCursor, Vector3d pointRef )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CREATE) || m_isAtomDraw )
      // for m_isAtomDraw : Necessary for the action of "isCreateAtom".
    {
      //qDebug() << "WmExtension::transformWrapperActionToCreateAtomBond" ;

      Molecule *molecule=m_widget->molecule() ;

      // molecule->lock()->tryLockForWrite() :
      // GLWidget::render(): Could not get read lock on molecule.
      // Lock the write on the molecule => so it locks the render too ...
      // This means the possibility that the Avogadro software can be really multi-thread
      // (or in anticipation).
      // The lock justifies it by the array used to store a new atom, and with the search
      // of a new id for this new atom y tutti quanti ...
      //if( molecule->lock()->tryLockForWrite() )
      //{

        // Add an atom.

        QPoint p=m_widget->mapFromGlobal(posCursor) ;

        if( !m_isAtomDraw )
        { // The 1st action : a request of creation

          m_lastCursor = p ;
          Primitive* prim=m_widget->computeClickedPrimitive( p ) ;
          m_timeFirst = m_time.elapsed() ;

          if( prim == NULL )
          {
            m_isAtomDraw = true ;
            m_drawBeginAtom = true ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            m_beginPosDraw = m_widget->camera()->unProject( p, pointRef ) ;
            m_curPosDraw = m_beginPosDraw ;
          }
          else if( prim->type() == Primitive::AtomType )
          {
            m_isAtomDraw = true ;
            m_drawBeginAtom = false ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            m_beginAtomDraw = static_cast<Atom*>(prim) ;
            m_beginPosDraw = *(m_beginAtomDraw->pos()) ;
            m_curPosDraw = m_beginPosDraw ;
          }
          else if( prim->type() == Primitive::BondType )
          {
            m_isBondOrder = true ;
            m_isAtomDraw = true ;
            m_drawBeginAtom = false ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            m_bondDraw = static_cast<Bond*>(prim) ;
          }
          //else
            // Nothing to do.

          // Request the temporary display of the 1st atom(by the WmTool class).
          if( m_isAtomDraw || m_isBondOrder )
            emit renderedAtomBond( m_beginPosDraw, m_curPosDraw, m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ) ;
        }
        else if( m_isAtomDraw && WMAVO_IS2(wmavoAction,WMAVO_CREATE) && !m_isBondOrder )
        { // The 2nd action.
          // That means, the 1st atom has been "selected/created" and
          // the mouse is moving to create/select an 2nd atom with (new) bond.


          // Timeout before to create a 2nd atom.
          if( !m_canDrawOther )
          {
            m_timeSecond = m_time.elapsed() ;

            if( (m_timeSecond-m_timeFirst) > 1000 )
              m_canDrawOther = true ;
          }

          if( m_canDrawOther )
          {
            m_curPosDraw = m_widget->camera()->unProject(p, pointRef) ;
            adjustRumble( true, &m_curPosDraw, NULL ) ;

            // Methode0 : RAPIDE !!
            Atom* a=NULL ;

            // Methode1 : LENT !!
            //a = m_widget->computeClickedAtom( p ) ;

            // Methode2 : LENT !!
            /*
            QList<GLHit> hits=m_widget->hits( p.x()-5, p.y()-5, 10, 10 ) ;

            foreach( const GLHit& hit, hits )
            {
              if( hit.type() == Primitive::AtomType )
              { // Le 1er element est le plus proche.

                a = m_widget->molecule()->atom( hit.name() ) ;
                break ;
              }
            }*/

            if( a == NULL )
            {
              double var1=m_beginPosDraw[0]-m_curPosDraw[0] ;
              double var2=m_beginPosDraw[1]-m_curPosDraw[1] ;
              double var3=m_beginPosDraw[2]-m_curPosDraw[2] ;
              double distVect=sqrt( var1*var1 + var2*var2 + var3*var3 ) ;

              // Draw a 2nd atom only if the distance is bigger than ...
              if( distVect > WMEX_DISTBEFORE_CREATE )
              {
                //cout << "Display current atom & bond" << endl ;
                m_drawCurrentAtom = true ;

                if( m_beginAtomDraw!=NULL && m_beginAtomDraw->isHydrogen()
                    && m_beginAtomDraw->bonds().count()>0 )
                  m_drawBond = false ;
                else
                  m_drawBond = true ;

                m_curAtomDraw = a ;
              }
              else
              {
                m_drawCurrentAtom = false ;
                m_drawBond = false ;

                m_curAtomDraw = NULL ;
              }
            }
            else if( *(a->pos()) == m_beginPosDraw )
            {
              //cout << "Display nothing" << endl ;

              m_drawCurrentAtom = false ;
              m_drawBond = false ;

              m_curAtomDraw = NULL ;
            }
            else //if( a )
            {
              //cout << "Display Bond" << endl ;

              m_drawCurrentAtom = false ;
              m_drawBond = true ;

              // Limit the number of bond if Hydrogen Atom.
              if( a->isHydrogen() && a->bonds().count() > 0 )
                m_drawBond = false ;

              if( m_drawBond && m_beginAtomDraw!=NULL
                  && m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count() > 0 )
                m_drawBond = false ;

              m_curAtomDraw = a ;
              m_curPosDraw = *(a->pos()) ;
            }


            // Request the temporary display of the atoms and bond (by the WmTool class).
            emit renderedAtomBond( m_beginPosDraw, m_curPosDraw, m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ) ;
          }
        }
        else if( m_isAtomDraw && !WMAVO_IS2(wmavoAction,WMAVO_CREATE) )
        { // The 3rd and last action : creation
          // - either adjust number of bond ;
          // - or create atoms(s) and bond.

          bool addSmth=false ;
          //QUndoCommand *undo=NULL ;

          if( m_isBondOrder )
          {
            //int oldBondOrder = m_bondDraw->order() ;

            // 1.
            if( m_addHydrogens )
              changeOrderBondBy1WithHydrogen( molecule, m_bondDraw ) ;
            else
              changeOrderBondBy1( molecule, m_bondDraw ) ;


            // 2.
            //undo = new ChangeBondOrderDrawCommand( molecule, m_bondDraw, oldBondOrder, m_addHydrogens ) ;
            //m_widget->undoStack()->push( undo ) ;

          }
          else //if( m_isAtomDraw && !m_isBondOrder )
          {
            //cout << "End of the creation of atom/bond" << endl ;
            Atom* a=NULL ;
            Vector3d addAtomPos ;

            adjustRumble( false, NULL, NULL ) ;

            if( m_beginAtomDraw == NULL )
            {
              addSmth = true ;
              m_hasAddedBeginAtom = true ;
            }

            // Timeout before to create a 2nd atom.
            if( !m_canDrawOther )
            {
              m_timeSecond = m_time.elapsed() ;

              if( (m_timeSecond-m_timeFirst) > 1000 )
                m_canDrawOther = true ;
            }

            // Add 2nd atom & bond.
            if( m_canDrawOther )
            {
              a = m_widget->computeClickedAtom( p ) ;

              if( a == NULL )
              { // Create atome/bond.

                double var1=m_beginPosDraw[0]-m_curPosDraw[0] ;
                double var2=m_beginPosDraw[1]-m_curPosDraw[1] ;
                double var3=m_beginPosDraw[2]-m_curPosDraw[2] ;
                double distVect=sqrt( var1*var1 + var2*var2 + var3*var3 ) ;

                // Draw a 2nd atom only if the distance is bigger than ...
                if( distVect > 0.6 )
                {
                  addAtomPos = m_widget->camera()->unProject( p, pointRef ) ;

                  addSmth = true ;
                  m_hasAddedCurAtom = true ;

                  if( m_drawBond
                      /* !(m_curAtomDraw->isHydrogen() && m_curAtomDraw->bonds().count()>0)
                         && !(m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count()>0)
                      */
                    )
                    m_hasAddedBond = true ;
                }
              }
              else
              { // Create bond.

                if( *(a->pos()) != m_beginPosDraw
                    && m_widget->molecule()->bond(a,m_beginAtomDraw) == NULL
                    && m_drawBond
                       /* !(a->isHydrogen() && a->bonds().count()>0)
                          && !(m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count()>0)
                       */
                  )
                {
                  m_curAtomDraw = a ;
                  addSmth = true ;
                  m_hasAddedBond = true ;
                }
              }
            }


            if( molecule->lock()->tryLockForWrite() )
            {

              // Create just the 1st atom.
              if( m_hasAddedBeginAtom && !m_hasAddedCurAtom && !m_hasAddedBond )
              {
                if( m_addHydrogens )
                  m_beginAtomDraw = addAtomWithHydrogen( molecule, &m_beginPosDraw, m_atomicNumberCurrent ) ;
                else
                  m_beginAtomDraw = addAtom( molecule, &m_beginPosDraw, m_atomicNumberCurrent ) ;
              }

              // Create just the 2nd atom.
              if( !m_hasAddedBeginAtom && m_hasAddedCurAtom && !m_hasAddedBond )
              {
                if( m_addHydrogens )
                  m_curAtomDraw = addAtomWithHydrogen( molecule, &addAtomPos, m_atomicNumberCurrent ) ;
                else
                  m_curAtomDraw = addAtom( molecule, &addAtomPos, m_atomicNumberCurrent ) ;
              }

              // Create just the bond.
              if( !m_hasAddedBeginAtom && !m_hasAddedCurAtom && m_hasAddedBond )
              {
                if( m_addHydrogens )
                  m_bondDraw = addBondWithHydrogen( molecule, m_beginAtomDraw, a, 1 ) ;
                else
                  m_bondDraw = addBond( molecule, m_beginAtomDraw, a, 1 ) ;
              }

              // Create the 2nd atom bonded at 1st.
              if( !m_hasAddedBeginAtom && m_hasAddedCurAtom && m_hasAddedBond )
              {
                if( m_addHydrogens )
                  m_curAtomDraw = addAtomWithHydrogen( molecule, &addAtomPos, m_atomicNumberCurrent,
                                                       m_beginAtomDraw, 1 ) ;
                else
                  m_curAtomDraw = addAtom( molecule, &addAtomPos, m_atomicNumberCurrent,
                                           m_beginAtomDraw, 1 ) ;
              }


              // Create the 1st atom bonded at 2nd.
              if( m_hasAddedBeginAtom && !m_hasAddedCurAtom && m_hasAddedBond )
              {
                if( m_addHydrogens )
                  m_beginAtomDraw = addAtomWithHydrogen( molecule, &m_beginPosDraw, m_atomicNumberCurrent,
                                                        m_curAtomDraw, 1 ) ;
                else
                  m_beginAtomDraw = addAtom( molecule, &m_beginPosDraw, m_atomicNumberCurrent,
                                            m_curAtomDraw, 1 ) ;
              }

              // Create 2 atoms.
              if( m_hasAddedBeginAtom && m_hasAddedCurAtom )
              {
                int order=0 ;
                PrimitiveList *pl=NULL ;

                if( m_hasAddedBond )
                  order = 1 ;

                if( m_addHydrogens )
                  pl = addAtomsWithHydrogen( molecule, &m_beginPosDraw, m_atomicNumberCurrent,
                                             &addAtomPos, m_atomicNumberCurrent, order ) ;
                else
                  pl = addAtoms( molecule, &m_beginPosDraw, m_atomicNumberCurrent,
                                 &addAtomPos, m_atomicNumberCurrent, order ) ;

                if( pl!=NULL && pl->size()>=2 )
                {
                  PrimitiveList::const_iterator ipl=pl->begin() ;
                  m_beginAtomDraw = static_cast<Atom*>(*ipl) ;
                  ipl++ ;
                  m_curAtomDraw = static_cast<Atom*>(*ipl) ;
                }

                if( pl != NULL )
                  delete pl ;
              }
            }

            molecule->lock()->unlock() ;
            m_widget->molecule()->update() ;
          }

          if( addSmth )
            m_wmavoThread->setWmRumble( true, false, false, 10 ) ;


          //addAdjustHydrogenRedoUndo( molecule ) ;


          // Initialization before next use.
          m_isBondOrder=false ; m_isAtomDraw=false ;
          m_drawBeginAtom=false ; m_drawCurrentAtom=false ; m_drawBond=false ;
          m_hasAddedBeginAtom=false ; m_hasAddedCurAtom=false ; m_hasAddedBond=false ;
          m_beginAtomDraw=NULL ; m_curAtomDraw=NULL ; m_bondDraw=NULL ;

          m_timeFirst=0 ; m_timeSecond=0 ;
          m_canDrawOther = false ;

          // "Push" all modifications & redraw of the molecule.
          emit renderedAtomBond( Vector3d(0,0,0), Vector3d(0,0,0), false, false, false ) ;
        }
      //}
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : delete all atom.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WmExtension::transformWrapperActionToDeleteAllAtomBond( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETEALL) )
      deleteAllElement( m_widget->molecule() ) ;
  }


  /**
    * Transform a wrapper action to an Avogadro action : delete atom(s).
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position of the cursor
    */
  void WmExtension::transformWrapperActionToRemoveAtomBond( int wmavoAction, QPoint posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETE) )
    {
      //qDebug() << "WmExtension::transformWrapperActionToRemoveAtomBond" ;

      if( m_widget->molecule()->lock()->tryLockForWrite() )
      {
        Molecule *molecule=m_widget->molecule() ;
        QPoint p=m_widget->mapFromGlobal(posCursor) ;
        Primitive* prim=m_widget->computeClickedPrimitive( p ) ;
        PrimitiveList pl=m_widget->selectedPrimitives() ;

        if( prim == NULL )
        { // Remove the selected atoms.

          if( m_addHydrogens )
            removeAtomsWithHydrogen( molecule, &pl ) ;
          else
            removeAtoms( molecule, &pl ) ;

          // 2. with undo/redo, not adjust hydrogen ...
          //deleteSelectedElementUndoRedo( molecule ) ;
        }
        else
        {

          if( prim->type() == Primitive::AtomType )
          { // Remove atom.

            Atom *atom = static_cast<Atom*>(prim) ;

            if( m_addHydrogens )
              removeAtomWithHydrogen( molecule, atom ) ;
            else
              removeAtom( molecule, atom ) ;
          }

          if( prim->type() == Primitive::BondType )
          { // Remove bond.

            Bond *bond = static_cast<Bond*>(prim) ;

            if( m_addHydrogens )
              removeBondWithHydrogen( molecule, bond ) ;
            else
              removeBond( molecule, bond ) ;

            // 2.
            //deleteBondWithUndoRedo( molecule, bond ) ;
          }
        }

        m_widget->molecule()->lock()->unlock() ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : rotate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param rotCamAxeXDeg The desired angle on the X-axis of the screen
    * @param rotCamAxeYDeg The desired angle on the Y-axis of the screen
    */
  void WmExtension::transformWrapperActionToRotateCamera( int wmavoAction, Vector3d pointRef, double rotCamAxeXDeg, double rotCamAxeYDeg )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE) )
    {
      //qDebug() << "WmExtension::transformWrapperActionToRotateCamera" ;

      //if( ((rotCamAxeXDeg==90.0 || rotCamAxeXDeg==-90.0) && rotCamAxeYDeg==0.0 )
      //    || ((rotCamAxeYDeg==90.0 || rotCamAxeYDeg==-90.0) && rotCamAxeXDeg==0.0)
      //  )
      if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE_BYWM) )
      { // If use the cross of the Wiimote.

        // Use this method to get the wanted angle of rotation.
        // Value in (radian / Avogadro::ROTATION_SPEED) == the desired angle.
        double rotCamAxeXRad = (rotCamAxeXDeg*WmAvo::m_PI180) / Avogadro::ROTATION_SPEED ;
        double rotCamAxeYRad = (rotCamAxeYDeg*WmAvo::m_PI180) / Avogadro::ROTATION_SPEED ;

        Navigate::rotate( m_widget, pointRef, rotCamAxeXRad, rotCamAxeYRad ) ;
      }
      else if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE_BYNC) )
      { // Turn in a direction.
        // Do not search an unit speed or other, just a direction.

        Navigate::rotate( m_widget, pointRef, rotCamAxeXDeg, rotCamAxeYDeg ) ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : translate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param distCamXTranslate Desired distance on the X-axis of the screen
    * @param distCamYTranslate Desired distance on the Y-axis of the screen
    */
  void WmExtension::transformWrapperActionToTranslateCamera( int wmavoAction, Vector3d pointRef, double distCamXTranslate, double distCamYTranslate )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_TRANSLATE) )
    {
      //qDebug() << "WmExtension::transformWrapperActionToTranslateCamera" ;
      Navigate::translate( m_widget, pointRef, distCamXTranslate, distCamYTranslate ) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : zoom the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param distCamZoom Desired distance for the zoom on the Z-axis of the screen
    */
  void WmExtension::transformWrapperActionToZoomCamera( int wmavoAction, Vector3d pointRef, double distCamZoom )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ZOOM) )
    {
      //qDebug() << "WmExtension::transformWrapperActionToZoomCamera" ;
      Navigate::zoom( m_widget, pointRef, distCamZoom ) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : initiate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point.
    */
  void WmExtension::transformWrapperActionToInitiateCamera( int wmavoAction, Vector3d pointRef )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_INITIAT) )
    {
      //qDebug() << "WmExtension::transformWrapperActionToInitiateCamera" ;

      #ifdef DEL
      // 1
      //m_widget->camera()->setModelview( m_cameraInitialViewPoint ) ;
      if( !(
            ((rotCamAxeXDeg==90.0 || rotCamAxeXDeg==-90.0) && rotCamAxeYDeg==0.0)
            || ((rotCamAxeYDeg==90.0 || rotCamAxeYDeg==-90.0) && rotCamAxeXDeg==0.0)
           )
        )
      { // If not use the cross of the Wiimote.

      // 2
      //qDebug() << "pointRefRot:" << pointRefRot[0] << pointRefRot[1] << pointRefRot[2] ;
      //Navigate::translate( m_widget, pointRefRot, -pointRefRot[0], -pointRefRot[1] ) ;

      // 3
      Vector3d barycenterScreen=m_widget->camera()->project(pointRefRot) ;
      QPoint barycenterScreen2(barycenterScreen[0], barycenterScreen[1]) ;
      //qDebug() << "pointRefRot:" << barycenterScreen[0] << barycenterScreen[1] << barycenterScreen[2] ;
      //Navigate::translate( m_widget, pointRefRot, -barycentreEcran[0], -barycentreEcran[1] ) ;

      // 4
      //Navigate::zoom( m_widget, pointRefRot, m_widget->cam_beginPosDrawmera()->distance(pointRefRot)/*+10*/ ) ;
      //qDebug() << "   distance:" << m_widget->camera()->distance(pointRefRot) ;

      // 5
      Vector3d transformedGoal = m_widget->camera()->modelview() * pointRefRot ;
      double distance=m_widget->camera()->distance(pointRefRot) ;
      double distanceToGoal = transformedGoal.norm() ;

      //qDebug() << " distance:" << distance ;
      //qDebug() << " distanceToGoal:" << distanceToGoal ;

      /*
      double distanceToGoal = transformedGoal.norm();m_beginPosDraw
      double t = ZOOM_SPEED * delta;
      const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
      double u = minDistanceToGoal / distanceToGoal - 1.0;
      if( t < u )
      {
        t = u;
        Navigate::rotate( m_widget, pointRefRot, rotCamAxeXDeg, rotCamAxeYDeg ) ;
               }
      widget->camera()->modelview().pretranslate(transformedGoal * t);
      */

      // 6
      //m_widget->camera()->modelview().pretranslate(-transformedGoal /*+ (camBackTransformedZAxis*-40)*/ ) ;
      //m_widget->camera()->modelview().translate(-transformedGoal /*+ (camBackTransformedZAxis*-10)*/ ) ;

      // 7
      //Vector3d camBackTransformedZAxis=m_widget->camera()->transformedZAxis() ;
      //m_widget->camera()->modelview().translate( camBackTransformedZAxis*-10.0 ) ;


      distance=m_widget->camera()->distance(pointRefRot) ;
      distanceToGoal = transformedGoal.norm();

      //qDebug() << " distance:" << distance ;
      //qDebug() << " distanceToGoal:" << distanceToGoal ;
      #endif



      //Vector3d barycenterScreen=m_widget->camera()->project(pointRefRot) ;
      //QPoint barycenterScreen2(barycenterScreen[0], barycenterScreen[1]) ;
      //qDebug() << "pointRefRot:" << barycenterScreen[0] << barycenterScreen[1] << barycenterScreen[2] ;

      //Transform3d cam=m_widget->camera()->modelview() ;

      /* OK
      Vector3d right(cam(0,0), cam(1,0), cam(2,0)) ;
      Vector3d up(cam(0,1), cam(1,1), cam(2,1)) ;
      Vector3d dir(cam(0,2), cam(1,2), cam(2,2)) ;
      Vector3d pos(cam(0,3), cam(1,3), cam(2,3)) ;

      cout << "right:" << right << endl ;
      cout << "dir:" << dir << endl  ;
      cout << "up:" << up << endl  ;
      cout << "pos:" << pos << endl  ;
      */

      /* OK
      cam(0,3) = 0 ;
      cam(1,3) = 0 ;
      cam(2,3) = -20 ;
      */

      /* Oui, et non, apres quelques rotations de camera, le barycentre n'est plus centre.
      qDebug() << "pointRefRot:" << pointRefRot[0] << pointRefRot[1] ;
      cam(0,3) = -pointRefRot[0] ;
      cam(1,3) = -pointRefRot[1] ;
      cam(2,3) = -25 ;

      m_widget->camera()->setModelview(cam) ;
      */


      Vector3d barycenterScreen=m_widget->camera()->project( pointRef ) ;
      QPoint barycenterScreen2(barycenterScreen[0], barycenterScreen[1]) ;

      GLint params[4] ;

      // Do not work (with .h and compilation flag, the final error is : ~"impossible to use without a first call of glinit"~).
      //int screen_pos_x = glutGet((GLenum)GLUT_WINDOW_X);
      //int screen_pos_y = glutGet((GLenum)GLUT_WINDOW_Y);
      //qDebug() << "  :" << screen_pos_x << screen_pos_y ;

      glGetIntegerv( GL_VIEWPORT, params ) ;

      GLenum errCode ;
      const GLubyte *errString ;
      if( (errCode=glGetError()) != GL_NO_ERROR )
      {
        errString = gluErrorString( errCode ) ;
        fprintf (stderr, "OpenGL Error: %s\n", errString);
      }

      GLdouble x=params[0] ;
      GLdouble y=params[1] ;
      GLdouble width=params[2] ;
      GLdouble height=params[3] ;

      //qDebug() << " :" << x << y << width << height ;

      //m_wmavoThread->setWmSizeWidget( x, y, width, height ) ;

      QPoint widgetCenter( (x+width)/2, (y+height)/2 ) ;

      Navigate::translate( m_widget, pointRef, barycenterScreen2, widgetCenter ) ;


      Transform3d cam=m_widget->camera()->modelview() ;
      cam(2,3) = -25 ;
      m_widget->camera()->setModelview(cam) ;

    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : close the periodic table.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void WmExtension::transformWrapperActionToClosePeriodicTable( int &wmavoAction, QPoint posCursor )
  {
    // Close periodic table when visible.
    // CAUTION !!
    // All features use menu must be before the mouse actions of context menu.
    // Because, the mouse actions of context menu active the opening of feature,
    // so, if we ask isVisible & OK_MENU, it opens feature with OK_MENU, then this
    // feature takes for it too.
    if( m_periodicTable->isVisible() && WMAVO_IS2(wmavoAction, WMAVO_MENU_OK) )
    {
      m_periodicTable->setFocus() ;
      //m_periodicTable->grabMouse() ; // Surtout pas !

      // QEvent::MouseButtonPress, QEvent::MouseButtonRelease

      // Select an atomic number (emit a signal).
      QMouseEvent me(QEvent::MouseButtonPress, m_periodicTable->mapFromGlobal(posCursor), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
      QApplication::sendEvent( m_periodicTable->viewport(), &me ) ;

      // Close the periodic table.
      QMouseEvent me2(QEvent::MouseButtonDblClick, m_periodicTable->mapFromGlobal(posCursor), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
      QApplication::sendEvent( m_periodicTable->viewport(), &me2 ) ;

      m_wmavoThread->setWmMenuMode( false ) ;
      WMAVO_SETOFF2( wmavoAction, WMAVO_MENU_ACTIVE ) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : show the context menu.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void WmExtension::transformWrapperActionToShowContextMenu( int &wmavoAction, QPoint posCursor )
  {

    // This must be before context menu actions (explain in the method).
    transformWrapperActionToClosePeriodicTable( wmavoAction, posCursor ) ;


    // Menu activation.
    if( WMAVO_IS2(wmavoAction, WMAVO_MENU_ACTIVE) )
    {
      //QContextMenuEvent cme( QContextMenuEvent::Mouse, QPoint(5,5) ) ;
      //QApplication::sendEvent( m_widget->current(), &cme ) ;

      //cout << "0menu active:" << m_menuActive << endl ;

      if( !m_menuActive && !m_periodicTable->isVisible() )
      {
        //m_periodicTable->show() ;

        m_menuActive = true ;
        m_contextMenuCurrent = m_contextMenuMain ;
        m_contextMenuMain->setActiveAction( m_periodicTableAct ) ;

        // Method 1
        //m_contextMenuMain->show() ; où est le setPos !!?

        // Method 2, soit Appel non bloquant.
        m_contextMenuCurrent->popup( posCursor ) ;

        // Method 3, soit appel bloquant
        //m_contextMenuMain->exec( posCursor ) ;
        // => return QAction* : either realized QAction, either NULL=echap

        // Here, do not try to realize action according to the return
        // because it realizes by predefined signals.
      }
      else
      {
        // If the context menu disappears.
        if( !m_contextMenuCurrent->isVisible() && !m_periodicTable->isVisible() )
        {
          /*
          if( m_contextMenuCurrent->getMenuParent() == NULL )
            // The sub-menu can reappear on the menu ...
            m_contextMenuCurrent->popup( QPoint(posCursor.x()-30, posCursor.y()-30) ) ;
          else
          */
            m_contextMenuCurrent->popup( posCursor ) ;
        }
      }

      //cout << "  menu active:" << m_menuActive << endl ;


      if( WMAVO_IS2(wmavoAction,WMAVO_MENU_OK) )
      {
        //cout << "  menu OK" << endl ;
        //QAction *actionMenu=m_contextMenuMain->actionAt( m_contextMenuMain->mapFromGlobal(posCursor) ) ;
        QAction *actionMenu=m_contextMenuCurrent->activeAction() ;

        if( actionMenu != NULL )
        {
          // The test is here to limit the time between the work realized by
          // the action called by trigger(), and limit the risk that wmavo returns
          // a signal to execut another instance of this function with bad parameters ...

          if( actionMenu->text().compare(tr("Periodic Table")) != 0 )
          {
            //qDebug() << "FIN" ;
            m_wmavoThread->setWmMenuMode( false ) ;
            WMAVO_SETOFF2( wmavoAction, WMAVO_MENU_ACTIVE ) ;
          }

          actionMenu->trigger() ;

          m_menuActive = false ;
          //m_contextMenuCurrent->close() ;
          m_contextMenuMain->close() ;

        }
      }

      //QWidget *widgetTmp=m_contextMenuMain->focusWidget() ;

      //if( widgetTmp != NULL )
      //{
        if( WMAVO_IS2(wmavoAction, WMAVO_MENU_DOWN) )
        {
          QKeyEvent ke(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier) ;
          QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;
        }

        if( WMAVO_IS2(wmavoAction, WMAVO_MENU_UP) )
        {
          QKeyEvent ke(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier) ;
          QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;
        }

        if( WMAVO_IS2(wmavoAction, WMAVO_MENU_RIGHT) )
        {
          QKeyEvent ke(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier) ;
          QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;

          if( m_contextMenuCurrent->activeAction() != NULL )
          {
            ContextMenu *cm=dynamic_cast<ContextMenu*>(m_contextMenuCurrent->activeAction()->menu()) ;

            if( cm!=NULL && cm!=m_contextMenuCurrent )
              m_contextMenuCurrent = cm ;
          }
        }

        if( WMAVO_IS2(wmavoAction, WMAVO_MENU_LEFT) )
        {
          QKeyEvent ke(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier) ;
          QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;

          if( m_contextMenuCurrent->getMenuParent() != NULL )
          {
            m_contextMenuCurrent->close() ;
            m_contextMenuCurrent = m_contextMenuCurrent->getMenuParent() ;
          }
        }
      //}
    }
    else
    {
      // Desactivate all context menu & Co.
      m_contextMenuMain->close() ;
    }
  }


  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for the Avogadro delete actions.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WmExtension::updateForAvoActions1( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETE)
        || WMAVO_IS2(wmavoAction,WMAVO_DELETEALL)
        /*|| WMAVO_IS2(wmavoAction,WMAVO_CREATE)*/
        // Put in the transformWrapperActionToCreateAtomBond() method to gain update.
      )
    {
      //qDebug() << "WmExtension::updateForAvoActions1" ;

      // Update
      // If we have done stuff then trigger a redraw of the molecule
      m_widget->molecule()->update() ;

      /*
      // Not resolve an update problem ...
      m_widget->molecule()->update() ;
      m_widget->update() ; // update( &Region ), update( int, int, int, int ) ...
      m_widget->updateGeometry() ;
      m_widget->updateGL() ;
      m_widget->updateOverlayGL() ;
      m_widget->updatesEnabled() ;

      //m_widget->molecule()->updateAtom() ;
      m_widget->molecule()->updateMolecule() ;
      m_widget->molecule()->calculateGroupIndices() ;
      */
    }
  }


  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for a lot of Avogadro actions.
    * This is a special update, because it simulates a mouse click to realize update.
    * In fact, some optimization are available only when some Avogadro class realize
    * update.
    * <br/>To activate the quick render (the previous optimization) , it is necessary to simulate a mouse click.
    * Explanation, the quick render is activated when :
    * - Check the quick render option
    *      Set (allowQuickRender) attribut to enable. Now Avogadro MAY accept
    *      quick render.
    * - While a mouse movement, if the mouse is down
    *      The call of GLWidget::mouseMoveEvent(), and only this method sets
    *      the (quickRender) attribut at true.
    *
    * @param wmavoAction All actions ask by the wrapper
    */
  void WmExtension::updateForAvoActions2( int wmavoAction )
  {
    if( (WMAVO_IS2(wmavoAction, WMAVO_MENU_ACTIVE) ? 0 // To update wmInfo in wmTool class
                                                   : 1 ) // To avoid a bug with periodic table.
        || WMAVO_IS2(wmavoAction, WMAVO_SELECT)
        || WMAVO_IS2(wmavoAction,WMAVO_CREATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_ZOOM)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_TRANSLATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_INITIAT)
      )
    {

      //qDebug() << "WmExtension::updateForAvoActions2" ;

      // 1. No quick render.
      //m_widget->update() ;

      // 2. No compile : mouseMove is protected.
      // Call directly GLWidget->mouseMove signal.
      //emit m_widget->mouseMove(&me) ;

      // 3. Call directly Tool->mouseMouseEvent. No quick render.
      //m_widget->tool()->mouseMoveEvent( m_widget, &me) ;
      //m_widget->tool()->mouseMoveEvent( m_widget->current(), &me) ;

      // 4. Try Fake mouse event. WORKS !!!
      if( !m_testEventPress )
      {
        m_testEventPress = true ;
        QApplication::sendEvent( m_widget->m_current, m_me1 ) ;
      }

      QApplication::sendEvent( m_widget->m_current, m_me2 ) ;

      /*
      // Installer un truc en plus.
      // 5. Try Fake mouse event.
      QTestEventList events;
      events.addMouseMove(p,1);
      events.simulate(m_widget);
      */
    }
    else
    {
      // 4. Finish fake mouse event.
      if( m_testEventPress )
      {
        m_testEventPress = false ;
        QApplication::sendEvent( m_widget->m_current, m_me3 ) ;
      }
    }
  }


  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for the Avogadro move actions.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WmExtension::updateForAvoActions3( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_MOVE) )
    { // Object is in "travel mode", but just in the mode.
      // It is necessary to know what is the movement.

      if( (WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE))
          && m_widget->selectedPrimitives().size()>0
        )
      {
        //qDebug() << "WmExtension::updateForAvoActions3" ;

        // 1. No Quick Render.
        //m_widget->molecule()->update() ; // Update & Redraw (without quick Render)

        // 2. Quick Render seems activated, but it lags ...
        m_widget->molecule()->updateMolecule() ; // Update & Redraw.
      }
    }
  }


  /**
    * Move atoms. This is the 1st action to move atoms. Then, use moveAtomEnd().
    * It is necesary to calculate the barycenter with less performance penality.
    * @param wmavoAction All actions ask by the wrapper
    * @param atomList The selected atoms which move
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void WmExtension::moveAtomBegin( int wmavoAction, QList<Atom*> atomList, Vector3d vectAtomTranslate, Transform3d transfAtomRotate  )
  {
    if( atomList.size()>0 )
    {
      foreach( Atom *atom, atomList )
      {
        if( atom!=NULL && atom->type()==Primitive::AtomType )
        {
          if( !m_isMoveAtom )
          { // Calcul the new barycenter foreach atom.
            // It must bo done once for all atoms.
            //m_wmavoThread->setWmAtomPos( *atom->pos(), false, true ) ;
            updateBarycenter( *(atom->pos()), false ) ;
          }

          if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) )
            atom->setPos( *(atom->pos()) + vectAtomTranslate ) ;

          if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) )
            atom->setPos( transfAtomRotate * *(atom->pos()) ) ;
        }
      }

      if( !m_isMoveAtom )
      { // Calcul the new barycenter once.
        m_isMoveAtom = true ;
      }
    }
  }


  /**
    * Move atoms. This is the 1st action to move atoms. Then, use moveAtomEnd().
    * It is necesary to calculate the barycenter with less performance penality.
    * @param wmavoAction All actions ask by the wrapper
    * @param primList The selected atoms which move
    * @param vectAtomTranslate The transformation vector to translate atom(s)
    * @param transfAtomRotate The transformation matrix to rotate atom(s)
    */
  void WmExtension::moveAtomBegin( int wmavoAction, QList<Primitive*> primList, Vector3d vectAtomTranslate, Transform3d transfAtomRotate  )
  {
    if( primList.size()>0 )
    {
      Atom *a=NULL ;

      foreach( Primitive *p, primList )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;

          if( !m_isMoveAtom )
          { // Calcul the new barycenter foreach atom.
            // It must bo done once for all atoms.
            //m_wmavoThread->setWmAtomPos( *(a->pos()), false, true ) ;
            updateBarycenter( *(a->pos()), false ) ;
          }

          if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) )
            a->setPos( *(a->pos()) + vectAtomTranslate );

          if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) )
            a->setPos( transfAtomRotate * *(a->pos()) );
        }
      }

      if( !m_isMoveAtom )
      { // Calcul the new barycenter once.
        m_isMoveAtom = true ;
      }
    }
  }


  /**
    * Finish the action of "Move atoms". It let to calculate the barycenter
    * with less performance penality.
    * @param atomList The selected atoms which change the barycenter
    */
  void WmExtension::moveAtomEnd( QList<Atom *> atomList )
  {
    if( m_isMoveAtom )
    {
      m_isMoveAtom = false ;

      // Calculate the barycenter.
      foreach( Atom *a, atomList )
      {
        if( a!=NULL && a->type()==Primitive::AtomType )
          updateBarycenter( *(a->pos()), true ) ;
      }
    }
  }


  /**
    * Finish the action of "Move atoms". It let to calculate the barycenter
    * with less performance penality.
    * @param primList The selected atoms which change the barycenter
    */
  void WmExtension::moveAtomEnd( QList<Primitive *> primList )
  {
    if( m_isMoveAtom )
    {
      Atom *a=NULL ;
      m_isMoveAtom = false ;

      // Calculate the barycenter.
      foreach( Primitive *p, primList )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          //m_wmavoThread->setWmAtomPos( *(a->pos()), true, false ) ;
          updateBarycenter( *(a->pos()), true ) ;
        }
      }
    }
  }


  /**
    * Change the order of a bond by +1 without adjustment of hydrogen. Only a (WMEX_MAXBONDNUMBER_BETWEENATOM)
    * is accepted.
    * @param molecule The molecule where the change is realized
    * @param bond The bond where the change is realized
    */
  void WmExtension::changeOrderBondBy1( Molecule *molecule, Bond *bond )
  {
    if( molecule!=NULL && bond != NULL )
    {
      if( bond->order()+1 > WMEX_MAXBONDNUMBER_BETWEENATOM )
        bond->setOrder( 1 ) ;
      else
        bond->setOrder( bond->order()+1 ) ;
    }
  }


  /**
    * Change the order of a bond by +1 with adjustment of hydrogen. Only a (WMEX_MAXBONDNUMBER_BETWEENATOM)
    * is accepted.
    * @param molecule The molecule where the change is realized
    * @param bond The bond where the change is realized
    */
  void WmExtension::changeOrderBondBy1WithHydrogen( Molecule *molecule, Bond *bond )
  {
    if( molecule!=NULL && bond != NULL )
    {
      Atom *a1=bond->beginAtom() ;
      Atom *a2=bond->endAtom() ;

      if( !(a1->isHydrogen() || a2->isHydrogen()) )
      {
        removeHydrogen_p( molecule, a1, a2 ) ;
        removeHydrogen_p( molecule, a2, a1 ) ;

        if( bond->order()+1 > WMEX_MAXBONDNUMBER_BETWEENATOM )
          bond->setOrder( 1 ) ;
        else
          bond->setOrder( bond->order()+1 ) ;

        OpenBabel::OBMol obmol=molecule->OBMol() ;

        addHydrogen_p( molecule, &obmol, a1 ) ;
        addHydrogen_p( molecule, &obmol, a2 ) ;

        adjustPartialCharge_p( molecule, &obmol ) ;
      }
    }
  }



  /**
    * Add an atom in a molecule without adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    */
  Atom* WmExtension::addAtom( Molecule *molecule, Vector3d *pos, int atomicNumber )
  {
    Atom *a=NULL ;

    if( molecule!=NULL && pos!=NULL )
    {
      a = molecule->addAtom() ;

      if( a != NULL )
      {
        a->setPos( pos ) ;
        a->setAtomicNumber( atomicNumber ) ;

        updateBarycenter( *(a->pos()), true ) ;
      }
      else
        qDebug() << "Bug in WmExtension::addAtom() : NULL-object non expected." ;
    }

    return a ;
  }

  /**
    * Add an atom bonded with an other atom in a molecule without adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    * @param bondedAtom The atom where the atom will be bonded
    * @param order Order of the new bond
    */
  Atom* WmExtension::addAtom( Molecule *molecule, Vector3d *pos, int atomicNumber, Atom *bondedAtom, int order )
  {
    Atom *a=NULL ;

    if( molecule!=NULL && pos!=NULL )
    {
      a = addAtom( molecule, pos, atomicNumber ) ;

      if( a!=NULL && bondedAtom!=NULL )
      {
        if( !bondedAtom->isHydrogen() || (bondedAtom->isHydrogen() && bondedAtom->valence()<1) )
          addBond( molecule, a, bondedAtom, order ) ;
      }
      else
        qDebug() << "Bug in WmExtension::addAtom() : NULL-object non expected." ;
    }

    return a ;
  }


  /**
    * Add an atom in a molecule without adjustment of hydrogen.
    * See Atom::setOBAtom() method of Avogadro.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param obAtom Atom from open babel class to add in the molecule
    */
  Atom* WmExtension::addAtom( Molecule *molecule, OpenBabel::OBAtom *obAtom )
  {
    Atom *a=NULL ;

    if( molecule!=NULL && obAtom!=NULL )
    {
      a = molecule->addAtom() ;

      if( a != NULL )
      {
        a->setPos( new Vector3d(obAtom->x(), obAtom->y(), obAtom->z()) ) ;
        a->setAtomicNumber( obAtom->GetAtomicNum() ) ;
        a->setPartialCharge( obAtom->GetPartialCharge() ) ;

        if( obAtom->GetFormalCharge() != 0 )
          a->setFormalCharge( obAtom->GetFormalCharge() ) ;

        // And add any generic data as QObject properties
        std::vector<OpenBabel::OBGenericData*> data ;
        OpenBabel::OBDataIterator j ;
        OpenBabel::OBPairData *property ;
        data = obAtom->GetAllData( OpenBabel::OBGenericDataType::PairData ) ;

        for( j=data.begin() ; j!=data.end() ; ++j )
        {
          property = static_cast<OpenBabel::OBPairData *>(*j) ;

          #if AVO_VERSION_ABOVE_1_0_1
          if (property->GetAttribute() == "label")
          {
            a->setCustomLabel( property->GetValue().c_str() ) ;
            continue;
          }
          else if( property->GetAttribute() == "color" )
          {
            a->setCustomColorName( property->GetValue().c_str() ) ;
            continue;
          }
          else if( property->GetAttribute() == "radius" )
          {
            a->setCustomRadius( QString(property->GetValue().c_str()).toDouble() ) ;
            continue;
          }
          #endif

          a->setProperty( property->GetAttribute().c_str(), property->GetValue().c_str() ) ;
        }

        updateBarycenter( *(a->pos()), true ) ;
      }
      else
        qDebug() << "Bug in WmExtension::addAtom() : There is a problem with the add of an atom." ;
    }

    return a ;
  }


  /**
    * Add an atom in a molecule with adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    */
  Atom* WmExtension::addAtomWithHydrogen( Molecule *molecule, Vector3d *pos, int atomicNumber )
  {
    Atom *a=addAtom( molecule, pos, atomicNumber ) ;

    if( a != NULL )
    { // Construct an OBMol, call AddHydrogens() and translate the changes

      OpenBabel::OBMol obmol=molecule->OBMol() ;

      addHydrogen_p( molecule, &obmol, a ) ;
      adjustPartialCharge_p( molecule, &obmol ) ;
    }

    return a ;
  }


  /**
    * Add an atom bonded with an other atom in a molecule with adjustment of hydrogen.
    * @return Atom* : Atom newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos Position of the new atom
    * @param atomicNumber The atomic number of the new atom
    * @param bondedAtom The atom where the atom will be bonded
    * @param order Order of the new bond
    */
  Atom* WmExtension::addAtomWithHydrogen( Molecule *molecule, Vector3d *pos, int atomicNumber, Atom *bondedAtom, int order )
  {
    Atom *a=NULL ;

    if( bondedAtom==NULL || (bondedAtom->isHydrogen() && bondedAtom->valence()>0) )
      a = addAtom( molecule, pos, atomicNumber ) ;
    else
    {
      if( !bondedAtom->isHydrogen() )
        removeHydrogen_p( molecule, bondedAtom, NULL ) ;
      a = addAtom( molecule, pos, atomicNumber, bondedAtom, order ) ;
    }


    if( a != NULL )
    {
      OpenBabel::OBMol obmol=molecule->OBMol() ;

      if( !a->isHydrogen() )
        addHydrogen_p( molecule, &obmol, a ) ;

      if( !bondedAtom->isHydrogen() )
        addHydrogen_p( molecule, &obmol, bondedAtom ) ;

      adjustPartialCharge_p( molecule, &obmol ) ;
    }

    return a ;
  }


  /**
    * Add 2 bonded (or not bonded) atoms without adjustment of hydrogen in a molecule.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return PrimitiveList* : Atoms newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos1 Position of the 1st new atom
    * @param atomicNumber1 The atomic number of the 1st new atom
    * @param pos2 Position of the 2nd new atom
    * @param atomicNumber2 The atomic number of the 2nd new atom
    * @param order Order of the new bond
    */
  PrimitiveList* WmExtension::addAtoms( Molecule *molecule, Vector3d *pos1, int atomicNumber1, Vector3d *pos2, int atomicNumber2, int order )
  {
    PrimitiveList *addedPrim=NULL ;

    if( molecule!=NULL && pos1!=NULL && pos2!=NULL )
    {
      Atom *a1=NULL, *a2=NULL ;
      Bond *b=NULL ;

      a1 = addAtom( molecule, pos1, atomicNumber1 ) ;
      a2 = addAtom( molecule, pos2, atomicNumber2 ) ;

      if( a1!=NULL || a2!=NULL )
        addedPrim = new PrimitiveList() ;

      if( a1 != NULL )
        addedPrim->append( static_cast<Primitive*>(a1) ) ;

      if( a2 != NULL )
        addedPrim->append( static_cast<Primitive*>(a2) ) ;

      if( a1!=NULL && a2!=NULL && order>0 )
      {
        b = addBond( molecule, a1, a2, order ) ;

        if( b != NULL )
          addedPrim->append( static_cast<Primitive*>(b) ) ;
      }
    }

    return addedPrim ;
  }


  /**
    * Add 2 bonded (or not) atoms with adjustment of hydrogen in a molecule.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return PrimitiveList* : Atoms newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param pos1 Position of the 1st new atom
    * @param atomicNumber1 The atomic number of the 1st new atom
    * @param pos2 Position of the 2nd new atom
    * @param atomicNumber2 The atomic number of the 2nd new atom
    * @param order Order of the new bond
    */
  PrimitiveList* WmExtension::addAtomsWithHydrogen( Molecule *molecule, Vector3d *pos1, int atomicNumber1, Vector3d *pos2, int atomicNumber2, int order )
  {
    PrimitiveList *addedPrim=addAtoms( molecule, pos1, atomicNumber1, pos2, atomicNumber2, order ) ;

    if( addedPrim != NULL )
    {
      OpenBabel::OBMol obmol=molecule->OBMol() ;
      bool addH=false ;
      Atom *a=NULL, *na=NULL ;

      foreach( Primitive *p, *addedPrim )
      {
        if( p->type() == Primitive::AtomType )
        {
          a = static_cast<Atom*>(p) ;
          addH = addHydrogen_p( molecule, &obmol, a ) ;

          // Add all new primitives.
          if( addH )
          {
            foreach( unsigned long i, a->neighbors() )
            {
              na = molecule->atomById(i) ;

              if( na != NULL )
              {
                addedPrim->append( na ) ;
                addedPrim->append( a->bond(na) ) ;
              }
            }
          }
        }
      }

      adjustPartialCharge_p( molecule, &obmol ) ;
    }

    return addedPrim ;
  }


  /**
    * Add a bond in a molecule without adjusmtent of hydrogen.
    * @return Bond* : Bond newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param a1 The 1st atom to connect
    * @param a2 The 2nd atom to connect
    * @param order Order of the new bond
    */
  Bond* WmExtension::addBond( Molecule *molecule, Atom *a1, Atom *a2, short order )
  {
    Bond *b=NULL ;

    if( molecule!=NULL && a1!=NULL && a2!=NULL && order>0 && order<WMEX_MAXBONDNUMBER_BETWEENATOM )
    {
      b = molecule->addBond() ;

      if( b != NULL )
      {
        b->setOrder( order ) ;
        b->setBegin( a1 ) ;
        b->setEnd( a2 ) ;
      }
      else
        qDebug() << "Bug in WmExtension::addBond() : a NULL-object not expected." ;
    }

    return b ;
  }


  /**
    * Add a bond in a molecule with adjustment of hydrogen.
    * @return Bond* : Bond newly created ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param a1 The 1st atom to connect
    * @param a2 The 2nd atom to connect
    * @param order Order of the new bond
    */
  Bond* WmExtension::addBondWithHydrogen( Molecule *molecule, Atom *a1, Atom *a2, short order )
  {
    Bond *b=NULL ;

    if( a1!=NULL && a2!=NULL
        && (!a1->isHydrogen() || (a1->isHydrogen()&&a1->valence()==0))
        && (!a2->isHydrogen() || (a2->isHydrogen()&&a2->valence()==0))
      )
      b = addBond( molecule, a1, a2, order ) ;

    if( b != NULL )
    {
      // Remove Hydrogen of a1 and a2.

      if( !a1->isHydrogen() ) removeHydrogen_p( molecule, a1, a2 ) ;
      if( !a2->isHydrogen() ) removeHydrogen_p( molecule, a2, a1 ) ;

      // Then construct an OBMol, call AddHydrogens() and translate the changes.
      OpenBabel::OBMol obmol=molecule->OBMol() ;

      if( !a1->isHydrogen() ) addHydrogen_p( molecule, &obmol, a1 ) ;
      if( !a2->isHydrogen() ) addHydrogen_p( molecule, &obmol, a2 ) ;

      adjustPartialCharge_p( molecule, &obmol ) ;
    }

    return b ;
  }


  /**
    * Deprecated : Avogadro style.
    * To add Hydrogen and Undo/Redo features after the creation.
    * @param molecule The molecule where the change is realized
    */
  void WmExtension::addAdjustHydrogenRedoUndo( Molecule *molecule )
  {
    // Add Hydrogen ...
    //molecule->addHydrogens( atom ) ;
    // Add by :
    // AddAtomDrawCommand
    // => AddAtomDrawCommand::redo()
    //   => d->postCommand = new AdjustHydrogensPostCommand(d->molecule, d->id);
    //   => d->postCommand->redo();
    //     => AdjustHydrogensPostCommand::redo()
    //       => d->molecule->addHydrogens(atom);


    /// Add undo, redo & Hydrogen.

    // we added At least the beginAtom or we created a bond to
    // an existing atom or to endAtom that we also created


    AdjustHydrogens::Options atomAdjustHydrogens = AdjustHydrogens::Never;
    if( m_addHydrogens )
    {
      if( m_drawBond )
      // if bond then only remove on undo, rest is handled by bond
        atomAdjustHydrogens = AdjustHydrogens::OnUndo ;
      else
      // if no bond then add on undo and redo
        atomAdjustHydrogens = AdjustHydrogens::Always ;
    }


    //cout << "test5" << endl ;

    /*
    if( atomAdjustHydrogens == AdjustHydrogens::Always )
      cout << "always" << endl ;

    if( atomAdjustHydrogens == AdjustHydrogens::Never )
      cout << "never" << endl ;

    if( atomAdjustHydrogens == AdjustHydrogens::OnUndo )
      cout << "onundo" << endl ;
      */

    //cout << "test6" << endl ;
    AddAtomDrawCommand *beginAtomDrawCommand=NULL ;
    //if( m_hasAddedBeginAtom ) => Comment to adjust H
    {
      if( m_hasAddedBeginAtom
          /* // it is the bond command which adjust H.
          || (!m_hasAddedBeginAtom
              && m_beginAtomDraw!=NULL && !m_beginAtomDraw->isHydrogen())
          */
        )
      { // Either new atom => QUndo,
        // either "old" atom, verif if it is a H => No QUndo, else Adjust H => QUndo

        beginAtomDrawCommand = new AddAtomDrawCommand( molecule, m_beginAtomDraw, atomAdjustHydrogens ) ;
        beginAtomDrawCommand->setText( tr("Draw Atom") ) ;
      }
    }

    //cout << "test7" << endl ;

    AddAtomDrawCommand *currentAtomDrawCommand=NULL ;
    //if( m_hasAddedCurAtom )
    {
      if( m_hasAddedCurAtom
          /* // it is the bond command which adjust H.
          || (!m_hasAddedCurAtom && m_curAtomDraw!=NULL && !m_curAtomDraw->isHydrogen())
          */
        )
      { // Either new atom => QUndo,
        // either "old" atom, verif if it is a H => No QUndo, else Adjust H => QUndo

        currentAtomDrawCommand = new AddAtomDrawCommand( molecule, m_curAtomDraw, atomAdjustHydrogens ) ;
        currentAtomDrawCommand->setText( tr("Draw Atom") ) ;
      }

      /*
      // For a Magic update. The "click update" is specified on the last created atom.
      QPoint p=m_curAtomDraw->pos() ;
      m_pointForCreate1 = m_widget->mapToGlobal(p) ;
      */
    }

    //cout << "test8" << endl ;
    AddBondDrawCommand *bondCommand=NULL ;
    if( m_hasAddedBond )
    {
      //cout << "tes9.1" << endl ;
      AdjustHydrogens::Options adjBegin=AdjustHydrogens::Never ;
      AdjustHydrogens::Options adjEnd=AdjustHydrogens::Never ;

      if( m_addHydrogens )
      {
        //cout << "test9.2" << endl ;
        /*if( m_hydrogenCommand )
        {
          // don't try to remove/add hydrogens to the hydrogen which will be changed
          // by the ChangeElement command...
          adjBegin = adjEnd = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;

          if( !m_endAtomAdded )
          {
            foreach( unsigned long id, m_bond->endAtom()->neighbors() )
            {
              Atom *nbr=widget->molecule()->atomById(id) ;
              if( nbr->isHydrogen() )
                adjEnd |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
            }
          }
        }
        else
        */
        {
          Atom *nbr=NULL ;

          if( m_bondDraw->beginAtom()->isHydrogen() )
          {
            cout << "1 m_bondDraw->endAtom()->isHydrogen()" << endl ;
            adjBegin=AdjustHydrogens::Never ;
          }
          else
          {
            adjBegin = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;


            // pre-existing atoms might need extra work
            if( !m_hasAddedBeginAtom )
            {
              cout << "test9.3" << endl ;
              foreach( unsigned long id, m_bondDraw->beginAtom()->neighbors() )
              {
                nbr = molecule->atomById(id) ;

                if( nbr->isHydrogen() )
                  adjBegin |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
              }
            }
          }

          if( m_bondDraw->endAtom()->isHydrogen() )
          {
            cout << "2 m_bondDraw->endAtom()->isHydrogen()" << endl ;
            adjEnd=AdjustHydrogens::Never ;
          }
          else
          {
            adjEnd = AdjustHydrogens::AddOnRedo | AdjustHydrogens::RemoveOnUndo ;

            if( !m_hasAddedCurAtom )
            {
              cout << "test9.4" << endl ;
              foreach( unsigned long id, m_bondDraw->endAtom()->neighbors() )
              {
                nbr = molecule->atomById(id) ;

                if( nbr->isHydrogen() )
                  adjEnd |= AdjustHydrogens::RemoveOnRedo | AdjustHydrogens::AddOnUndo ;
              }
            }
          }
        }
      }

      bondCommand = new AddBondDrawCommand( molecule, m_bondDraw, adjBegin, adjEnd ) ;
      cout << "  bondCommand:" << bondCommand << endl ;
      bondCommand->setText( tr("Draw Bond") ) ;
    }


    // Add undo command in undoStock of Avogadro.


    // Set the actual undo command -- combining sequence if possible
    // we can have a beginAtom w/out bond or endAtom
    // we can have bond w/out endAtom (i.e., to an existing atom)
    // we cannot have endAtom w/out bond
    if( currentAtomDrawCommand || (bondCommand && (beginAtomDrawCommand /*|| m_hydrogenCommand*/)) )
    {
      UndoSequence *seq=NULL ;

      //if( m_hydrogenCommand != NULL )
      //  seq->append( m_hydrogenCommand ) ;
      if( beginAtomDrawCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        cout << "  seq->append( beginAtomDrawCommand" << beginAtomDrawCommand << endl ;
        seq->append( beginAtomDrawCommand ) ;
      }

      if( currentAtomDrawCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        cout << "  seq->append( currentAtomDrawCommand" << currentAtomDrawCommand << endl ;
        seq->append( currentAtomDrawCommand ) ;
      }

      if( bondCommand != NULL )
      {
        if( seq == NULL )
        {
          seq = new UndoSequence() ;
          seq->setText(tr("Draw"));
        }

        cout << "  seq->append( bondCommand:" << bondCommand << endl ;
        seq->append( bondCommand ) ;
      }

      if( beginAtomDrawCommand!=NULL || currentAtomDrawCommand!=NULL || bondCommand!=NULL )
      {
        cout << "  undoStack->push( seq ), sequence undo:" << seq << endl ;
        m_widget->undoStack()->push( seq ) ;
      }
    }
    else if( bondCommand != NULL )
    {
      cout << "  undoStack->push( bondCommand )" << endl ;
      m_widget->undoStack()->push( bondCommand ) ;
    }
    else if( beginAtomDrawCommand != NULL )
    {
      cout << "  undoStack->push( beginAtomDrawCommand )" << endl ;
      m_widget->undoStack()->push( beginAtomDrawCommand ) ;
    }
  }


  /**
    * Copy the fragment in the molecule with adjustment of hydrogen.
    * Deprecated : Avogadro code style : no new elements returned.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    */
  void WmExtension::addFragment1( Molecule *molecule, Molecule *fragment )
  {
    *molecule += fragment ;
  }


  /**
    * Copy the fragment in the molecule without adjutment of hydrogen.
    * It copies just all atoms and bonds without Hydrogen atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    */
  PrimitiveList* WmExtension::addFragment2WithoutHydrogen( Molecule *molecule, Molecule *fragment )
  {
    PrimitiveList *addedPrim=NULL ;

    if( molecule!=NULL && fragment!=NULL )
    {
      QList<Atom*> newAtomList, oldAtomList ; // To retreive the atoms for the bond.
      Atom *newAtom=NULL, *a1=NULL, *a2=NULL, *a3=NULL, *a4=NULL ;
      Bond *newBond=NULL ;
      addedPrim = new PrimitiveList() ;
      int i=0, nbElt=0 ;
      bool find=false ;

      // "Copy" all atoms.
      foreach( Atom *oldAtom, fragment->atoms() )
      {
        if( !oldAtom->isHydrogen() )
        {
          // "Copy" atom of the fragment.
          newAtom = addAtom( molecule, const_cast<Vector3d*>(oldAtom->pos()), oldAtom->atomicNumber() ) ;

          // Store for the bonds.
          newAtomList.append( newAtom ) ;
          oldAtomList.append( oldAtom ) ;

          // Store for the return of the function.
          addedPrim->append( newAtom ) ;
        }
      }

      // "Copy" all bonds.
      foreach (Bond *oldBond, fragment->bonds() )
      {
        a1 = oldBond->beginAtom() ;
        a2 = oldBond->endAtom() ;

        if( !(a1->isHydrogen() || a2->isHydrogen()) )
        {
          a3 = NULL ;
          a4 = NULL ;
          i = 0 ;
          nbElt = oldAtomList.size() ;
          find = false ;

          // Search correspondence between fragment atoms and molecule atoms.
          while( i<nbElt && !find )
          {
            if( oldAtomList.at(i) == a1 )
              a3 = newAtomList.at(i) ;
            else if( oldAtomList.at(i) == a2 )
              a4 = newAtomList.at(i) ;

            if( a3!=NULL && a4!=NULL )
              find = true ;
            i++ ;
          }

          if( a3!=NULL && a4!=NULL )
          {
            // "Copy" bond of the fragment.
            newBond = addBond( molecule, a3, a4, oldBond->order() ) ;

            // Store for the return of the function.
            addedPrim->append( newBond ) ;
          }
        }
      }
    }

    if( addedPrim==NULL || addedPrim->size()<=0 )
    {
      if( addedPrim != NULL )
        delete addedPrim ;
      return NULL ;
    }
    else
      return addedPrim ;
  }



  /**
    * Copy the fragment in the molecule with adjustment of hydrogen.
    * It copies just all atoms and bonds.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    */
  PrimitiveList* WmExtension::addFragment2WithHydrogen( Molecule *molecule, Molecule *fragment )
  {
    PrimitiveList *addedPrim=NULL ;

    if( molecule!=NULL && fragment!=NULL )
    {
      QList<Atom*> newAtomList, oldAtomList ; // To retreive the atoms for the bond.
      Atom *newAtom=NULL, *a1=NULL, *a2=NULL, *a3=NULL, *a4=NULL ;
      Bond *newBond=NULL ;
      addedPrim = new PrimitiveList() ;
      int i=0, nbElt=0 ;
      bool find=false ;

      // "Copy" all atoms.
      foreach( Atom *oldAtom, fragment->atoms() )
      {
        // "Copy" atom of the fragment.
        newAtom = addAtom( molecule, const_cast<Vector3d*>(oldAtom->pos()), oldAtom->atomicNumber() ) ;

        // Store for the bonds.
        newAtomList.append( newAtom ) ;
        oldAtomList.append( oldAtom ) ;

        // Store for the return of the function.
        addedPrim->append( newAtom ) ;
      }

      // "Copy" all bonds.
      foreach (Bond *oldBond, fragment->bonds() )
      {
        a1 = oldBond->beginAtom() ;
        a2 = oldBond->endAtom() ;
        a3 = NULL ;
        a4 = NULL ;
        i = 0 ;
        nbElt = oldAtomList.size() ;
        find = false ;

        // Search correspondence between fragment atoms and molecule atoms.
        while( i<nbElt && !find )
        {
          if( oldAtomList.at(i) == a1 )
            a3 = newAtomList.at(i) ;
          else if( oldAtomList.at(i) == a2 )
            a4 = newAtomList.at(i) ;

          if( a3!=NULL && a4!=NULL )
            find = true ;
          i++ ;
        }

        if( a3!=NULL && a4!=NULL )
        {
          // "Copy" bond of the fragment.
          newBond = addBond( molecule, a3, a4, oldBond->order() ) ;

          // Store for the return of the function.
          addedPrim->append( newBond ) ;
        }
      }
    }

    if( addedPrim==NULL || addedPrim->size()<=0 )
    {
      if( addedPrim != NULL )
        delete addedPrim ;
      return NULL ;
    }
    else
      return addedPrim ;
  }



  /**
    * Copy a fragment in the molecule without hydrogen and without adjustment of hydrogen.
    * And it bonds it on an other atom.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    * @param bondedAtom The atom where the atom will be bonded
    */
  PrimitiveList* WmExtension::addFragment3WithoutHydrogen( Molecule *molecule, Molecule *fragment, Atom *bondedAtom )
  {
    PrimitiveList* addedPrim=NULL ;

    if( molecule!=NULL && fragment!=NULL && bondedAtom!=NULL )
    {
      bool emptyMol=(molecule->numAtoms() == 0) ;
      Atom *startAtom=NULL, *endAtom=NULL ;
      int nbAtomInFrag=fragment->numAtoms() ;

      // Add fragment in the molecule.
      addedPrim = addFragment2WithoutHydrogen( molecule, fragment ) ;

      // Verify if the (bondedAtom) belongs to the molecule. (fr:appartient)
      if( !emptyMol )
      {
        int tmp=bondedAtom->index() ;
        Atom *a=molecule->atom(tmp) ;
        if( a != bondedAtom )
          emptyMol = true ;
      }

      if( !emptyMol && addedPrim!=NULL && addedPrim->size()>0 )
      {
        startAtom = bondedAtom ;

        // Search the bonded non-hydrogen of the molecule, if it exists.
        // First, if (startAtom) is a Hydrogen, change the (startAtom).
        if( startAtom->isHydrogen() )
        {
          Atom *hydrogen=startAtom ;

          // Get the bonded non-hydrogen and remove this atom.
          if( hydrogen->neighbors().size() > 0 )
          {
            startAtom = molecule->atomById( hydrogen->neighbors()[0] ) ;
            removeAtom( molecule, hydrogen ) ;
          }
        }

        removeHydrogen_p( molecule, startAtom ) ;


        // Connect to the first atom of the fragment.
        // The 1st elements are atoms.
        Primitive *p=addedPrim->list().at(0) ;

        if( p!=NULL && p->type()==Primitive::AtomType )
          endAtom = static_cast<Atom*>(p) ;

        if( endAtom == NULL )
        {
          qDebug() << " BUG in WmExtension::addFragment3() : A NULL-object not expected." ;
          qDebug() << " Result : No connection realized between the molecule and the fragment." ;
        }
        else
        {
          // Search the bonded non-hydrogen of the fragment, if it exists.
          if( endAtom->isHydrogen() )
          {
            // Get the bonded non-hydrogen and remove this atom.
            Atom *hydrogen=endAtom ;

            if( hydrogen->neighbors().size() > 0 )
            {
              // the first bonded atom to this "H".
              endAtom = molecule->atomById( hydrogen->neighbors()[0] ) ;
              removeAtom( molecule, hydrogen ) ;
              nbAtomInFrag-- ;
            }
          }
          // else
            // Stay like that. Else, active a message error where there is not.
        }


        if( (startAtom==NULL || endAtom==NULL) //  If something to do
            || (startAtom->index() == endAtom->index()) // If bug ...
          )
        {
          qDebug() << " BUG : The connection between the fragment and the molecule are eguals !!" ;
          qDebug() << " Result : No connection realized between the molecule and the fragment." ;
        }
        else
        {
          // Transform Avogadro molecule to Openbabel molecule.
          OpenBabel::OBMol obmol=molecule->OBMol() ;

          // Arrange the added fragment with OpenBabel method.
          // Open Babel indexes atoms from 1, not 0
          OpenBabel::OBBuilder::Connect( obmol, startAtom->index()+1, endAtom->index()+1 ) ;

          // Clear and initiate the Avogadro molecule by the OpenBabel molecule.
          molecule->setOBMol( &obmol ) ;
          molecule->updateMolecule() ;
        }
      }

      // Get the added atoms. In fact, the molecule has been cleared.
      // So we must find its, and adjust the barycenter.
      if( endAtom != NULL )
      {
        int i=0 ;

        if( addedPrim == NULL )
          addedPrim = new PrimitiveList() ;
        else
          addedPrim->clear() ;

        resetBarycenter_p() ;
        foreach( Atom *atom, molecule->atoms() )
        {
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index()>=endAtom->index() && i++<nbAtomInFrag )
            addedPrim->append( atom ) ;
        }
      }
    }

    return addedPrim ;
  }


  /**
    * Copy a fragment in the molecule. And it attaches it on an other atom and adjust
    * its hydrogens.
    * Caution : Do not forget to delete the returned object after uses it.
    * @return A list of all new elements in the molecule ; else NULL if nothing add.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    * @param bondedAtom The atom where the atom will be bonded
    */
  PrimitiveList* WmExtension::addFragment3WithHydrogen( Molecule *molecule, Molecule *fragment, Atom *bondedAtom )
  {
    PrimitiveList* addedPrim=NULL ;

    if( molecule!=NULL && fragment!=NULL && bondedAtom!=NULL )
    {
      bool emptyMol=(molecule->numAtoms() == 0) ;
      Atom *startAtom=NULL, *endAtom=NULL ;
      unsigned long startAtomIndex=-1, endAtomIndex=-1 ;
      int nbAtomInFrag=fragment->numAtoms() ;

      // Add fragment in the molecule.
      addedPrim = addFragment2WithHydrogen( molecule, fragment ) ;

      // Verify if the (bondedAtom) belongs to the molecule. (fr:appartient)
      if( !emptyMol )
      {
        Atom *a=molecule->atom( bondedAtom->index() ) ;
        if( a != bondedAtom )
          emptyMol = true ;
      }

      if( !emptyMol && addedPrim!=NULL && addedPrim->size()>0 )
      {
        startAtom = bondedAtom ;

        // Search the bonded non-hydrogen of the molecule, if it exists.
        // First, if (startAtom) is a Hydrogen, change the (startAtom).
        if( startAtom->isHydrogen() )
        {
          Atom *hydrogen=startAtom ;

          // Get the bonded non-hydrogen and remove this atom.
          if( hydrogen->neighbors().size() > 0 )
          {
            startAtom = molecule->atomById( hydrogen->neighbors()[0] ) ;
            removeAtom( molecule, hydrogen ) ;
          }
        }

        startAtomIndex = startAtom->index() ;
        removeHydrogen_p( molecule, startAtom ) ;


        // Connect to the first atom of the fragment.
        // The 1st elements are atoms.
        Primitive *p=addedPrim->list().at(0) ;

        if( p!=NULL && p->type()==Primitive::AtomType )
          endAtom = static_cast<Atom*>(p) ;

        if( endAtom == NULL )
        {
          qDebug() << " BUG in WmExtension::addFragment3() : A NULL-object not expected." ;
          qDebug() << " Result : No connection realized between the molecule and the fragment." ;
        }
        else
        {
          // Search the bonded non-hydrogen of the fragment, if it exists.
          if( endAtom->isHydrogen() )
          {
            // Get the bonded non-hydrogen and remove this atom.
            Atom *hydrogen=endAtom ;

            if( hydrogen->neighbors().size() > 0 )
            {
              // the first bonded atom to this "H".
              endAtom = molecule->atomById( hydrogen->neighbors()[0] ) ;
              removeAtom( molecule, hydrogen ) ;
              nbAtomInFrag-- ;
            }
          }

          endAtomIndex = endAtom->index() ;
        }


        if( (startAtom==NULL || endAtom==NULL) //  If something to do
            || (startAtomIndex == endAtomIndex) // If bug ...
          )
        {
          qDebug() << " BUG : The connection between the fragment and the molecule are eguals !!" ;
          qDebug() << " Result : No connection realized between the molecule and the fragment." ;
        }
        else
        {
          removeHydrogen_p( molecule, endAtom ) ; //

          // Transform Avogadro molecule to Openbabel molecule.
          OpenBabel::OBMol obmol=molecule->OBMol() ;

          // Arrange the added fragment with OpenBabel method.
          // Open Babel indexes atoms from 1, not 0
          OpenBabel::OBBuilder::Connect( obmol, startAtomIndex+1, endAtomIndex+1 ) ;

          // Adjust the hydrogen of the startAtom.
          addHydrogen_p( molecule, &obmol, startAtom ) ; //

          // Adjust the hydrogen of the endAtom.
          addHydrogen_p( molecule, &obmol, endAtom ) ; //

          // Clear and initiate the Avogadro molecule by the OpenBabel molecule.
          molecule->setOBMol( &obmol ) ;
          molecule->updateMolecule() ;
        }
      }

      // Get the added atoms. In fact, the molecule has been cleared.
      // So we must find its, and adjust the barycenter.
      if( endAtom != NULL )
      {
        int i=0 ;
        bool find=false ;
        QList<Atom*> neighborsStartAtom ;
        Atom *startA=molecule->atomById( startAtomIndex ) ;
        Atom *n=NULL ;

        // Get the Hydrogen neighbors of the start atom.
        foreach( unsigned long ai, startA->neighbors() )
        {
          n = molecule->atomById( ai ) ;
          if( n->isHydrogen() )
            neighborsStartAtom.append( n ) ;
        }

        // Initiate the primitive list of selected atoms ...
        if( addedPrim == NULL )
          addedPrim = new PrimitiveList() ;
        else
          addedPrim->clear() ;

        // Calculate the barycenter and construct the selected atom list.
        resetBarycenter_p() ;
        foreach( Atom *atom, molecule->atoms() )
        {
          find = false ;
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index() >= endAtomIndex )
          {
            foreach( Atom *a, neighborsStartAtom )
            {
              if( atom == a )
                find = true ;
            }

            if( !find /*&& i<nbAtomInFrag*/ )
            {
              i ++ ;
              addedPrim->append( atom ) ;
            }
          }
        }
      }
      /*
      if( endAtom != NULL )
      {
        QList<Primitive *> matchedAtoms ;
        int i=0 ;
        resetBarycenter_p() ;

        foreach( Atom *atom, molecule->atoms() )
        {
          //m_wmavoThread->setWmAtomPos( *(atom->pos()), true, false ) ;
          updateBarycenter( *(atom->pos()), true ) ;

          if( atom->index()>=endAtom->index() && i++<nbAtomInFrag)
            matchedAtoms.append( atom ) ;
        }

        (*addedPrim) = matchedAtoms ;
      }
      */
    }

    return addedPrim ;
  }




  /**
    * Deprecated : Avogadro style.
    * Add a fragment in a molecule with "undo/redo" and "adjustment of hydrogen" features.
    * Bug in the bond with the selected atom.
    * @param molecule The molecule where the change is realized
    * @param fragment The molecule which is included in (molecule)
    * @param selectedAtom The atom where the atom will be bonded
    */
  void WmExtension::addFragmentWithUndoRedoHydrogen( Molecule *molecule, Molecule *fragment, int selectedAtom )
  {
    m_widget->undoStack()->push(
          new InsertAtomByFragmentCommand( molecule, fragment,
                                           m_widget, tr("Insert Fragment"),
                                           selectedAtom) ) ;
  }


  /**
    * Remove an atom in the molecule without adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atom Atom to remove
    */
  void WmExtension::removeAtom( Molecule *molecule, Atom *atom )
  {
    if( molecule!=NULL && atom!=NULL )
    {
      updateBarycenter( *(atom->pos()), false ) ;
      molecule->removeAtom(atom) ;
    }
  }


  /**
    * Remove an atom in the molecule with its hydrogen and ajust the hydrogens of neighbors.
    * @param molecule The molecule where the change is realized
    * @param atom Atom to remove
    */
  void WmExtension::removeAtomWithHydrogen( Molecule *molecule, Atom *atom )
  {
    if( molecule!=NULL && atom!=NULL )
    {
      if( !(atom->isHydrogen() && int(atom->valence())>0) )
      {
        QList<Atom*> neighborNoH ;

        // 1st, recup all objects ... Else, after remove and add the id (or index?)
        // are unused.
        Atom *n=NULL ;
        foreach( unsigned int ai, atom->neighbors() )
        {
          n = molecule->atomById( ai ) ;
          if( !n->isHydrogen() )
            neighborNoH.append( n ) ;
        }

        // Remove current atom and its Hydrogens.
        removeHydrogen_p( molecule, atom ) ;
        removeAtom( molecule, atom ) ;

        // Adjust the others atoms.
        OpenBabel::OBMol obmol=molecule->OBMol() ;
        foreach( Atom* a, neighborNoH )
          addHydrogen_p( molecule, &obmol, a ) ;
        adjustPartialCharge_p( molecule, &obmol ) ;
      }
    }
  }


  /**
    * Deprecated : Avogadro style.
    * Remove an atom in the molecule with undo/redo and adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atom Atom to remove
    */
  void WmExtension::removeAtomWithUndoRedoHydrogen( Molecule *molecule, Atom *atom )
  {
    if( molecule!=NULL && atom!=NULL )
    {
      //cout << m_widget->molecule() << " " << atom->index() << " " << m_addHydrogens << endl ;

      // don't delete H-? atom when adjust hydrogens is on
      if( !(m_addHydrogens && atom->isHydrogen() && int(atom->valence())) )
      {
        //cout << "  Create DeleteAtomDrawCommand" << endl ;
        DeleteAtomDrawCommand *undo=new DeleteAtomDrawCommand( m_widget->molecule(),
                                                               atom->index(),
                                                               m_addHydrogens);
        //cout << " undoStack()->push( DeleteAtomDrawCommand )" << endl ;
        m_widget->undoStack()->push( undo ) ;
        //cout << " fin undoStack()->push( DeleteAtomDrawCommand )" << endl ;
      }
    }
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtoms( Molecule *molecule, QList<Atom*> *atoms )
  {
    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      foreach( Atom* a, *atoms )
        removeAtom( molecule, a ) ;
    }
  }


  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtoms( Molecule *molecule, QList<Primitive*> *atoms )
  {
    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      foreach( Primitive* p, *atoms )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
          removeAtom( molecule, static_cast<Atom*>(p) ) ;
      }
    }
  }



  /**
    * Delete atoms in the molecule without adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtoms( Molecule *molecule, PrimitiveList *atoms )
  {
    QList<Primitive*> pl=atoms->subList(Primitive::AtomType) ;
    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
      removeAtoms( molecule, &pl ) ;
  }



  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtomsWithHydrogen( Molecule *molecule, QList<Atom*> *atoms )
  {
    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      int i=0, j=atoms->size() ;
      QList<Atom*> neighborsToAdjust ;
      Atom *n=NULL ;

      // Firstly, remove hydrogen which can be remove by removeHydrogen_p() in the 2nd part.
      // If a selected H-atom is not erased here, the removeH_p() method can erase it after.
      // So when the foreach reaches this selected atom, there is a pointer to nothing.
      while( i < j )
      {
        if( atoms->at(i)->isHydrogen() && int(atoms->at(i)->valence())>0 )
        {
          // Adjust atoms to remove.
          atoms->removeAt(i) ;
          j-- ;
        }
        else
        {
          // Adjust atoms to remove
          i++ ;

          // Recup the neighbors to adjust.
          foreach( unsigned long ai, atoms->at(i)->neighbors() )
          {
            n = molecule->atomById( ai ) ;
            if( !n->isHydrogen() )
              neighborsToAdjust.append( n ) ;
          }
        }
      }

      // Secondly, remove the neighbors already in the atoms to remove.
      i = 0 ;
      j = neighborsToAdjust.size() ;
      if( j > 0 )
      {
        foreach( Atom *a, *atoms )
        {
          while( i < j )
          {
            if( neighborsToAdjust.at(i) == a )
            {
              neighborsToAdjust.removeAt(i) ;
              j-- ;
              // Do not stop the traitment here, because it can have an other same atom.
              // Neighbors of a neighbor can be a neighbor of a neighbor.
              // What else !?
            }
            else
              i++ ;
          }
        }
      }

      // Remove all atoms.
      foreach( Atom* a, *atoms )
      {
        removeHydrogen_p( molecule, a ) ;
        removeAtom( molecule, a ) ;
      }

      // Adjust Hydrogen for the old neighbors.
      OpenBabel::OBMol obmol=molecule->OBMol() ;
      foreach( Atom *a, neighborsToAdjust )
        addHydrogen_p( molecule, &obmol, a ) ;
      adjustPartialCharge_p( molecule, &obmol ) ;
    }
  }


  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtomsWithHydrogen( Molecule *molecule, QList<Primitive*> *atoms )
  {
    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
    {
      int i=0, j=atoms->size() ;
      Atom *a=NULL ;
      QList<Atom*> neighborsToAdjust ;
      Atom *n=NULL ;

      // Firstly, remove hydrogen which can be remove by removeHydrogen_p() in the 2nd part.
      while( i < j )
      {
        if( atoms->at(i)->type() == Primitive::AtomType )
        {
          a = static_cast<Atom*>(atoms->at(i)) ;

          if( a->isHydrogen() && int(a->valence())>0 )
          {
            atoms->removeAt(i) ;
            j-- ;
          }
          else
          {
            // Adjust atoms to remove
            i++ ;

            // Recup the neighbors to adjust.
            foreach( unsigned long ai, a->neighbors() )
            {
              n = molecule->atomById( ai ) ;
              if( !n->isHydrogen() )
                neighborsToAdjust.append( n ) ;
            }
          }
        }
      }

      // Secondly, remove the neighbors already in the atoms to remove.
      i = 0 ;
      j = neighborsToAdjust.size() ;
      if( j > 0 )
      {
        foreach( Primitive *p, *atoms )
        {
          if( p->type() == Primitive::AtomType )
          {
            a = static_cast<Atom*>(p) ;

            while( i < j )
            {
              if( neighborsToAdjust.at(i) == a )
              {
                neighborsToAdjust.removeAt(i) ;
                j-- ;
                // Do not stop the traitment here, because it can have an other same atom.
                // Neighbors of a neighbor can be a neighbor of a neighbor.
                // What else !?
              }
              else
                i++ ;
            }
          }
        }
      }

      // Remove all atoms.
      foreach( Primitive* p, *atoms )
      {
        if( p!=NULL && p->type()==Primitive::AtomType )
        {
          removeHydrogen_p( molecule, static_cast<Atom*>(p) ) ;
          removeAtom( molecule, static_cast<Atom*>(p) ) ;
        }
      }

      // Adjust Hydrogen for the old neighbors.
      OpenBabel::OBMol obmol=molecule->OBMol() ;
      foreach( Atom *a, neighborsToAdjust )
        addHydrogen_p( molecule, &obmol, a ) ;
      adjustPartialCharge_p( molecule, &obmol ) ;
    }
  }



  /**
    * Delete atoms in the molecule with adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param atoms Atoms to remove
    */
  void WmExtension::removeAtomsWithHydrogen( Molecule *molecule, PrimitiveList *atoms )
  {
    QList<Primitive*> pl=atoms->subList(Primitive::AtomType) ;

    if( molecule!=NULL && atoms!=NULL && atoms->size()>0 )
      removeAtomsWithHydrogen( molecule, &pl ) ;
  }



  /**
    * Delete a bond in the molecule without adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param bond Bond to remove
    */
  void WmExtension::removeBond( Molecule *molecule, Bond *bond )
  {
    if( bond != NULL )
    {
      if( bond->order() > 1 )
      {
        int tmp=bond->order() ;
        bond->setOrder( tmp-1 ) ;
      }
      else
        molecule->removeBond( bond ) ;
    }
  }



  /**
    * Delete a bond in the molecule with adjustment of hydrogen.
    * @param molecule The molecule where the change is realized
    * @param bond Bond to remove
    */
  void WmExtension::removeBondWithHydrogen( Molecule *molecule, Bond *bond )
  {
    if( molecule!=NULL && bond!=NULL )
    {
      Atom *a1=bond->beginAtom() ;
      Atom *a2=bond->endAtom() ;

      if( !(a1->isHydrogen() || a2->isHydrogen()) )
      {
        removeBond( molecule, bond ) ;

        OpenBabel::OBMol obmol=molecule->OBMol() ;

        addHydrogen_p( molecule, &obmol, a1 ) ;
        addHydrogen_p( molecule, &obmol, a2 ) ;

        adjustPartialCharge_p( molecule, &obmol ) ;
      }
    }
  }

  /**
    * Deprecated : Avogadro style.
    * Delete a bond in the molecule with undo/redo feature. There is
    * an adjustment of Hydrogen.
    * @param molecule The molecule where the change is realized
    * @param bond Bond to remove
    */
  void WmExtension::deleteBondWithUndoRedo( Molecule *molecule, Bond *bond )
  {
    if( bond != NULL )
    {
      if( molecule->lock()->tryLockForWrite() )
      {
        if( bond->order() > 1 )
        {
          int tmp=bond->order() ;
          bond->setOrder( tmp-1 ) ;

         ChangeBondOrderDrawCommand *undo = new ChangeBondOrderDrawCommand( m_widget->molecule(), bond, tmp, m_addHydrogens ) ;
          m_widget->undoStack()->push( undo ) ;
        }
        else
        {
          if( !(
                m_addHydrogens
                && ( molecule->atomById(bond->beginAtomId())->isHydrogen()
                     || molecule->atomById(bond->endAtomId())->isHydrogen()
                   )
                ))
          {
            DeleteBondDrawCommand *undo=new DeleteBondDrawCommand( m_widget->molecule(),
                                                                   bond->index(),
                                                                   m_addHydrogens) ;
            m_widget->undoStack()->push( undo ) ;
          }
        }
      }

      molecule->lock()->unlock() ;
    }
  }


  /**
    * Deprecated : Avogadro style.
    * Delete elements in the molecule with undo/redo features. There is
    * no adjustment of hydrogen !
    * @param molecule The molecule where the change is realized
    */
  void WmExtension::deleteSelectedElementUndoRedo( Molecule *molecule )
  {

    // Clear the molecule or a set of atoms
    // has the inteligence to figure out based on the number of selected items
    ClearCommand *command=new ClearCommand( molecule,
                                            m_widget->selectedPrimitives() ) ;
    m_widget->undoStack()->push( command ) ;
  }


  /**
    * Delete all elements in the molecule.
    * @param molecule The molecule where the change is realized
    */
  void WmExtension::deleteAllElement( Molecule *molecule )
  {
    if( molecule != NULL )
    {
      if( molecule->lock()->tryLockForWrite() )
      {
        molecule->clear() ;
        molecule->lock()->unlock() ;
      }
    }
  }


  /**
    * Avogadro code :
    * In theorical, add feature in the "add Hydrogen" feature to correct OpenBabel.
    * But, OpenBabel has maybe correct this thing already ...
    * @param molecule The OpenBabel molecule where the change is realized
    * @param atom The OpenBabel atom to modify
    */
  OpenBabel::OBAtom* WmExtension::setImplicitValence_p( OpenBabel::OBMol *molecule, OpenBabel::OBAtom* atom )
  {
    // Set implicit valence for unusual elements not managed by OpenBabel
    // PR#2803076
    switch( atom->GetAtomicNum() )
    {
    case 3:
    case 11:
    case 19:
    case 37:
    case 55:
    case 85:
    case 87:
      atom->SetImplicitValence(1);
      atom->SetHyb(1);
      molecule->SetImplicitValencePerceived();
      break;
    case 4:
    case 12:
    case 20:
    case 38:
    case 56:
    case 88:
      atom->SetImplicitValence(2);
      atom->SetHyb(2);
      molecule->SetImplicitValencePerceived();
      break;
    case 84: // Po
      atom->SetImplicitValence(2);
      atom->SetHyb(3);
      molecule->SetImplicitValencePerceived();
      break;
    default: // do nothing
      break;
    }

    return atom ;
  }


  /**
    * Adjust partial charge between an OpenBabel molecule and an Avogadro molecule.
    * @param molecule The molecule where the change is realized
    * @param obmol The OpenBabel molecule which contains the new values
    */
  void WmExtension::adjustPartialCharge_p( Molecule *molecule, OpenBabel::OBMol *obmol )
  {
    if( molecule!=NULL && obmol!=NULL )
    {
      unsigned int i=0, nba=molecule->numAtoms() ;

      for( i=1 ; i<=nba ; ++i )
        molecule->atom(i-1)->setPartialCharge( obmol->GetAtom(i)->GetPartialCharge() ) ;
    }
  }


  /**
    * Fill out (atom) of hydrogen atom.
    * (molecule) and (obmol) must be identical (same atoms, same number ...).
    * Do not use it before know exactly how it works.
    * @return TRUE if hydrogen atom are added ; else FALSE.
    * @param molecule The molecule where the change is realized
    * @param obmol The OpenBabel molecule identical at (molecule)
    * @param atom Atom where the hydrogen must be adjust
    */
  bool WmExtension::addHydrogen_p( Molecule *molecule, OpenBabel::OBMol *obmol, Atom* atom )
  {
    bool hasAddedH=false ;

    if( molecule!=NULL && obmol!=NULL && atom!=NULL )
    {
      unsigned int nba=0, i=0 ;
      Atom *h=NULL ;
      OpenBabel::OBAtom *oba=NULL ;

      if( !atom->isHydrogen() )
      {
        oba = obmol->GetAtom( atom->index()+1 ) ;

        if( oba != NULL )
        {
          setImplicitValence_p( obmol, oba ) ;
          hasAddedH = obmol->AddHydrogens( oba ) ;

          // All new atoms in the OBMol must be the additional hydrogens.
          if( hasAddedH )
          {
            nba = molecule->numAtoms() ;

            for( i=nba+1 ; i<=obmol->NumAtoms() ; ++i )
            {
              if( obmol->GetAtom(i)->IsHydrogen() )
              {
                oba = obmol->GetAtom(i) ;
                h = addAtom( molecule, oba ) ;

                if( h != NULL )
                  addBond( molecule, atom, h, 1 ) ;
              }
            }
          }
        }
        else
          qDebug() << "Bug in WmExtension::addHydrogen_p() : a NULL-object not expected" ;
      }
      else
        qDebug() << "Rq in WmExtension::addHydrogen_p() : atom is a Hydrogen, do not add Hydrogen" ;
    }

    return hasAddedH ;
  }


  /**
    * Remove hydrogen(s) of an atom.
    * Do not use it before know exactly how it works.
    * @return TRUE if hydrogen atom are removed ; else FALSE.
    * @param molecule The molecule where the change is realized
    * @param atom Atom where the hydrogen must be removed
    * @param atomNoRemove Atom which does not remove
    */
  bool WmExtension::removeHydrogen_p( Molecule *molecule, Atom *atom, Atom *atomNoRemove )
  {
    bool hasRemoveH=false ;

    if( molecule!=NULL && atom!=NULL )
    {
      if( !atom->isHydrogen() )
      {
        Atom *nbrAtom=NULL ;

        foreach( unsigned long i, atom->neighbors() )
        {
          nbrAtom = molecule->atomById(i) ;

          if( nbrAtom!=NULL )
          {
            if( nbrAtom != atomNoRemove )
            {
              if( nbrAtom->isHydrogen() )
              {
                removeAtom( molecule, nbrAtom ) ;
                if( !hasRemoveH ) hasRemoveH=true ;
              }
            }
            //else
            //  qDebug() << "Rq in WmExtension::removeHydrogen_p() : do not erase the (atomNoRemove)." ;
          }
          else
            qDebug() << "Bug in WmExtension::removeHydrogen_p() : a NULL-object not expected." ;
        }
      }
      else
        qDebug() << "Rq in WmExtension::removeHydrogen_p() : atom is a Hydrogen, do not remove its neighbor." ;
    }

    return hasRemoveH ;
  }


  /**
    * Slot uses to close the context menu by the context menu itself for example.
    */
  void WmExtension::closeContextMenu()
  {
    if( m_contextMenuMain != NULL )
    {
      //m_contextMenuMain->hide() :
      m_contextMenuMain->close() ;
      m_menuActive = false ;
    }
    else
      qDebug() << "Bug in WmExtension::closeContextMenum() : A NULL-OBJECT not expected." ;
  }


  /**
    * Let the WmExtension class to ask the WmTool class to calcul and display nothing ...
    * In fact, this method informs the WmTool class to cancel the current actions using the calcul
    * of distance, angle and diedre angle between atoms.
    */
  void WmExtension::askWmToolToCalculNothing()
  {
    //cout << "nothing" << endl ;
    emit initiatedCalculDistDiedre( -1 ) ;
  }


  /**
    * Let the WmExtension class to ask the WmTool to calcul and display the distance between 2 atoms.
    */
  void WmExtension::askWmToolToCalculDistance()
  {
    //cout << "distance" << endl ;
    emit initiatedCalculDistDiedre( 2 ) ;
  }


  /**
    * Let the WmExtension class to ask the WmTool to calcul and display the angle between 3 atoms.
    */
  void WmExtension::askWmToolToCalculAngle()
  {
    //cout << "angle" << endl ;
    emit initiatedCalculDistDiedre( 3 ) ;
  }


  /**
    * Let the WmExtension class to ask the WmTool to calcul and display the diedre angle between 4 atoms.
    */
  void WmExtension::askWmToolToCalculDiedre()
  {
    //cout << "diedre" << endl ;
    emit initiatedCalculDistDiedre( 4 ) ;
  }


  /**
    * Slot which lets of the context menu to "add" or "substitut a hydrogen atom by" a fragment.
    * @param fragmentAbsPath Path where the fragment can be find
    */
  void WmExtension::substituteAtomByFrag( QString fragmentAbsPath )
  {
    // Get the current selected atom to know where the fragment is connected.
    QList<Primitive *> selectedAtoms=m_widget->selectedPrimitives().subList( Primitive::AtomType ) ;
    PrimitiveList* newElement=NULL ;
    int selectedAtom=-1 ;
    Atom* a=NULL ;
    QList<QString> strList ;

    // There is a connection only if one atom is selected.
    if( selectedAtoms.size() == 1 )
    {
      a = dynamic_cast<Atom*>(selectedAtoms[0]) ;

      if( a ==NULL ) // "It is possible", because a dynamic_cast is realized.
      {
        strList << "Erreur lors de l'ajout d'un fragment :" << "The selected atom is NULL ... ?" ;
        emit displayedMsg( strList, QPoint(300,20) ) ;
      }
      else if( !m_addHydrogens || (m_addHydrogens && a->atomicNumber()==1) )
      {
        selectedAtom = 1 ;

        // For SubstituteAtomByFragment() Avogadro method.
        //selectedAtom = selectedAtoms[0]->index() ; //or id() if there are bugs.
      }
      else
      {
        strList << "Erreur lors de l'ajout d'un fragment :" << "Substitution possible seulement pour un atome d'Hydrogene." ;
        emit displayedMsg( strList, QPoint(300,20) ) ;
      }
    }
    else if( selectedAtoms.size() == 0 )
    {
      // Nothing to realize. Just add the fragment.
    }
    else //if( selectedAtoms.size() != 0 )
    {
      strList << "Erreur lors de l'ajout d'un fragment :" << "Un SEUL atome ou aucun atome doit etre selectionne." ;
      emit displayedMsg( strList, QPoint(300,20) ) ;
    }


    // Read the file of the fragment, and store it.
    Molecule frag ;
    if( selectedAtoms.size() == 0
        || (selectedAtoms.size()==1 && selectedAtom!=-1)
      )
    {
      QString file( fragmentAbsPath ) ;

      OBConversion conv ;
      OBFormat *inFormat=conv.FormatFromExt( file.toAscii() ) ;

      if( !inFormat || !conv.SetInFormat(inFormat) )
      {
        qDebug() << "Cannot read file format of file " << file ;
        return ;
      }

      std::ifstream ifs ;
      ifs.open( QFile::encodeName(file) ) ;

      if( !ifs )
      {
        qDebug() << "Cannot read file " << file ;
        return ;
      }

      OBMol obfragment ;
      conv.Read( &obfragment, &ifs ) ;

      frag.setOBMol( &obfragment ) ;
      frag.center() ;
      ifs.close() ;
    }


    // Add fragment in molecule.
    if( frag.numAtoms() > 0 )
    {
      if( selectedAtoms.size() == 0 )
      {
        // 1. Deprecated : Avogadro style, impossibility to know the new atoms.
        // addFragment1( m_widget->molecule(), frag ) ;

        // 2. New version.
        if( m_addHydrogens )
          newElement = addFragment2WithHydrogen( m_widget->molecule(), &frag ) ;
        else
          newElement = addFragment2WithoutHydrogen( m_widget->molecule(), &frag ) ;
      }
      else if( selectedAtoms.size()==1 && selectedAtom!=-1 )
      {
        // 1. Add fragment & co, and realize selection after action.
        //addFragmentWithUndoRedoHydrogen( m_widget->molecule(), &frag, selectedAtom ) ;

        // 2.
        Primitive *p=selectedAtoms[0] ;
        if( m_addHydrogens )
          newElement = addFragment3WithHydrogen( m_widget->molecule(), &frag, static_cast<Atom*>(p) ) ;
        else
          newElement = addFragment3WithoutHydrogen( m_widget->molecule(), &frag, static_cast<Atom*>(p) ) ;
      }
      //else
        // Nothing.

      // Update() method is called, because it is not called in wmActions().
      m_widget->update() ;

      if( newElement != NULL )
      {
        m_widget->clearSelected() ;
        m_widget->setSelected( *newElement, true ) ;
        delete newElement ;
      }
    }
  }


  /**
    * Set the current chemical element used for the atom creation.
    * It is modified by the periodic table, that why it is a slot.
    * @param atomicNumber Atomic number of the future new atom.
    */
  void WmExtension::setAtomicNumberCurrent( int atomicNumber )
  {
    m_atomicNumberCurrent = atomicNumber ;
    emit displayedAtomicNumberCurrent( atomicNumber ) ;
  }


  /**
    * Set the Wiimote sensitive..
    * It is modified by the slider of the settings widget (of the Wiimote tool plugin),
    * that why it is a slot.
    * @param newSensitive Sensitivity of the move of the Wiimote
    */
  void WmExtension::setWmSensitive( int newSensitive )
  {
    if( PLUGIN_WM_SENSITIVE_MIN<newSensitive && newSensitive<PLUGIN_WM_SENSITIVE_MAX )
    {
      m_wmSensitive = newSensitive ;
      m_wmavoThread->setWmSensitive(m_wmSensitive) ;
    }
  }

  /**
    * Set if there is a adjusment of hydrogen or not when new atom(s) are created.
    * @param addH TRUE to adjust the hydrogens ; FALSE else
    */
  void WmExtension::setAddHydrogen( int addH )
  {
    if( addH==Qt::Unchecked || addH==Qt::PartiallyChecked )
      m_addHydrogens = false ;
    else
      m_addHydrogens = true ;
  }

  /**
    * Invert the current choice on the adjustment of hydrogen.
    */
  void WmExtension::invertAddHydrogen()
  {
    m_addHydrogens = !m_addHydrogens ;

    if( m_addHydrogens  )
      m_changeAddHAct->setText( "No adjust Hydrogen ...") ;
    else
      m_changeAddHAct->setText( "Adjust Hydrogen ...") ;
  }


  /**
    * Calcul, adjust and inform the type of vibration to the wrapper.
    * @param active "Activate" the rumble
    * @param posAtom The position of the reference atom for the distance with others atoms
    * @param atomNotUse The atom exception which does not take in the calcul.
    */
  void WmExtension::adjustRumble( bool active, const Vector3d *posAtom, Atom *atomNotUse )
  {
    if( active )
    {
      QList<Atom*> atomList=m_widget->molecule()->atoms() ;
      double max=6, min=1.5, act=max+1 ; // Angstrom distance.
      Vector3d dist ;
      const Vector3d *tmp ;
      // max -> 100
      // min -> 0
      // x   -> y

      //qDebug() << "(*posAtom)" << (*posAtom)[0] << (*posAtom)[1] << (*posAtom)[2] ;

      // Search the nearest distance between current atom and the others.
      //qDebug() << "nb atom:" << atomList.size() ;
      for( int i=0 ; i<atomList.size() ; i++ )
      {
        if( atomList.at(i) != atomNotUse )
        {
          tmp = atomList.at(i)->pos() ;

          if( (*posAtom)[0]>((*tmp)[0]-0.5) && (*posAtom)[0]<((*tmp)[0]+0.5)
              && (*posAtom)[1]>((*tmp)[1]-0.5) && (*posAtom)[1]<((*tmp)[1]+0.5)
              && (*posAtom)[2]>((*tmp)[2]-0.5) && (*posAtom)[2]<((*tmp)[2]+0.5)
              )
          {
            act = max + 1 ;
            //qDebug() << i << " BOUM : (*(atomList.at(i)->pos())):" << (*(atomList.at(i)->pos()))[0] << (*(atomList.at(i)->pos()))[1] << (*(atomList.at(i)->pos()))[2] ;
            break ;
          }
          else
          {
            //qDebug() << i << " (*(atomList.at(i)->pos())):" << (*(atomList.at(i)->pos()))[0] << (*(atomList.at(i)->pos()))[1] << (*(atomList.at(i)->pos()))[2] ;

            dist = (*posAtom) - *(atomList.at(i)->pos()) ;
            if( dist.norm() < act  )
              act = dist.norm() ;
          }
        }
        //else
          //qDebug() << " BOUM : Atome identique" ;
      }

      //cout << "max:" << max << " min:" << min <<  endl ;
      //cout << "act:" << act << endl ;

      if( act>=0 && act<min )
      {
        //qDebug() << "wmext: adjustRumble: setWmRumble(true, true)" ;
        //m_wmavoThread->setWmRumble(true, true) ;
      }
      else if( act>=min && act<=max )
      {
        int a=100-int( ( (act-min>0?act-min:act)*100.0 ) / (max-min) ) ;
        //cout << "int((min*100.0)/max):" << a << endl ;

        if( a>0 && a<100 )
        {
          //qDebug() << "wmext: adjustRumble: setWmRumble(true, false, true, a )" ;
          m_wmavoThread->setWmRumble(true, false, true, a ) ;
        }
        else
        {
          //qDebug() << "wmext: adjustRumble 1: setWmRumble(false)" ;
          m_wmavoThread->setWmRumble(false) ;
        }
      }

      else
      {
        //qDebug() << "wmext: adjustRumble 2: setWmRumble(false)" ;
        m_wmavoThread->setWmRumble(false) ;
      }
    }
    else
    {
      //qDebug() << "wmext: adjustRumble 3: setWmRumble(false)" ;
      m_wmavoThread->setWmRumble(false) ;
    }
  }


  /**
    * Reset the current barycenter value.
    * Do not use it before know exactly how it works.
    */
  void WmExtension::resetBarycenter_p()
  {
    m_pointRefBarycenter = Vector3d(0,0,0) ;
    m_sumOfWeights = 0 ;
    m_atomsBarycenter = Vector3d(0,0,0) ;
  }


  /**
    * Update the current barycenter value.
    * @param atomPos The position to add/del in the barycenter
    * @param addOrDel Add or del the position according the need
    */
  void WmExtension::updateBarycenter( Vector3d atomPos, bool addOrDel )
  {
    if( m_widget->molecule()->numAtoms() != (unsigned int)m_sumOfWeights )
      recalculateBarycenter( m_widget->molecule() ) ;

    if( addOrDel )
    {
      m_sumOfWeights ++ ;
      m_atomsBarycenter += atomPos ;
    }
    else
    {
      m_sumOfWeights -- ;
      m_atomsBarycenter -= atomPos ;
    }

    m_pointRefBarycenter = m_atomsBarycenter / m_sumOfWeights ;
  }

  /**
    * Reset and calculate the barycenter.
    * molecule The molucule where the barycenter must be calculate
    */
  void WmExtension::recalculateBarycenter( Molecule *molecule )
  {
    resetBarycenter_p() ;

    foreach( Atom* a, molecule->atoms() )
    {
      m_sumOfWeights ++ ;
      m_atomsBarycenter += *(a->pos()) ;
    }

    if( m_sumOfWeights > 0 )
      m_pointRefBarycenter = m_atomsBarycenter / m_sumOfWeights ;
  }


  /**
    * Calculate the transformation vector and/or matrix according to the need.
    * @param wmactions All actions ask by the wrapper
    * @param curPos The current position calculate by the Wiimote
    * @param lastPos The last position calculate by the Wiimote
    * @param refPoint The position of the reference point.
    * @param rotAtomdegX The desired X-axis angle
    * @param rotAtomdegY The desired Y-axis angle
    */
  void WmExtension::calculateTransformationMatrix( int wmactions, Vector3d curPos, Vector3d lastPos, Vector3d refPoint, double rotAtomdegX, double rotAtomdegY )
  {
    if( WMAVO_IS2(wmactions,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmactions,WMAVO_ATOM_ROTATE) )
    {
      QPoint currentPoint(curPos[0],curPos[1]) ;
      QPoint lastPoint(lastPos[0],lastPos[1]) ;

      Vector3d fromPos=m_widget->camera()->unProject( lastPoint, refPoint ) ;
      Vector3d toPos=m_widget->camera()->unProject( currentPoint, refPoint ) ;
      Vector3d camBackTransformedXAxis=m_widget->camera()->backTransformedXAxis() ;
      Vector3d camBackTransformedYAxis=m_widget->camera()->backTransformedYAxis() ;
      Vector3d camBackTransformedZAxis=m_widget->camera()->backTransformedZAxis() ;

      if( WMAVO_IS2(wmactions,WMAVO_ATOM_TRANSLATE)
          && !(curPos[0]==lastPos[0] && curPos[1]==lastPos[1]) )
      {
        m_vectAtomTranslate = (toPos - fromPos) / WMAVO_ATOM_SMOOTHED_MOVE_XY  ;

        //cout << "currentWmPos.x():" << currentWmPos.x() << " currentWmPos.y():" << currentWmPos.y() << endl ;
        //cout << "   lastWmPos.x():" << lastWmPos.x() << " lastWmPos.y():" << lastWmPos.y() << endl ;
        //cout << "  fromPos[0]:" << fromPos[0] << " fromPos[1]:" << fromPos[1] << " fromPos[2]:" << fromPos[2] << endl ;
        //cout << "    toPos[0]:" << toPos[0] << " toPos[1]:" << toPos[1] << " fromPos[2]:" << fromPos[2] << endl ;
        //cout << "      newVectAtomTranslate:" << m_vectAtomTranslate[0] << " " << m_vectAtomTranslate[1] << " " << m_vectAtomTranslate[2] << endl ;

        if( WMAVO_IS2(wmactions,WMAVO_ATOM_TRANSLATE) )
        {
          // Z-movement.
          if( (curPos[2]-lastPos[2]) <= -WMAVO_WM_Z_MINPOINTING_MOVEALLOWED )
            m_vectAtomTranslate += (camBackTransformedZAxis*WMAVO_ATOM_MAX_MOVE_Z) ;
          if( (curPos[2]-lastPos[2]) >= WMAVO_WM_Z_MINPOINTING_MOVEALLOWED )
            m_vectAtomTranslate -= (camBackTransformedZAxis*WMAVO_ATOM_MAX_MOVE_Z) ;
        }

        //cout << "      m_vectAtomTranslate:" << m_vectAtomTranslate[0] << " " << m_vectAtomTranslate[1] << " " << m_vectAtomTranslate[2] << endl ;
      }
      else if( WMAVO_IS2(wmactions,WMAVO_ATOM_ROTATE) )
      {
        if( m_tmpBarycenter == WmAvo::m_refPoint0 )
        { // Calculate the barycenter of selected atoms.

          Vector3d tmp=WmAvo::m_refPoint0 ;
          int i=0 ;
          Atom *a=NULL ;

          foreach( Primitive *p, m_widget->selectedPrimitives() )
          {
            if( p->type() == Primitive::AtomType )
            {
              a = static_cast<Atom*>(p) ;
              tmp += *(a->pos()) ;
              i++ ;
            }
          }

          m_tmpBarycenter = tmp / i ;
        }

        // Rotate the selected atoms about the center
        // rotate only selected primitives
        m_transfAtomRotate.matrix().setIdentity();

        // Return to the center of the 3D-space.
        m_transfAtomRotate.translation() = m_tmpBarycenter ;

        // Apply rotations.
        m_transfAtomRotate.rotate(
            AngleAxisd( (rotAtomdegX/90.0)*0.1, camBackTransformedYAxis) );

        m_transfAtomRotate.rotate(
            AngleAxisd( (rotAtomdegY/90.0)*0.1, camBackTransformedXAxis) );

        /*
        m_transfAtomRotate.rotate(
            AngleAxisd( m_vectAtomTranslate[1]*WMAVO_ATOM_ROTATION_SPEED,
                        camBackTransformedXAxis)
            );
        m_transfAtomRotate.rotate(
            AngleAxisd( m_vectAtomTranslate[0]*(-WMAVO_ATOM_ROTATION_SPEED),
                        camBackTransformedYAxis)
            );
        */

        // Return to the object.
        m_transfAtomRotate.translate( -m_tmpBarycenter ) ;
      }
      else
      { // Put all transformation "at zero".

        m_transfAtomRotate.matrix().setIdentity();
        m_transfAtomRotate.translation() = Vector3d(0,0,0) ;

        m_vectAtomTranslate[0] = 0.0 ;
        m_vectAtomTranslate[1] = 0.0 ;
        m_vectAtomTranslate[2] = 0.0 ;
      }
    }
  }


  /**
    * Receive the "calcul distance" feature from the WmTool class.
    * Deprecated : it is not the WmTool class which requests a distance calculation
    * (by keyboard action).
    */
  void WmExtension::receiveRequestToCalculDistance()
  {
    m_isCalculDistDiedre = true ;
  }


  /**
    * Send the size of the Widget to the "WmAvo class".
    * Deprecated : Do not let the wrapper to decide the limit of the mouse move.
    */
  void WmExtension::initSizeWidgetForWmAvo()
  {
    // Get environment variables of the GL_VIEWPORT.
    GLint params[4] ;
    glGetIntegerv( GL_VIEWPORT, params ) ;

    // Verify if an error is appeared.
    GLenum errCode ;
    const GLubyte *errString ;
    if( (errCode=glGetError()) != GL_NO_ERROR )
    {
      errString = gluErrorString( errCode ) ;
      fprintf (stderr, "OpenGL Error: %s\n", errString);
    }

    // Initiate the wrapper class (WmAvo).
    GLdouble x=params[0] ;
    GLdouble y=params[1] ;
    GLdouble width=params[2] ;
    GLdouble height=params[3] ;

    m_wmavoThread->setWmSizeWidget( x, y, width, height ) ;
  }
} // end namespace Avogadro

Q_EXPORT_PLUGIN2(WmExtension, Avogadro::WmExtensionFactory)
