#pragma once

#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include "WorldTransformUpdate.h"

#include <3d/DebugCamera.h>

#include <list>
#include <vector>

// ゲームシーン
class GameScene {

public:
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

private:
	/// <summary>
	/// ブロックの生成
	/// </summary>
	void GenerateBlocks();

	/// <summary>
	/// 全ての当たり判定を行う
	/// </summary>
	void CheckAllCollisions();

private:
	// 自キャラ用3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	// ブロック用3Dモデルデータ
	KamataEngine::Model* modelBlock_ = nullptr;

	// 天球用3Dモデルデータ
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 敵用3Dモデルデータ
	KamataEngine::Model* modelEnemy_ = nullptr;

	// デスパーティクル用3Dモデルデータ
	KamataEngine::Model* modelDeathParticle_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// カメラコントローラ
	CameraController* cameraController_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 自キャラ
	Player* player_ = nullptr;

	// 敵
	std::list<Enemy*> enemies_;

	// デスパーティクル
	DeathParticles* deathParticles_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// ブロック用ワールドトランスフォーム
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
};