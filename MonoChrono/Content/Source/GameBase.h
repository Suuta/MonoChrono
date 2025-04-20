
#pragma once


struct GameBase
{
    GameBase();
    virtual ~GameBase();

    void Initialize();
    void Update();
    void Finalize();

    virtual void BeginPlay() = 0;
    virtual void Tick(float DeltaTime) = 0;
    virtual void LateTick(float DeltaTime) = 0;
    virtual void EndPlay() = 0;

    bool bIsRunning;
};

