#include "Enemy.h"

#include "Player.h"
#include "WorldTransformUpdate.h"

#include <cassert>
#include <cmath>
#include <numbers>

namespace {

/// <summary>
/// 線形補間
/// </summary>
float Lerp(float start, float end, float t) { return start + (end - start) * t; }

} // namespace

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, EnemyType type) {

	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// 敵の種類を記録
	enemyType_ = type;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// 初期座標を設定
	worldTransform_.translation_ = position;

	// 左方向を向くように初期回転角を設定
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};

	// 経過時間を初期化
	walkTimer_ = 0.0f;

	// 吸い込み状態を初期化
	isBeingInhaled_ = false;

	// 吸い込み完了状態を初期化
	isInhaleFinished_ = false;
}

void Enemy::Update(const Player* player) {

	// プレイヤーが存在する場合は吸い込み判定
	if (player) {
		UpdateInhale(player);
	}

	// 吸い込まれている間は通常の移動をしない
	if (!isBeingInhaled_) {

		// 移動
		worldTransform_.translation_.x += velocity_.x;
		worldTransform_.translation_.y += velocity_.y;
		worldTransform_.translation_.z += velocity_.z;

		// タイマーを加算
		walkTimer_ += 1.0f / 60.0f;

		// サインカーブによる歩行アニメーション
		float param = std::sin(2.0f * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);

		// -1.0f～1.0fを0.0f～1.0fに変換
		float t = (param + 1.0f) / 2.0f;

		// 最初の角度と最後の角度を補間
		float walkMotionAngle = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);

		// 度をラジアンに変換してX軸回転を設定
		worldTransform_.rotation_.x = walkMotionAngle * std::numbers::pi_v<float> / 180.0f;
	}

	// ワールド行列の更新
	UpdateWorldTransform(worldTransform_);
}

void Enemy::UpdateInhale(const Player* player) {

	// Zキーが押されていなければ吸い込み解除
	if (!player->IsInhaling()) {
		isBeingInhaled_ = false;
		isInhaleFinished_ = false;
		return;
	}

	KamataEngine::Vector3 playerPosition = player->GetWorldPosition();
	KamataEngine::Vector3 enemyPosition = GetWorldPosition();

	// プレイヤーとの高さの差
	float heightDifference = std::abs(enemyPosition.y - playerPosition.y);

	// 高さが離れすぎている場合は対象外
	if (heightDifference > kInhaleHeightRange) {
		isBeingInhaled_ = false;
		isInhaleFinished_ = false;
		return;
	}

	// プレイヤーから敵までのX方向の距離
	float distanceX = enemyPosition.x - playerPosition.x;

	// プレイヤーの向いている方向
	bool facingRight = player->IsFacingRight();

	// 正面にいるか判定
	bool isInFront = false;

	if (facingRight) {

		if (distanceX > 0.0f && distanceX <= kInhaleRange) {
			isInFront = true;
		}

	} else {

		if (distanceX < 0.0f && -distanceX <= kInhaleRange) {
			isInFront = true;
		}
	}

	// 正面にいなければ吸い込み対象外
	if (!isInFront) {
		isBeingInhaled_ = false;
		isInhaleFinished_ = false;
		return;
	}

	// 吸い込み対象にする
	isBeingInhaled_ = true;

	// プレイヤーの口元を取得
	KamataEngine::Vector3 targetPosition = player->GetInhalePosition();

	// 口元までの距離
	float differenceX = targetPosition.x - enemyPosition.x;
	float differenceY = targetPosition.y - enemyPosition.y;

	float distance = std::sqrt(differenceX * differenceX + differenceY * differenceY);

	// 十分近づいたらそこで止める
	if (distance <= kInhaleStopDistance) {

		worldTransform_.translation_.x = targetPosition.x;
		worldTransform_.translation_.y = targetPosition.y;

		velocity_ = {};

		// 吸い込み完了
		isInhaleFinished_ = true;

		return;
	}

	// プレイヤーの口元方向へ移動
	if (distance > 0.0f) {

		worldTransform_.translation_.x += differenceX / distance * kInhaleSpeed;
		worldTransform_.translation_.y += differenceY / distance * kInhaleSpeed;
	}

	// 通常の歩行速度を止める
	velocity_ = {};
}

void Enemy::Draw() {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPosition;

	// ワールド行列の平行移動成分を取得
	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 worldPosition = GetWorldPosition();

	AABB aabb;

	aabb.min = {
	    worldPosition.x - kWidth / 2.0f,
	    worldPosition.y - kHeight / 2.0f,
	    worldPosition.z - kWidth / 2.0f,
	};

	aabb.max = {
	    worldPosition.x + kWidth / 2.0f,
	    worldPosition.y + kHeight / 2.0f,
	    worldPosition.z + kWidth / 2.0f,
	};

	return aabb;
}

void Enemy::OnCollision(const Player* player) { (void)player; }