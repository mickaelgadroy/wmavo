/*
 *    wiiusecpp.cpp
 *
 *    Written By:
 *        James Thomas
 *        Email: jt@missioncognition.org
 *
 *    Copyright 2009
 *
 *    Copyright (c) 2010 Mickael Gadroy
 *
 *    This file is part of wiiusecpp.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WIIUSECPP_H_
#define WIIUSECPP_H_

#include <vector>

#include <wiiuse.h>

class CButtonBase
{
public:
    CButtonBase(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr);

    int isPressed(int Button);
    int isHeld(int Button);
    int isReleased(int Button);
    int isJustPressed(int Button);

private:
    virtual short Cast(void *Ptr) {return *((short *)(Ptr));} // Inlined.

    void *mpBtnsPtr;
    void *mpBtnsHeldPtr;
    void *mpBtnsReleasedPtr;
};

class CButtons : public CButtonBase
{
public:
    enum ButtonDefs
    {
        BUTTON_TWO = WIIMOTE_BUTTON_TWO,
        BUTTON_ONE = WIIMOTE_BUTTON_ONE,
        BUTTON_B = WIIMOTE_BUTTON_B,
        BUTTON_A = WIIMOTE_BUTTON_A,
        BUTTON_MINUS = WIIMOTE_BUTTON_MINUS,
        BUTTON_HOME = WIIMOTE_BUTTON_HOME,
        BUTTON_LEFT = WIIMOTE_BUTTON_LEFT,
        BUTTON_RIGHT = WIIMOTE_BUTTON_RIGHT,
        BUTTON_DOWN = WIIMOTE_BUTTON_DOWN,
        BUTTON_UP = WIIMOTE_BUTTON_UP,
        BUTTON_PLUS = WIIMOTE_BUTTON_PLUS,
        BUTTON_UNKNOWN = WIIMOTE_BUTTON_UNKNOWN,
        BUTTON_ALL = WIIMOTE_BUTTON_ALL
    };

    CButtons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr);
};

class CNunchukButtons : public CButtonBase
{
public:
    enum ButtonDefs
    {
        BUTTON_Z = NUNCHUK_BUTTON_Z,
        BUTTON_C = NUNCHUK_BUTTON_C,
        BUTTON_ALL = NUNCHUK_BUTTON_ALL
    };

    CNunchukButtons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr);

private:
    short Cast(void *Ptr) {return (short)(*((byte *)(Ptr)));} // Inlined using the different type.
};

class CClassicButtons : public CButtonBase
{
public:
    enum ButtonDefs
    {
        BUTTON_X = CLASSIC_CTRL_BUTTON_X,
        BUTTON_Y = CLASSIC_CTRL_BUTTON_Y,
        BUTTON_B = CLASSIC_CTRL_BUTTON_B,
        BUTTON_A = CLASSIC_CTRL_BUTTON_A,
        BUTTON_MINUS = CLASSIC_CTRL_BUTTON_MINUS,
        BUTTON_HOME = CLASSIC_CTRL_BUTTON_HOME,
        BUTTON_LEFT = CLASSIC_CTRL_BUTTON_LEFT,
        BUTTON_RIGHT = CLASSIC_CTRL_BUTTON_RIGHT,
        BUTTON_DOWN = CLASSIC_CTRL_BUTTON_DOWN,
        BUTTON_UP = CLASSIC_CTRL_BUTTON_UP,
        BUTTON_PLUS = CLASSIC_CTRL_BUTTON_PLUS,
        BUTTON_ZR = CLASSIC_CTRL_BUTTON_ZR,
        BUTTON_ZL = CLASSIC_CTRL_BUTTON_ZL,
        BUTTON_FULL_R = CLASSIC_CTRL_BUTTON_FULL_R,
        BUTTON_FULL_L = CLASSIC_CTRL_BUTTON_FULL_L,
        BUTTON_ALL = CLASSIC_CTRL_BUTTON_ALL
    };

    CClassicButtons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr);
};

class CGH3Buttons : public CButtonBase
{
public:
    enum ButtonDefs
    {
        BUTTON_STRUM_UP = GUITAR_HERO_3_BUTTON_STRUM_UP,
        BUTTON_STRUM_DOWN = GUITAR_HERO_3_BUTTON_STRUM_DOWN,
        BUTTON_YELLOW = GUITAR_HERO_3_BUTTON_YELLOW,
        BUTTON_GREEN = GUITAR_HERO_3_BUTTON_GREEN,
        BUTTON_BLUE = GUITAR_HERO_3_BUTTON_BLUE,
        BUTTON_RED = GUITAR_HERO_3_BUTTON_RED,
        BUTTON_ORANGE = GUITAR_HERO_3_BUTTON_ORANGE,
        BUTTON_MINUS = GUITAR_HERO_3_BUTTON_MINUS,
        BUTTON_PLUS = GUITAR_HERO_3_BUTTON_PLUS,
        BUTTON_ALL = GUITAR_HERO_3_BUTTON_ALL
    };

    CGH3Buttons(void *ButtonsPtr, void *ButtonsHeldPtr, void *ButtonsReleasedPtr);
};

class CJoystick
{
public:
    CJoystick(struct joystick_t *JSPtr);

    void GetMaxCal(int &X, int &Y);
    void SetMaxCal(int X, int Y);

    void GetMinCal(int &X, int &Y);
    void SetMinCal(int X, int Y);

    void GetCenterCal(int &X, int &Y);
    void SetCenterCal(int X, int Y);

    void GetPosition(float &Angle, float &Magnitude);

private:
    struct joystick_t *mpJoystickPtr;
};

class CAccelerometer
{
public:
    CAccelerometer( struct accel_t *AccelCalPtr, struct vec3b_t *AccelerationPtr,
                    int *AccelThresholdPtr, struct orient_t *OrientationPtr, 
                    float *OrientationThresholdPtr, struct gforce_t *GForcePtr );
    
    /* Initiate like that :
      Accelerometer( (accel_t*) &(wmPtr->accel_calib), (vec3b_t*) &(wmPtr->accel),
                     (int*) &(wmPtr->accel_threshold), (orient_t*) &(wmPtr->orient),
                     (float*) &(wmPtr->orient_threshold), (gforce_t*) &(wmPtr->gforce) )
    */

    /**
      * wiiuse.accel_t.st_alĥa
      * Use to smooth the angle values when flags FLAGS_SMOOTHING is on.
      * @return Old value of alpha.
      */
    float SetSmoothAlpha(float Alpha);

    /**
      * wiiuse.wiimote_t.orient_threshold
      * Threshold ((fr:)seuil) for orient to generate an event.
      */
    float GetOrientThreshold();
    void SetOrientThreshold(float Threshold);

    /**
      * wiiuse.wiimote_t.accel_threshold
      * Threshold for accel to generate an event.
      */
    int GetAccelThreshold();
    void SetAccelThreshold(int Threshold);

    /**
      * wiiuse.orient_t.roll .pitch .yaw
      * Get value for pitch, roll (and yaw when it is possible).
      * This value can be smooth when flags FLAGS_SMOOTHING is on.
      */
    void GetOrientation(float &Pitch, float &Roll, float &Yaw);
    
    /**
      * wiiuse.orient_t.roll .pitch
      * Get value for pitch, roll.
      * This value are unsmoothed.
      */
    void GetRawOrientation(float &Pitch, float &Roll);

    /** wiiuse.accel_t.cal_zero.x .y .z : the "zero g"
      * wiiuse.accel_t.cal_g.x .y .z    : the result of ("one g" - "zero g")
      *
      * Represent the accel calibrated values get from the Wiimote.
      * This data are necessary to get acc values in g unit from the hex values 
      * provide ((fr:)fournie) of the Wiimote.
      * Here, it provide just for informations.
      * The acc calibrated zero values represent where are the "zero g" position.
      * The acc calibrated one values represent where are the "one g" position.
      */
    void GetAccCalOne(float &X, float &Y, float &Z);
    void GetAccCalZero(float &X, float &Y, float &Z);

    /*
     * wiiuse.gforce_t.x .y .z
     * Get GForce on 3 axes in g units. To get in m.s^-2, mult by ~9.8 .
     */
    void GetGForce(float &X, float &Y, float &Z) ;
    
    /**
      * Difference between the last and the current gForce values.
      * It lets to know the user movement in theory ...
      */
    //void GetInstantAcc(float &X, float &Y, float &Z) ;
    
    /**
      * Inform on the accel that the Wiimote has globaly.
      */
    //float GetAccGlobal() ;

