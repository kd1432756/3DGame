#include "ScoreManager.h"

int ScoreManager::CalcScore(int baseScore, float progress, SkillResult result, float bonusRate) const
{
	float skillRate = 1.0f;
	switch (result)
	{
	case SkillResult::Perfect: skillRate = 1.5f; break;
	case SkillResult::Good:    skillRate = 1.0f; break;
	case SkillResult::Miss:    skillRate = 0.5f; break;
	default: break;
	}

	// 朝×1.0 → 夜×1.5 に緩やかに上昇
	float timeRate = 1.0f + 0.5f * progress;

	return std::max(1, static_cast<int>(std::round(baseScore * skillRate * timeRate * bonusRate)));
}

ScoreManager::Rank ScoreManager::CalcRank(int score) const
{
	if (score >= kRankS) return Rank::S;
	if (score >= kRankA) return Rank::A;
	if (score >= kRankB) return Rank::B;
	return Rank::C;
}

const char* ScoreManager::RankToString(Rank rank)
{
	switch (rank)
	{
	case Rank::S: return "S";
	case Rank::A: return "A";
	case Rank::B: return "B";
	default:      return "C";
	}
}