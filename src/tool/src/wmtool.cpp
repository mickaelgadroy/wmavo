
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

void checkModelviewProjectionMatrix()
{
  GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
  GLdouble projection[16] ; // Where The 16 Doubles Of The Projection Matrix Are To Be Stored
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix
  glGetDoublev( GL_PROJECTION_MATRIX, projection ) ; // Retrieve The Projection Matrix
  printf( "modelview: " ) ;
  for( int i=0 ; i<16 ; i++ )
    printf( " %.3lf", modelview[i] ) ;
  printf( "\n" ) ;
  printf( "projection:" ) ;
  for( int i=0 ; i<16 ; i++ )
    printf( " %.3lf", projection[i] ) ;
  printf( "\n" ) ;
}

namespace Avogadro
{

  const double WmTool::m_PI=(double)103993/(double)33102 ;
  const double WmTool::m_PI180=m_PI/(double)180 ;
  const double WmTool::m_180PI=(double)180/m_PI ;

  const QString WmTool::m_angstromStr=QString::fromUtf8( " Å" ) ;
  const QString WmTool::m_degreeStr=QString::fromUtf8( "°" ) ;
  
  WmTool::WmTool(QObject *parent) :
      Tool(parent),
      m_settingsWidget(NULL),/* m_addHydrogensCheck(NULL),*/

      m_widget(NULL), m_wPainter(NULL), m_wmExt(NULL),
      m_wmIsConnected(false), m_wmNbDots(0), m_wmNbSources(0), m_wmDistance(0),
      m_atomicNumberCur( WMEX_CREATEDATOMDEFAULT ),

      m_rectPos1(QPoint(0,0)), m_rectPos2(QPoint(0,0)), m_activeRect(false),

      m_drawBeginAtom(false), m_drawEndAtom(false), m_drawBond(false),
      m_beginAtom(Vector3d(0,0,0)), m_endAtom(Vector3d(0,0,0)),
      m_isCalculDistDiedre(false), m_nbAtomForDistDiedre(0),

      /*m_initDisplayDistDiedre(false),*/
      m_nbHPixelDist(WMTOOL_SPACING_LEFT_WORDGRP ), m_nbHPixelAngle(WMTOOL_SPACING_LEFT_WORDGRP),
      m_nbHPixelDihedral(WMTOOL_SPACING_LEFT_WORDGRP), m_nbVPixelDist(WMTOOL_SPACING_DOWN_WORDGRP),
      m_nbVPixelAngle(0), m_nbVPixelDihedral(0),

      m_ratioFontSize(WMTOOL_POINTSIZE_RATIO_DEFAULT)
  {
    m_fontInfo.setFamily( WMTOOL_FONT_FAMILY_INFO ) ;
    m_fontInfo.setPointSizeF( WMTOOL_FONT_POINTSIZE_INFO ) ;
    m_fontInfo.setWeight( WMTOOL_FONT_WEIGHT_INFO ) ;

    m_fontError.setFamily( WMTOOL_FONT_FAMILY_ERROR ) ;
    m_fontError.setPointSizeF( WMTOOL_FONT_POINTSIZE_ERROR ) ;
    m_fontError.setWeight( WMTOOL_FONT_WEIGHT_ERROR ) ;

    m_fontDistDiedreInfo.setFamily( WMTOOL_FONT_FAMILY_DISTDIEDREINFO ) ;
    m_fontDistDiedreInfo.setPointSizeF( WMTOOL_FONT_POINTSIZE_DISTDIEDREINFO ) ;
    m_fontDistDiedreInfo.setWeight( WMTOOL_FONT_WEIGHT_DISTDIEDREINFO ) ;

    m_fontDistDiedreAtom.setFamily( WMTOOL_FONT_FAMILY_DISTDIEDREATOM ) ;
    m_fontDistDiedreAtom.setPointSizeF( WMTOOL_FONT_POINTSIZE_DISTDIEDREATOM ) ;
    m_fontDistDiedreAtom.setWeight( WMTOOL_FONT_WEIGHT_DISTDIEDREATOM ) ;

    m_gluQuadricParams = gluNewQuadric() ;
    gluQuadricDrawStyle( m_gluQuadricParams, GLU_LINE ) ;

    m_gluQuadricParamsCenter = gluNewQuadric() ;
    gluQuadricDrawStyle( m_gluQuadricParamsCenter, GLU_FILL ) ;

    for( int i=0 ; i<6 ; i++ )
      m_lastMeasurement[i] = 0.0 ;

    QAction *action = activateAction() ;
    action->setIcon(QIcon(QString::fromUtf8(":/wmtool/wiimote.png"))) ;
    action->setToolTip(tr("Render Tool")) ;
    //action->setShortcut(Qt::Key_F1) ;

    m_time.start() ;
  }

  WmTool::~WmTool()
  {
    gluDeleteQuadric( m_gluQuadricParams ) ;
    gluDeleteQuadric( m_gluQuadricParamsCenter ) ;
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
    cout << "WmTool::mousewheelEvent" << endl ;
    return 0 ;
  }

  QUndoCommand* WmTool::keyPressEvent(GLWidget *, QKeyEvent *)
  {
    /* Deprecated :
    // If more than 2 keys are presses ?

    int ret=0 ;

    switch ( event->key() )
    {
    case Qt::Key_2 :
      ret = 2 ; break ;
    case Qt::Key_3 :
      ret = 3 ; break ;
    case Qt::Key_4 :
      ret = 4 ; break ;
    case Qt::Key_0 :
      ret = -1 ; break ;
    default :
      ret = 0 ;
    }

    setCalculDistDiedre( ret ) ;
    */

    return 0;
  }

  QUndoCommand* WmTool::keyReleaseEvent(GLWidget *, QKeyEvent *){ return 0; }

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
    {
      m_settingsWidget = new QWidget ;

      QLabel *lblWmSensitive = new QLabel(tr("Wiimote sensitive :")) ;
      QLabel *lblWmSMoins = new QLabel(tr("(-)")) ;
      QLabel *lblWmSPlus = new QLabel(tr("(+)")) ;
      m_wmSensitiveSlider = new QSlider( Qt::Horizontal ) ;
      m_wmSensitiveSlider->setMaximum( PLUGIN_WM_SENSITIVE_MAX ) ;
      m_wmSensitiveSlider->setMinimum( PLUGIN_WM_SENSITIVE_MIN ) ;
      m_wmSensitiveSlider->setValue( PLUGIN_WM_SENSITIVE_DEFAULT ) ;
      m_wmSensitiveSlider->setTickInterval( 1 ) ;
      //m_addHydrogensCheck = new QCheckBox( "Adjust Hydrogen" ) ;

      QLabel *lblWmPointSize = new QLabel(tr("Font Size :")) ;
      QLabel *lblWmMoins = new QLabel(tr("(-)")) ;
      QLabel *lblWmPlus = new QLabel(tr("(+)")) ;
      m_wmPointSizeFontSlider = new QSlider( Qt::Horizontal ) ;
      m_wmPointSizeFontSlider->setMaximum( (int)(WMTOOL_POINTSIZE_RATIO_MAX * 10.0f) ) ;
      m_wmPointSizeFontSlider->setMinimum( (int)(WMTOOL_POINTSIZE_RATIO_MIN * 10.0f) ) ;
      m_wmPointSizeFontSlider->setValue( (int)(WMTOOL_POINTSIZE_RATIO_DEFAULT * 10.0f) ) ;
      m_wmPointSizeFontSlider->setTickInterval( 1 ) ;

      QLabel *lblVibration = new QLabel(tr("Vibration :")) ;
      m_checkBoxActivateVibration = new QCheckBox( "ON/OFF" ) ;
      m_checkBoxActivateVibration->setChecked(PLUGIN_WM_VIBRATION_ONOFF) ;


      QVBoxLayout *vBoxSens=new QVBoxLayout() ;
      QHBoxLayout *hBoxSens=new QHBoxLayout() ;
      hBoxSens->addWidget( lblWmSMoins ) ;
      hBoxSens->addWidget( m_wmSensitiveSlider ) ;
      hBoxSens->addWidget( lblWmSPlus ) ;
      vBoxSens->addWidget( lblWmSensitive ) ;
      vBoxSens->addLayout( hBoxSens ) ;
      vBoxSens->addStretch( 1 ) ;

      QVBoxLayout *vBoxPointSize=new QVBoxLayout() ;
      QHBoxLayout *hBoxPointSize=new QHBoxLayout() ;
      hBoxPointSize->addWidget( lblWmMoins ) ;
      hBoxPointSize->addWidget( m_wmPointSizeFontSlider ) ;
      hBoxPointSize->addWidget( lblWmPlus ) ;
      vBoxPointSize->addWidget( lblWmPointSize ) ;
      vBoxPointSize->addLayout( hBoxPointSize ) ;
      vBoxPointSize->addStretch( 1 ) ;

      QHBoxLayout *hBoxVibration=new QHBoxLayout() ;
      hBoxVibration->addWidget( lblVibration ) ;
      hBoxVibration->addWidget( m_checkBoxActivateVibration ) ;


      QVBoxLayout *vBox=new QVBoxLayout() ;
      vBox->addLayout( vBoxSens ) ;
      vBox->addSpacing( 30 ) ;
      vBox->addLayout( vBoxPointSize ) ;
      vBox->addSpacing( 30 ) ;
      vBox->addLayout( hBoxVibration ) ;
      vBox->addStretch( 1 ) ;

      m_settingsWidget->setLayout( vBox ) ;

      // Signals connected in wmEx class by the rederected signal ...
      bool isConnect = connect( m_wmSensitiveSlider,  SIGNAL(valueChanged(int)),
                                this, SLOT(changedWmSensitiveRedirect(int)) ) ;
      if( !isConnect )
        qDebug() << "Problem connection signal : m_wmSensitiveSlider.valueChanged() -> wmTool.changedWmSensitiveRedirect() !!" ;

      isConnect = connect( m_wmPointSizeFontSlider,  SIGNAL(valueChanged(int)),
                           this, SLOT(setSizeRatioFont(int)) ) ;
      if( !isConnect )
        qDebug() << "Problem connection signal : m_wmPointSizeFontSlider.valueChanged() -> wmTool.setSizeRatioFont() !!" ;

      //isConnect = connect( m_addHydrogensCheck, SIGNAL(stateChanged(int)),
      //                     this, SLOT(adjustedHydrogenRedirect(int)) ) ;
      //if( !isConnect )
      //  qDebug() << "Problem connection signal : m_addHydrogensCheck.stateChanged() -> wmTool.adjustedHydrogenRedirect() !!" ;
    }

