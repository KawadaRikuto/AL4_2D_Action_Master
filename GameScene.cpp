
#include "GameScene.h"

#include <cmath>

namespace {

/// <summary>
/// AABB同士の交差判定
/// </summary>
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x && aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y && aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

} // namespace

KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {

	KamataEngine::Matrix4x4 result{};

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			for (int i = 0; i < 4; ++i) {
				result.m[row][column] += m1.m[row][i] * m2.m[i][column];
			}
		}
	}

	return result;
}

KamataEngine::Matrix4x4 MakeScaleMatrix(const KamataEngine::Vector3& scale) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = std::cos(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeTranslateMatrix(const KamataEngine::Vector3& translate) {

	KamataEngine::Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

KamataEngine::Matrix4x4 MakeAffineMatrix(const KamataEngine::Vector3& scale, const KamataEngine::Vector3& rotation, const KamataEngine::Vector3& translation) {

	KamataEngine::Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	KamataEngine::Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotation.x);
	KamataEngine::Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotation.y);
	KamataEngine::Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotation.z);
	KamataEngine::Matrix4x4 translateMatrix = MakeTranslateMatrix(translation);

	KamataEngine::Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	return Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);
}

void UpdateWorldTransform(KamataEngine::WorldTransform& worldTransform) {

	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	worldTransform.TransferMatrix();
}

// ========================================
// 初期化
// ========================================

void GameScene::Initialize() {

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	model_ = KamataEngine::Model::CreateFromOBJ("player", true);
	modelBlock_ = KamataEngine::Model::CreateFromOBJ("block", true);
	modelSkydome_ = KamataEngine::Model::CreateFromOBJ("skydome", true);
	modelEnemy_ = KamataEngine::Model::CreateFromOBJ("enemy", true);

	modelDeathParticle_ = KamataEngine::Model::CreateFromOBJ("deathParticle", true);

	camera_.farZ = 1000.0f;
	camera_.Initialize();

	debugCamera_ = new KamataEngine::DebugCamera(KamataEngine::WinApp::kWindowWidth, KamataEngine::WinApp::kWindowHeight);

	debugCamera_->SetFarZ(1000.0f);

	// プレイヤー生成
	player_ = new Player();

	KamataEngine::Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(10, 16);

	player_->Initialize(model_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	// CSVから敵を生成
	SpawnEnemies();

	// カメラコントローラ
	cameraController_ = new CameraController();
	cameraController_->Initialize();

	CameraController::Rect cameraArea = {
	    0.0f,
	    100.0f,
	    0.0f,
	    20.0f,
	};

	cameraController_->SetMovableArea(cameraArea);
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	camera_.matView = cameraController_->GetCamera().matView;
	camera_.matProjection = cameraController_->GetCamera().matProjection;
	camera_.TransferMatrix();

	// 天球
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	// ブロック
	GenerateBlocks();
}

// ========================================
// 敵生成
// ========================================

void GameScene::SpawnEnemies() {

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	enemies_.clear();

	if (!mapChipField_) {
		return;
	}

	for (const MapChipField::EnemySpawnData& enemyData : mapChipField_->GetEnemySpawnData()) {

		Enemy* newEnemy = new Enemy();

		KamataEngine::Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(enemyData.index.xIndex, enemyData.index.yIndex);

		EnemyType enemyType = EnemyType::kNormal;

		if (enemyData.type == 1) {
			enemyType = EnemyType::kFire;
		}

		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition, enemyType);

		enemies_.push_back(newEnemy);
	}
}

// ========================================
// ブロック生成
// ========================================

void GameScene::GenerateBlocks() {

	worldTransformBlocks_.resize(MapChipField::kNumBlockVertical);

	for (uint32_t i = 0; i < MapChipField::kNumBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(MapChipField::kNumBlockHorizontal, nullptr);
	}

	for (uint32_t i = 0; i < MapChipField::kNumBlockVertical; ++i) {

		for (uint32_t j = 0; j < MapChipField::kNumBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlank) {
				continue;
			}

			worldTransformBlocks_[i][j] = new KamataEngine::WorldTransform();

			worldTransformBlocks_[i][j]->Initialize();

			KamataEngine::Vector3 blockPosition = mapChipField_->GetMapChipPositionByIndex(j, i);

			worldTransformBlocks_[i][j]->translation_ = blockPosition;
		}
	}
}

