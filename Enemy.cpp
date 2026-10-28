
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

	// E0専用モデルを解放
	for (int i = 0; i < kSwordModelCount; ++i) {

		delete swordModels_[i];
		swordModels_[i] = nullptr;
	}

	// model_はGameScene側で管理しているので削除しない
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

	// ========================================
	// E0 ソード敵の初期化
	// ========================================

	swordAttackPhase_ = SwordAttackPhase::kMove;
	swordAttackTimer_ = 0;

	swordFacingRight_ = false;

	if (enemyType_ == EnemyType::kSword) {

		swordModels_[0] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack1", true);

		swordModels_[1] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack2", true);

		swordModels_[2] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack3", true);

		swordModels_[3] = KamataEngine::Model::CreateFromOBJ("SordEnemyAttack4", true);

		velocity_ = {};
	}

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
	// 吸い込み判定
	// ========================================

	if (player) {
		UpdateInhale(player);
	}

	// ========================================
	// 吸い込み中は移動・攻撃停止
	// ========================================

	if (isBeingInhaled_) {

		velocity_ = {};

		// 吸い込み中に攻撃を続けない
		swordAttackPhase_ = SwordAttackPhase::kMove;
		swordAttackTimer_ = 0;

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
	// E1などの通常移動
	// ========================================

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

// ========================================
// E0 ソード敵の更新
// ========================================

void Enemy::UpdateSwordEnemy(const Player* player) {

	// 攻撃中・待機中は専用の更新処理
	if (swordAttackPhase_ != SwordAttackPhase::kMove) {

		UpdateSwordAttack();
		return;
	}

	// プレイヤーがいない場合は停止
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

	// ========================================
	// プレイヤーの方向を向く
	// ========================================

	UpdateSwordDirection(differenceX);

	// E0は直立モデルで移動するため
	// 歩行時の前後傾きは使用しない
	worldTransform_.rotation_.x = 0.0f;

	// ========================================
	// プレイヤーが攻撃範囲にいる
	// ========================================

	if (distanceX <= kSwordAttackStartRange && distanceY <= kSwordAttackHeightRange) {

		// 移動を止めて振りかぶり開始
		velocity_ = {};

		swordAttackPhase_ = SwordAttackPhase::kWindup;
		swordAttackTimer_ = 0;

		return;
	}

	// ========================================
	// プレイヤーに向かって横移動
	// ========================================

	// 高さが大きく違う場合も
	// 横方向の追跡は継続する

	if (distanceX > kSwordStopDistance) {

		float moveDistance = distanceX - kSwordStopDistance;

		float moveSpeed = (std::min)(kWalkSpeed, moveDistance);

		if (differenceX > 0.0f) {

			velocity_.x = moveSpeed;

		} else {

			velocity_.x = -moveSpeed;
		}

		worldTransform_.translation_.x += velocity_.x;

	} else {

		// プレイヤーの手前で停止
		velocity_ = {};
	}
}

// ========================================
// E0の向き更新
// ========================================

void Enemy::UpdateSwordDirection(float directionX) {

	// ほぼ同じX座標なら向きを変えない
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
// E0 攻撃アニメーション更新
// ========================================

void Enemy::UpdateSwordAttack() {

	// 攻撃中は移動しない
	velocity_ = {};

	swordAttackTimer_++;

	switch (swordAttackPhase_) {

		// ========================================
		// 振りかぶり
		// SordEnemyAttack2
		// ========================================

	case SwordAttackPhase::kWindup:

		if (swordAttackTimer_ >= kSwordWindupDuration) {

			swordAttackPhase_ = SwordAttackPhase::kSwing;
			swordAttackTimer_ = 0;
		}

		break;

		// ========================================
		// 振り下ろし
		// SordEnemyAttack3
		// この間だけ剣の攻撃判定を出す
		// ========================================

	case SwordAttackPhase::kSwing:

		if (swordAttackTimer_ >= kSwordSwingDuration) {

			swordAttackPhase_ = SwordAttackPhase::kFinish;
			swordAttackTimer_ = 0;
		}

		break;

		// ========================================
		// 振り下ろし終了
		// SordEnemyAttack4
		// ========================================

	case SwordAttackPhase::kFinish:

		if (swordAttackTimer_ >= kSwordFinishDuration) {

			swordAttackPhase_ = SwordAttackPhase::kCooldown;
			swordAttackTimer_ = 0;
		}

		break;

		// ========================================
		// 次の攻撃まで待機
		// SordEnemyAttack1
		// ========================================

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
// 吸い込み処理
// ========================================

void Enemy::UpdateInhale(const Player* player) {

	// Zキーが押されている間だけ吸い込み
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

	// ========================================
	// 口元に到達
	// ========================================

	if (distance <= kInhaleStopDistance) {

		worldTransform_.translation_.x = targetPosition.x;
		worldTransform_.translation_.y = targetPosition.y;
		worldTransform_.translation_.z = targetPosition.z;

		isBeingInhaled_ = false;
		isHeld_ = true;

		velocity_ = {};

		swordAttackPhase_ = SwordAttackPhase::kMove;
		swordAttackTimer_ = 0;

		return;
	}

	// ========================================
	// 口元へ移動
	// ========================================

	if (distance > 0.0f) {

		float moveX = differenceX / distance * kInhaleSpeed;

		float moveY = differenceY / distance * kInhaleSpeed;

		// 目標地点を通り越さない
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

	// 攻撃状態をリセット
	swordAttackPhase_ = SwordAttackPhase::kMove;
	swordAttackTimer_ = 0;

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

		// 直立・移動
		case SwordAttackPhase::kMove:
			modelIndex = 0;
			break;

		// 振りかぶり
		case SwordAttackPhase::kWindup:
			modelIndex = 1;
			break;

		// 振り下ろし途中
		case SwordAttackPhase::kSwing:
			modelIndex = 2;
			break;

		// 振り下ろし終了
		case SwordAttackPhase::kFinish:
			modelIndex = 3;
			break;

		// 攻撃後の待機
		case SwordAttackPhase::kCooldown:
			modelIndex = 0;
			break;
		}

		// 吸い込み・保持・吐き出し中は直立
		if (isBeingInhaled_ || isHeld_ || isSpitting_) {
			modelIndex = 0;
		}

		if (swordModels_[modelIndex]) {
			drawModel = swordModels_[modelIndex];
		}
	}

	if (drawModel) {
		drawModel->Draw(worldTransform_, *camera_);
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
// AABB取得
// ========================================

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 worldPosition = GetWorldPosition();

	AABB aabb{};

	// ========================================
	// 敵本体の当たり判定
	// ========================================

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
	// E0 振り下ろし中の攻撃判定
	// ========================================

	if (enemyType_ == EnemyType::kSword && swordAttackPhase_ == SwordAttackPhase::kSwing && !isBeingInhaled_ && !isHeld_ && !isSpitting_) {

		// 剣の攻撃範囲を本体AABBに追加する
		// GameSceneの既存衝突判定をそのまま利用

		float attackMinX;
		float attackMaxX;

		if (swordFacingRight_) {

			attackMinX = worldPosition.x;
			attackMaxX = worldPosition.x + kSwordAttackRange;

		} else {

			attackMinX = worldPosition.x - kSwordAttackRange;

			attackMaxX = worldPosition.x;
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

	// ノックバックはGameSceneから
	// Player::OnCollision()で処理される

	(void)player;
}