    return m_settingsWidget;
  }


  /**
    * The destructor of the "settings widget" object.
    */
  void WmTool::settingsWidgetDestroyed()
  {
    //m_settingsWidget = NULL ;
    delete m_settingsWidget ;
  }


  /**
    * The paint method of the tool which is used to paint any tool specific
    * visuals to the GLWidget.
    * @param widget The current GLWidget object used by Avogadro.
    */
  bool WmTool::paint( GLWidget *widget )
  {
    /*
    //widget->setFormat() ;
    QGLFormat fmt ;
    fmt.setStereo( true ) ;
    cout << "3D Stereo by QGLFormat:" << fmt.stereo() << endl ;

    GLboolean hasStereo[1] ;
    glGetBooleanv(GL_STEREO, hasStereo) ;
    cout << "3D Stereo by glGetBooleanv:" << hasStereo[0] << endl ;

    GLenum errCode ;
    const GLubyte *errString ;
    if( (errCode=glGetError()) != GL_NO_ERROR )
    {
      errString = gluErrorString( errCode ) ;
      fprintf (stderr, "OpenGL Error: %s\n", errString);
    }
    */

    /*
    QGLFormat fmt=widget->format() ;
    fmt.setStereo(true) ;
    widget->setFormat(fmt) ;

    if( !widget->format().stereo() )
    {
      // ok, goggles off
      if (!widget->format().hasOverlay())
      {
        qFatal("Cool hardware required");
      }
    }

    cout << "C'est passé ..." << endl ;
    */

    //printf( ":\n" ) ;
    //checkModelviewProjectionMatrix() ;
    
    if( m_widget == NULL )
    {
      m_widget = widget ;
      m_wPainter = m_widget->painter() ;

      // Initiate my projection matrix.
      GLdouble fovy=m_widget->camera()->angleOfViewY() ;
      GLdouble aspect=m_widget->width()/m_widget->height() ;
      GLdouble nearPlan=1.0 ;
      GLdouble farPlan=100.0 ;

      glMatrixMode( GL_PROJECTION ) ;
      glPushMatrix() ;
        glLoadIdentity() ;
        gluPerspective( fovy, aspect, nearPlan, farPlan ) ;
        glGetFloatv( GL_MODELVIEW_MATRIX, m_projectionMatrix ) ;
      glPopMatrix() ;

      // Normaly, useless ...
      //ContextMenuEater *cm=new ContextMenuEater( this, m_widget ) ;
      //m_widget->installEventFilter( cm ) ;
    }

    //displayTextMethods() ;
    //displayAtomicNumberCurrent() ;

    displayMsgInfo() ;
    displayInfo() ;
    
    if( !widget->hasFocus() )
      widget->setFocus() ;

    // Paint distance, diedre & dihedral.
    if( m_isCalculDistDiedre )
      paintDistDiedre() ;

    // Paint atoms & bond being created.
    if( m_drawBeginAtom || m_drawEndAtom || m_drawBond )
      drawBondAtom() ;

    // Paint the center (0,0,0) of OpenGL coord.
    drawCenter() ;
    //drawBarycenter() ;

    // Paint a selection rectangle (it is the last feature to realize the good effects of transparency).
    if( m_activeRect )
      drawRect( m_rectPos1, m_rectPos2 ) ;

    drawCursor() ;

    return true ;
  }


  /**
    * Draw a rectangle in the render zone.
    * @param p1 The up/left position of the rectangle.
    * @param p2 The bottom/right position of the rectangle.
    */
  void WmTool::drawRect( QPoint p1, QPoint p2, int r, int g, int b, int a )
  {
    drawRect( (float)p1.x(), (float)p1.y(), (float)p2.x(), (float)p2.y(), r, g, b, a ) ;
  }


  /**
    * Draw a rectangle in the render zone.
    * @param sx The up/left position in x of the rectangle.
    * @param sy The up/left position in y of the rectangle.
    * @param ex The bottom/right position in x of the rectangle.
    * @param ey The bottom/right position in y of the rectangle.
    */
  void WmTool::drawRect( float sx, float sy, float ex, float ey, int r, int g, int b, int a )
  {
    // - Inverse color : -1, -1, -1
    // - Wanted color : >=0, >=0, >=0

    int rBg=0, gBg=0, bBg=0, aBg=0 ;
    int rLimit=0, gLimit=0, bLimit=0 ;
    float rBgF=0, gBgF=0, bBgF=0, aBgF=0 ;
    if( !(r>=0 && r<=255 && g>=0 && g<=255 && b>=0 && b<=255 && a>=0 && a<=255) )
    { // Calculate the inverse color.

      QColor bgColor=m_widget->background() ;
      
      rBgF = 1.0f - (float)bgColor.redF() ;
      gBgF = 1.0f - (float)bgColor.greenF() ;
      bBgF = 1.0f - (float)bgColor.blueF() ;
      aBgF = 0.4f ;

      //printf( "%d %d %d - %d %d %d\n", rBg, gBg, bBg, bgColor.red(), bgColor.green(), bgColor.blue() ) ;
      //printf( "%f %f %f - %f %f %f\n", rBgF, gBgF, bBgF, bgColor.redF(), bgColor.greenF(), bgColor.blueF() ) ;
    }
    else
    {
      rBg=r ; bBg=b ; gBg=g ; aBg=a ;
      rBgF=(float)r/255.0f ; bBgF=(float)b/255.0f ; 
      gBgF=(float)g/255.0f ; aBgF=(float)a/255.0f ;
    }

    rLimit = (int)(float(rBg) / 2.0f) ;
    gLimit = (int)(float(gBg) / 2.0f) ;
    bLimit = (int)(float(bBg) / 2.0f) ;

    glPushMatrix();
    glLoadIdentity();

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    GLdouble startPos[3];
    GLdouble endPos[3];

    gluUnProject(float(sx), viewport[3] - float(sy), 0.1, modelview, projection, viewport, &startPos[0], &startPos[1], &startPos[2]);
    gluUnProject(float(ex), viewport[3] - float(ey), 0.1, modelview, projection, viewport, &endPos[0], &endPos[1], &endPos[2]);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    //glColor4f(1.0f, 0.5f, 0.5f, 0.4f);
    glColor4f( rBgF, gBgF, bBgF, aBgF ) ;
    //printf( "%f %f %f %f - %d %d %d %d\n", rBgF, gBgF, bBgF, aBgF, rBg, gBg, bBg, aBg ) ;

    glBegin(GL_POLYGON);
      glVertex3d(startPos[0],startPos[1],startPos[2]);
      glVertex3d(startPos[0],endPos[1],startPos[2]);
      glVertex3d(endPos[0],endPos[1],startPos[2]);
      glVertex3d(endPos[0],startPos[1],startPos[2]);
    glEnd();

    startPos[2] += 0.0001;
    glDisable(GL_BLEND);

    glColor3i( rLimit, gLimit, bLimit ) ;
    glBegin(GL_LINE_LOOP);
      glVertex3d(startPos[0],startPos[1],startPos[2]);
      glVertex3d(startPos[0],endPos[1],startPos[2]);
      glVertex3d(endPos[0],endPos[1],startPos[2]);
      glVertex3d(endPos[0],startPos[1],startPos[2]);
    glEnd();

    glPopMatrix();
    glPopAttrib();
    glPopMatrix();
  }


  /**
    * Quick render an atom in the render zone.
    * @param radius Radius of the atom.
    * @param from The position of the atom.
    */
  void WmTool::drawAtom( float radius, const Vector3d& from )
  {
    // Init & Save.
    glPushMatrix() ;
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

    // If active, do not paint the atom !!!
    // glLoadIdentity();

    // If no active, do not change color !!!
    glDisable(GL_LIGHTING) ;

    // Blue.
    glColor3f( 0.0, 0.0, 1.0 ) ;

    glTranslated( from[0], from[1], from[2] ) ;
    glRotatef( 22, 1, 0 , 0 ) ; // Just not to see the sphere at the bottom ...

    //GLUquadric* params=gluNewQuadric() ;
    //gluQuadricDrawStyle( params, GLU_LINE ) ;
    gluSphere( m_gluQuadricParams, radius, 10, 10) ;
    // params : GLUquadric = options
    // 0.75 : radius : rayon de la sphère.
    // 20 : slices : lontitudes.
    // 20 : stacks : latitudes.

    // Delete & Redo in initial state.
    //gluDeleteQuadric( params ) ;
    glPopAttrib();
    glPopMatrix() ;
  }


  /**
    * Draw a circle of the 0-origin of the OpenGL universe.
    */
  void WmTool::drawCenter()
  {
    /*
    GLdouble pos3D_x, pos3D_y, pos3D_z;

    // Arrays to hold matrix information.
    GLdouble model_view[16] ;
    glGetDoublev(GL_MODELVIEW_MATRIX, model_view) ;

    GLdouble projection[16] ;
    glGetDoublev(GL_PROJECTION_MATRIX, projection) ;

    GLint viewport[4] ;
    glGetIntegerv(GL_VIEWPORT, viewport) ;

    GLint params[4] ;
    glGetIntegerv( GL_VIEWPORT, params ) ;
    GLdouble width=params[2] ;
    GLdouble height=params[3] ;

    // Get 2D coordinates based on opengl coordinates.
    gluProject( width/2, height/2, 0.01,
        model_view, projection, viewport,
        &pos3D_x, &pos3D_y, &pos3D_z ) ;
    */

    // Init & Save.
    glPushMatrix() ;
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

    // If active, do not paint the atom !!!
    // glLoadIdentity();

    // If no active, do not change color !!!
    glDisable(GL_LIGHTING) ;

    // Black.
    glColor3f( 0.2f, 1.0f,  0.2f ) ;

    //glTranslatef( 0, 0 , 0 ) ;
    glRotatef( 22, 1, 0 , 0 ) ; // Just not to see the sphere at the bottom ...

    gluSphere( m_gluQuadricParamsCenter, 0.1, 5, 5) ;

    glPopAttrib() ;
    glPopMatrix() ;
  }


  /**
    * Draw a circle of the barycenter of the molecule.
    */
  void WmTool::drawBarycenter()
  {

    // Voir pour ajouter des cercles pour le barycentre des masses des atomes et
    // autres trucs sortant d'un atome.

    // Init & Save.
    glPushMatrix() ;
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

    // If active, do not paint the atom !!!
    // glLoadIdentity();

    // If no active, do not change color !!!
    glDisable(GL_LIGHTING) ;

    glColor3f( 0.2f, 1.0f,  0.2f ) ;

    //Vector3d b=m_wmExt->getPointRef() ;
    //glTranslatef( b[0], b[1] , b[2] ) ;

    gluSphere( m_gluQuadricParamsCenter, 0.1, 5, 5) ;

    glPopAttrib() ;
    glPopMatrix() ;
  }


  void WmTool::drawCursor()
  {
    glPushMatrix() ; //1 modelview

    // Try to fix the size which change according to the camera or the movement of a atom ...

    // Idea 1 : Load the matrix modelview of the camera 
    // Not solve : the objects are positioned differently and are not face to face at the camera.
    //Eigen::Transform3d modelviewTransf=m_widget->camera()->modelview() ;
    //float modelviewTab[16]= { modelviewTransf(0,0), modelviewTransf(0,1), modelviewTransf(0,2),
    //                          modelviewTransf(1,0), modelviewTransf(1,1), modelviewTransf(1,2),
    //                          modelviewTransf(2,0), modelviewTransf(2,1), modelviewTransf(2,2),
    //                          modelviewTransf(3,0), modelviewTransf(3,1), modelviewTransf(3,2) } ;                        
    //glMatrixMode(GL_MODELVIEW) ;
    //glLoadMatrixf( modelviewTab ) ;

    // Idea 2 : verify the modelview/projection matrix.
    // After check the state of the modelview/projection matrix, we can see that the projection matrix
    // is always recalculated when there is a movement on the z axis (object or camera).
    // This is a problem when we want draw an objet calculated by the right, up and direction vectors 
    // (get by the modelview). The problem is thata the projection does not render the same visual distance ...

    // Idea 3 : Load my projection matrix.
    glMatrixMode( GL_PROJECTION ) ;
    glPushMatrix() ; //2 projection
    glLoadMatrixf( m_projectionMatrix ) ;
    glMatrixMode( GL_MODELVIEW ) ;


    GLint viewport[4] ;       // Where The Viewport Values Will Be Stored
    GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
    GLdouble projection[16] ; // Where The 16 Doubles Of The Projection Matrix Are To Be Stored

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix
    glGetDoublev( GL_PROJECTION_MATRIX, projection ) ; // Retrieve The Projection Matrix
    glGetIntegerv( GL_VIEWPORT, viewport ) ;           // Retrieves The Viewport Values (X, Y, Width, Height)

    GLdouble rightX=modelview[0] ;
    GLdouble rightY=modelview[4] ;
    GLdouble rightZ=modelview[8] ;

    GLdouble upX=modelview[1] ;
    GLdouble upY=modelview[5] ;
    GLdouble upZ=modelview[9] ;

    GLdouble directionX=-modelview[2] ;
    GLdouble directionY=-modelview[6] ;
    GLdouble directionZ=-modelview[10] ;

    QPoint p=m_widget->mapFromGlobal(m_cursorPos) ;

    GLfloat winX=(float)p.x() ;
    GLfloat winY=(float)viewport[3] - (float)p.y() ;
    GLfloat winZ=0 ;
    GLdouble posX=0, posY=0, posZ=0 ;

    // Get the 1st intersection with the openGL object.
    //glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    // Widget position to openGL position.
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);


    // Draw section.
    glPushAttrib(GL_ALL_ATTRIB_BITS) ; //3 attrib
    glPushMatrix() ; //4 modelview
    
    //Will be transparent
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    //glDisable(GL_LIGHTING);
    //glDisable(GL_CULL_FACE);

    //Draw the cursor at the current mouse pos
    
    float r=0.02 ;
    float a=0, b=0 ;

    //glBegin( GL_TRIANGLES ) ;
    //    glColor4f( 0.75, 0.75, 0.75, 0.75 ) ;
    //    glVertex3d( posX, posY, posZ ) ;
    //    glVertex3d( posX+r*rightX, posY+r*rightY, posZ+r*rightZ ) ;
    //    glVertex3d( posX+r*upX, posY+r*upY, posZ+r*upZ ) ;
    //glEnd() ;

    // avoir right et up comme pour le triangle
    glBegin( GL_LINE_LOOP );
    for( float i=0 ; i<2*M_PI ; i+=(float)M_PI/10.0f )
    {
        a = r * cosf(i) ;
        b = r * sinf(i) ;
        float xf = posX + rightX * a + upX * b + -1.0f*directionX ;
        float yf = posY + rightY * a + upY * b + -1.0f*directionY ;
        float zf = posZ + rightZ * a + upZ * b + -1.0f*directionZ ;
        glVertex3f( xf, yf, zf ) ;
    }
    glEnd() ;

    glBegin( GL_LINE_LOOP );
    for( float i=0 ; i<2*M_PI ; i+=(float)M_PI/10.0f )
    {
        a = r * cosf(i) ;
        b = r * sinf(i) ;
        float xf = posX + 1*a + 0*b + -1.0f*directionX ;
        float yf = posY + 0*a + 1*b + -1.0f*directionY ;
        float zf = posZ + 1*a + 1*b + -1.0f*directionZ ;
        glVertex3f( xf, yf, zf ) ;
    }
    glEnd();
    
    glPopMatrix() ; //4 modelview
    glPopAttrib() ; //3 attrib

    glMatrixMode( GL_PROJECTION ) ;
    glPopMatrix() ; //2 projection
    glMatrixMode( GL_MODELVIEW ) ;

    glPopMatrix() ; //1 modelview
  }


  /**
    * Quick render the "bond" between 2 positions.
    * @param begin 1st end of the "bond".
    * @param end 2nd end of the "bond".
    */
  void WmTool::drawBond2( const Vector3d& begin, const Vector3d& end )
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // If active, do not paint the atom !!!
    // glLoadIdentity();

    // If no active, do not change color !!!
    glDisable( GL_LIGHTING ) ;

    glLineWidth( 8.0F ) ;

    glBegin( GL_LINES ) ;
      glColor3f( 0.0f, 0.0f, 0.0f ) ;
      glVertex3d( begin[0], begin[1], begin[2] ) ;
      glColor3f( 0.0f, 0.0f, 0.5f ) ;
      glVertex3d( end[0], end[1], end[2] ) ;
    glEnd() ;

    glPopAttrib() ;
  }

