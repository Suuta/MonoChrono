/************************************************
 * @file	conioex2.h
 * @brief	コンソール ライブラリ
 ************************************************/

#pragma once

#define _CRT_SECURE_NO_WARNINGS

 // 文字セットをマルチバイトに限定させる
#ifndef _MBCS
#error マルチバイト文字セットを使用してください。
#endif

/************************************************
    インクルード
************************************************/
#include <Windows.h>
#include <Xinput.h>
#include <Xaudio2.h>
#include <cstdio>
#include <string>
#include <vector>

#include "Audio.h"

#pragma comment (lib, "Winmm.lib")
#pragma comment (lib, "Xinput.lib")

/************************************************
    マクロ・型・定数
************************************************/
typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

constexpr int32  intMAX = 0x7FFFFFFF;
constexpr int32  intMIN = -intMAX;
constexpr int16  shortMAX = 0x7FFF;
constexpr int16  shortMIN = -shortMAX;
constexpr uint8  ByteMAX = 0xFF;
constexpr uint16 ushortMAX = 0xFFFF;
constexpr uint32 uintMAX = 0xFFFFFFFF;

#define ARRAY_SIZE(n) (sizeof(n) / sizeof(n[0]))
#define SUPER         __super

#define FOR(n)        for(int i = 0; i < n; i++)
#define FOR_X(n)      for(int x = 0; x < n; x++)
#define FOR_Y(n)      for(int y = 0; y < n; y++)

#define PI            3.1415926F

/************************************************
    列挙型
************************************************/

// 文字色
enum Color
{
    FONT_BLACK     = 0x0000,
    FONT_D_BLUE    = 0x0001,
    FONT_D_GREEN   = 0x0002,
    FONT_D_CYAN    = 0x0003,
    FONT_D_RED     = 0x0004,
    FONT_D_MAGENTA = 0x0005,
    FONT_D_YELLOW  = 0x0006,
    FONT_GREY      = 0x0007,
    FONT_DARK_GREY = 0x0008,
    FONT_BLUE      = 0x0009,
    FONT_GREEN     = 0x000A,
    FONT_CYAN      = 0x000B,
    FONT_RED       = 0x000C,
    FONT_MAGENTA   = 0x000D,
    FONT_YELLOW    = 0x000E,
    FONT_WHITE     = 0x000F,

    BACK_BLACK     = 0x0000,
    BACK_D_BLUE    = 0x0010,
    BACK_D_GREEN   = 0x0020,
    BACK_D_CYAN    = 0x0030,
    BACK_D_RED     = 0x0040,
    BACK_D_MAGENTA = 0x0050,
    BACK_D_YELLOW  = 0x0060,
    BACK_GREY      = 0x0070,
    BACK_DARK_GREY = 0x0080,
    BACK_BLUE      = 0x0090,
    BACK_GREEN     = 0x00A0,
    BACK_CYAN      = 0x00B0,
    BACK_RED       = 0x00C0,
    BACK_MAGENTA   = 0x00D0,
    BACK_YELLOW    = 0x00E0,
    BACK_WHITE     = 0x00F0,
};

// キーボード
enum Key
{
    RETURN = 0x0D,
    ESCAPE = 0x1B,
    SPACE  = 0x20,
    A      = 0x41,
    B      = 0x42,
    C      = 0x43,
    D      = 0x44,
    E      = 0x45,
    F      = 0x46,
    G      = 0x47,
    H      = 0x48,
    I      = 0x49,
    J      = 0x4A,
    K      = 0x4B,
    L      = 0x4C,
    M      = 0x4D,
    N      = 0x4E,
    O      = 0x4F,
    P      = 0x50,
    Q      = 0x51,
    R      = 0x52,
    S      = 0x53,
    T      = 0x54,
    U      = 0x55,
    V      = 0x56,
    W      = 0x57,
    X      = 0x58,
    Y      = 0x59,
    Z      = 0x5A,
    LEFT   = 0x25,
    UP     = 0x26,
    RIGHT  = 0x27,
    DOWN   = 0x28,
};

// キー入力
enum InputState
{
    Pressed,
    Hold,
    Release,
};

// マウスボタン
enum MouseButton
{
    L_Button,
    R_Button,
    Wheel_Button,
};

enum XBoxInputState
{
    XBox_Release,
    XBox_Up,
    XBox_Pressed,
    XBox_Down,
};

