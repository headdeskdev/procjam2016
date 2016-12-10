#include <xinput.h>

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static void platform_loadXInput(void)    
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}


    }
}



static void platform_processXInputDigitalButton(DWORD XInputButtonState,
                                platform_ButtonState *OldState, DWORD ButtonBit,
                                platform_ButtonState *NewState)
{
    NewState->isDown = ((XInputButtonState & ButtonBit) != 0);
    NewState->halfTransitionCount = (OldState->isDown != NewState->isDown) ? 1 : 0;
}

static F32 platform_processXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    F32 Result = 0;

    if(Value < -DeadZoneThreshold)
    {
        Result = (F32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (F32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(Result);
}


static void platform_getControllerInput(platform_ControllerState* oldController, platform_ControllerState* newController) {
    XINPUT_STATE ControllerState;
    
    if(XInputGetState(0, &ControllerState) == ERROR_SUCCESS)
    {
        newController->isEnabled = true;
        newController->analogEnabled = oldController->analogEnabled;
           
        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

        newController->leftStickX = platform_processXInputStickValue(
            Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        newController->leftStickY = platform_processXInputStickValue(
            Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        newController->rightStickX = platform_processXInputStickValue(
            Pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        newController->rightStickY = platform_processXInputStickValue(
            Pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);


        if((newController->leftStickX != 0.0f) ||
           (newController->leftStickY != 0.0f))
        {
            newController->analogEnabled = true;
        }


        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->directionDown, XINPUT_GAMEPAD_DPAD_DOWN,
                                        &newController->directionDown);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->directionRight, XINPUT_GAMEPAD_DPAD_RIGHT,
                                        &newController->directionRight);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->directionLeft, XINPUT_GAMEPAD_DPAD_LEFT,
                                        &newController->directionLeft);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->directionUp, XINPUT_GAMEPAD_DPAD_UP,
                                        &newController->directionUp);

        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->buttonDown, XINPUT_GAMEPAD_A,
                                        &newController->buttonDown);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->buttonRight, XINPUT_GAMEPAD_B,
                                        &newController->buttonRight);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->buttonLeft, XINPUT_GAMEPAD_X,
                                        &newController->buttonLeft);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->buttonUp, XINPUT_GAMEPAD_Y,
                                        &newController->buttonUp);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->shoulderLeftTop, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                        &newController->shoulderLeftTop);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->shoulderRightTop, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                        &newController->shoulderRightTop);
        platform_processXInputDigitalButton((Pad->bLeftTrigger > 127) ? 1 : 0,
                                        &oldController->shoulderLeftBottom, 1,
                                        &newController->shoulderLeftBottom);
        platform_processXInputDigitalButton((Pad->bRightTrigger > 127) ? 1 : 0,
                                        &oldController->shoulderRightBottom, 1,
                                        &newController->shoulderRightBottom);

        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->centralRight, XINPUT_GAMEPAD_START,
                                        &newController->centralRight);
        platform_processXInputDigitalButton(Pad->wButtons,
                                        &oldController->centralLeft, XINPUT_GAMEPAD_BACK,
                                        &newController->centralLeft);
    }
    else
    {
        newController->isEnabled = false;
    }
}