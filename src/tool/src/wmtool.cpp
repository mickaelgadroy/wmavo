
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

  const double WmTool::m_PI=(double)103993/(double)33102 ;
  const double WmTool::m_PI180=m_PI/(double)180 ;
  const double WmTool::m_180PI=(double)180/m_PI ;

  WmTool::WmTool(QObject *parent) :
      Tool(parent),
      m_settingsWidget(NULL),/* m_addHydrogensCheck(NULL),*/

      m_widget(NULL), m_wPainter(NULL), m_wmExt(NULL),
      m_wmIsConnected(false), m_wmNbDots(0), m_wmNbSources(0), m_wmDistance(0),
      m_atomicNumberCur( WMEX_CREATEDATOMDEFAULT ),

      m_rectPos1(QPoint(0,0)), m_rectPos2(QPoint(0,0)), m_activeRect(false),

      m_drawBeginAtom(false), m_drawEndAtom(false), m_drawBond(false),
      m_beginAtom(Vector3d(0,0,0)), m_endAtom(Vector3d(0,0,0)),
      m_isCalculDistDiedre(false), m_nbAtomForDistDiedre(0)
  {
    m_gluQuadricParams = gluNewQuadric() ;
    gluQuadricDrawStyle( m_gluQuadricParams, GLU_LINE ) ;

    m_gluQuadricParamsCenter = gluNewQuadric() ;
    gluQuadricDrawStyle( m_gluQuadricParamsCenter, GLU_FILL ) ;

    for( int i=0 ; i<6 ; i++ )
      m_lastMeasurement[i] = 0.0 ;

    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/wmtool/wiimote.png")));
    action->setToolTip(tr("Render Tool"));
    //action->setShortcut(Qt::Key_F1);

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

  QUndoCommand* WmTool::wheelEvent(GLWidget *widget, QWheelEvent *event )
  {
    event->accept();
    //widget->update();
    cout << "WmTool::mousewheelEvent" << endl ;
    return 0 ;
  }

  QUndoCommand* WmTool::keyPressEvent(GLWidget *widget, QKeyEvent *event)
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
    return 2500000 ;
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
      m_wmSensitiveSlider = new QSlider( Qt::Horizontal ) ;
      m_wmSensitiveSlider->setMaximum( PLUGIN_WM_SENSITIVE_MAX ) ;
      m_wmSensitiveSlider->setMinimum( PLUGIN_WM_SENSITIVE_MIN ) ;
      m_wmSensitiveSlider->setValue( PLUGIN_WM_SENSITIVE_DEFAULT ) ;
      m_wmSensitiveSlider->setTickInterval( 1 ) ;
      //m_addHydrogensCheck = new QCheckBox( "Adjust Hydrogen" ) ;

      QHBoxLayout *hBox=new QHBoxLayout() ;
      hBox->addWidget( lblWmSensitive ) ;
      hBox->addWidget( m_wmSensitiveSlider ) ;

      QVBoxLayout *vBox=new QVBoxLayout() ;
      vBox->addLayout( hBox ) ;
      //vBox->addWidget( m_addHydrogensCheck ) ;

      m_settingsWidget->setLayout( vBox ) ;

      // Signals connected in wmEx class by the rederected signal ...
      bool isConnect = connect( m_wmSensitiveSlider,  SIGNAL(valueChanged(int)),
                                this, SLOT(changedWmSensitiveRedirect(int)) ) ;
      if( !isConnect )
        qDebug() << "Problem connection signal : m_wmSensitiveSlider.valueChanged() -> wmTool.changedWmSensitiveRedirect() !!" ;

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


    if( m_widget == NULL )
    {
      m_widget = widget ;
      m_wPainter = m_widget->painter() ;

      // Normaly, useless ...
      //ContextMenuEater *cm=new ContextMenuEater( this, m_widget ) ;
      //m_widget->installEventFilter( cm ) ;
    }

    displayMsg() ;
    displayWmInfo() ;
    displayAtomicNumberCurrent() ;

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
    drawBarycenter() ;

    // Paint a selection rectangle (it is the last feature to realize the good effects of transparency).
    if( m_activeRect )
      drawRect( m_rectPos1, m_rectPos2 ) ;

    return true ;
  }


  /**
    * Draw a rectangle in the render zone.
    * @param p1 The up/left position of the rectangle.
    * @param p2 The bottom/right position of the rectangle.
    */
  void WmTool::drawRect( QPoint p1, QPoint p2 )
  {
    drawRect( p1.x(), p1.y(), p2.x(), p2.y() ) ;
  }


  /**
    * Draw a rectangle in the render zone.
    * @param sx The up/left position in x of the rectangle.
    * @param sy The up/left position in y of the rectangle.
    * @param ex The bottom/right position in x of the rectangle.
    * @param ey The bottom/right position in y of the rectangle.
    */
  void WmTool::drawRect( float sx, float sy, float ex, float ey )
  {
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

    glColor4f(1.0, 0.5, 0.5, 0.4);
    glBegin(GL_POLYGON);
      glVertex3f(startPos[0],startPos[1],startPos[2]);
      glVertex3f(startPos[0],endPos[1],startPos[2]);
      glVertex3f(endPos[0],endPos[1],startPos[2]);
      glVertex3f(endPos[0],startPos[1],startPos[2]);
    glEnd();

    startPos[2] += 0.0001;
    glDisable(GL_BLEND);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
      glVertex3f(startPos[0],startPos[1],startPos[2]);
      glVertex3f(startPos[0],endPos[1],startPos[2]);
      glVertex3f(endPos[0],endPos[1],startPos[2]);
      glVertex3f(endPos[0],startPos[1],startPos[2]);
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
  void WmTool::drawAtom( float radius, Vector3d from )
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

    glTranslatef( from[0], from[1], from[2] ) ;
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
    glColor3f( 0.2, 1.0,  0.2 ) ;

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

    glColor3f( 0.2, 1.0,  0.2 ) ;

    //Vector3d b=m_wmExt->getPointRef() ;
    //glTranslatef( b[0], b[1] , b[2] ) ;

    gluSphere( m_gluQuadricParamsCenter, 0.1, 5, 5) ;

    glPopAttrib() ;
    glPopMatrix() ;
  }



  /**
    * Quick render the "bond" between 2 positions.
    * @param begin 1st end of the "bond".
    * @param end 2nd end of the "bond".
    */
  void WmTool::drawBond2( Vector3d begin, Vector3d end )
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // If active, do not paint the atom !!!
    // glLoadIdentity();

    // If no active, do not change color !!!
    glDisable( GL_LIGHTING ) ;

    glLineWidth( 8.0F ) ;

    glBegin( GL_LINES ) ;
      glColor3f( 0.0, 0.0, 0.0 ) ;
      glVertex3f( begin[0], begin[1], begin[2] ) ;
      glColor3f( 0.0, 0.0, 0.5 ) ;
      glVertex3f( end[0], end[1], end[2] ) ;
    glEnd() ;

    glPopAttrib() ;
  }

