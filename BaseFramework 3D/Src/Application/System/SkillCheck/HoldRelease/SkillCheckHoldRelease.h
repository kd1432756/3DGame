#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 長押し＆離し
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
	float m_value = 0.0f;
	float m_fillTime = 0.7f;	// Maxまでの秒数
	float m_perfW = 0.08f;
	float m_goodW = 0.25f;

	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};