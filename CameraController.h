
#pragma once

#include "KamataEngine.h"

class Player;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void Initialize();
	void Update();
	void Reset();

	void SetTarget(Player* target) { target_ = target; }
	void SetMovableArea(Rect area) { movableArea_ = area; }

	const KamataEngine::Camera& GetCamera() const { return camera_; }

	// 攻撃ヒット時のカメラシェイク
	void StartShake(float power = 0.18f, int duration = 10);

private:
	KamataEngine::Camera camera_;
	Player* target_ = nullptr;

	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};

	Rect movableArea_ = {0.0f, 100.0f, 0.0f, 100.0f};

	KamataEngine::Vector3 targetPosition_ = {};

	static inline const float kInterpolationRate = 0.1f;
	static inline const float kVelocityBias = 6.0f;

	static inline const Rect targetMargin_ = {-8.0f, 8.0f, -4.0f, 4.0f};

	// シェイク管理
	float shakePower_ = 0.0f;
	int shakeTimer_ = 0;
	int shakeDuration_ = 0;

	KamataEngine::Vector3 shakeOffset_ = {};
};
