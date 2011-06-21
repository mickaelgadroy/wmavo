/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of example_qt of the wiiusecpp library.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


 #include <QtGui>
 #include <QtOpenGL>

 #include <math.h>

 #include "glwidget.h"
 #include "qtlogo.h"

 #ifndef GL_MULTISAMPLE
 #define GL_MULTISAMPLE  0x809D
 #endif

 GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
       /*m_logo(0),*/ m_xRot(0), m_yRot(0), m_zRot(0)
 {
     qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
     qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

     m_gluQuadricParams1 = gluNewQuadric() ;
     gluQuadricDrawStyle( m_gluQuadricParams1, GLU_FILL ) ;
 }

 GLWidget::~GLWidget()
 {
     gluDeleteQuadric( m_gluQuadricParams1 ) ;
 }

 QSize GLWidget::minimumSizeHint() const
 {
     return QSize(100, 100);
 }

 QSize GLWidget::sizeHint() const
 {
     return QSize(400, 400);
 }

 static void qNormalizeAngle(int &angle)
 {
     while (angle < 0)
         angle += 360 * SLIDER_ROT_CONST;
     while (angle > 360 * SLIDER_ROT_CONST)
         angle -= 360 * SLIDER_ROT_CONST;
 }

 void GLWidget::setXRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_xRot)
     {
         m_xRot = angle;
         emit xRotationChanged(angle);
         updateGL();
     }
 }

 void GLWidget::setYRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_yRot)
     {
         m_yRot = angle;
         emit yRotationChanged(angle);
         updateGL();
     }
 }

 void GLWidget::setZRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_zRot)
     {
         m_zRot = angle;
         emit zRotationChanged(angle);
         updateGL();
     }
 }

 static float qNormalizeDistance( float distance )
 {
     if( distance < 0)
         distance = 0 ;
     if( distance > SLIDER_TRANS_MAX )
         distance = SLIDER_TRANS_MAX ;

     return distance / SLIDER_TRANS_CONST ;
 }

 void GLWidget::setXTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_xTransl )
     {
         m_xTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setYTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_yTransl )
     {
         m_yTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setZTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_zTransl )
     {
         m_zTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setTranslation( float x, float y, float z )
 {
    m_zTransl = x ;
    m_yTransl = y ;
    m_zTransl = z ;
    updateGL() ;
 }

 void GLWidget::initializeGL()
 {
     qglClearColor(qtPurple.dark());

     //m_logo = new QtLogo(this, 64);
     //m_logo->setColor(qtGreen.dark());

     glEnable(GL_DEPTH_TEST);
     glEnable(GL_CULL_FACE);
     glShadeModel(GL_SMOOTH);
     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glEnable(GL_MULTISAMPLE);
     static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
     glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

     glMatrixMode( GL_PROJECTION ) ;

     // PI/2
     gluPerspective( 3.14*0.50, width()/height(), 1, 1000 );
     glMatrixMode( GL_MODELVIEW ) ;
 }

 void GLWidget::paintGL()
 {
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glLoadIdentity();

     // fonction pour fixer la matrice de vue
     // prend en paramètre les coordonnées de la position de la "caméra"
     // les coordonnées du point visé et le vecteur haut de la caméra
     //gluLookAt( 10, 0, 10, 0, 0, 0, 0, 1, 0 );
     gluLookAt( 2, 0, 10, 0, 0, 0, 0, 1, 0 );

     // sauvegarde de la matrice courante
     glPushMatrix();

     //glTranslatef(0.0f, 0.0f, -8.0f);


     // ->
     // Traitement lié à mon extension.

     //if( m_extension != NULL )
        //m_extension->translateObject() ;

     glTranslatef(m_xTransl, m_yTransl, m_zTransl) ;

     // Fin de traitement des données liées à mon extension.
     // <-

     glRotatef(m_xRot / SLIDER_ROT_CONST, 1.0, 0.0, 0.0) ;
     glRotatef(m_yRot / SLIDER_ROT_CONST, 0.0, 1.0, 0.0) ;
     glRotatef(m_zRot / SLIDER_ROT_CONST, 0.0, 0.0, 1.0) ;


     //m_logo->draw();
     drawSphere( 0.1f, 0.8f, 0.8f, 0.8f ) ;

     // restauration de la matrice précédemment sauvée
     glPopMatrix();

     // ne pas oublier de re push la matrice si on a encore d'autres objets à
     // dessiner ensuite

     //glLoadIdentity();
     //glTranslatef(0.1f, 0.0f, -8.0f);
     drawSphere( 0.1f, 0.8f, 0.8f, 0.8f ) ;
 }

 void GLWidget::resizeGL(int width, int height)
 {
     int side = qMin(width, height);
     glViewport((width - side) / 2, (height - side) / 2, side, side);

     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
 #ifdef QT_OPENGL_ES_1
     glOrthof(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
 #else
     glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
 #endif
     glMatrixMode(GL_MODELVIEW);
 }

 void GLWidget::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
 }

 void GLWidget::mouseMoveEvent(QMouseEvent *event)
 {
     int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();

     if (event->buttons() & Qt::LeftButton) {
         setXRotation(m_xRot + 8 * dy);
         setYRotation(m_yRot + 8 * dx);
     } else if (event->buttons() & Qt::RightButton) {
         setXRotation(m_xRot + 8 * dy);
         setZRotation(m_zRot + 8 * dx);
     }
     lastPos = event->pos();
 }

 void GLWidget::drawSphere( float radius, float r, float v, float b  )
 {
     // Init & Save.
     glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

     glColor3f( r, v, b ) ;
     gluSphere( m_gluQuadricParams1, radius, 10, 10) ;
     // params : GLUquadric = options
     // 0.75 : radius : rayon de la sphère.
     // 20 : slices : lontitudes.
     // 20 : stacks : latitudes.

     glPopAttrib();
 }
