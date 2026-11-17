#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class ScoreUI : public PlayerUIBase
{
public:
	void Update() override;
	void DrawSprite() override;

private:
	int m_displayScore = 0;

	static constexpr float kScoreX = 600.0f;
	static constexpr float kScoreY = 300.0f;

	static constexpr  float kDigitWidth = 48.0f;
};
