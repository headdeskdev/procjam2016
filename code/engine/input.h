#define BUTTON_WAS_PRESSED(b) ((b.halfTransitionCount + (b.isDown ? 1 : 0)) > 1)
#define BUTTON_WAS_LIFTED(b) ((b.halfTransitionCount + (b.isDown ? 0 : 1)) > 1)
#define BUTTON_WAS_CLICKED(b) ((b.halfTransitionCount + (b.isDown ? 0 : 1)) > 2)
struct platform_ButtonState {
  U8 halfTransitionCount;
  bool isDown;
  bool textRepeat;
};

#define MAX_TRANSITION_COUNT 8
#define MAX_FINGERS 20
#define MAX_BUTTONS 300
#define MAX_TEXT 20

struct platform_PointerState {
  F32 x[MAX_TRANSITION_COUNT+1];
  F32 y[MAX_TRANSITION_COUNT+1];    
  platform_ButtonState button;
  inline F32 finalX() {
    return x[button.halfTransitionCount];
  }
  inline F32 finalY() {
    return y[button.halfTransitionCount];
  }
  inline Vector2 finalPosition() {
    return {finalX(), finalY()};
  }
};

struct platform_ControllerState {
  bool isEnabled;
  bool analogEnabled;
  F32 leftStickX;
  F32 leftStickY;
  F32 rightStickX;
  F32 rightStickY;

  union {
    platform_ButtonState buttons[18];
    struct {
      platform_ButtonState directionUp;
      platform_ButtonState directionDown;
      platform_ButtonState directionLeft;
      platform_ButtonState directionRight;

      platform_ButtonState buttonUp;
      platform_ButtonState buttonDown;
      platform_ButtonState buttonLeft;
      platform_ButtonState buttonRight;

      platform_ButtonState shoulderLeftTop;
      platform_ButtonState shoulderRightTop;
      platform_ButtonState shoulderLeftBottom;
      platform_ButtonState shoulderRightBottom;

      platform_ButtonState centralLeft;
      platform_ButtonState centralRight;
    };
  };
};

struct platform_MousePointerState {
	platform_PointerState _mps[5];
  };

struct platform_Input {  
  // TIME STEP
  F32 t; 

  // MOUSE INPUT
  F32 mouseWheel;
  union {
    platform_PointerState mousePointers[5];
    platform_MousePointerState _mp;
  };
  bool mouseDisabled;
  // TOUCH INPUT
  // TODO: may want some sort of gesture system
  platform_PointerState touchPointers[MAX_FINGERS];
  
  // CONTROLLER INPUT
  platform_ControllerState controller;
  
  // KEYBOARD INPUT
  // TODO: will want some sort of enum system for simpler mapping
  union
  {
    platform_ButtonState keyboardButtons[MAX_BUTTONS];

    struct {
      platform_ButtonState _k0[4];
      platform_ButtonState k_a;
      platform_ButtonState k_b;
      platform_ButtonState k_c;
      platform_ButtonState k_d;
      platform_ButtonState k_e;
      platform_ButtonState k_f;
      platform_ButtonState k_g;
      platform_ButtonState k_h;
      platform_ButtonState k_i;
      platform_ButtonState k_j;
      platform_ButtonState k_k;
      platform_ButtonState k_l;
      platform_ButtonState k_m;
      platform_ButtonState k_n;
      platform_ButtonState k_o;
      platform_ButtonState k_p;
      platform_ButtonState k_q;
      platform_ButtonState k_r;
      platform_ButtonState k_s;
      platform_ButtonState k_t;
      platform_ButtonState k_u;
      platform_ButtonState k_v;
      platform_ButtonState k_w;
      platform_ButtonState k_x;
      platform_ButtonState k_y;
      platform_ButtonState k_z;
      platform_ButtonState k_1;
      platform_ButtonState k_2;
      platform_ButtonState k_3;
      platform_ButtonState k_4;
      platform_ButtonState k_5;
      platform_ButtonState k_6;
      platform_ButtonState k_7;
      platform_ButtonState k_8;
      platform_ButtonState k_9;
      platform_ButtonState k_0;
      platform_ButtonState k_return;
      platform_ButtonState k_esc;
      platform_ButtonState _k2[2];
      platform_ButtonState k_space;
      platform_ButtonState _k3[34];
      platform_ButtonState k_right;
      platform_ButtonState k_left;
      platform_ButtonState k_down;
      platform_ButtonState k_up;
      platform_ButtonState _k4[141];
      platform_ButtonState k_lcontrol;
      platform_ButtonState k_lshift;
      platform_ButtonState k_lalt;
    };
  };

  struct {
    char textInput[MAX_TEXT];
    U8 textInputSize;
  };
  
  // DID RELOAD OCCUR
  bool reload;  
  // WHAT IS THE HEIGHT AND WIDTH OF THE DRAWABLE AREA
  I32 windowWidth;
  I32 windowHeight; 
  
  // NOTE:(the following are function/variables to modify input state in the game)
  // CLIPBOARD FUNCTIONS   
  // Should these be under platform functions
  const char* (*getClipboardText)();
  void  (*setClipboardText)(const char* text); 
  bool lockMouse;

  // THIS IS TO FORCE THE APP TO QUIT
  bool quit;
};