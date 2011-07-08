
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


#ifndef WMTOOL_H
#define WMTOOL_H

#ifdef _WIN32
#pragma warning( disable : 4365 ) // conversion from 'x' to 'y', signed/unsigned mismatch
#pragma warning( disable : 4820 ) // 'x' bytes padding added after data member '...'
#pragma warning( disable : 4668 ) // '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable : 4514 ) // '...' : unreferenced inline function has been removed
#pragma warning( disable : 4738 ) // storing 32-bit float result in memory, possible loss of performance
#pragma warning( disable : 4710 ) // 'T qvariant_cast<QSet<QAccessible::Method>>(const QVariant &)' : function not inlined
#pragma warning( push, 0 )
#endif

#include "wmavo_const.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <openbabel/math/vector3.h>

#include <avogadro/navigate.h>
#include <avogadro/glwidget.h>
#include <avogadro/tool.h>
#include <avogadro/extension.h>
#include <avogadro/atom.h>
#include <avogadro/painter.h>

#include <QAction>
#include <qpoint.h>
#include <QDebug>
#include <QPointer>
#include <QGLFormat>
#include <QApplication>
#include <qevent.h>
#include <QContextMenuEvent>
#include <QWidget>
#include <QtGui>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QSlider>
#include <QPainter>

#include <Eigen/Core>
#include <Eigen/Geometry>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE
//! [0]


#ifdef _WIN32
#pragma warning( pop )
#endif

using namespace std ;
using namespace Eigen ;
using namespace OpenBabel;


namespace Avogadro
{


  // Intercept the context menu action to avoid ... Good question.
  // In resume, it is just a test.
  // So Useless ...
  /*
  class ContextMenuEater : public QObject
  {
    Q_OBJECT

  public :
    ContextMenuEater( QObject *o, QWidget *w ) : QObject(o)
    {
      m_o = o ;
      m_widget = w ;

      cutAct = new QAction(tr("Cu&t"), this);
      cutAct->setShortcuts(QKeySequence::Cut);
      cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                              "clipboard"));
      connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

      copyAct = new QAction(tr("&Copy"), this);
      copyAct->setShortcuts(QKeySequence::Copy);
      copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                               "clipboard"));
      connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

      pasteAct = new QAction(tr("&Paste"), this);
      pasteAct->setShortcuts(QKeySequence::Paste);
      pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                                "selection"));
      connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
    } ;

  protected:

    bool eventFilter(QObject *obj, QEvent *event)
    {
      if (event->type() == QEvent::ContextMenu )
      {
        //QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
        cout << "Ate ContextMenuEvent" << endl ;


        //QMenu menu( m_widget );
        //menu.addAction(cutAct);
        //menu.addAction(copyAct);
        //menu.addAction(pasteAct);
        //menu.exec( contextMenuEvent->globalPos() ) ;

        //menu.actions() => liste d'action
        //menu.setActiveAction(); => activé l'action que l'on veuty

        return true;
      }
      else
      {
        // standard event processing
        return QObject::eventFilter(obj, event);
      }
    };


  private :

    QObject *m_o ;
    QWidget *m_widget ;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;


  private slots :

    void cut()
    {
      //infoLabel->setText(tr("Invoked <b>Edit|Cut</b>"));
    };

    void copy()
    {
      //infoLabel->setText(tr("Invoked <b>Edit|Copy</b>"));
    };

    void paste()
    {
      //infoLabel->setText(tr("Invoked <b>Edit|Paste</b>"));
    };
  };
  */



  /**
    * @class WmTool
    * @brief It serves many things : realize quick render, calculate for some feature ...
    *
    * But it is not here to manipulate the Wiimote. It is just to complet some lack in
    * the extension class (the paint() method, desactivate all mouse use ...).
    */
  class WmTool : public Tool
  {
    /**
      * @name Object Qt initialization.
      * @{ */
    Q_OBJECT
      AVOGADRO_TOOL( PLUGIN_WMTOOL_IDENTIFIER,
                     tr( PLUGIN_WMTOOL_NAME ),
                     tr( PLUGIN_WMTOOL_DESCR ),
                     tr( PLUGIN_WMTOOL_WIDGET_TITLE )
                     )
      // @}


