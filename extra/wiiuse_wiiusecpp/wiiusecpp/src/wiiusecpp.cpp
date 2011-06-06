/*
 *wiiusecpp.cpp
 *
 *Written By:
 *James Thomas
 *Email: jt@missioncognition.org
 *
 *Copyright 2009
 *
 *Copyright (c) 2010 Mickael Gadroy
 *
 *This file is part of wiiusecpp.
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Note:  This C++ library is a (very) thin wrapper of the the wiiuse library.
 *        See http://www.wiiuse.net to get the wiiuse library which is required
 *        to build this package.  A SWIG based Python wrapper for this C++ library
 *        is available from http://www.missioncognition.org.
 */

#include "wiiusecpp.h"

/*
 * CButtonBase class methods.
 */
CButtonBase::CButtonBase() :
  mpBtnsPtr( NULL ), mpBtnsHeldPtr( NULL ), mpBtnsReleasedPtr( NULL )
{}
    
CButtonBase::CButtonBase(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr) :
  mpBtnsPtr( ButtonsPtr ), mpBtnsHeldPtr( ButtonsHeldPtr ), mpBtnsReleasedPtr( ButtonsReleasedPtr )
{}
  
CButtonBase::CButtonBase( const CButtonBase& cbb ) :
  mpBtnsPtr( cbb.mpBtnsPtr ), mpBtnsHeldPtr( cbb.mpBtnsHeldPtr ), mpBtnsReleasedPtr( cbb.mpBtnsReleasedPtr )
{}

CButtonBase::~CButtonBase()
{
  // All data desallocate by wiiuse.
  mpBtnsPtr = NULL ;
  mpBtnsHeldPtr = NULL ;
  mpBtnsReleasedPtr = NULL ;
}

int CButtonBase::isPressed(int Button)
{
    return (Cast(mpBtnsPtr) & Button) == Button;
}

int CButtonBase::isHeld(int Button)
{
    return (Cast(mpBtnsHeldPtr) & Button) == Button;
}

int CButtonBase::isReleased(int Button)
{
    return (Cast(mpBtnsReleasedPtr) & Button) == Button;
}

int CButtonBase::isJustPressed(int Button)
{
    return ((Cast(mpBtnsPtr) & Button) == Button) && ((Cast(mpBtnsHeldPtr) & Button) != Button);
}

/*
 * Initializers for classes derrived from CButtonBase.
 */
CButtons::CButtons() : CButtonBase()
{}
    
CButtons::CButtons( void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr ) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr)
{}

CButtons::CButtons( const CButtons& cb ) : CButtonBase( cb )
{}

CButtons::~CButtons() // See ~CButtonBase
{}


CNunchukButtons::CNunchukButtons() : CButtonBase()
{}

CNunchukButtons::CNunchukButtons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr)
{}

CNunchukButtons::CNunchukButtons( const CNunchukButtons& cnb ) : CButtonBase( cnb )
{}

CNunchukButtons::~CNunchukButtons() // See ~CButtonBase
{}


CClassicButtons::CClassicButtons() : CButtonBase()
{}

CClassicButtons::CClassicButtons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr)
{}

CClassicButtons::CClassicButtons( const CClassicButtons& ccb ) : CButtonBase( ccb )
{}

CClassicButtons::~CClassicButtons() // See ~CButtonBase
{}


CGH3Buttons::CGH3Buttons() : CButtonBase()
{}
    
CGH3Buttons::CGH3Buttons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr)
{}

CGH3Buttons::CGH3Buttons( const CGH3Buttons& cgb ) : CButtonBase( cgb )
{}

CGH3Buttons::~CGH3Buttons() // See ~CButtonBase
{}


/*
 * CJoystick class methods.
 */

CJoystick::CJoystick() : mpJoystickPtr( NULL )
{}
    
CJoystick::CJoystick(struct joystick_t *JSPtr) : mpJoystickPtr( JSPtr )
{}

CJoystick::CJoystick( const CJoystick& cj ) : mpJoystickPtr( cj .mpJoystickPtr )
{}

