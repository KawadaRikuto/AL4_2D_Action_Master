
#pragma once

#include "KamataEngine.h"

#include <cstdint>
#include <string>
#include <vector>

// ========================================
// マップチップの種類
// ========================================

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
	kGoal,  // ゴール
};

// ========================================
// マップチップデータ
// ========================================

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

// ========================================
// マップチップフィールド
// ========================================

class MapChipField {
public:
	// インデックスの組
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// 敵の配置データ
	struct EnemySpawnData {
		IndexSet index;
		int type;
	};

	// 範囲矩形
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	// マップサイズ
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	// マップデータを初期化
	void ResetMapChipData();

	// CSVからマップを読み込み
	void LoadMapChipCsv(const std::string& filePath);

	// 指定マスの種類を取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	// マップチップの座標を取得
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	// 座標からマップチップ番号を取得
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	// マップチップの範囲を取得
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	// CSVのE0・E1配置データ
	const std::vector<EnemySpawnData>& GetEnemySpawnData() const { return enemySpawnData_; }

	// CSVのG0配置データ
	const std::vector<IndexSet>& GetGoalSpawnData() const { return goalSpawnData_; }

private:
	// 1マスのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// マップチップデータ
	MapChipData mapChipData_;

	// 敵の配置
	std::vector<EnemySpawnData> enemySpawnData_;

	// ゴールの配置
	std::vector<IndexSet> goalSpawnData_;
};