/*
  void WmTool::drawBond( float radius, Vector3d begin, Vector3d end )
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
      drawAtom( 0.4, m_beginAtom ) ;

    if( m_drawEndAtom )
      drawAtom( 0.4, m_endAtom ) ;

    if( m_drawBond )
      drawBond2( m_beginAtom, m_endAtom ) ;
  }


  /**
    * Display informations of the Wiimote in the render zone.
    */
  void WmTool::displayWmInfo()
  {
    if( m_wPainter!=NULL )
    {
      QString msg="Wiimote connected : " ;

      if( m_wmIsConnected ) msg += "YES" ;
      else msg += tr("NO") ; //, Press 1+2") ;
      glColor3f( 1.0, 1.0, 1.0 ) ;
      m_wPainter->drawText( QPoint(10,20), msg ) ;

      if( m_wmIsConnected )
      {
        //msg = tr("Nb detected LEDs : ") + QString::number(m_wmNbDots) ;
        //glColor3f( 1.0, 1.0, 1.0 ) ;
        //m_wPainter->drawText( QPoint(10,40), msg ) ;

        msg = tr("Nb detected sources : ") + QString::number(m_wmNbSources) ;
        if( m_wmNbSources == 1 )
          glColor3f( 1.0, 0.5, 0.0 ) ;
        else if( m_wmNbDots == 0 )
          glColor3f( 1.0, 0.0, 0.0 ) ;
        else
          glColor3f( 1.0, 1.0, 1.0 ) ;
        m_wPainter->drawText( QPoint(10,60), msg ) ;

        //msg = tr("Distance : ") + QString::number(m_wmDistance) ;
        //m_wPainter->drawText( QPoint(10,80), msg ) ;

        // "Reset" color.
        glColor3f( 1.0, 1.0, 1.0 ) ;
      }
    }
  }


  /**
    * Display a message in the render zone.
    */
  void WmTool::displayMsg()
  {
    if( m_displayMsg && m_wPainter!=NULL )
    {
      int i=0 ;
      int t2=m_time.elapsed() ;

      if( (t2-m_displayTime) < WMTOOL_TIME_DISPLAY_MSG )
      {
        glColor3f( 1.0, 0.0, 0.0 ) ;
        foreach( QString str, m_displayList )
        {
          m_wPainter->drawText( QPoint(m_displayPos.x(), m_displayPos.y()+i), str ) ;
          i += 20 ;
        }
        glColor3f( 1.0, 1.0, 1.0 ) ;
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
    if( m_wPainter != NULL )
    {
      QString str ;
      str = tr("Current atomic number : ") + QString::number(m_atomicNumberCur) ;
      glColor3f( 1.0, 1.0, 1.0 ) ;
      m_wPainter->drawText( QPoint(10,100), str ) ;
    }
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
  void WmTool::setWmInfo( bool connect, int nbDots, int nbSources, int distance )
  {
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
  void WmTool::setWmExt( Extension *wmExtens )
  {
    if( wmExtens != NULL )
      m_wmExt = wmExtens ;
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
  void WmTool::renderAtomBond( Vector3d beginAtom, Vector3d endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond )
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

      Vector3d btza=m_widget->camera()->backTransformedZAxis() ;
      int wh=m_widget->height() ;

      QString angstrom=QString::fromUtf8( " Å" ) ;
      QString degree=QString::fromUtf8( "°" ) ;

      string tmp1, tmp2 ;
      ostringstream oss ;
      Vector3d textRelPos ;


      glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
      // Text Size ... ?

      for( int i=0 ; i<m_atomForDistDiedre.size() ; i++ )
      {
        oss.str("") ;
        oss << "*" << (i+1) ; tmp1 = oss.str() ;
        oss << " is a number." ; tmp2 = oss.str() ;
        textRelPos = (0.18 + m_widget->radius(m_atomForDistDiedre[i])) * btza ;

        switch( i )
        {
        case 0 :
          glColor3f(1.0,0.0,0.0) ; break ;
        case 1 :
          glColor3f(0.0,1.0,0.0) ; break ;
        case 2 :
          glColor3f(0.0,0.0,1.0) ; break ;
        case 3 :
          glColor3f(0.0,1.0,1.0) ; break ;
        default :
          glColor3f(0.0,0.0,0.0) ;
        }

        m_wPainter->drawText( *m_atomForDistDiedre[i]->pos()+textRelPos, tr(tmp1.c_str(), tmp2.c_str()) ) ;
      }


      if( m_atomForDistDiedre.size() >= 2 )
      {
        // Try to put the labels in a reasonable place on the display.

        QString format("%L1"); // For localized measurements, e.g. 1,02 in Europe.

        // Text position for distance.
        glColor3f(1.0,1.0,1.0);
        m_wPainter->drawText(QPoint(70, wh-25), tr("Distance(s):"));

        // Text position for the 1st distance.
        glColor3f(1.0,1.0,0.0);
        m_wPainter->drawText(QPoint(180, wh-25), format.arg(m_vector[0].norm(), 0, 'f', 3) + angstrom ) ;


        if( m_atomForDistDiedre.size() >= 3 )
        {
          // Text position for the 2nd distance.
          glColor3f(0.0,1.0,1.0);
          m_wPainter->drawText(QPoint(240, wh-25), format.arg(m_vector[1].norm(), 0, 'f', 3) + angstrom);

          // Text position for angle.
          glColor3f(1.0,1.0,1.0);
          m_wPainter->drawText(QPoint(70, wh-45), QString(tr("Angle:")));

          // Text position for the 1st angle.
          glColor3f(0.8, 0.8, 0.8);
          m_wPainter->drawText(QPoint(180, wh-45), format.arg(m_angle[0], 0, 'f', 1) + degree);


          // Draw the angle.
          const Vector3d *origin=m_atomForDistDiedre[1]->pos() ;
          double radius=m_widget->radius(m_atomForDistDiedre[1])+0.2 ;

          glEnable(GL_BLEND);
          glDepthMask(GL_FALSE);

          m_wPainter->setColor(0, 1.0, 0, 0.3);
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
            glColor3f(1.0, 1.0, 1.0);
            m_wPainter->drawText(QPoint(300, wh-25), format.arg(m_vector[2].norm(), 0, 'f', 3) + angstrom);

            // Text position for the 2nd angle.
            glColor3f(0.8, 0.8, 0.8);
            m_wPainter->drawText(QPoint(240, wh-45), format.arg(m_angle[1], 0, 'f', 1) + degree);

            // Text position for dihetral.
            glColor3f(1.0, 1.0, 1.0);
            m_wPainter->drawText(QPoint(70, wh-65), QString(tr("Dihedral:")));

            glColor3f(0.6, 0.6, 0.6);
            m_wPainter->drawText(QPoint(180, wh-65), format.arg(m_dihedral, 0, 'f', 1) + degree);
          }
        }
      }

      glPopAttrib() ;
    }
  }

}

Q_EXPORT_PLUGIN2(wmtool, Avogadro::WmToolFactory)
