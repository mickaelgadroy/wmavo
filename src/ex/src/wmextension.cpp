
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
#include "wrapper_chemicalcmd_to_avoaction.h"

namespace Avogadro
{

  /**
    * Constructor.
    * @param parent Instanciate and initiate by Avogadro
    */
  WmExtension::WmExtension(QObject *parent) :
    Extension(parent),
    m_widget(NULL), m_wmTool(NULL), m_drawTool(NULL),
    m_wmavoThread(NULL), m_wrapperChemToAvo(NULL),  
    m_wmIsConnected(false), m_wmIsAlreadyConnected(false), 
    m_wmSensitive(PLUGIN_WM_SENSITIVE_DEFAULT)
  {
    #if defined WIN32 || defined _WIN32
    mytoolbox::InitializeConsoleStdIO() ;
    puts( "Your message2" ) ;
    #endif

    // Initiate the Wiimote pull-down menu before IHM starts.
    initPullDownMenu() ;
  }


  /**
    * Destructor.
    */
  WmExtension::~WmExtension()
  {
    if( m_wmavoThread != NULL )
    {
      m_wmavoThread->quit() ;
      delete( m_wmavoThread ) ;
    }

    if( m_wrapperChemToAvo != NULL )
      delete( m_wrapperChemToAvo ) ;
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
  void WmExtension::mouseMoveEvent( QMouseEvent * /*event*/ )
  {
    //cout << "WmExtension::mouseMoveEvent : [" << event->globalX() << ";" << event->globalY() << "]" << endl ;
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
    }

    // Realize other initialization according to the case.
    switch( menuAction )
    {
    case ConnectWm :
      if( !wmBeginUse() )
      {
        QString msg="A problem appears in beginWiimoteUse() method. (Launch Avogadro in command line to see much informations." ;
        mytoolbox::dbgMsg( msg ) ;
      }
      else
      {
        m_pullDownMenuActions.at(ConnectWm)->setEnabled(false) ;
        //m_pullDownMenuActions.at(DisconnectWm)->setEnabled(true) ;
        // See initPullDownMenu
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

    /*
    // See initPullDownMenu
    case OpMode1 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE1) ;
      break ;

    case OpMode2 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE2) ;
      break ;

    case OpMode3 :
      m_wmavoThread->setWmOperatingMode(WMAVO_OPERATINGMODE3) ;
      break ;
    */
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
    //puts( "WmExtension::wmActions : Signal received" ) ;
    //m_widget->getQuickRender() ; ??? Test if is activate and desactivate it !

    QPoint posCursor=wmData.posCursor ;
    int wmavoAction=wmData.wmActions ;
    int nbDotsDetected=wmData.nbDotsDetected ;
    int nbSourcesDetected=wmData.nbSourcesDetected ;
    int distBetweenSource=wmData.distBetweenSources ;
    //cout << nbDotsDetected << " " << nbSourcesDetected << " " << distBetweenSource << endl ;

    if( m_wmIsConnected == true )
    {
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
        // Here to avoid to activate the wmTool always.
        initAndActiveForWmToolMenu() ;
        //initSizeWidgetForWmAvo() ;

        if( m_wrapperChemToAvo == NULL )
          m_wrapperChemToAvo = new WrapperChemicalCmdToAvoAction( m_widget, m_wmTool, this, m_wmavoThread ) ;
      }

      sendWmInfoToWmTool( posCursor, m_wmIsConnected, nbDotsDetected, nbSourcesDetected, distBetweenSource ) ;
      m_wrapperChemToAvo->transformWrapperActionToAvoAction( &wmData ) ;      
    }
    else
    {
      mytoolbox::dbgMsg( "Wiimote not connected." ) ;
      sendWmInfoToWmTool( posCursor, m_wmIsConnected, nbDotsDetected, nbSourcesDetected, distBetweenSource ) ;
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
      QString msg="" ;
      bool isConnect=false ;
      ok = true ;

      //
      // Initiate the "Wiimote class".

      m_wmavoThread = new WmAvoThread(this) ;
      m_wmavoThread->setWmRumbleEnable(PLUGIN_WM_VIBRATION_ONOFF) ;

      //
      // Connect all signal between the "Wiimote class" and WmExtension class.

      // connect() must have this for non-primitive type.
      //Q_DECLARE_METATYPE(...) ; // In .h .
      qRegisterMetaType<WmAvoThread::wmDataTransfert>("WmAvoThread::wmDataTransfert") ;
      isConnect = connect( m_wmavoThread, SIGNAL(wmPolled(WmAvoThread::wmDataTransfert)),
                           this, SLOT(wmActions(WmAvoThread::wmDataTransfert)) ) ;
      if( !isConnect )
      {
        msg = "Problem connection signal : m_wmavoThread.wmPolled() -> wmextension.wmActions() !!" ;
        mytoolbox::dbgMsg( msg ) ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(wmConnected(int)), this, SLOT(wmConnected(int)) ) ;
      if( !isConnect )
      {
        msg = "Problem connection signal : m_wmavoThread.wmConnected() -> wmextension.wmConnected() !!" ;
        mytoolbox::dbgMsg( msg ) ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(wmDisconnected()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        msg = "Problem connection signal : m_wmavoThread.wmDisconnected() -> wmextension.wmDisconnected() !!" ;
        mytoolbox::dbgMsg( msg ) ;
        isConnect = false ;
        ok = false ;
      }

      isConnect = connect( m_wmavoThread, SIGNAL(finished()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        msg = "Problem connection signal : m_wmavoThread.finished() -> wmextension.wmDisconnected() !!" ;
        mytoolbox::dbgMsg( msg ) ;
        isConnect = false ;
        ok = false ;
      }

      /*
      isConnect = connect( m_wmavoThread, SIGNAL(terminated()), this, SLOT(wmDisconnected()) ) ;
      if( !isConnect )
      {
        msg = "Problem connection signal : m_wmavoThread.terminated() -> wmextension.wmDisconnected() !!" ;
        mytoolbox::dbgMsg( msg ) ;
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

      std::ostringstream ossWm ;
      ossWm << "Number of Wiimote connected : " << nbFound ;
      const char *msg=ossWm.str().c_str() ;
      emit message( msg ) ;

      // No paint() available in extension class ...
    }
    else
      mytoolbox::dbgMsg( "No Wiimote find" ) ;
  }


  /**
    * Send some Wiimote informations to the WmTool class to display its.
    * @param connect Wiimote connected ?
    * @param nbDots The number of LEDs detected by the Wiimote
    * @param nbSources The number of "final LEDs" really used
    * @param distance The "Wiimote distance"
    */
  void WmExtension::sendWmInfoToWmTool( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance )
  {
    if( m_wmTool != NULL )
    {
      if( connect != m_wmIsAlreadyConnected )
        m_wmIsAlreadyConnected = connect ; // Not use ...

      emit displayedWmInfo( cursor, connect, nbDots, nbSources, distance ) ;
    }
    else
    {
      mytoolbox::dbgMsg( "m_wmTool not initialized." ) ;
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
      
      std::ostringstream ossWm ;
      ossWm << "The Wiimote is disconnected now." ;
      emit message( ossWm.str().c_str() ) ;
      mytoolbox::dbgMsg( ossWm ) ;

      emit displayedWmInfo( QPoint(0,0), false, 0, 0, 0 ) ;
    }
    else
      mytoolbox::dbgMsg( "The Wiimote is not connected, so it can not be disconnected ..." );
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
/*
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
    */
    
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
        //mytoolbox::dbgMsg( t->name() ) ;

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
      mytoolbox::dbgMsg( "m_wmTool not initiate in WmExtension::initConnectionBetweenWmExtWmTool() !!" ) ;
      ok = false ;
    }
    else
    {
      bool isConnect = connect( this, SIGNAL(setToolWmExt(Extension*)), m_wmTool, SLOT(setWmExt(Extension*)) ) ;
      if( !isConnect )
      {
        mytoolbox::dbgMsg( "Problem connection signal : m_wmextension.setToolWmExt() -> m_wmTool.setWmExt() !!" ) ;
        ok = false ;
      }
      else
      {
        emit setToolWmExt(this) ;
      }

      isConnect = connect( this, SIGNAL(displayedWmInfo(const QPoint&, bool, int, int, int)),
                           m_wmTool, SLOT(setWmInfo(const QPoint&,bool, int, int, int)) ) ;
      if( !isConnect )
      {
        mytoolbox::dbgMsg( "Problem connection signal : m_wmextension.displayedWmInfo() -> m_wmTool.setWmInfo() !!" ) ;
        ok = false ;
      }

      isConnect = connect( m_wmTool, SIGNAL(changedWmSensitive(int)),
                           this, SLOT(setWmSensitive(int)) ); ;
      if( !isConnect )
      {
        mytoolbox::dbgMsg( "Problem connection signal : m_wmTool.changedWmSensitive() -> m_wmextension.setWmSensitive() !!" ) ;
        ok = false ;
      }
    }

    return ok ;
  }


  void WmExtension::setActivatedVibration( int state )
  {
    m_wmavoThread->setWmRumbleEnable( (state==1?true:false) ) ;
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

    m_wmavoThread->setWmSizeWidget( (int)x, (int)y, (int)width, (int)height ) ;
  }


  /**
    * Calcul, adjust and inform the type of vibration to the wrapper.
    * @param active "Activate" the rumble
    * @param posAtom The position of the reference atom for the distance with others atoms
    * @param atomNotUse The atom exception which does not take in the calcul.
    */
  void WmExtension::adjustRumble( bool active, const Eigen::Vector3d *posAtom, Atom *atomNotUse )
  {
    if( !active || posAtom==NULL )
    {
      //mytoolbox::dbgMsg( "wmext: adjustRumble 3: setWmRumble(false)" ) ;
      m_wmavoThread->setWmRumble(false) ;
    }
    else
    {
      double max=WMAVOTH_DIST_MAX, min=WMAVOTH_DIST_MIN, act=max+1 ; // Angstrom distance.
      Eigen::Vector3d dist ;
      Atom *a=NULL
      
      a = m_moleculeManip->calculateNearestAtom( posAtom, atomNotUse ) ;

      if( a != NULL )
      {
        dist = (*posAtom) - (*(a->pos()) ;
        act = dist.norm() ;
      }

      //cout << "max:" << max << " min:" << min <<  endl ;
      //cout << "act:" << act << endl ;

      if( act>=0 && act<min )
      {
        //mytoolbox::dbgMsg( "wmext: adjustRumble: setWmRumble(true, true)" ) ;
        //setWmRumble(true, true) ;
      }
      else if( act>=min && act<=max )
      {
        int a=100-int( ( (act-min>0?act-min:act)*100.0 ) / (max-min) ) ;
        // max -> 100
        // min -> 0
        // x   -> y

        if( a>0 && a<100 )
        {
          //mytoolbox::dbgMsg( "wmext: adjustRumble: setWmRumble(true, false, true, a )" ) ;
          m_wmavoThread->setWmRumble(true, false, true, a ) ;
        }
        else // >=max+1
        {
          //mytoolbox::dbgMsg( "wmext: adjustRumble 1: setWmRumble(false)" ) ;
          m_wmavoThread->setWmRumble(false) ;
        }
      }

      else
      {
        //mytoolbox::dbgMsg( "wmext: adjustRumble 2: setWmRumble(false)" ) ;
        m_wmavoThread->setWmRumble(false) ;
      }
    }
  }


} // end namespace Avogadro

Q_EXPORT_PLUGIN2(WmExtension, Avogadro::WmExtensionFactory)
