
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

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


/// All constants use by the plugins.


#ifndef __WMAVO_CONST_H__
#define __WMAVO_CONST_H__

///**********************************************************
/// Naming plugins, descriptions ...
/// Specific for wmextension.h & wmtool.h.
///**********************************************************

#define PLUGIN_WMTOOL_IDENTIFIER "wm"
#define PLUGIN_WMTOOL_NAME "WmTool"
#define PLUGIN_WMTOOL_DESCR "Use Wiimote in Avogadro"
#define PLUGIN_WMTOOL_WIDGET_TITLE "WmTool Title"

#define PLUGIN_DRAWTOOL_NAME "Draw"

#define WMTOOL_TIME_DISPLAY_MSG 5000

#define PLUGIN_WM_SENSITIVE_DEFAULT 30
#define PLUGIN_WM_SENSITIVE_MIN 15
#define PLUGIN_WM_SENSITIVE_MAX 60


///**********************************************************
/// Specific for wmextension.h.
///**********************************************************

#define WMEX_DISTBEFORE_CREATE 0.6
#define WMEX_MAXBONDNUMBER_BETWEENATOM 3
#define WMEX_CREATEDATOMDEFAULT 6 // Carbon.
#define WMEX_ADJUST_HYDROGEN true


///**********************************************************
/// Specific for wmavo_thread.h.
///**********************************************************

#define WMAVOTH_SLEEPBEFORE_NEXTROUND 4 // ms


///**********************************************************
/// Specific for wmavo.h and use by wmextension.h.
// Many constants values are got by experience (and they have not unit...).
///**********************************************************

#define WMAVO_CONNECTION_TIMEOUT 5.0 // Computer lets few time to the connection of the Wiimote (in seconde).
#define WMAVO_IRSENSITIVITY 3
#define WMAVO_SMOOTH 10

#define WMAVO_CURSOR_CALIBRATION_X 2.4 // 2.5
#define WMAVO_CURSOR_CALIBRATION_Y 2.4 // 3.0
#define WMAVO_CURSOR_CALIBRATION_Z 1.2 // 1.8 // 2.0

#define WMAVO_CAM_CALIBRATION_ZOOM_DISTANCE 0.1
#define WMAVO_CAM_CALIBRATION_ROTATION 9.0 // 10.0
#define WMAVO_CAM_CALIBRATION_TRANSLATION 0.9 // 1.0

#define WMAVO_ATOM_MAX_MOVE_Z 0.08 //0.1
#define WMAVO_ATOM_SMOOTHED_MOVE_XY 4
#define WMAVO_ATOM_MAX_MOVE_FOR_PROHITED_MOVE 0.001 // 0.0 // For zoom, it is a step.
#define WMAVO_ATOM_ROTATION_SPEED 0.2

#define WMAVO_WM_XY_SELECTING 18 // Nb. pixel before activate move of atom. (Not use)
#define WMAVO_WM_XY_MINPOINTING_MOVEALLOWED 0
#define WMAVO_WM_XY_MAXPOINTING_MOVEALLOWED 60 // 40 Normally to avoid "jumping cursor".
  //Remember: cursor moves itself by difference calculate here (wmavo), not by a position gets by wiiuse.

#define WMAVO_WM_Z_MINPOINTING_MOVEALLOWED 0.2 // 0.1 : By experience.

#define WMAVO_NC_MINJOY_MOVEALLOWED 0.1

#define WMAVO_NBOPERATINGMODE 3
#define WMAVO_OPERATINGMODE1 0
#define WMAVO_OPERATINGMODE2 1
#define WMAVO_OPERATINGMODE3 2

// In m_isWhat => the events of the Wiimote for molecular visualisation.
#define WMAVO_IS(action) (m_isWhat & action)
#define WMAVO_IS2(allActions, action) (allActions & action) // For wmextension.cpp
#define WMAVO_SETON(action) (m_isWhat |= action)
#define WMAVO_SETOFF(action) (m_isWhat &= (~action))
#define WMAVO_SETON2(allActions, action) (allActions |= action)
#define WMAVO_SETOFF2(allActions, action) (allActions &= (~action))