private:
    struct accel_t *mpAccelCalibPtr;
    struct vec3b_t *mpAccelPtr;
    struct orient_t *mpOrientPtr;
    struct gforce_t *mpGForcePtr;
    int *mpAccelThresholdPtr;
    float *mpOrientThresholdPtr;
};

class CIRDot
{
public:
    CIRDot();
    CIRDot(struct ir_dot_t *DotPtr);
    CIRDot(const CIRDot & copyin);

    int isVisible();
    int GetSize();
    int GetOrder();
    void GetCoordinate(int &X, int &Y);
    void GetRawCoordinate(int &X, int &Y);

private:
    struct ir_dot_t *mpDotPtr;
};

class CIR
{
public:
    enum BarPositions
    {
        BAR_ABOVE = WIIUSE_IR_ABOVE,
        BAR_BELOW = WIIUSE_IR_BELOW
    };

    enum AspectRatioSelections
    {
        ASPECT_4_3 = WIIUSE_ASPECT_4_3,
        ASPECT_16_9 = WIIUSE_ASPECT_16_9
    };

    enum OnOffSelection
    {
        OFF = 0,
        ON = 1
    };

    CIR(struct wiimote_t *wmPtr);

    void SetMode(OnOffSelection State);
    void SetVres(unsigned int x, unsigned int y);

