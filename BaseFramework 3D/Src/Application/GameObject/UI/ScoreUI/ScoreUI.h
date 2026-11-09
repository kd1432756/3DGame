#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class ScoreUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	int m_displayScore = 0;

	static constexpr float ScoreX = 600.0f;
	static constexpr float ScoreY = 300.0f;
};
