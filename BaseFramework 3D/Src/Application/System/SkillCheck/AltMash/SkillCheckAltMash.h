#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 交互押し
//==================================================
class SkillCheckAltMash : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::AltMash; }

protected:
	void OnStart() override;
	
	void OnUpdate(float dt) override;
	
	void OnDraw() override;	

private:
	int   m_required = 10;
	float m_gauge = 0.0f;
	float m_inputTime = 0.0f;
	float m_perfectTime = 1.5f;
	float m_wrongFlash = 0.0f;
	bool  m_expectLeft = true;	

	static constexpr float kDecay = 0.25f;
	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kY = -120.0f;
	static constexpr float kSideHalf = 18.0f;
	static constexpr float kSideOffset = 30.0f;
};