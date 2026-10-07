#define NOMINMAX

#include "Player.h"

#include "Enemy.h"
#include "MapChipField.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <numbers>

    namespace {

	/// <summary>
	/// イーズイン・イーズアウト補間
	/// </summary>
	float EaseInOut(float start, float end, float t) {

		t = t * t * (3.0f - 2.0f * t);

		return start + (end - start) * t;
	}

} // namespace

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {

	// NULLポインタチェック
	assert(model);
	assert(camera);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期座標を設定
	worldTransform_.translation_ = position;

	// 初期回転角を指定
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 吸い込み状態を初期化
	isInhaling_ = false;

	// 攻撃状態を初期化
	isAttack_ = false;

	// ノックバック状態を初期化
	isKnockback_ = false;
}

void Player::InputMove() {

	// 接地状態
	if (onGround_) {

		// 左右移動操作
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			KamataEngine::Vector3 acceleration = {};

			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT)) {

				// 左移動中の右入力
				if (velocity_.x < 0.0f) {

					// 速度と逆方向に入力中は減速
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;

				// 右向きではなかったら右向きに変更
				if (lrDirection_ != LRDirection::kRight) {

					lrDirection_ = LRDirection::kRight;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}

			} else if (KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) {

				// 右移動中の左入力
				if (velocity_.x > 0.0f) {

					// 速度と逆方向に入力中は減速
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				// 左向きではなかったら左向きに変更
				if (lrDirection_ != LRDirection::kLeft) {

					lrDirection_ = LRDirection::kLeft;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			}

			// 加速
			velocity_.x += acceleration.x;

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {

			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ジャンプ入力
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_UP)) {

			// ジャンプ初速
			velocity_.y += kJumpAcceleration;
		}

	} else {

		// 落下速度
		velocity_.y -= kGravityAcceleration;

		// 落下速度制限
		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::InputInhale() {

	// Zキーを押している間は吸い込み状態
	isInhaling_ = KamataEngine::Input::GetInstance()->PushKey(DIK_Z);
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {

	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {
         +kWidth / 2.0f,
         -kHeight / 2.0f,
         0.0f, },
	    {
         -kWidth / 2.0f,
         -kHeight / 2.0f,
         0.0f, },
	    {
         +kWidth / 2.0f,
         +kHeight / 2.0f,
         0.0f, },
	    {
         -kWidth / 2.0f,
         +kHeight / 2.0f,
         0.0f, },
	};

	KamataEngine::Vector3 result = {};

	result.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;

	result.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;

	result.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;

	return result;
}

void Player::MapCollision(CollisionMapInfo& info) {

	MapCollisionUp(info);
	MapCollisionDown(info);
	MapCollisionRight(info);
	MapCollisionLeft(info);
}

void Player::MapCollisionUp(CollisionMapInfo& info) {

	// 上昇あり？
	if (info.move.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionsNew;

	KamataEngine::Vector3 centerNew = {};

	centerNew.x = worldTransform_.translation_.x + info.move.x;
	centerNew.y = worldTransform_.translation_.y + info.move.y;
	centerNew.z = worldTransform_.translation_.z + info.move.z;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(centerNew, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// 真上の当たり判定を行う
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;

		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でY方向のセル番号が変化した
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// ブロックの下面より下に収まるように移動量を修正
			info.move.y = (std::max)(0.0f, info.move.y + rect.bottom - positionsNew[kLeftTop].y - kBlank);

			// 天井に当たったことを記録する
			info.ceiling = true;
		}
	}
}

void Player::MapCollisionDown(CollisionMapInfo& info) {

	// 下降あり？
	if (info.move.y >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionsNew;

	KamataEngine::Vector3 centerNew = {};

	centerNew.x = worldTransform_.translation_.x + info.move.x;
	centerNew.y = worldTransform_.translation_.y + info.move.y;
	centerNew.z = worldTransform_.translation_.z + info.move.z;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(centerNew, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// 真下の当たり判定を行う
	bool hit = false;

	// ============================================================
	// 左下点の判定
	// ============================================================

	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// yIndex - 1 は yIndex == 0 のときアンダーフローするので、
	// 0の場合は次のマップチップを調べない
	if (indexSet.yIndex > 0) {

		MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

		if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

			hit = true;
		}

	} else {

		// マップ最下段の場合
		if (mapChipType == MapChipType::kBlock) {
			hit = true;
		}
	}

	// ============================================================
	// 右下点の判定
	// ============================================================

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// yIndex - 1 のアンダーフローを防止
	if (indexSet.yIndex > 0) {

		MapChipType mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

		if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {

			hit = true;
		}

	} else {

		// マップ最下段の場合
		if (mapChipType == MapChipType::kBlock) {
			hit = true;
		}
	}

	// ============================================================
	// ブロックにヒット？
	// ============================================================

	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;

		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));

		// 移動前と移動後でY方向のセル番号が変化した
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// ブロックの上面より上に収まるように移動量を修正
			info.move.y = (std::min)(0.0f, info.move.y + rect.top - positionsNew[kLeftBottom].y + kBlank);

			// 地面に当たったことを記録する
			info.landing = true;
		}
	}
}

