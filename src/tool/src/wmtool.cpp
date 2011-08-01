
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
      m_settingsWidget(NULL),
      m_wm(NULL), m_wmDataFrom(NULL), m_wmDataTo(NULL),
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
    if( m_widget == NULL )
    {
      m_widget = widget ;

      // Normaly, useless ...
      //ContextMenuEater *cm=new ContextMenuEater( this, m_widget ) ;
      //m_widget->installEventFilter( cm ) ;
    }

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


  void WmTool::initAllMainObject()
  {
    // See settingsWidget() method
    //m_settingsWidget = new SettingsWidget() ;

    m_wm = new InputDevice::WmDevice() ;
    m_wmDataFrom = m_wm->getDeviceDataFrom() ;
    m_wmDataTo = m_wm->getDeviceDataTo() ;

    m_drawObject = new DrawObject() ;
    m_renderText = new RenderText() ;
    m_distAngleDiedre = new DistanceAngleDiedre() ;
  }

}

Q_EXPORT_PLUGIN2(wmtool, Avogadro::WmToolFactory)
