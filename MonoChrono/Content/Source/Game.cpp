
#include "Game.h"
#include "Projectile.h"

#include "Audio.h"



Game::Game():
	m_CurrentLevel(ELevel::Title),
	m_TitleBackground(nullptr),
	TimeDilation(1.0f)
{
}

Game::~Game()
{
}

Game* Game::GetGameInstance()
{
	static Game instance;
	return &instance;
}

void Game::BeginPlay()
{
	m_Player = new Player; LOG("・new m_Player.\n");
	m_Enemy  = new Enemy;  LOG("・new m_Enemy.\n");

	SetGrayScale();
	LoadAsset();
}

void Game::Tick(float DeltaTime)
{
	switch (m_CurrentLevel)
	{
		case ELevel::Title:
		{
			TitleTask(DeltaTime, TimeDilation);
			break;
		}
		case ELevel::Main:
		{
			MainTask(DeltaTime, TimeDilation);
			break;
		}
		case ELevel::Clear:
		{
			ClearTask(DeltaTime, TimeDilation);
			break;
		}
		case ELevel::GameOver:
		{
			GameOverTask(DeltaTime, TimeDilation);
			break;
		}
	}
}

void Game::EndPlay()
{
	LOG("■Game::EndPlay().\n");
	if (m_Player)
	{
		delete m_Player;
		m_Player = nullptr;
		LOG("・delete m_Player.\n");
	}

	if (m_Enemy)
	{
		delete m_Enemy;
		m_Enemy = nullptr;
		LOG("・delete m_Enemy.\n");
	}
	
	delete Floors[0];
	LOG("・delete Floors[0]\n");
	delete Floors[1];
	LOG("・delete Floors[1]\n");
	delete Floors[2];
	LOG("・delete Floors[2]\n");

	//conioex2::StopSound(BGM_Main);
	AudioManager::StopStresm(BGM_Main);

	conioex2::ResetColorTable();
	conioex2::ClearScreen();
	conioex2::SetTitle("MonoChrono");

	// MessageBox(::GetConsoleWindow(), "ゲームは正常終了しました。\n プレイして頂きありがとうございます。", "Log", NULL);
}

void Game::DebugOutput()
{
	char buf[256] = {};

	sprintf_s(buf, sizeof(buf), "X: %.0f Y: %.0f", m_Player->Location.X, m_Player->Location.Y);
	conioex2::Draw(0, 0, buf, Color::FONT_GREEN, Color::BACK_BLACK);
	sprintf_s(buf, sizeof(buf), "Key::A: %d", conioex2::GetInputKey(Key::A, InputState::Hold));
	conioex2::Draw(0, 3, buf, Color::FONT_GREEN, Color::BACK_BLACK);
	sprintf_s(buf, sizeof(buf), "Key::D: %d", conioex2::GetInputKey(Key::D, InputState::Hold));
	conioex2::Draw(0, 4, buf, Color::FONT_GREEN, Color::BACK_BLACK);

	sprintf_s(buf, sizeof(buf), "HP: %.0f\n", m_Player->HP);
	LOG(buf);
}

void Game::OpenLevel(ELevel level)
{
	m_CurrentLevel = level;
}

