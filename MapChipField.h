#pragma once

#include "KamataEngine.h"

#include <cstdint>
#include <string>
#include <vector>

/// <summary>
/// マップチップの種類
/// </summary>
enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

/// <summary>
/// マップチップデータ
/// </summary>
struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

/// <summary>
/// マップチップフィールド
/// </summary>
class MapChipField {

public:
	// インデックスの組
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// 範囲矩形
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	/// <summary>
	/// マップチップデータをリセットする
	/// </summary>
	void ResetMapChipData();

	/// <summary>
	/// CSVファイルからマップチップデータを読み込む
	/// </summary>
	void LoadMapChipCsv(const std::string& filePath);

	/// <summary>
	/// 指定したマスのマップチップを取得
	/// </summary>
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// マップチップ番号から座標を取得する
	/// </summary>
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// 座標からマップチップ番号を取得する
	/// </summary>
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	/// <summary>
	/// 指定したマップチップの範囲を取得する
	/// </summary>
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// マップチップデータ
	MapChipData mapChipData_;
};