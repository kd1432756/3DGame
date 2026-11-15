// BonusUI.h
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

	// ---- 調整用パラメータ(画面中央原点・Y上向き) ----
	static constexpr float kBonusX = 500.0f;		// ScoreUIの真下に合わせて調整
	static constexpr float kBonusY = 215.0f;
	static constexpr int   kBarHalfW = 60;
	static constexpr int   kBarHalfH = 4;
	static constexpr float kBarOffsetY = -28.0f;	// ラベルからバーまでの距離

	static constexpr float kPromptX = 0.0f;
	static constexpr float kPromptY = -50.0f;
};