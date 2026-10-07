#pragma once

#include "KamataEngine.h"

/// <summary>
/// 軸平行境界ボックス
/// </summary>
struct AABB {
	KamataEngine::Vector3 min;
	KamataEngine::Vector3 max;
};