CJoystick::~CJoystick()
{
  // All data desallocate by wiiuse.
  mpJoystickPtr = NULL ;
}

void CJoystick::GetMaxCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->max.x;
    Y = mpJoystickPtr->max.y;
  }
}

void CJoystick::SetMaxCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->max.x = X;
    mpJoystickPtr->max.y = Y;
  }
}

void CJoystick::GetMinCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->min.x;
    Y = mpJoystickPtr->min.y;
  }
}

void CJoystick::SetMinCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->min.x = X;
    mpJoystickPtr->min.y = Y;
  }
}

void CJoystick::GetCenterCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->center.x;
    Y = mpJoystickPtr->center.y;
  }
}

void CJoystick::SetCenterCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->center.x = X;
    mpJoystickPtr->center.y = Y;
  }
}

void CJoystick::GetPosition(float &Angle, float &Magnitude)
{
  if( mpJoystickPtr != NULL )
  {
    Angle = mpJoystickPtr->ang;
    Magnitude = mpJoystickPtr->mag;
  }
}

/*
 * CAccelerometer class methods.
 */

CAccelerometer::CAccelerometer() :
  mpAccelCalibPtr(NULL), mpAccelPtr(NULL), mpAccelThresholdPtr(NULL),
  mpOrientPtr(NULL), mpOrientThresholdPtr(NULL), mpGForcePtr(NULL)
{}

CAccelerometer::CAccelerometer( struct accel_t *AccelCalPtr, struct vec3b_t *AccelerationPtr, int *AccelThresholdPtr,
                                struct orient_t *OrientationPtr, float *OrientationThresholdPtr,
                                struct gforce_t *GForcePtr ) :
  mpAccelCalibPtr(AccelCalPtr), mpAccelPtr(AccelerationPtr), mpOrientPtr(OrientationPtr),
  mpGForcePtr(GForcePtr), mpAccelThresholdPtr(AccelThresholdPtr), mpOrientThresholdPtr(OrientationThresholdPtr)
{}

CAccelerometer::CAccelerometer( const CAccelerometer& ca ) :
  mpAccelCalibPtr(ca.mpAccelCalibPtr), mpAccelPtr(ca.mpAccelPtr), mpOrientPtr(ca.mpOrientPtr),
  mpGForcePtr(ca.mpGForcePtr), mpAccelThresholdPtr(ca.mpAccelThresholdPtr), mpOrientThresholdPtr(ca.mpOrientThresholdPtr)
{}

CAccelerometer::~CAccelerometer()
{
  // All data desallocate by wiiuse.
  mpAccelCalibPtr = NULL ;
  mpAccelPtr = NULL ;
  mpOrientPtr = NULL ;
  mpGForcePtr = NULL ;
  mpAccelThresholdPtr = NULL ;
  mpOrientThresholdPtr = NULL ;
}

float CAccelerometer::SetSmoothAlpha(float Alpha)
{
  float old_value=0.0 ;

  if( mpAccelCalibPtr != NULL )
  {
    old_value = mpAccelCalibPtr->st_alpha ;
    mpAccelCalibPtr->st_alpha = Alpha ;
  }

  return old_value;
}

float CAccelerometer::GetOrientThreshold()
{
  return (mpOrientThresholdPtr!=NULL ? *mpOrientThresholdPtr : 0.0f ) ;
}

void CAccelerometer::SetOrientThreshold(float Threshold)
{
  if( mpOrientThresholdPtr != NULL )
    *mpOrientThresholdPtr = Threshold ;
}

int CAccelerometer::GetAccelThreshold()
{
  return (mpAccelThresholdPtr!=NULL ? *mpAccelThresholdPtr : 0 ) ;
}

void CAccelerometer::SetAccelThreshold(int Threshold)
{
  if( mpAccelThresholdPtr != NULL )
    *mpAccelThresholdPtr = Threshold ;
}

