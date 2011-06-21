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


 #ifndef GLWIDGET_H
 #define GLWIDGET_H

 #define SLIDER_ROT_CONST 16.0
 #define SLIDER_TRANS_CONST 100.0
 #define SLIDER_TRANS_MAX 30.0

 #include <QGLWidget>

 class GLWidget : public QGLWidget
 {
     Q_OBJECT

 public:
     GLWidget(QWidget *parent = 0);
     ~GLWidget();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;

 public slots:
     void setXRotation( int angle ) ;
     void setYRotation( int angle ) ;
     void setZRotation( int angle ) ;
     void setXTranslation( int distance ) ;
     void setYTranslation( int distance ) ;
     void setZTranslation( int distance ) ;
     void setTranslation( float x, float y, float z ) ;

 signals:
     void xRotationChanged( int angle ) ;
     void yRotationChanged( int angle ) ;
     void zRotationChanged( int angle ) ;
     void xTranslationChanged( int distance ) ;
     void yTranslationChanged( int distance ) ;
     void zTranslationChanged( int distance ) ;

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);

     void drawSphere( float radius, float r, float v, float b ) ;

 private:
     //QtLogo *m_logo;

     int m_xRot, m_yRot, m_zRot ;
     float m_xTransl, m_yTransl, m_zTransl ;

     QPoint lastPos;
     QColor qtGreen, qtPurple ;

     GLUquadric* m_gluQuadricParams1 ;
 };

 #endif
