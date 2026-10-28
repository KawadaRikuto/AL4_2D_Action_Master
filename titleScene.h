
#pragma once

#include "KamataEngine.h"

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

	// タイトルの座標
	KamataEngine::WorldTransform worldTransform_;

	// サブタイトルの座標
	KamataEngine::WorldTransform subWorldTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// シーン終了フラグ
	bool finished_ = false;
};
