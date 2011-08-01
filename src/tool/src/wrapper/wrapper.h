
/*******************************************************************************
  Copyright (C) 2011 Mickael Gadroy

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
#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#define WITD WrapperInputToDomain

#define WRAPPER_ID_NOTHING 0x00000000
#define WRAPPER_ID_CHEMICAL 0x00000001

#include "warning_disable_begin.h"
#include <iostream>
#include <Eigen/Core>
#include <QPoint>
#include "warning_disable_end.h"

namespace WrapperInputToDomain
{
  class WrapperData
  {

    protected :
    struct wrapperActions_t
    {
      int actionsGlobal ;
      int actionsWrapper ;
      int wrapperType ;

      wrapperActions_t()
      {
        actionsGlobal=0 ;
        actionsWrapper=0 ;
        wrapperType=WRAPPER_ID_NOTHING ;
      }
    } ;

    struct positionPointed_t
    {
      QPoint posCursor ;
      Eigen::Vector3d pos3dCur, pos3dLast ;

      positionPointed_t()
      {
        posCursor = QPoint(0,0) ;
        pos3dCur=Eigen::Vector3d(0,0,0) ;
        pos3dLast=Eigen::Vector3d(0,0,0) ;
      }
    } m_posPointed ;

    struct positionCamera_t
    {
      double angleRotateDegree[3] ; // X, Y, Z
      double distanceTranslate[3] ; // X, Y, Z

      positionCamera_t()
      {
        for( int i=0 ; i<3 ; i++ )
        {
          angleRotateDegree[i]=0 ;
          distanceTranslate[i]=0 ;
        }
      }
    } m_posCam ;


  public :
    WrapperData() ;
    virtual ~WrapperData() ;

    inline virtual unsigned int getWrapperType(){ return WRAPPER_ID_NOTHING ; } ;
    inline wrapperActions_t getWrapperAction(){ return m_wrapActions ; } ;
    inline positionPointed_t getPositionPointed(){ return m_posPointed ; } ;
    inline positionCamera_t getPositionCamera(){ return m_posCam ; } ;

  protected :

    wrapperActions_t m_wrapActions ;
   
  
  };


  class Wrapper
  {
  public :
    Wrapper() ;
    virtual ~Wrapper() ;

    virtual unsigned int getWrapperType(){ return WRAPPER_ID_NOTHING ; } ;
    virtual WrapperData* getWrapperData(){ return m_wrapData ; };

  protected :
    WrapperData *m_wrapData ;
  };

}

#endif
