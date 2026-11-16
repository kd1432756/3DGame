#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 連打
//==================================================
class SkillCheckMash : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::Mash; }

protected:
	void OnStart() override;

	void OnUpdate(float dt) override;

	void OnDraw() override;

private:
	int   m_required = 12;
	float m_gauge = 0.0f;
	float m_inputTime = 0.0f;	// 経過時間
	float m_perfectTime = 1.5f;
	float m_pushFlash = 0.0f;

	static constexpr float kDecay = 0.1f;	// 1秒あたりの減少量
	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};