void CAccelerometer::GetOrientation(float &Pitch, float &Roll, float &Yaw)
{
  if( mpOrientPtr != NULL )
  {
    Pitch = mpOrientPtr->pitch ;
    Roll = mpOrientPtr->roll ;
    Yaw = mpOrientPtr->yaw ;
  }
  else
  {
    Pitch = 0.0f ;
    Roll = 0.0f ;
    Yaw = 0.0f ;
  }
}
void CAccelerometer::GetRawOrientation(float &Pitch, float &Roll)
{
  if( mpOrientPtr != NULL )
  {
    Pitch = mpOrientPtr->a_pitch;
    Roll = mpOrientPtr->a_roll;
  }
  else
  {
    Pitch = 0.0 ;
    Roll = 0.0 ;
  }
}

void CAccelerometer::GetAccCalOne(float &X, float &Y, float &Z)
{
  if( mpAccelCalibPtr != NULL )
  {
    X = (float)(mpAccelCalibPtr->cal_g.x + mpAccelCalibPtr->cal_zero.x) ;
    Y = (float)(mpAccelCalibPtr->cal_g.y + mpAccelCalibPtr->cal_zero.y) ;
    Z = (float)(mpAccelCalibPtr->cal_g.z + mpAccelCalibPtr->cal_zero.z) ;
  }
  else
  {
    X = 0.0 ;
    Y = 0.0 ;
    Z = 0.0 ;
  }
}

void CAccelerometer::GetAccCalZero(float &X, float &Y, float &Z)
{
  if( mpAccelCalibPtr != NULL )
  {
    X = mpAccelCalibPtr->cal_zero.x;
    Y = mpAccelCalibPtr->cal_zero.y;
    Z = mpAccelCalibPtr->cal_zero.z;
  }
  else
  {
    X = 0.0 ;
    Y = 0.0 ;
    Z = 0.0 ;
  }
}

void CAccelerometer::GetGForce(float &X, float &Y, float &Z)
{
  if( mpGForcePtr != NULL )
  {
    X = mpGForcePtr->x;
    Y = mpGForcePtr->y;
    Z = mpGForcePtr->z;
  }
  else
  {
    X = 0.0 ;
    Y = 0.0 ;
    Z = 0.0 ;
  }
}

/*
 * CIRDot class methods.
 */

CIRDot::CIRDot() : mpDotPtr( NULL )
{}

CIRDot::CIRDot( struct ir_dot_t *DotPtr ) : mpDotPtr(DotPtr)
{}

// Copy constructor to handle pass by value.
CIRDot::CIRDot(const CIRDot &copyin) : mpDotPtr( copyin.mpDotPtr )
{}

CIRDot::~CIRDot()
{
  // All data desallocate by wiiuse.
  mpDotPtr = NULL ;
}

int CIRDot::isVisible()
{
  return (mpDotPtr!=NULL ? mpDotPtr->visible : 0 ) ;
}

int CIRDot::GetSize()
{
  return (mpDotPtr!=NULL ? mpDotPtr->size : 0 ) ;
}

int CIRDot::GetOrder()
{
  return (mpDotPtr!=NULL ? mpDotPtr->order : 0 ) ;
}

