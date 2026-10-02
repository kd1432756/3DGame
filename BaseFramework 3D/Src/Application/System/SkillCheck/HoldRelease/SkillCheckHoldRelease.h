#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 長押し離し
// スペースを押し続けてゲージを溜め、右端の緑ゾーンで離す
//  黄色 = Perfect / 緑 = Good / 手前で離す・溜めすぎ(満タン) = Miss
//==================================================
class SkillCheckHoldRelease : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::HoldRelease; }

protected:
	void OnStart() override;

	void OnUpdate(float dt) override;

	void OnDraw() override;

private:
	static float Clamp(float v, float lo, float hi)
	{
		return (v < lo) ? lo : (v > hi) ? hi : v;
	}

	bool  m_holding = false;
	float m_value = 0.0f;	// 0.0〜1.0
	float m_fillTime = 0.7f;	// 満タンまでの秒数
	float m_perfW = 0.08f;
	float m_goodW = 0.25f;

	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};