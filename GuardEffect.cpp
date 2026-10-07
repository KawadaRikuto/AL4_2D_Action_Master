#include "GuardEffect.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>

namespace {

/// <summary>
/// 線形補間
/// </summary>
float Lerp(float start, float end, float t) { return start + (end - start) * t; }

/// <summary>
/// イーズアウト
/// </summary>
float EaseOutCubic(float t) {

	const float value = 1.0f - t;

	return 1.0f - value * value * value;
}

/// <summary>
/// イーズイン
/// </summary>
float EaseInCubic(float t) { return t * t * t; }

} // namespace

// 静的メンバ変数の実体
KamataEngine::Model* GuardEffect::model_ = nullptr;
KamataEngine::Camera* GuardEffect::camera_ = nullptr;

GuardEffect* GuardEffect::Create(const KamataEngine::Vector3& position) {

	// インスタンス生成
	GuardEffect* instance = new GuardEffect();

	// newの失敗を検査
	assert(instance);

	// インスタンスの初期化
	instance->Initialize(position);

	// 初期化したインスタンスを返す
	return instance;
}

void GuardEffect::Initialize(const KamataEngine::Vector3& position) {

	// モデルとカメラが設定済みか確認
	assert(model_);
	assert(camera_);

	// ワールドトランスフォームを初期化
	worldTransform_.Initialize();

	// 最初は小さくしておく
	worldTransform_.scale_ = {
	    0.0f,
	    0.0f,
	    1.0f,
	};

	// 発生座標を設定
	worldTransform_.translation_ = position;

	// ワールド行列を更新
	UpdateWorldTransform(worldTransform_);

	// スプレッド状態から開始
	state_ = State::kSpread;

	// カウンターを初期化
	counter_ = 0;
}

void GuardEffect::Update() {

	switch (state_) {

	case State::kSpread: {

		float t = static_cast<float>(counter_) / static_cast<float>(kSpreadDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		const float easedT = EaseOutCubic(t);

		worldTransform_.scale_ = {
		    Lerp(0.0f, maximumScale_.x, easedT),
		    Lerp(0.0f, maximumScale_.y, easedT),
		    1.0f,
		};

		UpdateWorldTransform(worldTransform_);

		++counter_;

		if (counter_ >= kSpreadDuration) {

			state_ = State::kFade;
			counter_ = 0;
		}

		break;
	}

	case State::kFade: {

		float t = static_cast<float>(counter_) / static_cast<float>(kFadeDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		const float easedT = EaseInCubic(t);

		worldTransform_.scale_ = {
		    Lerp(maximumScale_.x, 0.0f, easedT),
		    Lerp(maximumScale_.y, 0.0f, easedT),
		    1.0f,
		};

		UpdateWorldTransform(worldTransform_);

		++counter_;

		if (counter_ >= kFadeDuration) {

			state_ = State::kDeath;
			counter_ = 0;
		}

		break;
	}

	case State::kDeath:
		break;
	}
}

void GuardEffect::Draw() {

	if (state_ == State::kDeath) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}
