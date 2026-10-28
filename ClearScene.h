
#pragma once

#include "KamataEngine.h"

class ClearScene {
public:
	~ClearScene();

	void Initialize();
	void Update();
	void Draw();

private:
	KamataEngine::Model* modelClear_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera camera_;
};
