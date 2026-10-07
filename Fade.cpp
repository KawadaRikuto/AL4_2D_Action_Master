#include "Fade.h"

#include <algorithm>

void Fade::Initialize() {

	// 白1×1テクスチャを読み込む
	uint32_t textureHandle = KamataEngine::TextureManager::Load("white1x1.png");

	// スプライト生成
	sprite_ = KamataEngine::Sprite::Create(
	    textureHandle, {
	                       0.0f,
	                       0.0f,
	                   });

	// 画面全体のサイズにする
	sprite_->SetSize({
	    static_cast<float>(KamataEngine::WinApp::kWindowWidth),
	    static_cast<float>(KamataEngine::WinApp::kWindowHeight),
	});

	// 黒色に設定
	sprite_->SetColor({
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f,
	});
}

void Fade::Start(Status status, float duration) {

	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() { status_ = Status::None; }

void Fade::Update() {

	// フェード状態による分岐
	switch (status_) {

	case Status::None:

		// 何もしない
		break;

	case Status::FadeIn:

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;

		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// 1.0から0.0へ変化させる
		sprite_->SetColor({
		    0.0f,
		    0.0f,
		    0.0f,
		    std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f),
		});

		break;

	case Status::FadeOut:

		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;

		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}

		// 0.0から1.0へ変化させる
		sprite_->SetColor({
		    0.0f,
		    0.0f,
		    0.0f,
		    std::clamp(counter_ / duration_, 0.0f, 1.0f),
		});

		break;
	}
}

void Fade::Draw() {

	// フェード状態ではない場合は描画しない
	if (status_ == Status::None) {
		return;
	}

	// スプライト描画前処理
	KamataEngine::Sprite::PreDraw();

	// スプライト描画
	sprite_->Draw();

	// スプライト描画後処理
	KamataEngine::Sprite::PostDraw();
}

bool Fade::IsFinished() const {

	// フェード状態による分岐
	switch (status_) {

	case Status::FadeIn:
	case Status::FadeOut:

		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}

Fade::~Fade() { delete sprite_; }