void CIRDot::GetCoordinate(int &X, int &Y)
{
  if( mpDotPtr != NULL )
  {
    X = mpDotPtr->x;
    Y = mpDotPtr->y;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

void CIRDot::GetRawCoordinate(int &X, int &Y)
{
  if( mpDotPtr != NULL )
  {
    X = mpDotPtr->rx ;
    Y = mpDotPtr->ry ;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

/*
 * CIR class methods.
 */

CIR::CIR() : mpWiimotePtr( NULL )
{
  //mpIRDotsVector.clear() ;
}

CIR::CIR(struct wiimote_t *wmPtr) : mpWiimotePtr( NULL )
{
  cout << mpIRDotsVector.capacity() << endl ;
  cout << mpIRDotsVector.empty() << endl ;
  cout << mpIRDotsVector.max_size() << endl ;
  cout << mpIRDotsVector.size() << endl ;
	mpIRDotsVector.push_back(0) ;
	mpIRDotsVector.clear() ;

  init( wmPtr ) ;
}

CIR::CIR( const CIR& ci ) : mpWiimotePtr( NULL )
{
  init( ci.mpWiimotePtr ) ;
}

CIR::~CIR()
{
  // mpWiimotePtr : delete by ~CWii.
  mpIRDotsVector.clear() ;
  mpWiimotePtr = NULL ;
}

void CIR::init( struct wiimote_t *wmPtr )
{
  mpWiimotePtr = wmPtr ;

  if( mpWiimotePtr != NULL )
  {
    CIRDot *dot=NULL ;
    for( int i=0 ; i<WIIUSECPP_NUM_IRDOTS ; i++ )
    {
      dot = new CIRDot( (struct ir_dot_t *) (&(mpWiimotePtr->ir.dot[i])) ) ;
      mpIRDotsVector.push_back( dot ) ;
    }
  }
}

void CIR::SetMode(CIR::OnOffSelection State)
{
    wiiuse_set_ir(mpWiimotePtr, State);
}

void CIR::SetVres(unsigned int x, unsigned int y)
{
    wiiuse_set_ir_vres(mpWiimotePtr, x, y);
}

CIR::BarPositions CIR::GetBarPositionSetting()
{
  if( mpWiimotePtr != NULL )
    return (CIR::BarPositions) mpWiimotePtr->ir.pos ;
  else
    return CIR::BAR_ABOVE ;
}

void CIR::SetBarPosition(CIR::BarPositions PositionSelection)
{
    wiiuse_set_ir_position( mpWiimotePtr, (ir_position_t) PositionSelection ) ;
}

CIR::AspectRatioSelections CIR::GetAspectRatioSetting()
{
  if( mpWiimotePtr != NULL )
    return (CIR::AspectRatioSelections) mpWiimotePtr->ir.aspect ;
  else
    return CIR::ASPECT_4_3 ;
}

void CIR::SetAspectRatio( CIR::AspectRatioSelections AspectRatioSelection )
{
    wiiuse_set_aspect_ratio( mpWiimotePtr, (enum aspect_t) AspectRatioSelection ) ;
}

void CIR::SetSensitivity(int Level)
{
    wiiuse_set_ir_sensitivity( mpWiimotePtr, Level ) ;
}

int CIR::GetSensitivity()
{
  int level=0 ;

  if( mpWiimotePtr != NULL )
  {
    if( mpWiimotePtr->state & 0x0200 )
      level = 1;
    else if( mpWiimotePtr->state & 0x0400 )
      level = 2;
    else if( mpWiimotePtr->state & 0x0800 )
      level = 3;
    else if( mpWiimotePtr->state & 0x1000 )
      level = 4;
    else if( mpWiimotePtr->state & 0x2000 )
      level = 5;
  }
  
  return level;
}

int CIR::GetNumDots()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.num_dots : 0 ) ;
}

int CIR::GetNumSources()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.nb_source_detect : 0 ) ;
}

std::vector<CIRDot*>& CIR::GetDots()
{
  /*
    int index;

    // Empty the array of irdots before reloading
    mpIRDotsVector.clear();

    for(index = 0; index < mpWiimotePtr->ir.num_dots; index++)
    {
        CIRDot dot((struct ir_dot_t *) (&(mpWiimotePtr->ir.dot[index])));
        mpIRDotsVector.push_back(dot);
    }

    //return numConnected;
    */
    return mpIRDotsVector;
}

void CIR::GetOffset(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.offset[0];
    Y = mpWiimotePtr->ir.offset[1];
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

int CIR::GetState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.state : 0 ) ;
}

void CIR::GetCursorPositionAbsolute(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.ax;
    Y = mpWiimotePtr->ir.ay;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

void CIR::GetCursorPosition(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.x ;
    Y = mpWiimotePtr->ir.y ;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

float CIR::GetPixelDistance()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.distance : 0.0f ) ;
}

