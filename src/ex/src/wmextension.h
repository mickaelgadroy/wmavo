
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

#ifndef __WMEXTENSION_H__
#define __WMEXTENSION_H__

#ifdef _WIN32
#define EIGEN_DONT_ALIGN
	//error C2719: 'transfAtomRotate': formal parameter with __declspec(align('16')) won't be aligned
#endif

#ifdef _WIN32
#pragma warning( disable : 4365 ) // conversion from 'x' to 'y', signed/unsigned mismatch
#pragma warning( disable : 4820 ) // 'x' bytes padding added after data member '...'
#pragma warning( disable : 4668 ) // '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable : 4514 ) // '...' : unreferenced inline function has been removed
#pragma warning( disable : 4738 ) // storing 32-bit float result in memory, possible loss of performance
#pragma warning( disable : 4710 ) // function not inlined
#pragma warning( disable : 4626 ) // '...' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning( disable : 4625 ) // '...' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning( disable : 4711 ) /* The compiler performed inlining on the given function, although it was not marked for inlining. Inlining is performed at the compiler's discretion. This warning is informational. */
#pragma warning( disable : 4628 ) /* Digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'. Digraphs are not supported under /Ze. This warning will be followed by an error. */
#pragma warning( push, 0 )
#endif

//#include "wmtool.h"
#include "wmavo_const.h"
#include "wmavo_thread.h"
#include "undocommand.h"
#include "context_menu.h"
#include "action_modified.h"
//Do not work. #include <avogadro/../../libavogadro/src/tools/drawtool.h>
//#include "/home/myck/install/linux/avogadro/4-avogadro-1.1.0/avogadro/libavogadro/src/tools/drawtool.h"
//#include "/home/myck/install/linux/avogadro/4-avogadro-1.1.0/avogadro/libavogadro/src/tools/drawcommand.h"

#include <avogadro/extension.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/navigate.h>
#include <avogadro/primitivelist.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/periodictableview.h>
#include <avogadro/glhit.h>
#include <avogadro/undosequence.h>

#include <QUndoCommand>
#include <QMessageBox>
#include <QAction>
#include <QTimer>
#include <QToolButton>
#include <qapplication.h>
#include <qevent.h>
#include <qcursor.h>
#include <QString>
#include <QDebug>
#include <QDir>

#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>
#include <openbabel/generic.h>
#include <openbabel/forcefield.h>
#include <openbabel/obiter.h>
#include <openbabel/obconversion.h>
#include <openbabel/builder.h>

#include <Eigen/Core>
#include <GL/glut.h> // To recognize GLUT_WINDOW_X and Y ...
                  // And to realize some tests on the projection matrix.

#ifdef _WIN32
#pragma warning( pop )
#endif

// For connect().
//qRegisterMetaType<...>("...") ; // In PerformAction(), before the connect method()
Q_DECLARE_METATYPE( WmAvoThread::wmDataTransfert )
Q_DECLARE_METATYPE( Eigen::Vector3d )

using namespace OpenBabel ;

namespace Avogadro
{

  //class AddAtomCommand;
  //struct AdjustHydrogens ;


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


      private :
      /**
        * @name Try to solve doxygen bug with the previous macro
        * Try to solve by add a no used method.
        * This message does not appear in the doc.
        * @{ */
      void solveDoxygenBug(){} ;
        //@}


      // Signal.
      signals :

        /**
          * @name Activate/initiate display in the render zone.
          * Inform WmTool class to display something
          * @{ */
        void renderedSelectRect( bool active, QPoint p1, QPoint p2 ) ;
        void renderedAtomBond( Vector3d beginAtom, Vector3d endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond ) ;

        void displayedMsg( QList<QString> strList, QPoint pos ) ;
        void displayedWmInfo( bool connect, int nbDots, int nbSources, int distance ) ;
        void displayedAtomicNumberCurrent( int atomicNumber ) ;
        //@}


