#include "Skydome.h"
#include "WorldTransformUpdate.h"

#include <cassert>

void Skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {

	// モデルが設定されていることを確認
	assert(model);

	// カメラが設定されていることを確認
	assert(camera);

	// モデルを受け取る
	model_ = model;

	// カメラを受け取る
	camera_ = camera;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// 天球の座標を原点に設定
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
}

void Skydome::Update() {

	// 行列を計算して定数バッファに転送
	UpdateWorldTransform(worldTransform_);
}

void Skydome::Draw() {

	// 3Dモデル描画
	model_->Draw(worldTransform_, *camera_);
}