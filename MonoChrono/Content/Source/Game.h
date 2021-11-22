
#pragma once

#include "GameBase.h"
#include "Player.h"
#include "Enemy.h"



enum ELevel
{
	Title =    0,
	Main =     1,
	Clear =    2,
	GameOver = 3,
};


struct Floor
{
	Floor(Vector2 location);

	Vector2 Location;
	Vector2 Size;
	conioex2::Engine::Image* Image;

	bool IsOnFloor(Player* player);
	void Draw();
};


struct Game : public GameBase
{
	Game();
	~Game();

	static Game* GetGameInstance();

	void BeginPlay() override;				// 開始処理
	void Tick(float DeltaTime) override;	// 毎フレーム実行
	void EndPlay() override;				// 終了処理

	void DebugOutput();
	void OpenLevel(ELevel level);
	void LoadAsset();
	void SetTimeDilation(float timeDilation);

	void SetGrayScale();
	void InverseGrayScale();

	void TitleTask(float DeltaTime, float dilation);
	void MainTask(float DeltaTime, float dilation);
	void ClearTask(float DeltaTime, float dilation);
	void GameOverTask(float DeltaTime, float dilation);

	float TimeDilation;
	ELevel m_CurrentLevel;

	Player* m_Player;
	Enemy*  m_Enemy;
	Floor* Floors[3];

	conioex2::Engine::Image* m_GameOverLogo;
	conioex2::Engine::Image* m_ClearLogo;

	conioex2::Engine::Image* m_TitleBackground;
	conioex2::Engine::Image* m_Titlelogo;
	conioex2::Engine::Image* m_PressA;
	conioex2::Engine::Image* m_PressB;

	conioex2::Engine::Image* m_hp_0;
	conioex2::Engine::Image* m_hp_1;
	conioex2::Engine::Image* m_hp_2;
	conioex2::Engine::Image* m_hp_3;
	conioex2::Engine::Image* m_hp_4;
	conioex2::Engine::Image* m_hp_5;

	conioex2::Engine::Image* floorImage;

	SoundHandle BGM_Title;
	SoundHandle BGM_Main;
	SoundHandle BGM_Clear;
	SoundHandle BGM_GameOver;

	SoundHandle SE_Select;
};

