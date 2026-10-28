
#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"

#include <Windows.h>

// ========================================
// Windowsアプリのエントリーポイント
// ========================================

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジン初期化
	KamataEngine::Initialize(L"LE2C_08_カワダ_リクト_COPY POP");

	using namespace KamataEngine;

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ========================================
	// シーン生成
	// ========================================

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	ClearScene* clearScene = nullptr;

	// false = ゲーム中
	// true  = クリア画面
	bool isClearScene = false;

	// ========================================
	// メインループ
	// ========================================

	while (true) {

		// エンジン更新
		if (KamataEngine::Update()) {
			break;
		}

		// ========================================
		// シーン更新
		// ========================================

		if (!isClearScene) {

			// ゲームシーン
			if (gameScene) {

				gameScene->Update();

				// ゴール到達を確認
				if (gameScene->IsGoalReached()) {

					isClearScene = true;

					// クリアシーン生成
					clearScene = new ClearScene();
					clearScene->Initialize();

					// ゲームシーン解放
					delete gameScene;
					gameScene = nullptr;
				}
			}

		} else {

			// クリアシーン
			if (clearScene) {
				clearScene->Update();
			}
		}

		// ========================================
		// 描画
		// ========================================

		dxCommon->PreDraw();

		Model::PreDraw();

		if (!isClearScene) {

			if (gameScene) {
				gameScene->Draw();
			}

		} else {

			if (clearScene) {
				clearScene->Draw();
			}
		}

		Model::PostDraw();

		dxCommon->PostDraw();
	}

	// ========================================
	// 解放
	// ========================================

	delete gameScene;
	gameScene = nullptr;

	delete clearScene;
	clearScene = nullptr;

	KamataEngine::Finalize();

	return 0;
}
