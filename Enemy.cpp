
#include "Enemy.h"

#include "MapChipField.h"
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

	for (int i = 0; i < kSwordModelCount; ++i) {
		delete swordModels_[i];
		swordModels_[i] = nullptr;
	}

	for (int i = 0; i < kFireModelCount; ++i) {
		delete fireModels_[i];
		fireModels_[i] = nullptr;
	}
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

	mapChipField_ = nullptr;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	fallSpeed_ = 0.0f;
	onGround_ = false;

	wallTurnTimer_ = 0;
	wallMoveRight_ = false;

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

	if (enemyType_ == EnemyType::kSword) {

		swordModels_[0] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack1", true);

		swordModels_[1] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack2", true);

		swordModels_[2] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack3", true);

		swordModels_[3] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack4", true);

		color_ = {0.2f, 0.4f, 1.0f, 1.0f};
		velocity_ = {};
	}

	if (enemyType_ == EnemyType::kFire) {

		fireModels_[0] = KamataEngine::Model::CreateFromOBJ("EnemyAttack1", true);

		fireModels_[1] = KamataEngine::Model::CreateFromOBJ("EnemyAttack2", true);

		fireModels_[2] = KamataEngine::Model::CreateFromOBJ("EnemyAttack3", true);

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

	// プレイヤーに設定されたマップを参照
	if (player) {
		mapChipField_ = player->GetMapChipField();
	}

	// 保持中
	if (isHeld_) {

		if (player) {
			UpdateHeldPosition(player);
		}

		velocity_ = {};
		fallSpeed_ = 0.0f;

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吐き出し中
	if (isSpitting_) {

		UpdateSpit();

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 吸い込み
	if (player) {
		UpdateInhale(player);
	}

	if (isBeingInhaled_ || isHeld_) {

		velocity_ = {};
		fallSpeed_ = 0.0f;

		swordAttackPhase_ = SwordAttackPhase::kMove;
		swordAttackTimer_ = 0;

		fireDashPhase_ = FireDashPhase::kMove;
		fireDashTimer_ = 0;

		UpdateWorldTransform(worldTransform_);
		return;
	}

	// 反転後の移動時間を更新
	if (wallTurnTimer_ > 0) {
		--wallTurnTimer_;
	}

	// E0 ソード敵
	if (enemyType_ == EnemyType::kSword) {

		UpdateSwordEnemy(player);

	}
	// E1 ファイヤー敵
	else if (enemyType_ == EnemyType::kFire) {

		UpdateFireEnemy(player);

	}
	// 通常敵
	else {

		if (velocity_.x == 0.0f) {
			velocity_.x = -kWalkSpeed;
		}

		walkTimer_ += 1.0f / 60.0f;

		float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);

		float t = (param + 1.0f) / 2.0f;

		float walkMotionAngle = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);

		worldTransform_.rotation_.x = walkMotionAngle * std::numbers::pi_v<float> / 180.0f;
	}

	// 全種類の敵に重力・壁・床判定を適用
	ApplyMapPhysics();

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

	worldTransform_.rotation_.x = 0.0f;

	// 壁に当たった後は一定時間反転方向へ移動
	if (wallTurnTimer_ > 0) {

		velocity_.x = wallMoveRight_ ? kWalkSpeed : -kWalkSpeed;

		UpdateSwordDirection(velocity_.x);
		return;
	}

	if (!player) {

		velocity_.x = 0.0f;
		return;
	}

	KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

	KamataEngine::Vector3 enemyPosition = worldTransform_.translation_;

	float differenceX = playerPosition.x - enemyPosition.x;

	float differenceY = playerPosition.y - enemyPosition.y;

	float distanceX = std::abs(differenceX);
	float distanceY = std::abs(differenceY);

	UpdateSwordDirection(differenceX);

	if (distanceX <= kSwordAttackStartRange && distanceY <= kSwordAttackHeightRange) {

		velocity_.x = 0.0f;

		swordAttackPhase_ = SwordAttackPhase::kWindup;
		swordAttackTimer_ = 0;

		return;
	}

	if (distanceX > kSwordStopDistance) {

		float moveDistance = distanceX - kSwordStopDistance;

		float moveSpeed = (std::min)(kWalkSpeed, moveDistance);

		velocity_.x = differenceX > 0.0f ? moveSpeed : -moveSpeed;

	} else {

		velocity_.x = 0.0f;
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

	worldTransform_.rotation_.y = swordFacingRight_ ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
}

// ========================================
// E0 ソード敵：攻撃
// ========================================

void Enemy::UpdateSwordAttack() {

	// 攻撃中も縦方向の重力は別途適用
	velocity_.x = 0.0f;
	++swordAttackTimer_;

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

	worldTransform_.rotation_.y = fireFacingRight_ ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
}

// ========================================
// E1 ファイヤー敵：追跡・突進
// ========================================

void Enemy::UpdateFireEnemy(const Player* player) {

	worldTransform_.rotation_.x = 0.0f;

	switch (fireDashPhase_) {

	case FireDashPhase::kMove: {

		velocity_.x = 0.0f;

		// 壁で反転した直後
		if (wallTurnTimer_ > 0) {

			velocity_.x = wallMoveRight_ ? kWalkSpeed : -kWalkSpeed;

			UpdateFireDirection(velocity_.x);
			break;
		}

		if (!player) {
			break;
		}

		KamataEngine::Vector3 playerPosition = player->GetWorldPosition();

		KamataEngine::Vector3 enemyPosition = worldTransform_.translation_;

		float differenceX = playerPosition.x - enemyPosition.x;

		float differenceY = playerPosition.y - enemyPosition.y;

		float distanceX = std::abs(differenceX);
		float distanceY = std::abs(differenceY);

		UpdateFireDirection(differenceX);

		if (distanceX <= kFireAttackStartRange && distanceY <= kFireAttackHeightRange) {

			fireDashPhase_ = FireDashPhase::kCharge;
			fireDashTimer_ = 0;

			break;
		}

		if (distanceX > kFireStopDistance) {

			float moveDistance = distanceX - kFireStopDistance;

			float moveSpeed = (std::min)(kWalkSpeed, moveDistance);

			velocity_.x = differenceX > 0.0f ? moveSpeed : -moveSpeed;
		}

		break;
	}

	case FireDashPhase::kCharge: {

		velocity_.x = 0.0f;

		++fireDashTimer_;

		if (fireDashTimer_ >= kFireChargeDuration) {

			fireDashPhase_ = FireDashPhase::kDash;
			fireDashTimer_ = 0;
		}

		break;
	}

	case FireDashPhase::kDash: {

		velocity_.x = fireFacingRight_ ? kFireDashSpeed : -kFireDashSpeed;

		++fireDashTimer_;

		if (fireDashTimer_ >= kFireDashDuration) {

			fireDashPhase_ = FireDashPhase::kRecovery;
			fireDashTimer_ = 0;
		}

		break;
	}

	case FireDashPhase::kRecovery: {

		velocity_.x = 0.0f;

		++fireDashTimer_;

		if (fireDashTimer_ >= kFireRecoveryDuration) {

			fireDashPhase_ = FireDashPhase::kMove;
			fireDashTimer_ = 0;
		}

		break;
	}
	}
}

// ========================================
// マップのブロックと重なっているか
// ========================================

bool Enemy::IsBlockCollision(float x, float y) const {

	if (!mapChipField_) {
		return false;
	}

	const float halfWidth = kWidth / 2.0f;
	const float halfHeight = kHeight / 2.0f;

	// 現在位置に重なる可能性があるマスだけ調べる
	for (uint32_t row = 0; row < MapChipField::kNumBlockVertical; ++row) {

		for (uint32_t column = 0; column < MapChipField::kNumBlockHorizontal; ++column) {

			if (mapChipField_->GetMapChipTypeByIndex(column, row) != MapChipType::kBlock) {
				continue;
			}

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(column, row);

			// 接触だけなら重なりとはみなさない
			bool overlapX = x + halfWidth > rect.left + 0.0001f && x - halfWidth < rect.right - 0.0001f;

			bool overlapY = y + halfHeight > rect.bottom + 0.0001f && y - halfHeight < rect.top - 0.0001f;

			if (overlapX && overlapY) {
				return true;
			}
		}
	}

	return false;
}

// ========================================
// 壁に衝突した時
// ========================================

void Enemy::OnWallHit() {

	bool wasMovingRight = velocity_.x > 0.0f;

	// 反転方向を記録
	wallMoveRight_ = !wasMovingRight;
	wallTurnTimer_ = kWallTurnDuration;

	// E0
	if (enemyType_ == EnemyType::kSword) {

		UpdateSwordDirection(wallMoveRight_ ? 1.0f : -1.0f);
	}

	// E1
	if (enemyType_ == EnemyType::kFire) {

		UpdateFireDirection(wallMoveRight_ ? 1.0f : -1.0f);

		// 突進中に壁に当たったら突進終了
		if (fireDashPhase_ == FireDashPhase::kDash) {

			fireDashPhase_ = FireDashPhase::kRecovery;
			fireDashTimer_ = 0;
		}
	}

	velocity_.x = 0.0f;
}

// ========================================
// 重力・床・壁の当たり判定
// ========================================

void Enemy::ApplyMapPhysics() {

	// 吸い込み・保持・吐き出し中は通常物理を使わない
	if (isBeingInhaled_ || isHeld_ || isSpitting_) {
		return;
	}

	// マップがない場合
	if (!mapChipField_) {

		worldTransform_.translation_.x += velocity_.x;

		fallSpeed_ -= kGravityAcceleration;

		fallSpeed_ = (std::max)(fallSpeed_, -kLimitFallSpeed);

		worldTransform_.translation_.y += fallSpeed_;

		return;
	}

	float& x = worldTransform_.translation_.x;
	float& y = worldTransform_.translation_.y;

	// ========================================
	// 横方向の移動・壁判定
	// ========================================

	float moveX = velocity_.x;

	int horizontalSteps = (std::max)(1, static_cast<int>(std::ceil(std::abs(moveX) / kCollisionStep)));

	float horizontalStep = moveX / static_cast<float>(horizontalSteps);

	for (int i = 0; i < horizontalSteps; ++i) {

		float nextX = x + horizontalStep;

		if (IsBlockCollision(nextX, y)) {

			// 壁をすり抜けない位置まで近づける
			float sign = horizontalStep > 0.0f ? 1.0f : -1.0f;

			for (int j = 0; j < 20; ++j) {

				float smallStep = sign * 0.005f;

				if (IsBlockCollision(x + smallStep, y)) {
					break;
				}

				x += smallStep;
			}

			OnWallHit();
			break;
		}

		x = nextX;
	}

	// ========================================
	// 重力
	// ========================================

	fallSpeed_ -= kGravityAcceleration;

	fallSpeed_ = (std::max)(fallSpeed_, -kLimitFallSpeed);

	// ========================================
	// 縦方向の移動・床・天井判定
	// ========================================

	float moveY = fallSpeed_;

	int verticalSteps = (std::max)(1, static_cast<int>(std::ceil(std::abs(moveY) / kCollisionStep)));

	float verticalStep = moveY / static_cast<float>(verticalSteps);

	onGround_ = false;

	for (int i = 0; i < verticalSteps; ++i) {

		float nextY = y + verticalStep;

		if (IsBlockCollision(x, nextY)) {

			// 床・天井の直前まで移動
			float sign = verticalStep > 0.0f ? 1.0f : -1.0f;

			for (int j = 0; j < 20; ++j) {

				float smallStep = sign * 0.005f;

				if (IsBlockCollision(x, y + smallStep)) {
					break;
				}

				y += smallStep;
			}

			if (moveY < 0.0f) {
				onGround_ = true;
			}

			fallSpeed_ = 0.0f;
			break;
		}

		y = nextY;
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

		worldTransform_.translation_ = targetPosition;

		isBeingInhaled_ = false;
		isHeld_ = true;

		velocity_ = {};
		fallSpeed_ = 0.0f;

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

void Enemy::UpdateHeldPosition(const Player* player) { worldTransform_.translation_ = player->GetInhalePosition(); }

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

	fallSpeed_ = 0.0f;

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

	// 保持中は描画しない
	if (isHeld_) {
		return;
	}

	if (!camera_) {
		return;
	}

	KamataEngine::Model* drawModel = model_;

	// E0 ソード敵
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

	// E1 ファイヤー敵
	else if (enemyType_ == EnemyType::kFire) {

		int modelIndex = 0;

		switch (fireDashPhase_) {

		case FireDashPhase::kMove:
			modelIndex = 0;
			break;

		case FireDashPhase::kCharge:
			modelIndex = 1;
			break;

		case FireDashPhase::kDash:
			modelIndex = 2;
			break;

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

	KamataEngine::Vector3 position{};

	position.x = worldTransform_.matWorld_.m[3][0];
	position.y = worldTransform_.matWorld_.m[3][1];
	position.z = worldTransform_.matWorld_.m[3][2];

	return position;
}

// ========================================
// 当たり判定
// ========================================

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 position = GetWorldPosition();

	AABB aabb{};

	aabb.min = {position.x - kWidth / 2.0f, position.y - kHeight / 2.0f, position.z - kWidth / 2.0f};

	aabb.max = {position.x + kWidth / 2.0f, position.y + kHeight / 2.0f, position.z + kWidth / 2.0f};

	// E0の剣攻撃中は前方にも判定を伸ばす
	if (enemyType_ == EnemyType::kSword && swordAttackPhase_ == SwordAttackPhase::kSwing && !isBeingInhaled_ && !isHeld_ && !isSpitting_) {

		float attackMinX;
		float attackMaxX;

		if (swordFacingRight_) {

			attackMinX = position.x + kWidth / 2.0f;

			attackMaxX = attackMinX + kSwordAttackRange;

		} else {

			attackMaxX = position.x - kWidth / 2.0f;

			attackMinX = attackMaxX - kSwordAttackRange;
		}

		float attackMinY = position.y - kSwordAttackHeight / 2.0f;

		float attackMaxY = position.y + kSwordAttackHeight / 2.0f;

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

	if (enemyType_ == EnemyType::kFire && fireDashPhase_ == FireDashPhase::kDash) {

		velocity_ = {};

		fireDashPhase_ = FireDashPhase::kRecovery;
		fireDashTimer_ = 0;
	}
}
