#define NOMINMAX

#include "Player.h"

#include "Enemy.h"
#include "MapChipField.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <array>
#include <numbers>

using namespace KamataEngine;

namespace {

float EaseInOut(float start, float end, float t) {

	t = std::clamp(t, 0.0f, 1.0f);

	float easedT = t * t * (3.0f - 2.0f * t);

	return start + (end - start) * easedT;
}

float EaseIn(float start, float end, float t) {

	t = std::clamp(t, 0.0f, 1.0f);

	return start + (end - start) * t * t;
}

float EaseOut(float start, float end, float t) {

	t = std::clamp(t, 0.0f, 1.0f);

	float easedT = 1.0f - (1.0f - t) * (1.0f - t);

	return start + (end - start) * easedT;
}

} // namespace

// ========================================
// 初期化
// ========================================

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	velocity_ = {};

	onGround_ = false;

	isInhaling_ = false;
	isAttack_ = false;
	isKnockback_ = false;

	copyAbility_ = EnemyType::kNormal;

	fireDashPhase_ = FireDashPhase::kNone;
	fireDashParameter_ = 0;

	spaceKeyPrevious_ = false;

	objectColor_.Initialize();

	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	objectColor_.SetColor(color_);
}

// ========================================
// 更新
// ========================================

void Player::Update() {

	// ========================================
	// ファイヤー突進中
	// ========================================

	if (fireDashPhase_ != FireDashPhase::kNone) {

		FireDashUpdate();

		return;
	}

	// ========================================
	// 通常移動
	// ========================================

	InputMove();

	// 吸い込み
	InputInhale();

	// ファイヤー突進入力
	InputFireDash();

	// ========================================
	// マップとの衝突
	// ========================================

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.move = velocity_;

	MapCollision(collisionMapInfo);

	Move(collisionMapInfo);

	CeilingCollision(collisionMapInfo);

	WallCollision(collisionMapInfo);

	SwitchGroundState(collisionMapInfo);

	// ========================================
	// 左右方向の回転
	// ========================================

	if (turnTimer_ > 0.0f) {

		turnTimer_ -= 1.0f / 60.0f;

		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		float turnProgress = 1.0f - turnTimer_ / kTimeTurn;

		turnProgress = std::clamp(turnProgress, 0.0f, 1.0f);

		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, turnProgress);
	}

	UpdateWorldTransform(worldTransform_);
}

// ========================================
// 通常移動入力
// ========================================

void Player::InputMove() {

	// ========================================
	// ノックバック
	// ========================================

	if (isKnockback_) {

		isKnockback_ = false;

		velocity_.x = IsFacingRight() ? -kKnockbackSpeed : kKnockbackSpeed;

		return;
	}

	// ========================================
	// 地上
	// ========================================

	if (onGround_) {

		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

			velocity_.x += kAcceleration;

			velocity_.x = (std::min)(velocity_.x, kLimitRunSpeed);

			if (lrDirection_ != LRDirection::kRight) {

				lrDirection_ = LRDirection::kRight;

				turnFirstRotationY_ = worldTransform_.rotation_.y;

				turnTimer_ = kTimeTurn;
			}

		} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

			velocity_.x -= kAcceleration;

			velocity_.x = (std::max)(velocity_.x, -kLimitRunSpeed);

			if (lrDirection_ != LRDirection::kLeft) {

				lrDirection_ = LRDirection::kLeft;

				turnFirstRotationY_ = worldTransform_.rotation_.y;

				turnTimer_ = kTimeTurn;
			}

		} else {

			velocity_.x *= kAttenuation;
		}

		// ========================================
		// ジャンプ
		// ========================================

		if (Input::GetInstance()->PushKey(DIK_UP)) {

			velocity_.y += kJumpAcceleration;

			onGround_ = false;
		}
	}

	// ========================================
	// 空中
	// ========================================

	else {

		velocity_.y -= kGravityAcceleration;

		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}
}

// ========================================
// 吸い込み入力
// ========================================

void Player::InputInhale() { isInhaling_ = Input::GetInstance()->PushKey(DIK_Z); }

// ========================================
// ファイヤー突進入力
// ========================================

void Player::InputFireDash() {

	bool spaceKeyNow = Input::GetInstance()->PushKey(DIK_SPACE);

	// ファイヤー以外
	if (copyAbility_ != EnemyType::kFire) {

		spaceKeyPrevious_ = spaceKeyNow;

		return;
	}

	// 押した瞬間だけ
	if (spaceKeyNow && !spaceKeyPrevious_) {

		FireDashInitialize();
	}

	spaceKeyPrevious_ = spaceKeyNow;
}

