#pragma once

#include "AABB.h"
#include "KamataEngine.h"

// 前方宣言
class MapChipField;
class Enemy;

/// <summary>
/// 自キャラ
/// </summary>
class Player {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 速度を取得
	/// </summary>
	const KamataEngine::Vector3 GetVelocity() const { return velocity_; }

	/// <summary>
	/// ワールドトランスフォームを取得
	/// </summary>
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	/// <summary>
	/// マップチップフィールドを設定
	/// </summary>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	KamataEngine::Vector3 GetWorldPosition();

	/// <summary>
	/// AABBを取得
	/// </summary>
	AABB GetAABB();

	/// <summary>
	/// 衝突応答
	/// </summary>
	void OnCollision(const Enemy* enemy);


private:
	// 左右
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

	void InputMove();
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

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

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
};