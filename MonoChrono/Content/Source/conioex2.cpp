/************************************************
 * @file	conioex2.cpp
 * @brief	コンソール ライブラリ
 *
 * @author	Suuta
 ************************************************/

#include "conioex2.h"

#include <string.h>
#include <math.h>
#include <random>


 // :::::::::: グローバル変数::::::::::
static conioex2::Engine* GEngine = nullptr;

static bool bIsInit = false;


// :::::::::: Vector2 ::::::::::

float Vector2::Length(const Vector2& v)
{
    return sqrtf(v.X * v.X + v.Y * v.Y);
}

Vector2 Vector2::Between(const Vector2& v1, const Vector2& v2)
{
    float x = v2.X - v1.X;
    float y = v2.Y - v1.Y;
    return Vector2(x, y);
}

Vector2 Vector2::Normalize(const Vector2& v)
{
    float x = v.X / Length(v);
    float y = v.Y / Length(v);
    return Vector2(x, y);
}

Vector2 Vector2::UnitVector(float degree)
{
    float radian = conioex2::RadianFromDegree(degree);
    float y = sin(radian);
    float x = cos(radian);
    return Vector2(x, y);
}


// :::::::::: XBoxヘルパー ::::::::::
#define NormalizeStick(value)                   (((fabs((value)) / shortMAX)) * ((value) < 0? -1 : 1))
#define NormalizeTrigger(value)                 (((fabs((value)) / ByteMAX)))
#define NormarileVibration(value)               ((value) / ushortMAX)
#define UnNormarileVibration(value)             ((value) * ushortMAX)
#define GetKeyStateFromBitField(field, mask)    (((field) & (mask)) == (mask) ? XBox_Down : XBox_Up)
#define LEFT_STICK_DEAD_ZONE                    XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
#define RIGHT_STICK_DEAD_ZONE                   XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
#define TRIGGER_DEAD_ZONE                       XINPUT_GAMEPAD_TRIGGER_THRESHOLD

void UpdateXBoxButtonState(XBoxInputState* state, XBoxInputState newstate)
{
    switch (*state)
    {
        case XBoxInputState::XBox_Down:
        {
            *state = XBox_Pressed;
            break;
        }
        case XBoxInputState::XBox_Up:
        {
            *state = XBox_Release;
            break;
        }
    }

    if (newstate == XBox_Up)
    {
        if (*state != XBox_Release)
        {
            *state = XBox_Up;
        }
    }
    else if (newstate == XBox_Down)
    {
        if (*state != XBox_Pressed)
        {
            *state = XBox_Down;
        }
    }
}

// :::::::::: ビットマップヘルパー ::::::::::
#define FILEHEADERSIZE 14                          // ファイルヘッダのサイズ
#define INFOHEADERSIZE 40                          // 情報ヘッダのサイズ
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE) // 合計サイズ

struct BMP
{
    unsigned int height;
    unsigned int width;
    Pixel* data;
};

BMP* AllocateBMP(int width, int height)
{
    BMP* img = (BMP*)malloc(sizeof(BMP));

    if (img)
    {
        img->data = (Pixel*)malloc(sizeof(Pixel) * width * height);
        img->width = width;
        img->height = height;
        return img;
    }
    else
    {
        return NULL;
    }
}

void DeallocateBMP(BMP* img)
{
    if (img)
    {
        if (img->data)
        {
            free(img->data);
        }

        img->width = 0;
        img->height = 0;

        free(img);
    }
}

BMP* LoadBMP(const char* filename)
{
    unsigned int i, j;
    int realWidth;                    // データ上の1行分のバイト数
    unsigned int width, height;       // 画像の横と縦のピクセル数
    unsigned int color;               // 何bitのBitmapファイルであるか
    unsigned char header[HEADERSIZE]; // ヘッダ情報を取り込む
    unsigned char* bmpLineData;       // 画像データ1行分

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("error: Can't Open file.");
        fclose(fp);
        return NULL;
    }

    //ヘッダ部分全てを取り込む
    fread(header, sizeof(unsigned char), HEADERSIZE, fp);

    //最初の2バイトがBM(Bitmapファイルの印)であるか
    if (strncmp((const char*)header, "BM", 2))
    {
        printf("error: Not bitmap file.");
        return NULL;
    }

    memcpy(&width, header + 18, sizeof(width));         // 画像の見た目上の幅を取得
    memcpy(&height, header + 22, sizeof(height));       // 画像の高さを取得
    memcpy(&color, header + 28, sizeof(unsigned int));  // 何bitのBitmapであるかを取得

    // 24bitじゃなかったら終了する
    if (color != 24)
    {
        printf("error: %s is not 24bit color image.", filename);
        return nullptr;
    }

    // RGB情報は画像の1行分が4byteの倍数で無ければならないためそれに合わせている
    realWidth = width * 3 + width % 4;

    // 画像の1行分のRGB情報を取ってくるためのバッファを動的に取得
    bmpLineData = (unsigned char*)malloc(sizeof(unsigned char) * realWidth);
    if (bmpLineData == NULL)
    {
        printf("error: Allocation failed.");
        return NULL;
    }

    // RGB情報を取り込むためのバッファを動的に取得D
    BMP* img = AllocateBMP(width, height);
    if (img == NULL)
    {
        free(bmpLineData);
        fclose(fp);
        return NULL;
    }

    // Bitmapファイルの情報は左下から右へ、下から上に並んでいる
    for (i = 0; i < height; i++)
    {
        fread(bmpLineData, 1, realWidth, fp);
        for (j = 0; j < width; j++)
        {
            img->data[(height - i - 1) * width + j].B = bmpLineData[j * 3];
            img->data[(height - i - 1) * width + j].G = bmpLineData[j * 3 + 1];
            img->data[(height - i - 1) * width + j].R = bmpLineData[j * 3 + 2];
        }
    }

    free(bmpLineData);
    fclose(fp);

    return img;
}