float CIR::GetDistance()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.z : 0.0f ) ;
}


/*
 * CExpansionDevice class methods.  This is a container class so there is not much.
 */
CExpansionDevice::CExpansionDevice() : 
  Nunchuk(NULL), Classic(NULL), GuitarHero3(NULL), mpExpansionPtr( NULL )
{}

CExpansionDevice::CExpansionDevice(struct expansion_t *ExpPtr) :
  Nunchuk(ExpPtr), Classic(ExpPtr), GuitarHero3(ExpPtr), mpExpansionPtr(ExpPtr)
{}
    
CExpansionDevice::CExpansionDevice( const CExpansionDevice& ced ) :
  Nunchuk(ced.mpExpansionPtr), Classic(ced.mpExpansionPtr), GuitarHero3(ced.mpExpansionPtr), mpExpansionPtr(ced.mpExpansionPtr)
{}

CExpansionDevice::~CExpansionDevice()
{
  // All data desallocate by wiiuse.
  mpExpansionPtr = NULL ;
}

CExpansionDevice::ExpTypes CExpansionDevice::GetType()
{
  return (mpExpansionPtr!=NULL ? (CExpansionDevice::ExpTypes) (mpExpansionPtr->type) : CExpansionDevice::TYPE_NONE ) ;
}

/*
 * CNunchuk class methods.
 */

CNunchuk::CNunchuk() : Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{}

CNunchuk::CNunchuk( struct expansion_t *ExpPtr ) :
   Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{
  if( ExpPtr != NULL )
    mpNunchukPtr = &(ExpPtr->nunchuk) ;
  else
    mpNunchukPtr = NULL ;

  init( mpNunchukPtr ) ;
}

CNunchuk::CNunchuk( const CNunchuk& cn ) :
   Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{
  mpNunchukPtr = cn.mpNunchukPtr ;
  init( mpNunchukPtr ) ;
}

CNunchuk::~CNunchuk()
{
  mpNunchukPtr = NULL ;
}

void CNunchuk::init( struct nunchuk_t *n )
{
  if( n != NULL )
  {
    Buttons = CNunchukButtons( (void*) &(n->btns), (void*) &(n->btns_held), (void*) &(n->btns_released) ) ;
    Joystick = CJoystick( &(n->js) ) ;
    Accelerometer = CAccelerometer( &(n->accel_calib), &(n->accel), &(n->accel_threshold), 
                                    &(n->orient), &(n->orient_threshold), &(n->gforce) ) ;
  }
  else
  {
    Buttons = CNunchukButtons() ;
    Joystick = CJoystick() ;
    Accelerometer = CAccelerometer() ;
  }
}


/*
 * CClassic class methods.
 */

CClassic::CClassic() : Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{}

CClassic::CClassic(struct expansion_t *ExpPtr) :
  Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{
  if( ExpPtr != NULL )
    mpClassicPtr = &(ExpPtr->classic) ;
  else
    mpClassicPtr = NULL ;

  init( mpClassicPtr ) ;
}

CClassic::CClassic( const CClassic& cc ) :
  Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{
  mpClassicPtr = cc.mpClassicPtr ;
  init( mpClassicPtr ) ;
}

CClassic::~CClassic()
{
  mpClassicPtr = NULL ;
}

void CClassic::init( struct classic_ctrl_t *c )
{
  if( c != NULL )
  {
    Buttons = CClassicButtons( (void*) &(c->btns), (void*) &(c->btns_held), (void*) &(c->btns_released) ) ;
    LeftJoystick = CJoystick( &(c->ljs) ) ;
    RightJoystick = CJoystick( &(c->rjs) ) ;
  }
  else
  {
    Buttons = CClassicButtons() ;
    LeftJoystick = CJoystick() ;
    RightJoystick = CJoystick() ;
  }
}

float CClassic::GetLShoulderButton()
{
  if( mpClassicPtr != NULL )
    return mpClassicPtr->l_shoulder ;
  else
    return 0.0f ;
}

