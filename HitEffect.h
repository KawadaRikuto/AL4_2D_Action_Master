#pragma once

#include "KamataEngine.h"

#include <array>
#include <cstdint>

/// <summary>
/// ヒット演出用エフェクト
/// </summary>
class HitEffect {

public:
	/// <summary>
	/// ヒットエフェクトの状態
	/// </summary>
	enum class State {
		// スプレッド状態
		kSpread,

		// フェード状態
		kFade,

		// デス状態
		kDeath,
	};

	/// <summary>
	/// インスタンス生成と初期化
	/// </summary>
	static HitEffect* Create(const KamataEngine::Vector3& position);

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
	// 楕円エフェクトの数
	static const size_t kNumEllipses = 3;

	// スプレッド状態の時間
	static const uint32_t kSpreadDuration = 8;

	// フェード状態の時間
	static const uint32_t kFadeDuration = 12;

	// モデル（借りてくる用）
	static KamataEngine::Model* model_;

	// カメラ（借りてくる用）
	static KamataEngine::Camera* camera_;

	// 楕円のワールドトランスフォーム
	std::array<KamataEngine::WorldTransform, kNumEllipses> ellipseWorldTransforms_;

	// 楕円ごとの最大スケール
	std::array<KamataEngine::Vector3, kNumEllipses> maximumScales_;

	// 現在の状態
	State state_ = State::kSpread;

	// 状態の経過時間
	uint32_t counter_ = 0;
};