void Game::LoadAsset()
{
	// 汎用画像
	m_TitleBackground = conioex2::CreateImage("Content/Asset/Image/idle_01.bmp");
	m_Titlelogo = conioex2::CreateImage("Content/Asset/Image/titlelogo.bmp");
	m_PressA = conioex2::CreateImage("Content/Asset/Image/press.bmp");
	m_PressB = conioex2::CreateImage("Content/Asset/Image/end.bmp");

	m_GameOverLogo = conioex2::CreateImage("Content/Asset/Image/gameover.bmp");
	m_ClearLogo = conioex2::CreateImage("Content/Asset/Image/clear.bmp");

	// 床
	floorImage = conioex2::CreateImage("Content/Asset/Image/floor.bmp");

	// HPゲージ
	m_hp_1 = conioex2::CreateImage("Content/Asset/Image/hp_1.bmp");
	m_hp_2 = conioex2::CreateImage("Content/Asset/Image/hp_2.bmp");
	m_hp_3 = conioex2::CreateImage("Content/Asset/Image/hp_3.bmp");
	m_hp_4 = conioex2::CreateImage("Content/Asset/Image/hp_4.bmp");
	m_hp_5 = conioex2::CreateImage("Content/Asset/Image/hp_5.bmp");

	// プレイヤーアニメ―ション
	m_Player->R_IdleAnimation = conioex2::CreateImage("Content/Asset/Image/idle_01.bmp");
	m_Player->L_IdleAnimation = conioex2::CreateImage("Content/Asset/Image/idle_02.bmp");

	// 空中アニメーション
	m_Player->R_AirAnimation[0] = conioex2::CreateImage("Content/Asset/Image/air_01.bmp");
	m_Player->R_AirAnimation[1] = conioex2::CreateImage("Content/Asset/Image/air_02.bmp");
	m_Player->L_AirAnimation[0] = conioex2::CreateImage("Content/Asset/Image/air_03.bmp");
	m_Player->L_AirAnimation[1] = conioex2::CreateImage("Content/Asset/Image/air_04.bmp");

	// ダッシュアニメーション
	m_Player->R_DashAnimation = conioex2::CreateImage("Content/Asset/Image/dash_01.bmp");
	m_Player->L_DashAnimation = conioex2::CreateImage("Content/Asset/Image/dash_02.bmp");

	// 歩きアニメーション
	m_Player->R_WalkAnimation[0] = conioex2::CreateImage("Content/Asset/Image/walk_01.bmp");
	m_Player->R_WalkAnimation[1] = conioex2::CreateImage("Content/Asset/Image/walk_02.bmp");
	m_Player->R_WalkAnimation[2] = conioex2::CreateImage("Content/Asset/Image/walk_03.bmp");
	m_Player->R_WalkAnimation[3] = conioex2::CreateImage("Content/Asset/Image/walk_04.bmp");	
	m_Player->R_WalkAnimation[4] = conioex2::CreateImage("Content/Asset/Image/walk_05.bmp");
	m_Player->R_WalkAnimation[5] = conioex2::CreateImage("Content/Asset/Image/walk_06.bmp");
	m_Player->R_WalkAnimation[6] = conioex2::CreateImage("Content/Asset/Image/walk_07.bmp");
	m_Player->L_WalkAnimation[0] = conioex2::CreateImage("Content/Asset/Image/walk_08.bmp");
	m_Player->L_WalkAnimation[1] = conioex2::CreateImage("Content/Asset/Image/walk_09.bmp");
	m_Player->L_WalkAnimation[2] = conioex2::CreateImage("Content/Asset/Image/walk_10.bmp");
	m_Player->L_WalkAnimation[3] = conioex2::CreateImage("Content/Asset/Image/walk_11.bmp");
	m_Player->L_WalkAnimation[4] = conioex2::CreateImage("Content/Asset/Image/walk_12.bmp");
	m_Player->L_WalkAnimation[5] = conioex2::CreateImage("Content/Asset/Image/walk_13.bmp");
	m_Player->L_WalkAnimation[6] = conioex2::CreateImage("Content/Asset/Image/walk_14.bmp");

	// 攻撃アニメーション
	m_Player->R_AttackAnimation[0] = conioex2::CreateImage("Content/Asset/Image/Attack_01.bmp");
	m_Player->R_AttackAnimation[1] = conioex2::CreateImage("Content/Asset/Image/Attack_02.bmp");
	m_Player->R_AttackAnimation[2] = conioex2::CreateImage("Content/Asset/Image/Attack_03.bmp");
	m_Player->L_AttackAnimation[0] = conioex2::CreateImage("Content/Asset/Image/Attack_04.bmp");
	m_Player->L_AttackAnimation[1] = conioex2::CreateImage("Content/Asset/Image/Attack_05.bmp");
	m_Player->L_AttackAnimation[2] = conioex2::CreateImage("Content/Asset/Image/Attack_06.bmp");

	// 攻撃トレイル
	m_Player->L_attackTrail = conioex2::CreateImage("Content/Asset/Image/l_attack_trail.bmp");
	m_Player->R_attackTrail = conioex2::CreateImage("Content/Asset/Image/r_attack_trail.bmp");

	// 敵アニメーション
	m_Enemy->Idle                 = conioex2::CreateImage("Content/Asset/Image/enemy_idle_01.bmp");
	m_Enemy->Damaged              = conioex2::CreateImage("Content/Asset/Image/enemy_damage.bmp");
	m_Enemy->ProjectileImage      = conioex2::CreateImage("Content/Asset/Image/projectile.bmp");
	m_Enemy->ProjectileImage2     = conioex2::CreateImage("Content/Asset/Image/projectile_02.bmp");
	m_Enemy->SpearProjectileImage = conioex2::CreateImage("Content/Asset/Image/spear.bmp");


#if 0
	// ピッチ変更したいサウンドは自作ライブラリの方使う
	BGM_Main = conioex2::LoadSound("Content/Asset/Sound/bgm.wav");

	// その他のサウンドは先生のライブラリ使う
	BGM_Title    = CreateSound("Content/Asset/Sound/title.wav");
	BGM_Clear    = CreateSound("Content/Asset/Sound/clear.wav");
	BGM_GameOver = CreateSound("Content/Asset/Sound/gameover.wav");

	SE_Select           = CreateSound("Content/Asset/Sound/Select.wav");
	m_Enemy->SE_Damage  = CreateSound("Content/Asset/Sound/hit.wav");
	m_Player->SE_Damage = CreateSound("Content/Asset/Sound/damage.wav");
	m_Player->SE_Attack = CreateSound("Content/Asset/Sound/attack.wav");
#endif


	BGM_Main     = AudioManager::LoadFromFile(L"Content/Asset/Sound/bgm.wav", SoundFileType::Stream);
	BGM_Title    = AudioManager::LoadFromFile(L"Content/Asset/Sound/title.wav", SoundFileType::Stream);
	BGM_Clear    = AudioManager::LoadFromFile(L"Content/Asset/Sound/clear.wav", SoundFileType::Stream);
	BGM_GameOver = AudioManager::LoadFromFile(L"Content/Asset/Sound/gameover.wav", SoundFileType::Stream);

	SE_Select           = AudioManager::LoadFromFile(L"Content/Asset/Sound/Select.wav", SoundFileType::Effect);
	m_Enemy->SE_Damage  = AudioManager::LoadFromFile(L"Content/Asset/Sound/hit.wav", SoundFileType::Effect);
	m_Player->SE_Damage = AudioManager::LoadFromFile(L"Content/Asset/Sound/damage.wav", SoundFileType::Effect);
	m_Player->SE_Attack = AudioManager::LoadFromFile(L"Content/Asset/Sound/attack.wav", SoundFileType::Effect);
}

