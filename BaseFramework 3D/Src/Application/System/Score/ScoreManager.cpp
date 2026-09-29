#include "ScoreManager.h"

#include "../../GameObject/Character/CharacterBase.h"

void ScoreManager::Init()
{
	m_characters.clear();
}

void ScoreManager::RegisterCharacter(const std::shared_ptr<CharacterBase>& chara)
{
	if (!chara)
	{
		return;
	}

	// 無効になったweak_ptrを削除
	m_characters.erase(
		std::remove_if(
			m_characters.begin(),
			m_characters.end(),
			[](const std::weak_ptr<CharacterBase>& weak)
			{
				return weak.expired();
			}),
		m_characters.end());

	// 二重登録チェック
	for (const auto& weak : m_characters)
	{
		if (auto existing = weak.lock())
		{
			if (existing.get() == chara.get())
			{
				return;
			}
		}
	}

	m_characters.push_back(chara);
}

std::vector<std::shared_ptr<CharacterBase>> ScoreManager::GetRanking() const
{
	std::vector<std::shared_ptr<CharacterBase>> ranking;

	for (const auto& weak : m_characters)
	{
		if (auto chara = weak.lock())
		{
			ranking.push_back(chara);
		}
	}

	// スコアの高い順に並べる
	std::sort(
		ranking.begin(),
		ranking.end(),
		[](const auto& a, const auto& b)
		{
			return a->GetScore() > b->GetScore();
		}
	);

	return ranking;
}

int ScoreManager::CalcScore(int baseScore, float progress, SkillResult result) const
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

	return std::max(1, static_cast<int>(std::round(baseScore * skillRate * timeRate)));
}