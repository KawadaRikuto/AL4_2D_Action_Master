
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

	void SetCopyAbility(EnemyType ability) { copyAbility_ = ability; }

	EnemyType GetCopyAbility() const { return copyAbility_; }

	bool IsSwordAttackActive() const { return copyAbility_ == EnemyType::kSword && isSwordAttacking_; }

	AABB GetSwordAttackAABB() const;

private:
	enum class LRDirection {
		kRight,
		kLeft,
	};

	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,
		kNumCorner,
	};

	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		KamataEngine::Vector3 move = {};
	};

	enum class FireDashPhase {
		kNone,
		kCharge,
		kDash,
		kRecovery,
	};

private:
	void InputMove();
	void InputInhale();
	void InputFireDash();

	void InputSwordAttack();
	void UpdateSwordAttack();

	void MapCollision(CollisionMapInfo& info);
	void MapCollisionUp(CollisionMapInfo& info);
	void MapCollisionDown(CollisionMapInfo& info);
	void MapCollisionRight(CollisionMapInfo& info);
	void MapCollisionLeft(CollisionMapInfo& info);

	void Move(const CollisionMapInfo& info);

	void CeilingCollision(const CollisionMapInfo& info);
	void WallCollision(const CollisionMapInfo& info);
	void SwitchGroundState(const CollisionMapInfo& info);

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	void FireDashInitialize();
	void FireDashUpdate();

	void UpdateAnimation();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	// 通常歩行モデル
	KamataEngine::Model* idleModel_ = nullptr;

	static constexpr int kWalkModelCount = 2;
	KamataEngine::Model* walkModels_[kWalkModelCount] = {};

	// ソード歩行モデル
	static constexpr int kSwordWalkModelCount = 3;
	KamataEngine::Model* swordWalkModels_[kSwordWalkModelCount] = {};

	// ソード攻撃モデル
	static constexpr int kSwordAttackModelCount = 3;
	KamataEngine::Model* swordAttackModels_[kSwordAttackModelCount] = {};

	// ファイヤー突進モデル
	static constexpr int kFireAttackModelCount = 3;
	KamataEngine::Model* fireAttackModels_[kFireAttackModelCount] = {};

	int animationIndex_ = 0;
	int animationFrame_ = 0;

	static constexpr int kAnimationInterval = 6;

	bool isSwordAttacking_ = false;
	int swordAttackFrame_ = 0;

	static constexpr int kSwordAttackFrameInterval = 6;

	static constexpr int kSwordAttackDuration = kSwordAttackModelCount * kSwordAttackFrameInterval;

	// 体の前面から0.5マス
	static constexpr float kSwordAttackRange = 0.5f;

	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	KamataEngine::Vector3 velocity_ = {};

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kBlank = 0.01f;

	static inline const float kAcceleration = 0.01f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kAttenuationLanding = 0.1f;
	static inline const float kAttenuationWall = 0.1f;
	static inline const float kLimitRunSpeed = 0.2f;

	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;

	bool onGround_ = true;

	static inline const float kGravityAcceleration = 0.05f;
	static inline const float kLimitFallSpeed = 0.5f;

	static inline const float kJumpAcceleration = 1.0f;

	bool isInhaling_ = false;

	static inline const float kInhaleOffsetX = 0.7f;
	static inline const float kInhaleOffsetY = 0.0f;

	bool isAttack_ = false;
	bool isKnockback_ = false;

	static inline const float kKnockbackSpeed = 0.35f;

	EnemyType copyAbility_ = EnemyType::kNormal;

	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_;

	FireDashPhase fireDashPhase_ = FireDashPhase::kNone;

	uint32_t fireDashParameter_ = 0;

	static inline const uint32_t kFireChargeDuration = 10;
	static inline const uint32_t kFireDashDuration = 10;
	static inline const uint32_t kFireRecoveryDuration = 10;

	static inline const float kFireDashSpeed = 0.7f;

	bool spaceKeyPrevious_ = false;
};
