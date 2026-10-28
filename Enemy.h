
#pragma once

#include "AABB.h"
#include "KamataEngine.h"

class Player;
class MapChipField;

enum class EnemyType {
	kNormal,
	kFire,
	kIce,
	kSword,
};

class Enemy {
public:
	~Enemy();

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, EnemyType type = EnemyType::kNormal);

	void Update(const Player* player);
	void Draw();

	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

	void OnCollision(const Player* player);

	bool IsBeingInhaled() const { return isBeingInhaled_; }
	bool IsInhaleFinished() const { return isHeld_; }
	bool IsHeld() const { return isHeld_; }
	bool IsSpitting() const { return isSpitting_; }
	bool IsFinished() const { return isFinished_; }

	EnemyType GetEnemyType() const { return enemyType_; }

	void StartSpit(bool isFacingRight);

	bool IsFireDashing() const { return enemyType_ == EnemyType::kFire && fireDashPhase_ == FireDashPhase::kDash && !isBeingInhaled_ && !isHeld_ && !isSpitting_; }

private:
	void UpdateInhale(const Player* player);
	void UpdateSpit();
	void UpdateHeldPosition(const Player* player);

	enum class SwordAttackPhase {
		kMove,
		kWindup,
		kSwing,
		kFinish,
		kCooldown,
	};

	void UpdateSwordEnemy(const Player* player);
	void UpdateSwordAttack();
	void UpdateSwordDirection(float directionX);

	enum class FireDashPhase {
		kMove,
		kCharge,
		kDash,
		kRecovery,
	};

	void UpdateFireEnemy(const Player* player);
	void UpdateFireDirection(float directionX);

	// 重力・床・壁との衝突
	void ApplyMapPhysics();

	// 指定位置がブロックと重なっているか
	bool IsBlockCollision(float x, float y) const;

	// 壁に当たった時の処理
	void OnWallHit();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	EnemyType enemyType_ = EnemyType::kNormal;

	static inline const float kWalkSpeed = 0.05f;
	static inline const float kInhaleSpeed = 0.12f;
	static inline const float kInhaleRange = 4.0f;
	static inline const float kInhaleHeightRange = 1.2f;
	static inline const float kInhaleStopDistance = 0.25f;

	static inline const float kSpitSpeed = 0.45f;
	static inline const uint32_t kSpitDuration = 30;

	KamataEngine::Vector3 velocity_ = {};

	static inline const float kWalkMotionAngleStart = -10.0f;
	static inline const float kWalkMotionAngleEnd = 10.0f;
	static inline const float kWalkMotionTime = 1.0f;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	float walkTimer_ = 0.0f;

	bool isBeingInhaled_ = false;
	bool isHeld_ = false;
	bool isSpitting_ = false;
	bool isFinished_ = false;

	uint32_t spitParameter_ = 0;

	KamataEngine::ObjectColor objectColor_;
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	static constexpr int kSwordModelCount = 4;
	KamataEngine::Model* swordModels_[kSwordModelCount] = {};

	SwordAttackPhase swordAttackPhase_ = SwordAttackPhase::kMove;

	uint32_t swordAttackTimer_ = 0;

	static inline const float kSwordStopDistance = 1.2f;
	static inline const float kSwordAttackStartRange = 1.35f;
	static inline const float kSwordAttackHeightRange = 0.8f;
	static inline const float kSwordAttackRange = 0.5f;
	static inline const float kSwordAttackHeight = 0.9f;

	static inline const uint32_t kSwordWindupDuration = 12;
	static inline const uint32_t kSwordSwingDuration = 8;
	static inline const uint32_t kSwordFinishDuration = 12;
	static inline const uint32_t kSwordCooldownDuration = 25;

	bool swordFacingRight_ = false;

	static constexpr int kFireModelCount = 3;
	KamataEngine::Model* fireModels_[kFireModelCount] = {};

	FireDashPhase fireDashPhase_ = FireDashPhase::kMove;

	uint32_t fireDashTimer_ = 0;

	bool fireFacingRight_ = false;

	static inline const float kFireStopDistance = 2.0f;
	static inline const float kFireAttackStartRange = 2.1f;
	static inline const float kFireAttackHeightRange = 0.8f;
	static inline const float kFireDashSpeed = 0.7f;

	static inline const uint32_t kFireChargeDuration = 10;
	static inline const uint32_t kFireDashDuration = 10;
	static inline const uint32_t kFireRecoveryDuration = 30;

	// ========================================
	// 追加：物理処理
	// ========================================

	static inline const float kGravityAcceleration = 0.05f;
	static inline const float kLimitFallSpeed = 0.5f;

	float fallSpeed_ = 0.0f;
	bool onGround_ = false;

	// 1回の移動を細分化して壁のすり抜けを防ぐ
	static inline const float kCollisionStep = 0.1f;

	// 壁に当たった後、すぐ追跡方向に戻らないための時間
	static inline const int kWallTurnDuration = 40;

	int wallTurnTimer_ = 0;
	bool wallMoveRight_ = false;
};
