
#include "Enemy.h"

#include "Player.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

namespace {

float Lerp(float start, float end, float t) { return start + (end - start) * t; }

} // namespace

// ========================================
// デストラクタ
// ========================================

Enemy::~Enemy() {

	// E0 ソードモデル
	for (int i = 0; i < kSwordModelCount; ++i) {
		delete swordModels_[i];
		swordModels_[i] = nullptr;
	}

	// E1 ファイヤーモデル
	for (int i = 0; i < kFireModelCount; ++i) {
		delete fireModels_[i];
		fireModels_[i] = nullptr;
	}

	// model_はGameScene側で管理
}

// ========================================
// 初期化
// ========================================

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, EnemyType type) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;
	enemyType_ = type;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 最初は左向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	walkTimer_ = 0.0f;

	isBeingInhaled_ = false;
	isHeld_ = false;
	isSpitting_ = false;
	isFinished_ = false;

	spitParameter_ = 0;

	swordAttackPhase_ = SwordAttackPhase::kMove;
	swordAttackTimer_ = 0;
	swordFacingRight_ = false;

	fireDashPhase_ = FireDashPhase::kMove;
	fireDashTimer_ = 0;
	fireFacingRight_ = false;

	objectColor_.Initialize();

	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// ========================================
	// E0 ソード敵
	// ========================================

	if (enemyType_ == EnemyType::kSword) {

		swordModels_[0] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack1", true);

		swordModels_[1] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack2", true);

		swordModels_[2] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack3", true);

		swordModels_[3] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack4", true);

		// 青色
		color_ = {0.2f, 0.4f, 1.0f, 1.0f};

		velocity_ = {};
	}

	// ========================================
	// E1 ファイヤー敵
	// ========================================

	if (enemyType_ == EnemyType::kFire) {

		// 直立
		fireModels_[0] = KamataEngine::Model::CreateFromOBJ("EnemyAttack1", true);

		// 突進開始
		fireModels_[1] = KamataEngine::Model::CreateFromOBJ("EnemyAttack2", true);

		// 突進中
		fireModels_[2] = KamataEngine::Model::CreateFromOBJ("EnemyAttack3", true);

		// 赤色
		color_ = {1.0f, 0.2f, 0.1f, 1.0f};

		velocity_ = {};
	}

	objectColor_.SetColor(color_);

	UpdateWorldTransform(worldTransform_);
}

// ========================================
// 更新
// ========================================

