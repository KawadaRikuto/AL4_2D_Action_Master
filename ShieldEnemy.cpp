#include "ShieldEnemy.h"

#include "GameScene.h"
#include "Player.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

namespace {

float Lerp(float start, float end, float t) { return start + (end - start) * t; }

float EaseOut(float start, float end, float t) {

	t = 1.0f - (1.0f - t) * (1.0f - t);

	return Lerp(start, end, t);
}

} // namespace

void ShieldEnemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {

	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	direction_ = Direction::kLeft;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {
	    -kWalkSpeed,
	    0.0f,
	    0.0f,
	};

	walkTimer_ = 0.0f;
	deathTimer_ = 0.0f;
	guardParameter_ = 0;

	behavior_ = Behavior::kRoot;
	behaviorRequest_ = Behavior::kUnknown;

	isDead_ = false;
	isCollisionDisabled_ = false;

	UpdateWorldTransform(worldTransform_);
}

void ShieldEnemy::Update() {

	if (behaviorRequest_ != Behavior::kUnknown) {

		behavior_ = behaviorRequest_;

		switch (behavior_) {

		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;

		case Behavior::kDeath:
			BehaviorDeathInitialize();
			break;

		case Behavior::kGuard:
			BehaviorGuardInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {

	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;

	case Behavior::kDeath:
		BehaviorDeathUpdate();
		break;

	case Behavior::kGuard:
		BehaviorGuardUpdate();
		break;
	}
}

void ShieldEnemy::BehaviorRootInitialize() {

	isCollisionDisabled_ = false;

	worldTransform_.rotation_.z = 0.0f;

	worldTransform_.scale_ = {
	    1.0f,
	    1.0f,
	    1.0f,
	};

	if (direction_ == Direction::kLeft) {

		velocity_ = {
		    -kWalkSpeed,
		    0.0f,
		    0.0f,
		};

	} else {

		velocity_ = {
		    kWalkSpeed,
		    0.0f,
		    0.0f,
		};
	}
}

void ShieldEnemy::BehaviorDeathInitialize() {

	deathTimer_ = 0.0f;
	velocity_ = {};
	isCollisionDisabled_ = true;
}

void ShieldEnemy::BehaviorGuardInitialize() {

	// ガードの最初はリアクションフェーズ
	guardPhase_ = GuardPhase::kReaction;

	// カウンターを初期化
	guardParameter_ = 0;

	// ガード中は移動を停止
	velocity_ = {};

	// ガード中は連続衝突を防ぐ
	isCollisionDisabled_ = true;
}

void ShieldEnemy::BehaviorRootUpdate() {

	worldTransform_.translation_.x += velocity_.x;

	worldTransform_.translation_.y += velocity_.y;

	worldTransform_.translation_.z += velocity_.z;

	if (velocity_.x < 0.0f) {

		direction_ = Direction::kLeft;

		worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	} else if (velocity_.x > 0.0f) {

		direction_ = Direction::kRight;

		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	}

	walkTimer_ += 1.0f / 60.0f;

	float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);

	float t = (param + 1.0f) / 2.0f;

	float walkMotionAngle = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);

	worldTransform_.rotation_.x = walkMotionAngle * std::numbers::pi_v<float> / 180.0f;

	UpdateWorldTransform(worldTransform_);
}

void ShieldEnemy::BehaviorDeathUpdate() {

	deathTimer_ += 1.0f / 60.0f;

	worldTransform_.rotation_.y += 0.5f;
	worldTransform_.rotation_.x += 0.05f;

	if (deathTimer_ >= kDeathMotionTime) {
		isDead_ = true;
	}

	UpdateWorldTransform(worldTransform_);
}

void ShieldEnemy::BehaviorGuardUpdate() {

	switch (guardPhase_) {

	case GuardPhase::kReaction:
	default: {

		float t = static_cast<float>(guardParameter_) / static_cast<float>(kGuardReactionDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		// 下向きから天井方向へ仰け反る
		const float rotation = direction_ == Direction::kLeft ? -kGuardRotation : kGuardRotation;

		worldTransform_.rotation_.z = EaseOut(0.0f, rotation, t);

		// ガードの衝撃で少し潰す
		worldTransform_.scale_.y = EaseOut(1.0f, 0.75f, t);

		worldTransform_.scale_.x = EaseOut(1.0f, 1.15f, t);

		if (guardParameter_ >= kGuardReactionDuration) {

			guardPhase_ = GuardPhase::kRecovery;

			guardParameter_ = 0;
		}

		break;
	}

	case GuardPhase::kRecovery: {

		float t = static_cast<float>(guardParameter_) / static_cast<float>(kGuardRecoveryDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		const float rotation = direction_ == Direction::kLeft ? -kGuardRotation : kGuardRotation;

		// 元の姿勢へ戻す
		worldTransform_.rotation_.z = EaseOut(rotation, 0.0f, t);

		worldTransform_.scale_.y = EaseOut(0.75f, 1.0f, t);

		worldTransform_.scale_.x = EaseOut(1.15f, 1.0f, t);

		if (guardParameter_ >= kGuardRecoveryDuration) {

			worldTransform_.rotation_.z = 0.0f;

			worldTransform_.scale_ = {
			    1.0f,
			    1.0f,
			    1.0f,
			};

			behaviorRequest_ = Behavior::kRoot;

			guardParameter_ = 0;
		}

		break;
	}
	}

	++guardParameter_;

	UpdateWorldTransform(worldTransform_);
}

void ShieldEnemy::Draw() { model_->Draw(worldTransform_, *camera_); }

KamataEngine::Vector3 ShieldEnemy::GetWorldPosition() {

	KamataEngine::Vector3 worldPosition;

	worldPosition.x = worldTransform_.matWorld_.m[3][0];

	worldPosition.y = worldTransform_.matWorld_.m[3][1];

	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

AABB ShieldEnemy::GetAABB() {

	KamataEngine::Vector3 worldPosition = GetWorldPosition();

	AABB aabb;

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

void ShieldEnemy::OnCollision(Player* player) {

	assert(player);

	if (behavior_ == Behavior::kDeath || behavior_ == Behavior::kGuard) {

		return;
	}

	if (player->IsAttack()) {

		const bool isPlayerFacingRight = player->IsFacingRight();

		const bool isEnemyFacingRight = direction_ == Direction::kRight;

		const bool isGuardFromLeft = isPlayerFacingRight && !isEnemyFacingRight;

		const bool isGuardFromRight = !isPlayerFacingRight && isEnemyFacingRight;

		// 前方からの攻撃ならガード成功
		if (isGuardFromLeft || isGuardFromRight) {

			const KamataEngine::Vector3 guardPosition = GetWorldPosition();

			gameScene_->CreateGuardEffect(guardPosition);

			// プレイヤーへノックバックを要求
			player->RequestKnockback();

			// ガード行動への切り替えを要求
			behaviorRequest_ = Behavior::kGuard;

			return;
		}

		behaviorRequest_ = Behavior::kDeath;

		KamataEngine::Vector3 enemyPosition = GetWorldPosition();

		KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

		KamataEngine::Vector3 effectPosition = {
		    (enemyPosition.x + playerPosition.x) / 2.0f,

		    (enemyPosition.y + playerPosition.y) / 2.0f,

		    (enemyPosition.z + playerPosition.z) / 2.0f,
		};

		gameScene_->CreateHitEffect(effectPosition);
	}
}
