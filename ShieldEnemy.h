#pragma once

#include "AABB.h"
#include "KamataEngine.h"

class Player;
class GameScene;

/// <summary>
/// 盾持ちの敵
/// </summary>
class ShieldEnemy {

public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	void Update();
	void Draw();

	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

	// ノックバックを要求するためconstを外す
	void OnCollision(Player* player);

	bool IsDead() const { return isDead_; }

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

private:
	enum class Behavior {
		kUnknown,
		kRoot,
		kDeath,
		kGuard,
	};

	enum class Direction {
		kLeft,
		kRight,
	};

	enum class GuardPhase {
		kReaction,
		kRecovery,
	};

	void BehaviorRootInitialize();
	void BehaviorDeathInitialize();
	void BehaviorGuardInitialize();

	void BehaviorRootUpdate();
	void BehaviorDeathUpdate();
	void BehaviorGuardUpdate();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	static inline const float kWalkSpeed = 0.03f;
	KamataEngine::Vector3 velocity_ = {};

	Direction direction_ = Direction::kLeft;

	static inline const float kWalkMotionAngleStart = -5.0f;
	static inline const float kWalkMotionAngleEnd = 5.0f;
	static inline const float kWalkMotionTime = 1.0f;

	float walkTimer_ = 0.0f;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	float deathTimer_ = 0.0f;
	static inline const float kDeathMotionTime = 1.0f;

	bool isDead_ = false;
	bool isCollisionDisabled_ = false;

	GameScene* gameScene_ = nullptr;

	// ガードフェーズ
	GuardPhase guardPhase_ = GuardPhase::kReaction;

	// ガード経過時間
	uint32_t guardParameter_ = 0;

	// ガード反応時間
	static inline const uint32_t kGuardReactionDuration = 8;

	// ガード復帰時間
	static inline const uint32_t kGuardRecoveryDuration = 12;

	// ガード時の傾き
	static inline const float kGuardRotation = 0.45f;
};
