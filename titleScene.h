
#pragma once

#include "KamataEngine.h"
#include "Skydome.h"

class TitleScene {
public:
	~TitleScene();

	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return finished_; }

private:
	// タイトルモデル
	KamataEngine::Model* modelTitle_ = nullptr;

	// サブタイトルモデル
	KamataEngine::Model* modelSub_ = nullptr;

	// 天球モデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// タイトルの座標
	KamataEngine::WorldTransform worldTransform_;

	// サブタイトルの座標
	KamataEngine::WorldTransform subWorldTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// 天球
	Skydome* skydome_ = nullptr;

	// シーン終了フラグ
	bool finished_ = false;
};