void Enemy::Update(const Player* player) {

	// ========================================
	// 保持中
	// ========================================

	if (isHeld_) {

		if (player) {
			UpdateHeldPosition(player);
		}

		velocity_ = {};
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// ========================================
	// 吐き出し中
	// ========================================

	if (isSpitting_) {

		UpdateSpit();
		UpdateWorldTransform(worldTransform_);
		return;
	}

	// ========================================
	// 吸い込み
	// ========================================

	if (player) {
		UpdateInhale(player);
	}

	if (isBeingInhaled_) {

		velocity_ = {};

		swordAttackPhase_ = SwordAttackPhase::kMove;
		swordAttackTimer_ = 0;

		fireDashPhase_ = FireDashPhase::kMove;
		fireDashTimer_ = 0;

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// ========================================
	// E0 ソード敵
	// ========================================

	if (enemyType_ == EnemyType::kSword) {

		UpdateSwordEnemy(player);

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// ========================================
	// E1 ファイヤー敵
	// ========================================

	if (enemyType_ == EnemyType::kFire) {

		UpdateFireEnemy(player);

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// ========================================
	// 通常敵
	// ========================================

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

// ========================================
// E0 ソード敵：追跡
// ========================================

void Enemy::UpdateSwordEnemy(const Player* player) {

	if (swordAttackPhase_ != SwordAttackPhase::kMove) {

		UpdateSwordAttack();
		return;
	}

	if (!player) {

		velocity_ = {};
		return;
	}

	KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

	KamataEngine::Vector3 enemyPosition = worldTransform_.translation_;

	float differenceX = playerPosition.x - enemyPosition.x;

	float differenceY = playerPosition.y - enemyPosition.y;

	float distanceX = std::abs(differenceX);
	float distanceY = std::abs(differenceY);

	UpdateSwordDirection(differenceX);

	worldTransform_.rotation_.x = 0.0f;

	if (distanceX <= kSwordAttackStartRange && distanceY <= kSwordAttackHeightRange) {

		velocity_ = {};

		swordAttackPhase_ = SwordAttackPhase::kWindup;
		swordAttackTimer_ = 0;

		return;
	}

	if (distanceX > kSwordStopDistance) {

		float moveDistance = distanceX - kSwordStopDistance;

		float moveSpeed = (std::min)(kWalkSpeed, moveDistance);

		velocity_.x = differenceX > 0.0f ? moveSpeed : -moveSpeed;

		worldTransform_.translation_.x += velocity_.x;

	} else {

		velocity_ = {};
	}
}

// ========================================
// E0 ソード敵：向き
// ========================================

void Enemy::UpdateSwordDirection(float directionX) {

	if (std::abs(directionX) < 0.01f) {
		return;
	}

	swordFacingRight_ = directionX > 0.0f;

	if (swordFacingRight_) {

		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	} else {

		worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	}
}

// ========================================
// E0 ソード敵：攻撃
// ========================================

void Enemy::UpdateSwordAttack() {

	velocity_ = {};
	swordAttackTimer_++;

	switch (swordAttackPhase_) {

	case SwordAttackPhase::kWindup:

		if (swordAttackTimer_ >= kSwordWindupDuration) {

			swordAttackPhase_ = SwordAttackPhase::kSwing;
			swordAttackTimer_ = 0;
		}

		break;

	case SwordAttackPhase::kSwing:

		if (swordAttackTimer_ >= kSwordSwingDuration) {

			swordAttackPhase_ = SwordAttackPhase::kFinish;
			swordAttackTimer_ = 0;
		}

		break;

	case SwordAttackPhase::kFinish:

		if (swordAttackTimer_ >= kSwordFinishDuration) {

			swordAttackPhase_ = SwordAttackPhase::kCooldown;
			swordAttackTimer_ = 0;
		}

		break;

	case SwordAttackPhase::kCooldown:

		if (swordAttackTimer_ >= kSwordCooldownDuration) {

			swordAttackPhase_ = SwordAttackPhase::kMove;
			swordAttackTimer_ = 0;
		}

		break;

	case SwordAttackPhase::kMove:
		break;
	}
}

// ========================================
// E1 ファイヤー敵：向き
// ========================================

void Enemy::UpdateFireDirection(float directionX) {

	if (std::abs(directionX) < 0.01f) {
		return;
	}

	fireFacingRight_ = directionX > 0.0f;

	if (fireFacingRight_) {

		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	} else {

		worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	}
}

// ========================================
// E1 ファイヤー敵：追跡・突進
// ========================================

void Enemy::UpdateFireEnemy(const Player* player) {

	// E1はモデル自体で動きを表現する
	worldTransform_.rotation_.x = 0.0f;

	switch (fireDashPhase_) {

		// ========================================
		// 直立モデルでプレイヤーを追跡
		// EnemyAttack1
		// ========================================

	case FireDashPhase::kMove: {

		velocity_ = {};

		if (!player) {
			return;
		}

		KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

		KamataEngine::Vector3 enemyPosition = worldTransform_.translation_;

		float differenceX = playerPosition.x - enemyPosition.x;

		float differenceY = playerPosition.y - enemyPosition.y;

		float distanceX = std::abs(differenceX);
		float distanceY = std::abs(differenceY);

		// プレイヤーの方向を向く
		UpdateFireDirection(differenceX);

		// プレイヤーの前に到達
		if (distanceX <= kFireAttackStartRange && distanceY <= kFireAttackHeightRange) {

			velocity_ = {};

			fireDashPhase_ = FireDashPhase::kCharge;
			fireDashTimer_ = 0;

			return;
		}

		// プレイヤーの手前まで横移動
		if (distanceX > kFireStopDistance) {

			float moveDistance = distanceX - kFireStopDistance;

			float moveSpeed = (std::min)(kWalkSpeed, moveDistance);

			velocity_.x = differenceX > 0.0f ? moveSpeed : -moveSpeed;

			worldTransform_.translation_.x += velocity_.x;

		} else {

			velocity_ = {};
		}

		break;
	}

		// ========================================
		// 突進準備
		// EnemyAttack2
		// ========================================

	case FireDashPhase::kCharge: {

		velocity_ = {};

		fireDashTimer_++;

		if (fireDashTimer_ >= kFireChargeDuration) {

			fireDashPhase_ = FireDashPhase::kDash;
			fireDashTimer_ = 0;
		}

		break;
	}

		// ========================================
		// 高速突進
		// EnemyAttack3
		// ========================================

	case FireDashPhase::kDash: {

		// 突進開始時の向きを維持
		float direction = fireFacingRight_ ? 1.0f : -1.0f;

		velocity_.x = direction * kFireDashSpeed;
		velocity_.y = 0.0f;
		velocity_.z = 0.0f;

		worldTransform_.translation_.x += velocity_.x;

		fireDashTimer_++;

		if (fireDashTimer_ >= kFireDashDuration) {

			velocity_ = {};

			fireDashPhase_ = FireDashPhase::kRecovery;
			fireDashTimer_ = 0;
		}

		break;
	}

		// ========================================
		// 突進終了後の待機
		// EnemyAttack1
		// ========================================

	case FireDashPhase::kRecovery: {

		velocity_ = {};

		fireDashTimer_++;

		if (fireDashTimer_ >= kFireRecoveryDuration) {

			fireDashPhase_ = FireDashPhase::kMove;
			fireDashTimer_ = 0;
		}

		break;
	}
	}
}

// ========================================
// 吸い込み
// ========================================

void Enemy::UpdateInhale(const Player* player) {

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

	if (distance <= kInhaleStopDistance) {

		worldTransform_.translation_.x = targetPosition.x;
		worldTransform_.translation_.y = targetPosition.y;
		worldTransform_.translation_.z = targetPosition.z;

		isBeingInhaled_ = false;
		isHeld_ = true;

		velocity_ = {};

		swordAttackPhase_ = SwordAttackPhase::kMove;
		swordAttackTimer_ = 0;

		fireDashPhase_ = FireDashPhase::kMove;
		fireDashTimer_ = 0;

		return;
	}

	if (distance > 0.0f) {

		float moveX = differenceX / distance * kInhaleSpeed;

		float moveY = differenceY / distance * kInhaleSpeed;

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

// ========================================
// 保持中の位置更新
// ========================================

void Enemy::UpdateHeldPosition(const Player* player) {

	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	worldTransform_.translation_.x = targetPosition.x;
	worldTransform_.translation_.y = targetPosition.y;
	worldTransform_.translation_.z = targetPosition.z;
}

// ========================================
// 吐き出し開始
// ========================================

void Enemy::StartSpit(bool isFacingRight) {

	if (!isHeld_) {
		return;
	}

	isHeld_ = false;
	isBeingInhaled_ = false;
	isSpitting_ = true;
	isFinished_ = false;

	spitParameter_ = 0;

	swordAttackPhase_ = SwordAttackPhase::kMove;
	swordAttackTimer_ = 0;

	fireDashPhase_ = FireDashPhase::kMove;
	fireDashTimer_ = 0;

	velocity_.x = isFacingRight ? kSpitSpeed : -kSpitSpeed;

	velocity_.y = 0.0f;
	velocity_.z = 0.0f;
}

// ========================================
// 吐き出し更新
// ========================================

void Enemy::UpdateSpit() {

	worldTransform_.translation_.x += velocity_.x;

	++spitParameter_;

	if (spitParameter_ >= kSpitDuration) {

		isSpitting_ = false;
		isFinished_ = true;
	}
}

// ========================================
// 描画
// ========================================

void Enemy::Draw() {

	// ========================================
	// 追加：吸い込んで保持している敵は描画しない
	// ========================================

	if (isHeld_) {
		return;
	}

	if (!camera_) {
		return;
	}

	KamataEngine::Model* drawModel = model_;

	// ========================================
	// E0 ソード敵
	// ========================================

	if (enemyType_ == EnemyType::kSword) {

		int modelIndex = 0;

		switch (swordAttackPhase_) {

		case SwordAttackPhase::kMove:
			modelIndex = 0;
			break;

		case SwordAttackPhase::kWindup:
			modelIndex = 1;
			break;

		case SwordAttackPhase::kSwing:
			modelIndex = 2;
			break;

		case SwordAttackPhase::kFinish:
			modelIndex = 3;
			break;

		case SwordAttackPhase::kCooldown:
			modelIndex = 0;
			break;
		}

		if (isBeingInhaled_ || isSpitting_) {

			modelIndex = 0;
		}

		if (swordModels_[modelIndex]) {
			drawModel = swordModels_[modelIndex];
		}
	}

	// ========================================
	// E1 ファイヤー敵
	// ========================================

	else if (enemyType_ == EnemyType::kFire) {

		int modelIndex = 0;

		switch (fireDashPhase_) {

		// 直立・横移動
		case FireDashPhase::kMove:
			modelIndex = 0;
			break;

		// 突進開始
		case FireDashPhase::kCharge:
			modelIndex = 1;
			break;

		// 突進中
		case FireDashPhase::kDash:
			modelIndex = 2;
			break;

		// 突進後は直立
		case FireDashPhase::kRecovery:
			modelIndex = 0;
			break;
		}

		if (isBeingInhaled_ || isSpitting_) {

			modelIndex = 0;
		}

		if (fireModels_[modelIndex]) {
			drawModel = fireModels_[modelIndex];
		}
	}

	if (drawModel) {
		drawModel->Draw(worldTransform_, *camera_, &objectColor_);
	}
}

// ========================================
// ワールド座標取得
// ========================================

KamataEngine::Vector3 Enemy::GetWorldPosition() {

	KamataEngine::Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

// ========================================
// 当たり判定
// ========================================

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 worldPosition = GetWorldPosition();

	AABB aabb{};

	// 敵本体
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

	// ========================================
	// E0 剣攻撃
	// ========================================

	if (enemyType_ == EnemyType::kSword && swordAttackPhase_ == SwordAttackPhase::kSwing && !isBeingInhaled_ && !isHeld_ && !isSpitting_) {

		float attackMinX;
		float attackMaxX;

		if (swordFacingRight_) {

			attackMinX = worldPosition.x + kWidth / 2.0f;

			attackMaxX = attackMinX + kSwordAttackRange;

		} else {

			attackMaxX = worldPosition.x - kWidth / 2.0f;

			attackMinX = attackMaxX - kSwordAttackRange;
		}

		float attackMinY = worldPosition.y - kSwordAttackHeight / 2.0f;

		float attackMaxY = worldPosition.y + kSwordAttackHeight / 2.0f;

		aabb.min.x = (std::min)(aabb.min.x, attackMinX);

		aabb.max.x = (std::max)(aabb.max.x, attackMaxX);

		aabb.min.y = (std::min)(aabb.min.y, attackMinY);

		aabb.max.y = (std::max)(aabb.max.y, attackMaxY);
	}

	return aabb;
}

// ========================================
// プレイヤーとの衝突
// ========================================

void Enemy::OnCollision(const Player* player) {

	(void)player;

	// E1は突進がプレイヤーに当たったら停止
	if (enemyType_ == EnemyType::kFire && fireDashPhase_ == FireDashPhase::kDash) {

		velocity_ = {};

		fireDashPhase_ = FireDashPhase::kRecovery;
		fireDashTimer_ = 0;
	}
}
