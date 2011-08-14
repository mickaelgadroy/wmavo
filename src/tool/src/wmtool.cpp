
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Benoit Jacob

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


#include "wmtool.h"


namespace Avogadro
{

  WmTool::WmTool(QObject *parent) :
      Tool(parent),
      m_widget(NULL), m_settingsWidget(NULL),
      m_wm(NULL), m_chemWrap(NULL), m_wrapperChemToAvo(NULL),
      m_updateActionDisplay(false), m_updateInfoDeviceDisplay(false),
      m_drawObject(NULL), m_renderText(NULL), m_distAngleDiedre(NULL)
  {
    QAction *action = activateAction() ;
    if( action != NULL )
    {
      action->setIcon(QIcon(QString::fromUtf8(":/wmtool/inputdevice/wiimote.png"))) ;
      action->setToolTip(tr("Wiimote Tool")) ;
      //action->setShortcut(Qt::Key_F1) ;    
    }
  }

  WmTool::~WmTool()
  {
    // Delete by Avogadro: see settingsWidgetDestroyed() method.
    //delete m_settingsWidget ;
  }


  QUndoCommand* WmTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    widget->update(); // Call the paint() method with optimisation by Qt !!!
    //cout << "WmTool::mouseMoveEvent" << endl ; // Is display.
    return 0;
  }

  QUndoCommand* WmTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    widget->update();
    //cout << "WmTool::mousePressEvent" << endl ; // Is display.
    return 0;
  }

  QUndoCommand* WmTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    widget->update();
    //cout << "WmTool::mouseReleaseEvent" << endl ; // Is display.
    return 0;
  }

  QUndoCommand* WmTool::wheelEvent(GLWidget *, QWheelEvent *event )
  {
    event->accept();
    //widget->update();
    //cout << "WmTool::mousewheelEvent" << endl ;
    return 0 ;
  }

  QUndoCommand* WmTool::keyPressEvent(GLWidget *, QKeyEvent *)
  {
    return 0;
  }

  QUndoCommand* WmTool::keyReleaseEvent(GLWidget *, QKeyEvent *){ return 0; }

  /**
   * @return the relative usefulness of the tool - affects the order in which
   * the tools are displayed.
   */
  int WmTool::usefulness() const
  {
    // If bigger value (to have our render in first) : NO, nothing appear ...
    // Leave it like that (fr:laisser comme ça).
    return 100000 ; // 2500000
  }

  /**
    * Construct the "widget preference" of the tool pluging.
    * @return Something uses by Avogadro, and, maybe describes in Avogadro documentation.
    */
  QWidget* WmTool::settingsWidget()
  {
    if( m_settingsWidget == NULL )
      m_settingsWidget = new SettingsWidget() ;

    return m_settingsWidget->getSettingsWidget() ;
  }


  /**
    * The destructor of the "settings widget" object.
    */
  void WmTool::settingsWidgetDestroyed()
  {
    if( m_settingsWidget != NULL )
    {
      delete m_settingsWidget ;
      m_settingsWidget = NULL ;
    }
  }


  /**
    * The paint method of the tool which is used to paint any tool specific
    * visuals to the GLWidget.
    * @param widget The current GLWidget object used by Avogadro.
    */
  bool WmTool::paint( GLWidget *widget )
  {
    /*
    if( m_updateInfoDeviceDisplay )
	    mutex updateInfoDevice = false ;
    if( m_updateActionDisplay )
	    mutex updateAction = false ;
      */


    if( !widget->hasFocus() )
      widget->setFocus() ;

    m_renderText->drawMsg() ;
    m_renderText->drawWmInfo() ;

    // Paint distance, diedre & dihedral.
    m_distAngleDiedre->drawDistDiedre() ;

    // Paint atoms & bond being created.
    //if( m_drawBeginAtom || m_drawEndAtom || m_drawBond )
      m_drawObject->drawBondAtom() ;
    m_drawObject->drawCenter() ;
    m_drawObject->drawBarycenter() ;
    m_drawObject->drawCursor() ;
    m_drawObject->drawSelectRect() ;

    return true ;
  }


  void WmTool::initAndStart( GLWidget *widget )
  {
    if( m_widget == NULL )
    {
      m_widget = widget ;

      // See settingsWidget() method
      //m_settingsWidget = new SettingsWidget() ;

      m_wm = new InputDevice::WmDevice() ;
      m_chemWrap = new WITD::ChemicalWrap( m_wm ) ;
      m_wrapperChemToAvo = new WrapperChemicalCmdToAvoAction( m_widget, m_chemWrap, m_wm ) ;

      m_drawObject = new DrawObject( m_widget ) ;
      m_renderText = new RenderText( m_widget ) ;
      m_distAngleDiedre = new DistanceAngleDiedre( m_widget ) ;

      connectSignals() ;
    }

    if( m_wm->connectAndStart() && m_chemWrap->connectAndStart() )
      applyActions() ;
  }

  void WmTool::applyActions()
  {
    if( m_widget != NULL )
    {
      mytoolbox::dbgMsg( "WmExtension::wmActions : Signal received" ) ;
      //m_widget->getQuickRender() ; ??? Test if is activate and desactivate it !

      WITD::ChemicalWrapData_from *dataFrom=m_chemWrap->getWrapperDataFrom() ; // POP Data !
      WITD::ChemicalWrapData_from::wrapperActions_t wa ;
      //bool upWa=dataFrom.getWrapperAction( wa ) ;
      wa = dataFrom->getWrapperAction() ;
      int state = wa.actionsWrapper ;

      //if( m_wmIsConnected == true )
      //{
        switchToThisTool( state ) ;

        if( m_wrapperChemToAvo != NULL )
          m_wrapperChemToAvo->transformWrapperActionToAvoAction( dataFrom ) ;
      //}
    }
  }

  void WmTool::waitNewAction()
  {
    m_mutex.lock() ;
    m_updateActionDisplay = true ;
    m_mutex.unlock() ;

    emit actionsApplied() ;
  }

  void WmTool::switchToThisTool( int state )
  {
    // Active WmTool, connect signal & few initialisation.
    if( WMAVO_IS2(state,WMAVO_CURSOR_MOVE)
        || WMAVO_IS2(state,WMAVO_SELECT)      || WMAVO_IS2(state,WMAVO_CREATE)
        || WMAVO_IS2(state,WMAVO_DELETE)      || WMAVO_IS2(state,WMAVO_ATOM_MOVE)
        || WMAVO_IS2(state,WMAVO_ATOM_ROTATE) || WMAVO_IS2(state,WMAVO_ATOM_TRANSLATE)
        || WMAVO_IS2(state,WMAVO_CAM_ROTATE)  || WMAVO_IS2(state,WMAVO_CAM_ZOOM)
        || WMAVO_IS2(state,WMAVO_CAM_TRANSLATE) || WMAVO_IS2(state,WMAVO_CAM_INITIAT)
      )
    {
      QString wmPluginName=PLUGIN_WMTOOL_NAME ;

      if( m_widget->toolGroup()->activeTool()->name().compare(wmPluginName) != 0 )
      { // Current tool isn't the WmTool.

        // Activate Wmtool.
        // It is realized here because it is safer with some openGL and draw call.
        // In fact, it lets to avoid that another tool class is called to realize its jobs
        // instead of expected jobs by WmTool. (fr:job prevu)

        m_widget->toolGroup()->setActiveTool( this ) ;
        m_widget->update() ; // Must be realized to display the change in the Avogadro IHM.
      }
    }
  }

  void WmTool::connectSignals()
  {
    if( m_widget != NULL )
    {
      bool isConnect=false ;

      MoleculeManipulation *mm=m_wrapperChemToAvo->getMoleculeManip() ;
      ContextMenuToAvoAction *cm=m_wrapperChemToAvo->getContextMenu() ;
      PeriodicTableView *pt=cm->getPeriodicTable() ;

      isConnect = connect( cm, SIGNAL(initiatedCalculDistDiedre(int)),
                           m_distAngleDiedre, SLOT(setCalculDistDiedre(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : ContextMenuToAvoAction.initiatedCalculDistDiedre() -> m_distAngleDiedre.setCalculDistDiedre() !!" ) ;

      isConnect = connect( cm, SIGNAL(displayedMsg(QList<QString>,QPoint)),
                           m_renderText, SLOT(setDisplayMsg(QList<QString>,QPoint)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : ContextMenuToAvoAction.displayedMsg() -> m_renderText.setDisplayMsg() !!" ) ;

      isConnect = connect( pt, SIGNAL(elementChanged(int)), mm, SLOT(setAtomicNumberCurrent(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_periodicTable.elementChanged() -> MoleculeManipulation.setAtomicNumberCurrent() !!" ) ;
     
      isConnect = connect( pt, SIGNAL(elementChanged(int)), m_renderText, SLOT(setAtomicNumberCurrent(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_periodicTable.elementChanged() -> m_renderText.setAtomicNumberCurrent() !!" ) ;

      isConnect = connect( m_distAngleDiedre, SIGNAL(askDistDiedre()), 
                                m_wrapperChemToAvo, SLOT(receiveRequestToCalculDistance()) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_distAngleDiedre.askDistDiedre() -> m_wrapperChemToAvo.receiveRequestToCalculDistance() !!" ) ;

      isConnect = connect( m_wrapperChemToAvo, SIGNAL(setCalculDistDiedre(Atom*)), 
                           m_distAngleDiedre, SLOT(addAtom(Atom*)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_wrapperChemToAvo.setCalculDistDiedre() -> m_distAngleDiedre.calculDistDiedre() !!" ) ;

      isConnect = connect( m_wrapperChemToAvo, SIGNAL(renderedSelectRect(bool,QPoint,QPoint)), 
                           m_drawObject, SLOT(setSelectRect(bool,QPoint,QPoint)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_wrapperChemToAvo.renderedSelectRect() -> m_drawObject.setActiveRect() !!" ) ;

      // connect() must have this for non-primitive type.
      //Q_DECLARE_METATYPE(...) ; // In .h .
      qRegisterMetaType<Eigen::Vector3d>("Eigen::Vector3d") ;
      isConnect=connect( m_wrapperChemToAvo, SIGNAL(renderedAtomBond( const Eigen::Vector3d&, const Eigen::Vector3d&, bool, bool, bool)),
                         m_drawObject, SLOT(setAtomBondToDraw( const Eigen::Vector3d&, const Eigen::Vector3d&, bool, bool, bool)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : WrapperChemicalCmdToAvoAction.renderedAtomBond() -> m_drawObject.renderAtomBond() !!" ) ;


      isConnect = connect( m_settingsWidget->getSliderPointSizeFont(), SIGNAL(valueChanged(int)),
                           cm, SLOT(setFontSizeContextMenu(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_settingsWidget->m_wmPointSizeFontSlider.valueChanged() -> m_wrapperChemToAvo->getContextMenu().setFontSizeContextMenu() !!" ) ;

      isConnect = connect( m_settingsWidget->getCheckboxVibration(), SIGNAL(stateChanged(int)),
                           mm, SLOT(setHasAddedHydrogen(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_settingsWidget->getCheckboxVibration().stateChanged() -> m_wrapperChemToAvo->getMoleculeManip().setActivatedVibration() !!" ) ;

      isConnect = connect( m_settingsWidget->getSliderIRSensitive(), SIGNAL(valueChanged(int)),
                                this, SLOT(changedIrSensitiveRedirect(int)) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_irSensitiveSlider.valueChanged() -> wmTool.changedIrSensitiveRedirect() !!" ) ;
    }
  }

}

Q_EXPORT_PLUGIN2(wmtool, Avogadro::WmToolFactory)
