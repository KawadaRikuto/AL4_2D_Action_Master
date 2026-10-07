#include "TitleScene.h"

#include "WorldTransformUpdate.h"

#include <cmath>

void TitleScene::Initialize() {

	// タイトル文字の3Dモデルを読み込む
	modelTitle_ = KamataEngine::Model::CreateFromOBJ("titleFont", true);

	// ワールドトランスフォームの初期化
	worldTransformTitle_.Initialize();

	// タイトル文字の初期座標
	worldTransformTitle_.translation_ = {
	    0.0f,
	    2.0f,
	    0.0f,
	};

	// タイトル文字の大きさ
	worldTransformTitle_.scale_ = {
	    1.0f,
	    1.0f,
	    1.0f,
	};

	// ワールド行列を更新
	UpdateWorldTransform(worldTransformTitle_);

	// カメラの初期化
	camera_.Initialize();

	// カメラの位置
	camera_.translation_ = {
	    0.0f,
	    0.0f,
	    -30.0f,
	};

	camera_.UpdateMatrix();

	// アニメーション用の時間を初期化
	animationTimer_ = 0.0f;

	// 終了フラグを初期化
	finished_ = false;

	// フェードの生成
	fade_ = new Fade();

	// フェードの初期化
	fade_->Initialize();

	// フェードイン開始
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);

	// フェードインフェーズから開始
	phase_ = Phase::kFadeIn;
}

void TitleScene::Update() {

	switch (phase_) {

	case Phase::kFadeIn:

		// フェードの更新
		fade_->Update();

		// フェードインが終了
		if (fade_->IsFinished()) {

			// フェードを停止
			fade_->Stop();

			// メインフェーズへ移行
			phase_ = Phase::kMain;
		}

		break;

	case Phase::kMain:

		// 1フレーム分、時間を進める
		animationTimer_ += 1.0f / 60.0f;

		// タイトル文字を上下に動かす
		worldTransformTitle_.translation_.y = 2.0f + std::sin(animationTimer_ * 2.0f) * 0.3f;

		// タイトル文字を少し左右に傾ける
		worldTransformTitle_.rotation_.z = std::sin(animationTimer_) * 0.03f;

		// ワールド行列を更新
		UpdateWorldTransform(worldTransformTitle_);

		// スペースキーを押した
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_SPACE)) {

			// フェードアウト開始
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);

			// フェードアウトフェーズへ移行
			phase_ = Phase::kFadeOut;
		}

		break;

	case Phase::kFadeOut:

		// フェードの更新
		fade_->Update();

		// フェードアウトが終了
		if (fade_->IsFinished()) {

			// タイトルシーンを終了
			finished_ = true;
		}

		break;
	}
}

void TitleScene::Draw() {

	// タイトル文字を描画
	modelTitle_->Draw(worldTransformTitle_, camera_);

	// 最前面にフェードを描画
	fade_->Draw();
}

TitleScene::~TitleScene() {

	// タイトル文字の3Dモデルを解放
	delete modelTitle_;

	// フェードを解放
	delete fade_;
}