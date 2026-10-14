
#include "Enemy.h"

#include "Player.h"
#include "WorldTransformUpdate.h"

#include <cassert>
#include <cmath>
#include <numbers>

namespace {

float Lerp(float start, float end, float t) { return start + (end - start) * t; }

} // namespace

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, EnemyType type) {

	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;
	enemyType_ = type;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	walkTimer_ = 0.0f;
	isBeingInhaled_ = false;
	isHeld_ = false;
	isSpitting_ = false;
	spitParameter_ = 0;

	UpdateWorldTransform(worldTransform_);
}

void Enemy::Update(const Player* player) {

	// 吸い込み完了後は、口元に固定する。
	// 通常移動や吸い込み判定は行わない。
	if (isHeld_) {
		if (player) {
			UpdateHeldPosition(player);
		}

		velocity_ = {};
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吐き出し中は、通常の歩行処理を行わない。
	if (isSpitting_) {
		UpdateSpit();
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吸い込み判定
	if (player) {
		UpdateInhale(player);
	}

	// 吸い込み中は通常移動を完全に停止する。
	// 口元への移動は UpdateInhale() 内だけで行う。
	if (isBeingInhaled_) {
		velocity_ = {};
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 通常移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	walkTimer_ += 1.0f / 60.0f;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);

	float t = (param + 1.0f) / 2.0f;

	float walkMotionAngle = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);

	worldTransform_.rotation_.x = walkMotionAngle * std::numbers::pi_v<float> / 180.0f;

	UpdateWorldTransform(worldTransform_);
}

void Enemy::UpdateInhale(const Player* player) {

	// Zキーが押されている間だけ、新しい吸い込みを行う。
	if (!player->IsInhaling()) {
		isBeingInhaled_ = false;
		return;
	}

	KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

	KamataEngine::Vector3 enemyPosition = GetWorldPosition();

	float heightDifference = std::abs(enemyPosition.y - playerPosition.y);

	if (heightDifference > kInhaleHeightRange) {
		isBeingInhaled_ = false;
		return;
	}

	float distanceX = enemyPosition.x - playerPosition.x;
	bool facingRight = player->IsFacingRight();

	bool isInFront = false;

	if (facingRight) {
		if (distanceX > 0.0f && distanceX <= kInhaleRange) {
			isInFront = true;
		}
	} else {
		if (distanceX < 0.0f && -distanceX <= kInhaleRange) {
			isInFront = true;
		}
	}

	if (!isInFront) {
		isBeingInhaled_ = false;
		return;
	}

	isBeingInhaled_ = true;
	velocity_ = {};

	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	float differenceX = targetPosition.x - enemyPosition.x;

	float differenceY = targetPosition.y - enemyPosition.y;

	float distance = std::sqrt(differenceX * differenceX + differenceY * differenceY);

	// 口元に到達したら、その場で保持状態にする。
	if (distance <= kInhaleStopDistance) {
		worldTransform_.translation_.x = targetPosition.x;
		worldTransform_.translation_.y = targetPosition.y;
		worldTransform_.translation_.z = targetPosition.z;

		isBeingInhaled_ = false;
		isHeld_ = true;
		velocity_ = {};
		return;
	}

	// 敵を口元へ移動させる。
	// 通常移動は Update() 側で停止している。
	if (distance > 0.0f) {
		float moveX = differenceX / distance * kInhaleSpeed;

		float moveY = differenceY / distance * kInhaleSpeed;

		// 目標地点を通り越さないようにする。
		if (std::abs(moveX) > std::abs(differenceX)) {
			moveX = differenceX;
		}

		if (std::abs(moveY) > std::abs(differenceY)) {
			moveY = differenceY;
		}

		worldTransform_.translation_.x += moveX;
		worldTransform_.translation_.y += moveY;
	}
}

void Enemy::UpdateHeldPosition(const Player* player) {

	// プレイヤーが移動しても、敵を口元に保持する。
	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	worldTransform_.translation_.x = targetPosition.x;
	worldTransform_.translation_.y = targetPosition.y;
	worldTransform_.translation_.z = targetPosition.z;
}

void Enemy::StartSpit(bool isFacingRight) {

	if (!isHeld_) {
		return;
	}

	isHeld_ = false;
	isBeingInhaled_ = false;
	isSpitting_ = true;

	spitParameter_ = 0;

	velocity_.x = isFacingRight ? kSpitSpeed : -kSpitSpeed;
	velocity_.y = kSpitUpSpeed;
	velocity_.z = 0.0f;
}

void Enemy::UpdateSpit() {

	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;

	// 吐き出された敵に重力を加える。
	velocity_.y -= kGravity;

	++spitParameter_;

	if (spitParameter_ >= kSpitDuration) {
		isSpitting_ = false;

		// 吐き出し終了後は通常の歩行に戻す。
		velocity_.x = -kWalkSpeed;
		velocity_.y = 0.0f;
		velocity_.z = 0.0f;
	}
}

void Enemy::Draw() {

	if (model_ && camera_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {

	KamataEngine::Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 worldPosition = GetWorldPosition();

	AABB aabb{};

	aabb.min = {
	    worldPosition.x - kWidth / 2.0f,
	    worldPosition.y - kHeight / 2.0f,
	    worldPosition.z - kWidth / 2.0f,
	};

	aabb.max = {
	    worldPosition.x + kWidth / 2.0f,
	    worldPosition.y + kHeight / 2.0f,
	    worldPosition.z + kWidth / 2.0f,
	};

	return aabb;
}

void Enemy::OnCollision(const Player* player) { (void)player; }
