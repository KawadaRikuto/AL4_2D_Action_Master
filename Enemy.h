#pragma once

#include "AABB.h"
#include "KamataEngine.h"

// 前方宣言
class Player;

/// <summary>
/// 敵の種類
/// </summary>
enum class EnemyType {

	// 通常の敵
	kNormal,

	// 今後追加する能力用
	kFire,

	// 今後追加する能力用
	kIce,
};

/// <summary>
/// 敵
/// </summary>
class Enemy {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, EnemyType type = EnemyType::kNormal);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const Player* player);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

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
	void OnCollision(const Player* player);

	/// <summary>
	/// 吸い込まれているか取得
	/// </summary>
	bool IsBeingInhaled() const { return isBeingInhaled_; }

	/// <summary>
	/// 吸い込みが完了したか取得
	/// </summary>
	bool IsInhaleFinished() const { return isInhaleFinished_; }

	/// <summary>
	/// 敵の種類を取得
	/// </summary>
	EnemyType GetEnemyType() const { return enemyType_; }

private:
	// 吸い込み処理
	void UpdateInhale(const Player* player);

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 敵の種類
	EnemyType enemyType_ = EnemyType::kNormal;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.05f;

	// 吸い込まれる速度
	static inline const float kInhaleSpeed = 0.12f;

	// 吸い込み範囲
	static inline const float kInhaleRange = 4.0f;

	// 吸い込み時の上下判定範囲
	static inline const float kInhaleHeightRange = 1.2f;

	// 吸い込み終了位置までの距離
	static inline const float kInhaleStopDistance = 0.25f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = -10.0f;

	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 10.0f;

	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 1.0f;

	// 経過時間
	float walkTimer_ = 0.0f;

	// 敵の当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 吸い込まれているか
	bool isBeingInhaled_ = false;

	// 吸い込み完了
	bool isInhaleFinished_ = false;
};