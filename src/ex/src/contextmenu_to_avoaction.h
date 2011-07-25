
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
#ifndef __CONTEXTMENU_TO_AVOACTION_H__
#define __CONTEXTMENU_TO_AVOACTION_H__

#include "warning_disable_begin.h"
#include "variousfeatures.h"

#include "wmtool.h"
#include "wmavo_thread.h"
#include "moleculeManipulation.h"
#include "qmenu_ex.h"
#include "qaction_ex.h"

#include <QObject>
#include <QDir>
#include <avogadro/glwidget.h>
#include <avogadro/atom.h>
#include <avogadro/periodictableview.h>

#include "warning_disable_end.h"


namespace Avogadro
{
  class WrapperChemicalCmdToAvoAction ;
/*
  // Plugins for Wiimote
  class WmTool ;
  class MoleculeManipulation ;

  // Avogadro
  class GLWidget ;
  class Atom ;
  class PeriodicTableView ;
*/

  class ContextMenuToAvoAction : QObject
  {
    Q_OBJECT // To manage signal

    // Signals.
    signals :
    /**
      * @name For the distance calcul feature.
      * Send to the WmTool class some informations for the distance calculation.
      * @{ */
    void initiatedCalculDistDiedre( int what ) ;
    //@}

    /**
      * @name Activate/initiate display in the render zone.
      * Inform WmTool class to display something
      * @{ */
    void displayedMsg( QList<QString> strList, QPoint pos ) ;
    //@}


    // Public methods (slots).
    public slots:
      void setFontSizeContextMenu( int ratio ) ;
      ///< Receive works from WmTool class


    // Private methods (slots).
    private slots :

      /**
        * @name Various
        * @{ */
      void closeContextMenu() ; ///< Close the context menu.
      void letToSubstituteAtomByFragment( QString fragmentAbsPath ) ;
          ///< For "substitute atom by fragment" feature (Context menu)
      // @}

      /**
        * @name For "distance between atoms" feature (Context menu)
        * Send to the WmTool class the informations to calculate distance and other angles
        * before display them.
        * @{ */
      void askWmToolToCalculNothing() ;
      void askWmToolToCalculDistance() ;
      void askWmToolToCalculAngle() ;
      void askWmToolToCalculDiedre() ;
      // @}


  public:

    ContextMenuToAvoAction( GLWidget *widget, MoleculeManipulation *mm, WmTool *wmtool, WmAvoThread *wmavoThread ) ;
    ~ContextMenuToAvoAction() ;

    void manageAction( int &wmavoAction, QPoint posCursor ) ;

  private :

    void buildContextMenu() ; ///< To manipulate a context menu in Avogadro.
    bool connectMainQActionSignal() ; ///< Connect the trigger signals of QAction to their methods.
    bool connectSignalToWmTool( WmTool *tool ) ; ///< Connect the signal to their wmTool methods.

    void closePeriodicTable( int &wmavoAction, QPoint posCursor ) ;

    /**
      * @name Create menu for "substitute atom by fragment" option.
      * @{ */
    QMenu_ex* createMenuSubstituteAtomByFragment() ;
        ///< To help in the initialization of the "Substiture atom by fragment" menu.
    QMenu_ex* createMenuResumeSubstituteAtomByFragment() ;
        ///< To help in the initialization of the "Substiture atom by fragment" resume menu.
    QMenu_ex* createMenuSABF( QMenu_ex *parent, QDir dirCur, bool withFamily=true ) ;
    // @}


  private :

    GLWidget *m_widget ; // (shortcut)
    WmAvoThread *m_wmavoThread ; // (shortcut)
    MoleculeManipulation *m_moleculeManip ; // (shortcut)
    PeriodicTableView *m_periodicTable ; // (object)

    /**
      * @name For context menu.
      * All attributs uses to manage the context menu.
      * @{*/
    bool m_menuActive ;
    QList<QAction*> m_menuList ;
    QMenu_ex *m_contextMenuCurrent ;
    QMenu_ex *m_contextMenuMain ;
    QAction *m_cancelAct, *m_periodicTableAct ;
    QMenu_ex *m_contextMenuMeasure ;
    QAction *m_noDistAct, *m_distAct, *m_angleAct, *m_diedreAct ;
    QMenu_ex *m_contextMenuFragment ;
    QMenu_ex *m_contextMenuResumeFragment ;
    QAction *m_insertFragAct ;
    QAction *m_addSubstituteFragAct ;
    QVector<QAction_ex*> m_fragAct ;
    QVector<QMenu_ex*> m_famillyFragAct ;

    QAction *m_changeAddHAct ;
    QMenu_ex *m_contextMenuHydrogen ;
    QAction *m_addAllHAct ;
    QAction *m_removeAllHAct ;
    // @}

  };
}
#endif