/*
  void WmTool::drawBond( float radius, const Vector3d& begin, const Vector3d& end )
  {

    // The position of the bond is calculated in 2 steps :

    // A = begin ; B = end

    // 1) Calculate angle of rotation between (begin) and (end) from (begin) on (X,Y) and (Y,Z) :
    //  a.In plan (X,Y), get a right triangle A1B1C, right in C :
    //    - A1 = [xA,yA,0]
    //    - B1 = [xB,yB,0]
    //    - C = [xA1,yB1,0] ;
    //    Calculte the angle A1 = acos( A1C / A1B1 ) .
    //  b.In plan (Y,Z), get a right triangle A2B2D, right in D :
    //    - A2 = begin = [0,yA,zA]
    //    - B2 = end = [0,yB,zB]
    //    - C = [0,yA2,zB2] ;
    //    Calculte the angle Â2 = acos( A2D / A2B2 ) .

    // 2) Calculate distance between (begin) and (end) to get height of the cylinder :
    //    ||AB|| = sqrt[(xA-xB)²+(yA-yB)²+(zA-zB)²]


    cout << "end:" << endl << end << endl ;

    /// 1) a. angle Â1
    Vector3d C(end[0], begin[1], 0) ;

    // ||A1C||
    double a=(begin[0]-C[0]) * (begin[0]-C[0]) ;
    double b=(begin[1]-C[1]) * (begin[1]-C[1]) ;
    double distA1C=sqrt(a+b) ;
    //qDebug() << "distA1C:" << distA1C ;

    // ||A1B1||
    a = (begin[0]-end[0]) * (begin[0]-end[0]) ;
    b = (begin[1]-end[1]) * (begin[1]-end[1]) ;
    double distA1B1=sqrt(a+b) ;
    //qDebug() << "distA1B1:" << distA1B1 ;

    // angle Â1
    double rotZ=acos(distA1C/distA1B1) * m_180PI ;
    //qDebug() << "distA1C/distA1B1:" << distA1C/distA1B1 ;
    //qDebug() << "acos(distA1C/distA1B1):" << acos(distA1C/distA1B1) ;
    qDebug() << "rz:" << rotZ ;
*/

    /* Test
    /// Put the values in the right quarts.
    // In (X,Y) :
    if( begin[0]<=end[0] && begin[1]<=end[1] )
    { // A (Personnal notation).
      //qDebug() << "A" ;
      rotZ = 90 + (90-rotZ) ;
    }
    else if( begin[0]<=end[0] && begin[1]>=end[1] )
    { // B
      //qDebug() << "B" ;
      //rotZ = rotZ ;
    }
    else if( begin[0]>=end[0] && begin[1]>=end[1] )
    { // C
      //qDebug() << "C" ;
      rotZ = -rotZ ;
    }
    else //if( begin[0]>=end[0] && begin[1]<=end[1] )
    { // D
      //qDebug() << "D" ;
      rotZ = rotZ + 180 ;
    }
    qDebug() << "A1:2:" << rotZ ;
    */