#define WMAVO_CURSOR_MOVE 0x00001
#define WMAVO_SELECT 0x00002
#define WMAVO_CREATE 0x00004
#define WMAVO_DELETE 0x00008
#define WMAVO_DELETEALL 0x00010
#define WMAVO_ATOM_MOVE 0x00020 // Translate or rotate, the mode is "activate", but it can be "not working" to limit the calling of wmextension.
#define WMAVO_ATOM_ROTATE 0x00040
#define WMAVO_ATOM_TRANSLATE 0x00080
#define WMAVO_CAM_ROTATE 0x00100
#define WMAVO_CAM_ROTATE_BYNC 0x00200
#define WMAVO_CAM_ROTATE_BYWM 0x00400
#define WMAVO_CAM_ZOOM 0x00800
#define WMAVO_CAM_TRANSLATE 0x01000
#define WMAVO_CAM_INITIAT 0x02000
#define WMAVO_MENU_ACTIVE 0x04000
#define WMAVO_MENU_RIGHT 0x08000
#define WMAVO_MENU_LEFT 0x10000
#define WMAVO_MENU_UP 0x20000
#define WMAVO_MENU_DOWN 0x40000
#define WMAVO_MENU_OK 0x80000


///**********************************************************
/// Specific for wmavo_rumble.h.
///**********************************************************


#define TIME_TRYLOCK 5 // µs
#define WMRUMBLE_TIME_SLEEP 2 // ms
#define WMRUMBLE_MAX_TIME 60000// ms
#define WMRUMBLE_MIN_TIME 80 // 50 // ms CAUTION : do not enter less, else the Wiimote sinks under Bluetooth connection, and PC blocks because mutex is always activate.

// Use for set gradual => [0;100]
// 0 => disable rumble
// 1 => WMRUMBLE_MIN_DURATION_TREMOR & WMRUMBLE_MAX_DURATION_POSE
// ...
// 99 => WMRUMBLE_MAX_DURATION_TREMOR & WMRUMBLE_MIN_DURATION_POSE
// 100 => active rumble
#define WMRUMBLE_MAX_DURATION_TREMOR 800 // 1000
#define WMRUMBLE_MIN_DURATION_TREMOR WMRUMBLE_MIN_TIME // 150 // 30
#define WMRUMBLE_MAX_DURATION_POSE 1000 // 2000 // ((fr) "prendre un pose")
#define WMRUMBLE_MIN_DURATION_POSE WMRUMBLE_MIN_TIME // 150 //


///**********************************************************
/// To disable warning.
///**********************************************************


//#define DISABLE_WARNING #ifdef _WIN32 \
//  #pragma warning( disable : 4365 ) /* conversion from 'x' to 'y', signed/unsigned mismatch */ \
//  #pragma warning( disable : 4820 ) /* 'x' bytes padding added after data member '...' */ \
//  #pragma warning( disable : 4668 ) /* '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif' */ \
//  #pragma warning( disable : 4514 ) /* '...' : unreferenced inline function has been removed */ \
//  #pragma warning( disable : 4738 ) /* storing 32-bit float result in memory, possible loss of performance */ \
//  #pragma warning( disable : 4710 ) /* function not inlined */ \
//  #pragma warning( disable : 4626 ) /* '...' : assignment operator could not be generated because a base class assignment operator is inaccessible */ \
//  #pragma warning( disable : 4625 ) /* '...' : copy constructor could not be generated because a base class copy constructor is inaccessible */ \
//  #pragma warning( disable : 4711 ) /* The compiler performed inlining on the given function, although it was not marked for inlining. Inlining is performed at the compiler's discretion. This warning is informational. */ \
//  #pragma warning( disable : 4628 ) /* Digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'. Digraphs are not supported under /Ze. This warning will be followed by an error. */ \
//  #pragma warning( push, 0 ) \
//  #endif

//#define ENABLE_WARNING \
//  #ifdef _WIN32 \
//  #pragma warning( pop ) \
//  #endif 

#endif
