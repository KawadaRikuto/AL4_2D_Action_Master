#include "HitEffect.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>

namespace {

// 円周率
const float kPi = 3.14159265358979323846f;

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

// 乱数生成エンジン
std::random_device seedGenerator;

// メルセンヌ・ツイスターエンジン
std::mt19937_64 randomEngine(seedGenerator());

} // namespace

// 静的メンバ変数の実体
KamataEngine::Model* HitEffect::model_ = nullptr;
KamataEngine::Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const KamataEngine::Vector3& position) {

	// インスタンス生成
	HitEffect* instance = new HitEffect();

	// newの失敗を検査
	assert(instance);

	// インスタンスの初期化
	instance->Initialize(position);

	// 初期化したインスタンスを返す
	return instance;
}

void HitEffect::Initialize(const KamataEngine::Vector3& position) {

	// モデルとカメラが設定済みか確認
	assert(model_);
	assert(camera_);

	// 回転角度の乱数
	std::uniform_real_distribution<float> rotationDistribution(0.0f, kPi * 2.0f);

	// 楕円の横幅の乱数
	std::uniform_real_distribution<float> widthDistribution(0.15f, 0.30f);

	// 楕円の長さの乱数
	std::uniform_real_distribution<float> lengthDistribution(2.5f, 4.5f);

	// 楕円エフェクトをまとめて初期化
	for (size_t i = 0; i < ellipseWorldTransforms_.size(); ++i) {

		KamataEngine::WorldTransform& worldTransform = ellipseWorldTransforms_[i];

		// ワールドトランスフォームを初期化
		worldTransform.Initialize();

		// 最初は小さくしておく
		worldTransform.scale_ = {
		    0.0f,
		    0.0f,
		    1.0f,
		};

		// Z軸をランダムに回転
		worldTransform.rotation_ = {
		    0.0f,
		    0.0f,
		    rotationDistribution(randomEngine),
		};

		// 発生座標を設定
		worldTransform.translation_ = position;

		// 楕円ごとの最大サイズを設定
		maximumScales_[i] = {
		    widthDistribution(randomEngine),
		    lengthDistribution(randomEngine),
		    1.0f,
		};

		// ワールド行列を更新
		UpdateWorldTransform(worldTransform);
	}

	// スプレッド状態から開始
	state_ = State::kSpread;

	// カウンターを初期化
	counter_ = 0;
}

void HitEffect::Update() {

	switch (state_) {

	case State::kSpread: {

		// スプレッド状態の進行度
		float t = static_cast<float>(counter_) / static_cast<float>(kSpreadDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		// 勢いよく広がる補間
		const float easedT = EaseOutCubic(t);

		for (size_t i = 0; i < ellipseWorldTransforms_.size(); ++i) {

			KamataEngine::WorldTransform& worldTransform = ellipseWorldTransforms_[i];

			const KamataEngine::Vector3& maximumScale = maximumScales_[i];

			worldTransform.scale_ = {
			    Lerp(0.0f, maximumScale.x, easedT),

			    Lerp(0.0f, maximumScale.y, easedT),

			    1.0f,
			};

			UpdateWorldTransform(worldTransform);
		}

		// カウンターを加算
		++counter_;

		// スプレッド終了
		if (counter_ >= kSpreadDuration) {

			state_ = State::kFade;
			counter_ = 0;
		}

		break;
	}

	case State::kFade: {

		// フェード状態の進行度
		float t = static_cast<float>(counter_) / static_cast<float>(kFadeDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		// 徐々に縮小する補間
		const float easedT = EaseInCubic(t);

		for (size_t i = 0; i < ellipseWorldTransforms_.size(); ++i) {

			KamataEngine::WorldTransform& worldTransform = ellipseWorldTransforms_[i];

			const KamataEngine::Vector3& maximumScale = maximumScales_[i];

			worldTransform.scale_ = {
			    Lerp(maximumScale.x, 0.0f, easedT),

			    Lerp(maximumScale.y, 0.0f, easedT),

			    1.0f,
			};

			UpdateWorldTransform(worldTransform);
		}

		// カウンターを加算
		++counter_;

		// フェード終了
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

void HitEffect::Draw() {

	// デス状態では描画しない
	if (state_ == State::kDeath) {
		return;
	}

	// 楕円エフェクトをまとめて描画
	for (KamataEngine::WorldTransform& worldTransform : ellipseWorldTransforms_) {

		model_->Draw(worldTransform, *camera_);
	}
}