    BarPositions GetBarPositionSetting();
    void SetBarPosition(BarPositions PositionSelection);

    AspectRatioSelections GetAspectRatioSetting();
    void SetAspectRatio(AspectRatioSelections AspectRatioSelection );

    void SetSensitivity(int Level);
    int GetSensitivity();

    int GetNumDots();
    int GetNumSources() ;
    std::vector<CIRDot>& GetDots();

    void GetOffset(int &X, int &Y);
    int GetState();
    void GetCursorPositionAbsolute(int &X, int &Y);
    void GetCursorPosition(int &X, int &Y);
    float GetPixelDistance();
    float GetDistance();

private:
    struct wiimote_t *mpWiimotePtr;
    std::vector<CIRDot> mpIRDotsVector;
};

class CNunchuk
{
public:
    CNunchuk(struct expansion_t *ExpPtr);

    CNunchukButtons Buttons;
    CJoystick Joystick;
    CAccelerometer Accelerometer;

private:
    struct nunchuk_t *mpNunchukPtr;
};

class CClassic
{
public:
    CClassic(struct expansion_t *ExpPtr);

    float GetLShoulderButton();
    float GetRShoulderButton();

    CClassicButtons Buttons;
    CJoystick LeftJoystick;
    CJoystick RightJoystick;

private:
    struct classic_ctrl_t *mpClassicPtr;
};

class CGuitarHero3
{
public:
    CGuitarHero3(struct expansion_t *ExpPtr);

    float GetWhammyBar();

    CGH3Buttons Buttons;
    CJoystick Joystick;

private:
    struct guitar_hero_3_t *mpGH3Ptr;
};

class CExpansionDevice
{
public:
    enum ExpTypes
    {
        TYPE_NONE = EXP_NONE,
        TYPE_NUNCHUK = EXP_NUNCHUK,
        TYPE_CLASSIC = EXP_CLASSIC,
        TYPE_GUITAR_HERO_3 = EXP_GUITAR_HERO_3
    };

    CExpansionDevice(struct expansion_t *ExpPtr);

    ExpTypes GetType();

    CNunchuk Nunchuk;
    CClassic Classic;
    CGuitarHero3 GuitarHero3;

private:
    struct expansion_t *mpExpansionPtr;
};


class CWiimote
{
public:
    enum LEDS
    {
        LED_NONE = WIIMOTE_LED_NONE,
        LED_1 = WIIMOTE_LED_1,
        LED_2 = WIIMOTE_LED_2,
        LED_3 = WIIMOTE_LED_3,
        LED_4 = WIIMOTE_LED_4

    };

