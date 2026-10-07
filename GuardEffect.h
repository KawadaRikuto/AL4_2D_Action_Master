#pragma once

#include "KamataEngine.h"

#include <cstdint>

/// <summary>
/// ガード演出用エフェクト
/// </summary>
class GuardEffect {

public:
	/// <summary>
	/// ガードエフェクトの状態
	/// </summary>
	enum class State {
		kSpread,
		kFade,
		kDeath,
	};

	/// <summary>
	/// インスタンス生成と初期化
	/// </summary>
	static GuardEffect* Create(const KamataEngine::Vector3& position);

	/// <summary>
	/// モデルを設定
	/// </summary>
	static void SetModel(KamataEngine::Model* model) { model_ = model; }

	/// <summary>
	/// カメラを設定
	/// </summary>
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// デス状態か取得
	/// </summary>
	bool IsDead() const { return state_ == State::kDeath; }

private:
	// スプレッド状態の時間
	static const uint32_t kSpreadDuration = 8;

	// フェード状態の時間
	static const uint32_t kFadeDuration = 12;

	// モデル
	static KamataEngine::Model* model_;

	// カメラ
	static KamataEngine::Camera* camera_;

	// リングのワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// 最大スケール
	KamataEngine::Vector3 maximumScale_ = {
	    1.8f,
	    1.8f,
	    1.0f,
	};

	// 現在の状態
	State state_ = State::kSpread;

	// 状態の経過時間
	uint32_t counter_ = 0;
};
