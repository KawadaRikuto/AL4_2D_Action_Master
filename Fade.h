#pragma once

#include "KamataEngine.h"

/// <summary>
/// フェード
/// </summary>
class Fade {

public:
	// フェードの状態
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// フェード開始
	/// </summary>
	void Start(Status status, float duration);

	/// <summary>
	/// フェード停止
	/// </summary>
	void Stop();

	/// <summary>
	/// フェード終了判定
	/// </summary>
	bool IsFinished() const;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Fade();

private:
	// フェード用スプライト
	KamataEngine::Sprite* sprite_ = nullptr;

	// 現在のフェード状態
	Status status_ = Status::None;

	// フェードの継続時間
	float duration_ = 0.0f;

	// 経過時間
	float counter_ = 0.0f;
};