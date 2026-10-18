#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 連打
// スペースを連打してゲージを満タンにする(ゲージは常に少しずつ減る)
//  素早く満タン = Perfect / 制限時間内に満タン = Good / 間に合わない = Miss
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
	int   m_required = 15;
	float m_gauge = 0.0f;		// 0.0〜1.0
	float m_inputTime = 0.0f;		// 入力受付開始からの経過時間
	float m_perfectTime = 1.5f;
	float m_pushFlash = 0.0f;

	static constexpr float kDecay = 0.2f;	// 1秒あたりの減少量
	static constexpr float kBarWidth = 400.0f;
	static constexpr float kBarHeight = 24.0f;
	static constexpr float kBarY = -120.0f;
};