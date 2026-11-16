#pragma once
#include "../SkillCheckBase.h"

//==================================================
// ゲージ止め
//==================================================
class SkillCheckGaugeStop : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::GaugeStop; }

protected:
	void OnStart() override;
	void OnUpdate(float dt) override;
	void OnDraw() override;

private:
	float m_cursor = 0.0f;
	float m_dir = 1.0f;	
	float m_speed = 1.0f;

	float m_zoneCenter = 0.5f;
	float m_goodWidth = 0.25f;	// Goodの幅
	float m_perfWidth = 0.09f;	// Perfectの幅

	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};