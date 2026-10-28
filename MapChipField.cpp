
#include "MapChipField.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// CSVのセル前後の空白・改行を除去
std::string Trim(const std::string& text) {

	size_t first = 0;
	size_t last = text.size();

	while (first < last && std::isspace(static_cast<unsigned char>(text[first]))) {
		++first;
	}

	while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
		--last;
	}

	return text.substr(first, last - first);
}

} // namespace

// ========================================
// マップデータ初期化
// ========================================

void MapChipField::ResetMapChipData() {

	mapChipData_.data.clear();
	enemySpawnData_.clear();
	goalSpawnData_.clear();

	mapChipData_.data.resize(kNumBlockVertical, std::vector<MapChipType>(kNumBlockHorizontal, MapChipType::kBlank));
}

// ========================================
// CSV読み込み
// ========================================

void MapChipField::LoadMapChipCsv(const std::string& filePath) {

	ResetMapChipData();

	std::ifstream file(filePath);
	assert(file.is_open());

	if (!file.is_open()) {
		return;
	}

	for (uint32_t y = 0; y < kNumBlockVertical; ++y) {

		std::string line;

		if (!std::getline(file, line)) {
			break;
		}

		std::istringstream lineStream(line);

		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {

			std::string word;

			if (!std::getline(lineStream, word, ',')) {
				break;
			}

			word = Trim(word);

			if (word == "B0") {

				mapChipData_.data[y][x] = MapChipType::kBlock;

			} else if (word == "E0") {

				EnemySpawnData enemyData{};

				enemyData.index = {x, y};
				enemyData.type = 0;

				enemySpawnData_.push_back(enemyData);

			} else if (word == "E1") {

				EnemySpawnData enemyData{};

				enemyData.index = {x, y};
				enemyData.type = 1;

				enemySpawnData_.push_back(enemyData);

			} else if (word == "G0") {

				mapChipData_.data[y][x] = MapChipType::kGoal;

				goalSpawnData_.push_back({x, y});
			}
		}
	}
}

// ========================================
// 指定マスの種類
// ========================================

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVertical) {

		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex];
}

// ========================================
// マップ番号から座標取得
// ========================================

KamataEngine::Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {

	KamataEngine::Vector3 position{};

	position.x = kBlockWidth * xIndex;

	position.y = kBlockHeight * (kNumBlockVertical - 1 - yIndex);

	position.z = 0.0f;

	return position;
}

// ========================================
// 座標からマップ番号取得
// ========================================

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) {

	IndexSet indexSet{};

	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);

	uint32_t reversedYIndex = static_cast<uint32_t>((position.y + kBlockHeight / 2.0f) / kBlockHeight);

	indexSet.yIndex = kNumBlockVertical - 1 - reversedYIndex;

	return indexSet;
}

// ========================================
// マップチップの範囲
// ========================================

MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {

	KamataEngine::Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect{};

	rect.left = center.x - kBlockWidth / 2.0f;

	rect.right = center.x + kBlockWidth / 2.0f;

	rect.bottom = center.y - kBlockHeight / 2.0f;

	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}
