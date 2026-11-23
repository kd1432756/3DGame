#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class BonusUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	std::shared_ptr<KdTexture> m_keyTex;

	float m_animTime = 0.0f;

	int m_prevBonusCount = 0;
	float m_popTimer = 0.0f;
	float m_ringTimer = 0.0f;

	static constexpr float kBonusX = 430.0f;
	static constexpr float kBonusY = 220.0f;
	static constexpr int   kBarHalfW = 48;
	static constexpr int   kBarHalfH = 4;
	static constexpr float kBarOffsetY = -28.0f;

	static constexpr float kRateGap = 4.0f;

	static constexpr float kScoreOffsetY = 28.0f;

	static constexpr float kPromptX = 0.0f;
	static constexpr float kPromptY = -50.0f;

	static constexpr float kPopTime = 0.35f;
	static constexpr float kRingTime = 0.5f;
	static constexpr float kPopHeight = 12.0f;

	static constexpr float kRingStartRadius = 60.0f;
	static constexpr float kRingEndRadius = 100.0f;
};