void UnloadBMP(BMP* img)
{
    DeallocateBMP(img);
}

bool CompareColor(COLORREF table, Pixel pixel)
{
    return table == RGB(pixel.R, pixel.G, pixel.B);
}


// :::::::::: conioex2 関数::::::::::

namespace conioex2
{
    void* GetOutputHandle()
    {
        return GEngine->console.hOutput;
    }

    void* GetInputHandle()
    {
        return GEngine->console.hInput;
    }

    const char* Title()
    {
        return GEngine->console.Title;
    }

    int Width()
    {
        return GEngine->console.WindowSize.X;
    }

    int Height()
    {
        return GEngine->console.WindowSize.Y;
    }

    void InitializeConsole(int width, int height, const char* titleName, int fontWidth, int fontHeight, const WCHAR* fontname)
    {
        LOG("■conioex2::InitializeConsole().\n");

        if (bIsInit)
        {
            LOG("・すでに初期化されています.\n");
            return;
        }


        GEngine = new Engine;

        LOG("・new GEngine.\n");

        GEngine->console.hInput  = GetStdHandle(STD_INPUT_HANDLE);
        GEngine->console.hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

        GEngine->console.WindowSize.X = width;
        GEngine->console.WindowSize.Y = height;
        GEngine->console.WindowRect = { 0, 0, static_cast<short>(GEngine->console.WindowSize.X - 1), static_cast<short>(GEngine->console.WindowSize.Y - 1) };

        // バッファーの情報
        GetConsoleScreenBufferInfoEx(GEngine->console.hOutput, &GEngine->console.ScreenBufferInfo);
        GEngine->console.ScreenBufferInfo.cbSize   = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
        GEngine->console.ScreenBufferInfo.dwSize   = { GEngine->console.WindowSize.X, GEngine->console.WindowSize.Y };
        GEngine->console.ScreenBufferInfo.srWindow = { 0, 0, (short)GEngine->console.WindowSize.X, (short)GEngine->console.WindowSize.Y };

        GEngine->console.ScreenBufferInfo.ColorTable[ 0] = RGB( 12,  12,  12);
        GEngine->console.ScreenBufferInfo.ColorTable[ 1] = RGB(  0,  55, 218);
        GEngine->console.ScreenBufferInfo.ColorTable[ 2] = RGB( 19, 161,  14);
        GEngine->console.ScreenBufferInfo.ColorTable[ 3] = RGB( 58, 150, 221);
        GEngine->console.ScreenBufferInfo.ColorTable[ 4] = RGB(197,  15,  31);
        GEngine->console.ScreenBufferInfo.ColorTable[ 5] = RGB(136,  23, 152);
        GEngine->console.ScreenBufferInfo.ColorTable[ 6] = RGB(193, 156,   0);
        GEngine->console.ScreenBufferInfo.ColorTable[ 7] = RGB(204, 204, 204);
        GEngine->console.ScreenBufferInfo.ColorTable[ 8] = RGB(118, 118, 118);
        GEngine->console.ScreenBufferInfo.ColorTable[ 9] = RGB( 59, 120, 255);
        GEngine->console.ScreenBufferInfo.ColorTable[10] = RGB( 22, 198,  12);
        GEngine->console.ScreenBufferInfo.ColorTable[11] = RGB( 97, 214, 214);
        GEngine->console.ScreenBufferInfo.ColorTable[12] = RGB(231,  72,  86);
        GEngine->console.ScreenBufferInfo.ColorTable[13] = RGB(180,   0, 158);
        GEngine->console.ScreenBufferInfo.ColorTable[14] = RGB(249, 241, 165);
        GEngine->console.ScreenBufferInfo.ColorTable[15] = RGB(242, 242, 242);

        // オーバーサイズ時に終了
        /*if (GEngine->console.ScreenBufferInfo.dwMaximumWindowSize.X < width)
        {
            conioex2::Print("指定されたコンソールサイズは大きすぎます。");
            return false;
        }
        else if (GEngine->console.ScreenBufferInfo.dwMaximumWindowSize.Y < height)
        {
            conioex2::Print("指定されたコンソールサイズは大きすぎます。");
            return false;
        }*/

        strcpy(GEngine->console.Title, titleName);
        conioex2::SetTitle(titleName);

        // サイズ変更可能にするために一度最小化する
        SMALL_RECT rect = { 0, 0, 1, 1 };
        SetConsoleWindowInfo(GetOutputHandle(), TRUE, &rect);

        // カーソル
        GEngine->console.CursorInfo.bVisible = TRUE;
        GEngine->console.CursorInfo.dwSize   = 25;
        SetConsoleCursorInfo(GetOutputHandle(), &GEngine->console.CursorInfo);

        // フォント
        wcscpy(GEngine->console.FontInfo.FaceName, fontname);
        GEngine->console.FontInfo.cbSize       = sizeof(GEngine->console.FontInfo);
        GEngine->console.FontInfo.nFont        = 0;
        GEngine->console.FontInfo.dwFontSize.X = fontWidth;
        GEngine->console.FontInfo.dwFontSize.Y = fontHeight;
        GEngine->console.FontInfo.FontFamily   = FF_DONTCARE;
        GEngine->console.FontInfo.FontWeight   = FW_NORMAL;
        SetCurrentConsoleFontEx(GetOutputHandle(), false, &GEngine->console.FontInfo);

        // バッファーの設定
        SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);

