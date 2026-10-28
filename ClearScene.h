
#pragma once

#include "KamataEngine.h"
#include "Skydome.h"

class ClearScene {
public:
	~ClearScene();

	void Initialize();
	void Update();
	void Draw();

private:
	// クリアモデル
	KamataEngine::Model* modelClear_ = nullptr;

	// 天球モデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// クリアモデルのワールド変換
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// 天球
	Skydome* skydome_ = nullptr;
};