void Game::SetGrayScale()
{
	conioex2::SetColorTable(15, RGB(  0,   0,   0));
	conioex2::SetColorTable(14, RGB( 16,  16,  16));
	conioex2::SetColorTable(13, RGB( 32,  32,  32));
	conioex2::SetColorTable(12, RGB( 48,  48,  48));
	conioex2::SetColorTable(11, RGB( 64,  64,  64));
	conioex2::SetColorTable(10, RGB( 80,  80,  80));
	conioex2::SetColorTable( 9, RGB( 96,  96,  96));
	conioex2::SetColorTable( 8, RGB(112, 112, 112));
	conioex2::SetColorTable( 7, RGB(128, 128, 128));
	conioex2::SetColorTable( 6, RGB(144, 144, 144));
	conioex2::SetColorTable( 5, RGB(160, 160, 160));
	conioex2::SetColorTable( 4, RGB(176, 176, 176));
	conioex2::SetColorTable( 3, RGB(192, 192, 192));
	conioex2::SetColorTable( 2, RGB(208, 208, 208));
	conioex2::SetColorTable( 1, RGB(224, 224, 224));
	conioex2::SetColorTable( 0, RGB(240, 240, 240));
}

void Game::InverseGrayScale()
{
	conioex2::SetColorTable( 0, RGB(  0,   0,   0));
	conioex2::SetColorTable( 1, RGB( 16,  16,  16));
	conioex2::SetColorTable( 2, RGB( 32,  32,  32));
	conioex2::SetColorTable( 3, RGB( 48,  48,  48));
	conioex2::SetColorTable( 4, RGB( 64,  64,  64));
	conioex2::SetColorTable( 5, RGB( 80,  80,  80));
	conioex2::SetColorTable( 6, RGB( 96,  96,  96));
	conioex2::SetColorTable( 7, RGB(112, 112, 112));
	conioex2::SetColorTable( 8, RGB(128, 128, 128));
	conioex2::SetColorTable( 9, RGB(144, 144, 144));
	conioex2::SetColorTable(10, RGB(160, 160, 160));
	conioex2::SetColorTable(11, RGB(176, 176, 176));
	conioex2::SetColorTable(12, RGB(192, 192, 192));
	conioex2::SetColorTable(13, RGB(208, 208, 208));
	conioex2::SetColorTable(14, RGB(224, 224, 224));
	conioex2::SetColorTable(15, RGB(240, 240, 240));
}

void Game::TitleTask(float DeltaTime, float dilation)
{
	static bool Do = true;
	static bool up = false;
	static float span = 0.f;
	static int r = 0;
	static int g = 0;
	static int b = 0;

	span += 1.f * DeltaTime * dilation;

	if (Do)
	{
		//PlaySound(BGM_Title, false);
		//SetVolume(BGM_Title, 0.1f);

		AudioManager::SetStreamVolume(BGM_Title, 0.1f);
		AudioManager::PlayStream(BGM_Title);
		Do = false;
	}

	// メインレベルへ移動
	if (conioex2::GetInputXBoxButton(Button::XBox_Button_B, XBoxInputState::XBox_Pressed))
	{
		//PlaySound(SE_Select, false);
		//SetVolume(SE_Select, 0.1f);
		//StopSound(BGM_Title);
		AudioManager::SetSEVolume(SE_Select, 0.1f);
		AudioManager::PlaySE(SE_Select);
		AudioManager::StopStresm(BGM_Title);

		OpenLevel(ELevel::Main);
	}

	if (span >= 0.1f)
	{
		if ((r + g + b) == 0)
		{
			up = true;
		}
		else if ((r + g + b) >= 720)
		{
			up = false;
		}

		if (up)
		{
			r += 16;
			g += 16;
			b += 16;
			conioex2::SetColorTable(10, RGB(r, g, b));
		}
		else
		{
			r -= 16;
			g -= 16;
			b -= 16;
			conioex2::SetColorTable(10, RGB(r, g, b));
		}
		span = 0.f;
	}

	conioex2::DrawImageGS(m_Titlelogo, 60, 80);
	conioex2::DrawImageGS(m_PressA, 135, 130);
}

