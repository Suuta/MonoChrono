
#include "conioex2.h"
#include "GameBase.h"
#include <chrono>

#include "dwmapi.h"
#pragma comment(lib, "Dwmapi.lib")


GameBase::GameBase()
    :bIsRunning(true)
{
}

GameBase::~GameBase()
{
}

void GameBase::Initialize()
{
    LOG("GameBase::Initialize().\n");
    conioex2::SetCursorVisibility(false);
}

void GameBase::Update()
{
    BeginPlay();

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();

    while (bIsRunning)
    {
        conioex2::UpdateAsyncKeyInput();
        conioex2::UpdateXBoxInput();

        // オーディオ更新
        AudioManager::Update();

        if (conioex2::GetInputKey(Key::ESCAPE, InputState::Pressed))
        {
            bIsRunning = false;
        }

        end = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = end - start;
        start = end;
        float DeltaTime = elapsedTime.count();

        conioex2::ClearBuffer();

        Tick(DeltaTime);

        conioex2::Render();

        LateTick(DeltaTime);


#if 1
        char title[256];
        static DWORD time = 0;
        static DWORD frame = 0;
        frame++;
        sprintf(title, "FPS :  %d", frame);
        if (timeGetTime() - time > 1000)
        {
            time = timeGetTime();
            frame = 0;
            conioex2::SetTitle(title);
        }
#endif
    }

    EndPlay();
}

void GameBase::Finalize()
{
    LOG("GameBase::Finalize().\n");
    conioex2::SetTitle("MonoChrono");
    conioex2::SetCursorVisibility(true);
}
