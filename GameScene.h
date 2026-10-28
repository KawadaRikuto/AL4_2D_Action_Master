
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

// ========================================
// ゲームシーン
// ========================================

class GameScene {
public:
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

	// ガードエフェクト生成
	void CreateGuardEffect(const KamataEngine::Vector3& position);

	// ヒットエフェクト生成
	void CreateHitEffect(const KamataEngine::Vector3& position);

private:
	// ブロック生成
	void GenerateBlocks();

	// ゴール生成
	void GenerateGoals();

	// 全当たり判定
	void CheckAllCollisions();

	// ゲームリセット
	void ResetGame();

	// CSVから敵生成
	void SpawnEnemies();

	// ブロック・ゴールの解放
	void ClearMapObjects();

private:
	// ========================================
	// モデル
	// ========================================

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelDeathParticle_ = nullptr;

	// ゴールモデル
	KamataEngine::Model* modelGoal_ = nullptr;

	// ========================================
	// カメラ
	// ========================================

	KamataEngine::Camera camera_;

	CameraController* cameraController_ = nullptr;

	bool isDebugCameraActive_ = false;

	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// ========================================
	// キャラクター
	// ========================================

	Player* player_ = nullptr;

	std::list<Enemy*> enemies_;

	// ========================================
	// エフェクト・背景
	// ========================================

	DeathParticles* deathParticles_ = nullptr;

	Skydome* skydome_ = nullptr;

	// ========================================
	// マップ
	// ========================================

	MapChipField* mapChipField_ = nullptr;

	// ブロックの配置データ
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// ゴールの配置データ
	std::vector<KamataEngine::WorldTransform*> goalTransforms_;

	// ゴール到達状態
	bool isGoalReached_ = false;
};
