#define NOMINMAX

#include "CameraController.h"

#include "Player.h"

#include <algorithm>
#include <cassert>

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

	// カメラの初期化
	camera_.Initialize();
}

void CameraController::Reset() {

	assert(target_);

	// 追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラの座標を計算
	camera_.translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;

	camera_.translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;

	camera_.translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;

	// 行列を更新する
	camera_.UpdateMatrix();
}

void CameraController::Update() {

	assert(target_);

	// 追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象の速度を取得
	const KamataEngine::Vector3 targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットと速度からカメラの目標座標を計算
	targetPosition_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;

	targetPosition_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;

	targetPosition_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;

	// 座標補間によりゆったり追従
	camera_.translation_ = Lerp(camera_.translation_, targetPosition_, kInterpolationRate);

	// 追従対象が画面外に出ないように補正
	camera_.translation_.x = std::max(camera_.translation_.x, targetWorldTransform.translation_.x + targetMargin_.left);

	camera_.translation_.x = std::min(camera_.translation_.x, targetWorldTransform.translation_.x + targetMargin_.right);

	camera_.translation_.y = std::max(camera_.translation_.y, targetWorldTransform.translation_.y + targetMargin_.bottom);

	camera_.translation_.y = std::min(camera_.translation_.y, targetWorldTransform.translation_.y + targetMargin_.top);

	// カメラ移動範囲
	camera_.translation_.x = std::clamp(camera_.translation_.x, movableArea_.left, movableArea_.right);

	camera_.translation_.y = std::clamp(camera_.translation_.y, movableArea_.bottom, movableArea_.top);

	// 行列を更新する
	camera_.UpdateMatrix();
}