float CClassic::GetRShoulderButton()
{   
  if( mpClassicPtr != NULL )
    return mpClassicPtr->r_shoulder ;
  else
    return 0.0f ;
}

/*
 * CGuitarHero3 class methods.
 */

CGuitarHero3::CGuitarHero3() : Buttons(), Joystick(), mpGH3Ptr(NULL)
{}

CGuitarHero3::CGuitarHero3( struct expansion_t *ExpPtr ) : Buttons(), Joystick(), mpGH3Ptr(NULL)
{
  if( ExpPtr != NULL )
    mpGH3Ptr = &(ExpPtr->gh3) ;
  else
    mpGH3Ptr = NULL ;

  init( mpGH3Ptr ) ;
}

CGuitarHero3::CGuitarHero3( const CGuitarHero3& cgh ) : Buttons(), Joystick(), mpGH3Ptr(NULL)
{
  mpGH3Ptr = cgh.mpGH3Ptr ;
  init( mpGH3Ptr ) ;
}

void CGuitarHero3::init( struct guitar_hero_3_t *g )
{
  if( g != NULL )
  {
    Buttons = CGH3Buttons( (void*) &(g->btns), (void*) &(g->btns_held), (void*) &(g->btns_released) ) ;
    Joystick = CJoystick( &(g->js) ) ;
  }
  else
  {
    Buttons = CGH3Buttons() ;
    Joystick = CJoystick() ;
  }
}

float CGuitarHero3::GetWhammyBar()
{
  if( mpGH3Ptr != NULL )
    return mpGH3Ptr->whammy_bar ;
  else
    return 0.0f ;
} 

/*
 * CWiimote class methods.
 */

CWiimote::CWiimote() : // SWIG insisted it exist for the vectors. Hopefully it will only be used for copy.
  IR(), Buttons(), Accelerometer( NULL, NULL, &mpTempInt, NULL, &mpTempFloat, NULL ),
  ExpansionDevice(), mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0)
{
  puts( "Constructor CWiimote par défaut" ) ;
}

CWiimote::CWiimote(struct wiimote_t *wmPtr) :
  IR(), Buttons(), Accelerometer(), ExpansionDevice(), 
  mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0)
{
  puts( "Constructor CWiimote par argument" ) ;
  if( wmPtr != NULL )
    mpWiimotePtr = wmPtr ;
  else
    mpWiimotePtr = NULL ;

  init( mpWiimotePtr  ) ;
}

CWiimote::CWiimote(const CWiimote &copyin) : // Copy constructor to handle pass by value.
  IR(), Buttons(), Accelerometer(), ExpansionDevice(), 
  mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0)
{
  puts( "Constructor CWiimote par copie" ) ;
  mpWiimotePtr = copyin.mpWiimotePtr ;
  init( mpWiimotePtr ) ;
}

CWiimote::~CWiimote()
{
  puts( "Destructor CWiimote" ) ;
  // mpWiimotePtr : delete by ~CWii.
  mpWiimotePtr = NULL ;
}

void CWiimote::init( struct wiimote_t *wmPtr )
{
  mpTempInt = 0 ;
  mpTempFloat = 0 ;

  if( wmPtr != NULL )
  {
    IR = CIR(wmPtr) ;
    Buttons = CButtons( (void*) &(wmPtr->btns), (void*) &(wmPtr->btns_held), (void*) &(wmPtr->btns_released) ) ;
    Accelerometer = CAccelerometer( (accel_t*) &(wmPtr->accel_calib), (vec3b_t*) &(wmPtr->accel),
                                    (int*) &(wmPtr->accel_threshold), (orient_t*) &(wmPtr->orient),
                                    (float*) &(wmPtr->orient_threshold), (gforce_t*) &(wmPtr->gforce) ) ;
    ExpansionDevice = CExpansionDevice( (struct expansion_t*) &(wmPtr->exp) ) ;
  }
  else
  {
    IR = CIR() ;
    Buttons = CButtons() ;
    Accelerometer = CAccelerometer( NULL, NULL, &mpTempInt, NULL, &mpTempFloat, NULL ) ;
    ExpansionDevice = CExpansionDevice() ;
  }
}