void Game::MainTask(float DeltaTime, float dilation)
{
	static bool Do = true;
	if (Do)
	{
		LOG("■Game::MainTask()\n");
		//conioex2::PlaytSound(BGM_Main, true);
		//conioex2::SetVolume(BGM_Main, 0.05f);

		AudioManager::SetStreamVolume(BGM_Main, 0.05f);
		AudioManager::PlayStream(BGM_Main);

		Floors[0] = new Floor(Vector2( 55.f, 150.f));  LOG("・new Floors[0]\n");
		Floors[1] = new Floor(Vector2(130.f, 120.f)); LOG("・new Floors[1]\n");
		Floors[2] = new Floor(Vector2(205.f, 150.f)); LOG("・new Floors[2]\n");

		if (m_Enemy)
		{
			m_Enemy->Teleport();
			m_Enemy->Tick(DeltaTime, dilation);
			m_Enemy->Attack1();
		}

		Do = false;
	}

	FOR(3) Floors[i]->Draw();

	FOR_Y(5)
	{
		FOR_X(300)
		{
			conioex2::Draw(x, y + 195, " ", 5 << 4, 0);
		}
	}

	if(m_Enemy)  m_Enemy->Tick(DeltaTime, dilation);
	if(m_Player) m_Player->Tick(DeltaTime, dilation);

}

void Game::ClearTask(float DeltaTime, float dilation)
{
	static bool Do = true;
	if (Do)
	{
		SetGrayScale();
		//PlaySound(BGM_Clear, false);
		//SetVolume(BGM_Clear, 0.1f);

		AudioManager::PlayStream(BGM_Clear);
		AudioManager::SetStreamVolume(BGM_Clear, 0.1f);

		Do = false;
	}

	// ゲームを終了させる
	if (conioex2::GetInputXBoxButton(Button::XBox_Button_B, XBoxInputState::XBox_Pressed))
	{
		//PlaySound(SE_Select, false);
		//SetVolume(SE_Select, 0.1f);
		//StopSound(BGM_Clear);

		//AudioManager::SetSEVolume(SE_Select, 0.1f);
		//AudioManager::PlaySE(SE_Select);
		AudioManager::StopStresm(BGM_Clear);

		Game::GetGameInstance()->bIsRunning = false;
	}

	conioex2::DrawImageGS(m_ClearLogo, 105, 80);
	conioex2::DrawImageGS(m_PressA, 135, 130);
}

void Game::GameOverTask(float DeltaTime, float dilation)
{
	static bool Do = true;
	if (Do)
	{
		SetGrayScale();
		//PlaySound(BGM_GameOver, false);
		//SetVolume(BGM_GameOver, 0.5f);

		AudioManager::SetStreamVolume(BGM_GameOver, 0.5f);
		AudioManager::PlayStream(BGM_GameOver);
		Do = false;
	}

	// ゲームを終了させる
	if (conioex2::GetInputXBoxButton(Button::XBox_Button_B, XBoxInputState::XBox_Pressed))
	{
		//PlaySound(SE_Select, false);
		//SetVolume(SE_Select, 0.1f);
		//StopSound(BGM_GameOver);
		AudioManager::StopStresm(BGM_GameOver);
		Game::GetGameInstance()->bIsRunning = false;
	}

	conioex2::DrawImageGS(m_GameOverLogo, 75, 80);
	conioex2::DrawImageGS(m_PressA, 135, 130);
}

void Game::SetTimeDilation(float Dilation)
{
	TimeDilation = Dilation;
}

// ステージの床
Floor::Floor(Vector2 location)
	: Location(location)
	, Size(Vector2(45.f, 4.f))
{
	Image = Game::GetGameInstance()->floorImage;
}

bool Floor::IsOnFloor(Player* player)
{
	return
	((player->CapsuleCollision.Max.Y) <= Location.Y) && ((player->CapsuleCollision.Max.Y) >= Location.Y - 3.f)
		&&
	((player->CapsuleCollision.Max.X - 6.f >= Location.X) && (player->CapsuleCollision.Min.X <= Location.X + Size.X));
}

void Floor::Draw()
{
	conioex2::DrawImageGS(Image, Location.X, Location.Y);
}