        /**
          * @name Initiate the WmTool class.
          * Send the WmExtention object itself to the WmTool class. It lets the WmTool class
          * to communicate with WmExtention.
          * @{ */
        void setToolWmExt( Extension *wmExt ) ;
        // @}


        /**
          * @name For the distance calcul feature.
          * Send to the WmTool class some informations for the distance calculation.
          * @{
          */
        void initiatedCalculDistDiedre( int what ) ;
        void setCalculDistDiedre( Atom *anAtom ) ;
        //@}


    // Public methods (slots).
    // Actions receives from the wrapper.
    public slots:

      /**
        * @name Main methods of the WmExtension class
        * This is the main method where all actions are executed.
        * It is called by the wrapper to execute new actions.
        * @{ */
      void wmActions( WmAvoThread::wmDataTransfert wmData ) ;
      // @}

      /**
        * @name Recup some Wiimote info
        * Recup informations on the Wiimote states from the WmAvo class.
        * @{ */
      void wmConnected( int nbConnected ) ; ///< Inform the result of a connection attempt.
      void wmDisconnected() ; ///< Inform the result of a disconnection attempt.
      // @}

      /**
        * @name Receive works from WmTool class
        * Deprecated : it is not the wrapper which request a distance calculation (by keyboard action).
        * @{ */
      void receiveRequestToCalculDistance() ;
      // @}


    // Private methods (slots).
    private slots :

      void closeContextMenu() ; ///< Close the context menu.
      void substituteAtomByFrag( QString fragmentAbsPath ) ;
          ///< For "substitute atom by fragment" feature (Context menu)
      void setAtomicNumberCurrent( int atomicNumber ) ;
          ///< Let the periodic table to save the selected element (Context menu)

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

      /**
        * @name Settings of the Wiimote
        * To recup option of the settings widget.
        * @{ */
      void setWmSensitive( int newSensitive ) ; ///< Set the sensitive of the Wiimote (from WmTool class)
      void setAddHydrogen( int addH ) ; ///< Add Hydrogen or not
      void invertAddHydrogen() ; ///< Add Hydrogen or not (Context menu)
      // @}


    // Public methods.
    public:

      /**
        * @name Avogadro default methods
        * @{ */
      WmExtension(QObject *parent=0) ; ///< Constructor
      ~WmExtension() ; ///< Destructor