// ========================================
// ファイヤー突進開始
// ========================================

void Player::FireDashInitialize() {

	fireDashPhase_ = FireDashPhase::kCharge;

	fireDashParameter_ = 0;

	isAttack_ = true;

	velocity_ = {};
}

// ========================================
// ファイヤー突進更新
// ========================================

void Player::FireDashUpdate() {

	isAttack_ = true;

	// ========================================
	// 溜め
	// ========================================

	if (fireDashPhase_ == FireDashPhase::kCharge) {

		float t = static_cast<float>(fireDashParameter_) / static_cast<float>(kFireChargeDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		worldTransform_.scale_.y = EaseOut(1.0f, 1.2f, t);

		worldTransform_.scale_.z = EaseOut(1.0f, 0.7f, t);

		velocity_ = {};

		if (fireDashParameter_ >= kFireChargeDuration) {

			fireDashPhase_ = FireDashPhase::kDash;

			fireDashParameter_ = 0;
		}

		fireDashParameter_++;

		UpdateWorldTransform(worldTransform_);

		return;
	}

	// ========================================
	// 突進
	// ========================================

	if (fireDashPhase_ == FireDashPhase::kDash) {

		float t = static_cast<float>(fireDashParameter_) / static_cast<float>(kFireDashDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		worldTransform_.scale_.z = EaseOut(0.7f, 1.4f, t);

		worldTransform_.scale_.y = EaseIn(1.2f, 0.7f, t);

		CollisionMapInfo collisionMapInfo;

		collisionMapInfo.move = {};

		if (lrDirection_ == LRDirection::kRight) {

			collisionMapInfo.move.x = kFireDashSpeed;

		} else {

			collisionMapInfo.move.x = -kFireDashSpeed;
		}

		// ========================================
		// 突進時も通常と同じ衝突処理
		// ========================================

		MapCollision(collisionMapInfo);

		Move(collisionMapInfo);

		// ========================================
		// 壁にぶつかった
		// ========================================

		if (collisionMapInfo.hitWall) {

			velocity_ = {};

			fireDashPhase_ = FireDashPhase::kRecovery;

			fireDashParameter_ = 0;

			UpdateWorldTransform(worldTransform_);

			return;
		}

		velocity_ = {};

		fireDashParameter_++;

		if (fireDashParameter_ >= kFireDashDuration) {

			fireDashPhase_ = FireDashPhase::kRecovery;

			fireDashParameter_ = 0;
		}

		UpdateWorldTransform(worldTransform_);

		return;
	}

	// ========================================
	// 戻り
	// ========================================

	if (fireDashPhase_ == FireDashPhase::kRecovery) {

		float t = static_cast<float>(fireDashParameter_) / static_cast<float>(kFireRecoveryDuration);

		t = std::clamp(t, 0.0f, 1.0f);

		worldTransform_.scale_.z = EaseOut(1.4f, 1.0f, t);

		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

		velocity_ = {};

		if (fireDashParameter_ >= kFireRecoveryDuration) {

			fireDashPhase_ = FireDashPhase::kNone;

			fireDashParameter_ = 0;

			isAttack_ = false;

			worldTransform_.scale_.x = 1.0f;
			worldTransform_.scale_.y = 1.0f;
			worldTransform_.scale_.z = 1.0f;

			velocity_ = {};
		}

		fireDashParameter_++;

		UpdateWorldTransform(worldTransform_);

		return;
	}
}

// ========================================
// マップ衝突
// ========================================

void Player::MapCollision(CollisionMapInfo& info) {

	MapCollisionUp(info);

	MapCollisionDown(info);

	MapCollisionRight(info);

	MapCollisionLeft(info);
}

// ========================================
// 上方向の衝突
// ========================================

void Player::MapCollisionUp(CollisionMapInfo& info) {

	if (info.move.y <= 0.0f) {
		return;
	}

	Vector3 nextPosition = worldTransform_.translation_;

	nextPosition.y += info.move.y;

	Vector3 leftTop = CornerPosition(nextPosition, kLeftTop);

	Vector3 rightTop = CornerPosition(nextPosition, kRightTop);

	MapChipField::IndexSet leftIndex = mapChipField_->GetMapChipIndexSetByPosition(leftTop);

	MapChipField::IndexSet rightIndex = mapChipField_->GetMapChipIndexSetByPosition(rightTop);

	MapChipType leftType = mapChipField_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);

	MapChipType rightType = mapChipField_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);

	if (leftType == MapChipType::kBlock) {

		info.ceiling = true;

		float blockBottom = mapChipField_->GetRectByIndex(leftIndex.xIndex, leftIndex.yIndex).bottom;

		float playerTop = worldTransform_.translation_.y + kHeight / 2.0f;

		info.move.y = blockBottom - playerTop - kBlank;

		return;
	}

	if (rightType == MapChipType::kBlock) {

		info.ceiling = true;

		float blockBottom = mapChipField_->GetRectByIndex(rightIndex.xIndex, rightIndex.yIndex).bottom;

		float playerTop = worldTransform_.translation_.y + kHeight / 2.0f;

		info.move.y = blockBottom - playerTop - kBlank;
	}
}

