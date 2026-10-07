#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// testメッセージ

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_07_カワダ_リクト");

	// 名前空間の使用
	using namespace KamataEngine;

	// 画面描画
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ゲームシーンの生成
	GameScene* gameScene = new GameScene();

	// ゲームシーンの初期化
	gameScene->Initialize();

	// メインループ
	while (true) {

		// testメッセージ

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ゲームシーンの更新
		gameScene->Update();

		// 描画前処理
		dxCommon->PreDraw();

		// 3Dモデル描画前処理
		Model::PreDraw();

		// ゲームシーンの描画
		gameScene->Draw();

		// 3Dモデル描画後処理
		Model::PostDraw();

		// 描画後処理
		dxCommon->PostDraw();
	}

	// ゲームシーンの解放
	delete gameScene;

	// nullptrを代入
	gameScene = nullptr;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}