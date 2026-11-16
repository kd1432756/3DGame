#pragma once
#include "SkillCheckBase.h"

class SkillCheckFactory
{
public:
	static std::unique_ptr<SkillCheckBase> Create(
		int  prevType = -1,
		float progress = 0.0f);

	// 種類を指定して生成
	static std::unique_ptr<SkillCheckBase> Make(SkillCheckType type);
};