void Player::MapCollisionRight(CollisionMapInfo& info) {

	// 右移動あり？
	if (info.move.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionsNew;

	KamataEngine::Vector3 centerNew = {};

	centerNew.x = worldTransform_.translation_.x + info.move.x;
	centerNew.y = worldTransform_.translation_.y + info.move.y;
	centerNew.z = worldTransform_.translation_.z + info.move.z;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(centerNew, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// 右上と右下の当たり判定
	bool hit = false;

	// 右上点の判定
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;

		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));

		// 移動前と移動後でX方向のセル番号が変化した
		if (indexSetNow.xIndex != indexSet.xIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// ブロックの左面より左に収まるように移動量を修正
			info.move.x = (std::min)(0.0f, info.move.x + rect.left - positionsNew[kRightTop].x - kBlank);

			// 壁に当たったことを判定結果に記録する
			info.hitWall = true;
		}
	}
}

void Player::MapCollisionLeft(CollisionMapInfo& info) {

	// 左移動あり？
	if (info.move.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<KamataEngine::Vector3, kNumCorner> positionsNew;

	KamataEngine::Vector3 centerNew = {};

	centerNew.x = worldTransform_.translation_.x + info.move.x;
	centerNew.y = worldTransform_.translation_.y + info.move.y;
	centerNew.z = worldTransform_.translation_.z + info.move.z;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(centerNew, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	// 左上と左下の当たり判定
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);

	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);

		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;

		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でX方向のセル番号が変化した
		if (indexSetNow.xIndex != indexSet.xIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

			// ブロックの右面より右に収まるように移動量を修正
			info.move.x = (std::max)(0.0f, info.move.x + rect.right - positionsNew[kLeftTop].x + kBlank);

			// 壁に当たったことを記録する
			info.hitWall = true;
		}
	}
}

void Player::Move(const CollisionMapInfo& info) {

	// 移動
	worldTransform_.translation_.x += info.move.x;

	worldTransform_.translation_.y += info.move.y;

	worldTransform_.translation_.z += info.move.z;
}

void Player::CeilingCollision(const CollisionMapInfo& info) {

	// 天井に当たった？
	if (info.ceiling) {

		KamataEngine::DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");

		velocity_.y = 0.0f;
	}
}

void Player::WallCollision(const CollisionMapInfo& info) {

	// 壁接触による減速
	if (info.hitWall) {

		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::SwitchGroundState(const CollisionMapInfo& info) {

	// 自キャラが接地状態？
	if (onGround_) {

		// ジャンプ開始
		if (velocity_.y > 0.0f) {

			// 空中状態に移行
			onGround_ = false;

		} else {

			MapChipType mapChipType;

			// 真下の当たり判定を行う
			bool hit = false;

			// 左下点の座標
			KamataEngine::Vector3 leftBottom = CornerPosition(worldTransform_.translation_, kLeftBottom);

			// 微妙に下へずらして判定する
			leftBottom.y -= kBlank;

			// 左下点の判定
			MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);

			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点の座標
			KamataEngine::Vector3 rightBottom = CornerPosition(worldTransform_.translation_, kRightBottom);

			// 微妙に下へずらして判定する
			rightBottom.y -= kBlank;

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {

				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} else {

		// 着地フラグ
		if (info.landing) {

			// 接地状態に切り替える
			onGround_ = true;

			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);

			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::Update() {

	// ①移動入力
	InputMove();

	// ②吸い込み入力
	InputInhale();

	// ③移動量を加味して衝突判定する

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;

	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	// マップ衝突判定
	MapCollision(collisionMapInfo);

	// ④判定結果を反映して移動させる
	Move(collisionMapInfo);

	// ⑤天井に接触している場合の処理
	CeilingCollision(collisionMapInfo);

	// ⑥壁に接触している場合の処理
	WallCollision(collisionMapInfo);

	// ⑦接地状態の切り替え処理
	SwitchGroundState(collisionMapInfo);

	// ⑧旋回制御
	if (turnTimer_ > 0.0f) {

		// 旋回タイマーを1/60秒だけカウントダウンする
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 補間割合
		float turnProgress = 1.0f - turnTimer_ / kTimeTurn;

		turnProgress = std::clamp(turnProgress, 0.0f, 1.0f);

		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = EaseInOut(turnFirstRotationY_, destinationRotationY, turnProgress);
	}

	// ⑨行列計算
	UpdateWorldTransform(worldTransform_);
}

void Player::Draw() {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}

KamataEngine::Vector3 Player::GetWorldPosition() const {

	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPosition;

	// ワールド行列の平行移動成分を取得
	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

AABB Player::GetAABB() {

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

KamataEngine::Vector3 Player::GetInhalePosition() const {

	KamataEngine::Vector3 inhalePosition = worldTransform_.translation_;

	if (lrDirection_ == LRDirection::kRight) {
		inhalePosition.x += kInhaleOffsetX;
	} else {
		inhalePosition.x -= kInhaleOffsetX;
	}

	inhalePosition.y += kInhaleOffsetY;

	return inhalePosition;
}

void Player::RequestKnockback() {

	isKnockback_ = true;

	if (lrDirection_ == LRDirection::kRight) {
		velocity_.x = -kKnockbackSpeed;
	} else {
		velocity_.x = kKnockbackSpeed;
	}

	velocity_.y = 0.25f;
}

void Player::OnCollision(const Enemy* enemy) {

	(void)enemy;

	// ジャンプ開始
	velocity_.y += 0.5f;
}
