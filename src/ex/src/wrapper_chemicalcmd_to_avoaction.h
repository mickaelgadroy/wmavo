
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
#ifndef __WRAPPER_CHEMICALCMD_TO_AVOACTION_H__
#define __WRAPPER_CHEMICALCMD_TO_AVOACTION_H__

#include "warning_disable_begin.h"
#include "variousfeatures.h"

#include "wmavo_const.h"
#include "wmavo_thread.h"
#include "moleculeManipulation.h"
#include "contextmenu_to_avoaction.h"

#include <Eigen/Core> 
#include <Eigen/Geometry> 

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
#include "warning_disable_end.h"

// For connect().
//qRegisterMetaType<...>("...") ; // before the connect method()
Q_DECLARE_METATYPE( Eigen::Vector3d )

namespace Avogadro
{
  class WmExtension ;

  class WrapperChemicalCmdToAvoAction : public QObject
  {
    Q_OBJECT // To manage signal

  // Signal.
  signals:

    void setCalculDistDiedre( Atom *anAtom ) ;
     ///< Send to the WmTool class some informations for the distance calculation.

    /**
      * @name Activate/initiate display in the render zone.
      * Inform WmTool class to display something
      * @{ */
    void renderedSelectRect( bool active, QPoint p1, QPoint p2 ) ;
    void renderedAtomBond( const Eigen::Vector3d& beginAtom, const Eigen::Vector3d& endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond ) ;
    // @}


  public slots:
    void receiveRequestToCalculDistance() ;


  // Public methods.
  public:

    // WmExtension *wmex, WmAvoThread *wmthread : Just to manage the vibration ...
    // Tool *wmtool : connect signal ...
    WrapperChemicalCmdToAvoAction( GLWidget *widget, Tool *wmtool, WmExtension *wmex, WmAvoThread *wmthread ) ;
    //WrapperChemicalCmdToAvoAction( GLWidget *widget, Tool *wmtool, WmAvoThread *wmthread ) ;
    ~WrapperChemicalCmdToAvoAction() ;

    /**
      * @name Transform the wrapper actions to the Avogadro actions.
      * @{ */
    void transformWrapperActionToAvoAction( WmAvoThread::wmDataTransfert *wmData ) ;

    void transformWrapperActionToMoveMouse( int wmavoAction, const QPoint& posCursor ) ;

    void transformWrapperActionToSelectAtom
          ( int wmavoAction, const QPoint& posCursor ) ;
    void transformWrapperActionToMoveAtom
          ( int wmavoAction, 
            const Eigen::Vector3d& pointRef,
            const Eigen::Vector3d& pos3dCurrent, 
            const Eigen::Vector3d& pos3dLast, 
            double rotAtomdegX, double rotAtomdegY ) ;
    void transformWrapperActionToCreateAtomBond
          ( int wmavoAction, 
            const Eigen::Vector3d& pointRef, 
            const QPoint &posCursor ) ;

    void transformWrapperActionToDeleteAllAtomBond( int wmavoAction ) ;
    void transformWrapperActionToRemoveAtomBond( int wmavoAction, const QPoint &posCursor ) ;

    void transformWrapperActionToRotateCamera
          ( int wmavoAction, const Eigen::Vector3d& pointRef, 
            double rotCamAxeXDeg, double rotCamAxeYDeg ) ;
    void transformWrapperActionToTranslateCamera
          ( int wmavoAction, const Eigen::Vector3d& pointRef, 
            double distCamXTranslate, double distCamYTranslate ) ;
    void transformWrapperActionToZoomCamera
          ( int wmavoAction, const Eigen::Vector3d& pointRef, 
            double distCamZoom ) ;
    void transformWrapperActionToInitiateCamera
          ( int wmavoAction, const Eigen::Vector3d& pointRef ) ;

    void transformWrapperActionToUseContextMenu( int &wmavoAction, const QPoint &posCursor ) ;
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
      * @name Calculate the move of the atoms
      * Calculate the transformation matrix to apply its in the modelview matrix to move atoms.
      * @{ */
    bool calculateTransformationMatrix( int wmactions_in, 
                                        const Eigen::Vector3d& curPos_in, 
                                        const Eigen::Vector3d& lastPos_in, 
                                        const Eigen::Vector3d& refPoint_in, 
                                        double rotAtomdegX_in, 
                                        double rotAtomdegY_in,
                                        Eigen::Vector3d &vectAtomTranslate_out, 
                                        Eigen::Transform3d &m_transfAtomRotate_out ) ;
    // @}

  // Private attributs.
  private:

    /**
      *  @name Main manipulated objets.
      * @{ */
    GLWidget *m_widget ; // (shortcut)
    MoleculeManipulation *m_moleculeManip ; // (object)
    ContextMenuToAvoAction *m_contextMenu ; // (object)
    WmAvoThread *m_wmavoThread ; // (shortcut)
    WmExtension *m_wmex ; // (shortcut)
    // @}

    /**
      * @name For tool actions
      * @{ */
    bool m_isCalculDistDiedre ;
    // @}

    /**
      * @name For the movement of atoms.
      * @{ */
    bool m_isMoveAtom ;
    Eigen::Vector3d m_tmpBarycenter ;
    // @}

    /**
      * @name For the multiple selection
      * @{ */
    bool m_isRenderRect ;
    QPoint m_rectP1, m_rectP2 ;
    // @}

    /**
      * @name For the creation atom/bond
      * All attributs uses to manage the creation of atoms/bonds.
      * @{*/
    bool m_isAtomDraw, m_isBondOrder ;
    bool m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ;
    bool m_hasAddedBeginAtom, m_hasAddedCurAtom, m_hasAddedBond ;

    Eigen::Vector3d m_beginPosDraw, m_curPosDraw ;
    QPoint m_lastCursor ;
    Atom *m_beginAtomDraw, *m_curAtomDraw ;
    Bond *m_bondDraw ;

    QTime m_time ;
    int m_timeFirst, m_timeSecond ; ///< In ms.
    bool m_canDrawOther ;
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
      * @name Static definition
      * @{ */
    static const Eigen::Vector3d m_vect3d0 ; ///< A null vector3d.
    static Eigen::Transform3d m_transf3d0 ; ///< A null transform3d.
    static const QPoint m_qpoint0 ; ///< A null QPoint.
    static const double m_PI180, m_180PI ; ///< PI/180, 180/PI
    // @}


  // Eigen need.
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
      // And NOT EIGEN_DONT_ALIGN in the begin of the file (see below).
      
      //#ifdef _WIN32
      //#define EIGEN_DONT_ALIGN
	      //error C2719: 'transfAtomRotate': formal parameter with __declspec(align('16')) won't be aligned
        // And EIGEN_MAKE_ALIGNED_OPERATOR_NEW at the end of the file.
      //#endif

  };
}

#endif