enum GAMEPAD_BUTTONS
{
    GAMEPAD_BUTTON_DPAD_UP        = XINPUT_GAMEPAD_DPAD_UP,
    GAMEPAD_BUTTON_DPAD_DOWN      = XINPUT_GAMEPAD_DPAD_DOWN,
    GAMEPAD_BUTTON_DPAD_RIGHT     = XINPUT_GAMEPAD_DPAD_LEFT,
    GAMEPAD_BUTTON_DPAD_LEFT      = XINPUT_GAMEPAD_DPAD_RIGHT,
    GAMEPAD_BUTTON_START          = XINPUT_GAMEPAD_START,
    GAMEPAD_BUTTON_BACK           = XINPUT_GAMEPAD_BACK,
    GAMEPAD_BUTTON_LEFT_THUMB     = XINPUT_GAMEPAD_LEFT_THUMB,
    GAMEPAD_BUTTON_RIGHT_THUMB    = XINPUT_GAMEPAD_RIGHT_THUMB,
    GAMEPAD_BUTTON_SHOULDER_RIGHT = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    GAMEPAD_BUTTON_SHOULDER_LEFT  = XINPUT_GAMEPAD_LEFT_SHOULDER,
    GAMEPAD_BUTTON_A              = XINPUT_GAMEPAD_A,
    GAMEPAD_BUTTON_B              = XINPUT_GAMEPAD_B,
    GAMEPAD_BUTTON_X              = XINPUT_GAMEPAD_X,
    GAMEPAD_BUTTON_Y              = XINPUT_GAMEPAD_Y,
};

enum Button
{
    XBox_Button_Up         = 0,
    XBox_Button_Down       = 1,
    XBox_Button_Right      = 2,
    XBox_Button_Left       = 3,
    XBox_Button_Start      = 4,
    XBox_Button_Bsck       = 5,
    XBox_Button_L_Thumb    = 6,
    XBox_Button_R_Thumb    = 7,
    XBox_Button_R_Shoulder = 8,
    XBox_Button_L_Shoulder = 9,
    XBox_Button_A          = 10,
    XBox_Button_B          = 11,
    XBox_Button_X          = 12,
    XBox_Button_Y          = 13,
};

enum Trigger
{
    Trigger_L,
    Trigger_R,
};

enum Stick
{
    Stick_L,
    Stick_R,
};

enum InputMode
{
    Input,
    GameInput,
};

enum SOUND_EFFECT_INSTANCE_FLAGS : uint32
{
    SoundEffectInstance_Default          = 0x0,
    SoundEffectInstance_Use3D            = 0x1,
    SoundEffectInstance_ReverbUseFilters = 0x2,
    SoundEffectInstance_NoSetPitch       = 0x4,
    SoundEffectInstance_UseRedirectLFE   = 0x10000,
};


/************************************************
    構造体
************************************************/

// 2次元ベクトル
struct Vector2
{
    float X;
    float Y;

    Vector2() :
        X(0.0f),
        Y(0.0f)
    {
    }

    Vector2(float x, float y) :
        X(x),
        Y(y)
    {
    }

    Vector2 operator*(float scalar)
    {
        Vector2 result;
        result.X = this->X * scalar;
        result.Y = this->Y * scalar;
        return result;
    }

    Vector2 operator/(float scalar)
    {
        Vector2 result;
        result.X = this->X / scalar;
        result.Y = this->Y / scalar;
        return result;
    }

    Vector2 operator/=(float scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    static float Length(const Vector2& v);
    static Vector2 Between(const Vector2& v1, const Vector2& v2);
    static Vector2 Normalize(const Vector2& v);
    static Vector2 UnitVector(float degree);

    Vector2 operator-(const Vector2& v)
    {
        Vector2 result;
        result.X = this->X - v.X;
        result.Y = this->Y - v.Y;
        return result;
    }

    Vector2 operator/(const Vector2& v)
    {
        Vector2 result;
        result.X = this->X / v.X;
        result.Y = this->Y / v.Y;
        return result;
    }
};

// バウンディングボックス
struct AABB
{
    AABB()
        : Min(0.f, 0.f)
        , Max(0.f, 0.f)
    {
    }

    AABB(Vector2 min, Vector2 max)
        : Min(min)
        , Max(max)
    {
    }

    Vector2 Min;
    Vector2 Max;
};

// ピクセル
struct Pixel
{
    Pixel()
        : R(0)
        , G(0)
        , B(0)
    {}

    Pixel(int r, int g, int b)
        : R(r)
        , G(g)
        , B(b)
    {}
    unsigned char R;
    unsigned char G;
    unsigned char B;
};


struct KeyState
{
    bool Pressed  = false;
    bool Hold     = false;
    bool Released = false;
};


namespace conioex2
{
    struct Engine
    {
        // コンソール
        struct Console
        {
            HANDLE                       hInput;
            HANDLE                       hOutput;
            CONSOLE_SCREEN_BUFFER_INFOEX ScreenBufferInfo;
            CONSOLE_FONT_INFOEX          FontInfo;
            CONSOLE_CURSOR_INFO          CursorInfo;
            COORD                        WindowSize;
            SMALL_RECT                   WindowRect;
            CHAR_INFO*                   ScreenBuffer = nullptr;
            char                         Title[256] = { "---" };
        };
        Console console;

