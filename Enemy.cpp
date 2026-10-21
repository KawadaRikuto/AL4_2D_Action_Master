
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
	isFinished_ = false;
	spitParameter_ = 0;

	UpdateWorldTransform(worldTransform_);
}

void Enemy::Update(const Player* player) {

	// 吸い込み完了後は口元に固定する
	if (isHeld_) {
		if (player) {
			UpdateHeldPosition(player);
		}

		velocity_ = {};
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吐き出し中は通常の歩行処理を行わない
	if (isSpitting_) {
		UpdateSpit();
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吸い込み判定
	if (player) {
		UpdateInhale(player);
	}

	// 吸い込み中は通常移動を停止
	if (isBeingInhaled_) {
		velocity_ = {};
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 通常移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 歩行アニメーション
	walkTimer_ += 1.0f / 60.0f;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);

	float t = (param + 1.0f) / 2.0f;

	float walkMotionAngle = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);

	worldTransform_.rotation_.x = walkMotionAngle * std::numbers::pi_v<float> / 180.0f;

	UpdateWorldTransform(worldTransform_);
}

void Enemy::UpdateInhale(const Player* player) {

	// Zキーが押されている間だけ吸い込みを行う
	if (!player->IsInhaling()) {
		isBeingInhaled_ = false;
		return;
	}

	KamataEngine::Vector3 playerPosition = player->GetWorldPosition();
	KamataEngine::Vector3 enemyPosition = GetWorldPosition();

	// 高さが離れすぎている場合は吸い込まない
	float heightDifference = std::abs(enemyPosition.y - playerPosition.y);

	if (heightDifference > kInhaleHeightRange) {
		isBeingInhaled_ = false;
		return;
	}

	// プレイヤーの前方に敵がいるか確認
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

	// プレイヤーの口元を目標にする
	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	float differenceX = targetPosition.x - enemyPosition.x;
	float differenceY = targetPosition.y - enemyPosition.y;

	float distance = std::sqrt(differenceX * differenceX + differenceY * differenceY);

	// 口元に到達したら保持状態にする
	if (distance <= kInhaleStopDistance) {
		worldTransform_.translation_.x = targetPosition.x;
		worldTransform_.translation_.y = targetPosition.y;
		worldTransform_.translation_.z = targetPosition.z;

		isBeingInhaled_ = false;
		isHeld_ = true;
		velocity_ = {};
		return;
	}

	// 口元へ移動
	if (distance > 0.0f) {
		float moveX = differenceX / distance * kInhaleSpeed;
		float moveY = differenceY / distance * kInhaleSpeed;

		// 目標地点を通り越さないようにする
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

	// プレイヤーが動いても敵を口元に保持する
	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	worldTransform_.translation_.x = targetPosition.x;
	worldTransform_.translation_.y = targetPosition.y;
	worldTransform_.translation_.z = targetPosition.z;
}

void Enemy::StartSpit(bool isFacingRight) {

	// 保持中の敵だけ吐き出せる
	if (!isHeld_) {
		return;
	}

	isHeld_ = false;
	isBeingInhaled_ = false;
	isSpitting_ = true;
	isFinished_ = false;

	spitParameter_ = 0;

	// プレイヤーが向いている方向へ水平に飛ばす
	velocity_.x = isFacingRight ? kSpitSpeed : -kSpitSpeed;
	velocity_.y = 0.0f;
	velocity_.z = 0.0f;
}

void Enemy::UpdateSpit() {

	// X方向だけ移動させ、一直線に飛ばす
	worldTransform_.translation_.x += velocity_.x;

	++spitParameter_;

	// 飛行時間が終わったら消滅フラグを立てる
	if (spitParameter_ >= kSpitDuration) {
		isSpitting_ = false;
		isFinished_ = true;
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
