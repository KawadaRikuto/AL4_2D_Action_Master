#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

/// <summary>
/// ステージ管理
/// </summary>
class StageManager {

public:
	/// <summary>
	/// 1ステージ分のデータ
	/// </summary>
	struct StageData {
		std::string name;  // ステージ名（フィールドCSVファイル名）
		int32_t timeLimit; // 制限時間［秒］
	};

	/// <summary>
	/// ステージデータファイルの読み込み
	/// </summary>
	void LoadStageData();

	/// <summary>
	/// ステージデータの取得
	/// </summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetStageData(int32_t index) const {

		assert(index < stageDatas_.size());

		return stageDatas_[index];
	}

	/// <summary>
	/// 現在のステージ番号を設定
	/// </summary>
	void SetCurrentStageIndex(int32_t index) {

		assert(index < stageDatas_.size());

		currentStageIndex_ = index;
	}

	/// <summary>
	/// ステージ名指定で現在ステージ番号を設定
	/// </summary>
	/// <param name="name">ステージ名</param>
	void SetCurrentStageIndexByName(const std::string name);

	/// <summary>
	/// 現在のステージ番号を取得
	/// </summary>
	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }

	/// <summary>
	/// 現在ステージのステージデータ取得
	/// </summary>
	const StageData& GetCurrentStageData() const { return GetStageData(currentStageIndex_); }

private:
	// 全ステージ分のデータ
	std::vector<StageData> stageDatas_;

	// 現在のステージ番号
	int32_t currentStageIndex_ = 0;
};
