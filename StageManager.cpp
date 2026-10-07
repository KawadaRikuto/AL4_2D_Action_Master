#include "StageManager.h"

#include <cassert>
#include <fstream>
#include <sstream>

void StageManager::LoadStageData() {

	// ステージデータファイルのパス
	const std::string filePath = "Resources/stageDatas.csv";

	// ステージデータファイルを開く
	std::ifstream file(filePath);

	assert(file && "ステージデータファイルが存在しません");

	// ファイルの内容を格納するstringstreamの宣言
	std::stringstream stageDataCsv;

	// ファイルの内容をstringstreamにコピーする
	stageDataCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// ステージデータを最終行まで1行ずつ読み込む
	std::string line;

	while (std::getline(stageDataCsv, line)) {

		// 1行分の内容を格納するstringstreamを宣言して、
		// stringから変換
		std::istringstream lineStream(line);

		// ステージデータを格納する構造体
		StageData stageData;

		// カンマ区切りで次のデータを取得する
		std::string word;

		std::getline(lineStream, word, ',');

		stageData.name = word;

		// カンマ区切りで次のデータを取得する
		std::getline(lineStream, word, ',');

		// 整数に変換して制限時間を格納する
		stageData.timeLimit = std::stoi(word);

		// ステージデータテーブルに格納する
		stageDatas_.push_back(stageData);
	}
}

void StageManager::SetCurrentStageIndexByName(const std::string name) {

	// 全ステージデータを検索
	for (size_t i = 0; i < stageDatas_.size(); ++i) {

		// ステージ名が一致した
		if (stageDatas_[i].name == name) {

			// 現在ステージ番号を設定
			currentStageIndex_ = static_cast<int32_t>(i);

			// 目的を達したので関数を抜ける
			return;
		}
	}

	assert(false && "指定されたステージ名は存在しません");
}