        // バッファーのサイズ
        SetConsoleScreenBufferSize(GetOutputHandle(), COORD{ (short)GEngine->console.WindowSize.X, (short)GEngine->console.WindowSize.Y });

        // ウィンドウの情報
        SetConsoleWindowInfo(GetOutputHandle(), TRUE, &GEngine->console.WindowRect);

        // 現在のバッファーを設定
        SetConsoleActiveScreenBuffer(GetOutputHandle());

#if 0
        // バッファーサイズのメモリ確保（確保済みなら解放してから確保）
        if (GEngine->console.ScreenBuffer == nullptr)
        {
            GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
            ::LOG("・new GEngine->console.ScreenBuffer.\n");
            ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));
        }
        else
        {
            delete GEngine->console.ScreenBuffer;
            GEngine->console.ScreenBuffer = nullptr;
            ::LOG("・delete GEngine->console.ScreenBuffer.\n");

            GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
            ::LOG("・new GEngine->console.ScreenBuffer.\n");
            ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));
        }
#endif

        // バッファーサイズのメモリ確保
        GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
        LOG("・new GEngine->console.ScreenBuffer.\n");
        ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));


        // ウィンドウのシステムメニューを非表示
        // 終了処理を実行させるために、ユーザーが×ボタンで終了させないようにする
        //HWND consoleWindow = GetConsoleWindow();
        //SetWindowLongPtr(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_SYSMENU);


        // コンソール入力形式を設定
        conioex2::ResetConsoleMode();

        // 時間の初期化
        conioex2::InitTime();

        //サウンドの初期化
        AudioManager::Init();

        // 文字色の初期化
        conioex2::SetTextAttribute(Color::FONT_WHITE, BACK_BLACK);

        bIsInit = true;
    }

    void FinalizeConsole()
    {
        LOG("■conioex2::FinalizeConsole().\n");
        if (bIsInit)
        {
            if (GEngine)
            {
                AudioManager::Fin();

                LOG("■conioex2::DestroySound().\n");

                if (GEngine->console.ScreenBuffer)
                {
                    delete GEngine->console.ScreenBuffer;
                    GEngine->console.ScreenBuffer = nullptr;
                    LOG("・delete GEngine->console.ScreenBuffer.\n");

                    delete GEngine;
                    GEngine = nullptr;
                    LOG("・delete GEngine.\n");
                }

                bIsInit = false;
            }
        }
        else
        {
            LOG("・すでに終了処理が呼ばれています。\n");
        }
    }

    void SetCursorVisibility(bool visibility)
    {
        visibility ? GEngine->console.CursorInfo.bVisible = true : GEngine->console.CursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetOutputHandle(), &GEngine->console.CursorInfo);
    }

    void SetCursorLocation(short x, short y)
    {
        SetConsoleCursorPosition(GetOutputHandle(), COORD{ x, y });
    }

    void SetTitle(const char* title)
    {
        SetConsoleTitleA(title);
    }

    void SetTextAttribute(Color font, Color background)
    {
        SetConsoleTextAttribute(GetOutputHandle(), static_cast<int>(font) + static_cast<int>(background));
    }

    void SetColorTable(int index, COLORREF color)
    {
        if (index < 0)  index = 0;
        if (index > 15) index = 15;

        GetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
        GEngine->console.ScreenBufferInfo.ColorTable[index] = color;

        // 呼ぶたびにウィンドウサイズ変わっちゃうから対応
        GEngine->console.ScreenBufferInfo.srWindow.Bottom++;
        GEngine->console.ScreenBufferInfo.srWindow.Right++;

        SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
    }

    void ResetColorTable()
    {
        GetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);

        GEngine->console.ScreenBufferInfo.ColorTable[ 0] = RGB( 12,  12,  12);
        GEngine->console.ScreenBufferInfo.ColorTable[ 1] = RGB(  0,  55, 218);
        GEngine->console.ScreenBufferInfo.ColorTable[ 2] = RGB( 19, 161,  14);
        GEngine->console.ScreenBufferInfo.ColorTable[ 3] = RGB( 58, 150, 221);
        GEngine->console.ScreenBufferInfo.ColorTable[ 4] = RGB(197,  15,  31);
        GEngine->console.ScreenBufferInfo.ColorTable[ 5] = RGB(136,  23, 152);
        GEngine->console.ScreenBufferInfo.ColorTable[ 6] = RGB(193, 156,   0);
        GEngine->console.ScreenBufferInfo.ColorTable[ 7] = RGB(204, 204, 204);
        GEngine->console.ScreenBufferInfo.ColorTable[ 8] = RGB(118, 118, 118);
        GEngine->console.ScreenBufferInfo.ColorTable[ 9] = RGB( 59, 120, 255);
        GEngine->console.ScreenBufferInfo.ColorTable[10] = RGB( 22, 198,  12);
        GEngine->console.ScreenBufferInfo.ColorTable[11] = RGB( 97, 214, 214);
        GEngine->console.ScreenBufferInfo.ColorTable[12] = RGB(231,  72,  86);
        GEngine->console.ScreenBufferInfo.ColorTable[13] = RGB(180,   0, 158);
        GEngine->console.ScreenBufferInfo.ColorTable[14] = RGB(249, 241, 165);
        GEngine->console.ScreenBufferInfo.ColorTable[15] = RGB(242, 242, 242);

        // 呼ぶたびにウィンドウサイズ変わっちゃうから対応
        GEngine->console.ScreenBufferInfo.srWindow.Bottom++;
        GEngine->console.ScreenBufferInfo.srWindow.Right++;

        SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
    }

    void SetInputMode(InputMode mode)
    {
        switch (mode)
        {
            case InputMode::Input:
            {
                SetConsoleMode(
                    GetInputHandle(),
                    ENABLE_ECHO_INPUT |
                    ENABLE_INSERT_MODE |
                    ENABLE_LINE_INPUT |
                    ENABLE_MOUSE_INPUT |
                    ENABLE_PROCESSED_INPUT |
                    ENABLE_EXTENDED_FLAGS |
                    ENABLE_QUICK_EDIT_MODE |
                    ENABLE_WINDOW_INPUT |
                    ENABLE_VIRTUAL_TERMINAL_INPUT);
                break;
            }
            case InputMode::GameInput:
            {
                SetConsoleMode(
                    GetInputHandle(),
                    ENABLE_EXTENDED_FLAGS |
                    ENABLE_MOUSE_INPUT);
                break;
            }
        }
    }

    void ResetConsoleMode()
    {
        SetConsoleMode(
            GetOutputHandle(),
            ENABLE_PROCESSED_OUTPUT |
            ENABLE_WRAP_AT_EOL_OUTPUT |
            ENABLE_VIRTUAL_TERMINAL_PROCESSING |
            DISABLE_NEWLINE_AUTO_RETURN |
            ENABLE_LVB_GRID_WORLDWIDE);

        SetConsoleMode(
            GetInputHandle,
            ENABLE_ECHO_INPUT |
            ENABLE_INSERT_MODE |
            ENABLE_LINE_INPUT |
            ENABLE_MOUSE_INPUT |
            ENABLE_PROCESSED_INPUT |
            ENABLE_EXTENDED_FLAGS |
            ENABLE_QUICK_EDIT_MODE |
            //ENABLE_WINDOW_INPUT |
            ENABLE_VIRTUAL_TERMINAL_INPUT);
    }

    void Draw(int x, int y, const char* string, Color font, Color background)
    {
        unsigned short color = static_cast<unsigned short>(font) + static_cast<unsigned short>(background);
        for (size_t i = 0; i < strlen(string); i++)
        {
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = color;
        }
    }

    void Draw(int x, int y, const char* string, uint16 font, uint16 background)
    {
        unsigned short color = static_cast<unsigned short>(font) + static_cast<unsigned short>(background);
        for (size_t i = 0; i < strlen(string); i++)
        {
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = color;
        }
    }

    void Draw24(int x, int y, int index, const char* string)
    {
        for (size_t i = 0; i < strlen(string); i++)
        {
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
            GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = index << 4;
        }
    }

    void DrawImage(Engine::Image* sprite, int startX, int startY)
    {
        int p = 0;
        int color = 0;

        for (int y = sprite->Height; y > 0; y--)
        {
            for (int x = 0; x < sprite->Width; x++)
            {
                color = RGB(sprite->Sprite[p], sprite->Sprite[p + 1], sprite->Sprite[p + 2]);

                     if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 0]) conioex2::Draw24(startX + x, startY + y - 1,  0, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 1]) conioex2::Draw24(startX + x, startY + y - 1,  1, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 2]) conioex2::Draw24(startX + x, startY + y - 1,  2, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 3]) conioex2::Draw24(startX + x, startY + y - 1,  3, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 4]) conioex2::Draw24(startX + x, startY + y - 1,  4, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 5]) conioex2::Draw24(startX + x, startY + y - 1,  5, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 6]) conioex2::Draw24(startX + x, startY + y - 1,  6, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 7]) conioex2::Draw24(startX + x, startY + y - 1,  7, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 8]) conioex2::Draw24(startX + x, startY + y - 1,  8, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[ 9]) conioex2::Draw24(startX + x, startY + y - 1,  9, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[10]) conioex2::Draw24(startX + x, startY + y - 1, 10, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[11]) conioex2::Draw24(startX + x, startY + y - 1, 11, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[12]) conioex2::Draw24(startX + x, startY + y - 1, 12, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[13]) conioex2::Draw24(startX + x, startY + y - 1, 13, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[14]) conioex2::Draw24(startX + x, startY + y - 1, 14, " ");
                else if (color == GEngine->console.ScreenBufferInfo.ColorTable[15]) conioex2::Draw24(startX + x, startY + y - 1, 15, " ");
                p += 3;
            }
        }
    }

    void DrawImageGS(Engine::Image* sprite, int startX, int startY)
    {
        int p = 0;
        int color = 0;

        for (int y = sprite->Height; y > 0; y--)
        {
            for (int x = 0; x < sprite->Width; x++)
            {
                color = RGB(sprite->Sprite[p] - 1, sprite->Sprite[p + 1] - 1, sprite->Sprite[p + 2] - 1);

                switch (color)
                {
                case RGB(  0,   0,   0): conioex2::Draw24(startX + x, startY + y - 1, 15, " "); break;
                case RGB( 16,  16,  16): conioex2::Draw24(startX + x, startY + y - 1, 14, " "); break;
                case RGB( 32,  32,  32): conioex2::Draw24(startX + x, startY + y - 1, 13, " "); break;
                case RGB( 48,  48,  48): conioex2::Draw24(startX + x, startY + y - 1, 12, " "); break;
                case RGB( 64,  64,  64): conioex2::Draw24(startX + x, startY + y - 1, 11, " "); break;
                case RGB( 80,  80,  80): conioex2::Draw24(startX + x, startY + y - 1, 10, " "); break;
                case RGB( 96,  96,  96): conioex2::Draw24(startX + x, startY + y - 1,  9, " "); break;
                case RGB(112, 112, 112): conioex2::Draw24(startX + x, startY + y - 1,  8, " "); break;
                case RGB(128, 128, 128): conioex2::Draw24(startX + x, startY + y - 1,  7, " "); break;
                case RGB(144, 144, 144): conioex2::Draw24(startX + x, startY + y - 1,  6, " "); break;
                case RGB(160, 160, 160): conioex2::Draw24(startX + x, startY + y - 1,  5, " "); break;
                case RGB(176, 176, 176): conioex2::Draw24(startX + x, startY + y - 1,  4, " "); break;
                case RGB(192, 192, 192): conioex2::Draw24(startX + x, startY + y - 1,  3, " "); break;
                case RGB(208, 208, 208): conioex2::Draw24(startX + x, startY + y - 1,  2, " "); break;
                case RGB(224, 224, 224): conioex2::Draw24(startX + x, startY + y - 1,  1, " "); break;
                case RGB(240, 240, 240): conioex2::Draw24(startX + x, startY + y - 1,  0, " "); break;
                }
                p += 3;
            }
        }
    }

    void DumpRGB(Engine::Image* sprite)
    {
        int p = 0;
        int color = 0;

        for (int y = sprite->Height; y > 0; y--)
        {
            for (int x = 0; x < sprite->Width; x++)
            {
                LOG("R %d G %d B %d\n", sprite->Sprite[p], sprite->Sprite[p + 1], sprite->Sprite[p + 2]);
                p += 3;
            }
        }
    }

    void Render()
    {
        WriteConsoleOutputA(GEngine->console.hOutput, GEngine->console.ScreenBuffer, GEngine->console.WindowSize, COORD{ 0, 0 }, &GEngine->console.WindowRect);
    }

    void ClearScreen()
    {
        for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
        {
            for (int x = 0; x < GEngine->console.WindowSize.X; x++)
            {
                Draw(x, y, " ", Color::FONT_BLACK, Color::BACK_BLACK);
            }
        }

        Render();
    }

    void ClearBuffer()
    {
        for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
        {
            for (int x = 0; x < GEngine->console.WindowSize.X; x++)
            {
                Draw(x, y, " ", Color::FONT_BLACK, Color::BACK_BLACK);
            }
        }
    }

    void ClearScreen(Color font, Color background)
    {
        for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
        {
            for (int x = 0; x < GEngine->console.WindowSize.X; x++)
            {
                Draw(x, y, " ", font, background);
            }
        }

        Render();
    }

    void ClearBuffer(Color font, Color background)
    {
        for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
        {
            for (int x = 0; x < GEngine->console.WindowSize.X; x++)
            {
                Draw(x, y, " ", font, background);
            }
        }
    }

    void ClearInputBuffer()
    {
        FlushConsoleInputBuffer(GEngine->console.hInput);
    }

    void Wait(unsigned long milliseconds)
    {
        Sleep(milliseconds);
    }

    void UpdateKeyInput()
    {
        if (::GetKeyboardState(GEngine->Keyboard.New))
        {
            FOR(256)
            {
                GEngine->Keyboard.Keys[i].Pressed = false;
                GEngine->Keyboard.Keys[i].Released = false;

                if (GEngine->Keyboard.New[i] != GEngine->Keyboard.Old[i])
                {
                    if (GEngine->Keyboard.New[i] & 0x80)
                    {
                        GEngine->Keyboard.Keys[i].Pressed = !GEngine->Keyboard.Keys[i].Hold;
                        GEngine->Keyboard.Keys[i].Hold = true;
                    }
                    else
                    {
                        GEngine->Keyboard.Keys[i].Released = true;
                        GEngine->Keyboard.Keys[i].Hold = false;
                    }
                }
                GEngine->Keyboard.Old[i] = GEngine->Keyboard.New[i];
            }
        }
    }

    void UpdateAsyncKeyInput()
    {
        for (int i = 0; i < 256; i++)
        {
            GEngine->Keyboard.NewKey[i] = GetAsyncKeyState(i);

            GEngine->Keyboard.Keys[i].Pressed = false;
            GEngine->Keyboard.Keys[i].Released = false;

            if (GEngine->Keyboard.NewKey[i] != GEngine->Keyboard.OldKey[i])
            {
                if (GEngine->Keyboard.NewKey[i] & 0x8000)
                {
                    GEngine->Keyboard.Keys[i].Pressed = !GEngine->Keyboard.Keys[i].Hold;
                    GEngine->Keyboard.Keys[i].Hold = true;
                }
                else
                {
                    GEngine->Keyboard.Keys[i].Released = true;
                    GEngine->Keyboard.Keys[i].Hold = false;
                }
            }

            GEngine->Keyboard.OldKey[i] = GEngine->Keyboard.NewKey[i];
        }
    }

    bool GetInputKey(Key key, InputState state)
    {
        switch (state)
        {
            case InputState::Pressed: return GEngine->Keyboard.Keys[key].Pressed;  break;
            case InputState::Release: return GEngine->Keyboard.Keys[key].Released; break;
            case InputState::Hold:    return GEngine->Keyboard.Keys[key].Hold;     break;
            default:                  return false;
        }
    }

    void UpdateMouseInput()
    {
        GetNumberOfConsoleInputEvents(GetInputHandle(), &GEngine->Mouse.Event);
        if (GEngine->Mouse.Event > 0)
        {
            ReadConsoleInput(GetInputHandle(), GEngine->Mouse.InputRecord, GEngine->Mouse.Event, &GEngine->Mouse.Event);
        }

        for (DWORD i = 0; i < GEngine->Mouse.Event; i++)
        {
            switch (GEngine->Mouse.InputRecord[i].EventType)
            {
                case FOCUS_EVENT:
                {
                    GEngine->Mouse.IsFocusInConsole = GEngine->Mouse.InputRecord[i].Event.FocusEvent.bSetFocus;
                    break;
                }
                case MOUSE_EVENT:
                {
                    switch (GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwEventFlags)
                    {
                        case MOUSE_MOVED:
                        {
                            GEngine->Mouse.Location.X = GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwMousePosition.X;
                            GEngine->Mouse.Location.Y = GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwMousePosition.Y;
                            break;
                        }
                        case 0:
                        {
                            for (int j = 0; j < 5; j++)
                            {
                                GEngine->Mouse.NewButton[j] = (GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwButtonState & (1 << j)) > 0;
                            }
                            break;
                        }
                    }
                    break;
                }

            default: break;
            }
        }

        for (int i = 0; i < 5; i++)
        {
            GEngine->Mouse.Button[i].Pressed = false;
            GEngine->Mouse.Button[i].Released = false;

            if (GEngine->Mouse.NewButton[i] != GEngine->Mouse.OldButton[i])
            {
                if (GEngine->Mouse.NewButton[i])
                {
                    GEngine->Mouse.Button[i].Pressed = true;
                    GEngine->Mouse.Button[i].Hold = true;
                }
                else
                {
                    GEngine->Mouse.Button[i].Released = true;
                    GEngine->Mouse.Button[i].Hold = false;
                }
            }

            GEngine->Mouse.OldButton[i] = GEngine->Mouse.NewButton[i];
        }
    }

    bool GetInputMouseInput(MouseButton button, InputState state)
    {
        switch (state)
        {
            case InputState::Pressed: return GEngine->Mouse.Button[button].Pressed;  break;
            case InputState::Hold:	  return GEngine->Mouse.Button[button].Hold;     break;
            case InputState::Release: return GEngine->Mouse.Button[button].Released; break;
        }
        return false;
    }

    int GetMouseX()
    {
        return GEngine->Mouse.Location.X;
    }

    int GetMouseY()
    {
        return GEngine->Mouse.Location.Y;
    }

    bool IsFocusInConsole()
    {
        return GEngine->Mouse.IsFocusInConsole;
    }

    bool IsInViewport(Vector2 location)
    {
        return (location.X < static_cast<float>(conioex2::Width())) && (location.X > 0.0f)
            &&
            (location.Y < static_cast<float>(conioex2::Height()) - 1.0f) && (location.Y > 0.0f);
    }

    bool IsHit(AABB self, AABB other)
    {
        if (self.Min.X > other.Max.X) return false;
        if (self.Max.X < other.Min.X) return false;
        if (self.Min.Y > other.Max.Y) return false;
        if (self.Max.Y < other.Min.Y) return false;
        return true;
    }

    float RadianFromDegree(float degree)
    {
        return degree * (PI / 180.f);
    }

    void UpdateXBoxInput()
    {
        DWORD dwResult;
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(state));

        conioex2::Engine::XBox_* gamepad = &GEngine->XBox;

        dwResult = XInputGetState(0, &state);
        if (dwResult == ERROR_SUCCESS)
        {
            gamepad->enabled = true;
            gamepad->left_trigger  = static_cast<float>(NormalizeTrigger(state.Gamepad.bLeftTrigger));
            gamepad->right_trigger = static_cast<float>(NormalizeTrigger(state.Gamepad.bRightTrigger));

            Vector2 zero{ 0.f, 0.f };
            Vector2 one{ 1.f, 1.f };

            Vector2 left_stick;
            left_stick.X = static_cast<float>(NormalizeStick(state.Gamepad.sThumbLX));
            left_stick.Y = static_cast<float>(NormalizeStick(state.Gamepad.sThumbLY));

            Vector2 right_stick;
            right_stick.X = static_cast<float>(NormalizeStick(state.Gamepad.sThumbRX));
            right_stick.Y = static_cast<float>(NormalizeStick(state.Gamepad.sThumbRY));

            gamepad->left_stick  = left_stick;
            gamepad->right_stick = right_stick;

            int buttons_field = state.Gamepad.wButtons;
            UpdateXBoxButtonState(&gamepad->buttons[ 0], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_UP));
            UpdateXBoxButtonState(&gamepad->buttons[ 1], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_DOWN));
            UpdateXBoxButtonState(&gamepad->buttons[ 2], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_RIGHT));
            UpdateXBoxButtonState(&gamepad->buttons[ 3], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_LEFT));
                                                     
            UpdateXBoxButtonState(&gamepad->buttons[ 4], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_START));
            UpdateXBoxButtonState(&gamepad->buttons[ 5], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_BACK));
                                                     
            UpdateXBoxButtonState(&gamepad->buttons[ 6], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_LEFT_THUMB));
            UpdateXBoxButtonState(&gamepad->buttons[ 7], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_RIGHT_THUMB));

            UpdateXBoxButtonState(&gamepad->buttons[ 8], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_SHOULDER_RIGHT));
            UpdateXBoxButtonState(&gamepad->buttons[ 9], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_SHOULDER_LEFT));

            UpdateXBoxButtonState(&gamepad->buttons[10], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_A));
            UpdateXBoxButtonState(&gamepad->buttons[11], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_B));
            UpdateXBoxButtonState(&gamepad->buttons[12], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_X));
            UpdateXBoxButtonState(&gamepad->buttons[13], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_Y));
        }
        else
        {
            gamepad->enabled = false;
        }
    }

    bool GetInputXBoxButton(Button button, XBoxInputState state)
    {
        return GEngine->XBox.buttons[button] == state;
    }

    float GetInputXBoxTrigger(Trigger axis)
    {
        switch (axis)
        {
            case Trigger::Trigger_L:
            {
                return GEngine->XBox.left_trigger;
            }
            case Trigger::Trigger_R:
            {
                return GEngine->XBox.right_trigger;
            }
            default: return 0.0f;
        }
    }

    Vector2 GetInputXBoxAxis(Stick axis)
    {
        switch (axis)
        {
            case Stick::Stick_L:
            {
                return GEngine->XBox.left_stick;
            }
            case Stick::Stick_R:
            {
                return GEngine->XBox.right_stick;
            }
        }
    }

    void PlayVibrate()
    {
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed  = static_cast<DWORD>(UnNormarileVibration(1.f));
        vibration.wRightMotorSpeed = static_cast<DWORD>(UnNormarileVibration(1.f));

        XInputSetState(0, &vibration);
    }

    void StopVibrate()
    {
        XINPUT_VIBRATION vibration;
        vibration.wLeftMotorSpeed  = static_cast<DWORD>(UnNormarileVibration(0.f));
        vibration.wRightMotorSpeed = static_cast<DWORD>(UnNormarileVibration(0.f));

        XInputSetState(0, &vibration);
    }

    Engine::Image* CreateImage(const char* filename)
    {
        BMP* img = LoadBMP(filename);
        if (!img) return nullptr;

        Engine::Image* sprite = new Engine::Image;
        if (!sprite) return nullptr;

        // RBGの三要素を格納するために3倍の領域確保する
        sprite->Sprite = new unsigned char[(img->height * img->width) * 3];
        sprite->Height = img->height;
        sprite->Width  = img->width;

        int elm = 0;
        for (unsigned int i = 0; i < img->height; i++)
        {
            for (unsigned int j = 0; j < img->width; j++)
            {
                unsigned char r = (img->data[(img->height - i - 1) * img->width + j].R) + 1;
                unsigned char g = (img->data[(img->height - i - 1) * img->width + j].G) + 1;
                unsigned char b = (img->data[(img->height - i - 1) * img->width + j].B) + 1;
                sprite->Sprite[elm] = r;
                sprite->Sprite[elm + 1] = g;
                sprite->Sprite[elm + 2] = b;
                elm += 3;
            }
        }

        UnloadBMP(img);
        return sprite;
    }

    void CreateColorTableFromImage(Engine::Image* sprite)
    {
        COLORREF table[16] = {}; // カラーテーブルのバッファ
        int elem = 0;            // 画像データの要素数
        int index = 0;           // 新しい色の時にどの場所に挿入するか
        bool isExitZero = false; // RGB(0, 0, 0)が入力されたかどうか
        int  match = 0;          // マッチした色の数

        // 画像サイズ分（縦×横）だけループする
        for (int n = 0; n < (sprite->Height * sprite->Width); n++)
        {
            // 画像データからピクセルデータを作る
            if (index > 15) break;
            Pixel pixel(sprite->Sprite[elem], sprite->Sprite[elem + 1], sprite->Sprite[elem + 2]);

            // 入力値がRGB(0, 0, 0)かどうか
            if ((pixel.R + pixel.G + pixel.B) == 0)
            {
                // すでに0があったらスキップ
                if (isExitZero)
                {
                    elem += 3;
                    continue;
                }
                else
                {
                    // 初めて0が入力された場合はマッチ、以降の0はスキップ
                    table[index] = RGB(pixel.R, pixel.G, pixel.B);
                    //Debug::LOG("[%2d]: R %3d, G %3d, B %3d\n", index, pixel.R, pixel.G, pixel.B);
                    index++;
                    isExitZero = true;
                }
            }
            else
            {
                // index回だけループさせる(ヒット・ノーヒット関わらず全部検査する)→後ろの要素に一致する値がある場合があるから
                for (int i = 0; i < index; i++)
                {
                    if (CompareColor(table[i], pixel))
                    {
                        match++;
                    }
                }
                // 一致した値が1つも無ければテーブルに追加する
                if (match == 0)
                {
                    table[index] = RGB(pixel.R, pixel.G, pixel.B);
                    //Debug::LOG("[%2d]: R %3d, G %3d, B %3d\n", index, pixel.R, pixel.G, pixel.B);
                    index++;
                }
            }

            match = 0;
            elem += 3;
        }

        FOR(16)
        {
            conioex2::SetColorTable(i, table[i] - RGB(1, 1, 1));
        }
    }

    void Print24(int x, int y, const char* string, Pixel font, Pixel background)
    {
        SetCursorLocation(x, y);
        printf("\x1b[48;2;%d;%d;%dm ", background.R, background.G, background.B);
        printf("\x1b[38;2;%d;%d;%dm%s", font.R, font.G, font.B, string);
    }

    void ShowImage(Engine::Image* sprite, int lx, int ly)
    {
        int i = 0;
        for (int y = sprite->Height; y > 0; y--)
        {
            for (int x = 0; x < sprite->Width; x++)
            {
                conioex2::Print24(lx + 2 * x, ly + y - 1, "  ", Pixel(255, 255, 255), Pixel(sprite->Sprite[i], sprite->Sprite[i + 1], sprite->Sprite[i + 2]));
                i += 3;
            }
        }
    }

    bool ReleaseImage(Engine::Image* object)
    {
        if (object)
        {
            if (object->Sprite)
            {
                delete[] object->Sprite; // delete[]？
                delete object;
                return true;
            }
        }
        return false;
    }

    void InitTime()
    {
        QueryPerformanceFrequency(&GEngine->Time.frequency);
        QueryPerformanceCounter(&GEngine->Time.start);
        QueryPerformanceCounter(&GEngine->Time.end);
    }

    double GetDeltaTime()
    {
        return GEngine->Time.DeltaTime;
    }

    void UpdateTime()
    {
        QueryPerformanceCounter(&GEngine->Time.end);
        GEngine->Time.DeltaTime = static_cast<double>((GEngine->Time.end.QuadPart - GEngine->Time.start.QuadPart) / (GEngine->Time.frequency.QuadPart));
        GEngine->Time.start.QuadPart = GEngine->Time.end.QuadPart;
    }

    uint64 RangeRand(uint64 min, uint64 max)
    {
        static std::mt19937_64 mt64(0);
        std::uniform_int_distribution<uint64_t> rand(min, max);
        return rand(mt64);
    }
}