#include "DeathParticles.h"

#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

/// <summary>
/// Z軸回転行列を作成
/// </summary>
KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);

	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);

	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	return result;
}

/// <summary>
/// ベクトルを行列で変換
/// </summary>
KamataEngine::Vector3 Transform(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix) {

	KamataEngine::Vector3 result{};

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0];

	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1];

	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2];

	return result;
}

} // namespace

void DeathParticles::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {

	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// 色変更オブジェクトの初期化
	objectColor_.Initialize();

	// 色を白に設定
	color_ = {
	    1.0f,
	    1.0f,
	    1.0f,
	    1.0f,
	};

	// 色を反映
	objectColor_.SetColor(color_);

	// 終了フラグをリセット
	isFinished_ = false;

	// 経過時間をリセット
	counter_ = 0.0f;

	// ワールド変換の初期化
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {

		worldTransform.Initialize();

		// プレイヤー中心座標
		worldTransform.translation_ = position;

		// 初期状態の行列を更新
		UpdateWorldTransform(worldTransform);
	}
}

void DeathParticles::Update() {

	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// パーティクルを8方向に移動
	for (uint32_t i = 0; i < kNumParticles; ++i) {

		// 基本となる速度ベクトル
		KamataEngine::Vector3 velocity = {
		    kSpeed,
		    0.0f,
		    0.0f,
		};

		// 回転角を計算する
		float angle = kAngleUnit * static_cast<float>(i);

		// Z軸回りの回転行列
		KamataEngine::Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);

		// 基本ベクトルを回転させる
		velocity = Transform(velocity, matrixRotation);

		// 移動
		worldTransforms_[i].translation_.x += velocity.x;
		worldTransforms_[i].translation_.y += velocity.y;
		worldTransforms_[i].translation_.z += velocity.z;

		// ワールド変換の更新
		UpdateWorldTransform(worldTransforms_[i]);
	}

	// カウンターを1フレーム分だけ加算する
	counter_ += 1.0f / 60.0f;

	// アルファ値を計算
	color_.w = std::clamp(1.0f - counter_ / kDuration, 0.0f, 1.0f);

	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {

		// カウンターを上限に合わせる
		counter_ = kDuration;

		// 終了扱いにする
		isFinished_ = true;
	}
}

void DeathParticles::Draw() {

	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// モデルの描画
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {

		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}