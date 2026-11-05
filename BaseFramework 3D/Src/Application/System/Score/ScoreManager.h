#pragma once

#include "../SkillCheck/SkillCheckBase.h"

class ScoreManager
{
public:
	enum class Rank
	{
		C,
		B,
		A,
		S
	};

	int CalcScore(int baseScore, float progress, SkillResult result = SkillResult::Good) const;

	Rank CalcRank(int score) const;

	static const char* RankToString(Rank rank);

private:
	// ランクの境界スコア(要調整)
	static constexpr int kRankS = 3000;
	static constexpr int kRankA = 2000;
	static constexpr int kRankB = 1000;

public:

	// シングルトンパターン
	static ScoreManager& Instance()
	{
		static ScoreManager instance;
		return instance;
	}
};