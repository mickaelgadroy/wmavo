
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

#include "chemicalwrapper.h"

namespace WrapperInputToDomain
{
  ChemicalWrapData::ChemicalWrapData()
  {
  }

  ChemicalWrapData::~ChemicalWrapData()
  {
  }

  ChemicalWrap::ChemicalWrap() : m_chemWrapData(NULL)
  {
    m_chemWrapData = new ChemicalWrapData() ;
  }

  ChemicalWrap::~ChemicalWrap()
  {
  }
}
