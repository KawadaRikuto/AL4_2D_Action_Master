
#include "TitleScene.h"

#include "WorldTransformUpdate.h"

using namespace KamataEngine;

// ========================================
// デストラクタ
// ========================================

TitleScene::~TitleScene() {

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

	// ========================================
	// カメラ初期化
	// ========================================

	camera_.Initialize();

	camera_.translation_ = {0.0f, 0.0f, -10.0f};

	camera_.rotation_ = {0.0f, 0.0f, 0.0f};

	camera_.UpdateMatrix();

	// ========================================
	// タイトルモデル初期化
	// ========================================

	worldTransform_.Initialize();

	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	worldTransform_.scale_ = {0.15f, 0.15f, 0.15f};

	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(worldTransform_);

	// ========================================
	// サブタイトルモデル初期化
	// ========================================

	subWorldTransform_.Initialize();

	// タイトルより少し下に配置
	subWorldTransform_.translation_ = {0.0f, -1.5f, 0.0f};

	// タイトルより小さく表示
	subWorldTransform_.scale_ = {0.05f, 0.05f, 0.05f};

	subWorldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	UpdateWorldTransform(subWorldTransform_);
}

// ========================================
// 更新
// ========================================

void TitleScene::Update() {

	// SPACEキーでゲームシーンへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		finished_ = true;
	}

	UpdateWorldTransform(worldTransform_);

	UpdateWorldTransform(subWorldTransform_);
}

// ========================================
// 描画
// ========================================

void TitleScene::Draw() {

	// タイトル描画
	if (modelTitle_) {

		modelTitle_->Draw(worldTransform_, camera_);
	}

	// サブタイトル描画
	if (modelSub_) {

		modelSub_->Draw(subWorldTransform_, camera_);
	}
}