    private :
    /**
      * @name Try to solve doxygen bug with the previous macro
      * Try to solve by add a no used method.
      * This message does not appear in the doc.
      * @{ */
    void solveDoxygenBug(){} ;
      //@}



  //
  // Signals.
  signals :

    void askDistDiedre() ; ///< Element to the calculation of the distance feature.
    void changedWmSensitive( int ) ;
    //void adjustedHydrogen( int ) ;


  //
  // Public slots.
  public slots :

    void setWmExt( Extension *wmExtens ) ;
    void setSizeRatioFont( int ratio ) ; //< Change the size of all the font.

    /**
      * @name Wiimote informations for the WmTool class.
      * @{ */
    void setWmInfo( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance ) ;
    void changedWmSensitiveRedirect( int ) ;
    // @}

    /**
      * @name To render something.
      * @{ */
    void setDisplayMsg( QList<QString> strList, QPoint pos ) ;
    void setActiveRect( bool active, QPoint p1, QPoint p2 ) ;
    void renderAtomBond( const Vector3d& beginAtom, const Vector3d& endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond ) ;
    void setAtomicNumberCurent( int atomicNumber ) ;
    // @}

    /**
      * @name For the "calculation of the distance" feature.
      */
    void setCalculDistDiedre( int what ) ;
    void calculDistDiedre( Atom *atom ) ;
    // @}

    //void setPos( QPoint p ) ;
    //void adjustedHydrogenRedirect( int ) ;


  //
  // Public constants.
  public:

    /**
      * @name Mathematical needs.
      * @{ */
    static const double m_PI ; // Pi approximation.
    static const double m_PI180 ; // 3.14/180 for degree to radian.
    static const double m_180PI ; // 180/3.14 for radian to degree.
    static const QString m_angstromStr ;
    static const QString m_degreeStr ;
    // @}


  //
  // Public methods.
  public:

    /** @name [Des|Cons]tructor methods
     * @{ */
    WmTool(QObject *parent = 0); //< Constructor.
    ~WmTool(); //< Destructor.
    // @}

    /** @name Tool Methods
     * @{
     * @brief Callback methods for ui.actions on the canvas.
     */
    virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);
    virtual QUndoCommand* keyPressEvent(GLWidget *widget, QKeyEvent *event);
    virtual QUndoCommand* keyReleaseEvent(GLWidget *widget, QKeyEvent *event);
    virtual QWidget* settingsWidget() ;
    virtual void settingsWidgetDestroyed() ;
    //@}

    /**
     * @return the relative usefulness of the tool - affects the order in which
     * the tools are displayed.
     */
    virtual int usefulness() const ;

    virtual bool paint(GLWidget *widget) ;
        ////< The paint() method of a Tool class to render in the render zone.

    //QSlider *getSliderWmSensitive() ;
    //QCheckBox *getAddHydrogenCheck() ;


  //
  // Private methods.
  private :

    /**
      * @name Draw something in the render zone
      * @{ */
    void drawRect( QPoint p1, QPoint p2, int r=-1, int g=-1, int b=-1, int a=-1  ) ;
    void drawRect( float sx, float sy, float ex, float ey, int r=-1, int g=-1, int b=-1, int a=-1 ) ;
    void drawBondAtom() ;
    void drawAtom( float rayon, const Vector3d& from ) ;
    void drawCenter() ;
    void drawBarycenter() ;
    void drawCursor() ;
    //void drawBond( float radius, const Vector3d& begin, const Vector3d& end ) ;
    void drawBond2( const Vector3d& begin, const Vector3d& end ) ;
    // @}

