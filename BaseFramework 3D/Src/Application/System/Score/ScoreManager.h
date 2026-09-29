#pragma once

#include "../SkillCheck/SkillCheckBase.h"

class CharacterBase;

class ScoreManager
{
public:
	void Init();

	void RegisterCharacter(const std::shared_ptr<CharacterBase>& chara);

	std::vector<std::shared_ptr<CharacterBase>> GetRanking() const;
	
	int CalcScore(int baseScore, float progress, SkillResult result = SkillResult::Good) const;
private:
	std::vector<std::weak_ptr<CharacterBase>> m_characters;

public:

	// シングルトンパターン
	static ScoreManager& Instance()
	{
		static ScoreManager instance;
		return instance;
	}
};