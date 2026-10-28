
#define NOMINMAX

#include "CameraController.h"
#include "Player.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end, float t) {
	KamataEngine::Vector3 result{};

	result.x = start.x + (end.x - start.x) * t;
	result.y = start.y + (end.y - start.y) * t;
	result.z = start.z + (end.z - start.z) * t;

	return result;
}

} // namespace

void CameraController::Initialize() {
	camera_.Initialize();

	shakePower_ = 0.0f;
	shakeTimer_ = 0;
	shakeDuration_ = 0;
	shakeOffset_ = {};
}

void CameraController::Reset() {
	assert(target_);

	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	camera_.translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;

	camera_.translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;

	camera_.translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;

	shakePower_ = 0.0f;
	shakeTimer_ = 0;
	shakeDuration_ = 0;
	shakeOffset_ = {};

	camera_.UpdateMatrix();
}

// ========================================
// カメラシェイク開始
// ========================================

void CameraController::StartShake(float power, int duration) {

	if (duration <= 0 || power <= 0.0f) {
		return;
	}

	// 揺れている最中は再開始しない
	if (shakeTimer_ > 0) {
		return;
	}

	shakePower_ = power;
	shakeDuration_ = duration;
	shakeTimer_ = duration;
}

// ========================================
// カメラ更新
// ========================================

void CameraController::Update() {

	assert(target_);

	// 前回の揺れを取り除く
	camera_.translation_.x -= shakeOffset_.x;
	camera_.translation_.y -= shakeOffset_.y;

	shakeOffset_ = {};

	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	const KamataEngine::Vector3 targetVelocity = target_->GetVelocity();

	targetPosition_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;

	targetPosition_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;

	targetPosition_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;

	// 通常の追従
	camera_.translation_ = Lerp(camera_.translation_, targetPosition_, kInterpolationRate);

	// 画面外に出ないよう補正
	camera_.translation_.x = (std::max)(camera_.translation_.x, targetWorldTransform.translation_.x + targetMargin_.left);

	camera_.translation_.x = (std::min)(camera_.translation_.x, targetWorldTransform.translation_.x + targetMargin_.right);

	camera_.translation_.y = (std::max)(camera_.translation_.y, targetWorldTransform.translation_.y + targetMargin_.bottom);

	camera_.translation_.y = (std::min)(camera_.translation_.y, targetWorldTransform.translation_.y + targetMargin_.top);

	camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);

	camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);

	// ========================================
	// ヒット時のカメラシェイク
	// ========================================

	if (shakeTimer_ > 0) {

		float progress = static_cast<float>(shakeTimer_) / static_cast<float>(shakeDuration_);

		float power = shakePower_ * progress;

		float time = static_cast<float>(shakeDuration_ - shakeTimer_);

		shakeOffset_.x = std::sin(time * 2.7f) * power;

		shakeOffset_.y = std::cos(time * 3.4f) * power;

		camera_.translation_.x += shakeOffset_.x;
		camera_.translation_.y += shakeOffset_.y;

		--shakeTimer_;
	}

	camera_.UpdateMatrix();
}