// ========================================
// 下方向の衝突
// ========================================

void Player::MapCollisionDown(CollisionMapInfo& info) {

	if (info.move.y >= 0.0f) {
		return;
	}

	Vector3 nextPosition = worldTransform_.translation_;

	nextPosition.y += info.move.y;

	Vector3 leftBottom = CornerPosition(nextPosition, kLeftBottom);

	Vector3 rightBottom = CornerPosition(nextPosition, kRightBottom);

	MapChipField::IndexSet leftIndex = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);

	MapChipField::IndexSet rightIndex = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	MapChipType leftType = mapChipField_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);

	MapChipType rightType = mapChipField_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);

	if (leftType == MapChipType::kBlock) {

		info.landing = true;

		float blockTop = mapChipField_->GetRectByIndex(leftIndex.xIndex, leftIndex.yIndex).top;

		float playerBottom = worldTransform_.translation_.y - kHeight / 2.0f;

		info.move.y = blockTop - playerBottom + kBlank;

		return;
	}

	if (rightType == MapChipType::kBlock) {

		info.landing = true;

		float blockTop = mapChipField_->GetRectByIndex(rightIndex.xIndex, rightIndex.yIndex).top;

		float playerBottom = worldTransform_.translation_.y - kHeight / 2.0f;

		info.move.y = blockTop - playerBottom + kBlank;
	}
}

// ========================================
// 右方向の衝突
// ========================================

void Player::MapCollisionRight(CollisionMapInfo& info) {

	if (info.move.x <= 0.0f) {
		return;
	}

	Vector3 nextPosition = worldTransform_.translation_;

	nextPosition.x += info.move.x;

	Vector3 rightTop = CornerPosition(nextPosition, kRightTop);

	Vector3 rightBottom = CornerPosition(nextPosition, kRightBottom);

	MapChipField::IndexSet topIndex = mapChipField_->GetMapChipIndexSetByPosition(rightTop);

	MapChipField::IndexSet bottomIndex = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	MapChipType topType = mapChipField_->GetMapChipTypeByIndex(topIndex.xIndex, topIndex.yIndex);

	MapChipType bottomType = mapChipField_->GetMapChipTypeByIndex(bottomIndex.xIndex, bottomIndex.yIndex);

	if (topType == MapChipType::kBlock) {

		info.hitWall = true;

		float blockLeft = mapChipField_->GetRectByIndex(topIndex.xIndex, topIndex.yIndex).left;

		float playerRight = worldTransform_.translation_.x + kWidth / 2.0f;

		info.move.x = blockLeft - playerRight - kBlank;

		return;
	}

	if (bottomType == MapChipType::kBlock) {

		info.hitWall = true;

		float blockLeft = mapChipField_->GetRectByIndex(bottomIndex.xIndex, bottomIndex.yIndex).left;

		float playerRight = worldTransform_.translation_.x + kWidth / 2.0f;

		info.move.x = blockLeft - playerRight - kBlank;
	}
}

// ========================================
// 左方向の衝突
// ========================================

