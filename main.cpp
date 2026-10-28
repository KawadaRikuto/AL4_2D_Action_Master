
#include "ClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h"

#include <Windows.h>
#include <mmsystem.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#pragma comment(lib, "winmm.lib")

// ========================================
// BGM管理
// ========================================

class BGMManager {
public:
	// ========================================
	// BGM再生
	// ========================================

	void Play(const std::wstring& fileName, bool loop) {

		// 前のBGMを停止
		Stop();

		// WAVファイルを探す
		std::wstring filePath = FindBGMFile(fileName);

		if (filePath.empty()) {

			std::wstring message = L"BGMファイルが見つかりません。\n\n"
			                       L"ファイル名: " +
			                       fileName +
			                       L"\n\nResources/BGM または Resources に"
			                       L"WAVファイルを配置してください。";

			MessageBoxW(nullptr, message.c_str(), L"COPY POP - BGMエラー", MB_OK | MB_ICONERROR);

			return;
		}

		// WAV形式として開く
		std::wstring command = L"open \"" + filePath + L"\" type waveaudio alias CopyPopBGM";

		MCIERROR result = mciSendStringW(command.c_str(), nullptr, 0, nullptr);

		if (result != 0) {

			ShowMCIError(L"BGMを開けませんでした。", result, filePath);

			return;
		}

		isPlaying_ = true;
		isLoop_ = loop;

		// 最初から再生
		result = mciSendStringW(L"play CopyPopBGM from 0", nullptr, 0, nullptr);

		if (result != 0) {

			ShowMCIError(L"BGMを再生できませんでした。", result, filePath);

			Stop();
		}
	}

	// ========================================
	// BGM更新
	// ========================================

	void Update() {

		if (!isPlaying_ || !isLoop_) {
			return;
		}

		wchar_t mode[128] = {};

		MCIERROR result = mciSendStringW(L"status CopyPopBGM mode", mode, 128, nullptr);

		if (result != 0) {
			return;
		}

		// 再生が終了したらループ
		if (lstrcmpiW(mode, L"stopped") == 0) {

			mciSendStringW(L"play CopyPopBGM from 0", nullptr, 0, nullptr);
		}
	}

	// ========================================
	// BGM停止
	// ========================================

	void Stop() {

		if (isPlaying_) {

			mciSendStringW(L"stop CopyPopBGM", nullptr, 0, nullptr);

			mciSendStringW(L"close CopyPopBGM", nullptr, 0, nullptr);
		}

		isPlaying_ = false;
		isLoop_ = false;
	}

	~BGMManager() { Stop(); }

private:
	// ========================================
	// BGMファイル検索
	// ========================================

	std::wstring FindBGMFile(const std::wstring& fileName) {

		namespace fs = std::filesystem;

		std::vector<fs::path> searchPaths;

		// 実行時の作業フォルダ
		std::error_code error;
		fs::path currentPath = fs::current_path(error);

		if (!error) {

			searchPaths.push_back(currentPath / L"Resources" / L"BGM" / fileName);

			searchPaths.push_back(currentPath / L"Resources" / fileName);
		}

		// 実行ファイルのフォルダ
		wchar_t exePath[MAX_PATH] = {};

		DWORD length = GetModuleFileNameW(nullptr, exePath, MAX_PATH);

		if (length > 0 && length < MAX_PATH) {

			fs::path exeDirectory = fs::path(exePath).parent_path();

			// 実行ファイルと同じフォルダ
			searchPaths.push_back(exeDirectory / L"Resources" / L"BGM" / fileName);

			searchPaths.push_back(exeDirectory / L"Resources" / fileName);

			// 1階層上
			searchPaths.push_back(exeDirectory.parent_path() / L"Resources" / L"BGM" / fileName);

			searchPaths.push_back(exeDirectory.parent_path() / L"Resources" / fileName);

			// 2階層上
			searchPaths.push_back(exeDirectory.parent_path().parent_path() / L"Resources" / L"BGM" / fileName);

			searchPaths.push_back(exeDirectory.parent_path().parent_path() / L"Resources" / fileName);
		}

		// ファイルの存在確認
		for (const fs::path& path : searchPaths) {

			std::error_code fileError;

			if (fs::is_regular_file(path, fileError)) {

				return fs::absolute(path).wstring();
			}
		}

		return L"";
	}

	// ========================================
	// MCIエラー表示
	// ========================================

	void ShowMCIError(const std::wstring& title, MCIERROR error, const std::wstring& filePath) {

		wchar_t errorText[256] = {};

		mciGetErrorStringW(error, errorText, 256);

		std::wstring message = title + L"\n\nファイル:\n" + filePath + L"\n\nエラー:\n" + errorText;

		MessageBoxW(nullptr, message.c_str(), L"COPY POP - BGMエラー", MB_OK | MB_ICONERROR);
	}

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

	Scene scene = Scene::kTitle;

	// ========================================
	// シーン生成
	// ========================================

	TitleScene* titleScene = new TitleScene();
	titleScene->Initialize();

	GameScene* gameScene = nullptr;

	ClearScene* clearScene = nullptr;

	// ========================================
	// タイトルBGM再生
	// ========================================

	bgmManager.Play(L"Title.wav", true);

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

					bgmManager.Play(L"GameScene.wav", true);
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

					bgmManager.Play(L"Clear.wav", false);
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

		case Scene::kTitle:

			if (titleScene) {
				titleScene->Draw();
			}

			break;

		case Scene::kGame:

			if (gameScene) {
				gameScene->Draw();
			}

			break;

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