/*
    // b. angle Â2
    Vector3d D( 0, begin[1], end[2] ) ;

    // ||A2D||
    a = (begin[1]-D[1]) * (begin[1]-D[1]) ;
    b = (begin[2]-D[2]) * (begin[2]-D[2]) ;
    double distA2D=sqrt(a+b) ;

    // ||A2B2||
    a = (begin[1]-end[1]) * (begin[1]-end[1]) ;
    b = (begin[2]-end[2]) * (begin[2]-end[2]) ;
    double distA2B2=sqrt(a+b) ;

    // angle Â2
    double rotX=acos(distA2D/distA2B2)*m_180PI ;
    //rotY *= m_180PI ;
    qDebug() << "rx:" << rotX ;
*/
    /* Test
    // Put the values in the right quarts, in (Y,Z) :
    if( begin[2]<=end[2] && begin[1]<=end[1] )
    {
      qDebug() << "A" ;
      //rotY = 90 + (90-rotY) ;
      //rotY = rotY + 180 ;
      //rotY = -rotY ;
      rotY = rotY ;
    }
    else if( begin[2]<=end[2] && begin[1]>=end[1] )
    {
      qDebug() << "B" ;
      rotY = rotY ;
    }
    else if( begin[2]>=end[2] && begin[1]>=end[1] )
    {
      qDebug() << "C" ;
      //rotY = -rotY ;
      //rotY = rotY + 180 ;
      //rotY = rotY ;
      rotY = 90 + (90-rotY) ;
    }
    else //if( begin[2]>=end[2] && begin[1]<=end[1] )
    {
      qDebug() << "D" ;
      //rotY = rotY + 180 ;
      //rotY = -rotY ;
      rotY = 90 + (90-rotY) ;
    }
    qDebug() << "A2:2:" << rotY ;
    */

/*
    // c. angle Â3
    Vector3d E( end[0], 0, begin[2] ) ;

    // ||A3E||
    a = (begin[0]-E[0]) * (begin[0]-E[0]) ;
    b = (begin[2]-E[2]) * (begin[2]-E[2]) ;
    double distA3E=sqrt(a+b) ;

    // ||A3B3||
    a = (begin[0]-end[0]) * (begin[0]-end[0]) ;
    b = (begin[2]-end[2]) * (begin[2]-end[2]) ;
    double distA3B3=sqrt(a+b) ;

    // angle Â3
    double rotY=acos(distA3E/distA3B3)*m_180PI ;
    //rotX *= m_180PI ;
    qDebug() << "ry:" << rotY ;
*/

    /* Test
    // Put the values in the right quarts, in (X,Z) :
    if( begin[2]<=end[2] && begin[0]<=end[0] )
    {
      qDebug() << "A" ;
      //rotX = 90 + (90-rotX) ;
      //rotX = -rotX ;
      //rotX = rotX + 180 ;
      //rotX = rotX ;
      rotX = 90 - rotX ;
    }
    else if( begin[2]<=end[2] && begin[0]>=end[0] )
    {
      qDebug() << "B" ;
      //rotX = 90 + (90-rotX) ; // 90
      //rotX = -rotX ; // -90
      //rotX = rotX + 180 ; // 270
      //rotX = rotX ; // 90
      rotX = 90 - rotX ;
    }
    else if( begin[2]>=end[2] && begin[0]>=end[0] )
    {
      qDebug() << "C" ;
      //rotX = 90 + (90-rotX) ;
      //rotX = -rotX ;
      //rotX = rotX + 180 ;
      //rotX = rotX ;
      rotX = 90 - rotX ;
    }
    else //if( begin[2]>=end[2] && begin[0]<=end[0] )
    {
      qDebug() << "D" ;
      //rotX = 90 + (90-rotX) ;
      //rotX = -rotX ;
      //rotX = rotX + 180 ;
      //rotX = rotX ;
      rotX = 90 - rotX ;
    }
    qDebug() << "A3:2:" << rotX ;
    */

    /* Test
    if( begin[0] >= end[0] )
      rotZ = -rotZ ;

    if( begin[1] >= end[1] )
      rotX = -rotX ;

    if( begin[2] >= end[2] )
      rotY = -rotY ;
      */
