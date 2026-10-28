
#include "ClearScene.h"

#include "WorldTransformUpdate.h"

using namespace KamataEngine;

// ========================================
// デストラクタ
// ========================================

ClearScene::~ClearScene() {

	delete skydome_;
	skydome_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete modelClear_;
	modelClear_ = nullptr;
}

// ========================================
// 初期化
// ========================================

void ClearScene::Initialize() {

	// ========================================
	// モデル読み込み
	// ========================================

	// クリアモデル
	modelClear_ = Model::CreateFromOBJ("Clear", true);

	// ゲームシーンと同じ天球
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// ========================================
	// カメラ初期化
	// ========================================

	camera_.farZ = 1000.0f;
	camera_.Initialize();

	camera_.translation_ = {0.0f, 0.0f, -10.0f};

	camera_.rotation_ = {0.0f, 0.0f, 0.0f};

	camera_.UpdateMatrix();

	// ========================================
	// クリアモデル初期化
	// ========================================

	worldTransform_.Initialize();

	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	// GitHubの最新設定を維持
	worldTransform_.scale_ = {0.25f, 0.25f, 0.25f};

	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);

	// ========================================
	// 天球初期化
	// ========================================

	skydome_ = new Skydome();

	skydome_->Initialize(modelSkydome_, &camera_);

	skydome_->Update();
}

// ========================================
// 更新
// ========================================

void ClearScene::Update() {

	// クリアモデル更新
	UpdateWorldTransform(worldTransform_);

	// 天球更新
	if (skydome_) {

		skydome_->Update();
	}
}

// ========================================
// 描画
// ========================================

void ClearScene::Draw() {

	// ========================================
	// 天球描画
	// ========================================

	if (skydome_) {

		skydome_->Draw();
	}

	// ========================================
	// クリアモデル描画
	// ========================================

	if (modelClear_) {

		modelClear_->Draw(worldTransform_, camera_);
	}
}
