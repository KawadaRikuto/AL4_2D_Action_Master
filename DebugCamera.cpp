#include "DebugCamera.h"

#include <dinput.h>

namespace KamataEngine {

// カメラ注視点までの距離
const float DebugCamera::sDistance_ = 50.0f;

DebugCamera::DebugCamera(int window_width, int window_height) {

	// 入力の取得
	input_ = Input::GetInstance();

	// カメラの初期化
	camera_.Initialize();

	// アスペクト比
	camera_.aspectRatio = static_cast<float>(window_width) / static_cast<float>(window_height);

	// カメラの初期位置
	camera_.translation_ = {0.0f, 0.0f, -sDistance_};

	// 行列更新
	UpdateMatrix();
}

void DebugCamera::Update() {

	// 左右移動
	if (input_->PushKey(DIK_A)) {
		camera_.translation_.x -= 0.5f;
	}

	if (input_->PushKey(DIK_D)) {
		camera_.translation_.x += 0.5f;
	}

	// 上下移動
	if (input_->PushKey(DIK_W)) {
		camera_.translation_.y += 0.5f;
	}

	if (input_->PushKey(DIK_S)) {
		camera_.translation_.y -= 0.5f;
	}

	// 前後移動
	if (input_->PushKey(DIK_Q)) {
		camera_.translation_.z += 0.5f;
	}

	if (input_->PushKey(DIK_E)) {
		camera_.translation_.z -= 0.5f;
	}

	// 上下回転
	if (input_->PushKey(DIK_UP)) {
		camera_.rotation_.x -= 0.02f;
	}

	if (input_->PushKey(DIK_DOWN)) {
		camera_.rotation_.x += 0.02f;
	}

	// 左右回転
	if (input_->PushKey(DIK_LEFT)) {
		camera_.rotation_.y -= 0.02f;
	}

	if (input_->PushKey(DIK_RIGHT)) {
		camera_.rotation_.y += 0.02f;
	}

	UpdateMatrix();
}

void DebugCamera::UpdateMatrix() { camera_.UpdateMatrix(); }

} // namespace KamataEngine