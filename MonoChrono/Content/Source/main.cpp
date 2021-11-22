
// チーム名: コマ投げ4
#include "Game.h"


int main(int argc, char* argv[])
{
	// COMの初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// コンソールの初期化
	conioex2::InitializeConsole(300, 200, "MonoChrono", 3, 3);


	// ゲームのグローバルインスタンスを取得
	Game* game = Game::GetGameInstance();

	game->Initialize();		// 初期化
	game->Update();			// 更新処理
	game->Finalize();		// 終了処理


	// コンソールの終了処理
	conioex2::FinalizeConsole();

	// COMの終了処理
	CoUninitialize();

	return 0;
}
