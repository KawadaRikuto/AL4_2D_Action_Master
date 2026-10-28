
#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"

#include <Windows.h>

// ========================================
// Windowsアプリのエントリーポイント
// ========================================

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ========================================
	// エンジン初期化
	// ========================================

	KamataEngine::Initialize(L"LE2C_08_カワダ_リクト_COPY POP");

	using namespace KamataEngine;

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ========================================
	// シーン管理
	// ========================================

	enum class Scene { kTitle, kGame, kClear };

	// 最初はタイトルシーン
	Scene scene = Scene::kTitle;

	// ========================================
	// シーン生成
	// ========================================

	// タイトルシーン
	TitleScene* titleScene = new TitleScene();
	titleScene->Initialize();

	// ゲームシーン
	GameScene* gameScene = nullptr;

	// クリアシーン
	ClearScene* clearScene = nullptr;

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

		switch (scene) {

			// ========================================
			// タイトルシーン
			// ========================================

		case Scene::kTitle:

			if (titleScene) {

				titleScene->Update();

				// SPACEキーでゲーム開始
				if (titleScene->IsFinished()) {

					// タイトルシーン解放
					delete titleScene;
					titleScene = nullptr;

					// ゲームシーン生成
					gameScene = new GameScene();
					gameScene->Initialize();

					// ゲームシーンへ移行
					scene = Scene::kGame;
				}
			}

			break;

			// ========================================
			// ゲームシーン
			// ========================================

		case Scene::kGame:

			if (gameScene) {

				gameScene->Update();

				// ゴール到達判定
				if (gameScene->IsGoalReached()) {

					// クリアシーン生成
					clearScene = new ClearScene();
					clearScene->Initialize();

					// ゲームシーン解放
					delete gameScene;
					gameScene = nullptr;

					// クリアシーンへ移行
					scene = Scene::kClear;
				}
			}

			break;

			// ========================================
			// クリアシーン
			// ========================================

		case Scene::kClear:

			if (clearScene) {

				clearScene->Update();
			}

			break;
		}

		// ========================================
		// 描画前処理
		// ========================================

		dxCommon->PreDraw();

		Model::PreDraw();

		// ========================================
		// シーン描画
		// ========================================

		switch (scene) {

		// タイトルシーン描画
		case Scene::kTitle:

			if (titleScene) {

				titleScene->Draw();
			}

			break;

		// ゲームシーン描画
		case Scene::kGame:

			if (gameScene) {

				gameScene->Draw();
			}

			break;

		// クリアシーン描画
		case Scene::kClear:

			if (clearScene) {

				clearScene->Draw();
			}

			break;
		}

		// ========================================
		// 描画後処理
		// ========================================

		Model::PostDraw();

		dxCommon->PostDraw();
	}

	// ========================================
	// 解放処理
	// ========================================

	delete titleScene;
	titleScene = nullptr;

	delete gameScene;
	gameScene = nullptr;

	delete clearScene;
	clearScene = nullptr;

	// ========================================
	// エンジン終了
	// ========================================

	KamataEngine::Finalize();

	return 0;
}
