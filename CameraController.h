#pragma once

#include "KamataEngine.h"

// 前方宣言
class Player;

/// <summary>
/// カメラコントローラ
/// </summary>
class CameraController {

public:
	// 矩形
	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 追従対象を設定
	/// </summary>
	void SetTarget(Player* target) { target_ = target; }

	/// <summary>
	/// カメラ移動範囲を設定
	/// </summary>
	void SetMovableArea(Rect area) { movableArea_ = area; }

	/// <summary>
	/// 瞬間合わせ
	/// </summary>
	void Reset();

	/// <summary>
	/// カメラを取得
	/// </summary>
	const KamataEngine::Camera& GetCamera() const { return camera_; }

private:
	// カメラ
	KamataEngine::Camera camera_;

	// 追従対象
	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};

	// カメラ移動範囲
	Rect movableArea_ = {0.0f, 100.0f, 0.0f, 100.0f};

	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_ = {};

	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	// 速度掛け率
	static inline const float kVelocityBias = 6.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect targetMargin_ = {
	    -8.0f, // 左マージン
	    8.0f,  // 右マージン
	    -4.0f, // 下マージン
	    4.0f   // 上マージン
	};
};