void CWiimote::Disconnected()
{
    wiiuse_disconnected(mpWiimotePtr);
}

void CWiimote::SetRumbleMode(CWiimote::OnOffSelection State)
{
    wiiuse_rumble(mpWiimotePtr, State);
}

void CWiimote::ToggleRumble()
{
    wiiuse_toggle_rumble(mpWiimotePtr);
}

int CWiimote::GetLEDs()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->leds : 0 ) ;
}

void CWiimote::SetLEDs(int LEDs)
{
    wiiuse_set_leds(mpWiimotePtr, LEDs);
}

float CWiimote::GetBatteryLevel()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->battery_level : 0.0f ) ;
}

int CWiimote::GetHandshakeState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->handshake_state : 0 ) ;
}

CWiimote::EventTypes CWiimote::GetEvent()
{
  return (mpWiimotePtr!=NULL ? (CWiimote::EventTypes) mpWiimotePtr->event : CWiimote::EVENT_NONE) ;
}

const unsigned char *CWiimote::GetEventBuffer()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->event_buf : NULL ) ;
}

void CWiimote::SetMotionSensingMode(CWiimote::OnOffSelection State)
{
    wiiuse_motion_sensing(mpWiimotePtr, State);
}

void CWiimote::ReadData(unsigned char *Buffer, unsigned int Offset, unsigned int Length)
{
    wiiuse_read_data(mpWiimotePtr, Buffer, Offset, Length);
}

void CWiimote::WriteData(unsigned int Address, unsigned char *Data, unsigned int Length)
{
    wiiuse_write_data(mpWiimotePtr, Address, Data, Length);
}

void CWiimote::UpdateStatus()
{
    wiiuse_status(mpWiimotePtr);
}

int CWiimote::GetID()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->unid : 0 ) ;
}

int CWiimote::GetState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->state : 0 ) ;
}

int CWiimote::GetFlags()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->flags : 0 ) ;
}

int CWiimote::SetFlags(int Enable, int Disable)
{
    return wiiuse_set_flags(mpWiimotePtr, Enable, Disable);
}

void CWiimote::Resync()
{
    wiiuse_resync(mpWiimotePtr);
}

void CWiimote::Disconnect()
{
    wiiuse_disconnect(mpWiimotePtr);
}

int CWiimote::isUsingACC()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0020) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingEXP()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0040) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingIR()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0080) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingSpeaker()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0100) != 0 ;
  else
    return 0 ;
}

int CWiimote::isLEDSet(int LEDNum)
{
  int result = 0;

  if( mpWiimotePtr != NULL )
  {
    switch(LEDNum)
    {
      case 1:
        result = (mpWiimotePtr->leds & LED_1) != 0;
        break;
      case 2:
        result = (mpWiimotePtr->leds & LED_2) != 0;
        break;
      case 3:
        result = (mpWiimotePtr->leds & LED_3) != 0;
        break;
      case 4:
        result = (mpWiimotePtr->leds & LED_4) != 0;
      default:
        result = 0;
    }
  }
  return result;
}

/*
 * Wii Class Methods
 */

CWii::CWii() : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  puts( "Constructor CWii par défaut" ) ;
  init( WIIUSECPP_DEFAULT_NUM_WM ) ;
}

CWii::CWii(int MaxNumWiimotes ) : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  puts( "Constructor CWii par arguments" ) ;
  init( MaxNumWiimotes ) ;
}

CWii::CWii( const CWii& cw ) : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  mpWiimoteArraySize = cw.mpWiimoteArraySize ;
  mpWiimoteArray = cw.mpWiimoteArray ;
  mpWiimotesVector = cw.mpWiimotesVector ;
}

