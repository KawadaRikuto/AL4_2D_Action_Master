
#pragma once

#include "AABB.h"
#include "KamataEngine.h"

class Player;

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

	// 口元に保持している敵を吐き出す
	void StartSpit(bool isFacingRight);

private:
	void UpdateInhale(const Player* player);
	void UpdateSpit();
	void UpdateHeldPosition(const Player* player);

	// ========================================
	// E0 ソード敵専用処理
	// ========================================

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

private:
	KamataEngine::WorldTransform worldTransform_;

	// 通常敵のモデル（GameSceneが所有）
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	EnemyType enemyType_ = EnemyType::kNormal;

	// ========================================
	// 通常移動・吸い込み
	// ========================================

	static inline const float kWalkSpeed = 0.05f;
	static inline const float kInhaleSpeed = 0.12f;
	static inline const float kInhaleRange = 4.0f;
	static inline const float kInhaleHeightRange = 1.2f;
	static inline const float kInhaleStopDistance = 0.25f;

	// ========================================
	// 吐き出し
	// ========================================

	static inline const float kSpitSpeed = 0.45f;
	static inline const uint32_t kSpitDuration = 30;

	KamataEngine::Vector3 velocity_ = {};

	// ========================================
	// 通常敵の歩行アニメーション
	// ========================================

	static inline const float kWalkMotionAngleStart = -10.0f;
	static inline const float kWalkMotionAngleEnd = 10.0f;
	static inline const float kWalkMotionTime = 1.0f;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	float walkTimer_ = 0.0f;

	// ========================================
	// 吸い込み・保持・吐き出し状態
	// ========================================

	bool isBeingInhaled_ = false;
	bool isHeld_ = false;
	bool isSpitting_ = false;
	bool isFinished_ = false;

	uint32_t spitParameter_ = 0;

	// ========================================
	// E0 ソード敵のモデル
	// ========================================

	// 0：直立
	// 1：振りかぶり
	// 2：振り下ろし途中
	// 3：振り下ろし終了
	static constexpr int kSwordModelCount = 4;

	KamataEngine::Model* swordModels_[kSwordModelCount] = {};

	// ========================================
	// E0 ソード敵の行動
	// ========================================

	SwordAttackPhase swordAttackPhase_ = SwordAttackPhase::kMove;

	uint32_t swordAttackTimer_ = 0;

	// プレイヤーの前で停止する距離
	static inline const float kSwordStopDistance = 1.2f;

	// 攻撃を開始できる距離
	static inline const float kSwordAttackStartRange = 1.35f;

	// 攻撃できる高さの差
	static inline const float kSwordAttackHeightRange = 0.8f;

	// 剣の攻撃距離（敵の中心から前方）
	static inline const float kSwordAttackRange = 1.6f;

	// 攻撃判定の高さ
	static inline const float kSwordAttackHeight = 0.9f;

	// 振りかぶり時間
	static inline const uint32_t kSwordWindupDuration = 12;

	// 振り下ろし時間
	static inline const uint32_t kSwordSwingDuration = 8;

	// 振り下ろし後の時間
	static inline const uint32_t kSwordFinishDuration = 12;

	// 次の攻撃までの待機時間
	static inline const uint32_t kSwordCooldownDuration = 25;

	// 右向きか
	bool swordFacingRight_ = false;
};