      virtual QList<QAction *> actions() const; ///< Perform Action
      virtual QString menuPath(QAction *action) const;
      virtual void mouseMoveEvent( QMouseEvent *event ) ;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget) ;
      //@}


    // Private methods.
    private :

      /**
        * @name Some methods of initialization
        * @{ */
      bool wmBeginUse() ; ///< To use a Wiimote.
      void initPullDownMenu() ; ///< To manipulate the pull-down menu in the Avogadro menu bar.
      void initSizeWidgetForWmAvo() ;
      // @}


      /**
        * @name Initiate some other data with WmTool class
        * @{ */
      void initAndActiveForWmToolMenu() ;
          ///< To activate the tool menu of the Wiimote tool plugin in Avogadro, and initiate some data for the use of WmTool class.
      bool initSignalBetweenWmExtWmTool() ;
      void sendWmInfoToWmTool( bool connect, int nbDots, int nbSources, int distance ) ;
      // @}


      /**
        * @name Create menu for "substitute atom by fragment" option.
        * @{ */
      bool initContextMenu() ; ///< To manipulate a context menu in Avogadro.
      ContextMenu* createMenuSubstituteAtomByFragment() ;
          ///< To help in the initialization of the "Substiture atom by fragment" menu.
      ContextMenu* createMenuSABF( ContextMenu *parent, QDir dirCur ) ;
      // @}


      /**
        * @name Transform the wrapper actions to the Avogadro actions.
        * @{ */
      void transformWrapperActionToMoveAtom( int wmavoAction, Vector3d pos3dCurrent, Vector3d pos3dLast, double rotAtomdegX, double rotAtomdegY ) ;
      void transformWrapperActionToMoveMouse( int wmavoAction, QPoint posCursor ) ;
      void transformWrapperActionToSelectAtom( int wmavoAction, QPoint posCursor ) ;
      void transformWrapperActionToCreateAtomBond( int wmavoAction, QPoint posCursor, Vector3d pointRef) ;
      void transformWrapperActionToDeleteAllAtomBond( int wmavoAction ) ;
      void transformWrapperActionToRemoveAtomBond( int wmavoAction, QPoint posCursor ) ;

      void transformWrapperActionToRotateCamera( int wmavoAction, Vector3d pointRef, double rotCamAxeXDeg, double rotCamAxeYDeg ) ;
      void transformWrapperActionToTranslateCamera( int wmavoAction, Vector3d pointRef, double distCamXTranslate, double distCamYTranslate ) ;
      void transformWrapperActionToZoomCamera( int wmavoAction, Vector3d pointRef, double distCamZoom ) ;
      void transformWrapperActionToInitiateCamera( int wmavoAction, Vector3d pointRef ) ;

      void transformWrapperActionToClosePeriodicTable( int &wmavoAction, QPoint posCursor ) ;
      void transformWrapperActionToShowContextMenu( int &wmavoAction, QPoint posCursor ) ;
      // @}


      /**
        * @name Update Avogadro actions
        * All actions (almost) are updated at the end the wmActions() method according to
        * the realised actions.
        * @{ */
      void updateForAvoActions1( int wmavoAction ) ;
      void updateForAvoActions2( int wmavoAction ) ;
      void updateForAvoActions3( int wmavoAction ) ;
      // @}


      /**
        * @name Tools for all basics manipulations
        * All basic methods rewrite/mainly encpasulate, and the Avogadro methods for the
        * same things (almost).
        * Avogadro methods are specify as deprecated (for some reason explain somewhere)
        * and they are the only to realize the undo/redo feature.
        * @{ */
      void moveAtomBegin( int wmavoAction, QList<Atom*> atomList, Vector3d vectAtomTranslate, Transform3d transfAtomRotate ) ;
      void moveAtomBegin( int wmavoAction, QList<Primitive*> primList, Vector3d vectAtomTranslate, Transform3d transfAtomRotate ) ;
      void moveAtomEnd( QList<Atom*> atomList ) ;
      void moveAtomEnd( QList<Primitive*> primList ) ;

      void changeOrderBondBy1( Molecule *molecule, Bond *bond ) ;
      void changeOrderBondBy1WithHydrogen( Molecule *molecule, Bond *bond ) ;

      void changeAtomicNumberOfAtom( Molecule *molecule, Atom *atom, int atomicNumber ) ;
      void changeAtomicNumberOfAtomWithHydrogen( Molecule *molecule, Atom *atom, int atomicNumber ) ;

      Atom* addAtom( Molecule *molecule, Vector3d *pos, int atomicNumber ) ;
      Atom* addAtom( Molecule *molecule, Vector3d *pos, int atomicNumber, Atom* bondedAtom, int order ) ;
      Atom* addAtom( Molecule *molecule, OpenBabel::OBAtom *atom ) ;
      Atom* addAtomWithHydrogen( Molecule *molecule, Vector3d *pos, int atomicNumber ) ;
      Atom* addAtomWithHydrogen( Molecule *molecule, Vector3d *pos, int atomicNumber, Atom* bondedAtom, int order ) ;

      PrimitiveList* addAtoms( Molecule *molecule, Vector3d *pos1, int atomicNumber1, Vector3d *pos2, int atomicNumber2, int order ) ;
      PrimitiveList* addAtomsWithHydrogen( Molecule *molecule, Vector3d *pos1, int atomicNumber1, Vector3d *pos2, int atomicNumber2, int order) ;

      Bond* addBond( Molecule *molecule, Atom *a1, Atom *a2, short order ) ;
      Bond* addBondWithHydrogen( Molecule *molecule, Atom *a1, Atom *a2, short order ) ;

      void addAdjustHydrogenRedoUndo( Molecule *molecule ) ;
          ///< Ajust hydrogen and add undo/redo once addAtom() or addBond() are called. Deprecated : Avogadro style.

      void addFragment1( Molecule *molecule, Molecule *fragment ) ; ///< Deprecated : Avogadro style.

      //QList<Primitive *>* addFragment( Molecule *molecule, Molecule *fragment ) ;
      PrimitiveList* addFragment2WithoutHydrogen( Molecule *molecule, Molecule *fragment ) ;
      PrimitiveList* addFragment2WithHydrogen( Molecule *molecule, Molecule *fragment ) ;
      PrimitiveList* addFragment3WithoutHydrogen( Molecule *molecule, Molecule *fragment, Atom *bondedAtom ) ;
      PrimitiveList* addFragment3WithHydrogen( Molecule *molecule, Molecule *fragment, Atom *bondedAtom ) ;
      void addFragmentWithUndoRedoHydrogen( Molecule *molecule, Molecule *fragment, int selectedAtom ) ;
      ///< Deprecated : Avogadro style.

      void removeAtom( Molecule *molecule, Atom *atom ) ;
      void removeAtomWithHydrogen( Molecule *molecule, Atom *atom ) ;
      void removeAtomWithUndoRedoHydrogen( Molecule *molecule, Atom *atom ) ;  ///< Deprecated : Avogadro style.

      void removeAtoms( Molecule *molecule, QList<Atom*> *atoms ) ;
      void removeAtoms( Molecule *molecule, QList<Primitive*> *atoms ) ;
      void removeAtoms( Molecule *molecule, PrimitiveList *atoms ) ;
      void removeAtomsWithHydrogen( Molecule *molecule, QList<Atom*> *atoms ) ;
      void removeAtomsWithHydrogen( Molecule *molecule, QList<Primitive*> *atoms ) ;
      void removeAtomsWithHydrogen( Molecule *molecule, PrimitiveList *atoms ) ;

      void removeBond( Molecule *molecule, Bond *bond ) ;
      void removeBondWithHydrogen( Molecule *molecule, Bond *bond ) ;
      void deleteBondWithUndoRedo( Molecule *molecule, Bond *bond ) ; ///< Deprecated : Avogadro style.

      void deleteSelectedElementUndoRedo( Molecule *molecule ) ; ///< Deprecated : Avogadro style.
      void deleteAllElement( Molecule *molecule ) ;
      // @}


      /**
        * @name Tools to add Hydrogen.
        * Generic code to help in the adjustment of the Hydrogen atoms.
        * @{ */
      OpenBabel::OBAtom* setImplicitValence_p( OpenBabel::OBMol *molecule, OpenBabel::OBAtom* atom ) ;
      void adjustPartialCharge_p( Molecule *molecule, OpenBabel::OBMol *obmol ) ;
      bool addHydrogen_p( Molecule *molecule, OpenBabel::OBMol *obmol, Atom* atom ) ;
      bool removeHydrogen_p( Molecule *molecule, Atom* atom, Atom *atomNoRemove=NULL ) ;
      // @}


      /**
        * @name Adjust rumble
        * Adjust rumble according to distance between the nearest atom.
        * @{ */
      void adjustRumble( bool active, const Vector3d *posAtom, Atom *atomNotUse ) ;
      // @}


      /**
        * @name Calculate the move of the atoms
        * Calculate the transformation matrix to apply its in the modelview matrix to move atoms.
        * @{ */
      bool calculateTransformationMatrix( int wmavoAction, Vector3d curPos, Vector3d lastPos, Vector3d refPoint, double rotAtomdegX, double rotAtomdegY ) ;
      // @}

      /**
        * @name For the barycenter
        * @{ */
      void resetBarycenter_p() ;
      void updateBarycenter( Vector3d atomPos, bool addOrDel, bool forceNoTestToRecalculateBarycenter=false ) ;
      void recalculateBarycenter( Molecule *molecule ) ;
      // @}



    // Private attributs.
    private:

      int iUnIncrementQuiVousVeutDuBIEN ;

      /**
        * Use in the pull-down menu to represent the actions of each button of
        * the Wiimote menu.
        */
      enum m_wmMenuState
      {
          ConnectWm = 0,
          DisconnectWm,
          OpMode1,
          OpMode2,
          OpMode3
      } ;


      //
      GLWidget *m_widget ; ///< (shortcut)
      Tool *m_wmTool, *m_drawTool ; ///< (shortcut)
      Transform3d m_cameraInitialViewPoint ; // Save the initial view point.
      QList<QAction*> m_pullDownMenuActions ;
      WmAvoThread *m_wmavoThread ;
      bool m_wmIsConnected ;
      bool m_isMoveAtom ;

      /**
        * @name Info. between the WmExtension class and the WmTool class.
        * To store the sensibility of the Wiimote and some works.
        * @{ */
      bool m_wmIsAlreadyConnected ;
      int m_wmSensitive ;
      // @}


      /**
        * @name Barycenters
        * For the barycenter of the molecule and temporary object.
        * @{ */
      Vector3d m_pointRefBarycenter ;
      int m_sumOfWeights ;
      Vector3d m_atomsBarycenter ;
      Vector3d m_tmpBarycenter ;
      // @}


      /**
        * @name Move atoms
        * To store the transformation matrix which represent the move.
        * @{ */
      Vector3d m_vectAtomTranslate ;
      Transform3d m_transfAtomRotate ;
      // @}


      /**
        * @name Use quick render
        * To redraw with quick render. En fact, it uses to simulate the click of mouse.
        * Then, Avogadro interprets and activate some options to obtain the quick render.
        * @{ */
      bool m_testEventPress ;
      QPoint *m_p ;
      QMouseEvent *m_me1, *m_me2, *m_me3 ;
      // @}


      /**
        * @name For the creation atom/bond
        * All attributs uses to manage the creation of atoms/bonds.
        * @{*/
      bool m_addHydrogens ;
      bool m_isAtomDraw, m_isBondOrder ;
      bool m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ;
      bool m_hasAddedBeginAtom, m_hasAddedCurAtom, m_hasAddedBond ;
      Vector3d m_beginPosDraw, m_curPosDraw ;
      QPoint m_lastCursor ;
      Atom *m_beginAtomDraw, *m_curAtomDraw ;
      Bond *m_bondDraw ;
      int m_atomicNumberCurrent ;
      PeriodicTableView *m_periodicTable ;

      QTime m_time ;
      int m_timeFirst, m_timeSecond ; ///< In ms.
      bool m_canDrawOther ;
      // @}

      /**
        * @name For tool actions
        * @{ */
      bool m_isCalculDistDiedre ;
      // @}

      /**
        * @name For the multiple selection
        * @{ */
      bool m_isRenderRect ;
      QPoint m_rectP1, m_rectP2 ;
      // @}

      /**
        * @name For context menu.
        * All attributs uses to manage the context menu.
        * @{*/
      bool m_menuActive ; // Better management of ON/OFF menu ...
      QList<QAction*> m_menuList ;
      ContextMenu *m_contextMenuCurrent ;
      ContextMenu *m_contextMenuMain ;
      QAction *m_cancelAct, *m_periodicTableAct ;
      ContextMenu *m_contextMenuMeasure ;
      QAction *m_noDistAct, *m_distAct, *m_angleAct, *m_diedreAct ;
      ContextMenu *m_contextMenuFragment ;
      QAction *m_insertFragAct ;
      QAction *m_addSubstituteFragAct ;
      QVector<ActionModified*> m_fragAct ;
      QVector<ContextMenu*> m_famillyFragAct ;

      QAction *m_changeAddHAct ;
      // @}


  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
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