/*
    /// 2) ||AB|| = sqrt[(Xa-Xb)²+(Ya-Yb)²+(Za-Zb)²]
    a = (begin[0]-end[0]) * (begin[0]-end[0]) ;
    b = (begin[1]-end[1]) * (begin[1]-end[1]) ;
    double c=(begin[2]-end[2]) * (begin[2]-end[2]) ;
    double distAB=sqrt(a+b+c) ;
    //qDebug() << "distAB:" << distAB ;


    // Init & Save.
    glPushMatrix() ;
    glTranslatef( begin[0], begin[1], begin[2] ) ;
    glRotatef( rotZ, 0, 0, 1 ) ;
    glRotatef( rotY, 0, 1, 0 ) ;
    glRotatef( rotX, 1, 0, 0 ) ;

    //GLUquadric* params=gluNewQuadric() ;
    //gluQuadricDrawStyle( params, GLU_LINE ) ;
    gluCylinder( m_gluQuadricParams, radius, radius, distAB, 20, 20 ) ;

    // Delete & Redo in initial state.
    //gluDeleteQuadric( params ) ;
    glPopMatrix() ;
  }
  */


  /**
    * Quick render the atom and the bond currently in construction.
    */
  void WmTool::drawBondAtom()
  {
    // plan supplémentaire : voir glOrtho

    // glGet( enum , params)
    // void glGetIntegerv( GLenum pname, GLint *params ) ;

    // glGet( GL_PROJECTION )
    // Dans la matrice, à la pos(0,0), récup valeur ==> f / aspect ==> f ==> cotangent( fovy / 2 ) ==> fovy ==> arccotan(f) *2 ==> atan(1/f) * 2
    // sinon, mettre le fovy à 45°


    /* Pose des problèmes lors de l'affichage des objets openGL temporaires
       Et ne règle pas le problème des plans qui n'affichent qu'une certaine zone.
    GLint params[4] ;
    glGetIntegerv( GL_VIEWPORT, params ) ;

    GLenum errCode ;
    const GLubyte *errString ;
    if( (errCode=glGetError()) != GL_NO_ERROR )
    {
      errString = gluErrorString( errCode ) ;
      fprintf (stderr, "OpenGL Error: %s\n", errString);
    }
    //else
    //  qDebug() << "x:" << params[0] << "y:" << params[1] << "width:" << params[2] << "height:" << params[3] ;

    GLdouble width=params[2] ;
    GLdouble height=params[3] ;

    GLdouble aspect=width/height ; // 0
    GLdouble fovy=45.0 ;
    GLdouble near=1.0, far=1000.0 ; // 1.0, 100.0


    glViewport(0, 0, width, height) ;

    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity() ;
    gluPerspective(fovy, aspect, near, far) ;
    */

    if( m_drawBeginAtom )
      drawAtom( 0.4f, m_beginAtom ) ;

    if( m_drawEndAtom )
      drawAtom( 0.4f, m_endAtom ) ;

    if( m_drawBond )
      drawBond2( m_beginAtom, m_endAtom ) ;
  }


  /**
    * Display informations of the Wiimote in the render zone.
    */
  void WmTool::displayInfo()
  {
    if( m_wPainter!=NULL )
    {
      QFontMetrics fontMetric( m_fontInfo ) ;
      QString msg ;
      int border=5 ;
      int height=fontMetric.height() ; //lineSpacing() ;
      int xB=10, yB=10+height ;
      int nbL=0, iH=0 ;

      if( m_wmIsConnected )
        nbL = 4 ;
      else
        nbL = 2 ;
      
      drawRect( float(xB-border),
                float(yB-height),
                float(xB+fontMetric.width("Current atomic number : 160")+border), 
                float(yB+(nbL-1)*height+border) ) ;
      
      // 1
      msg = "Wiimote connected : " ;
      if( m_wmIsConnected ) msg += "YES" ;
      else msg += tr("NO") ; //, Press 1+2") ;
      displayMsgOnScreen( QPoint(xB,yB), msg, m_fontInfo, 1.0, 1.0, 1.0 ) ;
      iH++ ;

      // 2
      msg = tr("Current atomic number : ") + QString::number(m_atomicNumberCur) ;
      displayMsgOnScreen( QPoint(xB,yB+height*iH), msg, m_fontInfo, 1.0, 1.0, 1.0 ) ;
      iH ++ ;

      if( m_wmIsConnected )
      {
        float r,g,b ;

        msg = tr("Nb detected LEDs : ") + QString::number(m_wmNbDots) ;
        displayMsgOnScreen( QPoint(xB,yB+height*iH), msg, m_fontInfo, 1.0, 1.0, 1.0 ) ;
        iH++ ; 

        msg = tr("Nb detected sources : ") + QString::number(m_wmNbSources) ;
        if( m_wmNbSources == 1 ){ r=1.0; g=0.5; b=0.0; }
        else if( m_wmNbDots == 0 ){ r=1.0; g=0.0; b=0.0; }
        else{ r=1.0; g=1.0; b=1.0; }

        displayMsgOnScreen( QPoint(xB,yB+height*iH), msg, m_fontInfo, r, g, b ) ;
        iH++ ;

        //msg = tr("Distance : ") + QString::number(m_wmDistance) ;
        //m_wPainter->drawText( QPoint(10,80), msg ) ;

        // "Reset" color.
        //glColor3f( 1.0, 1.0, 1.0 ) ;
      }
    }
  }


  /**
    * Display a message in the render zone.
    */
  void WmTool::displayMsgInfo()
  {
    if( m_displayMsg && m_wPainter!=NULL )
    {
      int i=0 ;
      int width=0, height=0, tmp=0 ;
      int t2=m_time.elapsed() ;
      int spacing=20, border=5 ;

      if( (t2-m_displayTime) < WMTOOL_TIME_DISPLAY_MSG )
      {
        QFontMetrics fontMetric(m_fontError) ;
        height = fontMetric.lineSpacing() ;

        foreach( QString str, m_displayList )
        {
          i++ ;
          tmp = fontMetric.width( str ) ;
          if( tmp > width )
            width = tmp ;
        }

        drawRect( float(m_displayPos.x()-border), 
                  float(m_displayPos.y()-(border+fontMetric.lineSpacing())), 
                  float(m_displayPos.x()+width+border), float(m_displayPos.y()+i*height+border) ) ;

        i = 0 ;
        //glColor3f( 1.0, 0.0, 0.0 ) ;
        foreach( QString str, m_displayList )
        {
          //m_wPainter->drawText( QPoint(m_displayPos.x(), m_displayPos.y()+i), str ) ;
          if( i == 0 )
            displayMsgOnScreen(QPoint(m_displayPos.x(), m_displayPos.y()+i), str, m_fontError, 1, 0.5, 0 ) ;
          else
            displayMsgOnScreen(QPoint(m_displayPos.x(), m_displayPos.y()+i), str, m_fontError, 0, 0, 0 ) ;
          i += 20 ;
        }
        //glColor3f( 1.0, 1.0, 1.0 ) ;
      }
      else
      {
        m_displayMsg = false ;
      }
    }
  }


  /**
    * Display the current atomic number used for a new atom.
    */
  void WmTool::displayAtomicNumberCurrent()
  {
    /*
    if( m_wPainter != NULL )
    {
      QFontMetrics fontMetric( m_fontInfo ) ;
      QString str ;
      int border=0 ;

      str = tr("Current atomic number : ") + QString::number(m_atomicNumberCur) ;

      //drawRect( float(10-border), float(100-border), 
      //          float(fontMetric.width(str)+border), 
      //          float(fontMetric.lineSpacing()+border) ) ;
      
      //glColor3f( 1.0, 1.0, 1.0 ) ;
      //m_wPainter->drawText( QPoint(10,100), str ) ;
      displayMsgOnScreen( QPoint(x,y), str, m_fontInfo, 1.0, 1.0, 1.0 ) ;
    }
  */
  }



  //QSlider* WmTool::getSliderWmSensitive()
  //{
  //  return m_wmSensitiveSlider ;
  //}


  //QCheckBox* WmTool::getAddHydrogenCheck()
  //{
  //  return m_addHydrogensCheck ;
  //}


  /**
    * Help to redirect a signal to transmit an information about the sensitive of the Wiimote.
    * @param wmSens The new sensitive of the Wiimote.
    */
  void WmTool::changedWmSensitiveRedirect( int wmSens )
  {
    emit changedWmSensitive( wmSens ) ;
  }

  //void WmTool::adjustedHydrogenRedirect( int adjustH )
  //{
  //  emit adjustedHydrogen( adjustH ) ;
  //}


  /**
    * Set the current atomic number used for a new atom.
    * @param atomicNumber The new value of the current atomic number.
    */
  void WmTool::setAtomicNumberCurent( int atomicNumber )
  {
    m_atomicNumberCur = atomicNumber ;
  }


  /**
    * Set the informations of the Wiimote.
    * @param connect Wiimote connected ?
    * @param nbDots Number of IR dots detected by the Wiimote.
    * @param nbSources Number of "IR dots" really used detected by the Wiimote.
    * @param distance The "distance" calculated by the Wiimote.
    */
  void WmTool::setWmInfo( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance )
  {
    m_cursorPos = cursor ;
    m_wmIsConnected = connect ;
    m_wmNbDots = nbDots ;
    m_wmNbSources = nbSources ;
    m_wmDistance = distance ;
  }


  /**
    * Set the message which will be displayed in the render zone.
    * @param strList A QList of QString to diplay message by line (1 QString = 1 line)
    * @param pos The position of the message.
    */
  void WmTool::setDisplayMsg( QList<QString> strList, QPoint pos )
  {
    m_displayList = strList ;
    m_displayPos = pos ;
    m_displayMsg = true ;
    m_displayTime = m_time.elapsed() ;
  }


  /**
    * Initiate/(des)activate the rectangle in the render zone.
    * @param active Display or not the rectangle.
    * @param p1 The up/left position of the rectangle.
    * @param p2 The bottom/right position of the rectangle.
    */
  void WmTool::setActiveRect( bool active, QPoint p1, QPoint p2 )
  {
    m_activeRect = active ;
    m_rectPos1 = p1 ;
    m_rectPos2 = p2 ;
  }

  /**
    * Set the WmExtension object (a hand on the object, a shortcut).
    * @param wmExtens The WmExtension object instanciate by Avogadro.
    */
  void WmTool::setWmExt( Extension *wmExtension )
  {
    if( wmExtension != NULL )
      m_wmExt = wmExtension ;

    bool isConnect = connect( m_wmPointSizeFontSlider,  SIGNAL(valueChanged(int)),
                         m_wmExt, SLOT(setFontSizeContextMenu(int)) ) ;
    if( !isConnect )
      qDebug() << "Problem connection signal : m_wmPointSizeFontSlider.valueChanged() -> m_wmExt.setFontSizeContextMenu() !!" ;

    isConnect = connect( m_checkBoxActivateVibration,  SIGNAL(stateChanged(int)),
                         m_wmExt, SLOT(setActivatedVibration(int)) ) ;
    if( !isConnect )
      qDebug() << "Problem connection signal : m_checkBoxActivateVibration.stateChanged() -> m_wmExt.setActivatedVibration() !!" ;
  }


  /**
    * Set the position of the atoms to display them.
    * The name of the method is confused, but it more comprehensible when an other class
    * connects it to realize the desired goal : draw the atoms and bond.
    * @param beginAtom The position of the 1st atom to draw.
    * @param endAtom The position of the 2nd atom to draw.
    * @param drawBeginAtom A boolean to draw or not the 1st atom.
    * @param drawEndAtom A boolean to draw or not the 2nd atom.
    * @param drawBond A boolean to draw or not the bond.
    */
  void WmTool::renderAtomBond( const Vector3d& beginAtom, const Vector3d& endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond )
  {
    /*
    cout << "atom begin:[" << beginAtom[0] << ";" << beginAtom[1] << ";" << beginAtom[2]  << "]" << endl ;
    cout << "atom end:[" << endAtom[0] << ";" << endAtom[1] << ";" << endAtom[2]  << "]" << endl ;
    cout << "drawBeginAtom:" << drawBeginAtom << endl ;
    cout << "drawEndAtom:" << drawEndAtom << endl ;
    cout << "drawBond:" << drawBond << endl ;
    */

    m_beginAtom = beginAtom ;
    m_endAtom = endAtom ;
    m_drawBeginAtom = drawBeginAtom ;
    m_drawEndAtom = drawEndAtom ;
    m_drawBond = drawBond ;

    // The display is updated by wmextension->wmActions():
    // (isCreateAtom) updates the fake cursor.
  }


  /**
    * Initiate the need to calculate the distance & co between atoms.
    * @param what Number of desired atoms to realize all the calculations (2 : distance, 3 : angle&distance, 4 : diedre angle&angle&distance).
    * @see WmTool::calculDistDiedre( Atom *atom )
    */
  void WmTool::setCalculDistDiedre( int what )
  {
    if( what == -1 )
    { // Clear everything which looks like "calcul distance, diedre, dihedral".

      clearDistDiedre() ;
    }

    if( what>=2 && what<=4 )
    { // Init. or re-init. wmExtension & wmTool.

      clearDistDiedre() ;

      m_isCalculDistDiedre = true ;
      m_nbAtomForDistDiedre = what ;

      emit askDistDiedre() ;
    }
  }


  /**
    * Get the atoms to realize the "calculation of the distance, angle, diedre angle" feature.
    * @param atom An added atom in the calculation.
    */
  void WmTool::calculDistDiedre( Atom *atom )
  {
    if( m_isCalculDistDiedre && m_nbAtomForDistDiedre>0 )
    {
      int indexOfAtom=m_atomForDistDiedre.indexOf(atom) ;

      if( indexOfAtom != -1 )
      { // Delete the atom.

        m_atomForDistDiedre.removeAt(indexOfAtom) ;
        m_nbAtomForDistDiedre++ ;
      }
      else
      { // Add the atom.

        m_nbAtomForDistDiedre-- ;
        m_atomForDistDiedre.append( atom ) ;

        // Realize into paint().
        //if( m_atomForDistDiedre.size() >= 2 ) calculateParameters() ;

      }

      if( m_nbAtomForDistDiedre > 0 )
        emit askDistDiedre() ;
    }
  }


  /**
    * Erase all informations used by the "calculation of the distance, angle, diedre angle" feature.
    */
  void WmTool::clearDistDiedre()
  {
    m_atomForDistDiedre.clear() ;
    m_isCalculDistDiedre = false ;
    m_nbAtomForDistDiedre = 0 ;
    //m_initDisplayDistDiedre = false ;

    for( int i=0 ; i<6 ; i++ )
      m_lastMeasurement[i] = 0.0 ;
  }


  /**
    * Sub-method used to calculate all informations for the "calculation of the distance, angle, diedre angle" feature.
    */
  void WmTool::calculateParameters()
  {
    int i=0 ;
    double norm=0 ;
    QString distanceString, angleString, dihedralString ;

    // Check if no atom is erased.
    while( i < m_atomForDistDiedre.size() )
    {
      if( m_atomForDistDiedre[i].isNull() )
      {
        //cout << " Atome effacé" << endl ;
        m_atomForDistDiedre.removeAt(i) ;

        if( m_nbAtomForDistDiedre > 0 )
          m_nbAtomForDistDiedre ++ ;
        else
          clearDistDiedre() ;
      }
      else
        i++ ;
    }

    // Calculate all parameters and store them in member variables.

    if( m_atomForDistDiedre.size() >= 2 )
    { // Two atoms selected - distance measurement only

      m_vector[0] = *(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[0]->pos()) ;
      norm = m_vector[0].norm() ;

      // Check whether we have already sent this out...
      if( m_lastMeasurement[0] != norm ) // coeff(0)~=[0] : coef more performant.
      {
        m_lastMeasurement[0] = norm ;

        distanceString = tr("Distance (1->2): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
          .arg(norm)
          .arg(QString::fromUtf8("Å")) ;

        emit message(distanceString) ;
      }
    }

    if( m_atomForDistDiedre.size() >= 3 )
    { // Two distances and the angle between the three selected atoms

      m_vector[1] = *(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[2]->pos()) ;
      norm = m_vector[1].norm() ;

      // Calculate the angle between the atoms
      m_angle[0] = acos(m_vector[0].normalized().dot(m_vector[1].normalized()));
      m_angle[0] *= m_180PI ; // To degree.

      // Check whether we have already sent this out
      if( m_lastMeasurement[1] != norm )
      {
        m_lastMeasurement[1] = norm ;

        distanceString = tr("Distance (2->3): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
          .arg(norm)
          .arg(QString::fromUtf8("Å")) ;

        emit message(distanceString) ;
      }

      if( m_lastMeasurement[3] != m_angle[0] )
      {
        m_lastMeasurement[3] = m_angle[0] ;

        angleString = tr("Angle 1: %L1 °").arg(m_angle[0]) ;
        emit message(angleString) ;
      }
    }

    if( m_atomForDistDiedre.size() >= 4 )
    { // Three distances, bond angle and dihedral angle

      m_vector[2] = *m_atomForDistDiedre[2]->pos() - *m_atomForDistDiedre[3]->pos() ;
      norm = m_vector[2].norm() ;

      // Calculate the angle between the atoms.
      Vector3d v1=*(m_atomForDistDiedre[2]->pos()) - *(m_atomForDistDiedre[1]->pos()) ;
      Vector3d v2=*(m_atomForDistDiedre[2]->pos()) - *(m_atomForDistDiedre[3]->pos()) ;
      m_angle[1] = acos(v1.normalized().dot(v2.normalized()));
      m_angle[1] *= m_180PI ;

      /*
      double aSq=m_vector[1].norm()*m_vector[1].norm() ;
      double cSq=m_vector[2].norm()*m_vector[2].norm() ;


      Vector3d v=*(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[3]->pos()) ;
      double bSq=v.norm()*v.norm() ;

      cout << "norm:" << v.norm() << endl ;
      cout << "aSq:" << aSq << " bSq:" << bSq  << "cSq:" << cSq << endl ;

      m_angle[1] = acos( (cSq+aSq-bSq)/(2*cSq*aSq) ) ;
      m_angle[1] *= m_180PI ; // To degree.
      */

      m_dihedral=CalcTorsionAngle(
                        vector3(m_atomForDistDiedre[0]->pos()->x(),
                                m_atomForDistDiedre[0]->pos()->y(),
                                m_atomForDistDiedre[0]->pos()->z()),
                        vector3(m_atomForDistDiedre[1]->pos()->x(),
                                m_atomForDistDiedre[1]->pos()->y(),
                                m_atomForDistDiedre[1]->pos()->z()),
                        vector3(m_atomForDistDiedre[2]->pos()->x(),
                                m_atomForDistDiedre[2]->pos()->y(),
                                m_atomForDistDiedre[2]->pos()->z()),
                        vector3(m_atomForDistDiedre[3]->pos()->x(),
                                m_atomForDistDiedre[3]->pos()->y(),
                                m_atomForDistDiedre[3]->pos()->z())
                        );



      // Check whether these measurements have been sent already
      if( m_lastMeasurement[2] != norm )
      {
        m_lastMeasurement[2] = norm ;

        distanceString = tr("Distance (3->4): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
          .arg(norm)
          .arg(QString::fromUtf8("Å")) ;

        emit message(distanceString) ;
      }

      if( m_lastMeasurement[4] != m_angle[1] )
      {
        m_lastMeasurement[4] = m_angle[1] ;

        angleString = tr("Angle 2: %L1 °").arg(m_angle[1]) ;
        emit message(angleString) ;
      }

      if( m_lastMeasurement[5] != m_dihedral )
      {
        m_lastMeasurement[5] = m_dihedral ;

        dihedralString = tr("Dihedral Angle: %1 °").arg(m_dihedral) ;
        emit message(dihedralString) ;
      }
    }
  }


  /**
    * Render all informations for the "calculation of the distance, angle, diedre angle" feature.
    */
  void WmTool::paintDistDiedre()
  {
    if( m_atomForDistDiedre.size() > 0 )
    {
      calculateParameters() ;
      //puts( "After calculateParameters() ;" ) ;

      Vector3d btza=m_widget->camera()->backTransformedZAxis() ;
      int wh=m_widget->height() ;
      
      QString msg ;
      string tmp1, tmp2 ;
      ostringstream oss ;
      Vector3d textRelPos ;
      float r,g,b ;

      
      int maxV = qMax( m_nbVPixelDist, m_nbVPixelAngle ) ;
      maxV = qMax( maxV, m_nbVPixelDihedral ) ;
      drawRect( 10.f, float(wh-maxV), float(m_nbHPixelDist), float(wh-3) ) ;

      //m_initDisplayDistDiedre = true ;
      QFontMetrics fontMetricDisDiedreInfo(m_fontDistDiedreInfo) ;

      m_nbHPixelDist = WMTOOL_SPACING_LEFT_WORDGRP ;
      m_nbHPixelAngle = WMTOOL_SPACING_LEFT_WORDGRP ;
      m_nbHPixelDihedral = WMTOOL_SPACING_LEFT_WORDGRP ;
      m_nbVPixelDist = WMTOOL_SPACING_DOWN_WORDGRP ;
      m_nbVPixelAngle = 0 ;
      m_nbVPixelDihedral = 0 ;


      glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
      // Save OpenGL attribute stack (color, lightning, texturing ...)

      for( int i=0 ; i<m_atomForDistDiedre.size() ; i++ )
      {
        oss.str("") ;
        oss << "." << (i+1) ; tmp1 = oss.str() ;
        oss << " is a number." ; tmp2 = oss.str() ;
        textRelPos = (0.18 + m_widget->radius(m_atomForDistDiedre[i])) * btza ;

        switch( i )
        {
        case 0 : r=1.0; g=0.0; b=0.0; break ;
        case 1 : r=0.0; g=1.0; b=0.0; break ;
        case 2 : r=0.0; g=0.0; b=1.0; break ;
        case 3 : r=0.0; g=1.0; b=1.0; break ;
        default : r=0.0; g=0.0; b=0.0; 
        }

        msg = tr(tmp1.c_str(), tmp2.c_str()) ;
        textRelPos += *m_atomForDistDiedre[i]->pos() ;
        displayMsgInRenderZone( textRelPos, msg, m_fontDistDiedreAtom, r, g, b ) ;
      }


      if( m_atomForDistDiedre.size() >= 2 )
      {
        // Try to put the labels in a reasonable place on the display.

        QString format("%L1"); // For localized measurements, e.g. 1,02 in Europe.

        // Text position for distance.
        msg = tr("Distance(s) :") ;
        displayMsgOnScreen( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist),
                            msg, m_fontDistDiedreInfo, 1.0,1.0,1.0 ) ;
        m_nbHPixelDist += fontMetricDisDiedreInfo.width(msg) ;
        m_nbVPixelAngle += (m_nbVPixelDist + fontMetricDisDiedreInfo.lineSpacing() + WMTOOL_SPACING_V_WORDGRP) ;

        // Text position for the 1st distance.
        msg = format.arg(m_vector[0].norm(), 0, 'f', 3) + m_angstromStr ;
        displayMsgOnScreen( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                            msg, m_fontDistDiedreInfo, 0.0,1.0,0.0 ) ;
        m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;


        if( m_atomForDistDiedre.size() >= 3 )
        {
          // Text position for the 2nd distance.
          msg = format.arg(m_vector[1].norm(), 0, 'f', 3) + m_angstromStr ;
          displayMsgOnScreen( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                              msg, m_fontDistDiedreInfo, 0.0,0.0,1.0 ) ;
          m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;

          // Text position for angle.
          msg = QString(tr("Angle         :")) ;
          displayMsgOnScreen( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle), 
                              msg, m_fontDistDiedreInfo, 1.0,1.0,1.0 ) ;
          m_nbHPixelAngle += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;
          m_nbVPixelDihedral = (m_nbVPixelAngle + fontMetricDisDiedreInfo.lineSpacing() + WMTOOL_SPACING_V_WORDGRP) ;

          // Text position for the 1st angle.
          msg = format.arg(m_angle[0], 0, 'f', 1) + m_degreeStr ;
          displayMsgOnScreen( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle), 
                              msg, m_fontDistDiedreInfo, 0.0f, 1.0f, 0.0f ) ;
          m_nbHPixelAngle += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;


          // Draw the angle.
          const Vector3d *origin=m_atomForDistDiedre[1]->pos() ;
          double radius=m_widget->radius(m_atomForDistDiedre[1])+0.2 ;

          glEnable(GL_BLEND);
          glDepthMask(GL_FALSE);

          m_wPainter->setColor(0, 1.0, 0, 0.3f);
          m_wPainter->drawShadedSector( *origin, *m_atomForDistDiedre[0]->pos(),
                                        *m_atomForDistDiedre[2]->pos(), radius ) ;
          glDepthMask(GL_TRUE);
          glDisable(GL_BLEND);

          m_wPainter->setColor(1.0, 1.0, 1.0, 1.0);
          m_wPainter->drawArc( *origin, *m_atomForDistDiedre[0]->pos(),
                               *m_atomForDistDiedre[2]->pos(), radius, true ) ;


          if(m_atomForDistDiedre.size() >= 4)
          {
            // Text position for the 3rd distance.
            msg = format.arg(m_vector[2].norm(), 0, 'f', 3) + m_angstromStr ;
            displayMsgOnScreen( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                                msg, m_fontDistDiedreInfo, 0.0, 1.0, 1.0 ) ;
            m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;

            // Text position for the 2nd angle.
            msg = format.arg(m_angle[1], 0, 'f', 1)+m_degreeStr ;
            displayMsgOnScreen( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle),
                                msg, m_fontDistDiedreInfo, 0.0f, 0.0f, 1.0f  ) ;

            // Text position for dihetral.
            msg = QString(tr("Dihedral    :")) ;
            displayMsgOnScreen( QPoint(m_nbHPixelDihedral, wh-m_nbVPixelDihedral), 
                                msg, m_fontDistDiedreInfo, 1.0, 1.0, 1.0 ) ;
            m_nbHPixelDihedral += (fontMetricDisDiedreInfo.width(msg) + WMTOOL_SPACING_H_WORDGRP) ;


            msg = format.arg(m_dihedral, 0, 'f', 1)+m_degreeStr ;
            displayMsgOnScreen( QPoint(m_nbHPixelDihedral , wh-m_nbVPixelDihedral), 
                                msg, m_fontDistDiedreInfo, 0.0f, 1.0f, 0.0f) ;
            m_nbVPixelDihedral += (fontMetricDisDiedreInfo.lineSpacing() + (WMTOOL_SPACING_V_WORDGRP/2)) ;
          }
        }
      }

      glPopAttrib() ;      
    }
  }


  /**
    * Display a message in the render zone, ie. it changes according to the camera.
    * @param pos (x,y) from the top left corner.
    * @param pos msg Message to display.
    * @param r Red composant.
    * @param g Green composant.
    * @param b Blue composant.
    */
  void WmTool::displayMsgInRenderZone( QPoint pos, QString msg, QFont font, float r, float g, float b )
  {
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( r, g, b ) ;

    // Method 5, see displayTextMethods().
    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
      myWidget->renderText( pos.x(), pos.y(), 0, msg, font ) ;
    else
      m_wPainter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }

  /**
    * Display a message in the render zone, ie. it changes according to the camera.
    * @param pos (x,y,z) in the render zone.
    * @param pos msg Message to display.
    * @param r Red composant.
    * @param g Green composant.
    * @param b Blue composant.
    */
  void WmTool::displayMsgInRenderZone( const Vector3d& pos, QString msg, QFont font, float r, float g, float b )
  {
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( r, g, b ) ;

    // Method 5, see displayTextMethods().
    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
      myWidget->renderText( pos[0], pos[1], pos[2], msg, font ) ;
    else
      m_wPainter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }

  /**
  * Display a message on the render zone. The message stays static.
  * @param pos (x,y) from the top left corner.
  * @param pos msg Message to display.
  * @param r Red composant.
  * @param g Green composant.
  * @param b Blue composant.
  */
  void WmTool::displayMsgOnScreen( QPoint pos, QString msg, QFont font, float r, float g, float b )
  {
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( r, g, b ) ;

    // Method 5, see displayTextMethods().
    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
      myWidget->renderText( pos.x(), pos.y(), msg, font ) ;
    else
      m_wPainter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }

  /**
    * Display a message on the render zone. The message stays static.
    * @param pos (x,y,z) in the render zone.
    * @param pos msg Message to display.
    * @param r Red composant.
    * @param g Green composant.
    * @param b Blue composant.
    */
  void WmTool::displayMsgOnScreen( const Vector3d& pos, QString msg, QFont font, float r, float g, float b )
  {
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( r, g, b ) ;

    // Method 5, see displayTextMethods().
    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
    {
      Vector3d proj=m_widget->camera()->project(pos) ;
      QPoint p( (int)proj[0], (int)proj[1] ) ;
      myWidget->renderText( (int)pos.x(), (int)pos.y(), msg, font ) ;
    }
    else
      m_wPainter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }


  /**
    * All method to render/draw text in the render zone.
    */
  void WmTool::displayTextMethods()
  {
    QString msg ;
    QFont myFont( "Times", 32, QFont::Bold ) ;


    // 1, Origin : Up/left screen. Nothing change during a camera movement.
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( 1.0, 1.0, 1.0 ) ;
    msg="Avo::Painter : DrawText() 1" ;
    m_wPainter->drawText( 150, 10, msg ) ;
    m_wPainter->drawText( QPoint(150,30), msg ) ;
    glPopAttrib() ; 

    // 2 Change with the camera like a point in the 3D-space
    // ! Works sometime ... Realize 2 calls of this method => don't work ...
    //glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    //glColor3f( 1.0, 1.0, 1.0 ) ;
    //msg="Avo::Painter : DrawText() 2" ;
    // Origin : center screen.
    //m_wPainter->drawText( Vector3d(0,0,0), msg, myFont ) ;
    //glPopAttrib() ;
   
    // 3.0 ! Painter class isn't a derived class of QPainter.
    //QPainter *myQPainter=dynamic_cast<QPainter*>(m_wPainter) ;

    // 3.1, Display, but in black ... Be carefull of the background color.
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glPushMatrix() ;
    glColor3f( 1.0, 1.0, 1.0 ) ;
    QPainter myQPainter ;
    myQPainter.begin(m_widget) ;
    msg = "Qt::QPainter : DrawText() 3.1" ;
    myQPainter.drawText( QPoint(150,70), msg ) ;
    msg = "Qt::QPainter : DrawText() 3.2" ;
    myQPainter.drawText( 150, 80, 150, 150, Qt::AlignLeft, msg ) ;
    myQPainter.end() ;
    glPopMatrix() ;
    glPopAttrib() ;

    // 3.2 No tested ... I do not understand when use it to "gain" performance.
    //QStaticText test ;
    //myQPainter.drawStaticText( 150, 130, ) ;
    //myQPainter.end() ;
    //glPopAttrib() ; 
 
    // 4 Works sometime, The font size change with zoom.
    // Problem with others visual feature like the blue selection ...
    //glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    //glColor3f( 1.0, 1.0, 1.0 ) ;
    //msg="Avo::QWidget : renderText() 4" ;
    //m_widget->renderText( 40, 200, 0, msg, myFont ) ;
    //glPopAttrib() ;
    

    // 5 Nothing change during a camera movement.
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( 1.0, 1.0, 1.0 ) ;

    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
    {
      msg="Qt::QWidget : renderText() 5" ;
      myWidget->renderText( 40, 40, msg, myFont ) ;
      msg="Qt::QWidget : renderText() 5.1" ;
      // Sometime below the previous message, sometime in the 3D-space coordonate.
      myWidget->renderText( 80, 80, 0, msg, myFont ) ;
      msg="Qt::QWidget : renderText() 5.2" ;
      myWidget->renderText( -6.879, 0.298, 0.51, msg, myFont ) ;
    }
    else
    {
      msg="Qt::QWidget : renderText() 5 FALSE" ;
      m_wPainter->drawText( 100, 140, msg ) ;
    }

    glPopAttrib() ;
  }

  void WmTool::setSizeRatioFont( int ratio )
  {
    float r=(float)(ratio)*0.1f ;

    if( r>=WMTOOL_POINTSIZE_RATIO_MIN && r<=WMTOOL_POINTSIZE_RATIO_MAX )
    {
      m_fontDistDiedreAtom.setPointSizeF( r * WMTOOL_FONT_POINTSIZE_DISTDIEDREATOM ) ;
      m_fontDistDiedreInfo.setPointSizeF( r * WMTOOL_FONT_POINTSIZE_DISTDIEDREINFO ) ;
      m_fontError.setPointSizeF( r * WMTOOL_FONT_POINTSIZE_ERROR ) ;
      m_fontInfo.setPointSizeF( r * WMTOOL_FONT_POINTSIZE_INFO ) ;
    }
  }

}

Q_EXPORT_PLUGIN2(wmtool, Avogadro::WmToolFactory)
