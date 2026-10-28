
#pragma once

#include "AABB.h"
#include "Enemy.h"
#include "KamataEngine.h"

class MapChipField;

class Player {
public:
	~Player();

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

	// ソード攻撃中か
	bool IsSwordAttackActive() const { return copyAbility_ == EnemyType::kSword && isSwordAttacking_; }

	// ソード攻撃の当たり判定
	AABB GetSwordAttackAABB() const;

private:
	// 左右方向
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// プレイヤーの四隅
	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,
		kNumCorner,
	};

	// マップ衝突情報
	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		KamataEngine::Vector3 move = {};
	};

	// ファイヤー突進状態
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

	// ソード攻撃
	void InputSwordAttack();
	void UpdateSwordAttack();

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

	// 四隅の座標
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// ファイヤー突進
	void FireDashInitialize();
	void FireDashUpdate();

	// 歩行アニメーション
	void UpdateAnimation();

private:
	KamataEngine::WorldTransform worldTransform_;

	// 元のプレイヤーモデル
	KamataEngine::Model* model_ = nullptr;

	// ========================================
	// 通常歩行モデル
	// ========================================

	// WalkPlayer1：直立
	KamataEngine::Model* idleModel_ = nullptr;

	// WalkPlayer2、WalkPlayer3
	static constexpr int kWalkModelCount = 2;
	KamataEngine::Model* walkModels_[kWalkModelCount] = {};

	// ========================================
	// ソード歩行モデル
	// ========================================

	// SordWalkPlayer1、2、3
	static constexpr int kSwordWalkModelCount = 3;
	KamataEngine::Model* swordWalkModels_[kSwordWalkModelCount] = {};

	// ========================================
	// ソード攻撃モデル
	// ========================================

	// SordPlayerAttack1、2、3
	static constexpr int kSwordAttackModelCount = 3;
	KamataEngine::Model* swordAttackModels_[kSwordAttackModelCount] = {};

	// ========================================
	// ファイヤー突進攻撃モデル
	// ========================================

	// PlayerAttack1：直立
	// PlayerAttack2：突進の構え
	// PlayerAttack3：突進中
	static constexpr int kFireAttackModelCount = 3;
	KamataEngine::Model* fireAttackModels_[kFireAttackModelCount] = {};

	// ========================================
	// 通常・ソード共通歩行アニメーション
	// ========================================

	// 0：直立
	// 1：右足前
	// 2：直立
	// 3：左足前
	int animationIndex_ = 0;
	int animationFrame_ = 0;

	static constexpr int kAnimationInterval = 6;

	// ========================================
	// ソード攻撃アニメーション
	// ========================================

	bool isSwordAttacking_ = false;
	int swordAttackFrame_ = 0;

	static constexpr int kSwordAttackFrameInterval = 6;

	static constexpr int kSwordAttackDuration = kSwordAttackModelCount * kSwordAttackFrameInterval;

	// 剣の攻撃距離
	static constexpr float kSwordAttackRange = 1.5f;

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

	// 移動パラメータ
	static inline const float kAcceleration = 0.01f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kAttenuationLanding = 0.1f;
	static inline const float kAttenuationWall = 0.1f;
	static inline const float kLimitRunSpeed = 0.2f;

	// 向き
	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;

	// 接地状態
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

	// 攻撃状態
	bool isAttack_ = false;

	// ノックバック
	bool isKnockback_ = false;

	static inline const float kKnockbackSpeed = 0.35f;

	// コピー能力
	EnemyType copyAbility_ = EnemyType::kNormal;

	// 色
	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_;

	// ========================================
	// ファイヤー突進
	// ========================================

	FireDashPhase fireDashPhase_ = FireDashPhase::kNone;

	uint32_t fireDashParameter_ = 0;

	static inline const uint32_t kFireChargeDuration = 10;
	static inline const uint32_t kFireDashDuration = 10;
	static inline const uint32_t kFireRecoveryDuration = 10;

	static inline const float kFireDashSpeed = 0.7f;

	bool spaceKeyPrevious_ = false;
};
