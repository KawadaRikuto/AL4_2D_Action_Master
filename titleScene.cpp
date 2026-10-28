
#include "TitleScene.h"

#include "WorldTransformUpdate.h"

using namespace KamataEngine;

// ========================================
// デストラクタ
// ========================================

TitleScene::~TitleScene() {

	delete skydome_;
	skydome_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete modelTitle_;
	modelTitle_ = nullptr;

	delete modelSub_;
	modelSub_ = nullptr;
}

// ========================================
// 初期化
// ========================================

void TitleScene::Initialize() {

	finished_ = false;

	// ========================================
	// モデル読み込み
	// ========================================

	modelTitle_ = Model::CreateFromOBJ("Title", true);

	modelSub_ = Model::CreateFromOBJ("Sub", true);

	// ゲームシーンと同じ天球モデル
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
	// タイトルモデル初期化
	// ========================================

	worldTransform_.Initialize();

	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	// GitHubの最新設定を維持
	worldTransform_.scale_ = {0.15f, 0.15f, 0.15f};

	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);

	// ========================================
	// サブタイトルモデル初期化
	// ========================================

	subWorldTransform_.Initialize();

	// タイトルより少し下に配置
	subWorldTransform_.translation_ = {0.0f, -1.5f, 0.0f};

	// GitHubの最新設定を維持
	subWorldTransform_.scale_ = {0.05f, 0.05f, 0.05f};

	subWorldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(subWorldTransform_);

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

void TitleScene::Update() {

	// SPACEキーでゲームシーンへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		finished_ = true;
	}

	// タイトル更新
	UpdateWorldTransform(worldTransform_);

	// サブタイトル更新
	UpdateWorldTransform(subWorldTransform_);

	// 天球更新
	if (skydome_) {

		skydome_->Update();
	}
}

// ========================================
// 描画
// ========================================

void TitleScene::Draw() {

	// ========================================
	// 天球描画
	// ========================================

	if (skydome_) {

		skydome_->Draw();
	}

	// ========================================
	// タイトル描画
	// ========================================

	if (modelTitle_) {

		modelTitle_->Draw(worldTransform_, camera_);
	}

	// ========================================
	// サブタイトル描画
	// ========================================

	if (modelSub_) {

		modelSub_->Draw(subWorldTransform_, camera_);
	}
}
