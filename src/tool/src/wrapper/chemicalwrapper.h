
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
#ifndef __CHEMICALWRAPPER_H__
#define __CHEMICALWRAPPER_H__

#include "wrapper.h"

namespace WrapperInputToDomain
{
  class ChemicalWrapData
  {
  public :
    ChemicalWrapData() ;
    virtual ~ChemicalWrapData() ;

    inline unsigned int getWrapperType(){ return WRAPPER_ID_CHEMICAL ; } ;
  };

  // Chemical wrapper data.
  class ChemicalWrap
  {
  public :
    ChemicalWrap() ;
    virtual ~ChemicalWrap() ;

    inline unsigned int getWrapperType(){ return WRAPPER_ID_CHEMICAL ; } ;
    inline ChemicalWrapData* getWrapperData(){ return m_chemWrapData ; } ;

  protected :
    ChemicalWrapData* m_chemWrapData ;
  };

}

#endif