CWii::~CWii()
{
  puts( "Destructor CWii" ) ;
  wiiuse_cleanup((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize);

  CWiimote *wm=NULL ;
  for( int i=0 ; i<mpWiimoteArraySize ; i++ )
  {
    wm = mpWiimotesVector.at(i) ;
    delete( wm ) ;
    wm = NULL ;
    mpWiimotesVector.at(i) = NULL ;
  }

  mpWiimotesVector.clear() ;
}

void CWii::init( int nbWm )
{
  if( nbWm>0 && nbWm<=WIIUSECPP_MAX_NUM_WM )  
    mpWiimoteArraySize = nbWm ;
  else
    mpWiimoteArraySize = WIIUSECPP_DEFAULT_NUM_WM ;

  mpWiimoteArray = wiiuse_init(mpWiimoteArraySize);
  //mpWiimotesVector.clear() ;
  //mpWiimotesVector.reserve(mpWiimoteArraySize) ; // Useless to use more memories.
  //mpWiimotesVector.assign( 1, CWiimote(mpWiimoteArray[0]) ) ;

  CWiimote *wm=NULL ;
  for( int i=0 ; i<mpWiimoteArraySize ; i++ )
  {
    wm = new CWiimote(mpWiimoteArray[i]) ;
    mpWiimotesVector.push_back( wm ) ;
  }

  cout << "wiiusecpp / pywii by Jim Thomas (jt@missioncognition.net)" << endl ;
  cout << "Download from http://missioncognition.net/" << endl ;
}

void CWii::RefreshWiimotes()
{
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    // This approach is a bit wasteful but it will work.  The other
    // option is to force the user to handle disconnect events to remove
    // wiimotes from the array.
    mpWiimotesVector.clear();

    for( int i=0 ; i<mpWiimoteArraySize ; i++ )
    {
        if( (mpWiimoteArray[i]->state & 0x0008) != 0 )
        {
          CWiimote *wm = new CWiimote( mpWiimoteArray[i] ) ;
          mpWiimotesVector.push_back(wm);
        }
    }
  }
}

int CWii::GetNumConnectedWiimotes()
{
  int count=0 ;
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    for( int i=0 ; i<mpWiimoteArraySize ; i++ )
    {
      if( (mpWiimoteArray[i]->state & 0x0008) != 0 )
        count++;
    }
  }
  return count;
}

CWiimote* CWii::GetByID( int UnID, int Refresh )
{
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    if( Refresh )
        RefreshWiimotes();

    //int cpt=0 ;
    for( std::vector<CWiimote*>::iterator i=mpWiimotesVector.begin() ; i!=mpWiimotesVector.end() ; ++i )
		{
			if( (*i)->GetID() == UnID )
			{
        //cout << "GetByID:cpt:" << cpt << endl ;
				return *i;
			}
			//cpt ++ ;
    }

    return *mpWiimotesVector.begin(); // Return the first one if it was not found.
  }
  else
    return NULL ;
}

/*
CWiimote& CWii::GetByID( int UnID )
{
	CWiimote *wm=NULL ;

	if( UnID>=0 && UnID<4 )
		wm = CWiimote( mpWiimoteArray[UnID] )  ; // No no no ...
	else
		wm = CWiimote( mpWiimoteArray[0] )  ;

	return *wm ;
}*/

std::vector<CWiimote*>& CWii::GetWiimotes(int Refresh)
{
    if(Refresh)
        RefreshWiimotes();

    return mpWiimotesVector;
}

void CWii::SetBluetoothStack(CWii::BTStacks Type)
{
    wiiuse_set_bluetooth_stack((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, (win_bt_stack_t) Type);
}

void CWii::SetTimeout(int NormalTimeout, int ExpTimeout)
{
    wiiuse_set_timeout((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, NormalTimeout, ExpTimeout);
}

int CWii::Find(int Timeout)
{
    return wiiuse_find((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, Timeout);
}

std::vector<CWiimote*>& CWii::Connect()
{
    wiiuse_connect((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize);
    return mpWiimotesVector;
}

int CWii::Poll()
{
    return wiiuse_poll((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize);
}