    enum Flags
    {
        FLAG_SMOOTHING = WIIUSE_SMOOTHING,
        FLAG_CONTINUOUS = WIIUSE_CONTINUOUS,
        FLAG_ORIENT_THRESH = WIIUSE_ORIENT_THRESH,
        FLAG_INIT_FLAGS = WIIUSE_INIT_FLAGS
    };

    enum EventTypes
    {
        EVENT_NONE = WIIUSE_NONE,
        EVENT_EVENT = WIIUSE_EVENT,
        EVENT_STATUS = WIIUSE_STATUS,
        EVENT_CONNECT = WIIUSE_CONNECT,
        EVENT_DISCONNECT = WIIUSE_DISCONNECT,
        EVENT_UNEXPECTED_DISCONNECT = WIIUSE_UNEXPECTED_DISCONNECT,
        EVENT_READ_DATA = WIIUSE_READ_DATA,
        EVENT_NUNCHUK_INSERTED = WIIUSE_NUNCHUK_INSERTED,
        EVENT_NUNCHUK_REMOVED = WIIUSE_NUNCHUK_REMOVED,
        EVENT_CLASSIC_CTRL_INSERTED = WIIUSE_CLASSIC_CTRL_INSERTED,
        EVENT_CLASSIC_CTRL_REMOVED = WIIUSE_CLASSIC_CTRL_REMOVED,
        EVENT_GUITAR_HERO_3_CTRL_INSERTED = WIIUSE_GUITAR_HERO_3_CTRL_INSERTED,
        EVENT_GUITAR_HERO_3_CTRL_REMOVED = WIIUSE_GUITAR_HERO_3_CTRL_REMOVED
    };

    enum OnOffSelection
    {
        OFF = 0,
        ON = 1
    };

    static const int EVENT_BUFFER_LENGTH = MAX_PAYLOAD;
    static const float ORIENT_PRECISION = WIIUSE_ORIENT_PRECISION;

    CWiimote();
    CWiimote(struct wiimote_t *wmPtr);
    CWiimote(const CWiimote & copyin);

    void Disconnected();

    void SetRumbleMode(OnOffSelection State);
    void ToggleRumble();

    int GetLEDs();
    void SetLEDs(int LEDs);

    float GetBatteryLevel();

    int GetHandshakeState();

    EventTypes GetEvent();
    const unsigned char *GetEventBuffer();

    void SetMotionSensingMode(OnOffSelection State);

    void ReadData(unsigned char *Buffer, unsigned int Offset, unsigned int Length);
    void WriteData(unsigned int Address, unsigned char *Data, unsigned int Length);

    void UpdateStatus();

    int GetID();

    int GetState();

    int GetFlags();
    int SetFlags(int Enable, int Disable);

    void Resync();

    void Disconnect();

    int isUsingACC();
    int isUsingEXP();
    int isUsingIR();
    int isUsingSpeaker();
    int isLEDSet(int LEDNum);

    CIR IR;
    CButtons Buttons;
    CAccelerometer Accelerometer;
    CExpansionDevice ExpansionDevice;

private:
    /* The pointer to the wm structure */
    struct wiimote_t *mpWiimotePtr;
    int mpTempInt;
    float mpTempFloat;
};

class CWii
{
public:
    enum BTStacks
    {
        STACK_UNKNOWN = WIIUSE_STACK_UNKNOWN,
        STACK_MS = WIIUSE_STACK_MS,
        STACK_BLUESOLEIL = WIIUSE_STACK_BLUESOLEIL
    };

    CWii();
    CWii(int MaxNumCWiimotes);
    virtual ~CWii();

    int GetNumConnectedWiimotes();

    void RefreshWiimotes();

    CWiimote& GetByID(int ID, int Refresh=1);
		//CWiimote& GetByID(int ID); // Without pass by mpWiimoteVector.
    std::vector<CWiimote>& GetWiimotes(int Refresh=1);

    void SetBluetoothStack(BTStacks Type);
    void SetTimeout(int NormalTimeout, int ExpTimeout);

    int Find(int timeout);
    std::vector<CWiimote>& Connect();

    int Poll();

//protected:


private:
    struct wiimote_t **mpWiimoteArray;
    int mpWiimoteArraySize;
    std::vector<CWiimote> mpWiimotesVector;
};

#endif /* WIIUSECPP_H_ */
