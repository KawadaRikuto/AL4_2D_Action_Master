
#pragma once

#include "AABB.h"
#include "KamataEngine.h"

class Player;

enum class EnemyType {
	kNormal,
	kFire,
	kIce,
};

class Enemy {
public:
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

	EnemyType GetEnemyType() const { return enemyType_; }

	// 口元に保持している敵を吐き出す
	void StartSpit(bool isFacingRight);

private:
	void UpdateInhale(const Player* player);
	void UpdateSpit();
	void UpdateHeldPosition(const Player* player);

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	EnemyType enemyType_ = EnemyType::kNormal;

	static inline const float kWalkSpeed = 0.05f;
	static inline const float kInhaleSpeed = 0.12f;
	static inline const float kInhaleRange = 4.0f;
	static inline const float kInhaleHeightRange = 1.2f;
	static inline const float kInhaleStopDistance = 0.25f;

	static inline const float kSpitSpeed = 0.45f;
	static inline const float kSpitUpSpeed = 0.08f;
	static inline const float kGravity = 0.015f;
	static inline const uint32_t kSpitDuration = 20;

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

	uint32_t spitParameter_ = 0;
};
