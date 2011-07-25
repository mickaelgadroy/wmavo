
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

#pragma once
#ifndef __WMEXTENSION_H__
#define __WMEXTENSION_H__

#include "warning_disable_begin.h"
#include "variousfeatures.h"

#include "wmtool.h"
#include "wmavo_const.h"
#include "wmavo_thread.h"
#include "wrapper_chemicalcmd_to_avoaction.h"
#include <Eigen/Core>
#include <avogadro/extension.h>
#include <avogadro/atom.h>

#include "warning_disable_end.h"


// For connect().
//qRegisterMetaType<...>("...") ; // In PerformAction(), before the connect method()
Q_DECLARE_METATYPE( WmAvoThread::wmDataTransfert )


namespace Avogadro
{
  //class WrapperChemicalCmdToAvoAction ;

  /**
    * @class WmExtension
    * @brief The class lets to include the Wiimote in Avogadro.
    *
    * In fact, the WmavoThread class is instancied to launch the connection of the Wiimote,
    * then it polls. There are many others elements manages here :
    * - interpret the signal to realize the associated actions ;
    * - the menu context
    * - the calls to display informations in the render zone
    * - ...
    */
  class WmExtension : public Extension
  {
    /**
      * @name Object Qt initialization.
      * @{ */
    Q_OBJECT
      AVOGADRO_EXTENSION("WmExtension", tr("WmExtension"),
                         tr("Use Wiimote to manage camera and manipulate atoms"))
      // @}

      #include "doxygenbug.h"


      // Signal.
      signals :

        /**
          * @name Activate/initiate display in the render zone.
          * Inform WmTool class to display something
          * @{ */
        void displayedWmInfo( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance ) ;
        //@}

        /**
          * @name Initiate the WmTool class.
          * Send the WmExtention object itself to the WmTool class. It lets the WmTool class
          * to communicate with WmExtention.
          * @{ */
        void setToolWmExt( Extension *wmExt ) ;
        // @}


    // Public methods (slots).
    public slots:

      /**
        * @name Main methods of the WmExtension class
        * This is the main method where all actions are executed.
        * It is called by the wrapper to execute new actions.
        * @{ */
      void wmActions( WmAvoThread::wmDataTransfert wmData ) ;
      // @}

      /**
        * @name Manage some Wiimote features
        * Recup/Manage informations on the Wiimote states from the WmAvo class.
        * @{ */
      void wmConnected( int nbConnected ) ; ///< Inform the result of a connection attempt.
      void wmDisconnected() ; ///< Inform the result of a disconnection attempt.
      void setActivatedVibration( int state ) ; ///< Set the vibration of the Wiimote (from WmTool class)
      void setWmSensitive( int newSensitive ) ; ///< Set the sensitive of the Wiimote (from WmTool class)
      // @}

     
    // Public methods.
    public:

      /**
        * @name Avogadro default methods
        * @{ */
      WmExtension(QObject *parent=0) ; ///< Constructor
      ~WmExtension() ; ///< Destructor

      virtual QList<QAction*> actions() const ;
      virtual QString menuPath(QAction *action) const;
      virtual void mouseMoveEvent( QMouseEvent *event ) ;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget) ;
      //@}

      void adjustRumble( bool active, const Eigen::Vector3d *posAtom=NULL, Atom *atomNotUse=NULL ) ;
          ///< Adjust rumble according to distance between the nearest atom.

      

    // Private methods.
    private :

      /**
        * @name Some methods of initialization
        * @{ */
      bool wmBeginUse() ; ///< To use a Wiimote.
      void initPullDownMenu() ; ///< To manipulate the pull-down menu in the Avogadro menu bar.
      void initSizeWidgetForWmAvo() ;

      void initAndActiveForWmToolMenu() ;
          ///< To activate the tool menu of the Wiimote tool plugin in Avogadro, and initiate some data for the use of WmTool class.
      bool initSignalBetweenWmExtWmTool() ;
      void sendWmInfoToWmTool( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance ) ;
      // @}


    // Private attributs.
    private:

      /** Use in the pull-down menu to represent the actions 
        * of each button of the Wiimote menu. */
      enum m_wmMenuState
      {
          ConnectWm = 0, DisconnectWm,
          OpMode1, OpMode2, OpMode3
      } ;

      GLWidget *m_widget ; ///< (shortcut)
      Tool *m_wmTool, *m_drawTool ; ///< (shortcut)
      QList<QAction*> m_pullDownMenuActions ;
      WmAvoThread *m_wmavoThread ; // (object)
      WrapperChemicalCmdToAvoAction *m_wrapperChemToAvo ; // (object)

      bool m_wmIsConnected ;
      bool m_wmIsAlreadyConnected ;
      int m_wmSensitive ;
  };


  /**
   * @class WmExtensionFactory wmextension.h
   * @brief Factory class to create instances of the WmExtension class.
   */
  class WmExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(WmExtension)
  };

} // end namespace Avogadro

#endif

