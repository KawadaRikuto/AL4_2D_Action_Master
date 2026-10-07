#pragma once

#include "AABB.h"
#include "Enemy.h"
#include "KamataEngine.h"

class MapChipField;

class Player {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);
	void Update();
	void Draw();

	const KamataEngine::Vector3 GetVelocity() const { return velocity_; }
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	KamataEngine::Vector3 GetWorldPosition() const;

	AABB GetAABB();

	void OnCollision(const Enemy* enemy);

	bool IsInhaling() const { return isInhaling_; }

	bool IsFacingRight() const { return lrDirection_ == LRDirection::kRight; }

	KamataEngine::Vector3 GetInhalePosition() const;

	bool IsAttack() const { return isAttack_; }

	void RequestKnockback();

	// コピー能力
	void SetCopyAbility(EnemyType ability) { copyAbility_ = ability; }

	EnemyType GetCopyAbility() const { return copyAbility_; }

private:
	// 左右方向
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 角
	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,
		kNumCorner,
	};

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		KamataEngine::Vector3 move = {};
	};

	// ファイヤー突進の状態
	enum class FireDashPhase {
		kNone,
		kCharge,
		kDash,
		kRecovery,
	};

private:
	// 入力
	void InputMove();
	void InputInhale();
	void InputFireDash();

	// マップ衝突
	void MapCollision(CollisionMapInfo& info);
	void MapCollisionUp(CollisionMapInfo& info);
	void MapCollisionDown(CollisionMapInfo& info);
	void MapCollisionRight(CollisionMapInfo& info);
	void MapCollisionLeft(CollisionMapInfo& info);

	// 移動
	void Move(const CollisionMapInfo& info);

	// 衝突後処理
	void CeilingCollision(const CollisionMapInfo& info);
	void WallCollision(const CollisionMapInfo& info);
	void SwitchGroundState(const CollisionMapInfo& info);

	// 角の座標
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// ファイヤー突進
	void FireDashInitialize();
	void FireDashUpdate();

private:
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// マップ
	MapChipField* mapChipField_ = nullptr;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// プレイヤーサイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// めり込み防止
	static inline const float kBlank = 0.01f;

	// 移動
	static inline const float kAcceleration = 0.01f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kAttenuationLanding = 0.1f;
	static inline const float kAttenuationWall = 0.1f;
	static inline const float kLimitRunSpeed = 0.2f;

	// 左右方向
	LRDirection lrDirection_ = LRDirection::kRight;

	// 方向転換
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;

	// 接地
	bool onGround_ = true;

	// 重力
	static inline const float kGravityAcceleration = 0.05f;
	static inline const float kLimitFallSpeed = 0.5f;

	// ジャンプ
	static inline const float kJumpAcceleration = 1.0f;

	// 吸い込み
	bool isInhaling_ = false;

	static inline const float kInhaleOffsetX = 0.7f;
	static inline const float kInhaleOffsetY = 0.0f;

	// 攻撃中か
	bool isAttack_ = false;

	// ノックバック
	bool isKnockback_ = false;

	static inline const float kKnockbackSpeed = 0.35f;

	// コピー能力
	EnemyType copyAbility_ = EnemyType::kNormal;

	// コピー能力による色変更
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_;

	// ========================================
	// ファイヤー突進
	// ========================================

	FireDashPhase fireDashPhase_ = FireDashPhase::kNone;

	// 現在の突進フレーム
	uint32_t fireDashParameter_ = 0;

	// 溜め時間
	static inline const uint32_t kFireChargeDuration = 10;

	// 突進時間
	static inline const uint32_t kFireDashDuration = 10;

	// 戻り時間
	static inline const uint32_t kFireRecoveryDuration = 10;

	// 突進速度
	static inline const float kFireDashSpeed = 0.7f;

	// SPACEキーの前フレーム状態
	bool spaceKeyPrevious_ = false;
};