void Player::MapCollisionLeft(CollisionMapInfo& info) {

	if (info.move.x >= 0.0f) {
		return;
	}

	Vector3 nextPosition = worldTransform_.translation_;

	nextPosition.x += info.move.x;

	Vector3 leftTop = CornerPosition(nextPosition, kLeftTop);

	Vector3 leftBottom = CornerPosition(nextPosition, kLeftBottom);

	MapChipField::IndexSet topIndex = mapChipField_->GetMapChipIndexSetByPosition(leftTop);

	MapChipField::IndexSet bottomIndex = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);

	MapChipType topType = mapChipField_->GetMapChipTypeByIndex(topIndex.xIndex, topIndex.yIndex);

	MapChipType bottomType = mapChipField_->GetMapChipTypeByIndex(bottomIndex.xIndex, bottomIndex.yIndex);

	if (topType == MapChipType::kBlock) {

		info.hitWall = true;

		float blockRight = mapChipField_->GetRectByIndex(topIndex.xIndex, topIndex.yIndex).right;

		float playerLeft = worldTransform_.translation_.x - kWidth / 2.0f;

		info.move.x = blockRight - playerLeft + kBlank;

		return;
	}

	if (bottomType == MapChipType::kBlock) {

		info.hitWall = true;

		float blockRight = mapChipField_->GetRectByIndex(bottomIndex.xIndex, bottomIndex.yIndex).right;

		float playerLeft = worldTransform_.translation_.x - kWidth / 2.0f;

		info.move.x = blockRight - playerLeft + kBlank;
	}
}

// ========================================
// 移動
// ========================================

void Player::Move(const CollisionMapInfo& info) {

	worldTransform_.translation_.x += info.move.x;
	worldTransform_.translation_.y += info.move.y;
	worldTransform_.translation_.z += info.move.z;
}

// ========================================
// 天井衝突
// ========================================

void Player::CeilingCollision(const CollisionMapInfo& info) {

	if (info.ceiling) {

		velocity_.y = 0.0f;
	}
}

// ========================================
// 壁衝突
// ========================================

void Player::WallCollision(const CollisionMapInfo& info) {

	if (info.hitWall) {

		velocity_.x = 0.0f;
	}
}

// ========================================
// 接地状態切り替え
// ========================================

void Player::SwitchGroundState(const CollisionMapInfo& info) {

	if (info.landing) {

		onGround_ = true;

		velocity_.y = 0.0f;

	} else if (velocity_.y < 0.0f) {

		onGround_ = false;
	}
}

// ========================================
// 角の座標
// ========================================

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[kNumCorner] = {

	    {+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
	};

	return {center.x + offsetTable[corner].x, center.y + offsetTable[corner].y, center.z + offsetTable[corner].z};
}

// ========================================
// 描画
// ========================================

void Player::Draw() {

	switch (copyAbility_) {

	case EnemyType::kNormal:

		color_ = {1.0f, 1.0f, 1.0f, 1.0f};

		break;

	case EnemyType::kFire:

		color_ = {1.0f, 0.2f, 0.1f, 1.0f};

		break;

	case EnemyType::kIce:

		color_ = {0.3f, 0.8f, 1.0f, 1.0f};

		break;
	}

	objectColor_.SetColor(color_);

	model_->Draw(worldTransform_, *camera_, &objectColor_);
}

// ========================================
// ワールド座標取得
// ========================================

Vector3 Player::GetWorldPosition() const { return worldTransform_.translation_; }

// ========================================
// AABB取得
// ========================================

AABB Player::GetAABB() {

	AABB aabb;

	aabb.min.x = worldTransform_.translation_.x - kWidth / 2.0f;

	aabb.min.y = worldTransform_.translation_.y - kHeight / 2.0f;

	aabb.min.z = worldTransform_.translation_.z - 0.5f;

	aabb.max.x = worldTransform_.translation_.x + kWidth / 2.0f;

	aabb.max.y = worldTransform_.translation_.y + kHeight / 2.0f;

	aabb.max.z = worldTransform_.translation_.z + 0.5f;

	return aabb;
}

// ========================================
// 敵との衝突
// ========================================

void Player::OnCollision(const Enemy* enemy) {

	if (!enemy) {
		return;
	}

	RequestKnockback();
}

// ========================================
// ノックバック要求
// ========================================

void Player::RequestKnockback() { isKnockback_ = true; }

// ========================================
// 吸い込み位置
// ========================================

Vector3 Player::GetInhalePosition() const {

	float direction = IsFacingRight() ? 1.0f : -1.0f;

	return {
	    worldTransform_.translation_.x + kInhaleOffsetX * direction,

	    worldTransform_.translation_.y + kInhaleOffsetY,

	    worldTransform_.translation_.z};
}