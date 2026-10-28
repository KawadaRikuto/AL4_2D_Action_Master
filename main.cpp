
#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"

#include <Windows.h>
#include <mmsystem.h>
#include <string>

#pragma comment(lib, "winmm.lib")

// ========================================
// BGM管理
// ========================================

class BGMManager {
public:
	// BGMを再生する
	void Play(const std::wstring& filePath, bool loop) {

		// 再生中のBGMを停止
		Stop();

		// ループ設定
		isLoop_ = loop;

		// MP3を開く
		std::wstring command = L"open \"" + filePath + L"\" type mpegvideo alias CopyPopBGM";

		MCIERROR result = mciSendStringW(command.c_str(), nullptr, 0, nullptr);

		if (result != 0) {

			isPlaying_ = false;
			return;
		}

		isPlaying_ = true;

		// BGMを最初から再生
		mciSendStringW(L"play CopyPopBGM from 0", nullptr, 0, nullptr);
	}

	// 毎フレーム更新
	void Update() {

		if (!isPlaying_ || !isLoop_) {
			return;
		}

		wchar_t mode[128] = {};

		// 現在の再生状態を取得
		MCIERROR result = mciSendStringW(L"status CopyPopBGM mode", mode, 128, nullptr);

		if (result != 0) {
			return;
		}

		// 曲の再生が終了したら最初から再生
		if (lstrcmpiW(mode, L"stopped") == 0) {

			mciSendStringW(L"play CopyPopBGM from 0", nullptr, 0, nullptr);
		}
	}

	// BGM停止
	void Stop() {

		if (!isPlaying_) {
			return;
		}

		mciSendStringW(L"stop CopyPopBGM", nullptr, 0, nullptr);

		mciSendStringW(L"close CopyPopBGM", nullptr, 0, nullptr);

		isPlaying_ = false;
		isLoop_ = false;
	}

	~BGMManager() { Stop(); }

private:
	bool isPlaying_ = false;
	bool isLoop_ = false;
};

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
	// BGM管理
	// ========================================

	BGMManager bgmManager;

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
	// タイトルBGM再生
	// ========================================

	bgmManager.Play(L"Resources/BGM/Title.mp3", true);

	// ========================================
	// メインループ
	// ========================================

	while (true) {

		// ========================================
		// エンジン更新
		// ========================================

		if (KamataEngine::Update()) {
			break;
		}

		// ========================================
		// BGM更新
		// ========================================

		bgmManager.Update();

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

					// タイトルBGM停止
					bgmManager.Stop();

					// タイトルシーン解放
					delete titleScene;
					titleScene = nullptr;

					// ゲームシーン生成
					gameScene = new GameScene();
					gameScene->Initialize();

					// ゲームシーンへ移行
					scene = Scene::kGame;

					// ========================================
					// ゲームBGM再生
					// ========================================

					bgmManager.Play(L"Resources/BGM/GameScene.mp3", true);
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

					// ゲームBGM停止
					bgmManager.Stop();

					// クリアシーン生成
					clearScene = new ClearScene();
					clearScene->Initialize();

					// ゲームシーン解放
					delete gameScene;
					gameScene = nullptr;

					// クリアシーンへ移行
					scene = Scene::kClear;

					// ========================================
					// クリアBGM再生
					// ========================================

					bgmManager.Play(L"Resources/BGM/Clear.mp3", false);
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

			// ========================================
			// タイトルシーン描画
			// ========================================

		case Scene::kTitle:

			if (titleScene) {

				titleScene->Draw();
			}

			break;

			// ========================================
			// ゲームシーン描画
			// ========================================

		case Scene::kGame:

			if (gameScene) {

				gameScene->Draw();
			}

			break;

			// ========================================
			// クリアシーン描画
			// ========================================

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
	// BGM停止
	// ========================================

	bgmManager.Stop();

	// ========================================
	// シーン解放
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