    /**
      * @name Display text in the render zone
      * @{ */
    void displayInfo() ;
    void displayMsgInfo() ;
    void displayAtomicNumberCurrent() ;
    void displayMsgInRenderZone( QPoint pos, QString msg, QFont font, float r, float g, float b ) ;
    void displayMsgInRenderZone( const Vector3d& pos, QString msg, QFont font, float r, float g, float b ) ;
    void displayMsgOnScreen( QPoint pos, QString msg, QFont font, float r, float g, float b ) ;
    void displayMsgOnScreen( const Vector3d& pos, QString msg, QFont font, float r, float g, float b ) ;
    void displayTextMethods() ; //< Methods tested to render text.
    // @}

    /**
      * @name To distance calcul feature
      * @{ */
    void clearDistDiedre() ;
    void calculateParameters() ;
    void paintDistDiedre() ;
    // @}


  //
  // Private attributs.
  private :

    /**
      * @name For settings widget
      * @{ */
    QWidget *m_settingsWidget ;
    QSlider *m_wmSensitiveSlider ;
    QSlider *m_wmPointSizeFontSlider ;
    QCheckBox *m_checkBoxActivateVibration ;
    //QCheckBox *m_addHydrogensCheck ;
    // @}

    /**
      * @name Shortcut to some object
      * @{ */
    GLWidget *m_widget ; ///< Shortcut.
    Painter *m_wPainter ; ///< Shortcut.
    Extension *m_wmExt ; ///< Shortcut.
    // @}

    /**
      * @name Miscellaneous.
      * @{ */
    QTime m_time ;
    float m_projectionMatrix[16] ; ///< My projection matrix for some object.
    // @}

    /**
      * @name Informations of the Wiimote & Co
      * @{ */
    bool m_wmIsConnected ;
    int m_wmNbDots ;
    int m_wmNbSources ;
    int m_wmDistance ;
    int m_atomicNumberCur ;
    // @}

    /**
      * @name Need objects to render/draw something in the render zone.
      * @{ */
    QPoint m_rectPos1, m_rectPos2 ;
    bool m_activeRect ;

    bool m_drawBeginAtom, m_drawEndAtom, m_drawBond ;
    Vector3d m_beginAtom, m_endAtom ;

    GLUquadric* m_gluQuadricParams ;
    GLUquadric* m_gluQuadricParamsCenter ;
    // @}


    /**
      * @name To display a message in rendering area & Co.
      * @{ */
    QList<QString> m_displayList ;
    QPoint m_displayPos ;
    bool m_displayMsg ;
    int m_displayTime ;
    QPoint m_cursorPos ;
    // @}


    /**
      * @name To calcul distance and diedre
      * @{ */
    bool m_isCalculDistDiedre ;
    int m_nbAtomForDistDiedre ;
    QList<QPointer<Atom> > m_atomForDistDiedre ;

    Vector3d m_vector[3] ;
    double m_angle[2] ;
    double m_dihedral ;
    // Need to store the previous values of all variables in order to only send
    // an event to the information pane once
    double m_lastMeasurement[6] ;
    // @}

    /**
      * @name To display distance and diedre
      * @{ */
    //bool m_initDisplayDistDiedre ;
    int m_nbHPixelDist, m_nbHPixelAngle, m_nbHPixelDihedral ;
    int m_nbVPixelDist, m_nbVPixelAngle, m_nbVPixelDihedral ;
    // @}

    /**
      * @name Fonts definition used in the plugin.
      * @{ */
    float m_ratioFontSize ; //< The user can modify this ratio to resize the messages.
    QFont m_fontInfo ;
    QFont m_fontError ;
    QFont m_fontDistDiedreAtom ;
    QFont m_fontDistDiedreInfo ;
    // @}

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
  };

  /**
   * @class WmToolFactory wmtool.h
   * @brief Factory class to create instances of the WmTool class.
   */
  class WmToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(WmTool)
  };

} // end namespace Avogadro

#endif