        // ビットマップ画像
        struct Image
        {
            unsigned char* Sprite;
            unsigned char  Width;
            unsigned char  Height;
        };
        Image Sprite;

        // キーボード
        struct Keyboard_
        {
            KeyState Keys[256];

            short NewKey[256] = { 0 };
            short OldKey[256] = { 0 };

            uint8 New[256] = { 0 };
            uint8 Old[256] = { 0 };
        };
        Keyboard_ Keyboard;

        // マウス
        struct Mouse_
        {
            KeyState     Button[5];
            INPUT_RECORD InputRecord[32];
            DWORD        Event;
            COORD        Location;
            bool         NewButton[5] = { 0 };
            bool         OldButton[5] = { 0 };
            bool         IsFocusInConsole;
        };
        Mouse_ Mouse;

        // 時間
        struct Time_
        {
            LARGE_INTEGER frequency;
            LARGE_INTEGER start;
            LARGE_INTEGER end;
            double        DeltaTime;
        };
        Time_ Time;

        // コントローラー
        struct XBox_
        {
            XBoxInputState buttons[14];
            Vector2        left_stick;
            Vector2        right_stick;
            float          left_trigger;
            float          right_trigger;
            bool           enabled;
        };
        XBox_ XBox;
    };


    //:::::::::::::::::::: コンソール :::::::::::::::::::://

    void InitializeConsole(int width, int height, const char* titleName, int fontwidth, int fontheight, const WCHAR* fontname = L"");
    void FinalizeConsole();
    void SetCursorVisibility(bool visibility);
    void SetCursorLocation(short x, short y);
    void SetTitle(const char* title);
    void SetTextAttribute(Color font, Color background);
    void SetColorTable(int index, COLORREF color);
    void ResetColorTable();
    void SetInputMode(InputMode mode);
    void ResetConsoleMode();
    int Width();
    int Height();
    void* GetOutputHandle();
    void* GetInputHandle();
    const char* Title();
    void Print24(int x, int y, const char* string, Pixel font = Pixel(255, 255, 255), Pixel background = Pixel(12, 12, 12));
    void Draw(int x, int y, const char* string, Color font, Color background);
    void Draw(int x, int y, const char* string, uint16 font, uint16 background);
    void Draw24(int x, int y, int index, const char* string);
    void DrawImage(Engine::Image* sprite, int startX, int startY);
    void DrawImageGS(Engine::Image* sprite, int startX, int startY);
    void DumpRGB(Engine::Image* sprite);
    void Render();
    void ClearScreen();
    void ClearBuffer();
    void ClearScreen(Color font, Color background);
    void ClearBuffer(Color font, Color background);
    void ClearInputBuffer();
    bool IsFocusInConsole();
    bool IsInViewport(Vector2 location);

    //:::::::::::::::::::: キーボード :::::::::::::::::::://

    void UpdateKeyInput();
    void UpdateAsyncKeyInput();
    bool GetInputKey(Key key, InputState state);

    //:::::::::::::::::::: マウス :::::::::::::::::::://

    void UpdateMouseInput();
    bool GetInputMouseInput(MouseButton button, InputState state);
    int GetMouseX();
    int GetMouseY();

    //:::::::::::::::::::: コントローラー :::::::::::::::::::://

    void UpdateXBoxInput();
    bool GetInputXBoxButton(Button button, XBoxInputState state);
    float GetInputXBoxTrigger(Trigger axis);
    Vector2 GetInputXBoxAxis(Stick axis);

    void PlayVibrate();
    void StopVibrate();

    //:::::::::::::::::::: ビットマップ :::::::::::::::::::://

    Engine::Image* CreateImage(const char* filename);
    void ShowImage(Engine::Image* sprite, int lx, int ly);
    bool ReleaseImage(Engine::Image* object);
    void CreateColorTableFromImage(Engine::Image* sprite);	// BMPから16色のカラーテーブルを生成する

    //:::::::::::::::::::: 時間 :::::::::::::::::::://

    void InitTime();
    void UpdateTime();
    void Wait(unsigned long milliseconds);
    double GetDeltaTime();

    //:::::::::::::::::::: 汎用 :::::::::::::::::::://

    uint64 RangeRand(uint64 min, uint64 max);
    bool IsHit(AABB self, AABB other);
    float RadianFromDegree(float degree);

}// namespace conioex2


#define _LOG( str, ... ) { char buf[256]; sprintf( buf, str, __VA_ARGS__ ); OutputDebugString( buf ); }


template<typename ... Args>
FORCEINLINE void LOG(const char* format, const Args& ... args)
{
#ifdef _DEBUG
    _LOG(format, args ...);
#endif
}
