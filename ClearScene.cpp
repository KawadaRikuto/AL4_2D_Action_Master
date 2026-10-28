
#include "ClearScene.h"

#include "WorldTransformUpdate.h"

// ========================================
// デストラクタ
// ========================================

ClearScene::~ClearScene() {

	delete modelClear_;
	modelClear_ = nullptr;
}

// ========================================
// 初期化
// ========================================


void ClearScene::Initialize() {

	// Clear.objを読み込み
	modelClear_ = KamataEngine::Model::CreateFromOBJ("Clear", true);

	// カメラ初期化
	camera_.Initialize();

	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.rotation_ = {0.0f, 0.0f, 0.0f};

	camera_.UpdateMatrix();

	// モデル初期化
	worldTransform_.Initialize();

	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	// 文字を大きめに表示
	worldTransform_.scale_ = {0.25f, 0.25f, 0.25f};

	// 正面を向くように回転
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);
}

// ========================================
// 更新
// ========================================

void ClearScene::Update() { UpdateWorldTransform(worldTransform_); }

// ========================================
// 描画
// ========================================

void ClearScene::Draw() {

	if (modelClear_) {
		modelClear_->Draw(worldTransform_, camera_);
	}
}