// ========================================
// ゲームリセット
// ========================================

void GameScene::ResetGame() {

	isDebugCameraActive_ = false;

	if (deathParticles_) {
		delete deathParticles_;
		deathParticles_ = nullptr;
	}

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	enemies_.clear();

	delete player_;
	player_ = nullptr;

	KamataEngine::Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(10, 16);

	player_ = new Player();

	player_->Initialize(model_, &camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);

	SpawnEnemies();

	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	camera_.matView = cameraController_->GetCamera().matView;
	camera_.matProjection = cameraController_->GetCamera().matProjection;
	camera_.TransferMatrix();
}

// ========================================
// 更新
// ========================================

void GameScene::Update() {

	auto input = KamataEngine::Input::GetInstance();

	// Rキーでリセット
	if (input->TriggerKey(DIK_R)) {
		ResetGame();
		return;
	}

#ifdef _DEBUG
	if (input->TriggerKey(DIK_TAB)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	// プレイヤー更新
	if (player_) {
		player_->Update();
	}

	// 敵更新
	for (Enemy* enemy : enemies_) {
		if (enemy) {
			enemy->Update(player_);
		}
	}

	// ========================================
	// 吸い込み・吐き出し・消滅処理
	// ========================================

	for (auto it = enemies_.begin(); it != enemies_.end();) {

		Enemy* enemy = *it;

		if (!enemy) {
			it = enemies_.erase(it);
			continue;
		}

		// 吐き出しの飛行時間が終わった敵を削除する
		if (enemy->IsFinished()) {
			delete enemy;
			it = enemies_.erase(it);
			continue;
		}

		// 口元に保持している敵だけ操作する
		if (enemy->IsHeld()) {

			// ↓キー：吸収してコピー能力を獲得
			if (input->TriggerKey(DIK_DOWN)) {

				if (player_) {
					player_->SetCopyAbility(enemy->GetEnemyType());
				}

				delete enemy;
				it = enemies_.erase(it);
				continue;
			}

			// Zキーを押し直す：前方へ吐き出す
			if (input->TriggerKey(DIK_Z)) {
				if (player_) {
					enemy->StartSpit(player_->IsFacingRight());
				}
			}
		}

		++it;
	}

	// デスパーティクル更新
	if (deathParticles_) {
		deathParticles_->Update();
	}

	// 天球更新
	if (skydome_) {
		skydome_->Update();
	}

	// ブロックのワールド行列更新
	for (std::vector<KamataEngine::WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {

		for (KamataEngine::WorldTransform* worldTransformBlock : worldTransformBlockLine) {

			if (!worldTransformBlock) {
				continue;
			}

			UpdateWorldTransform(*worldTransformBlock);
		}
	}

	// カメラ更新
	if (isDebugCameraActive_) {

		debugCamera_->Update();

		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();

	} else {

		cameraController_->Update();

		camera_.matView = cameraController_->GetCamera().matView;
		camera_.matProjection = cameraController_->GetCamera().matProjection;
		camera_.TransferMatrix();
	}

	// 当たり判定
	CheckAllCollisions();
}

// ========================================
// 当たり判定
// ========================================

void GameScene::CheckAllCollisions() {

	// ========================================
	// 吐き出した敵と、ほかの敵との衝突判定
	// ========================================

	for (auto spitIt = enemies_.begin(); spitIt != enemies_.end();) {

		Enemy* spitEnemy = *spitIt;

		if (!spitEnemy) {
			spitIt = enemies_.erase(spitIt);
			continue;
		}

		// 吐き出し中でなければ対象外
		if (!spitEnemy->IsSpitting()) {
			++spitIt;
			continue;
		}

		AABB spitAABB = spitEnemy->GetAABB();
		bool hitEnemy = false;

		for (auto targetIt = enemies_.begin(); targetIt != enemies_.end();) {

			Enemy* target = *targetIt;

			if (!target) {
				targetIt = enemies_.erase(targetIt);
				continue;
			}

			// 自分自身とは衝突させない
			if (target == spitEnemy) {
				++targetIt;
				continue;
			}

			// 吸い込み中・保持中・吐き出し中・消滅予定の敵は対象外
			if (target->IsBeingInhaled() || target->IsHeld() || target->IsSpitting() || target->IsFinished()) {

				++targetIt;
				continue;
			}

			AABB targetAABB = target->GetAABB();

			if (IsCollision(spitAABB, targetAABB)) {

				// 当たった相手を削除
				delete target;
				targetIt = enemies_.erase(targetIt);

				hitEnemy = true;
				break;
			}

			++targetIt;
		}

		if (hitEnemy) {

			// 相手に命中した吐き出し中の敵も削除
			delete spitEnemy;
			spitIt = enemies_.erase(spitIt);

		} else {
			++spitIt;
		}
	}

	// ========================================
	// コピー能力と敵の衝突判定
	// ========================================

	if (!player_) {
		return;
	}

	if (player_->IsAbilityAttackActive()) {
		AABB abilityAABB = player_->GetAbilityAABB();

		for (auto it = enemies_.begin(); it != enemies_.end();) {
			Enemy* enemy = *it;

			if (!enemy || enemy->IsBeingInhaled() || enemy->IsHeld() || enemy->IsSpitting() || enemy->IsFinished()) {
				++it;
				continue;
			}

			if (IsCollision(abilityAABB, enemy->GetAABB())) {
				delete enemy;
				it = enemies_.erase(it);

				if (player_->IsProjectileActive()) {
					player_->OnAbilityHit();
					break;
				}
				continue;
			}

			++it;
		}
	}

	// ========================================
	// プレイヤーと敵の衝突判定
	// ========================================

	AABB playerAABB = player_->GetAABB();

	for (Enemy* enemy : enemies_) {

		if (!enemy) {
			continue;
		}

		// 吸い込み中・保持中・吐き出し中・消滅予定の敵は
		// プレイヤーとの衝突判定を行わない
		if (enemy->IsBeingInhaled() || enemy->IsHeld() || enemy->IsSpitting() || enemy->IsFinished()) {

			continue;
		}

		AABB enemyAABB = enemy->GetAABB();

		if (IsCollision(playerAABB, enemyAABB)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}

// ========================================
// エフェクト生成
// ========================================

void GameScene::CreateGuardEffect(const KamataEngine::Vector3& position) {

	// 現在はガードエフェクト用モデルがない
	(void)position;
}

void GameScene::CreateHitEffect(const KamataEngine::Vector3& position) {

	// 現在はヒットエフェクト用モデルがない
	(void)position;
}

// ========================================
// 描画
// ========================================

void GameScene::Draw() {

	// 天球
	if (skydome_) {
		skydome_->Draw();
	}

	// プレイヤー
	if (player_) {
		player_->Draw();
	}

	// 敵
	for (Enemy* enemy : enemies_) {
		if (enemy) {
			enemy->Draw();
		}
	}

	// デスパーティクル
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// ブロック
	for (std::vector<KamataEngine::WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {

		for (KamataEngine::WorldTransform* worldTransformBlock : worldTransformBlockLine) {

			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}
}

// ========================================
// デストラクタ
// ========================================

GameScene::~GameScene() {

	// ブロック解放
	for (std::vector<KamataEngine::WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {

		for (KamataEngine::WorldTransform* worldTransformBlock : worldTransformBlockLine) {

			delete worldTransformBlock;
		}
	}

	worldTransformBlocks_.clear();

	// 敵解放
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	enemies_.clear();

	// その他解放
	delete deathParticles_;
	delete mapChipField_;
	delete skydome_;
	delete modelSkydome_;
	delete cameraController_;
	delete debugCamera_;
	delete modelEnemy_;
	delete modelDeathParticle_;
	delete player_;
	delete model_;
	delete modelBlock_;
}
